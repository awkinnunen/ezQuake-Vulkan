# Competitive Visuals

CV-001, 2026-09-13. Implementation and documentation: OpenAI Codex, directed by
the user. Based on tibazera's Vulkan ezQuake renderer; no replacement game assets.

2026-09-14 update, OpenAI Codex: Linear HDR, SSAO method/samples/bias and bloom
source are implemented. At the user's request, project/ESDF/WASD profiles enable
HDR, SSAO and emissive bloom while retaining their prior effect strengths.
See [RASTER-FEATURES.md](RASTER-FEATURES.md) for current controls and evidence.

## Using the feature

Launch `Start-Vulkan-Fullscreen.cmd` and open **Options > Graphics**:

- **Competitive Visuals** (`menu_competitive`): 34 settings on Profiles, World,
  Lighting and Silhouettes pages, focused on readability and player distinction.
- **Visual Effects** (`menu_visual_effects`): 19 settings on Effects and Image
  pages, including bloom, contact AO, projected shadows, surface decoration,
  exposure, tone curve, sharpening, AA and texture filtering.

The five duplicated projectile/flame controls remain in the ordinary Graphics
menu. Its duplicate outline row and classic-OpenGL-only Bloom row were removed;
legacy `r_bloom` remains available through the console for classic OpenGL.

Groups place prerequisites before indented dependent settings. Highlight any
row for its explanation; disabled controls are grey and explain the missing
prerequisite. Keyboard and mouse changes are blocked while disabled, without
changing the saved value. Group titles stay with their first control when
scrolling. All eight profile actions also have descriptions.

PgUp/PgDn/Tab change pages within the current menu, up/down or wheel scroll rows,
and left/right adjust values. Drag sliders, or click/Enter to toggle switches.
F6 compares the competitive look with the original; F7 resets the current page.
Clean, Soft Cel, Original and comparison leave general effects unchanged.
General effects work independently of the competitive Enabled switch.

MSAA and Linear HDR are applied when video initializes. A persistent warning
appears after either changes; **Restart video (F5)** applies it and clears the
warning. Their rows report the active sample count/HDR state, including hardware
limits and fallback. SSAO method/samples/bias and bloom source update live.
The original game remains loaded. Other settings in these two menus update live,
subject to assets, relevant geometry and server rules.

The prior 48-control version has stable rendered-image evidence in
[VISUAL-TEST-RESULTS.md](VISUAL-TEST-RESULTS.md), alongside menu and dependency tests.

The user approved the saved visual settings as project configuration defaults
(CV-DEFAULT-001). `cv load project-default` restores all 53 managed visual values,
including **Edge depth threshold 16**. The portable, visual-only source is
`runtime/project-default.cfg`; its installed copy is in `ezquake/competitive/`.
`exec visual-defaults.cfg` now loads this profile and the existing input defaults.

Normal launches use the active saved config, so subsequent user changes still
persist. The active config is preserved byte for byte. Full `configs/user-tuned.cfg`
and visual-only `competitive/user-tuned.cfg` snapshots were refreshed, and the
legacy ESDF profile's visual values were aligned without changing bindings or
aliases. Earlier snapshots remain backed up. Engine factory presets and F7 reset
retain their original meaning; restore project defaults with the named profile.

Console examples:

```
cv preset clean
cv preset cel
cv preset original
cv save tournament
cv load tournament
cv compare
```

Named profiles live in `gamedata/nquake/ezquake/competitive/<name>.cfg` when using
the supplied launchers. Names contain letters, digits, underscores or hyphens.
The loader parses only the visual cvar allowlist; it never executes a CFG.
Malformed, empty, out-of-range or unknown-command profiles are rejected before
any values or comparison state change. Bindings and network settings are excluded.
Existing cvar callbacks still enforce their own ruleset restrictions.

## Implemented rendering

- Mip-based broad-pattern reconstruction, fine detail/contrast, saturation,
  separate floor/wall palette tints and gradual distance smoothing. Original
  texture alpha, geometry, sky and water treatment are preserved.
- Original, gradient and soft-cel light response, independent world/model gates,
  shadow/midtone/highlight levels, band count/softness and restrained warm/cool balance.
- Existing depth-tested model/world outlines with independent width/opacity,
  optional palette overrides and friend/enemy colours. Existing ruleset gates
  remain authoritative; the menu reports the current ruleset/world-edge gate.
