ezQuake Vulkan - Windows x64 public test package

1. Install nQuake separately, or use your existing nQuake installation.
   No Quake PAKs, textures, models, sounds, particle packs or private configs
   are included. nQuake: https://nquake.com/
2. Extract this ZIP to its own writable folder. Do not put it in Program Files.
3. Double-click Start.cmd and enter your nQuake folder when asked.
   You can also run: Start.cmd "C:\Games\nQuake"
   The folder must contain id1/pak0.pak and ezquake/ezquake.pk3.

The engine runs from this package against your installed data; it does not
replace your existing engine. A current Vulkan-capable graphics driver is
required. The Vulkan SDK and Visual Studio are not needed. This x64 Release
build statically links its non-system runtime libraries.

First run and saved configurations
----------------------------------
On first use, Balanced graphics and Quick WASD provide startup defaults BEFORE
the normal default/config/autoexec loading chain. Existing user values win.
The original config.cfg and autoexec.cfg are not edited by setup. Normal engine
config saving still follows cfg_save_onquit. Your autoexec remains authoritative.
After a successful exit, setup records an ezquake/ezv-distribution.json marker;
later starts do not reapply package defaults. Existing preset files are preserved.
Profiles and the temporary startup layer use ezv- names and separate directories.

Graphics: Options > Graphics, F2 opens presets. Highlight previews; Enter/click
loads; Escape cancels. F5 restarts video if the menu says it is required.
Balanced preserves the project's current tuning. Ultra competitive reduces
visual cost. Athmospheric requests higher raster quality and may run much slower.
Old full configs can override Balanced on startup; select it explicitly if desired.

Controls: Options > Controls offers Quick WASD, Quick ESDF and nQuake.
Quick presets use weapon-specific mouse firing and rocket-jump aliases. nQuake
uses Mouse 1 to fire the selected weapon and replaces all keyboard bindings.
Graphics presets do not change controls. Console commands remain available:
  keyboard_preset quick-wasd
  keyboard_preset quick-esdf
  keyboard_preset nquake
  gfx load Balanced
  cfg_save

Local play / limitations
-----------------------
This is the Vulkan raster renderer, not RTX/path tracing. No RT harness is bundled.
Local Arena requires your existing KTX game module and map/bot support. The package
does not replace qwprogs.dll or install a game server. Original Quake single-player
requires appropriate game data; nQuake's shareware data covers the first episode.
Validation is bounded Windows/AMD testing, not all GPUs or a tournament approval.
Known issues and test details are in docs/BUGS.md and docs/VALIDATION.md.

Problems: run Diagnose.cmd. Attach the generated logs folder, GPU/driver details,
map, steps to reproduce and whether the problem happens with Balanced.
Report at https://github.com/awkinnunen/ezQuake-Vulkan/issues

Source, build instructions and attribution
-----------------------------------------
https://github.com/awkinnunen/ezQuake-Vulkan
manifest.json identifies the exact corresponding source commit and binary hash.
The matching source ZIP is provided alongside this package. See LICENSE and
notices/ for ezQuake and dependency notices, and docs/ATTRIBUTION.md for credits.
nQuake binding/alias data retains nQuake contributor attribution; Quick layout
preferences originate in the user's legacy configuration. Packaging: OpenAI Codex.
