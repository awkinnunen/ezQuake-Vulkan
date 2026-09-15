# Linear HDR, world SSAO and MSAA attachment stores

RASTER-001 (HDR-001, SSAO-001, PERF-OPT-002), implemented and documented by
OpenAI Codex at the user's request, 2026-09-14. This extends tibazera's ezQuake
Vulkan renderer and the previous local work; it is not a vkQuake-RT code import.
All three features use ordinary raster Vulkan and work on the development
Radeon integrated GPU. They do not need RTX hardware or an HDR monitor.

## Using the features

Use the existing `Start-Vulkan-Fullscreen.cmd` or `Start-Vulkan-Debug.cmd`.
The normal Release and Debug executables have been rebuilt in place.
Open **Options > Graphics > Visual Effects** (`menu_visual_effects`).

| Page | Menu item | Cvar | Behaviour |
|---|---|---|---|
| Image | Linear HDR | `r_cv_hdr` | Floating-point linear scene; F5/Restart video applies the change |
| Image | Exposure / Tone curve | `r_cv_exposure`, `r_cv_tonemap` | Existing live controls now operate on actual linear scene values in HDR |
| Effects | Bloom source | `r_cv_bloomsource` | 0 Bright pixels, 1 Emissive; emissive requires active HDR and nonzero bloom |
| Effects | Bloom strength / threshold / radius | `r_cv_bloom`, `r_cv_bloomthreshold`, `r_cv_bloomradius` | Existing live controls; zero strength disables the bloom shader work |
| Effects | World AO strength / radius | `r_cv_ao`, `r_cv_aoradius` | Existing live controls; zero strength gives the exact neutral image |
| Effects | AO method | `r_cv_aomode` | 0 legacy Contact, 1 reconstructed-position SSAO |
| Effects | SSAO samples | `r_cv_aoquality` | 8, 16, 24 or 32 hemisphere samples |
| Effects | SSAO bias | `r_cv_aobias` | 0.1–2 world units; reject near-coplanar self-occlusion |
| Image | MSAA | `vid_framebuffer_multisample` | Existing sample control; the store optimization is automatic |

Every new row has contextual help. Dependencies are indented and greyed out
when unavailable: bloom source needs bloom and active HDR; SSAO samples/bias
need nonzero World AO and the SSAO method. HDR and MSAA show requested versus
active state and a persistent restart notice. Reverting an unapplied change
clears the notice. Existing OpenGL `vid_framebuffer_hdr` is a separate legacy
setting; use **Linear HDR / r_cv_hdr** for this Vulkan implementation.

The user subsequently requested all new features enabled. The active config,
ESDF/WASD and user-tuned full configs, installed visual profiles, and distributable
project profile now set HDR=1, SSAO method=1, samples=16, bias=0.5, bloom source=1.
Existing positive AO/bloom strengths, exposure, tone curve and MSAA 4x remain
unchanged. Project defaults use AO 0.125/radius 22 and bloom 0.1/threshold 0.7/
radius 2.75. The original files are recoverable under
`cache/raster-defaults-backup`; `private-raster-defaults.json` records exact
before/after hashes. No bindings or game assets were changed. Factory cvar
defaults remain HDR off/Contact/AO off for installations without this profile.

## Implementation and limits

HDR selects RGBA16F only when attachment, blending, sampling, linear filtering
and the selected MSAA combination are supported. Otherwise it logs SDR fallback
and reports inactive HDR without leaving a permanent false restart notice.
Unsupported-format fallback is implemented but could not be forced on the
tested GPU without modifying capability reports; allocation-failure coverage
is not established by the supported-device tests.

World, model, sprite and transparency shaders decode colour inputs once and
accumulate in a float scene target. Baked lightmap and detail/caustic brightness
modulation remain scalar lighting, avoiding an extra sRGB decode. Fullbright
world/model passes and additive effects feed a separate RGBA16F emission target
with the scene's coverage/blending. Fullbright overlays have additional HDR
headroom. Emissive bloom therefore excludes ordinary bright albedo; it is a
glow effect, not light cast onto nearby geometry. No material assets are added.

