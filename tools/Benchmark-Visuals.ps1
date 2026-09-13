# MAINT-004, OpenAI Codex. Identical private MVD; warm-up plus two measurements.
param([string]$Label='visual-cost', [switch]$VSyncOnly, [switch]$BaselineOnly)
$ErrorActionPreference='Stop'
$taskRoot=Split-Path $PSScriptRoot -Parent
$cases=@(
 @{name='defaults';commands=@()},
 @{name='no-ao';commands=@('r_cv_ao 0')},
 @{name='no-outlines';commands=@('gl_outline 0')},
 @{name='no-msaa';commands=@('vid_framebuffer_multisample 0')},
 @{name='no-fxaa';commands=@('vid_framebuffer_fxaa 0')},
 @{name='no-bloom';commands=@('r_cv_bloom 0')},
 @{name='no-detail';commands=@('gl_detail 0')},
 @{name='minimal';commands=@('r_cv_enable 0','r_cv_ao 0','gl_outline 0','vid_framebuffer_multisample 0','vid_framebuffer_fxaa 0','r_cv_bloom 0','gl_detail 0','gl_caustics 0','r_shadows 0')}
)
if($VSyncOnly){$cases=@(@{name='vsync';commands=@('vid_vsync 1')})}
if($BaselineOnly){$cases=@(@{name='defaults';commands=@()})}
$results=@()
foreach($case in $cases) {
 $caseLabel="$Label-$($case.name)"
 & "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $caseLabel -Configuration Release -NoValidation -Width 1280 -Height 720 -TimeoutSeconds 180 -PrepareProfile {
  param($path)
  Copy-Item "$taskRoot/cache/motion.mvd" "$path/qw/motion.mvd"
  New-Item -ItemType Directory -Force "$path/ezquake/competitive","$path/qw/skins" | Out-Null
  Copy-Item "$taskRoot/profiles/ezquake/competitive/project-default.cfg" "$path/ezquake/competitive/project-default.cfg"
  New-Item -ItemType HardLink -Path "$path/ezquake/ezquake.pk3" -Target "${env:EZQUAKE_GAME_DIR}/ezquake/ezquake.pk3" | Out-Null
  Get-ChildItem "${env:EZQUAKE_GAME_DIR}/qw/skins" -File | ForEach-Object {New-Item -ItemType HardLink -Path "$path/qw/skins/$($_.Name)" -Target $_.FullName | Out-Null}
  @('alias f_spawn ""','echo CODEX_MAP_SPAWNED','menu_ingame 0','menu_main','togglemenu','developer 0','cl_maxfps 1000',
    'cv load project-default')+$case.commands+@('vid_restart','vid_gfxinfo','exec bench1.cfg') | Set-Content -Encoding ascii "$path/qw/finish.cfg"
  for($round=1;$round -le 3;$round++) {
   $next=$round+1
   @("echo BENCH_ROUND_$round", "alias f_demoend $([char]34)exec bench$next.cfg$([char]34)",'timedemo motion') | Set-Content -Encoding ascii "$path/qw/bench$round.cfg"
  }
  @('alias f_demoend ""','echo CODEX_SMOKE_COMPLETE','quit') | Set-Content -Encoding ascii "$path/qw/bench4.cfg"
 }
 $log=Get-Content "$taskRoot/cache/runtime-$caseLabel/qw/qconsole.log" -Raw
 $samples=[regex]::Matches($log,'(\d+) frames\s+([\d.]+) seconds\s+([\d.]+) fps[\s\S]*?avg frametime ([\d.]+)ms, std dev ([\d.]+)ms')
 if($samples.Count -ne 3){throw "Expected three timedemos for $caseLabel; got $($samples.Count)"}
 $rows=@($samples | ForEach-Object { [ordered]@{frames=[int]$_.Groups[1].Value;fps=[double]::Parse($_.Groups[3].Value,[cultureinfo]::InvariantCulture);averageMs=[double]::Parse($_.Groups[4].Value,[cultureinfo]::InvariantCulture);stddevMs=[double]::Parse($_.Groups[5].Value,[cultureinfo]::InvariantCulture)} })
 $results+= [ordered]@{case=$case.name;overrides=$case.commands;warmup=$rows[0];measured=@($rows[1],$rows[2]);meanMs=($rows[1].averageMs+$rows[2].averageMs)/2}
 $results | ConvertTo-Json -Depth 8 | Set-Content -Encoding utf8 "$taskRoot/cache/$Label-results.json"
 Write-Output "$($case.name): $($results[-1].meanMs) ms"
}
$counts=@($results | ForEach-Object {$_.measured | ForEach-Object {$_.frames}} | Select-Object -Unique)
if($counts.Count -ne 1){throw "Non-comparable frame counts: $counts"}
Write-Output "PASS: timedemo completion, identical frame count $counts, one discarded warm-up and two samples per case."
