# Tibazera Vulkan branch: task assessment

Historical snapshot archived by OpenAI Codex, 2026-09-14 (BUG-TRIAGE-001).
This preserves the prior assessment and completed work, including superseded
checkboxes. Use [TIBAZERA-TODO.md](TIBAZERA-TODO.md) for current work and
[BUGS.md](BUGS.md) for issue status. Historical wording is not proof of a current defect.

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


English assessment and translation of selected upstream notes: OpenAI Codex,
2026-09-13. Source revision: `91859a996daede0df166e2a55a5f08d56b052b21` from
`tibazera/ezquake-source`, branch `feature/sdl3-vulkan-pr`.

The source [CONTINUE.md](ezquake-vulkan/CONTINUE.md) is a Portuguese development
diary last updated on 2026-08-07. This is an edited English assessment, not a
complete translation. Older entries contain superseded findings and conflicting
diagnoses. See [ATTRIBUTION.md](ATTRIBUTION.md) for authorship. Upstream test
reports are not local verification; historical session instructions are not
project-wide policy.

## Explicit next task in the latest upstream entry

### Local gameplay work (LOCAL-008; separate from the upstream Vulkan TODO)

- [x] Update the local KTX game module to latest fetched master 1.48-dev at the
  user's explicit request; pin revision 631584f7 and retain its contributor history.
- [x] Adapt independent GPL QuakeC fixes for key-door audio, secret-door lethal
  activation and killtarget/target dispatch; exclude rerelease engine/assets.
- [x] Reproduce the three failures in pristine KTX, pass 19 corrected scenarios,
  build Debug/Release and run Vulkan/OpenGL native-module map tests.
- [x] Provide direct-link Release installation and rollback to the retained QVM.
- [ ] Broader gameplay regression coverage for the KTX 1.46-dev to 1.48-dev upgrade
  (bots, match modes, multiplayer and registered campaign maps).

See [QUAKEC-MERGE.md](QUAKEC-MERGE.md) for the exact scope and exclusions.

### Renderer work

- [ ] **Bindless world texturing.** The August 6–7 entry calls this task #4 and
  says it has not started. Scope: textured, lightmapped, alpha-textured, flat and
  overlay paths in `src/vk_world.c`, using the existing alias-model/texture bindless
  infrastructure. Fewer texture bindings are the intended benefit; performance
  needs measurement. The entry identifies descriptor-lifetime regressions as a risk.

## Source findings and current resolution

- [ ] **Bounded alias compatibility fallback:** the shader still requires bindless
  indexing. LOCAL-007 now rejects missing features/insufficient limits explicitly;
  a bounded-descriptor shader variant remains future work.
- [x] **Shader layout and device limits:** inherited world/sprite blocks of 176/160
  bytes are now 128/68 bytes. The actual local GPU limit is 128. Shared C/GLSL
  flags and layout checks fix caustics offsets; both 8192-entry sampler arrays
  are checked against the appropriate device limits before use.
- [ ] **Post-processing parity:** approximate FXAA, HDR/linear-light handling and
  3D/HUD framebuffer differences require a separate audit.
- [x] **Directional alias-model lighting:** normal-pass shader calculation matches
  the existing GLM formula in 511 GPU cases, preserving alpha and special passes.
- [x] **Skybox resolution assumptions:** bounds derive from actual face dimensions;
  42 GPU cases and mixed-size face loading passed.
- [ ] **Extended skybox seam inspection:** visually inspect all face edges/corners
  with real texture packs and moving views.
- [x] **NPOT capability reporting:** Vulkan initialization now calls the setter
  before shared texture/HUD initialization. 1,094 guarded mipmap cases passed.

## Historical reports: reproduce before treating as current bugs

- [x] **Local reproduction check: timedemo + vid_vsync 1.** MAINT-004 completes
  three 1,290-frame timedemos with FIFO on the development AMD GPU. Historical
  hang not reproduced; this does not establish a fix for every device/driver.
- [ ] **Black/untextured models or items on some machines.** Several explanations
  were attempted or rejected. Mipmap corruption is a hypothesis, not a locally
  established root cause. Current reproducibility is unknown.
- [ ] **Other visual differences:** lit water, skywind, muzzleflash interpolation,
  luma/lightmap combination and player outline colours. These are older audit
  candidates, not all confirmed defects in the current revision.

## Reported upstream as implemented or fixed

