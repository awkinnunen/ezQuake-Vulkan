# Attribution and change provenance

## SHADOW-003 authorship (2026-09-15)

The user reported the DM6 Baked with shadows / Map light radius regression.
OpenAI Codex diagnosed and implemented the correction in src/vk_shadows.c and
created the baked-reference regression fixture, evidence and documentation.
Patch 22 and Source-Changes.json identify this edit separately from prior work.
Existing engine, renderer and game-resource authorship remains unchanged.

## SHADOW-002 authorship (2026-09-15)

OpenAI Codex completed the requested shadow follow-up: up to eight point/spot
lights, BSP/optional authored lights, baked-shadow/realtime lighting modes, cached
world bounds and per-frame/view atlases, face/cone culling, bounded round-robin updates,
shader specialization and thirteen live menu controls. Project defaults use four lights
and four updates, preserving unrelated config values. See DYNAMIC-SHADOWS.md and
provenance/shadow2-validation.json for exact scope and evidence.

MULTIVIEW-001 is resolved: sky descriptor updates happen once per fenced frame;
view-specific buffers/uniforms/atlases are immutable for earlier recorded views.
The previously ignored 3D viewports and single-camera screenshot path were also fixed.
Screenshots now rebuild entity lists so static models do not accumulate between captures.
Source patch 21 and Source-Changes.json attribute these changes to Codex; upstream
contributors and game asset authors retain their original attribution.

## RASTER-001 and RASTER-DEFAULT-001 (2026-09-14)

OpenAI Codex implemented optional linear RGBA16F rendering and emission MRT,
world SSAO reconstruction/brush transforms, terminal MSAA/depth store changes,
new menu controls and test fixtures at the user's request. Patch 19 and the
RASTER-001 entry in scripts/Source-Changes.json distinguish these changes from
tibazera/upstream rendering and earlier local patches. No vkQuake-RT code or
replacement game assets were imported for this work.

OpenAI Codex also authored the raster build/export/test helpers and
RASTER-FEATURES.md, and updated the TODO, validation, feature and development
documentation. The user then requested all new settings enabled. Codex added
five explicit settings to active/named/distributable profiles while retaining
existing strengths and all unrelated config lines. The private activation
manifest and backups retain before/after hashes; the original gameplay bindings
and visual selections keep their prior user authorship. No GitHub upload is
part of this change.

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


## MAINT-001..004 / PUBLIC-001 (2026-09-13)

OpenAI Codex implemented the user-requested pre-publication maintenance pass:

- MAINT-001: remove the exact 55 unsupported standalone legacy cvar settings from
  four private configs, with byte-for-byte backups; controls and supported values remain.
- MAINT-002: `ezquake-vulkan/src/vk_main.c`, capture the current rendered/acquired
  swapchain image before presentation; include the existing texture capability header.
- MAINT-003: developer-only map/demo/seek/player-count diagnostics in
  `src/competitive_visuals.c`, and isolated motion/arena regression scripts.
- MAINT-004: repeated Release timedemos with controlled effect overrides and
  a correctly terminated private demo slice; no default graphics values changed.
- PUBLIC-001: an ancestry-preserving public source checkout, English documentation,
  controls-only WASD export with built-in crosshairs, and the exact approved visual profile.

Engine changes in this pass are patch 13, after patches 01–12. Original notices
and authors remain intact. The public controls module adapts user-supplied aliases;
unidentified private PNGs and the full personal configs are not redistributed.
The separate public checkout records the snapshot as a new commit; references below
to uncommitted changes describe the original development worktree.

Prepared by OpenAI Codex on 2026-09-13 at the user's request.
This ledger records provenance, not a transfer of ownership or a new license.
Existing licenses and copyright notices remain with their respective projects.

## Inherited work

| Source | Provenance and credit |
|---|---|
| ezQuake | QW-Group/ezquake-source and the contributors recorded in its Git history and source notices |
| SDL3/Vulkan ezQuake fork | tibazera/ezquake-source, feature/sdl3-vulkan-pr; inherited upstream contributions remain credited to their recorded authors |
| vkQuake | Novum/vkQuake and its recorded contributors |
| vkquake-rt | sultim-t/vkquake-rt and its recorded contributors |
| RTGL1 | sultim-t/RayTracedGL1, quake branch, and its recorded contributors |
| Dependencies | Their individual source notices, licenses and pinned submodule/package histories |
| Quake rerelease QuakeC | id-Software/quake-rerelease-qc, 634eefab09a77eb7b5f5ca7078ba3d8784a91142; id Software copyright and GPL notices retained. Git publisher identity is not asserted to be each fix's individual author. |
| KTX gameplay | QW-Group/ktx master, 631584f7fac3c3891826224d36d873c99880d353, and all recorded contributors; upgrade from the bundled 1.46-dev to 1.48-dev explicitly requested by the user. |

Repository/account names identify sources; they are not assertions of sole
individual authorship. sources.lock.json records exact base commits and submodules.
provenance/git-history.json exports author and committer names separately for the
pinned history. The retained repositories provide full Git records and blame.

