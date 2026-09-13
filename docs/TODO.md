# Next work

Priorities and status recorded by OpenAI Codex under user direction, 2026-09-13.

- [x] Clean unsupported legacy config values while preserving approved controls/visuals.
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
