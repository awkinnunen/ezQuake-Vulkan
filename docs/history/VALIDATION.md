# Development environment validation — 2026-09-13

## MAINT-001..004 / PUBLIC-001 - Final maintenance evidence (2026-09-13)

- MSVC x64 Debug and Release rebuilt successfully after the screenshot lifecycle
  change and header fix. Final binaries are recorded in build-artifacts.json.
- `screenshot-lifecycle-debug` and `screenshot-lifecycle-release`: immediate F5
  recreation capture, pending/reverted MSAA, 4x activation and hardware-clamped 64x
  request all pass, normal exit 0, no Vulkan validation errors. Each produced five
  nonblank 800x600 images (over 7,000 unique colors each). Earlier workaround waits
  have been removed from Test-VisualRestart.ps1.
- `motion-debug-3`: E1M1, DM4 and DM6, six-player demo, forward/backward seek,
  skin reload, video restart and paused-time stability pass with validation.
- `motion-arena-final`: map picker, actual KTX match modes/rules, moving bots,
  join/leave, stop/restart and single-player isolation pass. The installed map set
  now contains 123 valid choices; the older fixture expected 31.
- `visual-cost-2`, `visual-cost-closing`, `timedemo-vsync-2`: fixed 1,290-frame
  Release timedemos complete with one warm-up and two measured runs per case.
  All eight effect cases have equal frame counts. VSync uses FIFO and completes
  three runs; the historical hang was not reproduced on this GPU. PERFORMANCE.md
  gives the samples and limits. No approved default was changed.
- `public-profiles-final`: the cleaned full local config and portable WASD/defaults
  overlay load through the real parser; W/A/S/D and dynamic Caps Lock bindings
  are queried, all 53 graphics values are saved unchanged, crosshair size is 2.5,
  and pickup switches are 8/8. Missing private images/skins in the isolated fixture
  produce inherited startup fallback messages; the exported crosshairs are built-in.
  The 55 unsupported cvar warnings are gone. No communication binding is invoked.
- Thirteen ordered patches replay exactly to all 61 modified/new engine source
  files; attribution covers every path and git diff --check passes.

Initial failed harness attempts are retained locally. The motion wrapper first
rejected KTX's repeated `sv_enableprofile` compatibility message; only that exact
known warning is exempted for native KTX tests. The first benchmark slice lacked
EndOfDemo, causing playback to wait indefinitely; the fixture generator now writes
the standard marker. These failures do not justify claiming renderer/VSync fixes.

Limits: one Windows/AMD GPU and bounded sessions; no new OpenGL parity work,
no complete campaign playthrough, no RTX integration, no long-session leak result,
and no dedicated automatic-match-screenshot/movie-capture regression in this pass.

Recorded by OpenAI Codex from local tool results. Upstream diary test claims are
not included as local passes. Authorship: see ATTRIBUTION.md.

## Initial environment checks (before LOCAL-007 source changes)

| Check | Result |
|---|---|
| Source clones and pinned submodules | Downloaded; versions in sources.lock.json |
| MSVC x64, Windows SDK, CMake and Ninja | Available; exercised by builds |
| Tibazera SDL3/Vulkan ezQuake Debug x64 | Build passed without source changes; tibazera-build.log; Vulkan enabled |
| Tibazera SDL3/Vulkan ezQuake Release x64 | Build passed, exit 0; /O2 /Ob2 /DNDEBUG; tibazera-release-build.log |
| Vulkan SDK | 1.4.357.0 |
| Original ezQuake Debug x64, both OpenGL renderers | Build passed without source changes |
| Vulkan probe build/run | Passed |
| vkQuake Debug x64 | Build passed, 0 warnings and 0 errors |
| RTGL1 Debug x64, Win32, DLSS disabled | Build passed after local build compatibility fix |
| vkquake-rt Debug x64 | Build passed, 0 warnings and 0 errors |
| RT shaders | 51/51 compiled for Vulkan 1.2 |
| RT shader spirv-val | 51/51 passed |
| PowerShell syntax and workspace JSON | Parsed successfully |

