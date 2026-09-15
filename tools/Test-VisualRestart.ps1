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
 'menu_visual_effects','dev_competitive select r_cv_hdr','r_cv_hdr 0','dev_competitive key F5','wait','dev_competitive video',
 'echo HDR_PENDING','r_cv_hdr 1','dev_competitive video','screenshot',
 'echo HDR_REVERT','r_cv_hdr 0','dev_competitive video',
 'echo HDR_APPLY','r_cv_hdr 1','dev_competitive key F5','wait','wait','dev_competitive video','screenshot',
 'echo HDR_DISABLE','r_cv_hdr 0','dev_competitive key F5','wait','wait','dev_competitive video',
 'echo RESTART_COMPLETE','dev_competitive checkpoint')
# MAINT-002: deliberately capture immediately after recreation, without settling waits.
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration -TimeoutSeconds 180 -Commands $steps
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
foreach($pair in @(@('HDR_PENDING','pending=1.*?CV_HDR requested=1 applied=0 active=0'),
 @('HDR_REVERT','pending=0.*?CV_HDR requested=0 applied=0 active=0'),
 @('HDR_APPLY','pending=0.*?CV_HDR requested=1 applied=1 active=1'),
 @('HDR_DISABLE','pending=0.*?CV_HDR requested=0 applied=0 active=0'))) {
 if($log -notmatch ('(?s)'+$pair[0]+'.*?CV_VIDEO '+$pair[1])) {throw "Missing HDR restart state: $($pair[0])"}
}
Write-Output 'PASS: pending change, reverting to applied value, F5 video restart, actual 4x samples, and hardware-clamped request clear correctly.'
Write-Output 'PASS: HDR pending/revert/F5 enable/F5 disable feedback matches requested and active state.'
