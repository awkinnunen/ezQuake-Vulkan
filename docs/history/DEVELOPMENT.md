# Development log

## MAINT-005 correction: conditional particle settings (2026-09-13)

OpenAI Codex incorrectly classified 55 settings as unsupported during MAINT-001.
All 55 are registered by InitVXStuff when QMB particle assets initialize. The
earlier isolated fixture lacked the ezQuake particle resource pack; its unknown
command messages did not establish missing engine support. Removing those
settings reset enhanced lightning and other effects. The removal has been reversed
from the per-file backups, and the destructive cleanup script is disabled.

The user's requested bloom restoration is 0.1 strength, 0.7 threshold and 2.75 radius.
Lightning is restored to 1 with sparks 0.4. Other current graphics and bindings
are preserved. The public defaults include a separate 55-setting particle module;
the visual-only profile still contains 53 values. Runtime fixtures now include
the installed ezquake.pk3 before initialization. Earlier claims that these 55
settings were unsupported or safe to remove are superseded by this correction.

Debug weapon/shaft fire and release tests pass with the restored settings.
The combined public profile test verifies all 53 visual values and all 55 conditional
values after saving through the real engine. Engine source/binaries are unchanged.
Earlier performance measurements used the former bloom/particle state and are
historical evidence, not measurements of the restored defaults.

Menu paths: Options > Graphics > Visual Effects > Effects for Bloom strength,
threshold and radius; Options > Graphics > Advanced Options > Lighting > Particle
Shaft for the enhanced lightning switch. Lightning color, size and sparks do not
yet have dedicated menu rows.


## MAINT-001..004 / PUBLIC-001 - Pre-publication maintenance (2026-09-13)

The user requested configuration cleanup, immediate-restart screenshots,
moving-game regressions and effect-cost measurements, followed by a new public
ezQuake Vulkan repository. OpenGL/Vulkan visual parity was explicitly deprioritized.
WASD controls and all approved graphics defaults are included in publication scope.

OpenAI Codex removed the same 55 unsupported standalone settings from four private
profiles with exact backups. The approved 53-value graphics profile is unchanged.
Screenshot readback now records a copy of the rendered image before presentation,
instead of separately acquiring an image that may not have been initialized after
swapchain recreation. Ordinary frames do not allocate or copy screenshot buffers.

Debug/Release restart captures and moving demo/map/arena tests pass. The arena
test's expected map count was updated from 31 to 123 for the subsequently installed
data. Its original mismatch was a test-fixture assumption, not a renderer failure.
The initial truncated benchmark demo lacked EndOfDemo and waited for more data;
Prepare-MotionDemo.py now writes a normal disconnect/end marker. That first timeout
is not evidence of the historical VSync hang. See VALIDATION.md and PERFORMANCE.md
for final results and limits.

Publication uses a separate clone retaining the upstream branch ancestry. The
portable WASD module uses built-in weapon crosshairs; the private installation's
custom images and full configs remain local. The graphics file is byte-identical
to runtime/project-default.cfg, including depth threshold 16. Original authors,
user direction and Codex implementation are distinguished in the public ledger.

Each entry states its writer, who performed the work, and the evidence level.
Dates below are session dates, not invented commit timestamps. This log was
written in English by OpenAI Codex on 2026-09-13 from this conversation and local
artifacts. The user directed the work; no public name or Git identity is assumed.

## 2026-09-13 / LOCAL-001 — Workspace and build environment

Writer and implementation assistant: OpenAI Codex.
Requested by: the user. Status: local work; not committed or published.

Downloaded ezQuake, vkQuake, vkquake-rt and RTGL1 with their Git history. Installed
Git for Windows, Visual Studio Build Tools and Vulkan SDK. Added PowerShell build
scripts, an optional VS Code workspace and a Vulkan loader/device probe.
Selected RTGL1's quake branch after finding that its default hl1 branch was not
the interface expected by vkquake-rt.

The original ezQuake OpenGL build and vkQuake Debug build succeeded. RTGL1 and
vkquake-rt built with four local compatibility fixes, identified as CODE-001
through CODE-004 in ATTRIBUTION.md. Codex made these fixes in this session;
they are not attributed to the upstream authors. RT shaders compiled and passed
SPIR-V validation (51/51). The Vulkan probe passed, but the local driver did not
advertise the required RT extensions. No gameplay or RT image validation occurred.

Speex and SpeexDSP are voice dependencies. Replacing their autotools dependency
build was discussed; no such replacement was made.

## 2026-09-13 / LOCAL-002 — Existing Vulkan fork adopted

Writer and operator: OpenAI Codex. Direction: user requested tibazera's version.
Status: fetched and compiled; inherited source unchanged.