The Vulkan fork's Portuguese CONTINUE.md is inherited and unchanged. Its Git
history credits tibazera for the recent diary commits, while the text discloses
Claude, Opus, Fable and an earlier Codex session. Such disclosures do not establish
line-level authorship. The present OpenAI Codex session must not be confused with
the Codex/Linux session recorded there on July 5.

## Local source changes made by OpenAI Codex

CODE-001..004 prepared the environment. CODE-005..010 implement the subsequently
requested renderer correctness/parity pass and its test prerequisites, all on
2026-09-13. They are uncommitted changes implemented by OpenAI Codex under the
user's direction. No human code authorship, review or upstream acceptance is
inferred. Verification claims are limited to VALIDATION.md.

| ID | Repository and path | Change and reason | Patch |
|---|---|---|---|
| CODE-001 | rtgl1/CMakeLists.txt | Initialize BUILD_TYPE from CMAKE_BUILD_TYPE so a Debug build without DLSS selects Debug FSR2 libraries | patches/rtgl1-windows-build.patch |
| CODE-002 | rtgl1/Source/Shaders/CmPrepareFinal.comp | Select LPM/ffx_a.h explicitly to avoid the same-named CAS header | patches/rtgl1-windows-build.patch |
| CODE-003 | rtgl1/Source/Shaders/ShaderCommonGLSLFunc.h | Replace the sampler-returning helper with a macro accepted by the current SPIR-V compiler; also adds final newline | patches/rtgl1-windows-build.patch |
| CODE-004 | vkquake-rt/Quake/host_cmd.c | Replace sendsignon=true with enum PRESPAWN_FLUSH (both value 1), resolving an MSVC enum warning treated as an error | patches/vkquake-rt-msvc.patch |
| CODE-005 | ezquake-vulkan: world/sprite C and GLSL, vk_limits.h, checked pipeline calls, device/info code, CMake | Compact world/sprite blocks to 128/68 bytes; shared flags/ABI fixes caustics; check actual limits and fail unsupported bindless initialization clearly | patches/ezquake-01-limits-and-runtime.patch |
| CODE-006 | ezquake-vulkan/src/vk_swapchain.c, src/vk_main.c | Request supported transfer-source swapchain usage and guard unsupported screenshot readback | patches/ezquake-01-limits-and-runtime.patch |
| CODE-007 | ezquake-vulkan/src/EX_qtvlist.c, src/host.c | Retain/join QTV worker, bound request duration, remove unowned unlock and free shared state before console teardown | patches/ezquake-01-limits-and-runtime.patch |
| CODE-008 | ezquake-vulkan/src/vk_main.c | Enable NPOT capability before shared texture initialization | patches/ezquake-02-npot.patch |
| CODE-009 | ezquake-vulkan: vk_world_flat.frag, vk_skybox_uv.glsl, CMake | Derive sky sampling bounds from each face's dimensions | patches/ezquake-03-skybox.patch |
| CODE-010 | ezquake-vulkan: vk_aliasmodel.c, vk_alias_model.vert, vk_alias_lighting.glsl, CMake | Adapt the existing GLM lighting formula, retain its original mh credit, reuse normal-pass parameter lanes, remove uniform CPU lighting multiplier | patches/ezquake-04-alias-lighting.patch |
| CODE-011 | ktx/src/doors.c | Adapt rerelease key sound channel and secret-door death callback initialization; Codex adds a type-correct C wrapper shared by use/death callbacks | patches/ktx-quakec-fixes.patch |
| CODE-012 | ktx/src/subs.c | Adapt rerelease target dispatch: finish killtarget removal before firing ordinary targets | patches/ktx-quakec-fixes.patch |

Apply ezquake-01 through ezquake-04 in that order to the pinned Vulkan base.
The first patch groups three corrections exposed by the first stage's tests;
the CODE IDs distinguish their purposes. scripts/Source-Changes.json enumerates
all affected source paths. New local files have no base blob; existing files
retain their original notices and Git history. The GLM shader and anorms.h used
as regression references remain unmodified upstream work.

No local engine source changes were made to the original ezquake or vkquake.
The vkQuake v143 override is in the local build script, not its upstream project.

CODE-011/012 are the separate LOCAL-008 gameplay pass. The underlying fixes are
credited to the GPL id Software rerelease sources; the KTX C adaptation, typed
wrapper, tests and documentation were implemented by OpenAI Codex on 2026-09-13.
The rerelease reference repository is unmodified. The KTX patch applies to its
own pinned base independently of the four ezQuake renderer patches. No source or
asset from the proprietary rerelease engine is included in the runtime module.

## Local workspace contributions

OpenAI Codex created/edited the scripts under scripts/, the Vulkan probe under
probes/, ezquake-vulkan.code-workspace, runtime/ezquake/README.txt, the root README,
PORTING.md, TIBAZERA-TODO.md, VALIDATION.md, DEVELOPMENT.md, this ledger and the
provenance export tooling. Documentation translation and editorial summaries are
also Codex contributions. The user requested the work and selected the fork;
no public human name, email, code authorship or review approval is inferred.

