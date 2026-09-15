# PUBLIC-001 / MAINT-005, OpenAI Codex: exercise the overlay and restored conditional effects.
param([string]$Label='public-profiles')
$ErrorActionPreference='Stop'
$taskRoot=Split-Path $PSScriptRoot -Parent
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration Debug -TimeoutSeconds 90 -Commands @(
 'exec ezv-wasd-defaults.cfg','cfg_save_onquit 0','developer 1','dev_competitive runtime','dev_competitive video',
 'bind w','bind a','bind s','bind d','+legacy_fw','bind CAPSLOCK','-legacy_fw','bind CAPSLOCK',
 'cv save public-verified','cfg_save public-result','echo PUBLIC_PROFILE_COMPLETE','dev_competitive checkpoint'
) -PrepareProfile {
 param($path)
 Copy-Item "${env:EZQUAKE_GAME_DIR}/ezquake/configs/config.cfg" "$path/ezquake/configs/config.cfg"
 Copy-Item "$taskRoot/profiles/qw/*.cfg" "$path/qw/"
 New-Item -ItemType Directory -Force "$path/ezquake/competitive" | Out-Null
 Copy-Item "$taskRoot/profiles/ezquake/competitive/project-default.cfg" "$path/ezquake/competitive/"
 New-Item -ItemType HardLink -Path "$path/ezquake/ezquake.pk3" -Target "${env:EZQUAKE_GAME_DIR}/ezquake/ezquake.pk3" | Out-Null
}
$log=Get-Content "$taskRoot/cache/runtime-$Label/qw/qconsole.log" -Raw
foreach($expected in @('"w" = "+legacy_fw"','"a" = "+moveleft"','"s" = "+legacy_bw"','"d" = "+moveright"',
 '"CAPSLOCK" = "legacy_rj"','"CAPSLOCK" = "legacy_sj"','PUBLIC_PROFILE_COMPLETE')) {
 if(!$log.Contains($expected)){throw "Missing profile evidence: $expected"}
}
& "python" "$PSScriptRoot/Verify-PublicProfiles.py" $Label
if($LASTEXITCODE){throw 'Profile verification failed'}
