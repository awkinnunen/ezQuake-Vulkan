# Raster light and shadow maps

SHADOW-001 and SHADOW-002 were implemented by OpenAI Codex at the user's request
on 2026-09-14–15. This extends tibazera's Vulkan renderer and earlier local work.
It uses ordinary Vulkan rasterization, requires no RTX hardware, and imports no
vkQuake-RT code or game resources. The separate RT backend remains paused.

## Controls

Start the normal Vulkan Debug or fullscreen Release launcher. Open
**Options > Graphics > Visual Effects > Effects > Light and shadow maps**.
All thirteen controls apply live, including resolution; no video restart is needed.

| Control / cvar suffix (`r_cv_`) | Project default | Range / meaning |
|---|---:|---|
| Shadow maps / `shadows` | 1 | Off/on |
| Shadow resolution / `shadowquality` | 256 | 128–512 pixels per face |
| Shadow softness / `shadowsoft` | 1 | 0–2 PCF pixel radius |
| Shadow distance / `shadowdistance` | 768 | 128–2048 distance to light influence sphere |
| Shadow lights / `shadowlights` | 4 | 1–8 selected lights per view |
| Shadow bias / `shadowbias` | 0.5 | 0.05–4 world units |
| Shadow strength / `shadowstrength` | 1 | 0–1 |
| Cache shadow maps / `shadowcache` | 1 | Reuse unchanged depth maps |
| Shadow updates / `shadowupdates` | 4 | 1–8 map updates per view/frame |
| Caster budget / `shadowcasters` | 4096 | 64–8192 surfaces/models per light |
| Map lighting mode / `maplights` | 1 | 0 original baked; 1 shadow baked; 2 realtime replacement |
| Map light radius / `maplightscale` | 1 | 0.25–4 radius multiplier |
| Realtime ambient / `mapambient` | 0.1 | 0–1 base light in mode 2 |

Every row has contextual help and dependent rows are indented/disabled.
Dynamic lights must be enabled (`r_dynamic`), flashblend and fullbright must be off.
Map radius requires map lights; ambient requires mode 2. The controls are separate
from Competitive Visuals styling and the old projected `r_shadows` effect.
The engine factory master/map mode remain off. Eight active/named/shipped project
profiles enable four lights, caching and mode 1. Other user settings, including
their current HDR choice and ESDF/WASD bindings, were preserved. Recoverable originals
are under `cache/shadow2-defaults-backup`; hashes are in `private-shadow2-defaults.json`.
Public WASD continues to load the same shared graphics profile (71 saved values).

## Lighting and assets

Client dynamic lights (rockets, explosions, muzzle flashes) share the bounded
selection with map lights. Only selected client light IDs are removed from legacy
CPU lighting, avoiding double illumination. Nearby opaque world polygons, moving
brushes and interpolated alias models cast. Alpha-tested texture holes are respected.
First-person weapons, sky, liquids, translucent models and additive effects do not cast.
Fullbright/emissive overlays retain their emission. The weapon retains legacy lighting.
Geometry withheld by the server/PVS cannot cast shadows.

Mode 0 retains baked lighting and adds shadowed selected dynamic lights. Mode 1
adds entity/model occlusion to baked light using nearby BSP light entities. Since
SHADOW-003 it excludes static world occluders: their shadows are already in the
lightmap. Reapplying those shadows from a camera-selected light subset made rooms
darken as the player moved, especially with larger Map light radius values.
It adds no second copy of their lighting. This is an approximation: a compiled lightmap
cannot be decomposed back into its individual source lights. Mode 2 replaces the baked
world/model term with ambient plus the selected realtime lights. It can look substantially
different and only represents the configured nearest-light budget, not all baked bounce light.

The loader reads up to 256 light entities from the BSP and an optional additive
`lights/<map>.vklights` file in the game search path. Both use quoted BSP entity syntax.
Supported keys: `classname` beginning with `light`, `origin`, `light` (radius),
`_light` (R G B radius, RGB 0–255), `_color` (RGB 0–1), `style`, `target`,
`targetname` and `_cone` (full cone angle, 5–160 degrees). A target pointing to an
entity with an origin creates a spotlight; ordinary lights are omnidirectional.
Lightstyles change intensity without invalidating depth. The optional file adds to
the BSP lights and does not modify a PAK. Reload the map after editing it.

Example optional `qw/lights/e1m1.vklights` (illustrative coordinates):