PLAN-001 in IMPLEMENTATION-PLAN.md was authored by OpenAI Codex on 2026-09-13
after a further targeted source review and primary-documentation checks, at the
user's request for a detailed technical plan. Its proposed architecture and effort
estimates are local analysis; they are not renderer code changes or commitments
by tibazera or other upstream contributors.

LOCAL-005 adds Codex-authored Install-NQuake.ps1, Run.ps1, Start-*.cmd launchers,
debugger configuration changes, NQUAKE.md and a small local preset.cfg. The
downloaded nQuake packages, bundled assets/client and Quake shareware retain their
original authorship and notices; they are not Codex-authored runtime content.
nquake-install.json records official archive URLs/digests and the installation
snapshot. Normal game-generated configuration/log changes are not new engine code.

sources.lock.json, build-artifacts.json, build logs, patch exports and provenance
JSON are generated evidence. Compiled binaries and shaders derive from upstream
source plus the identified local fixes; they are not new renderer implementations
by Codex. provenance/local-artifacts.json records hashes of local authored files
and patches, while provenance/local-source-changes.json records the changed source
paths and their base/working hashes.

## Keeping attribution intact when publishing

Retain upstream history and notices. Give subsequent local commits the actual
contributor's configured identity and describe AI assistance in commit messages
or this ledger; do not manufacture a tibazera author or a human reviewer. Link
CODE/LOCAL IDs to real commit IDs once commits exist. Git author/committer fields
and the disclosed implementation assistant serve different purposes.

The English workspace notes are current local documentation. The English TODO
is explicitly an assessment of selected upstream entries, not a full translation
or replacement of the original diary. No repository was pushed or published by
this session. See DEVELOPMENT.md for dated entries and VALIDATION.md for limits.


LOCAL-007 also adds Codex-authored test runners, GPU/guard-page regression
fixtures, ordered patch-export tooling and documentation updates. Procedural sky
PNGs are test fixtures, not replacements for nQuake assets. Temporary assertion
stack tracing was removed from the engine; its captured log is diagnostic
provenance, not an additional shipped feature. The four source patches and
source-path manifest are review artifacts; no commit identities were fabricated.

LOCAL-008 adds Codex-authored Build-KTX.ps1, Test-KTX.ps1, Export-KTXPatch.py,
the C/Python KTX regression fixtures and QUAKEC-MERGE.md. Test-VulkanRuntime.ps1
now accepts a game DLL and pre-map commands, checks native loading and keeps
test helper windows hidden. The test-only extracted production code remains
upstream-derived work; recording stubs/assertions are Codex-authored fixtures.
The installed DLL derives from the pinned upstream KTX plus CODE-011/012, not
from an independently authored Codex game implementation.

## CFG-001 — Config browsing and binding preview

Implemented by OpenAI Codex on 2026-09-13 under user direction. New files
src/cfg_bindings.[ch] and src/menu_config.[ch] and the integration changes in
scripts/Source-Changes.json are local assistant work under the project's GPL
terms. The existing file list, key mapping, config reset/load behavior and menu
framework remain upstream ezQuake contributions. No code from the rerelease or
KTX was incorporated into this UI feature.

The implementation, new test runners/fixtures, CONFIG-BROWSER.md and
Start-Config-Browser.cmd are recorded separately as CFG-001. Apply
patches/ezquake-05-config-browser.patch after ezquake-01 through ezquake-04.
No author identity or human review was fabricated. These are uncommitted local
changes; provenance exports record the base blobs and current hashes.

## SP-001 — Campaign startup and saves

OpenAI Codex authored the local engine modifications in src/menu.c, sv_init.c,
pr_edict.c, pr2_exec.c and fs.c on 2026-09-13 under the user's direction. Existing
engine and id Software code retain their original notices and GPL terms. This
work adds no rerelease code or assets to original single-player QuakeC.

The new Test-SinglePlayer.ps1 fixture, Start-SinglePlayer.cmd, ordered patch
and documentation entry are Codex-authored workspace additions. Source hashes
and upstream base blobs are in the provenance export under SP-001. Changes
remain uncommitted; no human review, commit identity or public release is implied.

## ARENA-001 — Local Arena

OpenAI Codex authored menu_local.c/menu_local.h, the main-menu integration and
shared menu mouse helper, CMake entries, Test-LocalArena.ps1 and LOCAL-ARENA.md
on 2026-09-13 at the user's request. The requested main-menu name is Local Arena.
Original ezQuake and KTX notices/ownership remain. No third-party gameplay code
or assets are imported by this change. Patch 07 follows the six prior patches;
shared-file edits are attributed separately from SP-001. Source changes remain
uncommitted, with paths/base blobs/current hashes recorded by provenance export.

## ARENA-002 — Deathmatch and startup preference

