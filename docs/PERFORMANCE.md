# Visual effect cost measurements

Historical MAINT-004 profile: bloom 0.03, threshold 1.35, radius 1. MAINT-005
later restored bloom to 0.1 / 0.7 / 2.75 and restored the wrongly removed conditional
particle settings. These measurements do not describe the restored defaults.

MAINT-004. Run and documented by OpenAI Codex, 2026-09-13.

Windows x64 Release, AMD Radeon(TM) Graphics integrated GPU, Vulkan API 1.2.188.
1280 x 720 window, no developer/validation flag, IMMEDIATE presentation for cost tests.
The 19.995-second DM6 MVD contains six players. Every timedemo rendered 1,290 frames.
Each case has one discarded warm-up and two measured runs. Loading is outside the
engine timedemo interval. These are application frame times, not GPU timestamp costs.

Initial default mean: 4.361 ms. Closing default mean: 4.282 ms.
The table compares each case with their mean (4.321 ms). Differences of the
order of baseline drift or sample spread should be treated as noise. Two samples
and one scene do not establish general hardware rankings or worst-case latency.

| Case | Sample 1 ms | Sample 2 ms | Mean ms | Saved vs default mean ms |
|---|---:|---:|---:|---:|
| defaults | 4.427 | 4.294 | 4.361 | -0.040 |
| no-ao | 4.001 | 4.065 | 4.033 | +0.288 |
| no-outlines | 4.016 | 4.003 | 4.010 | +0.311 |
| no-msaa | 2.433 | 2.428 | 2.430 | +1.890 |
| no-fxaa | 4.188 | 4.214 | 4.201 | +0.120 |
| no-bloom | 3.692 | 3.710 | 3.701 | +0.620 |
| no-detail | 4.515 | 4.256 | 4.386 | -0.065 |
| minimal | 0.840 | 0.862 | 0.851 | +3.470 |

Overrides are applied individually to the unchanged, approved 53-value profile.
`no-msaa` changes 8x MSAA to 0 and restarts video. `no-fxaa` changes FXAA 5 to 0.
`minimal` additionally disables Competitive Visuals, AO, outlines, bloom, detail,
caustics and projected shadows. It retains exposure/tone settings and other engine
features, so it is a reduced-effects comparison rather than a fully unlit renderer.
These selected effect costs are not additive; shared passes, cache behavior and
shader branches interact. Controls that are already inactive have no incremental
effect at these defaults. No graphics defaults were changed from these results.

The separate VSync test selected FIFO (present mode 2) and completed all three
timedemos normally. Its two measured runs averaged 36.115 ms/frame.
That value includes presentation pacing in this desktop environment. The historical
VSync hang was not reproduced here; other GPUs/drivers and long sessions remain untested.

The first benchmark attempt timed out because its truncated private fixture lacked
EndOfDemo. Adding the normal disconnect marker fixed the fixture. No renderer or
VSync code was changed on the basis of that timeout.

Release executable SHA-256: `c9b966a22301fdc50b656771c33aa0676d324086971ccc9f81fdbaf4177bf9f8`.
Private fixture SHA-256: `2c3608719580eb7a42463d7b577614fe567d99ed46b21b4d57492c0783a340d8`.
The source release contains the generator and aggregate results, not the private demo.
