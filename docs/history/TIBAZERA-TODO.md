# ezQuake Vulkan: current TODO

Updated and curated by OpenAI Codex, 2026-09-15, at the user's request
(BUG-TRIAGE-001). This is the active work list. [BUGS.md](BUGS.md) records evidence,
reproduction conditions and practical impact. The prior chronological task list,
upstream assessment and completed checkboxes are preserved in
[TIBAZERA-TODO-HISTORY.md](TIBAZERA-TODO-HISTORY.md).

An unchecked feature or test below is not itself a known bug. Priorities reflect
the current raster-focused work; RT remains paused at the user's request.

## Next raster work

- [x] SHADOW-003: fix DM6 Baked with shadows darkening as the player moves
  with larger Map light radius. Preserve baked world occlusion; add only entity
  occlusion in mode 1. Dynamic lights and Realtime mode retain world casters.
  See DYNAMIC-SHADOWS.md and provenance/shadow3-validation.json.
- [ ] PERF-INVESTIGATE-003: profile the remaining outline-off Vulkan deficit.
  At 720p, shared high effects/MSAA 4x/FXAA 5 give 485.2 FPS versus original
  ezQuake's 583.4 FPS (-16.8%). Establish the responsible stages before changing
  code. This is measured throughput, not a rendering-correctness failure.
- [x] PERF-OPT-002: discard terminal single-view offscreen MSAA color stores and
  unused main depth stores. Preserve MSAA color for resumed/multiview and direct
  cross-frame LOAD paths. Evidence and measured benefit: [RASTER-FEATURES.md](RASTER-FEATURES.md).
- [x] MULTIVIEW-001: fix descriptor lifetime, per-view buffers/atlases and
  camera viewports; capture every camera. The 0/2/4/0 MVD sequence passes with
  HDR, MSAA, SSAO, bloom and shadows enabled. See DYNAMIC-SHADOWS.md and
  provenance/shadow2-validation.json; BUGS.md retains the original failure.

## Reproduction and issue triage

These are investigations, not a claim that ordinary gameplay is broken.

- [ ] STARTUP-001: retain two independent unexplained pre-log startup failures
  (KTX installation and CV fixture); collect exit/crash details if either recurs.
  Both had successful retries; do not assume a common root cause.
- [ ] CAPTURE-001: retry the old pre-frame screenshot layout diagnostic on current
  source before prescribing another screenshot fix. Later screenshot/lifecycle
  fixes may supersede this historical observation.
- [ ] LIGHTMAP-001: determine whether cold-camera stale lightmaps with r_dynamic 0
  are a renderer issue or fixture setup artifact. Stable final fixtures match
  exactly; do not label this a current normal-play regression.
- [ ] ASSET-001: identify the incompatible DM4/DM6 .lit sources and map pairing.
  Preserve installed nQuake/user resources; no replacement is authorized by this
  diagnostic entry. Visible impact has not been isolated.
- [ ] HIST-TEXTURE-001: seek a current device/config/map reproduction for upstream
  black/untextured model reports. No local reproduction is established.
- HIST-VSYNC-001: local FIFO/timedemo check passed. Monitor only; reopen with a
  concrete failing device/demo/log rather than repeatedly testing an old rumor.
- HIST-VISUAL-001: old water/skywind/muzzleflash/luma/outline differences are
  reference observations. OpenGL/Vulkan appearance matching is deferred by the user.

## Menu consistency and usability (MENU-UNIFY-001)

Requested by the user, 2026-09-14; task breakdown by OpenAI Codex. Planned work,
not implemented behavior. Coordinate this with CFG-PRESETS-001 below.

- [ ] Inventory original ezQuake, Competitive Visuals, new graphics, Local Arena
  and KTX menus; propose one navigation structure with clear ownership of each
  setting/action and remove redundant routes where they add confusion.
- [ ] Use shared menu widgets, fonts, spacing, selection/focus styling, sliders,
  toggles and keyboard/mouse navigation. New graphics pages should look and behave
  like the rest of the game menus; retain the established Quake visual character.
- [ ] Integrate relevant KTX match/mode/bot controls into native game menus and
  assign them by lifecycle: Local Arena owns preparing and starting a new game
  (map, mode, initial rules and starting bot setup); the in-game menu owns actions
  on the running match (including adding/removing bots and other available match
  management). Local Arena must not remain the destination for running-match
  administration. Reflect actual server state and distinguish local
  setup from actions available on a connected server; preserve underlying KTX
  behavior and account for server capabilities rather than assuming all servers
  support the same controls.