OpenAI Codex authored the new deathmatch selection, connection callback, tests
and documentation at the user's request on 2026-09-13. Source paths are listed
under ARENA-002 and patch 08 follows patch 07. The one-setting installed profile
change is separately recorded in runtime/arena-startup-setting.json. All
upstream notices remain, with no KTX gameplay source imported or altered here.
These are uncommitted changes; no human review or public release is implied.

## CV-001 — Competitive Visuals

OpenAI Codex authored the menu/profile system, Competitive Visuals uniforms and
shaders, renderer integration, regression fixtures and English documentation on
2026-09-13 under the user's direction, including the requested distinction between
subtle team-tinted rims and quad/invulnerability effects. The complete touched-path
list is in scripts/Source-Changes.json; patch 09 follows patch 08. Existing ezQuake,
id Software and tibazera notices remain. Riot articles informed design only;
no Riot source/assets or closed rerelease content was imported. These changes
remain uncommitted, with no fabricated human authorship or review. Provenance
exports preserve upstream commit authors/base blobs and local file hashes.

## DATA-001 — User-owned resources, separate from source authorship

The user supplied a private legacy Quake installation for local development tests and requested preservation
of nQuake enhancements. OpenAI Codex inspected/copied the selected PAKs, wrote
scripts/Test-ImportedQuakeData.ps1 and PRIVATE-GAME-DATA.md, and recorded paths,
hashes and preservation checks in private-game-data-import.json. No game asset
creation, asset modification or engine/QuakeC implementation is claimed for this
import. Original Quake assets belong to their original authors; the repack and
any changed map entries have unverified contributor history. The backup's 12
pickup models match installed original shareware resources. Private asset bytes
and their cache/test copies are not included in a public GPL source release.

## INPUT-001 — Legacy user controls and NQ crosshair trigger

The user supplied their old configuration and requested its RJ/weapon/crosshair
behaviour, ESDF keys and nQuake communication/helpers. The old fuh.cfg is a
user-customized FuhQuake config; original snippet/image contributor identities
are not established. private-controls-import.json records exact source/image
hashes. The five PNGs are unchanged copies, not Codex-authored artwork.

OpenAI Codex implemented extraction/namespacing, pitch restoration, overlapping
movement handling, weapon-number selection, idle/firing LG handling, shifted
communication bindings, profiles, tests and English documentation. The original
nQuake helpers keep their inherited credit. The source change in cl_nqdemo.c
routes NQ weapon changes through CL_SetStat and retains tonik's existing notice;
see INPUT-001 in scripts/Source-Changes.json and patch 10. No authorship transfer,
new license for privately sourced scripts/images or upstream acceptance is claimed.

CV-TUNE-001 (2026-09-13): OpenAI Codex authored the Vivid setting selection,
visual-defaults.cfg helper, targeted config edits and documentation under user
direction. Existing engine/shader code and inherited aliases/images were not
changed or reattributed. Private before/after hashes and the exact setting
allowlist are recorded in private-visual-tuning.json.

## CV-002 — Competitive menu widgets

OpenAI Codex, 2026-09-13, implemented the menu widgets, mouse handling, developer
test driver, Test-CompetitiveWidgets.ps1, crosshair-size edits and documentation
under user direction. The slider uses existing Quake menu glyphs and conventions;
no artwork or inherited rendering code is reattributed. Source changes are confined
to src/competitive_visuals.c, recorded in Source-Changes.json and patch 11. Config
byte backups and before/after hashes are in private-competitive-widgets.json.

## CV-003 - Menu dependencies, restart feedback and effect tests

OpenAI Codex, 2026-09-13, implemented the graphics-menu split, help/dependency
presentation, independent general-effect gates, float-boundary profile fix,
MSAA application feedback, opt-in local screenshot fixture and test scripts.
The user's saved visual choices are attributed to the user, with exact config
and snapshot hashes in private-visual-groups.json. Existing renderer code and
private Quake/nQuake models/textures retain their original authorship.
See Source-Changes.json and ordered patch 12 for the source delta.

## CV-DEFAULT-001 - Approved default visual profile

The user selected and approved these visual values, including world edge depth
threshold 16. OpenAI Codex (2026-09-13) extracted the values, packaged the portable
and installed profiles, updated the restore helper and legacy visual values,
and wrote verification/documentation. No engine source changed. Prior snapshots
and per-file hashes are retained in private-project-defaults.json and its backup
directory. The portable runtime/project-default.cfg contains only visual settings.

## INPUT-002 - WASD control variant

OpenAI Codex, 2026-09-13, derived a WASD variant at the user's request, including
literal bindings, alias-internal jump rebinding, help text, collision handling,
verification and documentation. The user's customized legacy controls and
nQuake helpers retain their prior authorship. No inherited images or engine code
were changed. See private-wasd-profile.json and scripts/Create-WASDProfile.py.

## FX-001 - Explosion without a shockwave ring