Newer entries report underwater caustics, world outlines, drawflat colours and
modes, Vulkan screenshot synchronization, an eyecandy-preset vid_restart hang,
GPU timestamps for timerefresh and dynamic-buffer memory selection. Treat these
as regression-test targets, not automatically missing features.
The later LOCAL-007 pass ran E1M1 and inspected rendered screenshots. This does
not validate every effect claimed in the upstream diary.

## Local progress and proposed order

- [x] Fetch the branch and configure the development worktree.
- [x] Build Windows x64 Debug with Vulkan enabled without source changes.
- [x] E1M1 with data and validation, screenshot, clean shutdown; Vulkan vid_restart.
- [x] Extend Vulkan runtime coverage to demo playback, seeking, three maps and video restart.
- [x] Reproduce and fix confirmed push-limit, screenshot usage and QTV shutdown issues.
- [ ] Reproduce the remaining historical timedemo and texture-lifetime reports.
- OpenGL/Vulkan visual parity deferred at the user's explicit request.
- [ ] Measure bindless changes and plan RTGL1 integration separately.

This ordering is Codex's proposal, not a commitment by tibazera.
The more detailed and dependency-ordered sequence is now in
[IMPLEMENTATION-PLAN.md](IMPLEMENTATION-PLAN.md), PLAN-001. It prioritizes a
working compatibility path and safe resource ownership before world bindless
expansion. Additional roadmap findings still require reproduction; the LOCAL-007 results
above identify the subset verified and fixed on the local GPU.