- [ ] In-game Escape menu: place running-match bot management in this menu,
  without routing through the main menu or Local Arena. User example: adding a
  bot currently requires that detour. Startup and in-game menus have distinct
  responsibilities but shared styling/widgets. Reuse the underlying bot actions
  and preserve the active match. Acceptance: while playing, Escape -> Bots ->
  Add bot -> return to play, with no Local Arena/main-menu visit or match restart.
- [ ] Apply consistent contextual help, units/ranges, dependency grouping,
  disabled-state explanations and restart indicators across all settings pages.
  Show changed values and consistent back/apply/reset actions where appropriate.
- [ ] Review common flows with keyboard and mouse: find an effect, change a
  binding, choose a game mode, add bots and return to play. Check readable layouts
  at multiple resolutions/UI scales and preserve existing config/console access.

## Menu-aligned config presets (CFG-PRESETS-001)

Design direction: a managed CFG file represents a named preset for a settings
category; menu categories and preset categories share the same ownership map.
Provisional groups are Graphics (including Competitive Visuals), Controls, HUD,
Audio and Local Game/KTX. Final grouping follows the agreed menu structure;
not every submenu needs a separate file.

- [ ] Define category ownership and a directory/naming convention. Category
  presets must only change their own settings: selecting graphics must retain
  bindings, communication aliases, sound and game rules. Allow a complete profile
  to compose selected category presets with documented load order/precedence.
- [ ] Add the same preset browser to each relevant menu: browse named CFG presets,
  show current selection and unsaved modifications, Load, Save, Save As and Reset.
  Save only the owning category; preserve shipped presets and existing user files
  when creating variants. Define startup and save-on-exit behavior for mixed presets.
- [ ] Support live browsing/preview in the game scene or suitable category preview,
  with Apply/Cancel restoring the exact pre-preview category state. Switching
  between presets must start from a defined category baseline so omitted settings
  do not leak from the previous preview. Display effective changes and pending
  video restarts; do not restart the renderer on every selection movement.
- [ ] Separate declarative managed presets from legacy CFG scripts. Keep existing
  explicit full-config loading/import, ESDF/WASD profiles, aliases and includes
  compatible. Do not execute arbitrary aliases, connection/map commands or KTX
  match actions merely by highlighting a preset. Preview game-rule values locally;
  server-changing actions retain an explicit apply/start action.
- [ ] Verify category isolation, live switching/cancel, load/save round trips,
  composed profile startup, legacy import, restart-required values and unsaved
  changes. Add a migration plan with recoverable originals before changing the
  installed config layout or save-on-exit behavior.

The existing CFG browser previews literal key bindings and loads full configs;
it does not yet implement live preset application or category-only saving.
See [CONFIG-BROWSER.md](CONFIG-BROWSER.md) for current behavior.

## Renderer and other UI features

- [ ] WORLD-BINDLESS: extend alias/texture bindless infrastructure to textured,
  lightmapped, alpha, flat and overlay world paths. Check descriptor lifetime
  and benchmark benefit; this is not a demonstrated current bug.
- [ ] ALIAS-FALLBACK: bounded-descriptor model shader for devices missing required
  indexing features/limits. Current behavior is an explicit initialization error.
- [x] HDR-001: optional RGBA16F linear scene and separate emission target, manual
  exposure/tone mapping to SDR, bright/emissive bloom selection, capability fallback
  and visible restart state. Project profiles enabled at the user's request;
  existing strengths and bindings retained. See RASTER-FEATURES.md.
- [x] SSAO-001: world-position reconstruction, 8–32 hemisphere samples, radius/bias/
  strength controls and moving BSP transforms; retain legacy Contact mode.
- [ ] BLOOM-PYRAMID: replace the current small 25-tap bloom kernel with a
  downsample/upsample chain if a wider glow is wanted; benchmark first.
- [ ] AO-EXTEND: consider half-resolution AO with depth-aware filtering and model
  normals. Current SSAO covers world/BSP surfaces at full resolution; models,
  weapons, sky and transparent surfaces are excluded. CACAO remains optional.
- [ ] HDR-OUTPUT: HDR10/scRGB monitor output and automatic exposure are separate
  future features; the implemented HDR internal buffer outputs ordinary SDR.