Requested by the user; implemented and documented by OpenAI Codex, 2026-09-13.
Adds Explosion (r_explosionType 11) using the inherited QMB detpack fire/rays
with its shockwave disabled. Big explosion (7) retains its original behavior.
The underlying particle artwork and effect implementation retain upstream
authorship. Source paths are recorded in Source-Changes.json; delta in patch 14.

## PUBLIC-002 - Latest settings and RT assessment (2026-09-13)

The user requested publication of all changes, including cfg files. OpenAI Codex
updated the portable profile from the saved game configuration: model outline
opacity 0.5, contact AO 0.125, MSAA 4, and Explosion style 11. All 53 visual and
55 conditional settings match the saved configuration. The existing WASD overlay
and built-in crosshairs remain included. Private full configs and game assets
are not part of the export. RTX-FEASIBILITY.md records the RTX 3060 target and
the staged integration proposal; ray tracing remains unimplemented.

## RT-PLAN-002 - Direct RTGL1 implementation plan (2026-09-13)

The user defined the scope: bring across effects and accept the RT renderer's
own appearance, with a technical plan first. OpenAI Codex reviewed the pinned
source interfaces and authored RT-IMPLEMENTATION-PLAN.md, the related roadmap/TODO
updates and the documentation export change. Proposed modules and shader/API
extensions are design work, not implemented source. Host, vkquake-rt and RTGL1
code retain their existing authorship. No renderer or configuration changes are
part of this planning step; future imports and adaptations require separate records.

## RT-FOUNDATION-001/002 - Runtime adapter and initialization handling

OpenAI Codex, 2026-09-13, implemented the device probe, manifest/hash/export
validation, SDL3 harness, generated requirements/inventory, CMake target and
packaging/boundary tests under the user's instruction to start steps 1 through 7.
The user will perform the first RTX 3060 test; no user hardware result is recorded.
The copied src/rt/RTGL1.h is upstream work by Sultim Tsyrendashiev, retaining its
MIT notice and API declarations. One CP1252 dash in a comment is normalized to
ASCII for portable patch replay. Generated function declarations derive from that API.

Codex added explicit UUID selection and central initialization cleanup/error
handling to RTGL1. These are adaptations of the existing library, whose other
code and notices are preserved. Source-Changes.json and the host/library patches
record the changed paths. This entry does not attribute an implemented gameplay
RT renderer or Competitive Visuals port: those are still pending.

## RT-GEOMETRY-001: CPU scene and overlay conversion

OpenAI Codex authored `rt_geometry.h`, `rt_geometry.c`, the instrumented ABI test
fixtures, CMake target and harness call-site change. RTGL1 API types retain their
upstream MIT attribution. The matrix/primitive helpers are new code implementing
the host-to-library conversion; they do not reattribute donor rendering or Quake
model data. Export: `patches/ezquake-16-rt-geometry.patch`.

### RT-PACKAGING-001: portable SDK identity

OpenAI Codex, 2026-09-13. The SDK header identity is computed from canonical
UTF-8/LF content so Git CRLF conversion cannot break package compatibility.
Runtime binary/shader hashes remain byte-exact. Patch 17 records this correction.
The ordered host series now contains 17 patches covering the same 78 paths.

## PERF-COMPARE-001: renderer performance comparison

OpenAI Codex authored scripts/Benchmark-Renderers.ps1,
scripts/Export-RendererPerformance.py and RENDERER-PERFORMANCE.md, and ran the
measurements on 2026-09-14. The high-eyecandy preset is derived from the existing
packaged ezQuake preset; its original content and authorship are not claimed by
Codex. Engine implementations and binary releases retain upstream authorship.
Raw demo/capture material remains in local test fixtures, outside public exports.

## PERF-COMPARE-002: AA diagnostics and isolated profiler

OpenAI Codex authored probes/vk_frame_profile.h, scripts/Build-VulkanProfiler.py,
scripts/Export-AADiagnostics.py, the executable/profiling/validation extensions
to Benchmark-Renderers.ps1 and AA-DIAGNOSTICS.md on 2026-09-14, and performed
the measurements. The profiler header is GPL-2.0-or-later. The generated source
copies and instrumentation.patch retain the upstream engine's existing notices;
only the diagnostic timing hooks are attributed to Codex. No donor renderer,
original ezQuake implementation, packaged preset or game asset is reattributed.
The normal engine source and executable were not changed by this diagnostic work.

## PERF-OPT-001: deferred scene-pass startup

OpenAI Codex, 2026-09-14, authored the scene-pass lifecycle changes in
src/vk_main.c and src/vk_world.c and their declarations in src/vk_local.h.
Existing rendering, shaders, assets and upstream copyright notices retain their
original authorship. The change is exported separately as
patches/ezquake-18-deferred-scene-pass.patch and identified by PERF-OPT-001 in
scripts/Source-Changes.json, including its actual implementation date.

Codex also authored Build-ScenePass.py, Test-ScenePass.ps1, Export-ScenePass.py,
SCENE-PASS-OPTIMIZATION.md and the accompanying benchmark/profiler/provenance
script maintenance. KTX's position command and existing photo-test hooks are
used without claiming authorship of KTX. Screenshots and installed game data
remain local test artifacts.

