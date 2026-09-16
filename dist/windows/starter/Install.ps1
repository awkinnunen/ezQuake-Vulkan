# DIST-002. OpenAI Codex, 2026-09-16. Unofficial nQuake-based Vulkan installation.
param([string]$Destination,[string]$QuakeDirectory,[string]$CacheDirectory,
      [switch]$NonInteractive)
$ErrorActionPreference='Stop'
. "$PSScriptRoot/Common.ps1"
if (![Environment]::Is64BitOperatingSystem) { throw 'Windows x64 is required.' }
$lock=Get-Content -LiteralPath "$PSScriptRoot/downloads.lock.json" -Raw | ConvertFrom-Json
Write-Host 'ezQuake Vulkan Starter - unofficial nQuake-based distribution'
Write-Host 'Downloads about 122 MB from nQuake and ezQuake-Vulkan GitHub Releases.'
Write-Host 'Third-party license notices are preserved. No game is launched by setup.'
if (!$Destination) {
    if ($NonInteractive) { throw '-Destination is required in non-interactive mode.' }
    $default=Join-Path $env:USERPROFILE 'Games/ezQuake-Vulkan'
    $Destination=Read-Host "New installation directory [Enter: $default]"
    if (!$Destination) { $Destination=$default }
}
$Destination=[IO.Path]::GetFullPath($Destination.Trim('"')).TrimEnd('\')
if (Test-Path -LiteralPath $Destination) { throw 'Destination already exists. Choose a new directory; setup never overwrites an installation.' }
if (!$QuakeDirectory -and !$NonInteractive) {
    $QuakeDirectory=Read-Host 'Optional: path to your OWN classic full Quake installation (Enter to skip)'
}
if ($QuakeDirectory) { $QuakeDirectory=$QuakeDirectory.Trim('"') }
if (!$CacheDirectory) { $CacheDirectory=Join-Path $env:LOCALAPPDATA 'ezQuake-Vulkan/downloads' }
$CacheDirectory=[IO.Path]::GetFullPath($CacheDirectory)
$parent=Split-Path $Destination -Parent
New-Item -ItemType Directory -Force -Path $parent | Out-Null
$stage=Join-Path $parent ('.ezv-install-'+[guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $stage | Out-Null
try {
    foreach ($package in $lock.packages) {
        $archive=Get-VerifiedDownload $package $CacheDirectory
        $out=if ($package.kind -eq 'engine') { Join-Path $stage 'engine-download' } else { $stage }
        Expand-CheckedZip $archive $out $package.kind
    }
    $engine=Join-Path $stage 'engine'
    [IO.Directory]::Move((Join-Path $stage ('engine-download/'+$lock.engineFolder)),$engine)
    # Preserve nQuake's first-run mechanism and its original autoexec verbatim.
    # nQuake loads preset.cfg once, after nquake_default.cfg. No forced overlay on later runs.
    $preset=Join-Path $stage 'ezquake/configs/preset.cfg'
    if (Test-Path -LiteralPath $preset) { throw 'Unexpected upstream preset.cfg; review the upstream package before updating Starter.' }
    $bootstrap=Get-Content -LiteralPath (Join-Path $engine 'ezv-dist-first-run.cfg') -Raw
    [IO.File]::WriteAllText($preset,('// Starter first-run preset: Balanced and Quick WASD.'+"`r`n"+$bootstrap+"`r`ncl_onload menu`r`n"),[Text.UTF8Encoding]::new($false))
    & (Join-Path $engine 'Start.ps1') -GameDirectory $stage -PrepareOnly | Out-Null
    $pak0=Join-Path $stage 'id1/pak0.pak'
    if (@(Get-PakEntries $pak0) -notcontains 'progs.dat') { throw 'Shareware game logic is missing.' }
    $imported=@()
    if ($QuakeDirectory) { $imported=@(Import-RegisteredData $QuakeDirectory $stage) }
    Copy-Item -LiteralPath "$PSScriptRoot/Launch.ps1" -Destination (Join-Path $stage 'Launch.ps1')
    foreach ($name in @('Start.cmd','Diagnose.cmd')) { Copy-Item -LiteralPath "$PSScriptRoot/$name" -Destination (Join-Path $stage $name) }
    Copy-Item -LiteralPath "$PSScriptRoot/README.txt" -Destination (Join-Path $stage 'START-HERE.txt')
    Copy-Item -LiteralPath "$PSScriptRoot/downloads.lock.json" -Destination (Join-Path $stage 'starter-downloads.lock.json')
    $receipt=[ordered]@{starterVersion=$lock.version;installed=(Get-Date -Format o);engineVersion=$lock.engineVersion;
        defaults='Balanced / Quick WASD';packages=$lock.packages;registeredData=$imported;
        attribution='nQuake and upstream contributors; installer by OpenAI Codex under user direction'}
    $receipt | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $stage 'starter-install.json') -Encoding utf8
    # Same-volume rename publishes the complete installation; existing target still fails closed.
    [IO.Directory]::Move($stage,$Destination)
    Write-Host "Installed: $Destination"
    Write-Host 'Run Start.cmd. Diagnose.cmd starts windowed and collects troubleshooting logs.'
    return $Destination
} catch {
    Write-Warning "Installation did not complete. Existing installations are untouched. Work files retained in $stage"
    throw
}
