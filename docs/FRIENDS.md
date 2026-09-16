# Friends games (Windows, Linux and macOS test builds)

## Host a game

1. Open **Local Arena** and choose the map, mode and bots.
2. Set **Who can join** to **Friends (invitation)**, then start the game.
3. Open **Esc → Friends / invitations → Copy invitation** once the room is ready.
4. Send the copied link to your friends yourself.

An existing local multiplayer game can also be opened through **Open / resume
invitations**. Friends controls are available from the main and in-game menus.
An online guest can join another invitation but cannot administer someone else's
server through these controls. Ordinary KTX match/bot permissions still apply.

## Join a game

Copy the complete `ezquake-vulkan://join?...` invitation, open **Friends → Paste
invitation and join**, and confirm. Confirmation leaves your current game.
Both sides need this Friends-enabled engine build and the map/game resources.
The earlier standalone probe's `/probe` links are deliberately not game links.

Enable **Friends → Windows links** once on each Windows installation to make
clickable invitations open this game. Registration applies only to the current
Windows user and does not need administrator rights. A link opens a confirmation
in the matching running game, or launches that installation if it is closed.
Confirm to leave your current game and join; Cancel keeps your current game.
Toggle the setting again to remove this installation's registration. Register
again after moving the game folder. Other installations can take over the handler;
the Windows setting reports whether it currently points at this executable and game folder.

Linux and macOS use **Friends → Invitation links**. Unix Starter registers links
for the installing user by default (`--no-links` skips this). Linux creates a
desktop protocol handler; macOS registers the installed app with LaunchServices.
Private, profile-specific Unix sockets deliver links to an already running game.
The Mac menu registers/repairs the OS association; it does not pretend to remove
an association owned by the operating system. Repair registration after moving
the installation. GUI link opening on a real Mac remains a tester check.

The handler is built into the engine; Starter 0.2.0 offers registration during
installation. Short-code approval remains deferred. Some chat apps do not make custom schemes
clickable; pasting the complete link in the game still works. No new relay, VPN,
Discord installation or separate server program is required by this transport.

## Reuse and control the invitation

The host's random room name, secret and pinned certificate are saved together in
`ezquake/friends.identity` under the game's base directory. The file is encrypted
with Windows DPAPI for the current Windows account. Linux/macOS instead use
owner-only (0600) regular files with no symlink following; they are **not encrypted
at rest**. Keep the identity private and retain it when updating the
engine. It is not part of `config.cfg`, `autoexec.cfg`, presets or normal logs.
Only one host can open the same identity file at once. A corrupt/unreadable file
produces an error; it is never silently replaced by a new invitation.

The invitation does not expire because a match ends, the engine is restarted or a
counter increments. Reopen invitations in a running game to use the same link.
The host must be running and reachable when guests actually connect.

- **Close invitations:** reject new guests; keep current players connected.
- **Open / resume invitations:** permit new guests using the saved link.
- **Change invitation:** after confirmation, replace the secret and save it
  atomically. Old links cannot admit new guests; current players may stay.
- **Guest / Remove selected guest:** remove that player's connection. Also change
  the invitation if you want to prevent them from returning with their old link.

There is no MAC address or rotation counter. A room name is not a password. The
existing public broker does not reserve a name while its owner is offline; a
collision is reported without automatically changing the saved identity.

## Network coverage

The engine uses the existing Frag-Net FTE broker and STUN service to establish
native libjuice ICE connections, with pinned OpenSSL DTLS protecting game traffic.
It does not deploy a new external service. There is currently no TURN allocation
or fallback relay: some NAT/firewall combinations will fail to connect.

The user supplied a successful standalone-probe result and confirmed the two
computers were in different cities (100/100 packets, direct route, mean RTT
31.7121 ms). This proves one network pair. Engine gameplay has separately been
tested with multiple local processes through the real broker; cross-city
**gameplay with this engine build** remains a user test. Sleep, broker reconnection,
IPv6 coverage and additional NAT combinations remain on the TODO list. Reopen
invitations after a lost broker connection; the saved link stays the same.

