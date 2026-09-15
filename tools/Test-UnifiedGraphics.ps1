# MENU-UNIFY-001 / CFG-PRESETS-001. Real menu/cvar/file regression, Codex 2026-09-15.
param([string]$Label='unified-graphics',[string]$Configuration='Debug',[int]$Width=800,[int]$Height=600)
$ErrorActionPreference='Stop';$root=Split-Path $PSScriptRoot -Parent
function Frames([int]$n=18){1..$n|ForEach-Object{if($_%8 -eq 0){'dev_competitive checkpoint'};'wait'}}
$steps=@('developer 1','con_notifytime 0','scr_centertime 0','cfg_save_unchanged 1','r_cv_hdr 0','r_cv_exposure 1.05','r_cv_bloom 0.15','gl_outline 3','r_cv_shadows 1','r_dynamic 1','gl_flashblend 0','bind w +forward','volume 0.37','vid_restart')+(Frames)+@(
 'menu_graphics','dev_graphics inventory','cfg_save gfx-base','dev_graphics select r_cv_shadowstrength','dev_graphics key UPARROW','dev_graphics key DOWNARROW')+(Frames)+@('screenshot',
 'dev_graphics browse','dev_graphics preview test-a','cfg_save gfx-a')+(Frames)+@('screenshot',
 'dev_graphics preview test-b','cfg_save gfx-b','dev_graphics cancel','cfg_save gfx-cancel',
 'dev_graphics browse','dev_graphics preview test-invalid','cfg_save gfx-invalid','dev_graphics cancel',
 'dev_graphics browse','dev_graphics preview test-a','dev_graphics key F5','dev_graphics cancel','cfg_save gfx-restart-cancel',
 'dev_graphics browse','dev_graphics preview test-b','dev_graphics apply','cfg_save gfx-applied',
 'dev_graphics browse','dev_graphics saveas test-roundtrip','dev_graphics cancel',
 'dev_graphics browse','dev_graphics scope 1','dev_graphics reset','cfg_save gfx-reset','dev_graphics cancel','cfg_save gfx-reset-cancel',
 'r_cv_exposure 0.7','dev_graphics browse','dev_graphics preview test-roundtrip','dev_graphics apply','cfg_save gfx-roundtrip',
 'r_cv_exposure 1.6','dev_graphics savecurrent','gfx load test-roundtrip','cfg_save gfx-overwrite',
 'r_cv_exposure 1.8','dev_graphics savecurrent',
 'dev_graphics browse','dev_graphics saveas test-roundtrip','dev_graphics cancel',
 'dev_graphics browse','dev_graphics scope 3','dev_graphics preview test-a','cfg_save gfx-scope','dev_graphics cancel',
 'dev_graphics select r_cv_exposure','dev_graphics key UPARROW','dev_graphics key DOWNARROW')+(Frames)+@('dev_graphics mouse 0','cfg_save gfx-mouse','r_cv_exposure 1.8',
 'dev_graphics select r_cv_shadowstrength','dev_graphics key UPARROW','dev_graphics key DOWNARROW','r_cv_shadows 0','dev_graphics key RIGHTARROW')+(Frames)+@('dev_graphics mouse 1','cfg_save gfx-disabled',
 'dev_graphics browse','dev_graphics preview test-a','togglemenu','cfg_save gfx-closed',
 'menu_graphics','dev_graphics browse')+(Frames)+@('screenshot','dev_graphics cancel',
 'gfx load Balanced','cfg_save gfx-balanced','gfx load Ultra-competitive','cfg_save gfx-ultra',
 'gfx load Athmospheric','cfg_save gfx-atmospheric','vid_restart')+(Frames)+@('screenshot',
 'keyboard_preset sdfe')+(Frames)+@('cfg_save keys-sdfe','keyboard_preset wasd')+(Frames)+@('cfg_save keys-wasd',
 'gfx load Balanced','cfg_save gfx-final','menu_controls')+(Frames)+@('screenshot','disconnect','dev_ingame bots','echo UNIFIED_GRAPHICS_COMPLETE')
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -Width $Width -Height $Height -TimeoutSeconds 180 -ExtraArguments @('-visual-tests') -BeforeMap @('sv_cheats 1') -Commands $steps -PrepareProfile {
 param($p)
 New-Item -ItemType Directory -Force "$p/ezquake/presets/graphics"|Out-Null
 @('r_cv_exposure "0.8"','gl_outline "1"','r_cv_shadowstrength "0.3"','r_cv_hdr "1"')|Set-Content -Encoding ascii "$p/ezquake/presets/graphics/test-a.cfg"
 'r_cv_exposure 1.8'|Set-Content -Encoding ascii "$p/ezquake/presets/graphics/test-b.cfg"
 @('r_cv_exposure 2','map dm6')|Set-Content -Encoding ascii "$p/ezquake/presets/graphics/test-invalid.cfg"
 Copy-Item -Recurse "$root/profiles/ezquake/presets/graphics/builtin" "$p/ezquake/presets/graphics"
 Copy-Item "$root/profiles/qw/ezv-wasd.cfg","$root/profiles/qw/ezv-sdfe.cfg" "$p/qw"
}
