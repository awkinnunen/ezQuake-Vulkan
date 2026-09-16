ezQuake Vulkan - Linux/macOS test distribution

Run Install.command to download verified nQuake data and create a fresh installation.
No Python, Wine, Tailscale or separate game server is required.
The installer never replaces an existing installation or edits autoexec.cfg.
Defaults: Balanced graphics / Quick WASD. Other presets and layouts are in-game.
This is an unofficial nQuake-based package, not an official nQuake release.
Commercial Quake assets are not included.

Linux: Ubuntu 24.04 or newer compatible x86_64 distribution (glibc >= 2.39).
Install a Vulkan-capable graphics driver and normal desktop audio/window libraries.
Extract the tar.gz, open a terminal there and run: sh Install.command
The installation defaults to ~/.local/share/ezquake-vulkan.
Run Start.command from the installed directory. Friends registers its URL handler.

macOS: separate Apple Silicon (arm64) and Intel (x64) packages, macOS 11+.
MoltenVK and the Vulkan loader are included. You do not need the Vulkan SDK.
The GPU must support the renderer's descriptor indexing requirements.
This is an ad-hoc signed, UNNOTARIZED developer test package. If macOS blocks it,
use System Settings > Privacy & Security > Open Anyway for this downloaded app,
then run Install.command again. Do not disable Gatekeeper globally.
Installation: ~/Library/Application Support/ezQuake-Vulkan.
A shortcut to the installed app is placed in ~/Applications when absent.
Start.command or the installed ezQuake.app launches the game.

Optional terminal installation arguments:
  --destination "/path/to/new/install" --cache "/path/to/download/cache" --yes
  --quake "/path/to/your/owned/classic/Quake"   (imports validated pak1+ files)
  --no-links                                 (skip URL handler registration)
The default is to register ezquake-vulkan:// invitations for your user account.
You can repair registration in Friends > Invitation links. macOS owns the default
application association; that menu repairs it rather than pretending to remove it.

Friends: choose Friends access in Local Arena, start the match, copy the invitation.
A join link asks the recipient for confirmation. Reuse it until the host rotates it.
Linux/macOS/Windows use the same version-1 encrypted protocol and existing Frag-Net
signalling service. Some NAT/firewall combinations still prevent direct UDP;
there is no new relay or guaranteed TURN fallback.
Unix identity files are owner-only (0600), not encrypted at rest. Keep them private.

Troubleshooting: run Start.command -window -width 1280 -height 720 -condebug.
Send qw/qconsole.log plus starter-install.json, OS/GPU model and reproduction steps.
Do not share friends.identity or invitation secrets publicly.
Mac graphics, GUI invitation dispatch, sound and actual cross-city play still need
hardware testing. CI/build success is not a claim that these tests have passed.

Source and issue reports: https://github.com/awkinnunen/ezQuake-Vulkan
Credits/licenses: engine/notices, engine/docs/ATTRIBUTION.md and engine/provenance.
Implementation: OpenAI Codex, directed by AWK; based on ezQuake and upstream work.
