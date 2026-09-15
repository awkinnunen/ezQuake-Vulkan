# DIST-001. OpenAI Codex, 2026-09-15. Run against separately installed game data.
param([string]$GameDirectory,[switch]$Diagnostics,[switch]$Windowed,[switch]$PrepareOnly)
$ErrorActionPreference='Stop'
$packageRoot=$PSScriptRoot
$remember=Join-Path $packageRoot 'GameDirectory.txt'
if (!$GameDirectory -and (Test-Path -LiteralPath $remember)) { $GameDirectory=(Get-Content -LiteralPath $remember -Raw).Trim() }
if (!$GameDirectory) { $GameDirectory=Read-Host 'Path to your installed nQuake folder (containing id1 and qw)' }
$GameDirectory=(Resolve-Path -LiteralPath $GameDirectory.Trim('"')).Path
if (!(Test-Path -LiteralPath (Join-Path $GameDirectory 'id1/pak0.pak'))) { throw 'Game data missing: install nQuake or supply id1/pak0.pak first.' }
if (!(Test-Path -LiteralPath (Join-Path $GameDirectory 'ezquake/ezquake.pk3'))) { throw 'Install the nQuake ezquake/ezquake.pk3 resource pack first. Enhanced particle settings require it.' }
$prefix=$GameDirectory.TrimEnd('\')+'\'
$copied=0;$preserved=0
foreach($file in Get-ChildItem -LiteralPath (Join-Path $packageRoot 'profiles') -Recurse -File) {
 $relative=$file.FullName.Substring((Join-Path $packageRoot 'profiles').Length+1)
 $destination=[IO.Path]::GetFullPath((Join-Path $GameDirectory $relative))
 if (!$destination.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid package profile path.' }
 if (Test-Path -LiteralPath $destination) { $preserved++;continue }
 New-Item -ItemType Directory -Force (Split-Path $destination -Parent)|Out-Null
 Copy-Item -LiteralPath $file.FullName -Destination $destination
 $copied++
}
$marker=Join-Path $GameDirectory 'ezquake/ezv-distribution.json'
$firstRun=!(Test-Path -LiteralPath $marker)
$bootstrap=Join-Path $GameDirectory 'qw/ezv-dist-first-run.cfg'
$bootstrapSource=Join-Path $packageRoot 'ezv-dist-first-run.cfg'
if($firstRun) {
 if(Test-Path -LiteralPath $bootstrap) {
  if((Get-FileHash -LiteralPath $bootstrap).Hash -ne (Get-FileHash -LiteralPath $bootstrapSource).Hash) { throw 'An existing ezv-dist-first-run.cfg differs; preserve it and rename it before first setup.' }
 } else { Copy-Item -LiteralPath $bootstrapSource -Destination $bootstrap }
}
Write-Output "Profiles installed: $copied; existing files preserved: $preserved."
if($PrepareOnly) { [pscustomobject]@{GameDirectory=$GameDirectory;FirstRun=$firstRun};return }
[IO.File]::WriteAllText($remember,$GameDirectory)
$gameArgs=@('-nohome','-basedir',$GameDirectory,'+set','vid_renderer','2')
if($firstRun){$gameArgs+='-ezv-first-run'}
if($Windowed){$gameArgs+=@('-window','-width','1280','-height','720')}else{$gameArgs+='-fullscreen'}
if($Diagnostics){$gameArgs+='-condebug'}
$exe=Join-Path $packageRoot 'ezquake.exe'
Push-Location -LiteralPath $GameDirectory
try { & $exe @gameArgs; $gameExit=$LASTEXITCODE } finally { Pop-Location }
if($Diagnostics) {
 $logDir=Join-Path $packageRoot ('logs/'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
 New-Item -ItemType Directory -Force $logDir|Out-Null
 $consoleLog=Join-Path $GameDirectory 'qw/qconsole.log'
 if(Test-Path -LiteralPath $consoleLog){Copy-Item -LiteralPath $consoleLog -Destination (Join-Path $logDir 'qconsole.log')}
 Copy-Item -LiteralPath (Join-Path $packageRoot 'manifest.json') -Destination $logDir
 Write-Output "Diagnostic files: $logDir"
}
if($gameExit -ne 0){throw "Game exited with code $gameExit. Run Diagnose.cmd and include its log in a bug report."}
if($firstRun){[ordered]@{initialized=(Get-Date -Format o);package=(Get-Content -LiteralPath (Join-Path $packageRoot 'manifest.json') -Raw|ConvertFrom-Json).version}|ConvertTo-Json|Set-Content -LiteralPath $marker -Encoding utf8}