Found tibazera's SDL3/Vulkan PR #1145 and fetched feature/sdl3-vulkan-pr at
91859a996daede0df166e2a55a5f08d56b052b21. Created the linked ezquake-vulkan worktree
on dev/tibazera-vulkan and set its upstream to tibazera/feature/sdl3-vulkan-pr.
Changed the workspace's default build/debug target to this fork. The original
OpenGL checkout remains available for comparison.

The Vulkan-enabled x64 Debug build succeeded without source modifications.
The log ends with step 483/483 linking Debug/ezquake.exe; CMake records
RENDERER_VULKAN=ON. The executable hash is in build-artifacts.json.

The earlier plan underestimated existing work because it had not included the
open PR. PORTING.md now records the changed direction. TIBAZERA-TODO.md separates
current source observations, historical reports and upstream claims of fixes.
Gameplay tests remain pending because game resources have not been supplied.

## 2026-09-13 / LOCAL-003 — English documentation and attribution

Writer/editor: OpenAI Codex. Requested by: the user, for possible publication.
Status: local documentation and provenance changes; no publication performed.

Rewrote the workspace README, porting notes, TODO assessment and validation record
in English. Added this dated log and an attribution ledger that identifies every
local source fix, its base repository and patch. Added machine-readable provenance
for source history and local artifacts. This entry does not claim authorship of
any inherited renderer implementation.

The upstream Portuguese CONTINUE.md remains unchanged as source evidence. The
English TODO assessment translates and consolidates selected findings; it is not
a line-by-line translation of the entire historical diary. Upstream files already
written in English retain their original author notices.

## 2026-09-13 / LOCAL-004 — Detailed Vulkan implementation plan

Writer and source reviewer: OpenAI Codex. Requested by: the user.
Status: planning/documentation only; no engine source changes.

Added IMPLEMENTATION-PLAN.md (PLAN-001) with a capability policy, shader-data and
resource-ownership design, file-level Vulkan packages, optional asset-preserving
visual improvements, a separate RTGL1 backend plan, test gates and effort ranges.
Checked current Khronos and AMD primary documentation for the relevant API rules.

The targeted review confirmed that alias models currently require bindless shaders
despite older fallback comments; the world push block is 176 bytes and the global
table has two 8192-element combined-image-sampler arrays. These are portability
and implementation findings, not newly reproduced runtime crashes. It also
confirmed existing per-swapchain-image presentation semaphores, MSAA, persistent
pipeline caching and dynamic-buffer memory selection, so the plan does not claim
those facilities need to be introduced from scratch.

The proposed first milestone is compatibility, lifetime safety, visual parity and
world bindless rendering. Temporal/sync2 and enhanced visual work are separately
scoped; the RT plan begins with a supported-hardware harness and an ownership
audit because the reviewed RTGL1 creation interface accepts native surfaces rather
than an injected raster VkDevice/command buffer. Full gameplay and RT testing
remain pending. Documentation links and provenance snapshots were checked.

## 2026-09-13 / LOCAL-005 — nQuake data and direct build launchers

Writer/installer and integration script author: OpenAI Codex, at the user's request.
Status: installed locally; engine source unchanged; no publication.

Downloaded official nQuake snapshot packages qsw106.zip, gpl.zip, non-gpl.zip and
textures.zip from nQuake/distfiles GitHub release assets. Installed their base data
and preserved notices under gamedata/nquake; the shareware archive contributes
pak0.pak and documentation, not old DOS executables. Optional addon packs were not
needed. Archive provenance and the initial file snapshot are in nquake-install.json.

Added Run.ps1, four double-click Debug launchers and prompt-free VS Code launch
configurations. Builds execute from their original output paths with nQuake as
the working directory and -basedir ., avoiding long absolute paths inside legacy
game/mod code. ezQuake uses -nohome. Added the small preset normally generated by
the nQuake installer. The bundled nQuake executable, if present, is not the target
of these development launchers.

An upstream ezQuake automated startup exited 0. A visible Vulkan launch reported
bindless support, loaded e1m1 and had a responsive game window titled standby -
1/32 - e1m1; the owned test process was then closed. The first hidden Vulkan test
outlasted its scripted timeout and was terminated. One SDL3-fork OpenGL automated
run exited with 0xC0000005 after shutdown began. Early scripted screenshots were
black, so they do not establish visual gameplay correctness. These limits and
the runtime log location are recorded in NQUAKE.md. Restored fresh nQuake first-run
settings after the test-generated config and closed the test processes.

## 2026-09-13 / LOCAL-006 — Release Vulkan fullscreen launcher

Writer/script editor and build operator: OpenAI Codex, at the user's request.
Added Start-Vulkan-Fullscreen.cmd and Run.ps1 -Fullscreen. The new launcher selects
the active fork's Release executable directly, renderer 2, vid_fullscreen=1 and
vid_usedesktopres=1. It does not enable a debugger, -dev or -condebug. Existing
Debug launchers retain their windowed defaults. Command-line overrides are applied
before video initialization and after autoexec to clear conflicting latched values.

