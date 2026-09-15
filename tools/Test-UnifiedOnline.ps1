# Owned loopback UDP server, separate process: exercises the remote-client path.
param([string]$Label='unified-online',[string]$Configuration='Debug',[int]$Port=27991)
$ErrorActionPreference='Stop';$root=Split-Path $PSScriptRoot -Parent
$serverProfile=Join-Path $root "cache/server-$Label"
if(Test-Path $serverProfile){throw 'Use a fresh label'}
New-Item -ItemType Directory -Force "$serverProfile/id1","$serverProfile/qw"|Out-Null
foreach($folder in @('id1','qw')){Get-ChildItem "${env:EZQUAKE_GAME_DIR}/$folder" -File|Where-Object{$_.Extension -in '.pak','.pk3'}|ForEach-Object{New-Item -ItemType HardLink "$serverProfile/$folder/$($_.Name)" -Target $_.FullName|Out-Null}}
New-Item -ItemType HardLink "$serverProfile/qw/qwprogs.dll" -Target "${env:EZQUAKE_GAME_DIR}/qw/qwprogs.dll"|Out-Null
@('cfg_save_onquit 0','cl_maxfps 30','spectator 0','sv_progtype 1','sv_progsname qwprogs','maxclients 8','deathmatch 3','set k_fb_enabled 1','set k_fb_admin_only 0','map dm6')|Set-Content -Encoding ascii "$serverProfile/qw/server-test.cfg"
$exe=Join-Path $root "build-msvc-x64/$Configuration/ezquake.exe"
$server=Start-Process $exe -WorkingDirectory $serverProfile -WindowStyle Hidden -ArgumentList @('-allowmultiple','-clientport','27011','-window','-width','640','-height','480','-ip','127.0.0.1','-port',"$Port",'-condebug','-nohome','-basedir','.','+exec','server-test.cfg') -PassThru -RedirectStandardError "$serverProfile/stderr.log"
try{
 $deadline=(Get-Date).AddSeconds(20)
 do{Start-Sleep -Milliseconds 250;if($server.HasExited){throw 'Dedicated server exited'};$log=if(Test-Path "$serverProfile/qw/qconsole.log"){Get-Content "$serverProfile/qw/qconsole.log" -Raw}else{''}}while($log -notmatch 'dm6' -and (Get-Date) -lt $deadline)
 function Frames([int]$n=90){1..$n|ForEach-Object{if($_%8 -eq 0){'dev_local_menu checkpoint'};'wait'}}
 $steps=@('developer 1','menu_ingame 1','con_notifytime 0')+(Frames 180)+@('togglemenu','dev_ingame bots','echo ONLINE_BASE','dev_ingame status')+(Frames 18)+@('screenshot','dev_ingame add')+(Frames 90)+@('echo ONLINE_ADDED','dev_ingame status','dev_ingame removeall')+(Frames 90)+@('echo ONLINE_REMOVED','dev_ingame status','echo UNIFIED_ONLINE_COMPLETE')
 & "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -Commands $steps -ExtraArguments @('-allowmultiple','-clientport','27012') -TimeoutSeconds 180 -PrepareProfile {
  param($p)
  $run=Get-Content "$p/qw/run.cfg" -Raw
  $run.Replace('map e1m1',"connect 127.0.0.1:$Port")|Set-Content -Encoding ascii "$p/qw/run.cfg"
 }
 $log=Get-Content "$root/cache/runtime-$Label/qw/qconsole.log" -Raw
 foreach($c in @(@('ONLINE_BASE',0),@('ONLINE_ADDED',1),@('ONLINE_REMOVED',0))){if($log -notmatch ("(?s)"+$c[0]+"\r?\nINGAME_STATE active=1 local=0 bots_page=1 available=1[^\r\n]*\r?\nINGAME_BOTS count="+$c[1])){throw "Remote menu state mismatch: $($c[0])"}}
 'PASS: separate UDP server, client local=0; advertised KTX capability, add and remove bot.'
}finally{if(!$server.HasExited){Stop-Process -Id $server.Id};$server.Dispose()}