## PERF-COMPARE-003: optimized renderer comparison

OpenAI Codex, 2026-09-14, authored the optional optimized comparison mode and
additional integrity checks in scripts/Export-RendererPerformance.py, ran the
ten-case comparison, and wrote RENDERER-PERFORMANCE-OPTIMIZED.md and accompanying
development, validation and TODO updates. Engine code, binaries, packaged presets
and game assets retain their existing authorship; none were modified for this
comparison. Private demos and captures remain local.

## BUG-TRIAGE-001: issue registry and focused reproduction

OpenAI Codex, 2026-09-14, authored BUGS.md, the reorganized TIBAZERA-TODO.md,
scripts/Test-MultiviewTriage.ps1 and related documentation/status updates at the
user's request, and executed the focused current multiview fixture. The prior
task assessment is preserved in TIBAZERA-TODO-HISTORY.md with its attribution.
The test uses existing developer checkpoints without claiming authorship of
upstream rendering, demo data or assets. No engine implementation or binary was
modified by this audit. Historical hypotheses are attributed as observations,
not as confirmed new code defects.


## SHADOW-001 attribution

OpenAI Codex authored the new raster-shadow implementation, integration edits, tests, documentation and config activation on 2026-09-14 at the user's request. Patch ezquake-20-dynamic-shadows.patch and Source-Changes.json identify exact source changes after patch 19. Existing tibazera/ezQuake/id Software code retains its authorship and licensing; this feature imports no vkQuake-RT code or game assets.


## MENU-UNIFY-001 / CFG-PRESETS-001 / INPUT-003

User authored the requirements, chose WASD as default/SDFE as alternative, and
selected the existing graphics values now named Balanced. OpenAI Codex authored
the native-menu integration, declarative preset loader/saver, capability checks,
tests, migration/export helpers, alternative preset tuning and English notes on
2026-09-15. Exact engine edits are in patch 23 and scripts/Source-Changes.json.
Original ezQuake settings widgets, tibazera's Vulkan renderer and KTX command
semantics retain their original authorship and licenses. The legacy weapon aliases
originate in the user's old config; earlier snippet authors remain unknown.
No ownership of upstream code or game resources is claimed by this integration.


## 2026-09-15: CFG-PRESETS-002 — preset activation

User report: Apply was unclear and graphics presets could not be loaded intuitively.
OpenAI Codex diagnosed and fixed the native browser: Enter previously only previewed
the focused file. Moving to Apply could preview other rows along the way; leaving
the browser then cancelled the preview. Enter or a click now commits the focused
valid preset and returns to Graphics. F3 keeps the preview without moving focus;
Escape cancels uncommitted changes. Empty/invalid confirmation cannot report success.
Pending video changes still require F5 after loading, with an on-screen reminder.

Patch 24 records the implementation separately from patch 23. Debug (800x600) and
Release (640x480) tests exercise native Enter, mouse down/up, arrows and F3, validate
all 170 preset values after menu exit, and check cancel/invalid-file behavior plus
audio/binding isolation. The existing Release save/backup/scope/restart regression
also passes. See provenance/preset-activation-validation.json for binary/source
hashes and limitations. Earlier unified-menu evidence describes the preceding build.


## CFG-BALANCED-002 — saved user tuning, 2026-09-15 20:34

The user's newly saved configuration updates Balanced: world edge width 1.4 to
2.2, world edge opacity 0.4 to 0.3, midtone response 1 to 0.825, and gl_gamma 0.8
to 0.7. Tuning: user. Extraction, packaging and verification: OpenAI Codex.
Only these four of the 170 graphics fields changed; the installed and portable
Balanced copies match. The previous Balanced file is backed up privately.

Release fixture balanced-update-2034 validates all 170 values through native
preset activation, cancellation, invalid-file handling and audio/binding isolation.
See provenance/balanced-update-2034.json for hashes and the exact delta. Earlier
preset evidence describes the preceding Balanced revision. This successful save
does not establish the cause of CFG-PERSIST-001's earlier missing changes.


## INPUT-004 — nQuake, Quick WASD and Quick ESDF (2026-09-15)

At the user's request, Controls now offers Quick WASD (default), Quick ESDF and
nQuake. The old SDFE label meant the same E-forward/D-back/S-left/F-right layout;
its displayed name is now Quick ESDF. Commands are keyboard_preset quick-wasd,
keyboard_preset quick-esdf and keyboard_preset nquake. Existing wasd/sdfe commands
and ezv-wasd.cfg/ezv-sdfe.cfg filenames remain compatible; esdf is also accepted.

nQuake uses 102 bind declarations and 20 helper aliases extracted from the cached
official distribution's non-gpl.zip (qw/nquake_default.cfg and qw/autoexec.cfg).
Original bindings/aliases: nQuake contributors. Quick behavior: the user's legacy
FuhQuake setup, with earlier snippet authors unknown. Selection code, extraction,
packaging and tests: OpenAI Codex. Exact source hashes are recorded in
provenance/keyboard-presets.json. No claim of new authorship over upstream aliases.