Built Release successfully (exit 0), with /O2 /Ob2 /DNDEBUG. Log:
tibazera-release-build.log. Updated build-artifacts.json and documentation/provenance.
Checked PowerShell syntax and resolved launch arguments. No fullscreen Release
runtime test was performed. Engine source remains unchanged.

## Inherited diary: reading and attribution

The authoritative historical text is ezquake-vulkan/CONTINUE.md at the pinned
revision. Its latest Git change is attributed to tibazera (91859a99). Within the
document, August and July entries label Claude-assisted sessions, some reference
Opus or Fable consultations, and the July 5 entry labels a Codex/Linux session.
These are upstream disclosures, not actions by the Codex session documented here.
Git authorship alone does not identify who wrote each sentence or used each tool.

The August 6–7 entry reports fixes for drawflat push-constant offsets, dynamic
buffer memory preference, GPU timing and descriptor updates. It names bindless
world rendering as the next unfinished task. Earlier August entries report SDL3
completion, caustics, outline and screenshot/restart fixes. July entries describe
black-item/TDR investigations and superseded diagnoses. The July 5 section
explicitly records a revert; its intermediate fixes must not be assumed present.
These are attributed upstream reports, not independently repeated tests here.

## Format for future entries

Use a stable entry ID, date, writer, implementer, human direction/review if known,
base revision, affected paths, commit or patch references, validation actually
performed, and remaining limitations. Distinguish translated/quoted upstream
reports from new observations. Do not backdate entries or assign another person's
Git identity to local assistant changes.
## LOCAL-007 — Vulkan correctness pass in progress (2026-09-13)

Writer and implementation assistant: OpenAI Codex. Directed by the user to apply
four changes in order, test each, and update documentation. Base remains
91859a996daede0df166e2a55a5f08d56b052b21; changes are uncommitted.

Step 1 passed: Debug build, production limit boundary tests, 13 C/SPIR-V layout
comparisons, all 22 shader modules validated, and an isolated E1M1 runtime test
with Vulkan validation, screenshot and normal exit 0. Evidence:
cache/step1-tests.log and cache/step1-verified-runtime.log.

The actual AMD device reports maxPushConstantsSize=128. Initial checks correctly
rejected the inherited 176-byte world block; a subsequent run found a 160-byte
sprite block. The final world ABI is 128 bytes (byte-valued tint colours packed
losslessly, independent flags shared by C/GLSL), and sprite ABI is 68 bytes
(precombined MVP). The inherited caustics offsets disagreed across three fragment
shaders; one shared layout now prevents this. Device indexing limits are checked
before creating the logical device and bindless resources; pipeline layouts
check ranges and descriptor-set counts. vid_gfxinfo reports actual limits.

Two runtime prerequisites were fixed separately: swapchain screenshot images
need transfer-source usage, and QTV shutdown unlocked an unowned mutex. A
temporary diagnostic stack traced the latter to qtvlist_deinit, not audio.
The diagnostic code was removed. QTV now retains/joins its updater, bounds HTTP
requests to 10 seconds, and tears down before the console. These changes are
recorded separately in attribution. Earlier failed test logs remain as evidence.

The user reported the previous Release fullscreen launcher worked before this
pass. Automated screenshots in the new fixture now contain the rendered map;
this is distinct from the earlier black startup screenshots.

Step 2 passed: VK_PopulateConfig now sets NPOT support before shared texture/HUD
initialization. OpenGL retains its own setter on renderer initialization. The
actual Image_MipReduce and VK_BuildMipPyramid implementations passed 1,094 cases
(1..33 on both axes plus larger NPOT, POT and 1xN/Nx1 images): every mip dimension,
offset and RGBA byte matched an independent reference, with protected guard pages
after input/output buffers. The inherited box filter drops the final row/column
for odd dimensions; no filtering algorithm change is claimed. Debug and E1M1
validation/screenshot/exit passed. Evidence: cache/step2-tests.log,
cache/step2-runtime.log; patch: patches/ezquake-02-npot.patch.

Step 3 passed: sky UV limits use textureSize for the selected face, independently
per axis. 512x512 keeps the inherited one-texel inset; 1- and 2-texel axes clamp
to their centre. Production GLSL passed 42 GPU-executed cases on the AMD device.
The isolated runtime loaded six synthetic PNG faces (256, 512, 1024, 300x173,
1x513, 513x1), captured E1M1 and exited 0 without Vulkan validation errors.
A test-only diagnostic command unavailable in this renderer was removed and
unknown-command checks made strict before the final run. Evidence:
cache/step3-tests.log, cache/step3-runtime.log; patches/ezquake-03-skybox.patch.
This tests sampling bounds and loading, not exhaustive visual seam inspection
from every direction or every texture pack.