The custom invitation/admission transport currently requires this fork at both
ends. Using the FTE broker does not imply native FTE clients can join these rooms.
Existing ezQuake voice code is present, but microphone-to-microphone operation over
Friends has not yet been validated.

## Technical notes

`NA_FRIENDS` carries a process-local 64-bit peer identifier, distinct from both
IPv4 and privileged loopback. Broker slot reuse cannot reuse the game's identity.
While hosting a Friends game, the server rejects ordinary UDP/TCP clients so they
cannot bypass admission. IP-based VIP checks and real-IP probing do not apply to
logical peers, and Friends packets cannot run connectionless RCON. Use normal
player kick controls and invitation rotation instead of IP bans for these peers.

Networking and TLS run on a worker thread. Engine state stays on the main thread.
Bounded datagram queues preserve QW's unreliable packet model and netchan sequence
handling. Packets up to 8192 bytes are fragmented into DTLS records carrying at
most 1000 game bytes each; incomplete assemblies expire and cannot grow without
limit. The engine still enforces its own receive-buffer limit. A new join has a
request generation to prevent stale state from an earlier asynchronous join.
Map reconnect preserves the established transport and performs a new QW sign-on.

The Windows URI handler uses `HKCU/Software/Classes/ezquake-vulkan` and an explicit
executable/base directory pair. A bounded, profile-specific local mailslot passes
only validated invitation data. URI contents never enter the console command
buffer, cvars or saved configs, and are removed from engine command-line reports.
The URI remains visible to the OS/application opening it, as with other private
links. Startup rejects malformed links and extra arguments before engine commands
execute. The parser accepts Windows' equivalent `join/?` spelling as well as the
original `join?` form. Existing `qw://` registration is not changed by this feature.

Admission is limited to 15 remote peers per host. Pending peer attempts, inactive
peers, signaling and receive queues are bounded. Graceful departure/kick sends an
authenticated goodbye; lost connections also expire. Protocol version 1 requires
the full invitation secret and host fingerprint; no public-room-only admission.

Build with CMake `ENABLE_FRIENDS=ON` (all platform defaults), a C++17 compiler,
libjuice and OpenSSL >= 3.2. Unix uses IXWebSocket for broker signalling, with
OpenSSL TLS on Linux and SecureTransport on macOS. All platforms retain protocol
version 1, including compatibility with Windows beta 0.2.0. See
[Unix builds and package tests](UNIX-PORT.md). The vcpkg submodule/manifest pins
the dependency source baseline.
`ENABLE_FRIENDS=OFF` omits the worker and Friends menu entries. Existing installations
whose CMake cache has `VCPKG_MANIFEST_INSTALL=OFF` must install the new dependencies
or explicitly point CMake to an existing matching dependency prefix.

Test target: `ezquake-friends-tests`. The `--unit <fresh-directory>` mode checks
binary fragments, malformed bounds, identity persistence, locking, rotation,
DPAPI (Windows) or private-file permissions (Unix), and corrupt-file rejection.
`tools/friends-probe/Test-GameTransport.py`,
`Test-Admission.py` and `Test-EngineFriends.py` run bounded tests with owned rooms.
The engine test's `--arena --links` mode covers cold start, actual Windows shell
dispatch to a running profile, confirmation/cancel, command-injection rejection,
secret redaction and multiplayer gameplay. That mode leaves the supplied engine
and game-data directory registered after the test. Developer polling runs after
normal engine initialization rather than keeping startup inside a config loop.
Their output directories contain private test links and must not be published.
Sanitized evidence is recorded in `provenance/friends-engine-validation.json`.

New adapter, menu, persistence and test code: **OpenAI Codex**, requested and
directed by **AWK**. ezQuake/QW, KTX, FTE, libjuice and OpenSSL retain their original
authorship and licenses. OpenSSL's Apache-2.0 license means the combined binary is
distributed under GPL-3.0-or-later, exercising the engine's GPL-2.0-or-later option.
Retain all dependency notices and matching source when packaging this build.
