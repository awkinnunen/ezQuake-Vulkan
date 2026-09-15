# Local Arena

Implementation and documentation: OpenAI Codex, 2026-09-13, directed by the user.
Change identifiers: **ARENA-001**, **ARENA-002**. All text in this document is new local work.

Open **Local Arena** in the main menu, or enter `menu_local` in the console.
The main-menu entry uses the existing Quake big font, including nQuake's font
replacement. If no big font exists, all main-menu entries use the regular font
so the new action remains accessible without editing game assets.

KTX is a game module, not a standalone server executable. This workspace runs
it inside ezQuake's existing listen server. The menu starts/stops that server;
it does not launch another process. The installed native `qw/qwprogs.dll` is
preferred, with the existing packaged QVM fallback supported by the engine.

## Playing

1. Select **Map** and choose an installed map. Enter selects it; arrows scroll,
   Page Up/Down change pages, Home/End jump to the ends, and letters jump by
   initial. F5 rescans. Mouse selection is supported.
2. Use left/right on **Apply mode** to choose Free for all, Duel, 2 on 2 or
   Clan Arena. A mode selection resets the proposed deathmatch value to its
   preset: FFA = 3, Duel = 3, 2 on 2 = 1, Clan Arena = 5.
3. **Deathmatch** selects 1–5 with left/right. Its `live` value is the actual
   server cvar. Select **Start local server** to start with the chosen mode and
   deathmatch value automatically; it replaces the current game/connection.
   With a server running the action reads **Restart on selected map**. To change
   rules without restarting, press Enter on Apply mode (mode plus deathmatch)
   or Deathmatch (deathmatch only). KTX may refuse changes during a match.
4. Set **New bot skill** (1–20) with left/right. Select **Add bot** for each bot.
   The skill applies to newly added bots. **Remove last bot** and **Remove all
   bots** operate on the current game. The displayed count comes from server
   clients, not from a local click counter. KTX assigns bot teams automatically;
   an empty human team defaults to red when starting the server.
5. Select **Ready / start match** to ready the human player and return to play.
   KTX controls countdowns, team requirements and whether changes are allowed
   during a match. **Return to game** resumes without sending ready.
6. **Stop local server** shuts down the listen server and disconnects its local
   client, leaving ezQuake and the menu running. Back/Escape only leaves the
   menu; it does not stop a game.

Choose and apply a mode before adding bots. Bots need navigation support for
the selected map; listing a BSP does not certify bot navigation. DM4 and DM6
are exercised by the fixture. Registered Quake data is not added by this feature.

FFA uses deathmatch 3 in the installed KTX preset: weapons stay after pickup.
Deathmatch 1 uses weapon respawning instead. The startup hook applies the
selected rules only after the requested local KTX map connects, so server.cfg
and initial map rules cannot silently leave an FFA start at deathmatch 1.

The installed user's config now has `cl_onload "menu"`, replacing nQuake's
`sb_refresh` startup action, which opened the console. This is a one-setting
profile change; the engine's default was already `menu`. Config resets or
importing a different player config can override this preference.

## Implementation and boundaries

`menu_local.c` implements the menu and calls existing server, filesystem and
drawing APIs. The main-menu wiring is in `menu.c`; its scaled mouse selection
helper is shared with the new module. No KTX gameplay source or game assets
are modified by ARENA-001.

The map list enumerates the active virtual filesystem, including loose files,
PAK and PK3 files. It sorts and deduplicates up to 2,048 names. Map tokens must
contain only ASCII letters, digits, underscore or hyphen; nested paths and
console metacharacters are excluded. The selected map and a game module must
be readable before an existing game is disconnected.

Bot/mode/ready actions require an active local KTX server (`ktxver`, `qwprogs`,
PR2 enabled) and an active loopback client, with demo playback excluded. Stop
also requires a local KTX server. They do not target an external server or the
original single-player interpreter. KTX receives its normal commands, including
`carena` for Clan Arena. Settings are subject to KTX's existing rules and cfgs.

The first implementation does not provide remote server administration, a
separate MVDSV process, a team roster editor, per-bot skill editing, map images
or guaranteed navigation on every installed map. LAN guests and Internet
hosting are not exercised by these tests. Existing server networking applies.

## Validation and provenance

`scripts/Test-LocalArena.ps1` exercises the production keyboard handlers and
the installed KTX DLL in isolated profiles. It checks main-menu navigation,
map selection/filtering, four modes, real bot counts, removal, Clan Arena
countdown/game state, observed bot movement, shutdown, single-player isolation
and a second map startup. Screenshots are saved under the runtime profile.
User configs and game archives are not edited by the fixture.

`dev_local_menu` is a developer-only test driver. Its `checkpoint` action resets
the command-buffer runaway counters for scripted waits. Interactive key/mouse
input naturally gets an empty command buffer between actions. The fixture
starts after the initial connection because Host_Init flushes startup scripts
before normal frames, ignoring the intended timing of scripted waits.

The ordered source export is `patches/ezquake-07-local-arena.patch`, applied
after patches 01–06. `scripts/Source-Changes.json` records ARENA-001 separately
from campaign repair SP-001, including the shared `menu.c` file. Upstream author
notices remain; this work is uncommitted and has not been publicly published.
See VALIDATION.md and DEVELOPMENT.md for run evidence and initial failures.

ARENA-002 follows as `patches/ezquake-08-arena-deathmatch.patch`. It adds the
deathmatch row and a one-shot `CL_MakeActive` callback for applying chosen
local rules. The profile delta is recorded separately in
`runtime/arena-startup-setting.json`; personal config contents are not exported.


## MENU-UNIFY-001: current menu ownership (2026-09-15)

The old twelve-row custom Arena menu below is historical. Local Arena now uses
native settings widgets for preparing a new map/mode/rules/starting bots. Changing
setup does not modify the current game. Escape > Bots manages the current KTX
session, including supported remote servers. Server capability, permissions and
map navigation still determine whether requests succeed. See UNIFIED-MENUS.md.
Test-UnifiedArena.ps1 and Test-UnifiedOnline.ps1 supersede the old live-action row
coordinates in Test-LocalArena.ps1.
