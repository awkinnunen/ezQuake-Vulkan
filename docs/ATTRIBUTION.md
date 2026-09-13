# Attribution and provenance

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