Sources: [PR #1145](https://github.com/QW-Group/ezquake-source/pull/1145) and
[pinned upstream diary](https://github.com/tibazera/ezquake-source/blob/91859a996daede0df166e2a55a5f08d56b052b21/CONTINUE.md).

## Local implementation pass — 2026-09-13 (supersedes corresponding source observations above)

- [x] Step 1: checked pipeline/device limits; world push block 128 bytes, sprite
  block 68 bytes; common world GLSL ABI fixes caustics flag disagreement.
- [x] Runtime prerequisites: valid screenshot image usage; QTV worker joined
  before freeing its data/mutex and shutting down the console.
- [x] Step 2: Vulkan NPOT reporting enabled; 1,094 guarded mipmap cases and map runtime passed.
- [x] Step 3: actual sky face dimensions; 42 GPU boundary cases and mixed-size sky runtime passed.
- [x] Step 4: normal-dependent alias lighting; 511 GPU comparisons against existing GLM and map runtime passed.

Step 1 gate passed: Debug build, limit boundary tests, 13 actual C/SPIR-V block
comparisons, 22 SPIR-V validations, E1M1 + screenshot + clean exit with validation.
Bindless alias fallback remains unimplemented; incompatible devices now fail
initialization explicitly. See DEVELOPMENT.md LOCAL-007 for evidence and scope.

## Local UI addition: CFG-001

- [x] List CFG files in the designated config directory and refresh with F5.
- [x] Preview keyboard/mouse bindings without executing the config.
- [x] Explicit load of the selected path, including same-name file handling.
- [x] Parser regressions, Vulkan Debug/Release, OpenGL and empty-directory tests.
- [x] English user documentation, launcher, attribution and ordered source patch.

See CONFIG-BROWSER.md. The diagram currently uses US engine-key positions;
physical Finnish-layout diagrams and evaluating included scripts are future
extensions, not part of the current preview contract.

## Local campaign repair: SP-001

- [x] Verify installed shareware progs.dat and all Episode 1 maps.
- [x] Preserve explicit single-player startup against nQuake KTX server settings.
- [x] Honor original QuakeC selection and avoid multiplayer .dat fallback.
- [x] Enable the load menu with original NetQuake data and fix -nohome save paths.
- [x] Provide a direct fullscreen campaign launcher and English documentation.
- [ ] Manual campaign playthrough: real exit triggers, difficulty selection,
  secrets, death/restart, boss and final episode transition.

Automated map/spawn/save tests are documented in VALIDATION.md; they do not
constitute a completed playthrough of the episode.

## Local Arena: ARENA-001

- [x] Main-menu Local Arena entry with the same Quake big font.
- [x] Installed-map picker, refresh, sorting, filtering and paging.
- [x] Start/restart/stop the embedded KTX listen server.
- [x] FFA, duel, 2on2 and Clan Arena mode actions.
- [x] Add/remove bots, new-bot skill, live count and Ready.
- [x] Exclude external connections and original single-player from bot actions.
- [ ] Future: team roster UI, per-bot controls and navigation-aware map filtering.
- [x] MAINT-002: replace separate screenshot acquisition with pre-present capture
      of the rendered image; immediate-restart Debug/Release tests pass.

Documentation and attribution: LOCAL-ARENA.md / ARENA-001.
- [x] Final Vulkan Debug/Release and Modern OpenGL integration evidence;
      seven-patch replay, English documentation and author/hash exports.

## ARENA-002 — Deathmatch and startup

- [x] Change the installed startup preference to the main menu; preserve other config bytes.
- [x] Add deathmatch 1–5 selection and live value display.
- [x] Start with selected rules automatically; FFA defaults to deathmatch 3.
- [x] Match the mode presets (FFA/Duel 3, 2on2 1, Clan Arena 5).
- [x] Debug/Release builds, real KTX rule-change regressions and layout review.

## Competitive Visuals: CV-001

- [x] Six-page Competitive Visuals menu, 53 controls, visual-only profiles, reset/compare.
- [x] World microcontrast reduction with broad texture patterns preserved.
- [x] Soft cel/gradient shading, silhouette controls and powerup-exclusive rim with ruleset gates.
- [x] Conservative bright-pixel bloom, world contact AO and existing projected-shadow controls.
- [x] Scene exposure/tone/sharpening before HUD, MSAA/FXAA/filtering controls.
- [x] Debug/Release, real menu/profile/restart tests and production C/GLSL GPU checks.
- [ ] Floating-point HDR and emissive bloom mask, full SSAO, dynamic shadow maps/budget, temporal facilities.
- [x] Recorded-motion, seek/restart and selected effect frame-time checks (MAINT-003/004).
- [ ] Broader player recognizability and gameplay assessment across maps and GPUs.

See COMPETITIVE-VISUALS.md for the user-requested scope and VALORANT references.

## Competitive Visuals widgets: CV-002

- [x] Give all strength/dimension settings visible sliders and binary settings On/Off switches.
- [x] Preserve discrete mode selectors with readable names where applicable.
- [x] Support scaled mouse dragging, clamping, keyboard stepping and page navigation.
- [x] Increase user crosshair size from 2 to 2.5 in active and reloadable configs.
- [x] Build Debug/Release and verify all 53 controls in an isolated Vulkan client.

## Graphics menu clarity and effect audit: CV-003

- [x] Separate general effects from competitive readability controls.
- [x] Remove duplicate menu rows while retaining old profile compatibility.
- [x] Add contextual help, dependency grouping and grey disabled controls.
- [x] Preserve the user's saved defaults and create recoverable snapshots.
- [x] CV-DEFAULT-001: adopt the later user-approved visual configuration as the project-default profile, including edge depth threshold 16.
- [x] INPUT-002: add a WASD alternative with shifted communication keys and Caps Lock jump-alias rebinding, retaining the ESDF profile.
- [x] Separate general rendering gates from the competitive master switch.
- [x] Explain pending video restarts and expose a Restart video action.
- [x] Add controlled rendered-image comparisons alongside control-value tests.
- [x] Verify all 48 menu settings with stable A/A/B/A rendered images (Debug validation and Release evidence).
- [x] MAINT-002: immediate swapchain-recreation captures pass without settling waits.

## Pre-publication pass: MAINT-001..004 / PUBLIC-001

- [x] MAINT-005: restore the 55 supported conditional settings and correct the missing-asset test fixture.
- [x] Fix the acquired-image screenshot lifecycle and compile Debug/Release.
- [x] Verify immediate/F5 restart captures, MSAA transitions and hardware clamping.
- [x] Exercise three maps, demo seek/pause, skin reload, video restart and KTX entity join/leave.
- [x] Measure selected effect costs with fixed Release timedemos and a closing baseline.
- [x] Retain the approved 53 graphics values and export the portable WASD module.
- [x] Verify the actual combined profile load, saved values and movement-dependent jump binding.
- [x] Verify the 13 source patches at the pre-publication pass; the later FX-001 patch brings the verified sequence to 14.
- [x] Complete the first public GitHub upload with portable cfg files; published main verified at bcace078b8cdbd8212bb9b16eb141f2ea9537d07 (PUBLIC-002).

## FX-001 - Explosion style (2026-09-13)

- [x] Add Explosion without the ring while keeping Big explosion unchanged.
- [x] Preserve existing config values and support the new value in visual profiles.
- [x] Rebuild Debug and Release. Runtime validation is recorded in VALIDATION.md.

## RT-AUDIT-001 / RT-PLAN-002 - Direct RTGL1 integration

- [x] Recheck integration feasibility and local device support; see RTX-FEASIBILITY.md.
- [x] Write the user-requested detailed plan for broad effects migration, allowing a distinct RT appearance.
- [ ] RT-00: inventory host/donor features, pin runtime/ABI and implement device diagnostics.
- [ ] RT-01: validate the SDL3 triangle/light harness on the RTX 3060.
- [ ] RT-02: integrate backend lifecycle, 2D rendering and raster recovery.
- [ ] RT-03: submit static BSP, materials, sky and map lights.
- [ ] RT-04: add moving models, skins, viewmodel and transient lights.
- [ ] RT-05: adapt all host/donor effects, particles, water, powerups and HUD details.
- [ ] RT-06: extend RTGL1 for Competitive Visuals and shared controls; add RT profiles/menus.
- [ ] RT-07: validate temporal resets, QW/demos, capture and multiview handling.
- [ ] RT-08: measure RTX 3060 performance/memory and package the optional runtime.

Implementation update: RT-00 requirements/package/API checks and the RT-01
standalone test harness are implemented. Central RTGL1 initialization cleanup
from RT-02 is also implemented. The RTX 3060 is unavailable; the user will perform
the first GPU test. Semantic inventory mapping and host integration remain open.
See [RT-IMPLEMENTATION-STATUS.md](RT-IMPLEMENTATION-STATUS.md) for exact status.

Architecture, scope and acceptance gates: [RT-IMPLEMENTATION-PLAN.md](RT-IMPLEMENTATION-PLAN.md).
All implementation stages remain open. Earlier RT numbering is superseded.
All requested effects remain inventoried; missing features must stay visible as
open work. No raster appearance-matching pass is planned.

### RT CPU preparation follow-up

- [x] RT-GEOMETRY-001: checked fan/strip triangles, affine matrices, pose interpolation,
  bounded ID packing and scene/overlay submission, tested in Debug and Release.
- [ ] Connect the tested helpers to actual BSP/model/texture/particle/HUD callbacks.
- [ ] Add host entity lifetime tracking and renderer lifecycle/switch fallback.
- [ ] Complete RT feature mapping, donor frame effects, shader extensions and profiles.
- [ ] User-run RTX 3060 triangle/resize/shutdown test; hardware is not currently available.

This completes no additional GPU milestone. See RT-IMPLEMENTATION-STATUS.md for
all seven requested steps and their remaining work.

### PERF-COMPARE-001 (2026-09-14)

- [x] Compare original nQuake ezQuake OpenGL and project Vulkan with matched
  shared high-effects settings at 720p/1080p; include same-project OpenGL control
  and a world-outline-off pair. See RENDERER-PERFORMANCE.md.
- [x] PERF-COMPARE-002: isolate MSAA/FXAA and profile GPU stages plus CPU waits.
  The 1080p deficit is concentrated in the MSAA path. See AA-DIAGNOSTICS.md.
- [x] PERF-OPT-001: defer the single-view main pass until after world normals/AO.
  Release/Debug updated; exact pixels in 13 configurations, AA/GPU measurements,
  motion/restart validation and 18-patch replay pass. See SCENE-PASS-OPTIMIZATION.md.
- [ ] PERF-OPT-002: audit terminal/resumed MSAA color and depth store/load operations
  after pass restructuring; preserve attachments needed by later rendering.
- [x] PERF-COMPARE-003: rerun original ezQuake / optimized Vulkan at 720p/1080p,
  same-executable OpenGL controls at both resolutions, world-outline-off pair and
  closing 1080p pair. Full shared effects are now near original throughput; the
  outline-off Vulkan path still trails by 16.8%. See RENDERER-PERFORMANCE-OPTIMIZED.md.
- [ ] PERF-INVESTIGATE-003: isolate the remaining 720p world-outline-off deficit
  with matched AA settings; this path bypasses the pass split removed by PERF-OPT-001.
  The new comparison establishes the deficit, not its root cause.
- [ ] MULTIVIEW-001: fix descriptors updated/destroyed while still referenced by
  a command buffer during multi-player demo views. The extended PERF-OPT-001
  fixture reproduces this in the retained pre-optimization binary; it is not a
  passing regression test or part of the single-view optimization.

RT development remains paused pending a suitable development GPU, as requested
by the user. The reported first RTX test stopped after GPU detection; no successful
RT image is established by that attempt or these raster benchmarks.
