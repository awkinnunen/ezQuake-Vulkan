# ezQuake Vulkan: bugs and unresolved observations

BUG-TRIAGE-001. Curated and tested by OpenAI Codex, 2026-09-14, at the user's
request; updated by SHADOW-002 on 2026-09-15. Scope: findings recorded in the workspace's validation/development
reports, current TODO, performance reports and user reports. This is an evidence
index, not proof that every possible defect has been discovered. The initial
audit changed no engine/config code; subsequent fixes are identified below.

## How to read this list

- **Confirmed:** a concrete violation has been observed; visible impact is stated
  separately. Validation messages alone do not establish a crash or broken image.
- **Reported:** a user observed a failure; local reproduction/root cause is pending.
- **Unresolved observation:** retained evidence exists but current reproducibility
  or attribution to engine code is not established.
- **Historical / monitor:** upstream or old fixture evidence; not a confirmed
  current defect. Successful retries are not retroactive fixes.

There is no established general blocker for ordinary single-view raster play in
these records. MULTIVIEW-001 was reproducible in multi-camera demos and is fixed by SHADOW-002.
The reported RT startup failure blocks the separate experimental RT test. These
statements do not establish long-session or all-device correctness.

## Active issue index

Subsequent RASTER-001 checks (OpenAI Codex, 2026-09-14): HDR/SSAO single-view
fixtures, moving demos, resize and F5 restarts passed without validation errors.
The extended F5 fixture initially hit its old 45-second timeout; the completed
rerun passed with the longer fixture budget. This is not a new startup defect.
Those initial checks closed no existing issue. SHADOW-002 later fixes MULTIVIEW-001. See
[RASTER-FEATURES.md](RASTER-FEATURES.md) for scope and test limitations.

| ID | Classification | Observed impact | Current reproduction / next action |
|---|---|---|---|
| BUILD-LINUX-001 | Fixed by Unix port | C90 flags rejected C99 loop declarations in `src/menu.c` | CMake now selects GNU C99; native Ubuntu 24.04 CI build, Friends unit and packaged-installer checks pass. Matrix jobs no longer cancel each other on failure |
| UNIX-REFRESH-001 | Fixed by Unix port | An unknown/zero monitor refresh rate could prevent menu frames and deferred startup commands | `CL_MinFrameTime` now bounds the refresh-derived limit to at least 30 FPS; Linux Xvfb/lavapipe reproducer now initializes and runs the local arena |
| UNIX-GAMEPLAY-001 | Unresolved test observation | Three simultaneous software-Vulkan engine instances did not complete the Linux gameplay fixture; timeouts and broker/peer failures occurred under heavy CPU load | Local Arena, bot, URI confirmation and one guest spawning were observed. Separate native two-guest transport and Windows/Linux exchanges pass. Retry on hardware Vulkan and isolate any recurring network failure; do not claim a full Linux gameplay pass |
| SHADOW-003 | Fixed; user-reported DM6 lighting instability | Baked with shadows could darken whole rooms while moving; larger Map light radius magnified it | Exclude already baked world occluders in mode 1; retain entity shadows. E1M1/DM6 image regression in shadow3 validation |
| MULTIVIEW-001 | Fixed by SHADOW-002 | Per-frame sky descriptor update and isolated per-view buffers/uniforms; actual viewports and full multiview captures | 0/2/4/0 MVD sequence, HDR/MSAA/SSAO/bloom/shadows, no VUIDs; see shadow2 validation |
| RT-STARTUP-001 | User-reported RT blocker; paused | RT test stops after GPU detection, before a usable log/image | Await supported development GPU; capture initialization failure |
| STARTUP-001 | Two unresolved startup observations | Individual pre-log launch failures; retries passed | No reliable reproduction; collect diagnostics on recurrence |
| CAPTURE-001 | Historical validation observation | Forced pre-frame screenshots logged image-layout errors | Recheck current code; later screenshot fixes may supersede it |
| LIGHTMAP-001 | Fixture/renderer attribution unresolved | Cold-camera comparison images had stale/missing lightmap regions | Controlled camera preparation yields exact matches; isolate a minimal case |
| ASSET-001 | Observed installed-data warnings | DM4/DM6 .lit size mismatches | Identify map/light-file pairing; visible impact unmeasured |

