ezQuake Vulkan Starter 0.1.0
Unofficial nQuake-based installation for Windows x64.

Extract the Starter ZIP, run Install.cmd, and choose a NEW installation folder.
Setup downloads the pinned nQuake packages and ezQuake Vulkan 0.1.0-beta.1.
An existing nQuake installation is not needed. Internet access is required for
downloads; verified cached downloads are reused. No administrator account,
compiler, Python or Vulkan SDK is required. Install a current graphics driver.

Optionally provide your own classic full Quake folder (or its id1 directory).
Setup imports numbered pak1.pak and later PAK files only after checking for
registered-game content. This also supports repacked numbered PAK sets. It
does not import personal configurations, executables, music or other addons.
It retains the installed shareware pak0.pak and nQuake's graphical improvements.
Substitute GPL maps are retained as gpl_maps.pk3.disabled when full maps are
imported. The rerelease's different asset format is not supported by this import.
Without an import, shareware Episode 1 and nQuake's multiplayer content are used.

In the installed folder, run Start.cmd. Diagnose.cmd starts windowed and copies
the console log into engine/logs. Start.cmd -Windowed is also supported.
Balanced graphics and Quick WASD are the fresh-install defaults. Options offers
Ultra competitive / Athmospheric graphics and Quick ESDF / nQuake controls.
The original nQuake autoexec.cfg is preserved. Its normal one-time preset.cfg
mechanism initializes the defaults; later saved config and manual autoexec edits
take precedence. Setup refuses existing destinations and does not launch the game.
The whole installed directory can be moved later; launchers use relative paths.

This is Vulkan raster rendering, not RTX/path tracing. Local Arena uses nQuake's
bundled KTX module, not the development workspace's separately patched native DLL.
The engine version is pinned: Starter does not silently install future builds.

Downloads are SHA-256 and size checked. If upstream replaces a snapshot and its
hash changes, installation stops; obtain an updated Starter rather than disabling
verification. An interrupted install leaves a .ezv-install-* work directory beside
the requested destination. Verified downloads remain in
%LOCALAPPDATA%/ezQuake-Vulkan/downloads for retry. No existing installation is changed.

Sources and licenses:
https://github.com/awkinnunen/ezQuake-Vulkan
https://github.com/awkinnunen/ezQuake-Vulkan/releases/tag/v0.1.0-beta.1
https://github.com/nQuake/distfiles/releases/tag/snapshot
https://github.com/nQuake/client-win32

This Starter ZIP contains installer code and its download manifest, not game data.
Downloaded components retain their separate licenses and authorship. Quake game
assets are not relicensed under the engine's GPL. See installed licenses,
LICENSE, engine/LICENSE, engine/notices and engine/docs/ATTRIBUTION.md. Engine
source is available from the exact release above. Full-game assets are supplied
only by the user and must not be included in a public redistribution of the
installed directory. This is not an official nQuake, id Software or Bethesda release.

Installer implementation and tests: OpenAI Codex, 2026-09-16, under user direction.
Engine and resources: their respective upstream authors; original notices retained.
