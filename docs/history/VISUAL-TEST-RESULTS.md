# Visual setting evidence

CV-003, 2026-09-13. Test implementation and report: OpenAI Codex, under user direction.

Scope: all 48 controls in Competitive Visuals and Visual Effects. This is not
an exhaustive audit of every inherited ezQuake menu or graphics preset.

The actual Vulkan renderer produced four 800 x 600 RGB images per setting:
**A, repeat A, B, restored A**. Every passing row has zero changed pixels
between its three A images and a nonzero difference for B. This verifies
a rendered effect with prerequisites enabled, not merely a changed cvar.

Scene: paused local E1M1, fixed camera, no HUD. Original model geometry and
embedded skins provide player and ordinary-model coverage; gl_nocolors 1
avoids synthetic scoreboard slots requiring live-player skin caches.
Caustics use an underwater camera. Detail/caustic assets come from nQuake.
MSAA changes use F5 and allow frames to settle after video reinitialization.

Validation hardware: AMD Radeon integrated graphics, Vulkan 1.2.188.
Debug run with Vulkan validation; no validation/runtime errors in this run.
Effects can be invisible with other settings, assets, geometry or server rules.
Fullbright overlays can mask surface lighting. These tests do not establish
performance, perceptual quality in motion, other GPU support, or ray tracing.

Runtime evidence: `cache/runtime-impact-final-debug/impact-results.json`.
The same folder preserves the exact command specification. Screenshots are
private local outputs containing game assets; they are not bundled source assets.

| Setting | A -> B | Changed pixels | A drift | Result |
|---|---|---:|---:|---|
| Enabled (`r_cv_enable`) | 0 -> 1 | 469084 | 0 | observed |
| Fine detail (`r_cv_detail`) | 0 -> 1 | 362826 | 0 | observed |
| Pattern scale (`r_cv_pattern`) | 0 -> 5 | 441968 | 0 | observed |
| Texture contrast (`r_cv_contrast`) | 0.2 -> 1.5 | 336898 | 0 | observed |
| Saturation (`r_cv_saturation`) | 0 -> 1.5 | 471814 | 0 | observed |
| Distance smoothing (`r_cv_distance`) | 0 -> 2 | 128345 | 0 | observed |
| Floor colour (`r_cv_floor`) | 0 -> 13 | 253179 | 0 | observed |
| Wall colour (`r_cv_wall`) | 0 -> 13 | 219900 | 0 | observed |
| Surface tint (`r_cv_tint`) | 0 -> 1 | 479976 | 0 | observed |
| Light response (`r_cv_light`) | 0 -> 2 | 465104 | 0 | observed |
| Style blend (`r_cv_lightmix`) | 0 -> 1 | 470239 | 0 | observed |
| World lighting (`r_cv_worldlight`) | 0 -> 1 | 465104 | 0 | observed |
| Shadow level (`r_cv_shadow`) | 0 -> 0.7 | 253223 | 0 | observed |
| Midtone level (`r_cv_midtone`) | 0.1 -> 1 | 313555 | 0 | observed |
| Highlight level (`r_cv_highlight`) | 0.5 -> 1.5 | 223587 | 0 | observed |
| Cel bands (`r_cv_bands`) | 2 -> 8 | 416054 | 0 | observed |
| Band softness (`r_cv_softness`) | 0.05 -> 1 | 299372 | 0 | observed |
| Warm / cool (`r_cv_warmth`) | -0.2 -> 0.2 | 458792 | 0 | observed |
| Outline mode (`gl_outline`) | 0 -> 3 | 18984 | 0 | observed |
| World edge width (`r_cv_edgewidth`) | 0.5 -> 3 | 51331 | 0 | observed |
| World edge opacity (`r_cv_edgestrength`) | 0 -> 1 | 18984 | 0 | observed |
| Edge depth threshold (`gl_outline_world_depth_threshold`) | 1 -> 16 | 835 | 0 | observed |
| Use edge palette (`r_cv_edgepalette`) | 0 -> 1 | 18984 | 0 | observed |
| World edge colour (`r_cv_worldedge`) | 0 -> 13 | 18984 | 0 | observed |
| Bloom strength (`r_cv_bloom`) | 0 -> 0.15 | 9526 | 0 | observed |
| Bloom threshold (`r_cv_bloomthreshold`) | 0.5 -> 2 | 9526 | 0 | observed |
| Bloom radius (`r_cv_bloomradius`) | 0.5 -> 5 | 10269 | 0 | observed |
| World AO strength (`r_cv_ao`) | 0 -> 0.4 | 85059 | 0 | observed |
| World AO radius (`r_cv_aoradius`) | 2 -> 32 | 120694 | 0 | observed |
| Exposure (`r_cv_exposure`) | 0.5 -> 2 | 479935 | 0 | observed |
| Tone curve (`r_cv_tonemap`) | 0 -> 2 | 479985 | 0 | observed |
| Scene sharpness (`r_cv_sharpen`) | 0 -> 0.5 | 81676 | 0 | observed |
| Detail overlay (`gl_detail`) | 0 -> 1 | 475148 | 0 | observed |
| Water caustics (`gl_caustics`) | 0 -> 1 | 465297 | 0 | observed |
| FXAA preset (`vid_framebuffer_fxaa`) | 0 -> 17 | 50768 | 0 | observed |
| Texture filtering (`gl_texturemode`) | GL_NEAREST -> GL_LINEAR_MIPMAP_LINEAR | 426748 | 0 | observed |
| Model lighting (`r_cv_modellight`) | 0 -> 1 | 10357 | 0 | observed |
| Player rim (`r_cv_rim`) | 0 -> 0.2 | 28542 | 0 | observed |
| Rim width (`r_cv_rimwidth`) | 1 -> 8 | 45110 | 0 | observed |
| Upward bias (`r_cv_rimup`) | 0 -> 1 | 21663 | 0 | observed |
| Team tint (`r_cv_rimteam`) | 0 -> 0.3 | 16129 | 0 | observed |
| Model edge width (`gl_outline_scale_model`) | 0 -> 1 | 7093 | 0 | observed |
| Model edge opacity (`r_cv_modelopacity`) | 0 -> 1 | 3559 | 0 | observed |
| Player edge colours (`gl_outline_use_player_color`) | 0 -> 1 | 3115 | 0 | observed |
| Friend edge colour (`r_cv_teamedge`) | 0 -> 13 | 1754 | 0 | observed |
| Enemy edge colour (`r_cv_enemyedge`) | 0 -> 13 | 1811 | 0 | observed |
| Model shadows (`r_shadows`) | 0 -> 1 | 16420 | 0 | observed |
| MSAA samples (`vid_framebuffer_multisample`) | 0 -> 4 | 13195 | 0 | observed |

Observed: **48/48**.