## Device probe

```text
Device: AMD Radeon(TM) Graphics; Vulkan 1.2.188
RT extension advertisement: pipeline=no, acceleration structure=no
Vulkan loader/device probe passed.
```

The probe creates a Vulkan instance and enumerates physical devices. It does not
test window presentation, rendering, every Vulkan feature or RT. RT runtime testing
was not performed because the driver does not advertise the required extensions.

## Outputs

- ezquake-vulkan/build-msvc-x64/Debug/ezquake.exe and debugger PDB
- ezquake-vulkan/build-msvc-x64/Release/ezquake.exe
- ezquake/build-msvc-x64/Debug/ezquake.exe and debugger PDB
- build/vulkan-probe/vulkan-probe.exe
- vkquake/Windows/VisualStudio/Build-vkQuake/x64/Debug/vkQuake.exe
- rtgl1/Build/x64-Debug/RayTracedGL1.dll and import library
- vkquake-rt/Windows/VisualStudio/Build-vkQuake/x64/Debug/vkQuake.exe
- build/rt-shaders/*.spv

Binary sizes and SHA-256 hashes are in build-artifacts.json. The comparison builds
were validated in Debug; the active Vulkan fork also built successfully in Release.
The initial build-only validation preceded game-data installation.
LOCAL-005 subsequently installed nQuake and exercised direct Debug launches:
see NQUAKE.md for the precise results and limitations. No IDE debugger or full
visual parity test has been completed. The original ezQuake build has upstream warnings,
including C4090 const qualification in gl_drawcall_wrappers.c; these did not stop
the build and were not changed here.

Local RT fixes are documented in ATTRIBUTION.md and exported under patches/.
The Release fullscreen launch command and script syntax were checked; fullscreen
Release gameplay has not been run as part of LOCAL-006.
They remain uncommitted. Visual effects of shader changes require a later RT run.
The initial documentation-only pass did not change engine sources; LOCAL-007
subsequently changed and rebuilt the active Vulkan fork as recorded below.


## LOCAL-007: final source-change validation

All four requested steps passed sequential gates before proceeding. Final binaries
include CODE-005..010. The full-screen Release launcher uses the rebuilt executable;
Release automation was windowed. The user had separately confirmed the previous
full-screen build worked. No claim of new full-screen automation is made.

| Check | Result / evidence |
|---|---|
| Final Debug and Release builds | Passed; cache/step4-build.log, cache/final-release-build.log |
| Push ranges / descriptor counts | Boundary acceptance/rejection, alignment, zero sizes/stages and overflow checked using production predicates |
| C vs actual SPIR-V layouts | 13 blocks passed; all members of world, alias and sprite layouts |
| Shader validation | All 22 engine SPIR-V modules passed spirv-val for Vulkan 1.2 |
| NPOT/POT mipmaps | 1,094 sizes, full per-level dimensions/offsets/pixel comparison; guard-page protected input/output buffers |
| Sky UV helper on GPU | 42 cases: 256/512/1024, NPOT rectangle, tiny and 1xN axes |
| Alias lighting on GPU vs existing GLM | 511 cases; all 162 MDL normals at 3 yaw angles, sentinel/saturation, special-pass bypass, alpha unchanged |
| Debug Vulkan E1M1 | Passed after each step; cache/step1-verified-runtime.log and cache/step2-runtime.log through cache/step4-runtime.log |
| Final Release Vulkan + mixed-size sky | Passed, exit 0; cache/final-release-runtime.log |
| Same-fork Modern OpenGL Debug | Passed, exit 0; cache/final-opengl-runtime.log |
| Vulkan vid_restart in map | Passed, exit 0; cache/final-restart-runtime.log |
| QTV shutdown with active request | Loopback response held until shutdown; duplicate update rejected, worker joined, exit 0; cache/qtv-shutdown-test.log |
| Ordered source patch replay | All 31 changed/new source files reconstructed exactly from pinned base; attribution coverage and git diff --check passed; cache/patch-verification.log |

GPU: AMD Radeon(TM) Graphics, Vulkan 1.2.188. The runtime reports **128** maximum
push-constant bytes and 32 descriptor sets. Both fixed bindless sampler arrays
(16,384 combined descriptors total) fit the reported update-after-bind limits.
The previous 176/160-byte world/sprite layouts exceeded this device's real limit.
The corrected blocks require 128/68 bytes; alias remains 128. Unsupported bindless
devices receive a clear initialization error; a bounded alias fallback remains open.

Test command: scripts/Test-Vulkan.ps1. Full detailed results: cache/step4-tests.log.
The GPU compute tests include production GLSL, and the alias reference is extracted
from the unmodified existing GLM shader. They establish formula equivalence, not
pixel-perfect equivalence of the complete rendering pipelines.

Runtime test command: scripts/Test-VulkanRuntime.ps1 with a new -Label each time.
-TestSky generates test-only PNG faces inside that run's cache profile. Tests use
separate configs, installed asset hard links and the actual build executable;
SDL assertions fail the run rather than being ignored. Validation errors and
unknown test commands fail the gate. KTX's inherited sv_enableprofile warning
occurs before the test sequence and is recorded, not treated as our command.
The first stricter step-3 gate rejected that inherited warning; its captured
exit-0 run was rechecked after narrowing command checks to the test sequence.
The final Debug/Release mixed-sky tests passed the corrected gate directly.

Screenshots now contain the rendered map and weapon; representative evidence is
cache/runtime-final-scene-vulkan/qw/ezquake000.png and
cache/runtime-final-scene-opengl/qw/ezquake000.png. Both final captures were
inspected with the startup menu closed; normal HUD/welcome text remains. Camera
positions were not locked, so these are smoke checks, not pixel comparisons.
Exhaustive sky seam inspection, MD3/shell/outline scene comparisons, competitive
ruleset/network testing, timedemo performance and other GPUs remain unverified.
No RT feature or Vulkan API-version upgrade was added.
# LOCAL-008: KTX master and QuakeC gameplay corrections

2026-09-13, executed by OpenAI Codex. KTX base: 631584f7fac3c3891826224d36d873c99880d353
(master 1.48-dev), local CODE-011/012. QuakeC reference: 634eefab09a77eb7b5f5ca7078ba3d8784a91142.

- Debug and Release native x64 API-16 DLL builds passed. Logs:
  `cache/ktx-final-debug-build.log`, `cache/ktx-final-release-build.log`.
  The upstream build emits existing MSVC warnings; warning-free compilation is
  not claimed. The focused regression executable builds with /W3 /WX.
- `cache/ktx-final-regressions.log`: all three regression groups reproduced in
  pristine KTX HEAD, then 19 production-function scenarios passed with the fixes.
  Eight whole functions including the typed wrapper compile against real KTX
  headers with recording stubs for engine services. This tests actual dispatch,
  callback initialization/rearming and sound-channel behavior, not copied formulas.
- `cache/ktx-final-debug-runtime.log`, `cache/ktx-final-release-runtime.log`,
  `cache/ktx-final-opengl-runtime.log`: E1M1 spawned, screenshot written, clean exit 0.
  Logs require `LoadLibrary (...qwprogs.dll)` and reject QVM fallback.
  Vulkan validation/runtime checks passed. Each run used an isolated profile;
  no normal nQuake configuration was loaded or overwritten.
- The source patch is replayed against pinned Git HEAD and compared with both
  modified files, normalizing line endings. Source-manifest coverage is checked;
  the rerelease reference remains unmodified. See `cache/ktx-patch-verification.log`.
- Installed Release: `cache/ktx-installed-retry-runtime.log` passed Vulkan E1M1,
  explicit native-module loading, screenshot, validation checks and exit 0. The
  test hard-links the actual installed DLL into an isolated profile. The first
  installed run (`cache/ktx-installed-runtime.log`) stopped in an Error window
  before a qconsole.log was created and was terminated by the 45-second fixture
  timeout. A fresh-profile retry passed with the same KTX DLL. The cause
  of that early startup failure is unresolved; it is not counted as a pass.

Concurrent workspace activity: a separate configuration-browser change rebuilt
the ezQuake Debug/Release executables and edited Run.ps1 while this pass finished.
Those files were not changed by the KTX work. Workspace-wide hashes are therefore
time-specific snapshots, not a frozen validation claim for that other change.
`provenance/ktx-gameplay.json` separately records the delivered KTX files, module
hashes and this pass's evidence logs. No comprehensive retest of the concurrent
configuration-browser work is claimed here.

The KTX upgrade from packaged 1.46-dev to master 1.48-dev was explicitly requested.
It includes upstream behavior changes beyond the three local adaptations. These
tests do not prove every KTX mode/bot/network/demo path, full campaign compatibility,
or visual/audio behavior in every map. Unit sound checks establish channel
coexistence, not subjective listening. Rerelease-only assets/services were not
imported. See QUAKEC-MERGE.md for the scope and rollback instructions.

## CFG-001 config browser validation

The cfg binding preview/load feature passed its production parser edge cases
and 1,000 randomized sequences, Vulkan Debug/Release, Modern OpenGL Debug, and
an empty-directory test. Isolated runtime tests verify non-executing preview,
exact-path load, selection, keyboard hit testing, modifier binds, screenshots
and normal exit. See CONFIG-BROWSER.md for detailed coverage, evidence paths,
failed exploratory attempts and layout/unsupported-script limitations.

## SP-001 — Shareware single player (2026-09-13)

Implemented/tested by OpenAI Codex under user direction. Debug and Release
builds succeeded with the existing offline dependency cache. Evidence:

- cache/runtime-sp-before/qw/qconsole.log reproduces the old incorrect KTX
  startup and refusal to save a multiplayer game despite requested SP settings.
- cache/runtime-sp-debug-final: Vulkan Debug, fresh newgame, start and E1M1–E1M8,
  original NQ interpreter on all ten loads (including restored E1M1), nine saves
  with player/monster entities, restored player health/items/ammo/weapon, exit 0.
- cache/runtime-sp-release-final: the same checks on Vulkan Release, preceded
  by an actual native KTX game and then newgame. Campaign settings are retained
  after that transition. No server.cfg, QVM or native KTX loads during the SP
  portion, and no Vulkan validation errors or unknown test commands.
- The saved E1M1 load-menu screenshot visibly lists The Slipgate Complex with
  0/23 kills. Test saves remain under each isolated profile's qw/save directory
  with the default home-save option and -nohome. Installed user configs untouched.
- cache/sp-patch-verification.log verifies all six patches against the 47-file
  export snapshot and verifies that all SP-001 hunks remain in the working tree.
  Concurrent multiplayer-menu changes appeared after that snapshot; they are
  outside SP-001 and prevent treating the whole live tree as this exact snapshot.

The initial SP test found the drive-root save-path bug and the ignored type-0
interpreter selection. A subsequent Debug run ended before the completion
marker; another hit the inherited two-second command-buffer runaway guard.
The fixture now uses twenty-frame waits, checks for the guard, and passed on
Debug and Release. Only successful final runs are claimed as validation.

These checks exercise actual game logic/spawns and save restoration, but map
advances are driven by console map commands. No complete playthrough, real exit
trigger traversal, difficulty-hall selection, secret hunting, death/restart or
boss completion is claimed. Save compatibility with other engines is untested.

## ARENA-001 — Local Arena validation

Writer: OpenAI Codex, 2026-09-13. Debug and Release builds succeeded with the
existing MSVC/dependency configuration. The fixture uses production menu key
handlers, actual native KTX and isolated copies of the profile with hard-linked
archives. It starts interaction after engine initialization.

- `cache/runtime-arena-final-release`: Vulkan Release passed main-menu entry,
  map picker and token filtering, FFA/duel/2on2/Clan Arena, actual bot counts
  0/2/1/0/2, skill 6, countdown to active CA, changed bot coordinates, stop,
  blocked bot actions in SP and restart on DM6, screenshots and normal exit 0.
- `cache/runtime-arena-debug-verified`: the final Vulkan Debug binary passed
  the same sequence, including the final map preflight and message wrapping.
- The main-menu screenshot shows Local Arena in the same Quake font as the
  other entries. The control screenshot shows the real map/mode/bot count.
- Seven ordered patches replay exactly to all 50 modified/new source files;
  the attribution manifest covers every path and git diff --check passes.

Only these named successful runs are evidence. Early scripts executed during
Host_Init (which flushes waits), then hit the inherited command-buffer runaway
guard after moving to real frames. The first CA check was insufficient: the
command was corrected from ca to carena and tests now assert k_clan_arena and
actual match status, plus bot movement. A debug status-string fixture bug was
fixed by copying Info_ValueForKey results. The first final Debug attempt reached
the last map load but exceeded its 55-second budget; the verified run uses a
90-second overall budget with one-second waits and owned-process cleanup.

The forced pre-frame startup screenshots in runtime-arena-first/second emitted
a Vulkan image-layout diagnostic. This path remains a follow-up item rather
than being counted as clean validation. The initialized final Vulkan runs have
no validation errors. Existing KTX/nQuake diagnostics (including the mismatched
DM6 .lit asset and inherited configuration notices) are not suppressed or fixed
by this menu change. Full campaign traversal, all bot maps, remote-server/LAN
sessions, and competitive correctness of every KTX mode are not claimed.

Final OpenGL evidence: `cache/runtime-arena-opengl-verified` passed the same
complete fixture on the final Release binary with Modern OpenGL, including
active Clan Arena, bot movement, SP isolation and server restart/stop. Exit 0.

## ARENA-002 validation (2026-09-13)

OpenAI Codex: Debug and Release build successfully. The expanded Vulkan Release
fixture in cache/runtime-arena-dmm-release passes automatic FFA deathmatch 3,
manual dmm1/dmm3, Clan Arena dmm5, all mode/bot/movement/stop/SP/restart checks
and exit 0, with no Vulkan validation errors or invalid test commands. The
Deathmatch row and compact 12-row layout were visually inspected.

The installed startup config was checked byte-for-byte against the pre-change
backup: only cl_onload changed from sb_refresh to menu. Its routing to the main
menu was inspected in Startup_Place; no separate cold-start screenshot is claimed.
Runtime/arena-startup-setting.json records only the setting delta and hashes.
Eight patches replay exactly to all 51 changed/new source paths; attribution
coverage and git diff --check pass (cache/arena-dmm-patch-verification.log).

## DATA-001 — Imported Quake resources

OpenAI Codex, 2026-09-13. scripts/Test-ImportedQuakeData.ps1, Vulkan Release,
isolated profile cache/runtime-imported-quake-release-2: PASS, normal exit 0.
All 38 maps loaded and saved: START, 30 episode maps, END, DM1–DM6. Saved player
entities and campaign monster entities were checked. Four screenshots were
captured; the E4M1 image was inspected. No Vulkan validation errors, missing-map
failures, interpreter errors or unexpected multiplayer module/config startup.
This validates loading/spawning/saving, not completion of every level or vanilla
identity of the repacked assets. The first attempt exceeded its 55-second test
harness limit after the campaign maps; the complete rerun used a 120-second
harness deadline. The binary snapshot's SHA-256 is in the profile's result.json.

Preservation: id1/pak0.pak, qw/textures.pk3, qw/models.pk3, qw/nquake.pk3,
ezquake/ezquake.pk3, qw/ktx.pk3 and qw/scoreboard_flags.pk3 all match their
nquake-install.json SHA-256 values. All 12 entries of the imported pickup backup
match shareware pak0 entries byte-for-byte. Import paths/hashes and the reversible
gpl_maps.pk3 rename are in private-game-data-import.json.

## CV-001 — Competitive Visuals (2026-09-13)

OpenAI Codex validation on the local AMD Radeon integrated GPU and Windows x64.

- Debug and optimized Release builds passed. Final source has 53 visual controls.
- `cache/runtime-cv-full-debug2`: Debug six-page/profile integration passed with
  MSAA changes, AO/outlines, postprocess and vid_restart, normal exit and no VUIDs.
- `cache/runtime-cv-final-release2`: final Release passed the expanded fixture,
  including the MSAA menu entry and rim vertex-work bypass. Six pages, page reset,
  named roundtrip, malformed/empty-profile rejection, comparison preservation,
  AO/outlines/FXAA/MSAA, video restart and screenshots passed without VUIDs.
- `cache/runtime-cv-opengl`: final Release Modern OpenGL map/menu smoke passed;
  new shader style stays inactive under OpenGL while the menu remains accessible.
- Existing regression suite passed: 13 C/push-constant layouts, 22 SPIR-V modules,
  1,094 guarded mip cases, 42 skybox GPU cases and 511 alias-lighting GPU comparisons.
- `probes/competitive-regressions.py`: 32 production CPU gate cases (all combinations
  of EF_RED/GREEN/BLUE, nonplayer, weapon and ruleset restrictions), seven C/GLSL
  224-byte UBO layouts and 522 actual GPU response results passed. Cel plateau
  comparisons allow 1e-6 arithmetic tolerance; observed roundoff was below 1.8e-7.
- Inspected E1M1 scene and menu captures. Scene effects precede HUD drawing; opaque
  HUD imagery is not input to the new bloom/tone/sharpen pass. Transparent HUD
  edges can naturally change with the scene beneath them. Pixel-perfect whole-HUD
  equivalence and recorded third-person powerup comparisons are not claimed.

Failures retained as evidence: `runtime-cv-first` exposed the next-frame offscreen
layout mismatch; `runtime-cv-full-debug` exposed differing external dependencies
between HUD/composite render passes. Both code defects were corrected and the
complete succeeding fixtures above contain no Vulkan validation errors. One
launch at `runtime-cv-final-release` exited before creating qconsole.log, with
empty stderr; its cause is not established. The unchanged final executable then
passed `runtime-cv-final-release2`. The harness now reports the exit code explicitly
if a future launch fails before log creation.

Full floating-point HDR, emissive-only bloom, dynamic shadow maps and temporal
AA remain unimplemented. AO here is world contact shading, and the shadow control
uses existing projected model shadows. No frame-time improvement, tournament
acceptance, long-session stability, recorded multiview or all-map visual parity
is inferred from these tests. See COMPETITIVE-VISUALS.md for precise controls and
remaining acceptance work.

## INPUT-001 — Legacy controls and NQ weapon event

OpenAI Codex, 2026-09-13. Debug and Release incremental builds succeeded without
new warnings after declaring CL_SetStat in cl_nqdemo.c. Actual Vulkan runtime:
cache/runtime-legacy-controls-5 (Release SP), runtime-legacy-controls-final-debug
(Debug SP), runtime-legacy-controls-final-qw (Release native KTX/QW), all PASS and
normal exit 0. Seven real weapon changes selected all five copied images. LG
hold/release, optional shotgun return, overlapping forward/back holds and pitch
restoration passed. No Vulkan errors or import command/image-load errors.
The RL screenshot was visually inspected. No messages were sent to other players.
The inherited startup-only KTX sv_enableprofile unknown command is explicitly
excluded by the fixture; other unknown commands still fail it.

probes/legacy-controls-verify.py: PASS, 181 nQuake aliases unchanged, six help
labels adjusted, movement/firing/communication bindings saved as expected.
The SP before/after saves show rockets 99 -> 98, upward velocity 606.798828 and
released attack/jump buttons. This is a real rocket jump, not only alias parsing.
The legacy frame-based timing has not been tuned or validated across all FPS,
latency, terrain and server/ruleset combinations. Nonstandard NQ weapon encoding
is preserved in code but was not separately played through in this test.

Early fixture attempts used ineffective server-side give commands with NQ's
interpreter, then switched to the original game's impulse 9 for fixture inventory.
A long prequeued script delayed f_weaponchange callbacks; final test stages are
chained by the real event and let the command queue drain. Failed fixture logs
remain in cache. The NQ direct-stat bypass was independently identified in source
and repaired through the existing shared stat handler.

Patch/provenance: patches/ezquake-10-nq-weapon-trigger.patch after patch 09.
Verify-StepPatches.py passed all 10 patches, exact replay to 60 modified/new source
files, complete source-path attribution and git diff --check.

## CV-TUNE-001 — Configured Vivid profile and input defaults

OpenAI Codex, 2026-09-13. PASS: isolated real Vulkan Release run at
cache/runtime-vivid-defaults, map spawn, named visual-only profile load/save,
60 frames and normal exit 0 without validation/runtime errors. Queried live
values: crosshairsize 2, w_switch 8, b_switch 8, r_cv_rim 0.18, r_cv_bloom 0.1.
The screenshot was inspected for stronger styling and reduced crosshair size.
Both autoswitch settings match the engine's existing Gun Autoswitch toggle;
this run verifies configuration, not a physical weapon/backpack pickup trial.
A separate line comparison verified all non-target configuration lines,
including movement/firing/communication aliases and bindings, were preserved.
No source rebuild was required because only existing settings were tuned.

## CV-002 — Interactive menu controls

OpenAI Codex, 2026-09-13. Debug and Release builds passed after the final
model-shadow switch correction. Test-CompetitiveWidgets.ps1, label
widgets-final-release: PASS, all 53 managed settings covered (34 sliders, nine
switches, ten selectors), scaled/unscaled mouse capture, outside-track clamping,
release, keyboard increments and On/Off reversal. The allowlist is compared with
saved profile keys to detect omitted controls. All six menu pages and longer
page tails were rendered; layout screenshots were inspected.

Test-CompetitiveVisuals.ps1, label widgets-final-debug: PASS, six pages, profile
roundtrip, transactional malformed/empty rejection, compare restoration, reset,
postprocess/HUD, MSAA and video restart. Both isolated Vulkan runs exited 0
without validation/runtime errors. Widget tests use the production CV_Mouse and
CV_Key handlers through the developer driver, not physical desktop input.
Earlier widget fixture runs exceeded the 31-character profile-name limit; compact
fixture names corrected that issue. Failed fixture logs remain in cache.

A source audit confirmed r_shadows.integer is the admission gate and the Vulkan
shadow pass uses fixed opacity. The menu is now On/Off; the intended Vivid/legacy
shadow value is 1. The active config's existing Off is preserved. These tests
verify control behavior, not exhaustive screenshots of every inherited effect.

Crosshair edits preserve every unrelated byte in the four configs (including
both size definitions in legacy-esdf.cfg). Subsequent legacy/Vivid shadow edits
are separately recorded with before/after hashes. The original user/nQuake
images and other assets are unchanged. Eleven ordered patches replay exactly to
all 60 modified/new source files; attribution coverage and git diff --check pass.

## CV-003 - Grouped menus, dependencies, restart feedback and pixel effects

2026-09-13. OpenAI Codex under user direction. Debug and Release builds succeed.
The 224-byte uniform ABI and rendering formulas are unchanged; general effects
are now independent of the competitive master switch.

Test-VisualImpact.ps1, impact-final-debug: PASS for all 48 visible settings.
Each setting uses four real 800 x 600 screenshots: A, repeat A, B, restored A.
All three A images match exactly, and B changes pixels for every setting.
192 comparison images plus the runtime smoke screenshot; normal exit 0 and no
Vulkan validation/runtime errors. The fixture freezes a local cheat-enabled
E1M1 server and camera; original model skins, a water camera and nQuake detail/
caustic assets cover conditional paths. Full per-setting values, pixel counts
and limits: VISUAL-TEST-RESULTS.md. Runtime command specs and images stay in cache.
Release evidence is also complete across impact-all-3 (36 world/general rows)
and impact-actors-4 (12 model/MSAA rows). This proves effects in eligible scenes;
it is not an exhaustive audit of inherited Graphics/High presets, motion quality,
frame-time costs, every GPU, every server ruleset, or ray tracing.

Test-VisualDependencies.ps1, groups-verified-debug2: PASS for 25 prerequisite
gates blocking both keyboard and mouse, grey-state reasons, menu separation,
competitive presets preserving general settings, independent general/AO-only
parameters, and exact numerical restoration of all 53 user profile values.
The float-boundary roundtrip found and fixed decimal 0.2 being rejected against
a float lower bound. Earlier OpenGL groups-opengl run also passed its four
Vulkan-only control gates and neutral renderer parameters.

Test-CompetitiveWidgets.ps1, widgets-final-cv003: PASS for 34 sliders, 8 switches,
6 selectors, scaled/unscaled mouse input, bounds, release and keyboard handling.
The five removed duplicate controls remain accounted for in 53-key profiles.
Test-CompetitiveVisuals.ps1, groups-regression-debug: PASS for profile rejection,
comparison, reset, six pages, postprocessing and video restart.

Test-VisualRestart.ps1, restart-final-debug: PASS for pending changes, reverting
to the applied value, F5 applying 4x MSAA, and requesting 64x while the device
applies 8x without retaining a false pending warning. The final wording directs
users to Restart video/F5. A subsequent restart-visible-final run waits for menu
frames before capture: PASS, and the pending warning, F5 help and active/pending
sample readout were visually inspected.

Earlier experiments remain as diagnostic evidence, not passes: animation drift
was fixed by pausing through SV_TogglePause; synthetic scoreboard slots lacked
translated skin caches, so the fixture uses embedded model skins; missing
ezquake.pk3 detail/caustic assets were linked into the isolated profile. An early
two-frame screenshot after video restart exposed inherited undefined swapchain
image readback; settled captures pass, and the immediate-readback issue remains
on the TODO list. One overlapping game startup was rejected by the single-instance
guard; final game tests run sequentially. None of these experiments touched the
active user configuration.

Active config SHA256 remains
AC8D02E99079402DB7D52DA0F1196B6A902BB4365105614B0CF02D6B7AD8464D.
User-authored choices have full and visual-only snapshots. CV-003 source deltas
are attributed to OpenAI Codex under user direction in patch 12 and the provenance
manifest; inherited code and game assets retain their existing authorship.
Twelve ordered patches replay exactly to all 61 modified/new source files;
attribution coverage and git diff --check pass. Production regressions pass
32 powerup/player/weapon/ruleset gates, the 224-byte ABI in seven shaders, and
522 GPU lighting/rim results. Debug/Release artifact hashes are refreshed.

## CV-DEFAULT-001 - Approved configuration defaults

Capture-VisualDefaults.py captured all 53 managed values from the user's latest
saved config, including edge depth threshold 16. Active config bytes are unchanged;
all non-visual legacy profile content, including bindings/aliases, is preserved.
Runtime label approved-defaults: Release loads project-default through the real
profile parser and saves all 53 values with identical numerical/string meaning;
normal exit 0. Prior snapshots and exact hashes are in private-project-defaults.json.
No engine code or binaries changed for this configuration-only task.

## INPUT-002 - WASD profile verification

Create-WASDProfile.py verifies collision-free final nonempty letter destinations
and updates the internal jump alias targets. Runtime label wasd-profile loaded
the full file in an isolated Release client. Queries verify W/A/S/D movement,
Caps Lock changing between legacy_sj and legacy_rj on forward/backward press and
release, Z's spare GL selection, and unchanged mouse, Tab, number and keypad
actions. All 53 approved visual values survive the load, and active config hashes
are unchanged. No communication commands were invoked. Evidence:
cache/wasd-verification.json and the runtime console log.

The generic smoke wrapper rejects this full inherited profile because it contains
55 unsupported legacy cvar names. Each name was checked against the unchanged
ESDF source; none is introduced by the WASD transformation. The isolated fixture
also lacks an inherited image/skin. These are retained compatibility warnings,
not a clean full-profile smoke pass. Binding/alias checks and normal exit complete
without Vulkan validation, recursive alias or host errors. Engine source is unchanged.
