Unofficial nQuake-based installer for **Windows x64**, using ezQuake Vulkan 0.1.0-beta.1.

Download **ezQuake-Vulkan-Starter-0.1.0.zip**, extract it and run **Install.cmd**.
Choose a new installation directory. After setup, run **Start.cmd** there.

- Downloads nQuake resources and the Vulkan engine from their official GitHub releases, with pinned SHA-256 checks.
- Starts with Balanced graphics and Quick WASD; all three graphics and keyboard presets are available.
- Preserves nQuake's normal config/autoexec behavior after first-run initialization.
- Optionally imports classic full Quake PAKs from your own installation.
- Installs separately and refuses to overwrite existing directories.
- Diagnose.cmd collects logs; the launcher waits correctly for the game to exit.

About 122 MB is downloaded. No existing nQuake installation, compiler or Vulkan SDK is needed.
This is Vulkan raster rendering, **not RTX/path tracing**. Local bots use nQuake's bundled KTX QVM.

The Starter archive contains its complete PowerShell source and no game resources.
The pinned engine and its matching source are in the [engine beta release](https://github.com/awkinnunen/ezQuake-Vulkan/releases/tag/v0.1.0-beta.1).
Third-party licenses remain in force; do not redistribute an installed directory containing your full-game assets.

Tested on Windows/AMD integrated graphics: fresh install, single-player startup/save, DM6 bot arena,
graphics defaults, user overrides, diagnostics and installation-integrity checks.
See [Starter documentation](https://github.com/awkinnunen/ezQuake-Vulkan/blob/main/docs/STARTER.md).