Both MSAA targets resolve before postprocessing. HDR bloom is added in linear
space, then manual exposure/tone mapping and sRGB encoding produce ordinary SDR
output. Existing approximate FXAA/sharpening and tint/gamma composition follow;
HUD and console are drawn separately. This is **internal HDR**, not HDR10/scRGB
monitor output. Automatic exposure, temporal reconstruction and a wide bloom
pyramid are not implemented. Bloom retains the existing normalized 25-tap
kernel; BLOOM-PYRAMID is a separate follow-up in TODO.

SSAO reconstructs camera-relative positions from the existing radial-depth and
world-normal buffer using the actual view/projection matrices. Moving BSP
entities are transformed into world space before normal/depth generation.
Hemisphere bias and distance falloff reject self-occlusion and unrelated depth
discontinuities; sky/water/invalid samples are excluded. Sampling is deterministic
and non-temporal, with no frame-varying noise. Legacy Contact mode is retained.
This implementation covers **world/BSP surfaces only**, at full resolution;
players, alias models, weapons and transparent surfaces do not participate.
It is not CACAO or a half-resolution/filtered AO pass. AO-EXTEND tracks those
possible extensions. SSAO can only use visible geometry and is not ray tracing.

PERF-OPT-002 uses a compatible terminal render-pass variant for the single-view
offscreen path. Its multisample colour attachments use DONT_CARE store operations;
the resolved colour/emission attachments are still stored for sampling. Resumed,
multiview and direct cross-frame LOAD paths retain MSAA colour stores. Main depth
is discarded because every scene pass clears it and nothing samples/resolves it.
SSAO reads its own prepass, not discarded main depth. The prior deferred-pass
optimization PERF-OPT-001 is retained.

At 1920×1080 with two swapchain images, HDR adds approximately 142.4 MiB of
uncompressed attachment storage at MSAA 4x (47.5 MiB without MSAA), relative to
the SDR offscreen path. This is an attachment-size estimate, excluding driver
alignment, heaps and transient allocation; it is not a measured process budget.

## Validation

Final activation checks pass: `Test-RasterStartup.ps1` starts the very first
Vulkan scene in HDR from a saved config without issuing any video restart.
Settings are registered before initial target selection; menu initialization
later reuses them. The public-profile fixture loads/saves all 58 graphics values,
retains all 55 conditional particle settings and verifies WASD/pickup/crosshair
behaviour. `Test-RasterMenu.ps1` exercises all five new widgets with keyboard,
mouse dragging, dependency lockouts and F5. Help text fits the existing three-line
area, including the HDR restart instruction. Completed final builds have separate
hashes in `provenance/raster-final-build.json`; earlier capture/benchmark build
identities are retained rather than reassigned to a newer executable.

Tests use isolated configs and the installed assets. Release and Debug each
passed 19 paired frozen captures covering SSAO 8/16/32, bias/radius, strength zero,
HDR curves/exposure, both bloom sources, combined effects, MSAA 4/0, FXAA and
800×600 -> 960×540 -> 800×600 recreation. Each pair is pixel-identical. Zero AO
and the final SDR restore exactly match the initial SDR image. Emissive bloom
changes 18,049 pixels in the Release powerup-actor fixture; switching bloom
source changes 19,859. These counts demonstrate execution, not aesthetic quality.

The compatibility fixture matches all 26 scene images (13 cases × two captures)
against the retained pre-change renderer exactly. Menu animation screenshots
are excluded from exact-image claims. Motion tests cover E1M1/DM4/DM6, forward/
backward demo seeking, pause, skin reload and video restart with HDR/SSAO active.
Completed single-view runtime fixtures exit normally with no Vulkan validation
errors. F5 tests cover pending/revert/enable/disable HDR, MSAA 4x, sample clamping
to the device's 8x maximum, and screenshots directly around recreation.

Production shader GPU tests pass 257 HDR encode/decode round trips, known linear
mid-grey and over-white values, SSAO rejection and bias cases. C/SPIR-V checks
cover the new 112-byte AO block and 128-byte brush-normal block, the existing
13 scene blocks and all 22 shader modules. The prior competitive/ruleset probe
also passed, including the 224-byte shared visual block and 522 GPU results.

Reproducible helpers: `Test-RasterFeatures.ps1`, `Export-RasterEvidence.py`,
`Test-VisualRestart.ps1`, `Test-MotionRuntime.ps1 -RasterFeatures`,
`Test-ScenePass.ps1`, and `probes/raster-features-regressions.py`.
Machine-readable image evidence: `provenance/raster-features-validation.json`.

