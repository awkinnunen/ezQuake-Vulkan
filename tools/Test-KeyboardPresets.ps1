# INPUT-004. Native menu actions and repeated preset transitions, OpenAI Codex.
param([string]$Label='keyboard-presets',[string]$Configuration='Debug')
$ErrorActionPreference='Stop';$root=Split-Path $PSScriptRoot -Parent
function Frames {1..16|ForEach-Object{if($_%8 -eq 0){'dev_competitive checkpoint'};'wait'}}
$steps=@('developer 1','con_notifytime 0','cfg_save_unchanged 1','cfg_save_aliases 1',
 'gfx load Balanced','sensitivity 3.17','volume 0.37','alias user_sentinel "echo personal-alias"',
 'alias f_weaponchange "echo personal-hook"','menu_controls')+(Frames)+@('screenshot',
 'dev_controls key HOME','dev_controls key DOWNARROW','dev_controls key DOWNARROW','dev_controls key ENTER')+(Frames)+@('cfg_save keyboard-nquake-first',
 'keyboard_preset quick-wasd')+(Frames)+@('cfg_save keyboard-quick-wasd',
 'dev_controls key HOME','dev_controls key DOWNARROW','dev_controls key ENTER')+(Frames)+@('cfg_save keyboard-quick-esdf',
 'keyboard_preset nquake')+(Frames)+@('cfg_save keyboard-nquake-after',
 'time_inc','cfg_save keyboard-timer','time_dec','vol+','vol-','demo_normal',
 'keyboard_preset wasd')+(Frames)+@('cfg_save keyboard-wasd-compat','keyboard_preset sdfe')+(Frames)+@('cfg_save keyboard-sdfe-compat',
 'keyboard_preset nquake')+(Frames)+@('cfg_save keyboard-nquake-final','screenshot','echo KEYBOARD_PRESETS_COMPLETE')
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -Width 640 -Height 480 -TimeoutSeconds 180 -Commands $steps -PrepareProfile {
 param($p)
 New-Item -ItemType Directory -Force "$p/ezquake/presets/graphics"|Out-Null
 Copy-Item -Recurse "$root/profiles/ezquake/presets/graphics/builtin" "$p/ezquake/presets/graphics"
 Copy-Item "$root/profiles/qw/ezv-wasd.cfg","$root/profiles/qw/ezv-sdfe.cfg","$root/profiles/qw/ezv-nquake.cfg" "$p/qw"
}
