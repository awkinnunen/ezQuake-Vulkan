# SHADOW-001. Native KTX weapon/projectile path, no synthetic light. Codex 2026-09-14.
param([string]$Label='shadow-combat')
$ErrorActionPreference='Stop';$taskRoot=Split-Path $PSScriptRoot -Parent
function Frames([int]$n){for($i=0;$i -lt $n;$i++){if($i%8 -eq 0){'dev_competitive checkpoint'};'wait'}}
$steps=@('developer 1','r_cv_shadows 1','r_cv_shadowlights 2','r_cv_shadowdistance 2048','r_cv_hdr 1','r_cv_ao 0.125','r_cv_aomode 1',
 'r_dynamic 1','gl_flashblend 0','vid_restart','give 1 7','give 1 r 100','give 1 h 1000')+(Frames 20)+@('impulse 7')+(Frames 20)+
 @('echo COMBAT_WEAPON $weaponnum','+attack')
for($i=0;$i -lt 20;$i++){$steps+=(Frames 6)+@('dev_competitive shadows');if($i%5 -eq 0){$steps+=@('screenshot')}}
$steps+=@('-attack','echo SHADOW_COMBAT_COMPLETE')
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration Debug -TimeoutSeconds 120 `
 -GameLibrary "${env:EZQUAKE_GAME_DIR}/qw/qwprogs.dll" -BeforeMap @('sv_cheats 1','maxclients 8','deathmatch 1') -Commands $steps -PrepareProfile {
 param($path)
 '// Isolated combat test.'|Set-Content -Encoding ascii "$path/qw/server.cfg"
}
$log=Get-Content "$taskRoot/cache/runtime-$Label/qw/qconsole.log" -Raw
if($log -notmatch 'COMBAT_WEAPON 7' -or $log -notmatch 'SHADOW_COMBAT_COMPLETE' -or $log -notmatch 'lights=[12].*faces=(6|12)'){throw 'Combat coverage missing'}
Write-Output 'PASS: native rocket launcher firing with selected shadow lights and normal shutdown.'