The first extended F5 fixture exceeded its inherited 45-second timeout; the
completed 180-second-budget rerun passed. Early powerup screenshots varied
because view setup calculated shell scroll before the test restored its clock;
the opt-in fixture now freezes that derived value too. Neither incomplete test
is counted as passing evidence. Normal gameplay clocks are unchanged.

The first activation export used ordinary CFG quoting/blank-line conventions,
which the stricter visual-only profile loader rejected. The exporter was corrected
to use that loader's existing format; the successful round trip uses the corrected
profiles. A first startup probe omitted `developer 1`, suppressing its diagnostic
assertions despite producing the HDR image; the corrected probe passed. These
incomplete fixtures are retained locally and do not represent remaining defects.

MULTIVIEW-001 remains open; this work does not claim to fix multi-camera
descriptor lifetime. RT startup remains paused. Long-session visual quality,
other GPU families, driver fault paths and HDR monitor output are not certified
by these single-device raster tests.

## Performance on the development GPU

Release, integrated AMD Radeon Graphics / Ryzen 5 5600U, Vulkan 1.2.188,
uncapped windowed rendering with no validation or GPU timestamp instrumentation.
The same high-eyecandy preset, 1,290-frame six-player DM6 demo, MSAA 4x and FXAA 5
were used throughout. Each case has one discarded warm-up and five measured
runs. Baseline/current pairs have identical recorded commands, preset and demo
hashes. Factory CV styling and bloom are disabled in these tests; these numbers
do not measure the user's complete styled profile. New-feature cases add HDR
with Filmic tone mapping and/or SSAO strength 0.4, radius 32, samples 16.

| Case | Resolution | Mean frame time | FPS from mean | Run mean range |
|---|---|---:|---:|---:|
| Before, full shared effects | 1920×1080 | 4.828 ms | 207.1 | 4.761–4.924 ms |
| Current, full shared effects | 1920×1080 | 4.777 ms | 209.3 | 4.734–4.888 ms |
| Before, outlines off | 1280×720 | 1.931 ms | 518.0 | 1.906–1.950 ms |
| Current, outlines off | 1280×720 | 1.920 ms | 520.9 | 1.910–1.938 ms |
| Current + HDR | 1920×1080 | 8.797 ms | 113.7 | 8.706–8.931 ms |
| Current + SSAO | 1920×1080 | 8.775 ms | 114.0 | 8.697–8.861 ms |
| Current + HDR + SSAO | 1920×1080 | 12.967 ms | 77.1 | 12.446–13.442 ms |

The compatibility-path increases are only **1.06% and 0.57%**, with overlapping
run ranges: there is no demonstrated large throughput improvement on this GPU.
The MSAA change removes unnecessary stores without changing the tested images;
its practical speed benefit here is small/uncertain. HDR adds about 4.02 ms and
SSAO about 4.00 ms to this 1080p workload; the combination adds about 8.19 ms.
These are application frame-time deltas, not isolated GPU-stage timings.

This comparison uses the retained pre-task Vulkan binary, not original ezQuake.
Historical original-renderer comparisons remain in
RENDERER-PERFORMANCE-OPTIMIZED.md. One scene/GPU and short sequential runs do not
establish universal rankings or worst-case latency. Final post-benchmark edits
adjust menu help/lockouts and register saved cvars before initial target selection;
the timed draw code and shaders are unchanged. Per-run commands, hashes and
measurements are in `provenance/raster-performance.json`, generated by
`scripts/Export-RasterPerformance.py` from seven isolated `cache/raster-bench-*`
profiles.

## Build and attribution

Patch `patches/ezquake-19-hdr-ssao-msaa.patch` follows patches 1–18.
`scripts/Source-Changes.json` records RASTER-001 paths and authorship; upstream
copyright and earlier change authors remain intact. CMake tracks the new shared
HDR shader include. The local builds used `Build-RasterFeatures.py`, a bounded
MSVC/link fallback using generated CMake options because the local CMake/Ninja
compiler launch was unreliable. It rebuilt all affected translation units and
shader resources; it is not a successful clean-checkout CMake build claim.
Normal binary hashes are recorded in `build-artifacts.json` and image evidence.
