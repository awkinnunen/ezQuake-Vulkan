# Keyboard presets

Options > Controls: **Quick WASD (default)**, **Quick ESDF**, **nQuake**.
Use `keyboard_preset quick-wasd`, `keyboard_preset quick-esdf` or
`keyboard_preset nquake`. Old `wasd` and `sdfe` spellings remain supported.
nQuake replaces all bindings with its original controls; Quick layouts include
weapon-specific firing and rocket-jump aliases. Graphics and sensitivity stay
unchanged. See [current details](history/UNIFIED-MENUS.md#controls).

> Updated 2026-09-15: the current native menu/preset behavior is documented in
> [Unified menus](history/UNIFIED-MENUS.md). Graphics presets contain 170 values;
> Quick WASD is default; Quick ESDF and nQuake are alternatives. Earlier menu coordinates and the
> 53-setting profile counts below are historical. Use the current
> [TODO](history/TIBAZERA-TODO.md) and
> [validation evidence](../provenance/unified-menus-validation.json).

# Portable WASD controls

Prepared by OpenAI Codex on 2026-09-13 from the user's requested legacy controls.
The rocket-jump and weapon preferences originate in the user's old FuhQuake
configuration; earlier snippet authors are not known. This records provenance
without asserting that the user or Codex invented those techniques.

Load `exec ezv-wasd.cfg` after your nQuake configuration. It is an overlay, not
a complete replacement config. It sets scripting/triggers for the movement and
weapon aliases; server rules may restrict scripted actions.

| Key | Action |
|---|---|
| W / A / S / D | Forward / left / back / right |
| Space | Jump |
| Caps Lock | Movement-dependent rocket jump |
| Mouse 1 / 2 / 3 | Rocket launcher / lightning gun / grenade launcher fire |
| Z | Select grenade launcher |
| F / G | Report location safe / request help |
| R / T | Report pointed item or player / report lost location |
| C / V | Report coming / request replacement |
| Y / U | Team / public chat input |
| Keypad * | Toggle return to shotgun after firing |
| Keypad / | Save match |

Letters move one column left from the ESDF profile. Boundary keys use Caps Lock
and Z; number keys, modifiers, Tab, function keys, keypad and wheel retain the
installed nQuake bindings except the two explicitly listed keypad assignments.
Communication is only sent when the player presses its binding.

The public module uses built-in weapon-specific crosshairs, size 2.5. The local
development installation retains the user's custom PNG variants; their origin
and redistribution terms were not established, so those images are not included.
Lightning crosshair hiding while firing and the weapon-change trigger remain.
`w_switch 8` and `b_switch 8` select the strongest weapon on pickup automatically.

`exec ezv-defaults.cfg` loads the approved graphics independently.
`exec ezv-wasd-defaults.cfg` loads both and applies the pending video restart.
The WASD module itself does not change the 53 graphics profile values.


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
