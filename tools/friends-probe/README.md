# Friends connection probe

FRIENDS-002, 2026-09-16. Native Windows x64 transport prototype by OpenAI Codex,
under the user's direction. **This is a network test, not a playable Quake build.**
It does not change an existing game installation or register a URL handler.

## Two-computer test

1. Extract the test ZIP on both Windows computers. No Python, OpenSSL installation,
   FTE installation, account or VPN is needed. Keep the extracted files together.
2. Use different internet connections, for example home broadband and a phone
   hotspot. Two processes on one computer are only a local protocol test.
3. On the hosting computer, run **Host.cmd**. Wait for **Room ready**. Its invitation
   is copied to the clipboard, replacing the clipboard contents, and also written
   to `invitation.txt`. Send that invitation to the other tester through your usual
   chat. The host waits without a time limit; leave it running. Joining the next
   day uses the same invitation while the program and broker connection stay open.
   Keep the host computer awake and connected to the internet.
4. On the other computer, run **Join.cmd**. Paste the complete invitation and press
   Enter. This probe uses `ezquake-vulkan://probe`, which is not registered as a
   clickable system link. The joiner waits up to 90 seconds.
5. Both sides should report **PASS: 100 protected datagrams verified** and exit.
   Collect `host-result.json` and `join-result.json`. These reports omit addresses,
   room IDs, keys, certificate fingerprints and SDP.
6. Repeat with the roles reversed. Record which connections were used separately
   from the reports. A successful same-network test is not NAT traversal evidence.

Version 0.1.1 removes the host's former five-minute limit. `--host` now defaults to
unlimited waiting; `Host.cmd` explicitly uses `--seconds 0`. WebSocket keepalives
and room metadata refreshes run every 30 seconds. Idle polling is reduced to avoid
unnecessary CPU use overnight. A finite `--seconds 10..600` remains available for
automated tests. After a peer appears, unlimited mode allows 180 seconds for the
actual connection/test; the join launcher retains its 90-second attempt limit.
The probe still exits after a successful test. Automatic recovery after sleep,
network loss or broker failure is not implemented; restart the host and share its
new invitation in that case. The transport uses the invitation fingerprint for
identity, so the ephemeral certificate's one-day CA date does not expire the invite.

Ctrl+C cancels. If Windows asks about network access, permit this test program on
the network you intend to test. The host leaving ends the test; old invitations
then cease working. Share result JSONs for diagnosis, not invitation files.

The generated room can appear in the existing broker's public list, under a generic
transport-test name with zero game slots. Knowledge of its room name does not grant
test admission. ICE candidates are exchanged through the broker; libjuice may
include local as well as public addresses. This prototype does not hide addresses
from the broker/peer. Its logs and JSON results do not record them.

## What is implemented

- Real TLS/WebSocket connection to `master.frag-net.com:27950`, with Windows
  certificate validation; FTE `rtc_host` / `rtc_client` and binary signaling frames.
- Random room registration, SDP exchange and trickled ICE candidates.
- libjuice 1.7.0 native ICE/STUN, without browser SCTP or a new external service.
- OpenSSL 3.6.0 DTLS 1.2, ephemeral certificates, host fingerprint pinned by the
  invitation, and peer certificate matched against its SDP before application I/O.
- A separate 256-bit invitation key checked inside DTLS before echo traffic.
- One peer; 100 numbered random echo payloads, including roughly 1 KB datagrams,
  sent at 25 ms intervals. Duplicates do not count as additional received packets.
- Bounded queues/messages, unlimited host waiting, bounded active attempts,
  cancellation and secret-free results.
- A pinned build manifest, protocol/crypto self-tests and optional live pair tests.

The latency includes this harness's polling and ICE library behavior; it is not
a game latency benchmark. This strict probe marks missing echo packets as a failed
test and records the observed counts; loss does not by itself establish that a
normal game would be unusable. It does not retransmit application echo packets.

## Not implemented / acceptance gates

No Quake packet transport integration, multi-guest hosting, host approval screen,
short-code joins, game menus, production join links, full FTE gameplay-client
interoperability or public release is claimed. The peers speak our versioned probe
admission protocol over an FTE-compatible broker; an unmodified FTE player cannot
join this test. The use of a dedicated ICE library replaces a large direct extraction
of FTE's engine-dependent networking code.

Existing TURN metadata is counted but **not allocated or used**. The observed local
tests offered none. Restrictive network pairs may therefore fail. No TURN server,
new broker, service account, router mapping or VPN is created by this program.
Review actual existing relay availability and dependency updates before any public
gameplay release. The prototype is not hardened for multiple unsolicited joiners;
it accepts one test peer and rejects incompatible/protocol-invalid sessions.

The user's second network machine was unavailable on 2026-09-16. Consequently
FRIENDS-002's two-network gate remains open, and engine integration has not started.
The current engine, presets, configuration and published Starter are unchanged.

## Build and developer tests

Use Visual Studio 2022 x64 C++ tools, CMake and vcpkg. The probe's manifest pins
vcpkg baseline `66c0373dc7fca549e5803087b9487edfe3aca0a1`; it is isolated from the
engine manifest. In an x64 Developer PowerShell:

```powershell
./Build.ps1 -VcpkgRoot C:/path/to/vcpkg -Configuration Release
./Build.ps1 -VcpkgRoot C:/path/to/vcpkg -Configuration Debug
```

The executable statically links the C runtime and dependencies. `--self-test`
requires no network. It checks strict invitations, malformed signaling, DTLS
certificate pinning and binary datagram preservation. Build.ps1 runs it with CTest.

The optional developer live test uses Python and creates one temporary public test
room, hosting both endpoints itself. It closes the processes and removes secret
invitation fixtures afterward:

```powershell
python Test-LivePair.py --exe C:/path/to/FriendsProbe.exe --output C:/path/to/results --case success
python Test-LivePair.py --exe C:/path/to/FriendsProbe.exe --output C:/path/to/results --case wrong-key
python Test-LivePair.py --exe C:/path/to/FriendsProbe.exe --output C:/path/to/results --case wrong-fingerprint
```

Do not commit `invitation.txt`, reports, logs, build outputs or captured signaling.
This tool directory's ignore rules cover its own runtime files. Reviewed sanitized
validation belongs in the repository's provenance directory.

## Attribution and source

The FTE wire protocol was studied at commit
`f937b9d88f71fc4429db5fe56c6a98d922711b2e`, particularly `netinc.h`, `net_ice.c`
and `net_wins.c`. No FTE implementation file is copied into this prototype.
The FTE authors retain credit for their broker and connection establishment work.

Adapter and test source: GPL-2.0-or-later. The combined test executable is distributed
under GPL-3.0-or-later, with the included dependency notices and corresponding
source archive. libjuice 1.7.0's actual source license is MPL-2.0; this vcpkg
baseline's port metadata incorrectly says LGPL-2.1-only. OpenSSL is Apache-2.0.
Original notices and dependency source bytes are retained. Windows system libraries
provide TLS/WebSocket and clipboard facilities.
