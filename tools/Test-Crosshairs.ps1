# OpenAI Codex, 2026-09-13. Isolated legacy controls and crosshair integration test.
param([Parameter(Mandatory=$true)][string]$GameDirectory, [Parameter(Mandatory=$true)][string]$Executable, [string]$Label='crosshairs', [ValidateSet('Debug','Release')][string]$Configuration='Release', [ValidateSet('SinglePlayer','QuakeWorld')][string]$Mode='SinglePlayer')
$ErrorActionPreference='Stop'
if ($Label -notmatch '^[a-zA-Z0-9_-]+$') { throw 'Invalid label' }
$taskRoot=Split-Path $PSScriptRoot -Parent
$taskProfile=Join-Path $taskRoot "cache/runtime-$Label"
if(Test-Path -LiteralPath $taskProfile) { throw 'Use a fresh test label' }
New-Item -ItemType Directory -Force "$taskProfile/id1","$taskProfile/qw","$taskProfile/ezquake/configs","$taskProfile/bin" | Out-Null
foreach($folder in @('id1','qw')) {
    Get-ChildItem "$GameDirectory/$folder" -File | Where-Object { $_.Extension -in @('.pak','.pk3') } | ForEach-Object {
        New-Item -ItemType HardLink -Path "$taskProfile/$folder/$($_.Name)" -Target $_.FullName | Out-Null
    }
}
# INPUT-005: exercise only the publicly packaged profiles, never a private config.
Copy-Item -LiteralPath $Executable -Destination "$taskProfile/bin/ezquake.exe"
Copy-Item "$taskRoot/profiles/qw/ezv-sdfe.cfg","$taskRoot/profiles/qw/ezv-crosshairs.cfg" "$taskProfile/qw"
'exec ezv-sdfe.cfg' | Set-Content -Encoding ascii "$taskProfile/ezquake/configs/config.cfg"
Copy-Item -Recurse "$taskRoot/profiles/ezquake/crosshairs" "$taskProfile/ezquake"
New-Item -ItemType HardLink -Path "$taskProfile/ezquake/ezquake.pk3" -Target "$GameDirectory/ezquake/ezquake.pk3" | Out-Null
function Frames([int]$count=20) {
 for($i=0;$i -lt $count;$i++) { if($i%10 -eq 0) { 'dev_local_menu checkpoint' }; 'wait' }
}
@('cfg_save_onquit 0','cl_confirmquit 0','developer 1','cl_maxfps 60','vid_vsync 0','tp_triggers 1','sv_cheats 1',
 'alias f_spawn "exec controls-test.cfg"','newgame') | Set-Content -Encoding ascii "$taskProfile/qw/run.cfg"

if($Mode -eq 'QuakeWorld') {
 New-Item -ItemType HardLink -Path "$taskProfile/qw/qwprogs.dll" -Target "$GameDirectory/qw/qwprogs.dll" | Out-Null
 '// Isolated native module test: explicit run.cfg settings.' | Set-Content -Encoding ascii "$taskProfile/qw/server.cfg"
 @('cfg_save_onquit 0','cl_confirmquit 0','developer 1','cl_maxfps 60','vid_vsync 0','tp_triggers 1','sv_cheats 1','sv_progtype 1','sv_progsname qwprogs','maxclients 8','deathmatch 1',
 'alias f_spawn "exec controls-test.cfg"','map start') | Set-Content -Encoding ascii "$taskProfile/qw/run.cfg"
}
$initial=@('alias f_spawn ""','echo INPUT_BEGIN','menu_ingame 0','menu_main','togglemenu','impulse 9')+(Frames 20)+@('impulse 1')+(Frames 20)+
 @('alias f_weaponchange "legacy_crosshair;exec capture-2.cfg"','impulse 2')