Step 4 passed: the normal alias material pass uses the existing GLM directional
lighting formula (normal, ambient, shade and entity yaw). Lighting occupies the
normal pass's otherwise-unused altColor lanes; the push block stays 128 bytes.
The inherited uniform CPU multiplier was removed to avoid double lighting.
The shadelight>=1000 sentinel, alpha and special passes are preserved. The shared
vertex input supplies the current pose's normals, as GLM does; this is not a new
normal-interpolation feature. MD3 dispatch uses the same VK_AliasQueueDraw path.

511 production GLSL GPU cases matched the untouched GLM source: all 162 MDL
normals at three yaw angles, zero/light saturation and sentinel boundaries, and
fullbright/shell/shadow/outline mode bypasses. Alpha was unchanged in every case.
Debug and E1M1 + mixed-size sky + screenshot + normal exit 0 passed validation.
Evidence: cache/step4-tests.log, cache/step4-runtime.log;
patches/ezquake-04-alias-lighting.patch. Full scene parity and a dedicated MD3
visual suite remain outside this pass's verification.


Final verification: Release was rebuilt and passed its isolated Vulkan map test.
Same-fork Modern OpenGL Debug and Vulkan vid_restart also passed, all exiting 0.
The QTV regression used a loopback endpoint held until engine shutdown started:
only one updater request was issued, the duplicate update was rejected, the
owned worker completed during shutdown, and the client exited 0. This test also
exposed a Windows PowerShell 5 process-handle issue in the harness; caching its
handle before WaitForExit fixed exit-code capture. Engine assertions remain
failures, never ignored. Evidence: cache/qtv-shutdown-test.log.

The four ordered patches were applied to copies of pinned source files in a
fresh temporary repository. All 31 modified/new source files matched the working
tree exactly, all were covered by the attribution manifest, and git diff --check
passed (cache/patch-verification.log). Debug/Release hashes were refreshed in
build-artifacts.json. Documentation, TODO and provenance are English and disclose
Codex implementation separately from inherited authorship. No commits, push or
publication occurred; the user may review the ordered patches before publishing.

Final screenshot fixture adjustment closes startup menus explicitly in its own
profile. Vulkan/OpenGL E1M1 scene captures were inspected: world, weapon and HUD
are visible. Both runs exited 0 (cache/final-scene-vulkan.log and
cache/final-scene-opengl.log). The differing spawn views are not a visual parity
benchmark. The test harness and these final notes are included in the refreshed
workspace provenance hashes.

## LOCAL-008 — KTX master update and selective QuakeC fixes (2026-09-13)

Writer/reviewer/implementation assistant: OpenAI Codex. The user requested merging
useful GPL rerelease QuakeC corrections while omitting closed-engine and new-asset
dependencies, then explicitly requested upgrading KTX to its newest source branch.

The installed nQuake local-game path runs a KTX QVM. KTX contains the relevant
gameplay as C; the ezQuake tree has no QuakeC game-source package. The final
integration therefore updates KTX to master 631584f7fac3c3891826224d36d873c99880d353
(1.48-dev) and adapts three fixes from the unmodified id Software QuakeC reference
634eefab09a77eb7b5f5ca7078ba3d8784a91142. Existing source histories/notices remain.
The local branch is dev/rerelease-fixes; no commits or pushes were made.

CODE-011 preserves key audio on its own channel and installs the secret-door
death callback at initial spawn and rearm, with a type-correct C use/death wrapper.
CODE-012 lets target dispatch proceed after killtarget removal. Only doors.c and
subs.c are modified in KTX. The reviewed exclusions (engine built-ins, localization,
achievements, new resources, expansion modes, unnecessary balance changes and
already-fixed cases) are recorded in QUAKEC-MERGE.md.

The three regression groups fail against pristine KTX HEAD and pass after the
adaptations in 19 production-function scenarios. Debug and Release builds and
Vulkan Debug/Release plus Modern OpenGL Debug E1M1 runs passed; native DLL loading
was required, so silently falling back to the old QVM would fail the fixture.
The Release linker outputs directly into the installed qw directory for existing
launchers. Original archives/configs remain; the old QVM is available for rollback.

Exploratory testing also checked a 1.46-dev source baseline before the user chose
the current master upgrade. Only ktx-final-* and ktx-installed-* logs describe the
delivered branch/build. Initial sandboxed CMake detection failed; MSVC builds
succeeded with the required execution access. A test fixture initially lacked its
string-comparison stub; after adding it, no production test failures remained.

The KTX version upgrade and native compilation are material changes beyond the
three local fixes. The full set of upstream KTX changes, all multiplayer modes,
bots, registered maps and network/demo behavior are not comprehensively validated
by this pass. The fixes affect local KTX games, not external servers. Attribution,
source lock, module hashes, patch replay and validation notes accompany the work.

