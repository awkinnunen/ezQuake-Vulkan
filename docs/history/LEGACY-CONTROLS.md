# Legacy ESDF and WASD controls with nQuake helpers

INPUT-001, 2026-09-13. Adapted and installed by OpenAI Codex at the user's request.
The user identified their rocket-jump and weapon/crosshair scripts, selected the
old ESDF layout and requested nQuake communication/helper commands alongside it.
Conflicting communication letters were shifted one column right as requested.

## Activation and profiles

The active `gamedata/nquake/ezquake/configs/config.cfg` retains its complete
previous contents, followed by the imported controls. Existing launchers use
this on the next start. The self-contained `legacy-esdf.cfg` profile is available
in **Options > Config > Browse configs / key map**; its literal bindings also
appear in the keyboard preview. Runtime changes made by movement aliases are
not evaluated by the preview.

`nquake-before-legacy.cfg` is the complete pre-import profile and restores the
previous controls/settings through the config browser. It is a snapshot, so
loading it also restores other settings to their pre-import values.

To apply only the imported control layer to an already running game, use
`exec legacy-controls.cfg`. This module is in `gamedata/nquake/qw/`; use `exec`,
not `cfg_load`, for this partial configuration. nQuake's weapon aliases remain
defined, but the imported firing buttons use separate `legacy_*` aliases.
No `unbindall`, graphics preset, network setting or remote connection is imported.

## Movement and weapons

| Key | Action |
|---|---|
| E / D / S / F | Forward / backward / strafe left / strafe right |
| Space | Jump |
| A | Moving forward/back: vertical rocket jump; stationary: the old speed-jump sequence |
| Mouse 1 | Fire RL, falling back through the user's old weapon preference list |
| Mouse 2 | Fire LG with the user's fallback order; hide the custom LG crosshair while held |
| Mouse 3 | Fire GL with the user's fallback order |
| Keypad * | Toggle returning to shotgun on firing-button release; initially off |
| Keypad / | nQuake `match_save`, moved from Keypad * |

Five byte-identical PNGs from the old installation use new `legacy_sg`,
`legacy_ng`, `legacy_gl`, `legacy_rl` and `legacy_lg` names, preserving existing
nQuake crosshairs. SG/SSG share one image; NG/SNG share another. Axe clears the
custom image. The old colour, alpha and vertical offset are retained. CV-TUNE-001 reduces
`crosshairsize` from 8 to 2 at the user's request (one quarter of the width/height).
Idle LG now consistently shows its LG image; holding its fire alias clears it.

The jump scripts retain the old frame-dependent `wait` timing. The adaptation
restores `cl_pitchspeed` afterward and handles overlapping forward/backward
holds when selecting A's action. `allow_scripts 2` enables the required
multi-command input and `rotate`; existing server/ruleset restrictions still
apply. No restriction bypass is added.

## nQuake communication and helpers

| Key | nQuake command |
|---|---|
| G | Safe location (`tp_msgsafe`, formerly F) |
| H | Request help (`tp_msghelp`, formerly G) |
| T | Pointed item/player (`tp_msgpoint; shownick`, formerly R) |
| Y | Lost location (`tp_msglost`, formerly T) |
| V | Coming (`tp_msgcoming`, formerly C) |
| B | Request replacement (`tp_msgreplace`, formerly V) |
| U / I | Team chat / public chat, formerly Y / U |
| Ctrl / Shift | Took item / status report, unchanged |
| 1 / 5 | Quad dead / enemy powerup, unchanged |
| F1–F4 | Quad/pent requests and timers, unchanged |

The old W/C/R bindings are cleared after their functions move. Console help
from `teamsays` uses the new letters. Ready/break/join/observe, scoreboard,
screenshots, volume and demo controls remain available. Communication commands
were inspected as bindings and retained definitions; automated tests do not
send team/public chat messages.

## Source and verification

The old launchers load `configs/wilhos.cfg`, which includes
`configs/my/fuh.cfg`. Imported logic comes from its lines 60–84, 264–300 and
317–325. The user customized that file; original snippet/PNG authors have not
been independently established. Codex's work is extraction, namespacing,
compatibility adjustments, rebinding, tests and documentation, not creation of
the inherited scripts/images. Exact source/image hashes and configuration
backups are recorded privately in `private-controls-import.json` and
`cache/legacy-controls-backup/`.