- [x] SHADOW-001: dynamic point-light raster shadow maps, world/brush/alias
  casters, live controls and bounded per-frame light/caster budget. See
  [DYNAMIC-SHADOWS.md](DYNAMIC-SHADOWS.md).
- [x] SHADOW-OPT / SHADOW-002: active-light GPU profiling, shadow-off shader
  specialization, cached world bounds/depth maps, face/cone culling and bounded
  round-robin updates; maximum eight lights / 8192 casters.
- [x] SHADOW-EXTEND / SHADOW-002: BSP and additive authored lights, target-based
  spotlights, baked-shadow and realtime replacement modes, thirteen live controls.
  Transparent transmission and reconstruction of baked indirect light remain
  outside the raster-shadow approximation; see DYNAMIC-SHADOWS.md.
- [ ] TEMPORAL: establish history/reset handling before temporal visual effects.
- [ ] SHAFT-UI: dedicated lightning color, size and sparks menu controls.
- [ ] CFG-PREVIEW: optional Finnish physical-key layout and include-script handling.
  The current browser intentionally previews literal bindings without execution.
- [ ] ARENA-UI: team rosters, per-bot controls and navigation-aware map filtering.

Detailed design: [IMPLEMENTATION-PLAN.md](IMPLEMENTATION-PLAN.md). Old observations
about oversized shader blocks, missing NPOT reporting or screenshot fixes in that
plan are superseded by completed work; use this list and BUGS.md for current status.

## Validation coverage still missing

- [ ] Full manual Episode 1 playthrough: exits, difficulty, secrets, death/restart,
  boss and episode transition. Existing spawn/save tests do not cover all of this.
- [ ] Broader KTX 1.48-dev modes, bots, registered maps and remote/LAN regression tests.
- [ ] Moving-view skybox edge/corner inspection with real mixed-resolution packs.
- [ ] Player recognizability/gameplay assessment across maps and GPUs.
- [ ] Supported-device fault testing of RT component constructors and host fallback
  when RT work resumes. Central cleanup alone does not establish complete safety.

## RT: paused until a suitable development GPU is available

- [ ] RT-STARTUP-001 / RT-01: diagnose the user-reported stop after GPU detection
  and obtain the first actual triangle/light image, resize and clean shutdown.
- [ ] RT-00: complete semantic feature mapping. Requirements, package/API checks,
  device diagnostics and a standalone harness already exist.
- [ ] RT-02: host backend lifecycle, 2D/HUD callbacks and renderer-switch fallback.
- [ ] RT-03: static BSP, materials, sky and map lights.
- [ ] RT-04: moving models, skins, viewmodel, transient lights and entity lifetimes.
- [ ] RT-05: connect all inventoried effects, particles, water and powerup behavior.
- [ ] RT-06: RTGL1 shader/API extensions, Competitive Visuals and RT profiles/menus.
- [ ] RT-07: temporal resets, QuakeWorld/demo/capture and multiview validation.
- [ ] RT-08: RTX 3060 performance/memory measurements and optional runtime packaging.

CPU geometry helpers pass but are not connected to the host scene callbacks.
There is no playable RT renderer yet. See
[RT-IMPLEMENTATION-STATUS.md](RT-IMPLEMENTATION-STATUS.md) and
[RT-IMPLEMENTATION-PLAN.md](RT-IMPLEMENTATION-PLAN.md).

## Recently completed

- [x] PERF-OPT-001: deferred single-view scene pass; normal Debug/Release rebuilt,
  13 exact-image comparisons and motion/restart/GPU checks completed.
- [x] PERF-COMPARE-003: fresh original/OpenGL/Vulkan comparison. Full shared effects
  are now near original throughput; outline-off deficit is tracked above.
- [x] BUG-TRIAGE-001: consolidate current tasks and issue evidence; rerun a focused
  0 -> 2 -> 4 -> 0 multiview sequence on current Release. Capturing the defect is
  historical failing evidence; SHADOW-002 adds the passing multiview regression.

Earlier completed engine, menu, gameplay, config and public-source work is in
the history archive and [DEVELOPMENT.md](DEVELOPMENT.md). Current performance:
[RENDERER-PERFORMANCE-OPTIMIZED.md](RENDERER-PERFORMANCE-OPTIMIZED.md).
