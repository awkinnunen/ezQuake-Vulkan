# MAINT-003, OpenAI Codex. Private demo/map/seek/restart regression.
param([string]$Label='motion-runtime',[string]$Configuration='Debug',[switch]$RasterFeatures,[switch]$Shadows)
$ErrorActionPreference='Stop'
$taskRoot=Split-Path $PSScriptRoot -Parent
function Frames([int]$count=90) { for($i=0;$i -lt $count;$i++) {if($i%10 -eq 0){'dev_competitive checkpoint'};'wait'} }
$featureSettings=if($RasterFeatures){@('r_cv_hdr 1','r_cv_tonemap 2','r_cv_ao 0.4','r_cv_aomode 1','r_cv_aoquality 16','r_cv_aoradius 32')}else{@()}
if($Shadows){$featureSettings+=@('r_cv_shadows 1','r_cv_shadowlights 2','r_cv_maplights 0','r_cv_shadowcache 0','gl_flashblend 0','r_dynamic 1','cl_hightrack 1')}
$steps=@('developer 1','cv load project-default')+$featureSettings+@('vid_restart','screenshot','dev_competitive runtime',
 'map dm4')+(Frames 120)+@('dev_competitive runtime','screenshot','map dm6')+(Frames 120)+
 @('dev_competitive runtime','screenshot','playdemo motion')+(Frames 100)+@('dev_competitive runtime',
 'demo_jump 12')+(Frames 60)+@('dev_competitive runtime','screenshot','demo_jump 3')+(Frames 60)+
 @('dev_competitive runtime','screenshot','skins','vid_restart','screenshot')+(Frames 30)+
 @('dev_competitive runtime','cl_demospeed 0')+(Frames 20)+@('dev_competitive runtime')+(Frames 20)+
 @('dev_competitive runtime','cl_demospeed 1','disconnect','map e1m1')+(Frames 120)+
 @('dev_competitive runtime','screenshot','echo MOTION_COMPLETE','dev_competitive checkpoint')
if($Shadows){$steps=@($steps|ForEach-Object{$_;if($_ -eq 'dev_competitive runtime'){'dev_competitive shadows'}})}
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -TimeoutSeconds 180 `
 -GameLibrary "${env:EZQUAKE_GAME_DIR}/qw/qwprogs.dll" -Commands $steps -PrepareProfile {
 param($path)
 '// Explicit local test server settings in run.cfg.' | Set-Content -Encoding ascii "$path/qw/server.cfg"
 Copy-Item "$taskRoot/cache/motion.mvd" "$path/qw/motion.mvd"
 New-Item -ItemType Directory -Force "$path/ezquake/competitive","$path/qw/skins" | Out-Null
 Copy-Item "$taskRoot/profiles/ezquake/competitive/project-default.cfg" "$path/ezquake/competitive/project-default.cfg"
 New-Item -ItemType HardLink -Path "$path/ezquake/ezquake.pk3" -Target "${env:EZQUAKE_GAME_DIR}/ezquake/ezquake.pk3" | Out-Null
 Get-ChildItem "${env:EZQUAKE_GAME_DIR}/qw/skins" -File | ForEach-Object {New-Item -ItemType HardLink -Path "$path/qw/skins/$($_.Name)" -Target $_.FullName | Out-Null}
}
$log=Get-Content "$taskRoot/cache/runtime-$Label/qw/qconsole.log" -Raw
if($log -notmatch 'MOTION_COMPLETE' -or $log -match 'Error: demo|Corrupted demo|infinite loop|Host_Error') {throw 'Motion runtime failed'}
if($RasterFeatures -and $log -notmatch 'RGBA16F linear'){throw 'HDR was not activated'}
$states=[regex]::Matches($log,'CV_RUNTIME map=(\S+) state=(\d+) demo=(\d+) time=([\d.]+) seeking=(\d+) players=(\d+) vulkan=(\d+) msaa=(\d+)')
if($states.Count -ne 10) {throw "Expected 10 states, got $($states.Count)"}
foreach($map in @('maps/e1m1.bsp','maps/dm4.bsp','maps/dm6.bsp')) {if($log -notmatch [regex]::Escape("CV_RUNTIME map=$map")){throw "Missing map $map"}}
$demo=@($states | Where-Object {$_.Groups[3].Value -ne '0'})
if($demo.Count -lt 5) {throw 'Missing demo states'}
$times=@($demo | ForEach-Object {[double]::Parse($_.Groups[4].Value,[cultureinfo]::InvariantCulture)})
if($times[1] -lt 12 -or $times[2] -ge $times[1] -or $times[-1] -ne $times[-2]) {throw "Seek/pause states failed: $times"}
Write-Output 'PASS: three maps, demo playback, forward/backward seeking, skin reload, video restart and paused demo stability.'

if($Shadows -and $log -notmatch 'lights=[12].*faces=(6|12)'){throw 'No real shadowed dynamic lights observed'}

