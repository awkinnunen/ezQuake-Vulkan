> Updated 2026-09-15: the current native menu/preset behavior is documented in
> [Unified menus](history/UNIFIED-MENUS.md). Graphics presets contain 170 values;
> Quick WASD is default; Quick ESDF and nQuake are alternatives. Earlier menu coordinates and the
> 53-setting profile counts below are historical. Use the current
> [TODO](history/TIBAZERA-TODO.md) and
> [validation evidence](../provenance/unified-menus-validation.json).

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
- [x] Plan direct RTGL1 integration with broad effect coverage and independent RT visuals (RT-PLAN-002).
- [ ] RT-00: inventory host/donor features and validate dependency, ABI and device requirements.
- [ ] RT-01: render the SDL3 triangle/light harness on RTX 3060.
- [ ] RT-02: integrate backend lifecycle, 2D rendering and fallback.
- [ ] RT-03: add static maps, materials, sky and lighting.
- [ ] RT-04: add animated entities, skins, viewmodel and dynamic lights.
- [ ] RT-05: port host/donor effects, including particles, shaft, explosions, water and HUD.
- [ ] RT-06: implement Competitive Visuals/shader extensions and RT settings/profiles.
- [ ] RT-07: test temporal history, QW/demos, capture and multiview handling.
- [ ] RT-08: benchmark RTX 3060, verify resource lifetime and package the optional runtime.

OpenGL/Vulkan appearance parity is deferred at the user's request.
The engine currently renders with Vulkan rasterization; RTX hardware alone does
not turn on ray tracing. See [the technical plan](history/IMPLEMENTATION-PLAN.md)
and [the assessment of inherited work](history/TIBAZERA-TODO.md) for context.
The active RT sequence and acceptance gates are in
[RT-PLAN-002](history/RT-IMPLEMENTATION-PLAN.md), superseding the older RT numbering.
All RT implementation stages remain open; functional effects are required,
while different RT lighting and postprocessing appearance are acceptable.

RT foundation update: device/package/API checks and the standalone triangle/light
harness are implemented, together with central RTGL1 initialization cleanup.
The first RTX 3060 image test is pending; host adapters and styling remain open.
See [the implementation status](history/RT-IMPLEMENTATION-STATUS.md).

Latest update: Explosion without the ring (11) is implemented alongside Big
explosion (7). Portable defaults match the latest saved settings. See
[the RT feasibility assessment](history/RTX-FEASIBILITY.md); RTX 3060 rendering
validation and the optional RT backend are still future work.

- [x] CFG-PRESETS-002: load the focused graphics preset with Enter/click; F3 keeps preview, Escape cancels. Native input and save regression verified in Debug/Release.

- [x] INPUT-004: nQuake keyboard preset plus Quick WASD/Quick ESDF names; native menu and switch-isolation tests pass in Debug/Release.


## Friends hosting through existing infrastructure (2026-09-16)

- [x] FRIENDS-001: investigate FTE rooms, ICE, STUN, relay selection, public room
  visibility and current QW proxy boundaries. Record the UX and technical plan in
  [FRIENDS-HOSTING-PLAN.md](history/FRIENDS-HOSTING-PLAN.md). Research only; no engine changes.
- [x] FRIENDS-002A: implement the native FTE-broker / ICE / pinned-DTLS prototype;
  Debug/Release, real-broker local pair, wrong-key and wrong-fingerprint tests pass.
- [x] FRIENDS-002B: package a standalone Windows two-computer test with matching
  source, dependency notices, Host/Join launchers and sanitized results.
- [x] FRIENDS-002: user supplied a successful direct probe result and confirmed
  the computers were in different cities (100/100 packets, mean RTT 31.7121 ms).
  This is one network pair; reversed roles, more NAT types and TURN remain open.
- [x] FRIENDS-003: logical peer addresses integrated with embedded QW/KTX;
  two simultaneous guests, rejoin, chat and DM6 -> DM2 sign-on tested.
- [x] FRIENDS-004: pinned encrypted admission, close/reopen, secret rotation,
  guest removal and private-server UDP/TCP gate. Latest scope uses full links;
  code-only host approval is deferred to keep the first workflow simple.
- [x] FRIENDS-005A: Local Arena access selector, main/in-game Friends menus,
  clipboard joining with confirmation, help text and unavailable-action policies.
- [x] FRIENDS-005B: opt-in per-user Windows URI registration, native cold start and
  profile-specific running-instance handoff, with confirmation and secret redaction.
- [x] FRIENDS-005C: Starter offers optional per-user link registration after installing
  the final executable/data directory; in-game registration remains available.
- [ ] FRIENDS-006: validate outages, lifecycle and eligible existing TURN fallback.
- [ ] FRIENDS-007: package and document the measured network coverage.

No new external relay is part of this scope. Existing STUN/broker reachability is
confirmed; cross-city engine gameplay and relay availability remain unverified. Room
codes may be public and must not act as passwords. See the plan's acceptance gates.

Prototype details and pending gates: [Friends hosting status](history/FRIENDS-HOSTING-STATUS.md).

- [x] FRIENDS-002C: unlimited host waiting in probe 0.1.1, periodic broker keepalive,
  stable invitation, reduced idle polling and retained explicit test deadlines.

- [x] FRIENDS-004P: generate and privately save one reusable invitation and host
  identity; preserve it across restarts, matches and upgrades. Change invitation
  replaces the access secret and invalidates old links. No MAC or rotation counter.
  Handle broker room conflicts without promising offline name reservation.

Current player instructions: [Friends games](FRIENDS.md).
- [ ] FRIENDS-VOICE: validate inherited ezQuake VoIP through Friends using two microphones; no new voice implementation is needed before this check.
