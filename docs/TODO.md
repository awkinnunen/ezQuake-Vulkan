# Next work

## MAINT-005 correction: conditional particle settings (2026-09-13)

OpenAI Codex incorrectly classified 55 settings as unsupported during MAINT-001.
All 55 are registered by InitVXStuff when QMB particle assets initialize. The
earlier isolated fixture lacked the ezQuake particle resource pack; its unknown
command messages did not establish missing engine support. Removing those
settings reset enhanced lightning and other effects. The removal has been reversed
from the per-file backups, and the destructive cleanup script is disabled.

The user's requested bloom restoration is 0.1 strength, 0.7 threshold and 2.75 radius.
Lightning is restored to 1 with sparks 0.4. Other current graphics and bindings
are preserved. The public defaults include a separate 55-setting particle module;
the visual-only profile still contains 53 values. Runtime fixtures now include
the installed ezquake.pk3 before initialization. Earlier claims that these 55
settings were unsupported or safe to remove are superseded by this correction.

Debug weapon/shaft fire and release tests pass with the restored settings.
The combined public profile test verifies all 53 visual values and all 55 conditional
values after saving through the real engine. Engine source/binaries are unchanged.
Earlier performance measurements used the former bloom/particle state and are
historical evidence, not measurements of the restored defaults.

Menu paths: Options > Graphics > Visual Effects > Effects for Bloom strength,
threshold and radius; Options > Graphics > Advanced Options > Lighting > Particle
Shaft for the enhanced lightning switch. Lightning color, size and sparks do not
yet have dedicated menu rows.


Priorities and status recorded by OpenAI Codex under user direction, 2026-09-13.

- [x] Restore supported conditional effects and include the particle assets in runtime fixtures (MAINT-005).
- [x] Fix screenshot ownership immediately after swapchain recreation.
- [x] Validate moving demos, seeking, map/restart transitions and bot entity lifetime.
- [x] Measure selected effect costs and check the historical timedemo/VSync report.
- [x] Package WASD controls, all 53 approved visual defaults and authorship records.
- [ ] Broaden GPU/driver and long-session tests, including automatic match and movie captures.
- [ ] Complete campaign and player-recognition testing across more maps.
- [ ] Investigate remaining texture lifetime reports when reproducible.
- [ ] Establish an optional RT backend interface and capability/fallback rules before RTGL1 integration.
- [ ] Implement and validate static/dynamic geometry, material mapping and light extraction for RT.
- [ ] Measure the RT path, resource lifetime and fallback behavior on suitable hardware.

OpenGL/Vulkan appearance parity is deferred at the user's request.
The engine currently renders with Vulkan rasterization; RTX hardware alone does
not turn on ray tracing. See [the technical plan](history/IMPLEMENTATION-PLAN.md)
and [the assessment of inherited work](history/TIBAZERA-TODO.md) for context.

Latest update: Explosion without the ring (11) is implemented alongside Big
explosion (7). Portable defaults match the latest saved settings. See
[the RT feasibility assessment](history/RTX-FEASIBILITY.md); RTX 3060 rendering
validation and the optional RT backend are still future work.