- A continuous player surface rim with adjustable width, upward bias and very
  slight scoreboard shirt/team colour. It is not an extruded or animated shell.
  Strength is capped at 0.2, tint at 0.3. **Any EF_RED, EF_GREEN or EF_BLUE powerup
  bit disables the new rim entirely**, including combined quad/pent effects.
  Weapons, invisibility eyes and models barred by the outline ruleset get no rim.
  Restricted models also bypass the new model light response. Original powerup,
  fullbright, shadow and outline shader passes bypass the new lighting/rim path.
- Optional 25-tap bright-pixel bloom, capped at 0.15, with threshold and small
  radius controls. World contact AO uses eight normal/depth samples, excludes
  coplanar surfaces and sky/water, and is capped at 0.4. AO precedes player drawing.
- Manual exposure, soft-shoulder/filmic tone curves and locally clamped sharpening.
  Scene composition, including existing approximate FXAA, happens before HUD and
  crosshair rendering. MSAA/filtering controls use the existing renderer facilities.
- Existing projected model shadows, detail texture and caustics are in Visual
  Effects. Rocket/grenade trails, explosions, muzzle flashes and torch flames
  retain their ordinary Graphics controls.

New controls use the `r_cv_` prefix. The World page's Enabled switch enables the
new Vulkan style; existing renderer controls retain their own enable settings.
New effects are Vulkan-only. The menu and existing controls also work under OpenGL.
Values sent to shaders are finite and bounded; the menu shows bounded new values.

## Renderer changes and limits

A 224-byte std140 UBO carries the style and per-model transform. Per-draw slices
are aligned to the device's UBO limit and retained until the corresponding frame
fence completes; world/postprocess use the frame snapshot. World and model push
constants remain at 128 bytes. Five bound descriptor sets are now required by
world pipelines; the new allocation reports a clear failure below this limit.
The per-frame arena supports 8,191 model draw slices plus the scene snapshot and
fails explicitly on overflow. Disabled rim/special passes skip its vertex normal
matrix work. This is not the separate bounded-model fallback milestone.

The scene-to-HUD split fixes the next-frame offscreen image layout, uses compatible
single-sample HUD/composite passes, and preserves MSAA colour/depth when the scene
pass is resumed after world normals. Multiview uses the engine's two-pass 3D/2D
schedule; SHADOW-002 now verifies 0/2/4/0 views with the new raster effects.

RASTER-001 subsequently added an optional floating-point HDR scene target and
scene-depth SSAO; see RASTER-FEATURES.md. SHADOW-001/002 added dynamic and map-light
shadow maps, spots, caching and budgets; see DYNAMIC-SHADOWS.md. The Model shadows
slider still controls inherited projected shadows independently. Temporal
AA/upscaling and temporal accumulation remain backend work
in IMPLEMENTATION-PLAN.md. No inert controls are presented for them.
The existing Vulkan FXAA remains an approximation, not NVIDIA quality-preset parity.

## Validation

Debug and Release builds, six real menu pages, keyboard navigation/page reset,
visual-only profile roundtrip, invalid/empty-profile atomicity, F6 restoration,
MSAA transitions, vid_restart and E1M1 screenshots are exercised by
`scripts/Test-CompetitiveVisuals.ps1`. The new production C gate has 32 checks for
all powerup combinations, weapon/nonplayer and ruleset rejection. Seven compiled
shader UBO layouts match C. Another 522 actual GPU results verify neutral and
model bypasses, continuous monotonic lighting and bounded/upward-biased rim.
Existing 13 push-block layouts, 22 SPIR-V modules, 1,094 mip cases, 42 skybox GPU
cases and 511 alias-lighting GPU comparisons remain covered.

Run after building:

```powershell
. ./scripts/Enter-Dev.ps1
./scripts/Test-Vulkan.ps1
python probes/competitive-regressions.py
./scripts/Test-CompetitiveVisuals.ps1 -Label my-cv-test -Configuration Release
```

The scripted UI driver is available only with `-dev` and `developer 1`; it calls
the production handlers. Runtime fixtures use isolated profiles and existing
asset bytes. No competitive advantage/performance improvement is claimed.
Recorded movement, full quad/pent third-person image comparisons, long-session
latency, representative-map recognition and multiview gameplay remain manual
acceptance work; the powerup suppression itself is covered by production-code tests.

## Lessons from VALORANT

Riot's 2020 shader article describes gradient-remapped lighting with independently
adjustable shadow, midtone and highlight regions, plus additional grazing-angle
character lighting biased toward upward-facing surfaces. These support a smooth
illustrative response as an alternative to visible cel bands. Its environment-art article describes moderated
material value contrast, avoiding excessive darkness, and concentrating decorative
detail above player height. These are historical design explanations, not an
audit of the current game's renderer or features.

