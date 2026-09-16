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

## Unified menus and graphics presets

MENU-UNIFY-001 / CFG-PRESETS-001 / INPUT-003 implemented on 2026-09-15 by
OpenAI Codex at the user's request. See [UNIFIED-MENUS.md](UNIFIED-MENUS.md).

- [x] Native graphics widgets and contextual help; seven categories integrate
  original ezQuake controls, Competitive Visuals and new Vulkan effects.
- [x] Remove duplicate image controls from System. Show disabled dependencies
  and pending video changes; make legacy menu commands open the native Graphics UI.
- [x] Graphics CFG list, live highlight preview, Apply/Cancel, category scope,
  factory reset preview, Save and Save As with recoverable backups.
- [x] Validate declarative presets before applying; never execute scripts,
  bindings, map changes or server commands while previewing graphics.
- [x] Complete Balanced, Ultra competitive and Athmospheric presets. Balanced
  preserves the user's 170 current graphics values; alternatives are Codex tuning.
- [x] WASD default and SDFE alternative in Controls, including the user's weapon
  aliases and nQuake communication keys, independently of graphics selection.
- [x] Native Local Arena owns new-game setup/start; native in-game Bots handles
  current-match requests without leaving for the main menu. KTX availability uses
  server-advertised capability and works over an ordinary remote client connection.
- [x] Preserve explicit legacy full-config import and key-map preview.
- [ ] Extend category preset editing to HUD/audio and richer control profiles.
  Those categories currently retain their existing settings and full-config saving.
- [ ] Per-bot/team rosters, navigation-aware map filtering, more KTX administration.
- [ ] Broader manual keyboard/mouse usability sessions, UI scales/localization and
  additional third-party server policies. Automated cases do not cover every flow.

Graphics presets live under ezquake/presets/graphics; shipped files are read-only
in its builtin subdirectory. Old competitive-only profiles remain importable.
The full-config browser remains explicit and does not execute includes for preview.

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
- [x] SHAFT-UI: lightning colour, size and sparks controls are under Graphics / Particles and effects.
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

- [x] CFG-PRESETS-002: load the focused graphics preset with Enter/click; F3 keeps preview, Escape cancels. Native input and save regression verified in Debug/Release.

- [ ] CFG-PERSIST-001: investigate user-reported world-edge slider changes missing from saved config; isolated native keyboard/mouse, F5 and quit-save checks pass. Capture the failing interaction sequence.

- [x] CFG-BALANCED-002: refresh Balanced from the user configuration saved at 20:34; four graphics changes, all 170 values verified by Release preset activation tests.

- [x] INPUT-004: nQuake keyboard preset plus Quick WASD/Quick ESDF names; native menu and switch-isolation tests pass in Debug/Release.

- [x] DIST-001: Windows x64 test package, config-preserving first-run defaults, diagnostics, dependency notices and exact source identity.

- [x] DIST-002: source-only Windows Starter with nQuake downloads, hash checking, isolated install, optional full-game PAK import and first-run presets.

- [x] INPUT-005: include the original five legacy crosshairs in Quick profiles and Starter; provide a small update ZIP for existing installs.

- [x] DIST-003: remove the unneeded crosshair patch download and old-release upgrade directions before sharing Starter.


## Friends hosting through existing infrastructure (2026-09-16)

- [x] FRIENDS-001: investigate FTE rooms, ICE, STUN, relay selection, public room
  visibility and current QW proxy boundaries. Record the UX and technical plan in
  [FRIENDS-HOSTING-PLAN.md](FRIENDS-HOSTING-PLAN.md). Research only; no engine changes.
- [x] FRIENDS-002A: implement the native FTE-broker / ICE / pinned-DTLS prototype;
  Debug/Release, real-broker local pair, wrong-key and wrong-fingerprint tests pass.
- [x] FRIENDS-002B: package a standalone Windows two-computer test with matching
  source, dependency notices, Host/Join launchers and sanitized results.
