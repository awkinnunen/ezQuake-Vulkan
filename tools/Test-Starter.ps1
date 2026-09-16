# DIST-002: archive boundaries, integrity failures, existing-directory protection and import.
param([Parameter(Mandatory=$true)][string]$WorkDirectory,
      [Parameter(Mandatory=$true)][string]$CacheDirectory,
      [string]$RegisteredDirectory,[switch]$TestNetwork)
$ErrorActionPreference='Stop'
$starter=Join-Path (Split-Path $PSScriptRoot -Parent) 'dist/windows/starter'
. "$starter/Common.ps1"
if(Test-Path -LiteralPath $WorkDirectory){throw 'Use a new disposable work directory.'}
New-Item -ItemType Directory -Path $WorkDirectory|Out-Null
$WorkDirectory=(Resolve-Path -LiteralPath $WorkDirectory).Path
$results=New-Object 'System.Collections.Generic.List[string]'
function MustFail([scriptblock]$Action,[string]$Name) {
    $failed=$false
    try { & $Action | Out-Null } catch { $failed=$true }
    if(!$failed){throw "Expected rejection: $Name"}
    $results.Add($Name)
}
foreach($bad in @('../escape','/absolute','C:/absolute','qw/a:stream','qw/../outside','qw/NUL.txt','qw/a.')) {
    MustFail {Get-SafePath $WorkDirectory $bad} "reject path $bad"
}
$zipPath=Join-Path $WorkDirectory 'traversal.zip'
$zip=[IO.Compression.ZipFile]::Open($zipPath,[IO.Compression.ZipArchiveMode]::Create)
try {
    $writer=New-Object IO.StreamWriter($zip.CreateEntry('good.txt').Open());$writer.Write('good');$writer.Dispose()
    $writer=New-Object IO.StreamWriter($zip.CreateEntry('../escape.txt').Open());$writer.Write('bad');$writer.Dispose()
}finally{$zip.Dispose()}
MustFail {Expand-CheckedZip $zipPath (Join-Path $WorkDirectory 'bad-extract')} 'reject complete malicious ZIP before any extraction'
if(Test-Path "$WorkDirectory/bad-extract/good.txt"){throw 'ZIP preflight wrote a partial archive.'}
$lock=Get-Content "$starter/downloads.lock.json" -Raw|ConvertFrom-Json
$tiny=[pscustomobject]@{name='test.zip';url=$lock.packages[0].url;bytes=4;sha256=('0'*64)}
$badCache=Join-Path $WorkDirectory 'bad-cache';New-Item -ItemType Directory $badCache|Out-Null
[IO.File]::WriteAllText((Join-Path $badCache ('000000000000-test.zip')),'oops')
MustFail {Get-VerifiedDownload $tiny $badCache} 'reject corrupt cached archive'
$existing=Join-Path $WorkDirectory 'existing';New-Item -ItemType Directory $existing|Out-Null
[IO.File]::WriteAllText("$existing/autoexec.cfg",'personal sentinel')
MustFail {& "$starter/Install.ps1" -Destination $existing -CacheDirectory $CacheDirectory -NonInteractive} 'refuse existing installation'
if([IO.File]::ReadAllText("$existing/autoexec.cfg") -ne 'personal sentinel'){throw 'Existing configuration changed.'}
MustFail {Import-RegisteredData $existing $existing} 'reject source without registered PAK data'
if($RegisteredDirectory){
    $installed=Join-Path $WorkDirectory 'Registered installation'
    & "$starter/Install.ps1" -Destination $installed -CacheDirectory $CacheDirectory -QuakeDirectory $RegisteredDirectory -NonInteractive | Out-Null
    if(!(Test-Path "$installed/id1/gpl_maps.pk3.disabled") -or (Test-Path "$installed/id1/gpl_maps.pk3")){throw 'Owned maps do not take priority.'}
    $receipt=Get-Content "$installed/starter-install.json" -Raw|ConvertFrom-Json
    if(@($receipt.registeredData).Count -lt 1){throw 'Import receipt missing.'}
    foreach($file in $receipt.registeredData){
        if((Get-FileHash "$installed/id1/$($file.file)").Hash -ne $file.sha256){throw 'Imported asset mismatch.'}
    }
    $before=(Get-FileHash "$installed/id1/pak1.pak").Hash
    MustFail {Import-RegisteredData $RegisteredDirectory $installed} 'reject repeated import without overwriting PAKs'
    if((Get-FileHash "$installed/id1/pak1.pak").Hash -ne $before){throw 'Existing PAK changed.'}
    $results.Add('registered import preserves names, hashes and substitute-map backup')
}
if($TestNetwork){
    $package=@($lock.packages|Where-Object kind -eq 'engine')[0]
    $file=Get-VerifiedDownload $package (Join-Path $WorkDirectory 'network-cache')
    if(!(Test-Path -LiteralPath $file)){throw 'Download missing.'}
    $results.Add('live GitHub HTTPS engine download and pinned SHA-256 validation')
}
$results|ConvertTo-Json|Set-Content "$WorkDirectory/results.json" -Encoding utf8
$results|ForEach-Object{Write-Output "PASS: $_"}
