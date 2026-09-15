# Deferred scene-pass optimization

PERF-OPT-001. Implementation, tests and documentation by OpenAI Codex, 2026-09-14.

## Change

Single-view rendering with world outlines or AO now records the normals prepass
before opening the main scene pass. This avoids the initial empty MSAA resolve
and its clear/store/reload work. Scene clear/load decisions and actual clear values
are latched at frame start. The scene pass starts before world draws or, for menus
and early scene exits, before the HUD transition. Abandon/restart handles both an
active command buffer without a pass and a buffer with a scene/HUD pass.

Shaders, effects, draw ordering within the scene, attachment store operations and
approved graphics settings are unchanged. Multiview retains its previous eager
scene-pass ordering; this optimization applies to a single view.

## Throughput

1920 x 1080 on the Ryzen 5 5600U integrated Radeon, shared high-eyecandy profile,
same private six-player DM6 demo. One warm-up plus five measured 1,290-frame runs
per process. Values are medians of run-average frame times; FPS = 1000 / median.
These are normal Release builds without profiling or validation. Saved cvars,
demo/preset hashes and screenshot dimensions match within every before/after pair.

| MSAA | FXAA | Before ms / FPS | After ms / FPS | FPS change |
|---|---|---:|---:|---:|
| 4x | 5 | 6.091 / 164.2 | 5.050 / 198.0 | +20.6% |
| Off | 5 | 3.084 / 324.3 | 2.994 / 334.0 | +3.0% |
| 4x | Off | 4.982 / 200.7 | 4.060 / 246.3 | +22.7% |
| Off | Off | 2.490 / 401.6 | 2.480 / 403.2 | +0.4% |

Closing full-AA controls: before 6.091 ms, after 4.942 ms.

## GPU confirmation

Separate diagnostic captures use six timestamps with the same interval boundaries
and fixed boundary-frame trim as AA-DIAGNOSTICS.md. The pre-change GPU capture is
the retained PERF-COMPARE-002 capture; the after capture uses the new code. These
stage means are diagnostic evidence, separate from uninstrumented throughput.

| Interval | Before ms | After ms |
|---|---:|---:|
| gpu_ms | 5.883 | 4.757 |
| setup_normals_ms | 1.865 | 0.835 |
| scene_ms | 2.002 | 1.879 |
| scene_end_resolve_ms | 1.193 | 1.225 |
| postprocess_ms | 0.819 | 0.814 |
| hud_ms | 0.004 | 0.004 |

CPU waits overlap GPU work and must not be added to these GPU intervals.

## Image and lifecycle validation

Thirteen frozen actor/world configurations have zero changed pixels between old
and new builds, with zero repeat-frame drift in each build. They cover all eight
MSAA 0/4 x FXAA 0/5 x world-outline off/on combinations, AO without outlines,
combined CV/outlines/AO/bloom, explicit clear, reduced viewport and 960x540 resize.
Both complete validation-layer processes also cover menu-only rendering.

The pixel fixture fixes the actual KTX player position as well as the camera and
disables animated particles/dynamic lighting and settles camera resources before
capture. Earlier random-spawn/cold-camera captures were not comparable and are
excluded. Effects remain enabled in the separate timedemo
and motion regression. Final Debug motion validation covers three maps, forward/
backward demo seeks, paused demo, skin reload and video restart. The diagnostic
validation-layer smoke test has zero timestamp-query errors. No VUID or device
loss appears in these passing fixtures.

An additional multiview extension exposed a descriptor updated/destroyed while
still referenced by a command buffer in the retained pre-change binary. It is a
separate known failure, not a passing test; see MULTIVIEW-001 in TIBAZERA-TODO.md.

## Build and reproduction

Release and Debug executables are updated at their normal build paths, used by
the existing launchers. CMake/Ninja did not launch compiler processes in this
environment; Build-ScenePass.py rebuilt the two changed translation units directly
with the CMake-generated MSVC flags and linked the existing matching objects and
libraries. The header change only adds declarations; no shared layout changed.
A clean source checkout uses the normal CMake build. Local build identities and
compiler/linker logs are in cache/scene-pass-build/{Release,Debug}.

Source export: patches/ezquake-18-deferred-scene-pass.patch. The ordered patch
replay and attribution manifest must match all modified host files.

Run Test-ScenePass.ps1 with fresh labels and optional -Executable for the retained
baseline; use Benchmark-Renderers.ps1 for the AA matrix and Test-MotionRuntime.ps1
for lifecycle validation. Build-VulkanProfiler.py --output vulkan-profiler-opt
creates the separate diagnostic executable without replacing the old capture build.

Aggregate evidence: cache/scene-pass-results.json. Raw fixtures: cache/opt-*,
cache/runtime-opt-images-ready-*, cache/runtime-opt-motion. Private demos, game
assets, screenshots and executables stay in the local fixtures. No RT work or
user configuration changes are part of this optimization.