## MULTIVIEW-001 — Fixed; historical descriptor-lifetime evidence

SHADOW-002 fixes the repeat sky-descriptor update in VK_RenderView and separates
view-specific GPU data. It also applies the engine viewports and captures all cameras.
Current evidence: provenance/shadow2-validation.json. The following records the
original failure, not the current status.

**Affected situation:** Vulkan, a multi-player MVD recording, with
`cl_multiview 2` or `cl_multiview 4` displaying multiple player cameras in one
window. This is unrelated to simply having several players in an ordinary game.
The tested single-camera controls use `cl_multiview 0`.

**Original failure evidence:** `cache/runtime-multiview-triage-release2/triage-results.json`
and its `qw/qconsole.log`. Historical Release executable SHA-256:
`e62ee9ca693f130bb77962ee43de6c0f226faca71882a6f89b8829b02d13cf07`.
Local AMD integrated Radeon, Vulkan 1.2.188, 800x600 window, MSAA 4x, FXAA 5,
world outlines 3, Competitive Visuals/AO/bloom disabled. Vulkan validation enabled.
The private DM6 MVD contains six players. There is no frozen camera or synthetic
photo actor injection in this new fixture.

| Stage | VUID messages | Descriptor invalidation | Screenshot |
|---|---:|---|---|
| Initial single view (0) | 0 | No | ezquake000.png |
| Two views (2) | 132 | Yes | ezquake001.png |
| Four views (4) | 8 | Yes | ezquake002.png |
| Return to single view (0) | 0 | No | ezquake003.png |

Counts describe this one capture, not 140 independent bugs. Several commands
report the same invalidated object. The message is:
`VkDescriptorSet ... was destroyed or updated without UPDATE_AFTER_BIND`.
Subsequent bind/draw/end commands reject the invalid recording state.
The update site was subsequently identified in VK_RenderView: it reset the sky-descriptor guard on every view rather than every fenced frame.
Four views were tested after two views, not in an independent fresh process.

**Practical impact observed:** both multi-view screenshots contain rendered scenes
and view dividers; the fixture reaches completion and the executable exits 0.
No crash, device loss or independently established visual corruption occurred in
this run. The screenshots are not a pixel-correctness comparison with OpenGL.
A driver continuing to render does not make the Vulkan resource usage valid;
flickering, corruption or failures on another driver are possible consequences,
not outcomes established here. Normal one-view gameplay has not been shown to be
affected by this defect. Avoiding multi-camera mode avoids this measured trigger.

**Reproduce locally:** run `scripts/Test-MultiviewTriage.ps1 -Label <fresh-label>`
from the development workspace. It links assets into an isolated profile and
uses the local private MVD; it does not alter normal settings. It plays the demo,
sets 0, 2, 4, then 0, waits 120 frames per stage and captures images. Test-only
checkpoints prevent the inherited script-buffer guard from terminating long
automation; they do not modify rendering. `triage-results.json` records the
validation failure even when evidence collection finishes successfully.

For manual investigation, play a multi-player MVD in the Vulkan client and change
`cl_multiview` to 2 or 4; use 0 to return. Start with `-dev` to obtain Vulkan
validation diagnostics, with validation layers installed. This manual recipe is
the trigger family; the automated local fixture is the verified reproduction.
No claim is made that every MVD/settings combination fails or shows visible damage.

**Earlier evidence:** `cache/runtime-opt-images-before` also logged this violation
in the retained pre-PERF-OPT-001 binary. Its multi-view tail followed photo/restart
tests and screenshots had the main menu covering the scene. It established API
errors, not a demonstrated visible multi-view rendering failure. The new focused
fixture narrows that evidence. The first new attempt,
`runtime-multiview-triage-release`, hit the test script's runaway guard in stage 0
and timed out; it is an incomplete fixture, not a reproduced renderer hang.

## RT-STARTUP-001 — Experimental RT test stops after GPU detection

