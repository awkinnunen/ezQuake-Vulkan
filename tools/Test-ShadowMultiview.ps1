# BUG-TRIAGE-001, OpenAI Codex, 2026-09-14. Evidence collection, not a passing regression test.
param([string]$Label='shadow2-multiview', [ValidateSet('Debug','Release')][string]$Configuration='Release')
$ErrorActionPreference='Stop'
$taskRoot=Split-Path $PSScriptRoot -Parent
$steps=@('developer 1','cfg_save_unchanged 1','alias f_demostart ""','alias f_demoend ""',
 'vid_framebuffer 1','vid_framebuffer_multisample 4','vid_framebuffer_fxaa 5',
 'gl_outline 3','r_cv_enable 0','r_cv_ao 0.125','r_cv_aomode 1','r_cv_bloom 0.2','r_cv_hdr 1','r_dynamic 1','gl_flashblend 0','r_cv_shadows 1','r_cv_maplights 1','r_cv_shadowlights 4','vid_restart',
 'cfg_save triage','disconnect','playdemo motion','menu_main','togglemenu')
foreach($mode in @(0,2,4,0)) {
 $steps+=@("echo MULTIVIEW_STAGE $mode","cl_multiview $mode")+
  @(1..120|ForEach-Object{if($_%10 -eq 0){'dev_competitive checkpoint'};'wait'})+@('dev_competitive runtime','dev_competitive shadows','cl_multiview','screenshot')
}
$steps+=@('echo MULTIVIEW_STAGES_COMPLETE','disconnect')
$fixtureError=$null
try {
 & "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration $Configuration `
  -GameLibrary "${env:EZQUAKE_GAME_DIR}/qw/qwprogs.dll" -ExtraArguments @('-visual-tests') -TimeoutSeconds 90 -Commands $steps -PrepareProfile {
   param($path)
   Copy-Item "$taskRoot/cache/motion.mvd" "$path/qw/motion.mvd"
   '// Isolated fixture.' | Set-Content -Encoding ascii "$path/qw/server.cfg"
   New-Item -ItemType Directory -Force "$path/qw/skins" | Out-Null
   Get-ChildItem "${env:EZQUAKE_GAME_DIR}/qw/skins" -File | ForEach-Object {
    New-Item -ItemType HardLink -Path "$path/qw/skins/$($_.Name)" -Target $_.FullName | Out-Null
   }
  }
} catch { $fixtureError=$_.Exception.Message }
$profile=Join-Path $taskRoot "cache/runtime-$Label"
$log=Get-Content "$profile/qw/qconsole.log" -Raw
$stages=@([regex]::Matches($log,'(?s)MULTIVIEW_STAGE (\d)(.*?)(?=MULTIVIEW_STAGE |MULTIVIEW_STAGES_COMPLETE|\z)')|ForEach-Object{
 [ordered]@{mode=[int]$_.Groups[1].Value;vuidCount=[regex]::Matches($_.Groups[2].Value,'VUID-').Count;
 descriptorInvalidation=$_.Groups[2].Value.Contains('was destroyed or updated without UPDATE_AFTER_BIND');
 screenshots=[regex]::Matches($_.Groups[2].Value,'Wrote ezquake\d+\.png').Count}
})
$result=[ordered]@{author='OpenAI Codex';date='2026-09-14';configuration=$Configuration;
 exeSHA256=(Get-FileHash "$taskRoot/build-msvc-x64/$Configuration/ezquake.exe").Hash;
 demoSHA256=(Get-FileHash "$profile/qw/motion.mvd").Hash;fixtureError=$fixtureError;
 completed=$log.Contains('MULTIVIEW_STAGES_COMPLETE') -and $log.Contains('CODEX_SMOKE_COMPLETE');stages=$stages}
$result|ConvertTo-Json -Depth 5|Set-Content -Encoding utf8 "$profile/triage-results.json"
$result|ConvertTo-Json -Depth 5|Write-Output
if(!$result.completed -or $stages.Count -ne 4 -or ($fixtureError -and $fixtureError -notmatch '^Vulkan validation/runtime errors;')) {
 throw "Triage incomplete: $fixtureError"
}
# A captured VUID remains a renderer failure even though evidence collection completed.

if($fixtureError -or ($stages|Where-Object{$_.vuidCount -gt 0})){throw "Multiview regression: $fixtureError"}
if($log -notmatch "view=3.*views=[1-8],[1-8],[1-8],[1-8]"){throw "Four shadowed views missing"}
Write-Output "PASS: 0/2/4/0 views with HDR, MSAA, SSAO, bloom and map shadows; no validation errors."