The NetQuake client parser previously assigned active-weapon stats directly,
skipping `f_weaponchange`. The small engine change routes both standard and
nonstandard weapon encodings through the existing `CL_SetStat` path. The
unchanged-stat guard remains. `cl_nqdemo.c` retains tonik's copyright notice;
this local integration change is by OpenAI Codex. See patch 10 and INPUT-001
in the source provenance manifest.

`scripts/Test-LegacyControls.ps1` starts an isolated game using a snapshot of the
executable, the installed nQuake profile and private crosshair copies. It checks
seven real weapon switches, five image loads, LG hold/release, shotgun return,
movement-dependent jump bindings and restoration of keyboard look speed.
Trigger-driven stages let the normal command queue drain; a long queued test
script would delay event callbacks until its end. See VALIDATION.md for tested
builds and known test-harness failures.

## CV-TUNE-001 defaults

The active config, legacy-esdf.cfg and the partial legacy-controls.cfg now set
`w_switch 8` and `b_switch 8`, matching the engine's Gun Autoswitch enabled state
for weapon and backpack pickups. The server's weapon ranking/rules determine
the automatic pickup choice. The legacy mouse firing preferences remain intact;
`cl_weaponpreselect` is unchanged, so they are not replaced by a universal
weapon order on every click. `crosshairsize 2` replaces the imported size 8.

## CV-002 — Crosshair size follow-up

On 2026-09-13 the user requested a small increase after trying size 2. The active
default is now crosshairsize 2.5, 25% larger in width and height. This is applied
to config.cfg, legacy-esdf.cfg, legacy-controls.cfg and visual-defaults.cfg.
In a running client, enter crosshairsize 2.5. Weapon-specific images, pickup
autoswitch, aliases, bindings and all other config bytes are unchanged. Backups
and before/after hashes are recorded in private-competitive-widgets.json.

## INPUT-002 - Additional WASD profile

`legacy-wasd.cfg` is available alongside `legacy-esdf.cfg` in **Options > Config >
Browse configs / key map**. Select it and load to switch; load legacy-esdf.cfg to
return. Creating this alternative does not change the active ESDF configuration.
Both full profiles include the approved project visual defaults (CV-DEFAULT-001).

The WASD variant shifts alphabetic controls one key left on the usual QWERTY
letter rows. Numbers, punctuation, Tab, Enter, Space, modifiers, function keys,
navigation, keypad and mouse keep their original actions. Two boundary cases are
explicit: A's jump action moves to Caps Lock; Q's spare grenade selection moves
to unused Z so Tab retains the scoreboard. The old Z binding was empty.

| ESDF key | WASD key | Action |
|---|---|---|
| E / D / S / F | W / S / A / D | Forward / backward / left / right |
| A | Caps Lock | Movement-dependent rocket/speed jump |
| G / H | F / G | Safe / help |
| T / Y | R / T | Point / lost |
| V / B | C / V | Coming / replacement |
| U / I | Y / U | Team / public chat |
| Q | Z | Spare grenade-launcher selection |

Movement aliases rebind Caps Lock, never the new A strafe key. Weapon/crosshair
aliases and mouse preferences are retained, and nQuake teamsays help reflects
the WASD letters. A final literal binding block resolves empty-key collisions
and keeps the config-browser preview accurate. New profile packaging/remapping:
OpenAI Codex, 2026-09-13, at user request; inherited script authorship is unchanged.
Exact source/output hashes and mapping: private-wasd-profile.json.


## INPUT-003: WASD default, SDFE alternative (2026-09-15)

The user selected WASD as default. SDFE is the alternative name for the original
E-forward/D-back/S-left/F-right layout (previously called ESDF in these notes).
Both are available in native Controls and as portable modules. Switching them
also moves weapon/rocket-jump aliases and communication bindings; graphics presets
leave controls untouched. Packaging and migration: OpenAI Codex. The active private
configuration is backed up before installing the WASD overlay.


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