Sources:
- [VALORANT Shaders and Gameplay Clarity](https://www.riotgames.com/en/news/valorant-shaders-and-gameplay-clarity)
- [The Art of VALORANT Map Environments](https://playvalorant.com/en-us/news/dev/the-art-of-valorant-map-environments/)

Proposed Quake adaptation (Codex engineering/design judgment): reduce world
texture microcontrast while retaining larger patterns and baked light structure;
offer restrained, depth-tested model rim lighting and silhouettes separately;
keep gameplay effects recognizable and preserve stable image behaviour in motion.

VALORANT's author-controlled placement of decorative detail cannot be recovered
reliably from arbitrary Quake maps by a universal shader. Do not suppress an
arbitrary screen-height band: it would move as the player aims and could erase
useful details. Original authored shadows and landmarks also constrain automatic
restyling. Use optional material overrides later only when automatic processing
loses recognizability, without requiring edits to the original assets.


## CV-TUNE-001 — More visible user defaults (2026-09-13)

At the user's request, the active config and legacy-esdf.cfg now use the saved
Vivid visual profile. Load it independently with cv load vivid. It strengthens
cel response (blend 0.9, three bands, softness 0.15), texture simplification,
surface rim (0.18 versus 0.06), world/model edges, contact AO (0.3), manual
exposure and scene sharpening. Bloom is enabled at 0.1, threshold 0.7 and radius
2.75. All values use the existing bounded renderer controls; shader ranges and
powerup/ruleset exclusions are unchanged. The built-in Clean/Soft Cel presets
retain their previous conservative values; Vivid is a separate named profile.

The same request reduces crosshairsize 8 -> 2 and enables pickup autoswitch
with w_switch 8 / b_switch 8. exec visual-defaults.cfg applies this combination
to an already running client. Current config lines were changed in place with
all non-target bytes/lines preserved; original copies are in
cache/visual-tuning-backup and hashes/settings in private-visual-tuning.json.

## CV-002 — Sliders and switches (2026-09-13)

OpenAI Codex, under user direction. All 53 managed controls now have appropriate
widgets: 34 bounded sliders with numeric readouts, nine On/Off switches and ten
discrete selectors. Light response, tone curve, outlines, muzzle flashes and
filtering show readable mode names. Dragging captures the original row until
release, clamps outside the track, snaps to the same steps as the keyboard and
respects menu scaling. Clicking outside the rows does not change a setting.
Existing binary settings now toggle off as well as on with Enter or a click.
The row counter makes additional rows visible in long sections.

No renderer ranges or profile membership changed. Bloom remains adjustable from
0 (off) to 0.15. Powerup/ruleset exclusions are unchanged; the shadow correction is described below.
User crosshairs are now size 2.5 (25% larger than size 2), in the active config,
legacy profile, control overlay and visual-defaults.cfg. Crosshair size remains
separate from visual-only profiles. Apply it immediately with crosshairsize 2.5.

### Preset provenance and limits

The main Graphics menu and its High Eyecandy preset come from ezQuake. The active
package contains cfg/gfx_gl_higheyecandy.cfg: it enables many particle/light/texture
features but sets r_shadows 0 and gl_detail 0, uses bilinear mip filtering, and
does not set r_cv_* or framebuffer AA. It is not a maximum-quality master switch.
Its label is cached/inferred from a few legacy settings, not an audit of every
current setting. Loading it can overwrite overlapping settings in Vivid.

Competitive Visuals contains 38 project-added r_cv_* controls and 15 inherited
ezQuake controls, backed by tibazera's Vulkan work plus local fixes/extensions.
No vkQuake-RT/RayTracedGL1 integration has been made into this client. Vulkan
raster rendering does not imply ray tracing. New shader effects require Enabled
and the relevant blend/strength/colour gates; existing effects can also depend
on assets, scene content, hardware and server rulesets. A nonzero menu value
alone is not proof every effect is rendering in every scene.

The follow-up source audit found that r_shadows uses an integer admission gate
and fixed Vulkan opacity. CV-002 corrects its misleading strength slider to
On/Off and changes the intended enabled Vivid/legacy-profile value from 0.5 to 1.
The active config already has 0 and that user setting is preserved. Other visual
ranges are unchanged. Vivid still deliberately disables MSAA, FXAA, detail
overlays and palette overrides; it is a style profile, not All Effects.


## Dynamic shadows (SHADOW-001)

Thirteen live controls are on the general Visual Effects / Effects page under Light and shadow maps, with prerequisite help and disabled dependent rows. They are independent of Competitive Visuals styling. The existing Shadow level control still means styled-lighting brightness, and Model shadows still means legacy projected shadows. See DYNAMIC-SHADOWS.md.