Source: the user's first RTX test report. The program detected the GPU and then
stopped before producing a usable log. A crash, blocked call or initialization
failure cannot be distinguished from the available evidence. The intended test
device is a GeForce RTX 3060; no successful RT image is recorded. This affects the
separate RT harness, not the normal Vulkan raster executable. Work is paused at
the user's request until a suitable development GPU is available. See
[RT-IMPLEMENTATION-STATUS.md](RT-IMPLEMENTATION-STATUS.md).

## STARTUP-001 — Unexplained one-off pre-log failures

Two separate observations are grouped for tracking, not asserted to share a cause:

- `cache/ktx-installed-runtime.log`: first installed KTX test stopped in an Error
  window before qconsole.log and was terminated by a 45-second fixture timeout.
  `ktx-installed-retry-runtime.log` passed with the same KTX DLL and a fresh profile.
- `cache/runtime-cv-final-release`: one launch exited before qconsole.log, with
  empty stderr. `runtime-cv-final-release2` passed with the unchanged executable.

No current repeated startup failure or ordinary-play symptom is established.
On recurrence, preserve exit code, dialog/crash details, executable hash and
profile before retrying. These are not evidence that the KTX DLL caused both
failures. Sources: [VALIDATION.md](VALIDATION.md), KTX and CV-001 sections.

## CAPTURE-001 — Historical pre-frame screenshot image-layout diagnostic

`cache/runtime-arena-first` and `runtime-arena-second` emitted
`VUID-vkCmdDraw-None-09600` for forced pre-frame startup screenshots. Initialized
final Arena runs passed. Subsequent offscreen-layout and MAINT-002 screenshot
lifecycle fixes, plus immediate-restart screenshot checks, also passed.
The exact old pre-frame trigger has not been rerun on the current executable.
Keep this as historical/recheck-needed, not a confirmed current screenshot bug
or a claim that the normal screenshot key fails. Do not count it as fixed without
replaying the original trigger or establishing which fix covers it.

## LIGHTMAP-001 — Cold-camera lightmap content during fixture calibration

Evidence: early PERF-OPT-001 image comparisons, including
`cache/runtime-opt-images-stable-before` and `runtime-opt-images-stable-after`.
The artificial setup disabled dynamic lighting and moved the actual player and
test camera; early images showed stale/missing lightmap regions. Random spawns
also contaminated earlier comparisons. Fixing position and allowing camera
resources to settle before restart produced 13 exactly matching final cases in
`runtime-opt-images-ready-before/after`. No production lightmap fix was made.
Whether any remaining cold-start effect is an engine bug or fixture artifact is
unresolved. There is no demonstrated normal-play lighting failure from this work.

## ASSET-001 — Installed map/light-file mismatches

`cache/runtime-arena-final-release/qw/qconsole.log` reports incorrect sizes for
`dm4.lit` and `dm6.lit`; the initialized Arena fixture still completes. These are
installed-data compatibility warnings, not proof of a Vulkan implementation bug.
The exact supplying archive and expected map/light pairing still need checking.
No isolated screenshot comparison establishes their visible impact. Do not
overwrite nQuake improvements or user resources based on the warning alone.

## Historical reports and items that are not bugs

| ID / item | Status |
|---|---|
| HIST-TEXTURE-001: black/untextured models/items on some GPUs | Older upstream diary reports; not reproduced locally on current code; mipmap corruption is only a hypothesis |
| HIST-VSYNC-001: timedemo with VSync hangs | Local three-round FIFO test passed; monitor rather than call this an active bug |
| HIST-VISUAL-001: water, skywind, muzzleflash interpolation, luma/lightmaps and outline colors | Older audit candidates; backend differences alone are not bugs; appearance parity deferred by the user |
| PERF-INVESTIGATE-003: outline-off Vulkan throughput deficit | Confirmed performance result (-16.8% at 720p); tracked in TODO, no evidence of wrong output |
| Missing bounded alias fallback, full HDR/SSAO/shadows/temporal effects, remaining RT callbacks | Known feature/compatibility work, not regressions of implemented features |
| Campaign playthrough, broader KTX/network/GPU coverage | Test gaps, not demonstrated failures |
| Script-buffer runaway, truncated benchmark MVD, missing QMB asset pack in old fixtures | Known fixture problems; do not reopen these as unexplained engine bugs |

