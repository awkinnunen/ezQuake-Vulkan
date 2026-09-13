# ezQuake Vulkan

Experimental QuakeWorld client based on tibazera's SDL3/Vulkan work in ezQuake.
This project adds competitive readability controls, general visual effects,
configuration previews, local single-player/arena menus, and renderer fixes.

**This is a Vulkan rasterizer. RTX/path tracing is not integrated.**
An RTX GPU can run the Vulkan renderer, but does not enable ray tracing.

The base is `tibazera/ezquake-source`, branch `feature/sdl3-vulkan-pr`, commit
`91859a996daede0df166e2a55a5f08d56b052b21`. Original Git history and source notices
are preserved. See [attribution](docs/ATTRIBUTION.md) for the origin of each
change and the distinction between inherited work, user direction and Codex implementation.

## Features added here

- Competitive Visuals: texture detail control, soft cel shading, lighting,
  team-aware silhouettes and restrained rim lighting with powerup/ruleset gates.
- Separate general effects: bloom, contact AO, exposure, tone mapping, sharpening,
  MSAA and FXAA. Contextual help, disabled dependencies and video restart feedback.
- Named graphics profiles and a `.cfg` browser with a keyboard binding preview.
- Local Arena controls for an external KTX game module, and original Quake
  single-player startup/save handling when the required game data is installed.
- GPU-limit, texture, lighting, skybox, shutdown and screenshot lifecycle fixes.

## Build and run on Windows

Install Visual Studio 2022 C++ Build Tools, CMake 3.22 or newer, Ninja, Git and
the Vulkan SDK. Open an **x64 Native Tools Command Prompt** with the SDK's
`Bin` directory on `PATH`, then run:

```bat
git submodule update --init --recursive
vcpkg\bootstrap-vcpkg.bat -disableMetrics
cmake --preset msvc-x64 -DENABLE_LTO=OFF -DRENDERER_VULKAN=ON
cmake --build build-msvc-x64 --config Release --parallel 4
Start-Vulkan.cmd "C:\Games\nQuake"
```

The launcher runs the build directly against your installed game data.
Use `--config Debug` to build a Debug executable; pass `Debug` as the launcher's
second argument. This enables developer diagnostics and Vulkan validation.
See [the inherited build guide](BUILD.md) for other platforms; this project's
current validation was performed on Windows x64.

## WASD and graphics defaults

Copy the **contents** of `profiles/` into your game-data directory once, preserving
the `qw/` and `ezquake/` subdirectories. Back up any same-named files first.
In the game console, load either or both:

```text
exec ezv-wasd.cfg
exec ezv-defaults.cfg
// Or both together:
exec ezv-wasd-defaults.cfg
```

The graphics profile contains all 53 user-approved values, including edge depth
threshold **16**. Loading `ezv-defaults.cfg` also restarts video to apply MSAA.
It is a named profile; the engine's factory reset remains a separate action.
The defaults helper also loads `ezv-particles.cfg`, restoring enhanced lightning
and the other 55 conditional legacy settings. These require nQuake's
`ezquake/ezquake.pk3` particle textures. Restored bloom is 0.1 / 0.7 / 2.75
(strength / threshold / radius).
The WASD overlay preserves installed nQuake non-letter bindings and communication
aliases. See [controls](docs/CONTROLS.md). Game data, private full configs and
custom crosshair images are not bundled.

## Status and license

See [validation](docs/VALIDATION.md) and [next work](docs/TODO.md).
The project is experimental; the tests are bounded checks, not a complete campaign
playthrough or a tournament/ruleset certification.

Existing ezQuake GPL terms and component notices remain in force; see
[LICENSE](LICENSE). Quake game assets must be supplied separately.
The upstream project README is retained in [docs/UPSTREAM-README.md](docs/UPSTREAM-README.md).
