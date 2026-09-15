# Reproducing the runtime checks

Build Debug and Release first. Use PowerShell and Python 3. Tests create isolated
profiles in ignored `cache/`; installed asset files are shared as hard links,
and user configs are copied, not edited. No test fixture or demo is bundled.
The game-data directory and checkout must be on a volume supporting hard links.
Run tests sequentially because this client has a single-instance guard.

```powershell
$env:EZQUAKE_GAME_DIR = 'C:\Games\nQuake'
./tools/Test-VisualRestart.ps1 -Label restart-debug
./tools/Test-VisualRestart.ps1 -Label restart-release -Configuration Release
./tools/Test-PublicProfiles.ps1 -Label profiles
python ./tools/Prepare-MotionDemo.py 'C:\Demos\your-game.mvd' ./cache/motion.mvd
./tools/Benchmark-Visuals.ps1 -Label costs
./tools/Benchmark-Visuals.ps1 -Label vsync -VSyncOnly
```

The screenshot test deliberately captures immediately after video recreation,
checks requested/applied MSAA and hardware clamping, and rejects Vulkan validation
errors. The public-profile test needs an existing nQuake config and verifies all
71 saved visual values, all 55 conditional particle settings, the actual WASD/jump
bindings and crosshair/pickup settings. The installed ezquake.pk3 is required for
QMB initialization; its absence must not be interpreted as missing cvar support.

The benchmark plays the same 20-second slice three times per case, discards the
first run, compares frame counts, and records both measured averages. VSync is
disabled for costs; a separate FIFO/VSync run checks completion. The demo must
contain useful motion; different scenes/GPU/drivers yield different costs.
The fixture writer adds the normal EndOfDemo marker at the cut.

`Test-MotionRuntime.ps1` additionally requires DM4, DM6, E1M1, and a compatible
native KTX `qw/qwprogs.dll` in your installation. `Test-LocalArena.ps1` exercises
map selection, match modes and bot join/leave; pass `-ExpectedMapCount` for your
installed set of valid maps (the recorded development data set had 123).
These local tests use your own assets. Do not commit generated cache files.

The detailed historical CPU/GPU shader and 48-control image evidence is described
in `docs/history/VALIDATION.md` and `VISUAL-TEST-RESULTS.md`. Historical cache paths
refer to the original private workspace; the source release includes no raw
gameplay screenshots, full configs or player-containing logs/demos.

Shadow checks: `Test-Shadow2.ps1` / `Verify-Shadow2.py`, `Test-ShadowMultiview.ps1`, and `Test-MapLightStability.ps1 -Map dm6`. Use your own KTX/game assets. The published provenance manifests record development runs, not tests on your hardware.