nQuake explicitly replaces all bindings, including otherwise unused keys. WASD
moves; Mouse 1 attacks with the current weapon; Mouse 2 selects lightning; E/Q
select rocket/grenade; Mouse 4/5 select nailgun/shotgun preferences. Team, timer,
demo and volume keys retain their original commands. The loader removes only the
Quick-owned f_weaponchange hook; other hooks survive. A Quick-hidden crosshair
is made visible when that hook is detached. Existing crosshair appearance is
otherwise retained. Quick layouts retain their weapon, jump and crosshair aliases.

Selection leaves the 170 graphics values, sensitivity and audio values unchanged.
The user's config.cfg and autoexec.cfg were neither rewritten nor executed by the
installation step. No automatic startup-load policy was added in this change.
Debug/Release fixtures verify native nQuake/Quick ESDF menu activation, Quick WASD,
legacy command compatibility, exact nQuake binding restoration across repeated
switches, timer aliases, graphics/input isolation and unrelated alias preservation.
The native Controls screen was inspected at 640x480. See the provenance record.


## DIST-001 — Windows x64 test distribution, 2026-09-15

User requested a deploy package and GitHub publication. OpenAI Codex packaged the
Release Vulkan raster client with static runtime libraries, all dependency copyright
notices, portable graphics/keyboard presets, setup/diagnostic launchers, an exact
source-commit manifest and a matching source ZIP. Game assets, private configurations,
KTX binaries, RT harnesses and debug symbols are excluded. Requires existing nQuake
data. Quick WASD/Quick ESDF/nQuake and the updated Balanced profile are included.

A launcher-managed first-run flag loads ezv-dist-first-run.cfg before the existing
config chain, including after startup resets. User config and autoexec values win;
normal launches are unchanged. Setup preserves existing files and installs missing
namespaced profiles. Normal engine saving retains its existing cfg_save_onquit policy.
The distribution marker is written only after exit code 0. Diagnose.cmd collects
qconsole.log and package identity. No autoexec or full user config is rewritten by setup.

Patch 26 records the startup hook. Isolated packaged-binary tests cover fresh,
existing-config/autoexec, and normal starts, all 170 graphics values and repeated
setup preservation. Debug/Release compilation and source patch replay pass. See
provenance/distribution-validation.json. Earlier records describe earlier binaries.
The public test is version 0.1.0-beta.1; no RTX functionality is claimed.


## FRIENDS-001 - existing FTE infrastructure research (2026-09-16)

The user requested easy friend-hosted games without a newly operated external
relay and preferably without additional player-installed software. OpenAI Codex
inspected pinned FTE, QWFWD and MVDSV sources and this fork's networking/menu code,
then wrote FRIENDS-HOSTING-PLAN.md. The proposal uses existing FTE room signaling,
native ICE, authenticated invitations and the embedded KTX server. Short room
codes require host approval because broker rooms may be publicly listed.

Read-only verified TLS/HTTP and one STUN probe against the existing Frag-Net master
succeeded. No room was registered, no TURN allocation was requested, and no game
was joined. Direct gameplay across separate networks and usable existing TURN
coverage are unverified acceptance gates. Ordinary QWFWD/Qizmo client forwarding
was not established as a reverse-hosting solution. No new service was deployed.

This change adds planning, TODO and provenance documentation only. Engine source,
binaries, installed configuration, launchers and published releases are unchanged.
FTE networking work remains credited to its upstream authors; no donor code was
ported. See provenance/friends-hosting-research.json for inspected source revisions,
hashes, sanitized checks and limitations. Implementation tasks FRIENDS-002 through
FRIENDS-007 remain open.


## FRIENDS-002 - native connection prototype and test package (2026-09-16)

OpenAI Codex implemented a standalone Windows x64 FTE-broker adapter, native
libjuice ICE, OpenSSL DTLS, host certificate pinning and secret invitation admission.
No FTE implementation files were copied. The user stated that a second computer
on another network is unavailable now, so the planned two-network gate remains
open and engine/menu integration is deferred pending that evidence.

Debug/Release self-tests, real-broker local pair tests, wrong-key/wrong-fingerprint
rejection, host timeout, expired invitation and extracted-package tests pass.
Host.cmd and Join.cmd require no separately installed runtime. The local ZIP has
matching source/dependency archives, notices and hashes. No new external relay or
other server was deployed; no TURN allocation was attempted. No GitHub release was
published, and no current game binary, launcher or user configuration was changed.
See FRIENDS-HOSTING-STATUS.md and provenance/friends-probe-validation.json.


## FRIENDS-002C - wait for a guest without a deadline (2026-09-16)

