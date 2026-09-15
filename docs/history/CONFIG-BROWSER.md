# Config browser and binding map

Implemented by OpenAI Codex at the user's request, 2026-09-13 (CFG-001).

Start `Start-Config-Browser.cmd`, or use **Options > Config > Browse configs /
key map** in the game. The console command is `menu_configs`.

The default directory in this installation is `gamedata/nquake/ezquake/configs/`.
Put `.cfg` files directly in that directory. Subdirectories, archives and other
extensions are excluded. The browser respects `cfg_use_home` and
`cfg_use_gamedir`, using the same destination directory as config saving; its
header shows the actual directory. It does not search several directories at once.

The installed `legacy-esdf.cfg` and `legacy-wasd.cfg` provide both control layouts.
See LEGACY-CONTROLS.md for the left-shift mapping and boundary-key exceptions.

## Controls

- Up/Down or the file list: select a config. Typing searches the list.
- Click a key or mouse button: inspect its command.
- Left/Right: cycle through every bound key, including keypad and auxiliary keys
  that are not drawn on the main keyboard.
- Shift+Left/Right: scroll a long binding command.
- Enter or **Load config**: load the selected file.
- F5: reread the directory and the selected file after editing it externally.
- Escape or right click: return to config settings.

Highlighted keys have a binding declared in the file. **Unbound** means an
explicit `unbind`, empty `bind`, or `unbindall`; **Not set in this file** means
there is no declaration. The diagram uses engine key names at US keyboard
positions, with ISO shown separately. It is not a physical Finnish-layout diagram.
Modifier groups (CTRL, ALT, SHIFT and WIN) follow the engine's left/right rules.

Previewing never executes the config or changes active bindings. It reads literal
`bind`, `unbind`, `unbindall` and `con_bindphysical` declarations, in order,
including quoted command sequences, comments and semicolon-separated statements.
It does not evaluate aliases, conditions, variable expansion, included configs,
or brace syntax. It therefore shows declarations in the selected file, not a
prediction of the complete state after execution. A warning reports declarations
that cannot be previewed. Keypad/extra bindings remain accessible with Left/Right.

Loading uses the exact selected path, avoiding a same-name file in another
directory. It follows config-loading reset/default/autoexec and `f_cfgload`
semantics. Files are read completely before resetting; missing/unreadable files,
embedded NULs, files over 1 MiB and files that cannot fit the command queue are
rejected. Previewing a file does not approve or execute its commands; pressing
Load is the explicit execution action. Graphics settings in a loaded config can
still require the engine's usual `vid_restart`.

## Validation

`scripts/Test-ConfigBrowser.ps1` compiles and exercises the actual parser with
edge cases and 1,000 deterministic randomized 30-command sequences. Checks cover
ordering, quoted semicolons/comments, duplicates, queries, unbinds, UTF-8 BOM,
unsupported scripts, invalid keys, oversized statements, embedded NUL and
physical-mode changes.

`scripts/Test-ConfigBrowserRuntime.ps1` creates an isolated profile and starts
the actual engine. It verifies two CFG files (including uppercase extension),
exclusion of TXT/ZIP/subdirectories, unchanged active bindings during preview,
file selection, keyboard hit testing, left/right CTRL expansion, exact-path
loading with a conflicting same-name file in `qw/`, and normal exit. It also
has an empty-directory test. User configs and game packages are not modified.

Vulkan Debug/Release, Modern OpenGL Debug and the empty-directory Vulkan run
passed. Screenshot captures were inspected. Evidence is under `cache/`:
`cfg-browser-parser-final.log`, `cfg-runtime-verified.log`,
`cfg-runtime-empty.log`, `cfg-runtime-opengl.log`, `cfg-runtime-release.log`.
The final parser comment-handling adjustment is additionally covered by the
final builds and `cfg-runtime-final.log`.

The first runtime attempt failed because the inherited file list needs at least
80 logical pixels of height; the layout now reserves that space. Failed logs are
retained. Tests use simulated UI actions through a development-only driver and
actual engine rendering/loading; exhaustive physical mouse/keyboard testing and
all desktop/layout combinations are not claimed.

Source patch: `patches/ezquake-05-config-browser.patch`, applied after patches
01 through 04. Full source paths are listed under CFG-001 in
`scripts/Source-Changes.json`; upstream authorship is preserved.

INPUT-001 adds two complete selectable profiles: legacy-esdf.cfg combines the
old ESDF/weapon/RJ/crosshair controls with the installed nQuake helpers, and
nquake-before-legacy.cfg preserves the pre-import configuration. Unlike the
partial qw/legacy-controls.cfg overlay, these files are suitable for browser
loading. See LEGACY-CONTROLS.md for their bindings and rollback semantics.


## 2026-09-15: graphics presets and Controls

The native Graphics menu now has a separate declarative preset browser with live
preview, category scope, Apply/Cancel and safe saving. Balanced, Ultra competitive
and Athmospheric are complete graphics profiles. Controls offers WASD (default)
and SDFE independently. This supersedes earlier statements that all CFG browsing
was limited to a static keyboard preview. The original full-config browser retains
that explicit import workflow. See UNIFIED-MENUS.md for paths and save behavior.