The final installed Release module also passed its own native-loading Vulkan map
test on retry. The first installation test stopped before console logging in an
Error window; its cause remains unresolved and its timeout is retained as failed
evidence. The retry used the same installed KTX DLL.
`cache/ktx-installed-retry-runtime.log` is the successful installed-module evidence.

## CFG-001 — Config browser and keyboard preview (2026-09-13)

Writer and implementation assistant: OpenAI Codex, directed by the user. The
existing Options/Config import screen now lists only CFG files in the config
save directory and previews literal bindings on a keyboard/mouse diagram.
Previewing does not execute commands. The explicit Load action reads the exact
selected file before the existing defaults/autoexec/reset sequence; no basename
fallback can select a different file. Modifier groups use the engine's own key
mapping, with an explicit physical-mode parameter for read-only previews.

New parser/menu modules, exact-path loading, an opt-in registered-file-types
filter, a direct menu command and a Release fullscreen launcher implement this
feature. All upstream notices remain. CFG-001 in the source manifest and
patches/ezquake-05-config-browser.patch identify this work independently from
Vulkan corrections and the separate KTX QuakeC adaptations.

Validation: production parser edge cases plus 1,000 randomized 30-command
sequences; real Vulkan Debug/Release and Modern OpenGL Debug map/menu tests;
empty-directory handling; extension/archive/subdirectory filtering; file
selection and keyboard hit testing; unchanged live binds while browsing; actual
load and left/right modifier binds; screenshot and exit 0. See
CONFIG-BROWSER.md and cache/cfg-runtime-*.log for evidence and limits.

The first test found the inherited FL_Draw minimum height of 80 pixels; the
layout was corrected and subsequent tests passed. Builds used installed pinned
dependencies with VCPKG_MANIFEST_INSTALL=OFF and GIT_SUBMODULE=OFF after restricted
network/subprocess errors. Native Windows execution was required for Ninja and
runtime tests. No dependency update, public push or commit was made here.

Final CFG-001 evidence: Debug and Release rebuilt after the trailing-comment
parser correction; cfg-runtime-final.log passed on the final Release binary.
Five ordered ezQuake patches replay to all 42 modified/new source files exactly
(cache/cfg-browser-patch-verification-final.log). The source manifest covers every
path and git diff --check passed. Launcher arguments and PowerShell parsing were
checked; final binary hashes and provenance exports were refreshed.

## SP-001 — Restore the nQuake shareware campaign (2026-09-13)

Writer and implementation assistant: OpenAI Codex, directed by the user.
Inspection of the actual installed PAK directory confirmed original progs.dat,
start and all eight Episode 1 maps. Reproduction on the preceding Release
binary (cache/runtime-sp-before/qw/qconsole.log) proved that server.cfg changed
maxclients 1 to 32, deathmatch 0 to 1, spprogs to qwprogs, and interpreter type
0 to 1; saving was refused as a multiplayer game. Earlier E1M1 rendering smoke
tests had not verified campaign gameplay.

The explicit campaign path now bypasses multiplayer server.cfg, resets map
progression/pause/match limits, honors the original QuakeC interpreter selection,
and prefers original progs.dat over multiplayer fallback when spprogs.dat is
absent. The load menu accepts the compiled-in NetQuake support and closes its
probe handle in builds that still require spprogs.dat. Save paths fall back to
the game directory when -nohome leaves com_homedir empty.

New command newgame uses the same implementation as the New Game menu. The
Start-SinglePlayer.cmd launcher starts the Release Vulkan fullscreen campaign.
No user configs or PAK/PK3 assets were edited. Native KTX source/DLL were unchanged.
Source changes are SP-001 in the manifest and ezquake-06-singleplayer.patch,
following the five previous patches. No upstream authorship or commit is claimed.

Test-SinglePlayer.ps1 uses isolated profiles with hard-linked nQuake assets and
native KTX, plus an invalid multiplayer .dat sentinel to detect wrong fallback.
It visits start and all eight episode maps, verifies NQ interpreter selection,
monster/player entities, E1M1 save/reload and player health/inventory, and writes
load-menu/game screenshots. -AfterMultiplayer exercises KTX before newgame.
The first run also exposed the inherited empty-home save-path bug; its nine
misdirected test saves were moved to cache/runtime-sp-debug/misdirected-save.
See VALIDATION.md for the final runs and the untested gameplay boundaries.

SP-001 final evidence: Vulkan Debug fresh startup and Vulkan Release after KTX
passed the full scripted map/spawn/save sequence (runtime-sp-debug-final and
runtime-sp-release-final). The load menu screenshot shows the E1M1 save and
23 total monsters. Six patches replay exactly to the 47-file export snapshot;
concurrent multiplayer-menu work in another task is tracked separately. Runtime
fixture waits were shortened to avoid the engine's command-buffer loop guard.

## ARENA-001 — Local Arena menu (2026-09-13)

