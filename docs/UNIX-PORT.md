# Linux and macOS port

Requested by AWK; implementation by OpenAI Codex, 2026-09-16.

This work adds native Friends networking and invitation links on Linux and macOS.
The existing Windows wire protocol and Frag-Net signalling service are retained.
There is no new relay service and no separately installed networking application.
Linux uses IXWebSocket with OpenSSL; macOS uses IXWebSocket with SecureTransport
for broker TLS. Both use libjuice ICE and the existing OpenSSL DTLS transport.

Windows retains DPAPI identity protection. Unix identities use owner-only (0600)
regular files, no symlink following, exclusive locks and atomic replacement.
Unix storage is not encrypted at rest. Invitation secrets never enter console
commands. Profile-specific private Unix sockets deliver links to a running game.

macOS uses MoltenVK and Vulkan portability enumeration. This is raster Vulkan,
not the unfinished Windows RTGL1 integration. An actual Mac graphics test remains
required, particularly descriptor indexing limits on the tester's GPU.

## Packages and installation

Linux is built on Ubuntu 24.04 for x86_64 (glibc 2.39 or newer). The tarball
includes its non-driver runtime libraries. A working Vulkan driver and desktop
environment are required. macOS has separate arm64 and x64 packages with an app
bundle, MoltenVK and the Vulkan loader. The build targets macOS 11; actual GPU and
OS compatibility still require hardware testing. These are raster-renderer ports.

Extract the package, then run `sh Install.command` on Linux or open
`Install.command` on macOS. The native installer downloads the four pinned,
hash-verified nQuake resource archives. No Python, Wine or VPN application is
required. It creates a fresh directory and refuses to replace an existing one.
It preserves nQuake's autoexec.cfg and applies Balanced / Quick WASD once through
nQuake's first-run preset mechanism. Owned classic Quake PAKs can be imported with
`--quake /path/to/Quake`; commercial game assets are not distributed.

Default data directories:

- Linux: `$XDG_DATA_HOME/ezquake-vulkan` or `~/.local/share/ezquake-vulkan`.
- macOS: `~/Library/Application Support/ezQuake-Vulkan`.

The installer registers invitation links and creates a per-user launcher by
default. `--no-links` and `--no-shortcut` disable those actions. Run the installed
`Start.command` to play. On macOS there is also an app shortcut in `~/Applications`.
Moving the installation requires repairing registration from the Friends menu.
The macOS menu repairs the OS-owned association rather than offering a misleading
unregister toggle. Linux uses a desktop protocol handler and private Unix IPC.

Mac packages are ad-hoc signed, **not Developer ID signed or notarized**. If
Gatekeeper blocks the downloaded app, use the per-app Open Anyway control in
System Settings > Privacy & Security. Do not disable Gatekeeper globally.

## Build and reproduce

The complete dependency installation and packaging commands are in
[the CI workflow](../.github/workflows/main.yml). Builds run on their target OS;
Windows orchestrates Linux containers and macOS GitHub runners. This avoids
requiring an unofficial macOS SDK cross-toolchain on Windows.

Initialize submodules first: `git submodule update --init --recursive`.
Linux's `tools/Build-UnixRuntime.sh /opt/ezv-deps` builds pinned shared OpenSSL
3.6.0, SDL 3.4.0, Vulkan headers and static Friends dependencies. Install the
Ubuntu packages listed in the workflow, then:

```sh
cmake --preset dynamic -DRENDERER_VULKAN=ON -DENABLE_SANDBOX=OFF \
  -DENABLE_LTO=OFF -DCMAKE_PREFIX_PATH=/opt/ezv-deps
cmake --build --preset dynamic-release
```

On macOS install Xcode command-line tools, CMake, Ninja, autoconf/automake/libtool,
pkg-config and the pinned universal Vulkan SDK shown in CI. Bootstrap vcpkg and
use `macos-arm64` or `macos-x64` instead of `dynamic`; use the corresponding
`macos-arm64-release` / `macos-x64-release` build preset. Disable sandbox and LTO
as in CI. The x64 build uses the SDK's universal loader, not an arm64-only Homebrew
loader. SDL3 handles relative mouse input on both architectures.

`tools/Package-Unix.py` verifies the embedded engine revision, relocates runtime
libraries, signs Mac copies, and emits packages, SHA-256 sidecars and matching
source ZIPs. Linux's source ZIP includes sources for bundled Ubuntu libraries;
Mac's includes vcpkg source archives and the exact submodule snapshots. Build
recipes and upstream attribution are retained. Source ZIPs are optional for
players and intentionally much larger than the game-engine download.

## Validation scope

Native Linux and Windows Friends unit tests pass. Real-broker transport checks
pass with two Linux guests, rejoin and a host restart using the same invitation.
Windows-host/Linux-guest and Linux-host/Windows-guest each exchanged 100 binary
1450-byte payloads through the existing broker. These were local machine/container
tests, not independent internet connections.

Linux's native installer passes fresh installation, paths containing spaces,
unchanged nQuake autoexec, crosshair installation, existing-directory refusal,
corrupt-cache checks and rejection of unsafe ZIP paths, symlinks and duplicates.
CI repeats native unit and packaged-installer checks on each Unix architecture.
The relocated engine is also launched with an intentionally invalid invitation
to verify runtime loading without changing the user's URL association.

Actual Mac graphics, mouse, audio, GUI invitation dispatch, long sessions and
cross-city gameplay remain external tester gates. CI success does not establish
those results. The friend should test Balanced first, then the other presets,
Friends hosting/joining in both directions, a map change and a reused invitation
after restarting. Report OS/GPU, package architecture and `qw/qconsole.log`;
never publish `friends.identity` or private invitation links.
