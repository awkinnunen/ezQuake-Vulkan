# Unified menus and presets

MENU-UNIFY-001 / CFG-PRESETS-001 / INPUT-003, 2026-09-15.
Requirements and Balanced tuning: user. Implementation, alternative preset tuning,
tests and this document: OpenAI Codex. The native widgets remain ezQuake's work.

Graphics uses the original Options widgets throughout. Categories are Image quality,
World and textures, Lighting and shadows, Players and weapons, Particles and effects,
Competitive clarity, and Camera and view. HDR, bloom and antialiasing belong to Image
quality, while AO and shadow maps belong to Lighting. Dependencies remain focusable
for help but cannot be adjusted until enabled. System retains display resolution,
fullscreen, renderer selection and audio; duplicate image controls were removed.

Use Options > Graphics > Browse graphics presets, or F2. Highlighting a CFG previews
it against the graphics state captured when browsing began. Press Enter or click
the highlighted preset to load it and return to Graphics. F3 (Keep preview) also
commits the preview without moving through other rows. Before committing, Cancel,
Escape, changing menu/tab or closing the menu restores the captured state.
After loading, leaving the menu keeps the loaded values. Invalid presets cannot
be committed. The old separate Apply workflow was corrected by CFG-PRESETS-002.
Missing values start from that baseline each time. Scope can restrict application
and saving to one graphics category. Pending HDR/MSAA changes are identified; F5
restarts video only after the preview has been applied or cancelled. Previewing a
preset never restarts video automatically.

Save current preset saves edits to an active user preset. Save As creates a variant.
Replacing a user preset preserves the old file as `.cfg.bak-1`, then `.bak-2`, etc.
Shipped presets are read-only in the browser. Graphics saves do not change the full
config's existing save-on-quit policy. Unsaved graphics edits are marked with `*`.

Managed presets live in `ezquake/presets/graphics/`; shipped presets are in its
`builtin/` subdirectory. Names contain letters, digits, underscores or hyphens.
These are declarative files: comments and allowlisted graphics values only. The
loader validates the entire file before applying it. Aliases, exec, binds, map and
connection commands, unknown/duplicate settings and invalid values are rejected.
Legacy full-config import and its keyboard preview remain available under Config.
The older `ezquake/competitive` files can also be previewed as graphics-only legacy
profiles. `cv load` remains compatible; `cv` and the old menu commands open the new UI.

Console commands: `gfx list`, `gfx load Balanced`, `gfx load Ultra-competitive`,
`gfx load Athmospheric`, `gfx save my-variant`. Console load applies immediately;
it still requires an explicit `vid_restart` when the menu reports pending video state.
Use these commands after engine initialization, such as in an explicit console load
or a launcher's `+exec` file. The early full-config pass precedes menu registration.

## Shipped presets

Each contains the same 170 graphics values, avoiding carryover between complete
presets. Hardware resolution, FOV-independent input, HUD, audio and server rules are
excluded. FOV and view effects are part of the Camera and view graphics category.

| Preset | Purpose |
| --- | --- |
| Balanced | Exact graphics values from the user's saved configuration on 2026-09-15. The default graphics profile. |
| Ultra competitive | Smooth world detail, brighter shadow response and clear player silhouettes. Disables HDR, bloom, AO, shadow maps, MSAA/FXAA and many decorative particles; retains recognizable geometry and projectile trails. |
| Athmospheric | Original material colours, linear HDR to SDR, emissive bloom, 32-sample SSAO, 512-pixel shadows with eight lights, 8x requested MSAA and anisotropic filtering. Keeps baked map illumination with entity shadows. |

Athmospheric deliberately retains the user's requested spelling. Its settings are
quality targets, not a claim of high FPS on every GPU. Requested MSAA can be limited
by the device. These are raster features; none require RTX. Server rules still
restrict player outlines and other controls. HDR monitor output is a separate TODO.

## Controls

Options > Controls offers Quick WASD (default), Quick ESDF and nQuake. Quick ESDF uses E forward,
D back, S left and F right. The layout modules also move the user's weapon/rocket-jump
aliases and nQuake team communication bindings. Graphics presets never load controls.
`keyboard_preset quick-wasd`, `keyboard_preset quick-esdf` and `keyboard_preset nquake` invoke the same actions as the menu.
The namespaced weapon actions and movement aliases are visible in Controls.