The user requested leaving the probe open for a guest joining on a later day.
OpenAI Codex made --host default to unlimited waiting and Host.cmd select
--seconds 0 explicitly. Room metadata and WebSocket keepalives refresh every
30 seconds. Idle polling uses 100 ms; active test polling remains 5 ms. Explicit
finite test durations, console cancellation and the bounded active attempt remain.
A certificate-date regression check verifies the existing invitation-pinning policy
continues working after the ephemeral certificate's date window. No host identity
or invitation is rotated just because time passes. Sleep/network-loss reconnection
is still outside the prototype. The updated standalone package is 0.1.1; engine,
user configuration and published Starter are unchanged. See the wait validation
record in provenance/friends-probe-wait-validation.json.


## FRIENDS-003/004/005A - native gameplay and reusable invitations (2026-09-16)

Requirements/product choices: AWK. Implementation, tests and documentation:
OpenAI Codex. The user supplied join-result.json and confirmed host/guest were in
different cities: direct ICE, pinned DTLS, admission and 100/100 echo packets pass.
That evidence opened the engine-integration gate for this network pair.

New code in src/friends.c and src/friends/* implements asynchronous native
multi-guest transport, opaque NA_FRIENDS addressing, protected persistent identity,
secret rotation, admission gates and existing-widget menus. Local Arena can open
the saved room after starting KTX; online guests cannot administer another host.
OpenSSL symbols are kept separate from the inherited unrelated SHA1 functions.
Broker slot reuse, stale join generations, graceful departure and map sign-on were
covered during integration. No existing graphics/key presets or autoexec changed.

Release gameplay validation uses a host, two guests and one arena bot through
the real Frag-Net service. Rejoining in the same client, chat and DM6 -> DM2 work.
Separate tests cover normal UDP before hosting, rejection of plaintext bypass,
binary fragmentation, persistent identities, wrong keys/pins, close/reopen and
rotation. See provenance/friends-engine-validation.json for exact source/binary
hashes and the scope of each result. Wider network coverage and distribution
integration remain TODO items; no new external service or GitHub release was made.

New adapter code is GPL-2.0-or-later. libjuice remains MPL-2.0; OpenSSL remains
Apache-2.0. Distribute the combined Friends-enabled binary under GPL-3.0-or-later
with matching sources and dependency notices. FTE broker/protocol authors and
ezQuake/QW/KTX authors retain their existing credit; no FTE implementation files
were copied. See docs/FRIENDS.md for current player instructions and limitations.

## 2026-09-16 — Native Windows invitation registration (FRIENDS-005B)

Requested explicitly by AWK; implementation, test automation and notes by OpenAI
Codex. Friends -> Windows links registers the current executable and data folder
for ezquake-vulkan:// in HKCU. A matching running instance receives validated data
through a bounded local mailslot; otherwise the engine starts and asks the player
to confirm. Existing qw:// registration and personal configuration are preserved.

Actual Windows shell dispatch revealed the OS inserts a slash before the query.
The parser now accepts that equivalent spelling while preserving old invitations
and strict validation elsewhere. Cold start, warm handoff, cancel/confirm, secret
redaction and rejection of appended console arguments pass in the native engine.
The test config polling was moved out of startup's Cbuf_Flush loop so tests now
exercise completed initialization. Two guests, a bot, same-process rejoin, chat
and DM6 -> DM2 were then revalidated through the real broker.

The development installation is registered at AWK's request. Distribution/Starter
updates and cross-city engine gameplay remain separate pending work. See
provenance/friends-engine-validation.json for sanitized evidence.

## 2026-09-16 — Friends beta distribution

AWK requested a new test package, GitHub push/release, refreshed installer and
retirement of the unused old Starter. OpenAI Codex prepared version 0.2.0-beta.1
and Starter 0.2.0, with an exact source commit, pinned engine download, upstream
notices and matching Friends dependency sources. The installer offers registration
only after the final directory exists; interactive choice or -RegisterLinks is
required. Package tests use nQuake's distributed KTX QVM as well as the development
DLL. Personal configs and commercial assets are excluded from release archives.
Final release verification is recorded separately under provenance.

Release verification: the published engine is built from commit 4e70d587.
The extracted Starter passed first/second launch, E1M1 save, all 170 Balanced
values, Quick crosshairs, bot arena and installer integrity checks. The packaged
KTX QVM also passed host/two-guest Friends gameplay, rejoin, DM6 -> DM2, URI
cold/warm start, confirmation and redaction. This remains local network-path
evidence through the real broker, not cross-city engine gameplay evidence.

Publication completed: engine v0.2.0-beta.1 and Starter v0.2.0 are public.
OpenAI Codex downloaded all six published assets, verified GitHub digests, ZIP
checksums and manifests, and installed the public Starter into a fresh directory
with an actual engine download. At AWK's request, obsolete Starter 0.1.0/0.1.1
release downloads were removed after verification; source tags and historical
engine beta remain. The development URI registration was restored after fixtures.
The general Linux CI failure also exists at the prior baseline and is recorded
as BUILD-LINUX-001; it does not describe the separately tested Windows binaries.