Implementation and writer: OpenAI Codex, directed by the user. Added Local Arena
between Multiplayer and Options, using the existing Quake big font as requested.
The menu hosts KTX inside ezQuake, lists installed BSP maps, applies FFA/duel/2on2/
Clan Arena, adds/removes bots at a chosen skill, readies the player and stops the
local server. There is no additional server process. See LOCAL-ARENA.md.

The new module reuses VFS enumeration and the main menu's scaled mouse helper.
Map tokens are bounded/validated and data availability is checked before ending
a game. Actions require local KTX plus an active loopback client; the original
single-player interpreter and external servers are excluded. User cfgs, assets
and KTX source remain as they were before this feature.

Initial compilation exposed a private mouse helper/type; these were made shared
menu API. A temporary Debug link lock cleared when the campaign test exited.
Initial automation was invalid because Host_Init flushes waits before ordinary
frames; the fixture now starts after map connection and resets the inherited
runaway counters only through a developer test checkpoint. The initial forced
startup screenshot also emitted a Vulkan layout diagnostic; normal initialized
menu tests are tracked separately. The first CA assertion was too weak and
accepted the previous FFA state: it now requires k_clan_arena=1 and match state.
KTX's actual client command is carena, not its internal mode name ca. A debug
status formatter was corrected to copy rotating Info_ValueForKey buffers.

ARENA-001 is separate from the concurrently completed SP-001 campaign repairs.
The new menu/test/docs and patch identify Codex's contribution; upstream notices
are preserved, with no fabricated commit/human author or review identity.

ARENA-001 verification: Vulkan Release and the final Vulkan Debug binary passed
all four mode actions, bot add/remove/count/skill, active Clan Arena and bot
movement, shutdown, SP exclusion and second-map startup. Seven patches replay
exactly to all 50 changed/new source paths. The initial Debug timeout happened
during final DM6 loading; the successful retry gives the full scenario 90 seconds.
User-facing instructions are in LOCAL-ARENA.md. OpenGL evidence is appended
when its independent run completes.

ARENA-001 Modern OpenGL Release also passed the full scenario on the final
binary (runtime-arena-opengl-verified). Debug/Release builds, documentation,
source patch and provenance are complete. No commit or publication was made.

## ARENA-002 — Startup menu and deathmatch selection (2026-09-13)

OpenAI Codex, directed by the user. The installed profile's cl_onload was
sb_refresh, which intentionally opens the console before refreshing the server
browser. Changed only that value to menu; a byte-for-byte check verified every
other config byte was preserved. A local backup is in cache; public provenance
records the setting delta and hashes, not the user's personal config text.

Added a deathmatch 1–5 row showing both the proposed and live server values.
Mode selection sets appropriate defaults: FFA/Duel 3, 2on2 1, Clan Arena 5.
Start snapshots the mode/rules/map and applies them once when that local KTX
connection becomes active. KTX's normal dmmN commands enforce its own rules.
Mode changes apply the matching deathmatch selection too. No KTX source or game
assets were changed. Twelve control rows fit the original menu canvas.

Release/Debug builds passed after correcting an include-order dependency on
mouse_state_t. The expanded real KTX Vulkan Release fixture passed automatic
FFA with deathmatch 3 before any Apply click, live 3->1->3 changes, Clan Arena 5,
all four modes, bots, movement, shutdown, SP exclusion and map restart. Evidence:
cache/runtime-arena-dmm-release. The updated layout screenshot was inspected.
The startup preference was verified in the installed profile and traced through
Startup_Place; a separate cold-start screen capture is not claimed.

## CV-PLAN-001 — Competitive Visuals design (2026-09-13)

Writer: OpenAI Codex, directed by the user. Recorded approval of the next engine
work sequence, the conservative bloom constraint, and a dedicated menu holding
all discussed clarity/stylization controls. Reviewed existing Vulkan drawflat,
world/model outlines and their ruleset gates, plus Riot's primary descriptions
of VALORANT shading and environment clarity. COMPETITIVE-VISUALS.md distinguishes
those references from the proposed Quake adaptation. This turn adds design
notes only; no new renderer/menu implementation or performance claim is made.

## CV-001 — Competitive Visuals implementation (2026-09-13)

OpenAI Codex, directed by the user. Implemented the six-page menu, 53 independent
settings, Original/Clean/Soft Cel presets, visual-only named profiles and complete
managed-setting comparison/reset. New Vulkan shader code implements world detail
reduction, gradient/soft-cel lighting, a restrained shirt-colour surface rim,
world contact AO, bright-pixel bloom, manual tone/exposure and sharpening.
Powerup bits unconditionally suppress the new rim; original powerup shell passes
remain separate. Existing outline/ruleset, shadow, decoration and AA facilities
are exposed with bounded independent controls. No Riot code or assets were used.