Portable profiles are under `runtime/public-profiles` in the development workspace,
or `profiles` in the publication tree. Copy the contents into the game directory.
After startup, `exec ezv-wasd-defaults.cfg` loads WASD and Balanced, then restarts
video. `exec ezv-defaults.cfg` loads graphics only. The development installation
preserves a private backup when migrating its active controls. No commercial assets
or complete personal configs belong in the public profiles.

## Local Arena and the running game

Local Arena prepares a new game: map, mode, deathmatch rules, initial bot count and
skill. Start new local game explicitly replaces the current session. Editing setup
values has no effect on an already running match. The map picker uses native widgets.

While playing, Escape > Bots handles adding/removing bots and selecting the skill of
new bots. Ready, Break, Join, Observe, Disconnect and Options remain in the in-game
menu. The KTX path works for both the embedded local server and an online server.
Availability requires a live connection, the server's `ktxver` information and its
server-origin `botcmd` alias. Every action sends a fixed KTX request; the server
checks administrative permission, game mode and map navigation support. Advertising
the command does not promise permission to use it. The menu reports that distinction
and leaves the server's result in the console. Existing `fbca` and demo/QTV menus
retain their separate protocol behavior.

Per-bot rosters, automatic navigation filtering and a generic preset editor for HUD,
audio and match rules remain follow-up work. They are not part of graphics previews.


## CFG-BALANCED-002 — saved user tuning, 2026-09-15 20:34

The user's newly saved configuration updates Balanced: world edge width 1.4 to
2.2, world edge opacity 0.4 to 0.3, midtone response 1 to 0.825, and gl_gamma 0.8
to 0.7. Tuning: user. Extraction, packaging and verification: OpenAI Codex.
Only these four of the 170 graphics fields changed; the installed and portable
Balanced copies match. The previous Balanced file is backed up privately.

Release fixture balanced-update-2034 validates all 170 values through native
preset activation, cancellation, invalid-file handling and audio/binding isolation.
See provenance/balanced-update-2034.json for hashes and the exact delta. Earlier
preset evidence describes the preceding Balanced revision. This successful save
does not establish the cause of CFG-PERSIST-001's earlier missing changes.


## INPUT-004 — nQuake, Quick WASD and Quick ESDF (2026-09-15)

At the user's request, Controls now offers Quick WASD (default), Quick ESDF and
nQuake. The old SDFE label meant the same E-forward/D-back/S-left/F-right layout;
its displayed name is now Quick ESDF. Commands are keyboard_preset quick-wasd,
keyboard_preset quick-esdf and keyboard_preset nquake. Existing wasd/sdfe commands
and ezv-wasd.cfg/ezv-sdfe.cfg filenames remain compatible; esdf is also accepted.

nQuake uses 102 bind declarations and 20 helper aliases extracted from the cached
official distribution's non-gpl.zip (qw/nquake_default.cfg and qw/autoexec.cfg).
Original bindings/aliases: nQuake contributors. Quick behavior: the user's legacy
FuhQuake setup, with earlier snippet authors unknown. Selection code, extraction,
packaging and tests: OpenAI Codex. Exact source hashes are recorded in
provenance/keyboard-presets.json. No claim of new authorship over upstream aliases.

nQuake explicitly replaces all bindings, including otherwise unused keys. WASD
moves; Mouse 1 attacks with the current weapon; Mouse 2 selects lightning; E/Q
select rocket/grenade; Mouse 4/5 select nailgun/shotgun preferences. Team, timer,
demo and volume keys retain their original commands. The loader removes only the
Quick-owned f_weaponchange hook; other hooks survive. A Quick-hidden crosshair
is made visible when that hook is detached. Existing crosshair appearance is
otherwise retained. Quick layouts retain their weapon, jump and crosshair aliases.

Selection leaves the 170 graphics values, sensitivity and audio values unchanged.
The user's config.cfg and autoexec.cfg were neither rewritten nor executed by the
installation step. No automatic startup-load policy was added in this change.
Debug/Release fixtures verify native nQuake/Quick ESDF menu activation, Quick WASD,
legacy command compatibility, exact nQuake binding restoration across repeated
switches, timer aliases, graphics/input isolation and unrelated alias preservation.
The native Controls screen was inspected at 640x480. See the provenance record.
