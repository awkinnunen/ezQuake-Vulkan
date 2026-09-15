# SHADOW-001, real renderer/casters/lights. OpenAI Codex, 2026-09-14.
param([string]$Label='shadows',[string]$Configuration='Release',[switch]$ToggleOnly)
$ErrorActionPreference='Stop';$taskRoot=Split-Path $PSScriptRoot -Parent
function Frames([int]$n=18){for($i=0;$i -lt $n;$i++){if($i%8 -eq 0){'dev_competitive checkpoint'};'wait'}}
$steps=@('developer 1','con_notifytime 0','scr_centertime 0','showpause 0','cl_bonusflash 0',
 'viewsize 120','scr_newhud 0','show_fps 0','crosshair 0','r_drawviewmodel 0','gl_polyblend 0',
 'gl_gamma 1','gl_contrast 1','vid_software_palette 1','r_fastsky 1','r_fastturb 1','gl_flashblend 0',
 'r_drawparticles 0','gl_caustics 0','gl_detail 0','r_shadows 0','r_cv_enable 0','r_cv_hdr 1',
 'r_cv_tonemap 2','r_cv_exposure 1','r_cv_ao 0','r_cv_bloom 0','r_cv_shadows 0','r_dynamic 1',
 'r_cv_shadowcache 0','r_cv_shadowdistance 2048','r_cv_shadowquality 256','r_cv_shadowlights 1','r_cv_shadowstrength 1',
 'vid_framebuffer_multisample 4','vid_framebuffer_fxaa 0','gl_outline 0','cmd pos_origin 1312.0625 880.0625 -256')+
 (Frames 120)+@('dev_competitive photo freeze','dev_competitive photo camera 480.0625 48.0625 46.0938 0 90 0',
 'dev_competitive photo actors','dev_competitive photo light 0 480 90 72 350','vid_restart')+(Frames)
$cases=@(
 @{name='legacy';cmd=@()},
 @{name='shadow';cmd=@('r_cv_shadows 1')},
 @{name='unoccluded';cmd=@('r_cv_shadowstrength 0')},
 @{name='hard';cmd=@('r_cv_shadowstrength 1','r_cv_shadowsoft 0')},
 @{name='soft';cmd=@('r_cv_shadowsoft 2')},
 @{name='low';cmd=@('r_cv_shadowquality 128')},
 @{name='high';cmd=@('r_cv_shadowquality 512')},
 @{name='bias';cmd=@('r_cv_shadowbias 2')},
 @{name='moved-light';cmd=@('r_cv_shadowbias 0.5','dev_competitive photo light 0 520 90 72 350')},
 @{name='moved-actor';cmd=@('dev_competitive photo actor 0 -35 0 0')},
 @{name='two-lights';cmd=@('dev_competitive photo light 1 420 120 56 300','r_cv_shadowlights 2')},
 @{name='one-light';cmd=@('r_cv_shadowlights 1')},
 @{name='brush';cmd=@('dev_competitive photo brush')},
 @{name='moved-brush';cmd=@('dev_competitive photo actor 3 60 0 0')},
 @{name='far-light';cmd=@('dev_competitive photo light 0 480 650 72 350','dev_competitive photo light 1 0 0 0 0')},
 @{name='distance';cmd=@('r_cv_shadowdistance 128')},
 @{name='sdr';cmd=@('dev_competitive photo light 0 480 90 72 350','r_cv_shadowcache 0','r_cv_shadowdistance 2048','r_cv_hdr 0','vid_restart')},
 @{name='no-msaa';cmd=@('vid_framebuffer_multisample 0','vid_restart')},
 @{name='resize';cmd=@('vid_win_width 960','vid_win_height 540','vid_restart')}
)
if($ToggleOnly){$cases=@(@{name='legacy';cmd=@()},@{name='shadow';cmd=@('r_cv_shadows 1')},@{name='legacy-return';cmd=@('r_cv_shadows 0')})}
$records=@();$shot=0
foreach($case in $cases){
 $steps+=@("echo SHADOW_CASE $($case.name)")+$case.cmd+(Frames)+@('dev_competitive shadows')
 $shots=@();foreach($repeat in @(0,1)){$steps+=(Frames 12)+@('screenshot');$shots+=('ezquake{0:d3}.png' -f $shot);$shot++}
 $records+=@{name=$case.name;shots=$shots}
}
$steps+=@('echo SHADOW_COMPLETE','disconnect','menu_visual_effects','dev_competitive select r_cv_shadows')+(Frames)
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -TimeoutSeconds 180 `
 -GameLibrary "${env:EZQUAKE_GAME_DIR}/qw/qwprogs.dll" -BeforeMap @('sv_cheats 1') -ExtraArguments @('-visual-tests') -Commands $steps -PrepareProfile {
 param($path)
 $records|ConvertTo-Json -Depth 5|Set-Content -Encoding utf8 "$path/shadow-spec.json"
 '// Isolated fixture.'|Set-Content -Encoding ascii "$path/qw/server.cfg"
}
$log=Get-Content "$taskRoot/cache/runtime-$Label/qw/qconsole.log" -Raw
if($log -notmatch 'SHADOW_COMPLETE' -or $log -notmatch 'lights=1.*faces=6' -or (!$ToggleOnly -and $log -notmatch 'lights=2.*faces=12')){throw 'Shadow lights did not execute'}
Write-Output 'PASS: shadow lights, moves, controls, MSAA/SDR/resize and normal shutdown; compare pixels separately.'

