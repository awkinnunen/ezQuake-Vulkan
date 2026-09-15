# Local smoke-test fixture by OpenAI Codex, 2026-09-13.
# Separate writable profile; hard links share asset bytes, never user configs.
param([string]$Label='smoke', [ValidateSet('Debug','Release')][string]$Configuration='Debug',
    [ValidateSet('Vulkan','OpenGL')][string]$Renderer='Vulkan', [switch]$TestSky, [string[]]$Commands=@(),
    [string]$GameLibrary, [string[]]$BeforeMap=@(), [scriptblock]$PrepareProfile,
    [string[]]$ExtraArguments=@(), [ValidateRange(5,180)][int]$TimeoutSeconds=45,
    [switch]$NoValidation, [int]$Width=800, [int]$Height=600, [string]$Executable)
$ErrorActionPreference = 'Stop'
if ($Label -notmatch '^[a-zA-Z0-9_-]+$') { throw 'Invalid test label.' }
$taskRoot = Split-Path $PSScriptRoot -Parent
$taskProfile = Join-Path $taskRoot "cache/runtime-$Label"
if (Test-Path $taskProfile) { throw "Use a new test label: $taskProfile already exists." }
New-Item -ItemType Directory -Force "$taskProfile/id1","$taskProfile/qw","$taskProfile/ezquake/configs" | Out-Null
foreach ($folder in @('id1','qw')) {
    Get-ChildItem "${env:EZQUAKE_GAME_DIR}/$folder" -File |
        Where-Object { $_.Extension -in @('.pak','.pk3') } | ForEach-Object {
            New-Item -ItemType HardLink -Path "$taskProfile/$folder/$($_.Name)" -Target $_.FullName | Out-Null
        }
}
if ($GameLibrary) {
    $taskLibrary = (Resolve-Path -LiteralPath $GameLibrary).Path
    New-Item -ItemType HardLink -Path "$taskProfile/qw/qwprogs.dll" -Target $taskLibrary | Out-Null
    $BeforeMap = @('sv_progtype 1','sv_progsname qwprogs') + $BeforeMap
}
if ($TestSky) {
    & "python" "$taskRoot/tools/make-test-sky.py" $taskProfile
    if ($LASTEXITCODE) { throw 'Sky fixture generation failed.' }
    $Commands = @('r_fastsky 0','loadsky codex','r_skyname') + $Commands
}
@('cfg_save_onquit 0','cl_confirmquit 0','cl_onload console','cl_maxfps 60','vid_vsync 0',
    'tp_triggers 1','alias f_spawn "exec finish.cfg"') + $BeforeMap + @('map e1m1') |
    Set-Content -Encoding ascii "$taskProfile/qw/run.cfg"
@('alias f_spawn ""','echo CODEX_MAP_SPAWNED','menu_ingame 0','menu_main','togglemenu') + $Commands +
    @((1..60 | ForEach-Object { 'wait' }), 'vid_gfxinfo','screenshot','echo CODEX_SMOKE_COMPLETE','quit') |
    Set-Content -Encoding ascii "$taskProfile/qw/finish.cfg"
$taskRenderer = if ($Renderer -eq 'Vulkan') { '2' } else { '1' }
$taskExe = Join-Path $taskRoot "build-msvc-x64/$Configuration/ezquake.exe"
if ($Executable) { $taskExe = (Resolve-Path -LiteralPath $Executable).Path }
$taskArgs = @('-condebug','-nohome','-basedir','.','-window','-width',"$Width",'-height',"$Height",
    '+set','vid_renderer',$taskRenderer,'+exec','run.cfg') + $ExtraArguments
if(!$NoValidation) { $taskArgs=@('-dev')+$taskArgs }
$taskAssert = $env:SDL_ASSERT
if ($PrepareProfile) { & $PrepareProfile $taskProfile }
# MAINT-005: missing this pack disables QMB initialization and makes supported
# VX cvars appear unknown. Preserve the real installation's particle resources.
$taskEffectsPack=Join-Path $env:EZQUAKE_GAME_DIR 'ezquake/ezquake.pk3'
if ((Test-Path $taskEffectsPack) -and !(Test-Path "$taskProfile/ezquake/ezquake.pk3")) {
    New-Item -ItemType HardLink -Path "$taskProfile/ezquake/ezquake.pk3" -Target $taskEffectsPack | Out-Null
}
try {
    # An assertion must fail automation rather than wait forever in a dialog.
    $env:SDL_ASSERT = 'abort'
    $taskProcess = Start-Process -FilePath $taskExe -WorkingDirectory $taskProfile -ArgumentList $taskArgs -WindowStyle Hidden -PassThru -RedirectStandardError "$taskProfile/stderr.log"
    # Cache the native handle before exit; Windows PowerShell 5 otherwise may
    # lose ExitCode after WaitForExit even though the child ended normally.
    $null = $taskProcess.Handle
} finally { $env:SDL_ASSERT = $taskAssert }
try {
    $taskDeadline=(Get-Date).AddSeconds($TimeoutSeconds)
    while(!$taskProcess.WaitForExit(1000) -and (Get-Date) -lt $taskDeadline) { }
    if (!$taskProcess.HasExited) {
        $taskProcess.Refresh()
        throw "Test timed out. PID=$($taskProcess.Id), window=$($taskProcess.MainWindowTitle), responding=$($taskProcess.Responding)"
    }
    $taskExit = $taskProcess.ExitCode
    if (!(Test-Path "$taskProfile/qw/qconsole.log")) { throw "Test exited $taskExit before creating a console log: $taskProfile" }
    $taskLog = Get-Content "$taskProfile/qw/qconsole.log" -Raw
    if ($taskExit -ne 0) { throw "Test exited $taskExit; see $taskProfile/qw/qconsole.log" }
    if ($taskLog -notmatch 'CODEX_SMOKE_COMPLETE') { throw 'Map test did not complete.' }
    if ($GameLibrary -and ($taskLog -notmatch 'LoadLibrary \(.*qwprogs\.dll\)' -or $taskLog -match 'Loading vm file')) {
        throw 'Test did not use the requested native game library exclusively.'
    }
    # KTX's inherited server.cfg mentions sv_enableprofile, which this client
    # does not implement. Enforce command checks on our own test sequence.
    $taskCommandsLog = $taskLog.Substring($taskLog.IndexOf('CODEX_MAP_SPAWNED'))
    # KTX can execute its packed server.cfg again after later map changes.
    if ($GameLibrary) { $taskCommandsLog = $taskCommandsLog.Replace('Unknown command "sv_enableprofile"','') }
    if ($taskCommandsLog -match 'Unknown command|Couldn.t load skybox') { throw 'Test commands/assets failed; inspect console log.' }
    if ($taskCommandsLog -match 'CV_PROFILE load [^\r\n]*: Invalid profile') { throw 'Visual profile was rejected; inspect console log.' }
    if ($taskLog -match 'Validation Error|VUID-|VK_ERROR_DEVICE_LOST|invalid push range|Callback registration failed') { throw 'Vulkan validation/runtime errors; inspect console log.' }
    Write-Output "PASS: $Label ($Renderer $Configuration), completion marker and normal exit 0. Profile: $taskProfile"
} finally {
    if (!$taskProcess.HasExited) { Stop-Process -Id $taskProcess.Id }
    $taskProcess.Dispose()
}