- [x] FRIENDS-002: user supplied a successful direct probe result and confirmed
  the computers were in different cities (100/100 packets, mean RTT 31.7121 ms).
  This is one network pair; reversed roles, more NAT types and TURN remain open.
- [x] FRIENDS-003: logical peer addresses integrated with embedded QW/KTX;
  two simultaneous guests, rejoin, chat and DM6 -> DM2 sign-on tested.
- [x] FRIENDS-004: pinned encrypted admission, close/reopen, secret rotation,
  guest removal and private-server UDP/TCP gate. Latest scope uses full links;
  code-only host approval is deferred to keep the first workflow simple.
- [x] FRIENDS-005A: Local Arena access selector, main/in-game Friends menus,
  clipboard joining with confirmation, help text and unavailable-action policies.
- [x] FRIENDS-005B: opt-in per-user Windows URI registration, native cold start and
  profile-specific running-instance handoff, with confirmation and secret redaction.
- [x] FRIENDS-005C: Starter offers optional per-user link registration after installing
  the final executable/data directory; in-game registration remains available.
- [ ] FRIENDS-006: validate outages, lifecycle and eligible existing TURN fallback.
- [x] FRIENDS-007: 0.2.0-beta.1 engine/source packages and Starter 0.2.0, with
  packaged KTX QVM multiplayer, clean installation and invitation-link tests.
  Cross-city gameplay remains an explicit external test gate.

No new external relay is part of this scope. Existing STUN/broker reachability is
confirmed; cross-city engine gameplay and relay availability remain unverified. Room
codes may be public and must not act as passwords. See the plan's acceptance gates.

Prototype details and pending gates: [Friends hosting status](FRIENDS-HOSTING-STATUS.md).

- [x] FRIENDS-002C: unlimited host waiting in probe 0.1.1, periodic broker keepalive,
  stable invitation, reduced idle polling and retained explicit test deadlines.

- [x] FRIENDS-004P: generate and privately save one reusable invitation and host
  identity; preserve it across restarts, matches and upgrades. Change invitation
  replaces the access secret and invalidates old links. No MAC or rotation counter.
  Handle broker room conflicts without promising offline name reservation.

Current player instructions: [Friends games](../FRIENDS.md).
- [ ] FRIENDS-VOICE: validate inherited ezQuake VoIP through Friends using two microphones; no new voice implementation is needed before this check.

## Native Unix distribution (2026-09-16/17)

Requested by AWK; implementation and automated validation by OpenAI Codex.

- [x] PORT-UNIX-001: native Linux/macOS Friends broker, wake events, private saved
  identities and invitation handoff. Preserve the Windows version-1 wire format;
  Windows/Linux 100-packet exchanges pass in both host directions.
- [x] PORT-UNIX-002: C99 build repair, portable CMake resources and system-library
  compatibility. Mac uses Vulkan portability enumeration/MoltenVK argument
  buffers and SDL3 relative mouse input.
- [x] DIST-UNIX-001: native config-preserving nQuake installer, pinned downloads,
  optional owned PAK import, per-user launchers and invitation registration.
  No Python dependency on the player's machine.
- [x] DIST-UNIX-002: target-OS CI and packaging recipes for Linux x86_64 and macOS
  arm64/x64; relocated runtimes, exact sources, notices and SHA-256 sidecars.
- [ ] PORT-UNIX-HW: friend tests real Mac Vulkan output, presets, mouse/audio,
  cold/warm GUI invitation dispatch and both directions of cross-city gameplay.
  Linux GPU/desktop diversity and long sessions also need tester coverage.
- [ ] DIST-MAC-SIGN: Developer ID signing/notarization for a future wider Mac
  release; current tester packages are explicitly ad-hoc signed and unnotarized.

See [Unix port instructions and evidence](../UNIX-PORT.md). Successful builds or
local/container tests do not close the external hardware/network gates.

- [x] DIST-UNIX-003: publish v0.3.0-beta.1 with Linux x86_64, Mac arm64/x64,
  native installers, matching sources and hashes. All three public runtime
  downloads independently verified; retain PORT-UNIX-HW and UNIX-GAMEPLAY-001.
