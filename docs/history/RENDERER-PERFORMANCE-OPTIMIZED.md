# Optimized Vulkan versus original ezQuake performance

PERF-COMPARE-003. Measured and documented by OpenAI Codex, 2026-09-14.

Fresh measurements after PERF-OPT-001; the original PERF-COMPARE-001 report is retained.
The project Release executable includes the deferred scene-pass optimization.
Both resolutions include a same-executable OpenGL control. The final two rows
repeat the 1080p original/Vulkan pair to check drift; headline results use the first pair.

AMD Ryzen 5 5600U, integrated AMD Radeon(TM) Graphics, Windows. Vulkan 1.2.188;
OpenGL 4.6 driver 30.0.13032.4000. Windowed, VSync off; Vulkan selected IMMEDIATE.
Native pixel dimensions were checked from captures, not the OpenGL gfxinfo desktop-resolution field.

Original baseline: the nQuake-supplied ezQuake 3.6.6 binary, build 7947~baab4dfd7.
Project: the existing MSVC x64 Release binary, based on 8251~91859a996, reporting
3.7.0-dev. The same project executable supplies the additional OpenGL control.
This compares actual applications; the old distribution build uses a different
source revision and build environment. The same-executable control better isolates the backend.

Each case uses the same private 19.995-second, six-player DM6 MVD and installed
game assets. One warm-up is discarded, followed by five measured runs in the same
process. Every timedemo reports 1,290 frames. Loading is outside the timedemo interval.
Runs are sequential. Values are medians of the five reported average frame times;
FPS is 1000 / that median. The range shows variation between those five runs,
not individual-frame minimum/maximum latency.

## Shared high-effects profile

The packaged high-eyecandy preset is used with detail and caustics enabled,
projected shadows, model outlines, trilinear textures, full-resolution textures,
classic/QMB effects, shaft, coronas and powerup shells. Explosion type is the
shared Big explosion (7), not project-only Explosion (11). MSAA is 4x and FXAA 5.
World outlines are on in the full preset and measured separately off.
Shaft interpolation is explicitly 1 in all builds; the two versions had different defaults.
The GL-only texture-compression switch is removed from the shared preset and
explicitly disabled on OpenGL. Every remaining shared setting is recognized.
All shared saved cvar values match within each comparison except vid_renderer.
Competitive Visuals, project bloom/AO/exposure/tone mapping/sharpening and HDR are off.
The old bloom path belongs to classic OpenGL; it is not a shared modern-GL/Vulkan effect.

## Results

| Profile | Resolution | Renderer | Median ms | FPS | Run range ms |
|---|---|---|---:|---:|---:|
| Full shared effects | 1280 x 720 | nQuake ezQuake 3.6.6 / OpenGL | 2.606 | 383.7 | 2.551-2.696 |
| Full shared effects | 1280 x 720 | Project fork / Vulkan | 2.581 | 387.4 | 2.488-2.751 |
| Full shared effects | 1280 x 720 | Project fork / OpenGL | 2.677 | 373.6 | 2.644-2.779 |
| Full shared effects | 1920 x 1080 | Project fork / Vulkan | 5.190 | 192.7 | 5.144-5.288 |
| Full shared effects | 1920 x 1080 | nQuake ezQuake 3.6.6 / OpenGL | 5.270 | 189.8 | 5.124-5.319 |
| Full shared effects | 1920 x 1080 | Project fork / OpenGL | 5.448 | 183.6 | 5.360-5.740 |
| World outlines off | 1280 x 720 | nQuake ezQuake 3.6.6 / OpenGL | 1.714 | 583.4 | 1.711-1.732 |
| World outlines off | 1280 x 720 | Project fork / Vulkan | 2.061 | 485.2 | 2.046-2.121 |
| Full shared effects | 1920 x 1080 | nQuake ezQuake 3.6.6 / OpenGL (closing repeat) | 5.238 | 190.9 | 5.195-5.396 |
| Full shared effects | 1920 x 1080 | Project fork / Vulkan (closing repeat) | 5.106 | 195.8 | 5.044-5.167 |

| Comparison | Vulkan FPS change | Vulkan frame-time change |
|---|---:|---:|
| original-720 -> vulkan-720 | +1.0% | -1.0% |
| fork-gl-720 -> vulkan-720 | +3.7% | -3.6% |
| original-1080 -> vulkan-1080 | +1.5% | -1.5% |
| original-worldoff -> vulkan-worldoff | -16.8% | +20.2% |
| fork-gl-1080 -> vulkan-1080 | +5.0% | -4.7% |
| original-1080-close -> vulkan-1080-close | +2.6% | -2.5% |

## Interpretation and limits

The opening 1080p pair is 189.8 FPS (original) versus 192.7 FPS (Vulkan).
The closing pair is 190.9 versus 195.8 FPS.
Small percentage differences should be read alongside the repeated-run ranges and closing pair.
The retained pre-optimization comparison showed Vulkan 11.4% behind at 720p and 17.0%
behind at 1080p. Those historical percentages come from a separate session, not these new runs.
See [the original comparison](RENDERER-PERFORMANCE.md) and the controlled
[before/after optimization report](SCENE-PASS-OPTIMIZATION.md).

World outlines and FXAA use different implementations between backends; equal
setting values do not imply identical pixels or identical GPU work. The separate
world-outline pair quantifies that part of the selected visual workload.
This is one gameplay scene, not a stress test that activates every possible effect;
weather, underwater caustics and Team Fortress effects are not comprehensively exercised.
Results describe this integrated GPU, driver and desktop presentation environment.
They do not predict RTX 3060 performance or establish universal OpenGL/Vulkan rankings.
No RT code, normal graphics defaults or user configuration was changed for this benchmark.

## Reproduction and evidence

`scripts/Benchmark-Renderers.ps1` accepts a fresh label and original/fork-gl/vulkan
variant, dimensions, round count, MSAA, FXAA and outline mode. It creates an isolated
profile, links only installed assets, records hashes, saves effective settings and
rejects unknown shared commands, abnormal exits or incomplete demo runs.
`scripts/Export-RendererPerformance.py --optimized` validates frame counts, capture dimensions
and actual saved settings before generating this report.

Aggregate data and binary/demo/preset hashes: `cache/renderer-optimized-results.json`.
Raw local evidence: `cache/renderer-optimized-*/{manifest.json,results.json}`, saved configs
and `qw/qconsole.log`. Captures and the private demo stay in those local fixtures.
Earlier renderer-probe-* and renderer-main-* runs were calibration runs and are excluded.
