# DIST-001: real first-run default ordering and config-preserving package setup.
param([string]$Label='distribution',[string]$Package,[switch]$Existing,[switch]$Normal)
$ErrorActionPreference='Stop';$root=Split-Path $PSScriptRoot -Parent
$argsExtra=if($Normal){@()}else{@('-ezv-first-run')}
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration Release -Executable "$Package/ezquake.exe" -TimeoutSeconds 120 -ExtraArguments $argsExtra -Commands @('cfg_save_unchanged 1','cfg_save distribution-result','echo DISTRIBUTION_COMPLETE') -PrepareProfile {
 param($p)
 New-Item -ItemType Directory -Force "$p/ezquake"|Out-Null
 New-Item -ItemType HardLink -Path "$p/ezquake/ezquake.pk3" -Target "${env:EZQUAKE_GAME_DIR}/ezquake/ezquake.pk3"|Out-Null
 if($Existing) {
  @('r_cv_edgewidth "1.1"','r_cv_midtone "0.9"','bind w "+forward"','alias personal_test "echo retained"','cfg_save_onquit 0')|Set-Content -Encoding ascii "$p/ezquake/configs/config.cfg"
  @('r_cv_edgewidth "2.6"','bind w "+back"','set autoexec_seen 1')|Set-Content -Encoding ascii "$p/qw/autoexec.cfg"
  $beforeConfig=(Get-FileHash "$p/ezquake/configs/config.cfg").Hash
  $beforeAuto=(Get-FileHash "$p/qw/autoexec.cfg").Hash
 }
 & "$Package/Start.ps1" -GameDirectory $p -PrepareOnly
 # A second setup pass must preserve same-named user-edited preset files.
 $profile="$p/ezquake/presets/graphics/builtin/Balanced.cfg"
 Add-Content -Encoding ascii $profile '// user-owned test annotation'
 $beforePreset=(Get-FileHash $profile).Hash
 & "$Package/Start.ps1" -GameDirectory $p -PrepareOnly
 if((Get-FileHash $profile).Hash -ne $beforePreset){throw 'Setup replaced a user preset'}
 if($Existing -and ((Get-FileHash "$p/ezquake/configs/config.cfg").Hash -ne $beforeConfig -or (Get-FileHash "$p/qw/autoexec.cfg").Hash -ne $beforeAuto)){throw 'Setup changed personal config/autoexec'}
}
