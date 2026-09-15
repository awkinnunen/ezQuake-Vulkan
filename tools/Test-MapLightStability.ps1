# SHADOW-003, OpenAI Codex, 2026-09-15. Baked world lighting must not depend on selected static occluders.
param([string]$Label='maplight-stability',[string]$Configuration='Release',[ValidateSet('e1m1','dm6')][string]$Map='e1m1',[string]$Executable)
$ErrorActionPreference='Stop';$root=Split-Path $PSScriptRoot -Parent
function Frames([int]$n=18){1..$n|ForEach-Object{if($_%8 -eq 0){'dev_competitive checkpoint'};'wait'}}
$steps=@('developer 1','con_notifytime 0','scr_centertime 0','showpause 0','viewsize 120','scr_newhud 0','show_fps 0','crosshair 0','r_drawviewmodel 0','r_drawentities 0','r_drawparticles 0','gl_polyblend 0','r_fastsky 1','gl_flashblend 0','r_cv_enable 0','r_cv_hdr 1','r_cv_ao 0','r_cv_bloom 0','r_cv_shadows 1','r_dynamic 1','r_shadows 0','r_cv_shadowdistance 2048','r_cv_shadowquality 256','r_cv_shadowlights 8','r_cv_shadowupdates 8','r_cv_shadowcasters 8192','vid_framebuffer_multisample 4','vid_framebuffer_fxaa 0','cmd pos_origin 1312.0625 880.0625 -256')+(Frames 120)+@('dev_competitive photo freeze','dev_competitive photo light 0 0 0 0 0','vid_restart')+(Frames)
if($Map -eq 'dm6'){$steps=$steps|ForEach-Object{$_ -replace 'cmd pos_origin 1312.0625 880.0625 -256','cmd pos_origin 232 -1512 40'}}
$records=@();$shot=0
$positions=if($Map -eq 'dm6'){@(232,202,172)}else{@(480,510,540)}
foreach($x in $positions) {
 $steps+=@(if($Map -eq 'dm6'){"dev_competitive photo camera $x -1512 62 0 180 0"}else{"dev_competitive photo camera $x 48 46 0 90 0"})
 foreach($radius in @('0.25','1','4')) {foreach($mode in @(0,1)) {
  $steps+=@("r_cv_maplightscale $radius","r_cv_maplights $mode")+(Frames)+@('dev_competitive shadows')+(Frames 12)+@('screenshot')
  $records+=@{camera=$x;radius=$radius;mode=$mode;shot=('ezquake{0:d3}.png' -f $shot)};$shot++
 }}
}
$steps+=@('echo MAPLIGHT_STABILITY_COMPLETE')
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -Executable $Executable -TimeoutSeconds 180 -GameLibrary "${env:EZQUAKE_GAME_DIR}/qw/qwprogs.dll" -BeforeMap @('sv_cheats 1') -ExtraArguments @('-visual-tests') -Commands $steps -PrepareProfile {
 param($p)
 (Get-Content "$p/qw/run.cfg") -replace 'map e1m1',"map $Map" | Set-Content -Encoding ascii "$p/qw/run.cfg"
 '// Isolated fixture.'|Set-Content -Encoding ascii "$p/qw/server.cfg"
 $records|ConvertTo-Json|Set-Content -Encoding utf8 "$p/maplight-spec.json"
}