```text
{
"classname" "light"
"origin" "480 90 100"
"light" "1000"
"target" "spot_target"
"_cone" "100"
}
{
"classname" "info_null"
"targetname" "spot_target"
"origin" "480 250 35"
}
```

## Resource ownership and optimization

Each frame-in-flight and camera owns its depth atlas, receiver uniform and immutable
descriptor set. Dynamic geometry/index buffers also have separate view copies. The
atlas is six columns by eight rows at the selected resolution: 12 MiB at 256 or
48 MiB at 512 with D32, per active frame/view. Unused/off views use 1x1 placeholders;
resources are resized only after their owning fence. D16 is the format fallback.

World polygon bounds are cached on map load. Light-range and conservative face/cone
tests reject irrelevant casters. Point lights render six faces; spots render one.
Pipeline/vertex/material bindings are reused between compatible draws. Light/caster
fingerprints include transforms, interpolation, texture identity and resolution.
Unchanged maps are reused; a completely cached view skips the depth render pass.
Only updated tiles are cleared. Light selection has per-view hysteresis.

Round-robin updates enforce a hard map-update cap. A dirty light deferred by that cap,
or a light whose caster list overflows, is rendered fully lit rather than using an
incomplete or stale occluder map. Lower update caps can therefore temporarily remove
shadows during motion; set updates to the light count for every selected moving light
to update each frame. The three frame slots warm their caches independently.
The alias-model collection has a separate fixed cap of 256 batches; exceeding it
also falls back to unshadowed light, rather than rendering incomplete shadows.

Fragment specialization removes shadow code when the feature has no lighting work;
this avoids paying the large shader's register/branch cost on the disabled path.
Multiview now uses actual per-camera viewports, isolates CPU-written GPU resources,
and updates shared sky descriptors only once after the frame fence. Console screenshots
render all cameras and rebuild entity lists, preventing duplicate static casters.

## Verification and attribution

Reproduce using `Test-Shadow2.ps1` / `Verify-Shadow2.py`, `Test-Shadows.ps1`,
`Test-ShadowMultiview.ps1`, `Test-ShadowMenu.ps1`, `Test-PublicProfiles.ps1`,
`Test-ShadowCombat.ps1`, `Test-MotionRuntime.ps1 -Shadows -RasterFeatures` and
`Test-ScenePass.ps1`. Test assets/demos/captures stay private. The opt-in local
photo fixture now supports eight client lights; it cannot run in ordinary play.

`Build-ShadowProfiler.py` creates an isolated executable with GPU timestamps.
`Benchmark-ShadowScene.ps1` measures a fixed, actively lit scene with HDR and MSAA 4x
at 1280x720; AO/bloom are off to isolate this workload. Numeric CSV labels identify
cases. Cached and forced-redraw costs are separate; these are not whole-game FPS claims.
The earlier short-MVD SHADOW-001 timings did not isolate active shadows and remain
historical evidence in `provenance/shadow-performance.json`.

Final verification on 2026-09-15 covers nineteen stable control image pairs,
five exact cached/fresh or fallback comparisons, thirteen menu widgets, 71 public
graphics values, native rocket combat, three-map/demo lifecycle tests, and
0/2/4/0 multiview with HDR, MSAA, SSAO and bloom. See `SHADOW-PERFORMANCE.md`
for measured costs and their limits.

Disabled rendering has thirteen stable image pairs. An isolated diagnostic restoring
the old screenshot entity-list behaviour matches all 26 retained pre-shadow images
exactly; normal captures deliberately keep the duplicate-static-model correction.
Turning shadows off restores the original dynamic-light image exactly.

Final evidence is recorded in `provenance/shadow2-validation.json`,
`provenance/shadow2-performance.json` and `provenance/shadow2-final-build.json`.
Ordered patch 21 applies after patches 1–20; `scripts/Source-Changes.json` identifies
the exact Codex edits while preserving upstream authorship and licenses.
Allocation-failure and unsupported-format paths are not hardware-forced tests.
No RTX-machine validation is claimed for this raster feature.

SHADOW-003 (OpenAI Codex, 2026-09-15) fixes the user-reported DM6 / Baked with
shadows radius regression. `Test-MapLightStability.ps1` and
`Verify-MapLightStability.py` compare three camera positions and radii 0.25/1/4
against baked reference images. `provenance/shadow3-validation.json` records
current results; patch 22 follows patch 21. Existing user config values are
preserved. Dynamic entity shadow transitions and the bounded approximation of
Realtime mode remain separate from preserving the static baked world.
