Native Linux and macOS tester packages, including the Vulkan raster renderer, graphics presets, Local Arena/KTX and Friends invitations. No RTX card is required; this does not enable the unfinished ray-tracing integration.

### Choose a package

- **Linux x86_64:** `linux-x86_64.tar.gz`, Ubuntu 24.04 or compatible (glibc 2.39+), with a working Vulkan graphics driver and desktop libraries.
- **Apple Silicon Mac:** `macos-arm64.zip` or `.dmg`.
- **Intel Mac:** `macos-x64.zip` or `.dmg`.

Mac binaries target macOS 11+. The GPU must support the renderer's required Vulkan features through bundled MoltenVK. Older GPU compatibility still needs testing. The Vulkan loader and MoltenVK are included; do not install the Vulkan SDK just to play.

Extract/open the package and run **Install.command** (Linux: `sh Install.command`). The native installer downloads about 112 MB of verified nQuake data and creates a fresh installation. It refuses to overwrite an existing directory, preserves autoexec.cfg, and applies Balanced / Quick WASD defaults once. Use the installed **Start.command** to play. No Python, Wine, VPN or separate server application is required.

Mac packages are **ad-hoc signed and not notarized**. If macOS blocks the downloaded app, use its per-app **System Settings → Privacy & Security → Open Anyway** control. Do not disable Gatekeeper globally. This is an early hardware-testing release.

### Friends and compatibility

The installer registers `ezquake-vulkan://` links for your user account by default (`--no-links` skips registration). A link opens a confirmation before joining. Host through Local Arena → Friends access, then copy the invitation from Friends / invitations. Reuse the link until the host changes it. Windows beta 0.2.0 uses the same protocol and remains available through the [Windows Starter](https://github.com/awkinnunen/ezQuake-Vulkan/releases/tag/starter-v0.2.0).

The existing Frag-Net broker/STUN is used; no new external relay is introduced. Some NAT/firewall combinations cannot connect directly. Unix host identities are stored in private owner-only files, not encrypted at rest. Do not publish identities or invitation secrets.

### Verified and still to test

- Native Linux, macOS arm64 and macOS x64 builds, Friends unit tests, relocated engine startup and full native installer checks pass in CI.
- Installer checks include config preservation, paths with spaces, existing-install refusal, checksums and rejection of unsafe ZIP entries.
- Native Windows ↔ Linux binary transport checks pass in both host directions through the real broker; two Linux transport guests and invitation reuse also pass.
- These checks do **not** establish Mac graphics, physical mouse/audio, GUI invitation dispatch, long-session stability or cross-city gameplay. Those are the purpose of this tester release. Linux software-Vulkan gameplay checks provide partial evidence, not a complete multiplayer pass.

Please test Balanced first, then other presets; host/join in both directions, change maps, and reuse the same invitation after restarting. Report OS, GPU, package architecture, reproduction steps and `qw/qconsole.log`. [Port instructions and current validation](https://github.com/awkinnunen/ezQuake-Vulkan/blob/main/docs/UNIX-PORT.md).

Matching `-source.zip` files and SHA-256 sidecars accompany every platform. Source downloads are not needed to play. Commercial Quake assets are not included. This is an unofficial nQuake-based distribution. ezQuake, tibazera and other upstream authors retain their credit; new port/installer code and tests are by OpenAI Codex at AWK's direction.
