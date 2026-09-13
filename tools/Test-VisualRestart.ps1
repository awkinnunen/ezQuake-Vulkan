# CV-003 applied/requested MSAA and visible restart feedback. OpenAI Codex, 2026-09-13.
param([string]$Label='visual-restart',[ValidateSet('Debug','Release')][string]$Configuration='Debug')
$ErrorActionPreference='Stop'
$taskRoot=Split-Path $PSScriptRoot -Parent
$steps=@('developer 1','cl_bonusflash 0','menu_visual_effects','dev_competitive select vid_framebuffer_multisample',
 'vid_framebuffer_multisample 0','dev_competitive key F5','wait','wait','dev_competitive params',
 'vid_framebuffer_multisample 4','dev_competitive params','wait','screenshot',
 'vid_framebuffer_multisample 0','dev_competitive params',
 'vid_framebuffer_multisample 4','dev_competitive key F5','screenshot','wait','wait','dev_competitive params','screenshot',
 'dev_competitive checkpoint','vid_framebuffer_multisample 64','dev_competitive params',
 'dev_competitive key F5','wait','wait','dev_competitive params','screenshot',
 'echo RESTART_COMPLETE','dev_competitive checkpoint')
# MAINT-002: deliberately capture immediately after recreation, without settling waits.
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -Commands $steps
$log=Get-Content "$taskRoot/cache/runtime-$Label/qw/qconsole.log" -Raw
$states=[regex]::Matches($log,'CV_VIDEO pending=(\d+) requested=([\d.]+) applied=(\d+) actual=(\d+)')
foreach($required in @('pending=0 requested=0 applied=0 actual=1','pending=1 requested=4 applied=0 actual=1',
 'pending=0 requested=4 applied=4 actual=4','pending=1 requested=64 applied=4 actual=4')) {
 if($log -notmatch [regex]::Escape($required)) {throw "Missing restart state: $required"}
}
$last=$states[$states.Count-1]
if($last.Groups[1].Value -ne '0' -or $last.Groups[2].Value -ne '64' -or $last.Groups[3].Value -ne '64' -or [int]$last.Groups[4].Value -lt 1) {
 throw 'Hardware sample clamping left a false restart pending state'
}
if($log -notmatch 'RESTART_COMPLETE') {throw 'Restart test incomplete'}
Write-Output 'PASS: pending change, reverting to applied value, F5 video restart, actual 4x samples, and hardware-clamped request clear correctly.'