The postprocess/HUD split exposed layout and render-pass compatibility errors
under validation. Fixed next-frame offscreen discard, matching HUD/composite
dependencies, and MSAA/depth preservation when the scene pass is resumed. Kept
128-byte push constants with a fence-owned 224-byte UBO and five descriptor sets.
Multiview now selects the existing separate 3D/2D scheduling path. Shader tests
initially required a floating-point tolerance for cel plateaus (maximum observed
negative numerical delta 1.8e-7); this was not a visible response discontinuity.

COMPETITIVE-VISUALS.md records usage, implementation, tests and explicit limits:
8-bit scene target, bright-pixel rather than emissive-mask bloom, world-only AO,
projected rather than new dynamic shadows, and no temporal AA. Those backends and
recorded-motion/gameplay acceptance remain on the roadmap. The original complete
visual design remains the intended direction; this entry does not mark all future
renderer work complete. Source patch 09 and provenance distinguish CV-001 from
all upstream, earlier Codex and concurrently completed Arena contributions.

## DATA-001 — Private Quake asset import (2026-09-13)

OpenAI Codex performed the import, inspections, test fixture and documentation
under explicit user direction in the separate asset-import task. The user
provided a private legacy Quake installation, flagged possible modifications and asked to preserve nQuake
improvements. Imported id1/pak1.pak and the maps-only pak2.pak; imported the
12-entry pak3 pak.bak as destination pak3.pak after verifying every entry against
the installed shareware PAK. The active old replacement-model/sound PAKs and
Source/Frogbot packages were excluded. The map pack's shared-entry differences
are recorded without assuming original/modified authorship. Renamed only the
GPL substitute-map package to .disabled; seven core nQuake asset packages match
the original installation's SHA-256 values. No engine or QuakeC edits in DATA-001.

The isolated Vulkan Release test loaded/saved all 38 base-game maps and checked
campaign monsters. The first run reached DM1 before its 55-second harness timeout;
a rerun with a longer deadline completed normally, without Vulkan validation
errors. See cache/runtime-imported-quake-release-2/result.json for the exact
executable hash, and PRIVATE-GAME-DATA.md / private-game-data-import.json for
resource provenance and rollback. Private game bytes are excluded from any
public source release; original asset authorship is retained.

## INPUT-001 — Legacy controls and nQuake communication (2026-09-13)

OpenAI Codex, under user direction. Located the user's old launcher chain through
wilhos.cfg to configs/my/fuh.cfg. Extracted the RJ/speed-jump, weapon preference,
shotgun-return and weapon-crosshair logic with five original PNGs. The user chose
ESDF and requested nQuake communication/helpers alongside it, shifting colliding
letters one column right. Imported aliases/images use legacy_ names; 181 original
nQuake alias definitions remain unchanged. Six teamsays help labels match the new
keys. All pre-import config bytes remain as the active config's prefix; full
before/after profiles are available in the config browser. The private import
manifest records source and image hashes without asserting unknown authorship.

Codex adaptations restore keyboard pitch speed after jumps, handle overlapping
forward/back holds, select crosshairs by weapon number instead of configurable
item names, and consistently show the idle LG image while hiding it during fire.
The initial old-style compound condition was corrected to nested aliases after
a real runtime test rejected 'and'. Script permission level 2 enables the user's
requested multi-command controls; server/ruleset limits remain enforced.

Single-player testing exposed the NQ parser's direct active-weapon assignment,
which skipped f_weaponchange. Both encoding branches now call existing CL_SetStat
inside their unchanged-stat guards. Patch 10 contains only cl_nqdemo.c. Debug and
Release builds passed. Real Vulkan SP and native KTX/QW tests passed weapon/image
changes, LG hold/release, shotgun return, jump binding states and pitch restoration.
The server save confirms one consumed rocket and upward velocity 606.798828 with
attack/jump released. Ten patches replay exactly to 60 changed/new source files.
See LEGACY-CONTROLS.md and VALIDATION.md for usage and exact test limits.

## CV-TUNE-001 — Visible effects, smaller crosshairs and pickup autoswitch

OpenAI Codex, 2026-09-13, under user direction. Added the Vivid visual-only
profile and applied stronger existing shader settings to the active config
and legacy-esdf.cfg. Reduced the imported crosshairsize 8 to 2; enabled the
existing Gun Autoswitch settings w_switch 8 and b_switch 8 for weapon/backpack
pickups. The legacy weapon-button aliases and all non-target configuration
lines remain unchanged. The control overlay also applies the new size/switch
values so reloading it does not undo them. No engine source or shader code was
changed. Backup bytes and exact setting/hash deltas are in the private tuning
record; inherited user configuration is not reattributed to Codex.

## CV-002 — Interactive competitive controls and crosshair adjustment

OpenAI Codex, 2026-09-13, under user direction. Replaced numeric-only rows with
34 sliders, nine On/Off switches and ten mode selectors. Added bounded mouse
capture/drag with menu scaling, named common modes, page-title mouse navigation
and row counts. Fixed existing binary cvars failing to turn off on activation.
All 53 settings retain their profile membership, ranges and renderer behaviour.