if($Mode -eq 'QuakeWorld') {
 $give=@(3..8 | ForEach-Object { "give 1 $_" })+@('give 1 s 100','give 1 n 100','give 1 r 100','give 1 c 100','give 1 h 1000')
 $initial=@($initial | ForEach-Object { if($_ -eq 'impulse 9') { $give } else { $_ } })
}
$initial | Set-Content -Encoding ascii "$taskProfile/qw/controls-test.cfg"
# Each weapon-change trigger extends the production crosshair callback with a
# capture step. Let the queue drain: long queued scripts delay normal f_triggers.
foreach($weapon in 2..8) {
 $capture=@('alias f_weaponchange "legacy_crosshair"')+(Frames 20)+@('echo INPUT_WEAPON $weaponnum $crosshairimage','screenshot')
 if($weapon -lt 8) { $capture+=@("alias f_weaponchange `"legacy_crosshair;exec capture-$($weapon+1).cfg`"","impulse $($weapon+1)") }
 else { $capture+=@('exec fire-test.cfg') }
 $capture | Set-Content -Encoding ascii "$taskProfile/qw/capture-$weapon.cfg"
}
$fire=@('+legacy_shaft')+(Frames 8)+@('echo INPUT_LG_HELD $weaponnum','crosshairimage','screenshot','-legacy_shaft')+(Frames 8)+
 @('echo INPUT_LG_RELEASED $weaponnum $crosshairimage','legacy_denial_on','alias f_weaponchange "legacy_crosshair;exec rl-fire.cfg"','+legacy_rl')
$fire | Set-Content -Encoding ascii "$taskProfile/qw/fire-test.cfg"
@('alias f_weaponchange "legacy_crosshair;exec returned.cfg"')+(Frames 10)+@('-legacy_rl') | Set-Content -Encoding ascii "$taskProfile/qw/rl-fire.cfg"
$returned=@('alias f_weaponchange "legacy_crosshair"','echo INPUT_RETURN $weaponnum $crosshairimage','legacy_denial_off',
 '+legacy_fw','bind a','+legacy_bw','-legacy_fw','bind a','-legacy_bw','bind a',
 'echo INPUT_PITCH_BEFORE $cl_pitchspeed','save before-jump','legacy_rj')+(Frames 2)+@('echo INPUT_PITCH_AFTER $cl_pitchspeed','save after-jump')+(Frames 40)+
 @('legacy_sj')+(Frames 20)+@('echo INPUT_PITCH_AFTER_SJ $cl_pitchspeed','cfg_save controls-result','echo INPUT_COMPLETE','quit')
if($Mode -eq 'QuakeWorld') { $returned=@($returned | Where-Object { $_ -notmatch '^save ' }) }
$returned | Set-Content -Encoding ascii "$taskProfile/qw/returned.cfg"
$taskAssert=$env:SDL_ASSERT
try {
 $env:SDL_ASSERT='abort'
 $process=Start-Process -FilePath "$taskProfile/bin/ezquake.exe" -WorkingDirectory $taskProfile -ArgumentList @('-dev','-condebug','-nohome','-basedir','.','-window','-width','800','-height','600','+set','vid_renderer','2','+exec','run.cfg') -WindowStyle Hidden -PassThru -RedirectStandardError "$taskProfile/stderr.log"
 $null=$process.Handle
} finally { $env:SDL_ASSERT=$taskAssert }
try {
 $deadline=[DateTime]::UtcNow.AddSeconds(90)
 while(!$process.WaitForExit(1000)) { if([DateTime]::UtcNow -gt $deadline) { throw 'Controls runtime timed out' } }
 $log=Get-Content "$taskProfile/qw/qconsole.log" -Raw
 if($process.ExitCode -ne 0 -or $log -notmatch 'INPUT_COMPLETE' -or $log -match 'VUID-|Validation Error|Unknown command "(?!sv_enableprofile")|Couldn.t load image|PR_RunError|Host_Error|recursive alias|infinite loop|Cheats are not allowed') { throw "Controls test failed: $taskProfile" }
 foreach($check in @('INPUT_WEAPON 2 legacy_sg','INPUT_WEAPON 3 legacy_sg','INPUT_WEAPON 4 legacy_ng','INPUT_WEAPON 5 legacy_ng','INPUT_WEAPON 6 legacy_gl','INPUT_WEAPON 7 legacy_rl','INPUT_WEAPON 8 legacy_lg','INPUT_LG_RELEASED 8 legacy_lg','INPUT_RETURN 2 legacy_sg')) {
  if(!$log.Contains($check)) { throw "Missing: $check" }
 }
 $held=[regex]::Match($log,'(?s)INPUT_LG_HELD.*?INPUT_LG_RELEASED').Value
 if($held -notmatch 'crosshairimage[^\r\n]*default value[^\r\n]*""' -or $held -match 'current value is "legacy_lg"') { throw 'LG firing did not clear custom crosshair' }
 $pitches=[regex]::Matches($log,'INPUT_PITCH_(?:BEFORE|AFTER|AFTER_SJ) ([\d.]+)')
 if($pitches.Count -ne 3 -or $pitches[0].Groups[1].Value -ne $pitches[1].Groups[1].Value -or $pitches[0].Groups[1].Value -ne $pitches[2].Groups[1].Value) { throw 'Jump changed keyboard pitch speed' }
 if($log -notmatch '"a" = "legacy_rj"' -or $log -notmatch '"a" = "legacy_sj"') { throw 'Movement-dependent jump binding failed' }
 Write-Output "PASS: public Quick profile, seven weapon switches/five PNGs, LG fire/release, shotgun-return toggle, jump bindings and pitch restore; $taskProfile"
} finally {
 if(!$process.HasExited) { Stop-Process -Id $process.Id }
 $process.Dispose()
}
