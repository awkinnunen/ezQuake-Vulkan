# CFG-PRESETS-002, OpenAI Codex, 2026-09-15. Exercise native activation events.
param([string]$Label='preset-activation',[string]$Configuration='Debug',[int]$Width=800,[int]$Height=600)
$ErrorActionPreference='Stop';$root=Split-Path $PSScriptRoot -Parent
function Frames {1..18|ForEach-Object{if($_%8 -eq 0){'dev_competitive checkpoint'};'wait'}}
$steps=@('developer 1','con_notifytime 0','scr_centertime 0','cfg_save_unchanged 1','volume 0.37','bind w +forward',
 'gfx load Balanced','menu_graphics','cfg_save activation-base',
 'dev_graphics browse','dev_graphics key F3','dev_graphics cancel',
 'dev_graphics browse','dev_graphics preview Ultra-competitive')+(Frames)+@('screenshot',
 'echo ACTIVATE_ENTER','dev_graphics key ENTER','togglemenu','cfg_save activation-enter',
 'menu_graphics','dev_graphics browse','dev_graphics preview Athmospheric')+(Frames)+@(
 'echo ACTIVATE_MOUSE','dev_graphics presetclick','togglemenu','cfg_save activation-mouse',
 'menu_graphics','dev_graphics browse','dev_graphics preview Balanced','dev_graphics key ESCAPE','cfg_save activation-cancel',
 'dev_graphics browse','dev_graphics preview invalid-activation','echo ACTIVATE_INVALID','dev_graphics key ENTER','cfg_save activation-invalid','dev_graphics key ESCAPE',
 'dev_graphics browse','dev_graphics preview Balanced','echo ACTIVATE_SHORTCUT','dev_graphics key F3','togglemenu','cfg_save activation-f3',
 'menu_graphics','dev_graphics browse','dev_graphics key END','dev_graphics key UPARROW','dev_graphics key DOWNARROW')+(Frames)+@(
 'echo ACTIVATE_NAVIGATION','dev_graphics key ENTER','togglemenu','cfg_save activation-navigation',
 'menu_graphics')+(Frames)+@('screenshot','echo PRESET_ACTIVATION_COMPLETE')
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -Width $Width -Height $Height -TimeoutSeconds 180 -ExtraArguments @('-visual-tests') -Commands $steps -PrepareProfile {
 param($p)
 New-Item -ItemType Directory -Force "$p/ezquake/presets/graphics"|Out-Null
 @('r_cv_exposure 2','map dm6')|Set-Content -Encoding ascii "$p/ezquake/presets/graphics/invalid-activation.cfg"
 Copy-Item -Recurse "$root/profiles/ezquake/presets/graphics/builtin" "$p/ezquake/presets/graphics"
 # END deterministically reaches a valid legacy row; navigation must activate it.
 New-Item -ItemType Directory -Force "$p/ezquake/competitive"|Out-Null
 'r_cv_exposure "1.7"'|Set-Content -Encoding ascii "$p/ezquake/competitive/last-activation.cfg"
}