The user-reported moving dark outline was acceptable after raising its threshold;
approved defaults include that value. It is not an unresolved defect report.

## Fixed issues remain in history

Oversized world/sprite push blocks, caustics ABI mismatch, NPOT reporting, skybox
dimension assumptions, alias lighting, screenshot lifecycle, QTV shutdown,
CV offscreen/HUD render-pass errors, single-player startup/save handling and the
three adapted KTX gameplay corrections have documented fixes and scoped tests.
The accidental removal of conditional QMB settings was reversed by MAINT-005.
Do not count their old failing fixtures as new open bugs. Full provenance and
scope are retained in [DEVELOPMENT.md](DEVELOPMENT.md),
[VALIDATION.md](VALIDATION.md) and [TIBAZERA-TODO-HISTORY.md](TIBAZERA-TODO-HISTORY.md).


## SHADOW-001 scope and follow-up

Dynamic point-light shadow maps are implemented; see DYNAMIC-SHADOWS.md. Server-withheld entities cannot cast, and budget overflow intentionally uses an unshadowed selected light. SHADOW-002 adds map/spot lights, approximate baked-shadow and realtime replacement modes, caching, bounded updates and multiview support; MULTIVIEW-001 and SHADOW-OPT are completed. See DYNAMIC-SHADOWS.md for the remaining intentional material/lighting limits.


### SHADOW-OPT: historical disabled-path throughput observation

SHADOW-002 replaces this inconclusive whole-demo comparison with labeled GPU
timestamps in a fixed active-light scene and specializes out disabled shadow code.
See provenance/shadow2-performance.json; the older numbers below are retained.

The SHADOW-001 720p shared-profile MVD comparison measured 8.228 ms before and
8.639 ms with the new code/shadows off (about 5% slower). Two measured rounds,
variable results in enabled cases and uninstrumented light activity do not
establish a stable regression or its cause. All 26 disabled compatibility images
match. Reproduction: Benchmark-Shadows.ps1; raw timing and binary hashes are in
provenance/shadow-performance.json. Profile and repeat before optimizing or
making a broader performance claim.


## Menu/preset implementation audit, 2026-09-15

OpenAI Codex corrected prototype issues before delivery: complete graphics saves
must accept the inherited empty/vector viewmodel offset and current particle
trail detail; native disabled rows must distinguish integer storage from cvars;
layout movement aliases must display their actual keys. New automated fixtures
cover transaction rollback, complete preset round trips and real KTX requests.
These findings are fixed implementation defects, not new outstanding beta bugs.
Existing rendering/RT issues above are unaffected. Full manual UI/input/server
coverage remains on the TODO; no claim is made that all engine bugs are known.


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


## CFG-PERSIST-001 — Reported slider changes absent from saved configuration

2026-09-15, user report; investigation by OpenAI Codex. The user increased world
edge visibility using sliders, but the configuration saved at 20:20 local time
still contains r_cv_edgestrength 0.4 and r_cv_edgewidth 1.4, identical to Balanced.
The intended replacement values and the exact interaction sequence are unknown.
Do not interpret an unchanged file as evidence that the user's changes succeeded.

Not reproduced in isolated Release Vulkan fixtures: world-edge-savequit changes
opacity/width through native keyboard widget events to 0.5/1.5, closes the menu
and verifies those values in the configuration written by normal quit.
world-edge-mouse-restart-quit uses native mouse down/up events to set 0.8/2,
restarts video with F5, changes tabs, closes the menu and verifies quit saving.
Both exit normally without Vulkan validation errors. These checks do not explain
the user's session; no persistence fix is claimed. Capture values immediately
after the user's adjustment and compare before/after menu exit and config save.
The r_cv_worldedge value is a palette index, not outline strength.

Follow-up at 20:34 on 2026-09-15: the user saved again; opacity 0.3 and width 2.2 are now present in config.cfg and were packaged into Balanced (CFG-BALANCED-002). The earlier missing-change cause remains unconfirmed.