Adjusted crosshairsize 2 -> 2.5 in four user configs/overlays. A byte-preserving
edit updates both definitions in the full legacy profile; every unrelated byte
is unchanged. Backups: cache/competitive-widgets-backup; exact hashes and affected
paths: private-competitive-widgets.json. Source patch 11 records the menu change.

The user's preset-provenance question exposed a pre-existing CV-001 menu error:
r_shadows was presented as a strength although the shared admission gate uses
r_shadows.integer and Vulkan fixes opacity. CV-002 now exposes On/Off and repairs
Vivid/legacy values 0.5 -> 1. The active config's current value 0 is preserved.
No shadow shader change was made.

## CV-003 - Grouped graphics menus and verifiable effects (2026-09-13)

OpenAI Codex under user direction. Split competitive readability (34 controls)
from general effects/image quality (14 controls); retained five legacy profile
keys whose controls already exist in Graphics. Added section grouping, per-row
help, disabled reasons and shared keyboard/mouse gating. General postprocessing
and contact AO now operate independently of r_cv_enable; competitive presets and
comparison preserve them. Removed duplicate Outline and classic-only GL Bloom
rows from the main Graphics menu. Original shader uniform ABI is unchanged.

Preserved the user's saved active config exactly and created full/visual-only
user-tuned snapshots with hashes in private-visual-groups.json. Tests caught a
profile-boundary bug: decimal 0.2 was compared against a float lower bound at
higher precision. Validation now uses cvar/renderer float precision.

Added pending-vs-applied MSAA tracking, an active sample-count readout, restart
warning and Restart video/F5 action. Effective hardware-clamped samples are
reported separately from requested samples, preventing repeated false warnings.

The user's follow-up requested proof of rendered effect. Added an explicitly
opted-in local screenshot fixture (-dev -visual-tests, local cheat-enabled server)
that freezes time/camera and uses original model assets through the ordinary
renderer. A/A/B/A image checks reject animation or HUD drift as false evidence.
The fixture is inert in ordinary launches and remote games. Coverage/results are
recorded separately from value-edit and dependency tests.

Final evidence: all 48 controls changed real rendered pixels with zero A/A/A
drift, including underwater caustics, original model skins and MSAA after F5.
Debug validation, 25 dependency gates, 53-value user-profile restoration, all
48 widgets and applied/requested restart states pass. See VISUAL-TEST-RESULTS.md
and VALIDATION.md. Patch 12 replays with the preceding patches to all 61 source
changes; no inherited code or private assets are reattributed.

## CV-DEFAULT-001 - User-approved project visual defaults (2026-09-13)

The user confirmed that increasing Edge depth threshold removed the moving dark
wall boundary, then approved the current settings as defaults. Captured all 53
managed visual values from the newly saved active config; threshold is 16.
Values and appearance choices are authored by the user; OpenAI Codex packaged
runtime/project-default.cfg, installed its named visual profile, refreshed full
and visual user-tuned snapshots, and aligned the legacy ESDF profile's visual
values. The visual-defaults helper now loads project-default instead of Vivid.
Active config bytes, legacy bindings/aliases, engine factory defaults and preset
semantics are preserved. Exact hashes and prior-file backups are recorded in
private-project-defaults.json and cache/project-defaults-backup.

## INPUT-002 - WASD alternative (2026-09-13)

The user requested a second keyboard configuration with actions shifted left
from ESDF. OpenAI Codex created legacy-wasd.cfg from the complete ESDF profile.
The practical mapping shifts letters and preserves the other key groups; boundary
A moves to Caps Lock and Q's spare weapon selection to Z, preserving scoreboard
and console access. Movement aliases' internal rebinding targets and team-help
text are updated too. Final literal bindings support the config browser and avoid
empty-key collisions. The existing active/ESDF files are unchanged by this step.
The new file inherits the approved visual defaults; no engine source changed.
See private-wasd-profile.json for mapping, provenance and hashes.

## FX-001 - Separate Explosion style (2026-09-13, OpenAI Codex)

The user requested the Big explosion appearance without its expanding ring,
then clarified that the existing Big explosion should remain available.
Appended Explosion as value 11 in Graphics > Projectiles > Explosion Type,
preserving all existing numeric values (including off = 10). Both styles use
the same fire, spark rays and underwater bubbles. Only value 7 requests the
optional shockwave; value 11 suppresses both flat and particle ring forms.
Like value 7, value 11 excludes the separate horizontal corona flash.
The visual-profile allowlist now accepts values through 11, so save/load does
not clamp the new selection. Both styles still require initialized QMB assets
and gl_part_explosions; the inherited fallback remains available otherwise.
Installed user configs and approved defaults were not changed.
