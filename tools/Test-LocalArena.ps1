# OpenAI Codex, 2026-09-13. Exercise production menu handlers and real KTX.
param([string]$Label = 'arena', [ValidateSet('Debug','Release')][string]$Configuration = 'Release',
    [ValidateSet('Vulkan','OpenGL')][string]$Renderer = 'Vulkan', [int]$ExpectedMapCount=123)
$ErrorActionPreference = 'Stop'
if ($Label -notmatch '^[a-zA-Z0-9_-]+$') { throw 'Invalid label.' }
$taskRoot = Split-Path $PSScriptRoot -Parent
$taskProfile = Join-Path $taskRoot "cache/runtime-$Label"
if (Test-Path $taskProfile) { throw 'Use a new label.' }
New-Item -ItemType Directory -Force "$taskProfile/id1","$taskProfile/qw","$taskProfile/ezquake/configs" | Out-Null
foreach ($folder in @('id1','qw')) {
    Get-ChildItem "${env:EZQUAKE_GAME_DIR}/$folder" -File | Where-Object { $_.Extension -in @('.pak','.pk3') } | ForEach-Object {
        New-Item -ItemType HardLink -Path "$taskProfile/$folder/$($_.Name)" -Target $_.FullName | Out-Null
    }
}
New-Item -ItemType HardLink -Path "$taskProfile/qw/qwprogs.dll" -Target "${env:EZQUAKE_GAME_DIR}/qw/qwprogs.dll" | Out-Null
New-Item -ItemType HardLink -Path "$taskProfile/ezquake/ezquake.pk3" -Target "${env:EZQUAKE_GAME_DIR}/ezquake/ezquake.pk3" | Out-Null
New-Item -ItemType Directory "$taskProfile/qw/maps" | Out-Null
'invalid map' | Set-Content -LiteralPath "$taskProfile/qw/maps/unsafe;quit.bsp"
'invalid map' | Set-Content -LiteralPath "$taskProfile/qw/maps/bad space.bsp"
function Frames([int]$Count = 90) {
    for ($i = 0; $i -lt $Count; $i++) {
        if ($i % 10 -eq 0) { 'dev_local_menu checkpoint' }
        'wait'
    }
}
function Row([int]$Index) {
    if ($Index -ge 3) { $Index++ } # existing action indexes, before DMM row
    'dev_local_menu key HOME'
    for ($i = 0; $i -lt $Index; $i++) { 'dev_local_menu key DOWNARROW' }
}
$steps = @('alias f_spawn ""','disconnect','cfg_save_onquit 0','cl_confirmquit 0','cl_onload console','developer 1',
    'cl_maxfps 60','vid_vsync 0','menu_ingame 0','menu_main') + (Frames 30) + @('screenshot',
    'dev_local_menu mainkey HOME','dev_local_menu mainkey DOWNARROW','dev_local_menu mainkey DOWNARROW',
    'dev_local_menu mainkey ENTER','echo ARENA_MAIN','dev_local_menu inspect') + (Frames 30) + @('screenshot') +
    (Row 0) + @('dev_local_menu key ENTER') + (Frames 20) + @('screenshot','dev_local_menu key END',
    'dev_local_menu key HOME','dev_local_menu key F5','dev_local_menu map dm4','dev_local_menu key ENTER') +
    (Row 4) + @('dev_local_menu key ENTER','echo ARENA_GUARD','dev_local_menu inspect') +
    (Row 1) + @('dev_local_menu key ENTER') + (Frames 180) + @('menu_local','echo ARENA_STARTED','dev_local_menu inspect') +
    @('echo ARENA_AUTO_FFA','dev_local_menu inspect') +
    @('dev_local_menu key HOME','dev_local_menu key DOWNARROW','dev_local_menu key DOWNARROW','dev_local_menu key DOWNARROW','dev_local_menu key LEFTARROW','dev_local_menu key LEFTARROW','dev_local_menu key ENTER') + (Frames 30) +
    @('echo ARENA_DMM1','dev_local_menu inspect','dev_local_menu key RIGHTARROW','dev_local_menu key RIGHTARROW','dev_local_menu key ENTER') + (Frames 30) +
    @('echo ARENA_DMM3','dev_local_menu inspect') + (Row 2) + @('dev_local_menu key ENTER') + (Frames 90) + @('menu_local') +
    (Row 3) + @('dev_local_menu key RIGHTARROW') + (Row 4) + @('dev_local_menu key ENTER') + (Frames 60) +
    @('dev_local_menu key ENTER') + (Frames 60) + @('echo ARENA_TWO_BOTS','dev_local_menu inspect','screenshot') +
    (Row 5) + @('dev_local_menu key ENTER') + (Frames 60) + @('echo ARENA_ONE_BOT','dev_local_menu inspect') +
    (Row 6) + @('dev_local_menu key ENTER') + (Frames 60) + @('echo ARENA_NO_BOTS','dev_local_menu inspect') +
    (Row 2) + @('dev_local_menu key RIGHTARROW','dev_local_menu key ENTER') + (Frames 60) +
    @('echo ARENA_DUEL','dev_local_menu inspect','dev_local_menu key RIGHTARROW','dev_local_menu key ENTER') + (Frames 60) +
    @('echo ARENA_TDM','dev_local_menu inspect','dev_local_menu key RIGHTARROW','dev_local_menu key ENTER') + (Frames 90) + @('menu_local') +
    (Row 4) + @('dev_local_menu key ENTER') + (Frames 60) + @('dev_local_menu key ENTER') + (Frames 60) +
    @('echo ARENA_CA','dev_local_menu inspect') + (Row 7) + @('dev_local_menu key ENTER') + (Frames 720) +
    @('menu_local','echo ARENA_READY','dev_local_menu inspect','screenshot') +
    (Row 9) + @('dev_local_menu key ENTER') + (Frames 30) + @('echo ARENA_STOPPED','dev_local_menu inspect') +
    (Row 4) + @('dev_local_menu key ENTER','echo ARENA_STOP_GUARD','dev_local_menu inspect',
    'disconnect','maxclients 1','deathmatch 0','coop 0','sv_progsname spprogs','sv_progtype 0','map start') + (Frames 180) + @('menu_local') + (Row 4) + @('dev_local_menu key ENTER',
    'echo ARENA_SP_GUARD','dev_local_menu inspect') + (Row 1) + @('dev_local_menu map dm6','dev_local_menu key ENTER') +
    (Frames 180) + @('menu_local','echo ARENA_RESTART','dev_local_menu inspect') + (Row 9) +
    @('dev_local_menu key ENTER','echo ARENA_COMPLETE','quit')
