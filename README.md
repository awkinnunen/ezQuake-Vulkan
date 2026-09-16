# ezQuake Vulkan

Experimental QuakeWorld client based on tibazera's SDL3/Vulkan work in ezQuake.
This project adds competitive readability controls, general visual effects,
configuration previews, local single-player/arena menus, and renderer fixes.

**This is a Vulkan rasterizer. RTX/path tracing is not integrated.**
An RTX GPU can run the Vulkan renderer, but does not enable ray tracing.
See the [RT feasibility assessment](docs/history/RTX-FEASIBILITY.md) and
[direct RTGL1 implementation plan](docs/history/RT-IMPLEMENTATION-PLAN.md).
The plan covers existing and donor effects, independent RT defaults and RTX 3060
validation. A matching raster appearance is not an acceptance requirement.
An optional RTGL1 package/device test harness now builds with
`-DRENDERER_RTGL1=ON`. It does not enable RT gameplay or add a new renderer choice.
See the [implementation status](docs/history/RT-IMPLEMENTATION-STATUS.md).

The base is `tibazera/ezquake-source`, branch `feature/sdl3-vulkan-pr`, commit
`91859a996daede0df166e2a55a5f08d56b052b21`. Original Git history and source notices
are preserved. See [attribution](docs/ATTRIBUTION.md) for the origin of each
change and the distinction between inherited work, user direction and Codex implementation.

## Features added here

- Competitive Visuals: texture detail control, soft cel shading, lighting,
  team-aware silhouettes and restrained rim lighting with powerup/ruleset gates.
- Separate general effects: optional floating-point HDR, scene SSAO, bloom, exposure, tone mapping, sharpening,
  MSAA and FXAA. Contextual help, disabled dependencies and video restart feedback.
- Raster point/spot shadow maps, BSP/authored lights, cached maps and bounded updates.
  Includes the DM6 baked-light stability fix and corrected multiview resource ownership.
- Native graphics categories with live CFG preview, Enter/click to load, Escape to cancel and safe saving.
  Balanced, Ultra competitive and Athmospheric presets; Quick WASD, Quick ESDF and nQuake controls.
- Native Local Arena setup and in-game KTX bot controls on local or remote servers.
- Friends hosting through the existing FTE broker: reusable private invitations,
  encrypted connections and native Windows/Linux/macOS invitation links. Both
  players need a Friends-enabled build of this fork.
  See [Friends instructions and network limits](docs/FRIENDS.md).
- Explicit full-config import with a keyboard binding preview.
- Local Arena controls for an external KTX game module, and original Quake
  single-player startup/save handling when the required game data is installed.
- GPU-limit, texture, lighting, skybox, shutdown and screenshot lifecycle fixes.
- Explosion style without the expanding ring; original Big explosion remains available.

## Build and run on Windows

For a **new installation**, use the **[Windows Starter](https://github.com/awkinnunen/ezQuake-Vulkan/releases/tag/starter-v0.2.0)**.
Extract it, run `Install.cmd`, and choose a new directory. It downloads verified
nQuake resources and our Vulkan engine, installs Balanced / Quick WASD defaults,
and optionally imports full-game PAKs from your own classic Quake installation.
An existing nQuake installation is not required. See [Starter details](docs/STARTER.md).

Use the installed `Diagnose.cmd` to collect a log for an issue report.

To build from source:

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
See [the inherited build guide](BUILD.md) for additional build background.

## Linux and macOS test packages

Native packages and installation instructions are in the
[Unix test release](https://github.com/awkinnunen/ezQuake-Vulkan/releases/tag/v0.3.0-beta.1).
Choose Linux x86_64 (Ubuntu 24.04 or compatible) or the matching macOS arm64 / x64
package. Extract it and run `Install.command` to create a fresh nQuake-based
installation; no existing nQuake, Python or separately installed Vulkan SDK is
needed. Linux still needs a working system Vulkan driver. Mac packages include
MoltenVK and are ad-hoc signed, not notarized.

Build, packaging, native Friends and installer checks are automated. Actual Mac
graphics/input/audio and cross-city engine gameplay remain tester gates.
See [the port guide and validation scope](docs/UNIX-PORT.md).

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

**Balanced is the default graphics preset**, preserving 170 graphics values from
the user's saved configuration on 2026-09-15. Ultra competitive favors readability
and reduced rendering cost; Athmospheric requests high raster quality, including
HDR, SSAO, shadows and MSAA. None requires RTX. The older 71-value CV profile and
55-value particle module remain for compatibility.

Options > Graphics uses native widgets and seven categories. F2 browses presets;
highlight to preview, then Enter/click to load or Escape to cancel. F3 keeps the
current preview without navigating to another row. Save creates graphics-only CFGs with
recoverable backups. F5 applies pending video changes after leaving preview.
See [unified menus and presets](docs/history/UNIFIED-MENUS.md).

Options > Controls offers **Quick WASD (default)**, **nQuake**, and **Quick ESDF** (E forward, D back, S left,
F right). `keyboard_preset sdfe` selects the alternative. Graphics selection never
changes bindings. Quick controls include the five original weapon crosshair PNGs at legacy size 2.5.
Base game resources and personal full configs are not bundled. See
[Controls](docs/CONTROLS.md) for keyboard and crosshair settings.

Copying profiles does not overwrite an existing user's full configuration or force
a preset every launch. Run `exec ezv-wasd-defaults.cfg` once after startup to adopt
WASD and Balanced, then use normal config saving. Conditional particle effects need
the installed `ezquake/ezquake.pk3` resources. In-game KTX commands remain subject
to the connected server's bot support, permissions and map navigation.

## Status and license

See [validation](docs/VALIDATION.md) and [next work](docs/TODO.md).
The project is experimental; the tests are bounded checks, not a complete campaign
playthrough or a tournament/ruleset certification.

Existing ezQuake GPL terms and component notices remain in force; see
[LICENSE](LICENSE). Friends-enabled binaries use GPL-3.0-or-later because they link
OpenSSL; see [GPL-3.0](licenses/GPL-3.0.txt) and the dependency notices.
Quake game assets must be supplied separately.
The upstream project README is retained in [docs/UPSTREAM-README.md](docs/UPSTREAM-README.md).
