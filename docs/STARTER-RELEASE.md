Unofficial nQuake-based installer for **Windows x64**, using the ezQuake Vulkan renderer.

Download **ezQuake-Vulkan-Starter-0.2.0.zip**, extract it and run **Install.cmd**.
Choose a new installation directory. After setup, run **Start.cmd** there.

- Downloads nQuake resources and the Vulkan engine from their official GitHub releases, with pinned SHA-256 checks.
- Starts with Balanced graphics and Quick WASD; all three graphics and keyboard presets are available.
- Preserves nQuake's normal config/autoexec behavior after first-run initialization.
- Optionally imports classic full Quake PAKs from your own installation.
- Installs separately and refuses to overwrite existing directories.
- Diagnose.cmd collects logs; the launcher waits correctly for the game to exit.

About 124 MB is downloaded. No existing nQuake installation, compiler or Vulkan SDK is needed.
This is Vulkan raster rendering, **not RTX/path tracing**. Local bots use nQuake's bundled KTX QVM.

Quick WASD/ESDF use the original five weapon crosshair PNGs and legacy
size 2.5, including LG hiding while firing. The Starter includes these images,
profiles and complete PowerShell source. Attribution/hashes are in `crosshairs.json`.

Engine build identity, verified download locations and corresponding source are
recorded in the included `downloads.lock.json` and `SOURCE.txt`.
Third-party licenses remain in force; do not redistribute an installed directory containing your full-game assets.

Tested on Windows/AMD integrated graphics: fresh install, single-player startup/save, DM6 bot arena,
graphics defaults, user overrides, diagnostics and installation-integrity checks.
See [Starter documentation](https://github.com/awkinnunen/ezQuake-Vulkan/blob/main/docs/STARTER.md).

Friends test release: start Local Arena with **Who can join -> Friends**, copy the
invitation and send it to your friend. They click or paste it and confirm. Setup
offers clickable Windows link registration; it is also available in the Friends
menu. Invitations persist until the host chooses Change invitation.

Both ends need this version and compatible map resources. No new relay or VPN
installation is required. Some NAT/firewall combinations remain unsupported;
there is no TURN fallback. Cross-city integrated gameplay is the purpose of this
test release; the earlier standalone probe succeeded for one network pair.
