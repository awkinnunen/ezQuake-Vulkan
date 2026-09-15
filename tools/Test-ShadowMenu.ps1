# SHADOW-001, OpenAI Codex, 2026-09-14. Real menu handlers and saved settings.
param([string]$Label='shadow-menu')
$ErrorActionPreference='Stop';$taskRoot=Split-Path $PSScriptRoot -Parent
$steps=@('developer 1','menu_visual_effects','r_dynamic 1','gl_flashblend 0','r_cv_shadows 0',
 'dev_competitive select r_cv_shadowquality','dev_competitive key RIGHTARROW','cv save disabled','screenshot',
 'dev_competitive select r_cv_shadows','dev_competitive key ENTER','cv save enabled')
$expect=@(@('disabled','r_cv_shadowquality',256),@('enabled','r_cv_shadows',1))
foreach($row in @(@('shadowquality',384),@('shadowsoft',1.25),@('shadowdistance',832),@('shadowlights',5),@('shadowbias',0.55),@('shadowupdates',5),@('shadowcasters',4160),@('maplights',1),@('maplightscale',1.25))){
 $steps+=@("dev_competitive select r_cv_$($row[0])",'dev_competitive key RIGHTARROW',"cv save $($row[0])",'dev_competitive checkpoint')
 $expect+=,@($row[0],"r_cv_$($row[0])",$row[1])
}
$steps+=@('dev_competitive select r_cv_shadowcache','dev_competitive key LEFTARROW','cv save cache','dev_competitive select r_cv_maplights','dev_competitive key RIGHTARROW','dev_competitive select r_cv_mapambient','dev_competitive key RIGHTARROW','cv save ambient','dev_competitive select r_cv_shadowstrength','dev_competitive key LEFTARROW','cv save strength',
 'dev_competitive select r_cv_shadowquality','dev_competitive mouse down 0.5','dev_competitive mouse move 2','dev_competitive mouse up 2','cv save maximum',
 'r_dynamic 0','dev_competitive select r_cv_shadows','dev_competitive key ENTER','cv save no_dynamic',
 'r_dynamic 1','gl_flashblend 1','dev_competitive select r_cv_shadows','dev_competitive key ENTER','cv save flashblend',
 'gl_flashblend 0','r_fullbright 1','dev_competitive select r_cv_shadows','dev_competitive key ENTER','cv save fullbright',
 'r_fullbright 0','dev_competitive select r_cv_shadows','screenshot','echo SHADOW_MENU_COMPLETE','dev_competitive checkpoint')
$expect+=@(@('cache','r_cv_shadowcache',0),@('ambient','r_cv_mapambient',0.15),@('strength','r_cv_shadowstrength',0.95),@('maximum','r_cv_shadowquality',512),@('no_dynamic','r_cv_shadows',1),@('flashblend','r_cv_shadows',1),@('fullbright','r_cv_shadows',1))
& "$PSScriptRoot/Test-VulkanRuntime.ps1" -Label $Label -Configuration Debug -BeforeMap @('sv_cheats 1') -Commands $steps
$profile="$taskRoot/cache/runtime-$Label";$log=Get-Content "$profile/qw/qconsole.log" -Raw
foreach($row in $expect){
 $saved=Get-Content "$profile/ezquake/competitive/$($row[0]).cfg" -Raw
 $m=[regex]::Match($saved,'(?m)^'+$row[1]+'\s+"?([\d.]+)')
 if(!$m.Success -or [math]::Abs([double]::Parse($m.Groups[1].Value,[cultureinfo]::InvariantCulture)-$row[2]) -gt .00001){throw "Menu value failed: $row"}
}
foreach($text in @('CV_HELP r_cv_shadowquality enabled=0','CV_HELP r_cv_shadows enabled=0','SHADOW_MENU_COMPLETE')){if(!$log.Contains($text)){throw "Missing menu evidence: $text"}}
Write-Output 'PASS: thirteen menu controls, keyboard/drag bounds, prerequisite lockouts and saved values.'