$steps | Set-Content -Encoding ascii "$taskProfile/qw/arena-test.cfg"
# Run interaction after initialization. Host_Init flushes startup commands,
# including waits, before normal rendering and client/server frame processing.
@('cfg_save_onquit 0','cl_confirmquit 0','cl_maxfps 60','vid_vsync 0','developer 1','tp_triggers 1',
    'alias f_spawn "exec arena-test.cfg"','map e1m1') | Set-Content -Encoding ascii "$taskProfile/qw/run.cfg"
$taskExe = Join-Path $taskRoot "build-msvc-x64/$Configuration/ezquake.exe"
$taskRenderer = if ($Renderer -eq 'Vulkan') { '2' } else { '1' }
$taskAssert = $env:SDL_ASSERT
try {
    $env:SDL_ASSERT = 'abort'
    $taskProcess = Start-Process -FilePath $taskExe -WorkingDirectory $taskProfile -ArgumentList @('-dev','-condebug','-nohome','-basedir','.','-window','-width','800','-height','600','+set','vid_renderer',$taskRenderer,'+exec','run.cfg') -WindowStyle Hidden -PassThru -RedirectStandardError "$taskProfile/stderr.log"
    $null = $taskProcess.Handle
} finally { $env:SDL_ASSERT = $taskAssert }
try {
    $deadline = [DateTime]::UtcNow.AddSeconds(90)
    while (!$taskProcess.WaitForExit(1000)) {
        if ([DateTime]::UtcNow -gt $deadline) { throw "Arena runtime timed out: $taskProfile" }
    }
    $log = Get-Content "$taskProfile/qw/qconsole.log" -Raw
    if ($taskProcess.ExitCode -ne 0 -or $log -notmatch 'ARENA_COMPLETE' -or $log -match 'VUID-|Validation Error|Cbuf_AddText: overflow|Bad user command|infinite loop') { throw "Arena runtime failed: $taskProfile" }
    foreach ($check in @(
        @('ARENA_MAIN','LOCAL_STATE[^\r\n]*arena=1'),
        @('ARENA_GUARD','LOCAL_MENU[^\r\n]*connected=0 bots=0'),
        @('ARENA_STARTED','LOCAL_MENU[^\r\n]*selected=dm4[^\r\n]*connected=1'),
        @('ARENA_AUTO_FFA','LOCAL_DMM selected=3 live=3'),
        @('ARENA_AUTO_FFA','LOCAL_STATE[^\r\n]*mode=ffa'),
        @('ARENA_DMM1','LOCAL_DMM selected=1 live=1'),
        @('ARENA_DMM3','LOCAL_DMM selected=3 live=3'),
        @('ARENA_CA','LOCAL_DMM selected=5 live=5'),
        @('ARENA_TWO_BOTS','LOCAL_MENU[^\r\n]*connected=1 bots=2 skill=6'),
        @('ARENA_ONE_BOT','LOCAL_MENU[^\r\n]*bots=1'),
        @('ARENA_NO_BOTS','LOCAL_MENU[^\r\n]*bots=0'),
        @('ARENA_DUEL','LOCAL_STATE[^\r\n]*mode=1on1'),
        @('ARENA_TDM','LOCAL_STATE[^\r\n]*mode=2on2'),
        @('ARENA_CA','LOCAL_MENU[^\r\n]*connected=1 bots=2'),
        @('ARENA_CA','LOCAL_STATE[^\r\n]*ca=1'),
        @('ARENA_READY','LOCAL_STATE[^\r\n]*ca=1 status=0 min left'),
        @('ARENA_STOPPED','LOCAL_MENU[^\r\n]*connected=0 bots=0[^\r\n]*server=0'),
        @('ARENA_STOP_GUARD','LOCAL_MENU[^\r\n]*connected=0 bots=0[^\r\n]*server=0'),
        @('ARENA_SP_GUARD','LOCAL_MENU[^\r\n]*connected=0 bots=0[^\r\n]*server=1'),
        @('ARENA_RESTART','LOCAL_STATE[^\r\n]*map=dm6')
    )) {
        $pos = $log.IndexOf($check[0] + "`n")
        if ($pos -lt 0) { $pos = $log.IndexOf($check[0] + "`r`n") }
        if ($pos -lt 0 -or $log.Substring($pos, [Math]::Min(600,$log.Length - $pos)) -notmatch $check[1]) { throw "Missing evidence: $($check[0]) $($check[1]); $taskProfile" }
    }
    if ($log -notmatch 'LoadLibrary \(.*qwprogs\.dll\)') { throw 'Native KTX not loaded.' }
    $before = [regex]::Match($log, '(?s)ARENA_CA\r?\n(.*?)LOCAL_BOT id=(\d+) pos=([^\r\n]+)')
    $after = [regex]::Match($log, '(?s)ARENA_READY\r?\n(.*?)LOCAL_BOT id=(\d+) pos=([^\r\n]+)')
    if (!$before.Success -or !$after.Success -or $before.Groups[2].Value -ne $after.Groups[2].Value -or $before.Groups[3].Value -eq $after.Groups[3].Value) { throw 'Bot movement was not observed during the match.' }
    if ($log -notmatch "LOCAL_MENU maps=$ExpectedMapCount ") { throw 'Map enumeration/filter fixture changed; set ExpectedMapCount for the installed data set.' }
    Write-Output "PASS: $Label ($Renderer $Configuration), main menu, map picker, KTX start, add/remove bots, CA, ready, stop, SP isolation, restart, screenshots, exit 0. $taskProfile"
} finally {
    if (!$taskProcess.HasExited) { Stop-Process -Id $taskProcess.Id }
    $taskProcess.Dispose()
}
