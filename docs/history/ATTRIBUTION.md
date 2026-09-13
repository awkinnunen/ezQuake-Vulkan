# Attribution and change provenance

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
