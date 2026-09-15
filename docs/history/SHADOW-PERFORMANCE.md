# SHADOW-002 performance

1280x720, HDR, MSAA 4x, AO/bloom off, fixed camera and local active light fixture. No Vulkan validation. GPU timestamps bracket scene and shadow work; discard 32 boundary samples at each end of each labeled case. Whole-frame GPU time is not game FPS.

| Case | Before GPU median | Current GPU median | Current shadow interval |
|---|---:|---:|---:|
| off | 3.451 ms | 3.457 ms | 0.0001 ms |
| one128 | 3.590 ms | 3.472 ms | 0.0001 ms |
| one256 | 3.745 ms | 3.605 ms | 0.0001 ms |
| one512 | 4.112 ms | 3.499 ms | 0.0001 ms |
| two256 | 3.911 ms | 3.904 ms | 0.0001 ms |
| off-return | 3.408 ms | 4.064 ms | 0.0001 ms |
| two256-redraw | — | 4.247 ms | 0.3004 ms |
| eight256-redraw | — | 6.158 ms | 1.0655 ms |
| eight256-cached | — | 4.928 ms | 0.0001 ms |

The initial disabled case is approximately 3.45 ms in both versions. The late disabled samples vary; no universal frame-rate improvement or cross-GPU claim follows from this fixture.
Cached one/two/eight-light cases record no shadow draw work. The near-zero timestamp interval is marker overhead, not a physical zero-cost claim; CPU selection/fingerprinting and receiver shading still run.
Eight-light cached versus forced redraw uses identical lighting/geometry and measures the benefit of reusing depth. Moving in-range casters invalidate their maps.
An intermediate diagnostic found out-of-range animated alias batches unnecessarily dirtied every light; range filtering fixes that without changing physical occlusion.
The earlier short-MVD throughput observation remains historical and is not substituted for active-light pass timing.

Eight-light cached GPU reduction in this fixture: 20.0%.
