# DIST-002: run ONLY against a disposable fresh Starter installation.
param([Parameter(Mandatory=$true)][string]$Installation,[switch]$SecondRun,[switch]$Arena)
$ErrorActionPreference='Stop'
$Installation=(Resolve-Path -LiteralPath $Installation).Path
if (!(Test-Path "$Installation/starter-install.json")) { throw 'Expected a Starter installation.' }
$autoexec="$Installation/qw/autoexec.cfg"
$original=[IO.File]::ReadAllBytes($autoexec)
$phase=if($Arena){'arena'}elseif($SecondRun){'second'}else{'first'}
if($Arena){
    $extra=''
    $frames=@(1..240|ForEach-Object{if($_%8 -eq 0){'dev_local_menu checkpoint'};'wait'})
    @('cl_confirmquit 0','developer 1','cl_maxfps 60','tp_triggers 1','alias f_spawn "exec starter-qa-arena.cfg"','menu_local','dev_local_menu map dm6','dev_local_menu bots 1','dev_local_menu start')|Set-Content "$Installation/qw/starter-qa.cfg" -Encoding ascii
    @('alias f_spawn ""')+$frames+@('echo STARTER_ARENA_STATE','dev_local_menu inspect','screenshot','echo STARTER_ARENA_COMPLETE','quit')|Set-Content "$Installation/qw/starter-qa-arena.cfg" -Encoding ascii
}elseif($SecondRun){
    Add-Content -LiteralPath "$Installation/ezquake/configs/config.cfg" -Encoding ascii -Value @('r_cv_edgewidth 1.7','bind w +back')
    $extra="`r`nr_cv_edgewidth 2.8`r`n"
    @('cl_confirmquit 0','cfg_save_unchanged 1','cfg_save starter-second','echo STARTER_SECOND_COMPLETE','quit')|Set-Content "$Installation/qw/starter-qa.cfg" -Encoding ascii
}else{
    $extra=''
    @('cl_confirmquit 0','tp_triggers 1','alias f_spawn "exec starter-qa-start.cfg"','newgame')|Set-Content "$Installation/qw/starter-qa.cfg" -Encoding ascii
    @('echo STARTER_START_OK','alias f_spawn "exec starter-qa-level.cfg"','map e1m1')|Set-Content "$Installation/qw/starter-qa-start.cfg" -Encoding ascii
    @('alias f_spawn ""','echo STARTER_E1M1_OK','menu_ingame 0','menu_main','togglemenu','cfg_save_unchanged 1','cfg_save starter-first','save starter-e1m1')+@(1..25|ForEach-Object{'wait'})+@('screenshot','echo STARTER_FIRST_COMPLETE','quit')|Set-Content "$Installation/qw/starter-qa-level.cfg" -Encoding ascii
}
[IO.File]::WriteAllBytes($autoexec,($original+[Text.Encoding]::ASCII.GetBytes($extra)))
$process=$null
try {
    $process=Start-Process powershell.exe -ArgumentList @('-NoProfile','-ExecutionPolicy','Bypass','-File',('"'+$Installation+'/Launch.ps1"'),'-Diagnostics','-Windowed','-StartupConfig','starter-qa.cfg') -WorkingDirectory $Installation -WindowStyle Hidden -PassThru -RedirectStandardOutput "$Installation/qa-$phase.out" -RedirectStandardError "$Installation/qa-$phase.err"
    $null=$process.Handle
    if(!$process.WaitForExit(45000)){throw 'Starter runtime test timeout.'}
    if($process.ExitCode -ne 0){throw "Launcher failed: see qa-$phase.err"}
    $log=Get-Content "$Installation/qw/qconsole.log" -Raw
    if($log -notmatch "STARTER_$($phase.ToUpper())_COMPLETE" -or $log -match 'VUID-|Validation Error|VK_ERROR_DEVICE_LOST|couldn.t load progs.dat|Host_Error'){throw 'Starter runtime failed.'}
    if($Arena -and $log -notmatch '(?s)STARTER_ARENA_STATE\r?\nLOCAL_MENU[^\r\n]*connected=1 bots=1'){throw 'Bundled KTX bot did not join the arena.'}
    if(!(Test-Path "$Installation/ezquake/ezv-distribution.json")){throw 'First-run completion marker not written.'}
    if(!(Test-Path "$Installation/engine/logs")){throw 'Diagnostics were not collected.'}
    Write-Output "PASS: Starter $phase launch, real launcher, exit 0 and diagnostics."
} finally {
    # Only test-owned processes in the disposable installation are stopped.
    if($process -and !$process.HasExited){
        Get-Process ezquake -ErrorAction SilentlyContinue | Where-Object {$_.Path -eq "$Installation\engine\ezquake.exe"} | Stop-Process
        Stop-Process -Id $process.Id
    }
    if($process){$process.Dispose()}
    [IO.File]::WriteAllBytes($autoexec,$original)
}
