# Attribution and provenance

## Raster follow-up and latest defaults (2026-09-15)

OpenAI Codex implemented the deferred scene, HDR/SSAO/MSAA, raster shadow and
multiview changes, including the user-reported DM6 baked-light correction.
Ordered patches 18–22 and provenance/source-changes.json identify these edits;
upstream notices and history are retained. The user selected the current
71 graphics and 55 conditional-effect defaults, packaged by Codex and verified
through the real engine. Earlier default values below are historical.


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


This project retains the complete ancestry of tibazera's Vulkan branch at
`91859a996daede0df166e2a55a5f08d56b052b21`. Original Git authors, committers,
copyright notices and licenses are retained.

| Contribution | Credit |
|---|---|
| Quake, QuakeWorld and inherited engine code | Original notices and Git history, including id Software, FuhQuake and ezQuake contributors |
| SDL3/Vulkan base | tibazera/ezquake-source, feature/sdl3-vulkan-pr and its recorded contributors |
| Changes above that base in this project's first source snapshot | Implemented by OpenAI Codex on 2026-09-13 under user direction |
| Feature priorities, approved visual values and requested control layout | User direction; no claim of human source review or implementation is implied |
| Legacy control aliases | Adapted from a user-customized FuhQuake config; earlier snippet authors are not established |
| Independent KTX gameplay patch | Codex C adaptations of GPL id Software rerelease QuakeC fixes; original source credit retained |

The first project commit groups the local development snapshot. It does not
reattribute the inherited code or fabricate individual historical commits for
each development step. The ordered engine patches in `patches/` and
`provenance/source-changes.json` distinguish local changes by ID and source path.
`docs/history/ATTRIBUTION.md` and `docs/history/DEVELOPMENT.md` preserve the English
development ledger. Their references to uncommitted changes and private test
paths describe the original development workspace, not additional published files.

The inherited `CONTINUE.md` documents earlier assistant-assisted work and remains
unchanged. Its references to Claude, Opus, Fable or an earlier Codex session do
not refer to this project's 2026-09-13 implementation session, nor do they establish
line-by-line authorship.

The renderer is derived from the ezQuake Vulkan fork. The local vkQuake-RT and
RTGL1 experiments are separate; their presence in the development notes does
not mean ray tracing has been merged here. Related standalone patches are
included as reference material, with their own pinned bases in `sources.lock.json`.

Commercial game data, user logs/demos, full personal configs, machine credentials
and custom crosshair images are excluded. This ledger records provenance; it
does not change licenses or transfer ownership.

Latest update: Explosion without the ring (11) is implemented alongside Big
explosion (7). Portable defaults match the latest saved settings. See
[the RT feasibility assessment](history/RTX-FEASIBILITY.md); RTX 3060 rendering
validation and the optional RT backend are still future work.

RT-PLAN-002: the user selected broad host/donor effect migration with a distinct
RT appearance. OpenAI Codex authored the source-reviewed
[implementation plan](history/RT-IMPLEMENTATION-PLAN.md) and related documentation.
This is planning work only; proposed adapters and RTGL1 extensions are not yet code.

## Optional RT foundation

OpenAI Codex authored the RT device/package adapter, SDL3 hardware harness,
CPU geometry adapters and tests. The vendored RTGL1 SDK header retains its MIT
notice and API; one comment dash was normalized to ASCII for portable patches.
RTGL1 changes are supplied as attributed patches against the pinned upstream
source. Existing RTGL1 code and shaders retain their original authorship.
This foundation is not a completed ezQuake RT gameplay renderer.
