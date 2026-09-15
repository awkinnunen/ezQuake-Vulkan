# MENU-UNIFY-001. Real native menus and KTX requests, OpenAI Codex, 2026-09-15.
param([string]$Label='unified-arena',[string]$Configuration='Debug')
$ErrorActionPreference='Stop';$root=Split-Path $PSScriptRoot -Parent
function Frames([int]$n=90){1..$n|ForEach-Object{if($_%8 -eq 0){'dev_local_menu checkpoint'};'wait'}}
$steps=@('developer 1','menu_ingame 1','con_notifytime 0','menu_local','dev_local_menu map dm6','dev_local_menu bots 1')+(Frames 15)+@('screenshot','dev_local_menu start')+(Frames 240)+@(
 'echo ARENA_INITIAL','dev_local_menu inspect','togglemenu','dev_ingame bots')+(Frames 18)+@('screenshot','dev_ingame add')+(Frames 90)+@(
 'echo ARENA_ADDED','dev_local_menu inspect','dev_ingame remove')+(Frames 90)+@('echo ARENA_REMOVED','dev_local_menu inspect',
 'menu_local','dev_local_menu key HOME','dev_local_menu key DOWNARROW','dev_local_menu key RIGHTARROW','echo ARENA_SETUP_ONLY','dev_local_menu inspect')+(Frames 18)+@(
 'screenshot','togglemenu','togglemenu','dev_ingame bots','dev_ingame removeall')+(Frames 90)+@('echo ARENA_EMPTY','dev_local_menu inspect','echo UNIFIED_ARENA_COMPLETE')
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -GameLibrary "${env:EZQUAKE_GAME_DIR}/qw/qwprogs.dll" -BeforeMap @('deathmatch 3','maxclients 16','k_fb_enabled 1') -Commands $steps -TimeoutSeconds 180
$log=Get-Content "$root/cache/runtime-$Label/qw/qconsole.log" -Raw
foreach($c in @(@('ARENA_INITIAL',1),@('ARENA_ADDED',2),@('ARENA_REMOVED',1),@('ARENA_EMPTY',0))){
 if($log -notmatch ("(?s)"+$c[0]+"\r?\nLOCAL_MENU[^\r\n]*connected=1 bots="+$c[1])){throw "Bot state mismatch: $($c[0])"}
}
if($log -notmatch 'INGAME_STATE active=1 local=1 bots_page=1 available=1'){throw 'KTX bot capability missing'}
if($log -notmatch '(?s)ARENA_SETUP_ONLY\r?\nLOCAL_MENU[^\r\n]*\r?\nLOCAL_DMM selected=3 live=3\r?\nLOCAL_STATE[^\r\n]*mode=ffa map=dm6'){throw 'Setup affected running match'}
'PASS: native Local Arena setup and in-game KTX add/remove; setup edits leave running match unchanged.'
