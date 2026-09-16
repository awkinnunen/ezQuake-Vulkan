# DIST-002. No remembered absolute directory: the installed folder can be moved.
param([switch]$Diagnostics,[switch]$Windowed,[string]$StartupConfig)
$ErrorActionPreference='Stop'
$engine=Join-Path $PSScriptRoot 'engine'
$setup=& "$engine/Start.ps1" -GameDirectory $PSScriptRoot -PrepareOnly
$marker=Join-Path $PSScriptRoot 'ezquake/ezv-distribution.json'
$firstRun=!(Test-Path -LiteralPath $marker)
# GUI-subsystem executables do not reliably block an interactive PowerShell
# invocation. Own the process and wait explicitly before diagnostics/first-run marking.
$gameArgs=@('-nohome','-basedir','.','+set','vid_renderer','2')
if($firstRun){$gameArgs+='-ezv-first-run'}
if($Windowed){$gameArgs+=@('-window','-width','1280','-height','720')}else{$gameArgs+='-fullscreen'}
if($Diagnostics){$gameArgs+='-condebug'}
if($StartupConfig){
    if($StartupConfig -notmatch '^[A-Za-z0-9_-]+\.cfg$'){throw 'StartupConfig must be a plain CFG filename in qw.'}
    $gameArgs+=@('+exec',$StartupConfig)
}
$process=Start-Process -FilePath "$engine/ezquake.exe" -WorkingDirectory $PSScriptRoot -ArgumentList $gameArgs -PassThru
$null=$process.Handle
try { $process.WaitForExit(); $gameExit=$process.ExitCode } finally { $process.Dispose() }
if($Diagnostics){
    $logDir=Join-Path $engine ('logs/'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
    New-Item -ItemType Directory -Force -Path $logDir|Out-Null
    if(Test-Path "$PSScriptRoot/qw/qconsole.log"){Copy-Item -LiteralPath "$PSScriptRoot/qw/qconsole.log" -Destination $logDir}
    Copy-Item -LiteralPath "$engine/manifest.json" -Destination $logDir
    Write-Host "Diagnostic files: $logDir"
}
if($gameExit -ne 0){throw "Game exited with code $gameExit. Use Diagnose.cmd to collect a log."}
if($firstRun){
    @{initialized=(Get-Date -Format o);package=(Get-Content "$engine/manifest.json" -Raw|ConvertFrom-Json).version}|
        ConvertTo-Json|Set-Content -LiteralPath $marker -Encoding utf8
}
