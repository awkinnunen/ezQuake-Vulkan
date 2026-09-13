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
