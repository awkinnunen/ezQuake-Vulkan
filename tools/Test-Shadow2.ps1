# SHADOW-002. Cache invalidation, budgets, map/spot lights. Codex 2026-09-14.
param([string]$Label='shadow2-extended',[string]$Configuration='Release')
$ErrorActionPreference='Stop';$root=Split-Path $PSScriptRoot -Parent
function Frames([int]$n=24){1..$n|ForEach-Object{if($_%8 -eq 0){'dev_competitive checkpoint'};'wait'}}
$steps=@('developer 1','con_notify 0','con_notifytime 0','scr_centertime 0','viewsize 120','scr_newhud 0','show_fps 0','crosshair 0','r_drawviewmodel 0','gl_polyblend 0','r_fastsky 1','gl_flashblend 0','r_cv_enable 0','r_cv_hdr 1','r_cv_ao 0','r_cv_bloom 0','r_cv_shadows 1','r_dynamic 1','r_cv_shadowdistance 2048','r_cv_shadowquality 256','r_cv_shadowlights 1','r_cv_shadowupdates 8','vid_framebuffer_multisample 4','vid_framebuffer_fxaa 0','cmd pos_origin 1312.0625 880.0625 -256')+(Frames 120)+@('dev_competitive photo freeze','dev_competitive photo camera 480.0625 48.0625 46.0938 0 90 0','dev_competitive photo actors','dev_competitive photo light 0 480 90 72 350','vid_restart')+(Frames)
$cases=@(
 @('uncached',@('r_cv_shadowcache 0')),
 @('cached',@('r_cv_shadowcache 1')),
 @('move',@('dev_competitive photo light 0 520 90 72 350')),
 @('move-uncached',@('r_cv_shadowcache 0')),
 @('actor',@('dev_competitive photo actor 0 -35 0 0','r_cv_shadowcache 1')),
 @('actor-uncached',@('r_cv_shadowcache 0')),
 @('eight',(@('r_cv_shadowlights 8')+@(1..7|ForEach-Object{"dev_competitive photo light $_ $(440+$_*10) 100 72 300"}))),
 @('cached-eight-direct',@('r_cv_shadowcache 1')),
 @('budget',@('r_cv_shadowcache 0','r_cv_shadowupdates 1')),
 @('cached-eight',@('r_cv_shadowcache 1')),
 @('fresh-eight',@('r_cv_shadowcache 0','r_cv_shadowupdates 8')),
 @('overflow',@('r_cv_shadowcasters 64','r_cv_shadowcache 0')),
 @('overflow-reference',@('r_cv_shadowstrength 0','r_cv_shadowcasters 4096')),
 @('baked',(@('r_cv_shadowstrength 1','r_cv_shadowcasters 4096','r_cv_shadowlights 1','r_cv_shadowupdates 8','r_cv_maplights 0','dev_competitive photo light 0 0 0 0 0')+@(1..7|ForEach-Object{"dev_competitive photo light $_ 0 0 0 0"}))),
 @('map-shadows',@('r_cv_maplights 1')),
 @('spot-realtime',@('r_cv_maplights 2')),
 @('spot-soft',@('r_cv_shadowsoft 2')),
 @('ambient',@('r_cv_mapambient 0.4')),
 @('map-radius',@('r_cv_maplightscale 0.5')),
 @('map-eight',@('r_cv_shadowlights 8','r_cv_maplightscale 1')),
 @('quality512',@('r_cv_shadowquality 512')),
 @('quality128',@('r_cv_shadowquality 128')),
 @('off',@('r_cv_shadows 0')),
 @('on-return',@('r_cv_shadows 1','r_cv_shadowquality 256','r_cv_shadowcache 1'))
)
$records=@();$shot=0
foreach($case in $cases){$steps+=@("echo SHADOW2_CASE $($case[0])")+$case[1]+(Frames)+@('dev_competitive shadows','screenshot');$records+=@{name=$case[0];shot=('ezquake{0:d3}.png' -f $shot)};$shot++}
$steps+=@('echo SHADOW2_COMPLETE')
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -TimeoutSeconds 180 -GameLibrary "${env:EZQUAKE_GAME_DIR}/qw/qwprogs.dll" -BeforeMap @('sv_cheats 1') -ExtraArguments @('-visual-tests') -Commands $steps -PrepareProfile {
 param($p)
 '// Isolated fixture.'|Set-Content -Encoding ascii "$p/qw/server.cfg"
 New-Item -ItemType Directory -Force "$p/qw/lights" | Out-Null
 @('{','"classname" "light"','"origin" "480 90 100"','"light" "1000"','"target" "shadow-test-target"','"_cone" "100"','}','{','"classname" "info_null"','"targetname" "shadow-test-target"','"origin" "480 250 35"','}') | Set-Content -Encoding ascii "$p/qw/lights/e1m1.vklights"
 $records|ConvertTo-Json|Set-Content -Encoding utf8 "$p/shadow2-spec.json"
}
$log=Get-Content "$root/cache/runtime-$Label/qw/qconsole.log" -Raw
if($log -notmatch 'SHADOW2_COMPLETE' -or $log -notmatch 'lights=8' -or $log -notmatch 'cached=8' -or $log -notmatch 'faces=1 ' -or $log -notmatch 'overflow=[1-9]'){throw 'Missing extended shadow coverage'}
Write-Output 'PASS: eight lights, cached atlases, dirty updates, budget/overflow fallback, BSP lights and one-face spotlights.'





