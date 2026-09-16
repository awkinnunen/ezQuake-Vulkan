# Friends hosting through existing FTE infrastructure

Status: engine transport, persistent full invitations and clipboard menus implemented.
The user's later simplicity choice makes the saved full link the first supported
workflow. Windows URI registration and native click-to-launch are implemented;
Starter integration is included in 0.2.0; code-only approval is deferred.
See [current player instructions](../FRIENDS.md); the design below also includes
future resilience, relay and distribution work, not all implemented features.
2026-09-16, FRIENDS-001 / FRIENDS-002. See [implementation status](FRIENDS-HOSTING-STATUS.md).
Requirements and product direction: user. Research and design: OpenAI Codex.
Existing transport/broker work belongs to the FTE authors; existing game, server
and proxy work retains its ezQuake, MVDSV, KTX and QWFWD authorship.

## Decision

Build an FTE-compatible room client into ezQuake-Vulkan and connect it to the
existing Frag-Net broker. Keep the current embedded QW/KTX server authoritative.
Players start a local game and share an invitation; their own computer hosts it.
No separate player installation, account, VPN, newly operated relay, or rented
game server is part of the design.

Prefer direct UDP established through ICE. Use a pre-existing TURN service only
when it is actually offered for this purpose and passes interoperability tests.
Do not promise universal connectivity: some pairs of networks require a working
relay. An available STUN/broker service alone cannot provide that guarantee.

The first implementation milestone is a two-network transport experiment, before
building the complete menus. This is a substantial networking adaptation, not a
menu-only change. No FTE renderer, game logic or full engine replacement is needed.

## Findings and evidence

| Facility | What it gives this project | Boundary |
| --- | --- | --- |
| FTE named rooms | A host registers a room; clients find it without copying an IP address. | Requires FTE-compatible signaling in our client and host. |
| Existing Frag-Net broker | Exchanges connection setup messages over a persistent connection. | It is not automatically a relay for gameplay. |
| ICE and STUN | Discover candidate routes and test a direct UDP path through NAT. | Success depends on both networks; CGNAT is not an automatic yes or no. |
| FTE TURN client and master selection | Support a relay candidate when an eligible existing service is available. | Live relay availability and allocations were not verified. |
| Normal QW master lists | Discover conventional reachable servers. | A list entry does not make a private host reachable. |
| QWFWD / ezQuake Qizmo routing | Forward a client connection toward a reachable game server. | No compatible reverse-hosting facility was identified. |
| Existing local KTX server | Runs the match, bots and server-side permission checks. | Invite access must be enforced before a remote peer enters it. |

FTE documents named hosting, ICE, optional TURN and the distinction between native
ICE and browser WebRTC. Its default broker is `tls://master.frag-net.com:27950`.
The checked server callback accepts a named `sv_public /room`; the hosting guide
also describes `sv_port_rtp` and automatic rooms with `sv_public 2`. These are FTE
interfaces, **not commands implemented in this fork**. See [hosting documentation][fte-hosting]
and [broker address/transport implementation][fte-net].

The broker uses a WebSocket upgrade with `rtc_host` or `rtc_client`, a game/room
path, and binary messages carrying a command and peer ID. Messages cover greeting,
name collision, peers, offers, candidates and server metadata. The adapter must
match these details and the SDP/candidate dialect; speaking generic WebRTC alone
is insufficient. See [FTE ICE/broker client][fte-ice].

The master implementation can select a server marked as TURN-capable in the same
game group and issue temporary credentials. It skips password-protected entries;
its current selection is random, not a closest-relay measurement. A normal QWFWD
entry is a different capability. The master also publishes broker rooms: registering
an unpredictable room name does **not** make it an unlisted private room. No
supported unlisted flag was established. See [master listing and relay selection][fte-master].

QWFWD's connection request resolves a target and sends a Quake challenge toward
it. This does not establish an outbound registration tunnel from the target host.
See [QWFWD request handling][qwfwd-svc] and [peer setup][qwfwd-peer]. ezQuake's
documented Qizmo options reuse a proxy for connecting to servers and route discovery;
they do not document hosting a private server behind it. See [ezQuake multiplayer
settings][ezq-settings]. This is not a claim that every historical Qizmo extension
has been audited.

### Checks actually performed

On 2026-09-16 the existing `master.frag-net.com:27950` accepted verified TLS and
returned HTTP 200 for its index and game lists. One UDP STUN Binding Request
received a valid success reply with the matching transaction ID. The active Quake
namespace in the returned index was `FTE-Quake`, not `QuakeWorld`; the latter
returned a page but did not establish a usable game group.

The `FTE-Quake` public list was read. This did not establish a usable TURN allocation
or guarantee that all advertised capabilities were represented in the raw list.
No test room was registered, no player was joined, no TURN allocation was requested,
and no game traffic was sent. No operators were contacted. Production broker
compatibility, service usage expectations and relay availability remain release
gates, not assumed facts. This is a point-in-time service check, not an uptime promise.
Sanitized evidence and pinned source hashes: [research record](../../provenance/friends-hosting-research.json).

## Player experience

### Starting a game

1. Open **Local Arena**. Choose the map, rules and bots as today.
2. Choose **Who can join: Only this computer / Friends**. Default remains local.
3. Press **Start game**. In Friends mode, the local match starts and room setup
   runs asynchronously. A network failure must not prevent playing against bots.
4. Once the broker confirms the room, show **Copy invitation**, **Copy room code**
   and **Close invitations**. Explain that successful guest connectivity is checked
   when a guest connects; room registration is not a reachability test.

During a running local match, **Esc > Friends** offers the same controls. Opening
invitations must not restart the map. Host controls, including adding bots and
managing guests, stay in the in-game menu. The host leaving ends the hosted game;
automatic host migration is outside this proposal.

### Joining

The main menu gets **Join friends**, accepting either a complete invitation or a
room code. Paste, Enter, Escape and mouse operation work through existing menu
widgets. Explain each focused choice with the same help style as other menus.

The preferred invitation is a versioned application link, for example:

`ezquake-vulkan://join?v=1&service=fragnet&room=EZV-7M2K-9Q4R#key=...&fp=...`

This is a proposed syntax, not a working link. `key` is a random session invitation
secret; `fp` binds the invitation to the host's transport certificate. The room
code is a random, collision-checked locator. It carries no public IP address and
is not a password. Use an ambiguity-free alphabet and display groups for reading
aloud. Version the format before release and set strict size limits.

Where a chat application does not make custom links clickable, copying the whole
invitation into **Join friends** works identically. A website is not required.
Implemented registration is per-user and opt-in through Friends -> Windows links.
The native engine handler preserves `qw://`, launches the exact registered
executable/base directory or forwards to that profile's running instance. Future
Starter packages can expose the same registration API; no PowerShell intermediary
is required to handle invitation URLs.

Joining from a full link needs no password typing. Joining by the short code asks
the host to approve the peer. Show matching short verification phrases on both
screens if the host wants to confirm identity through the existing conversation;
display names alone are not authenticated identities. Rate-limit pending requests
and allow disabling code-only joins. Approval grants access to that authenticated
transport instance, not to every user with the same name.

### Privacy and access

Label this **Friends / invitation required**, not **Invisible server**. Room metadata
may appear in the broker's public list. Publish only generic room metadata and the
appropriate protected-access indication; omit player names and unnecessary details
where the protocol allows. Neither a code prefix nor omitting normal QW heartbeats
is an access control mechanism.

Full-link admission must validate the separate invitation secret over authenticated
DTLS, bound to this session and host certificate. Keep secrets out of broker
messages, serverinfo, status replies, cfg files, demos and ordinary logs. A custom
URI can still be exposed to the application/OS handling it; redact diagnostics.
Use existing cryptographic primitives and a defined admission exchange, not a
home-designed encryption protocol. Never silently downgrade invite sessions to
plaintext. Short-code approval needs its own explicit peer authentication policy;
do not present it as equivalent to a fingerprint-bearing invitation.

Only admitted peers may reach the QW challenge/connect flow. Enforce the same
Friends policy on plain UDP and spectator entrances so an IP connection cannot
bypass it. Keep normal public/LAN server modes available separately. Do not expose
rcon, confer local-player privileges, or overwrite a user's persistent password
and autoexec settings. Rotating the invitation invalidates old invitations;
existing admitted players stay unless the host explicitly removes them.

### Progress and failure messages

| State | UI and action |
| --- | --- |
| Registering | “Preparing invitation…” with Cancel. |
| Room registered | Copy buttons enabled; guest connectivity still untested. |
| Connecting | “Connecting to your friend's game…”; network details expandable. |
| Pending approval | “Waiting for the host”; host gets a nonmodal request. |
| Connected | Show latency and Direct / Relayed in connection details. |
| Room gone | “This game has ended or its invitation has changed.” |
| Room name collision | Generate another name automatically before publishing it. |
| Broker unavailable | Retry, continue local play, or use an ordinary reachable address. |
| No route | “These networks could not connect. Try having your friend host.” Also offer existing public servers. |
| Existing relay unavailable | Continue direct attempts; explain failure if no route succeeds. |
| Already in another game | Show the destination and require the normal leave-game confirmation. |

Do not claim a NAT type or a missing relay as a proven diagnosis from a generic
timeout. Apply finite retry budgets, allow cancellation throughout, and record
specific transport errors in redacted diagnostics. Online guests receive ordinary
KTX controls according to server permissions, never the local-host controls.

## Architecture

```mermaid
sequenceDiagram
    participant H as Host's ezQuake
    participant B as Existing Frag-Net broker
    participant J as Friend's ezQuake
    H->>B: Register room over TLS/WebSocket
    B-->>H: Confirm room
    Note over H,J: Host shares invitation through an existing chat
    J->>B: Join room
    B-->>H: New peer and setup messages
    H->>B: Host candidates and offer/answer
    B-->>J: Forward setup messages
    H->>J: ICE connectivity checks
    J->>H: ICE connectivity checks
    Note over H,J: Select direct route; existing TURN only if available
    J->>H: Authenticate invitation over protected datagrams
    H-->>J: Admit peer
    J->>H: Normal QW connection and gameplay
```

### Transport seam

Introduce a narrow `net_friends` interface for lifecycle, polling, sending,
receiving and connection status. Begin with FTE's native ICE path and a compatible
broker adapter. Isolate the necessary FTE code behind ezQuake allocation, time,
socket, TLS and logging adapters; preserve upstream notices and record each ported
file/commit. Its network files depend on FTE internals, so importing them unchanged
is not an implementation plan. The proof-of-concept must establish the actual
dependency slice and the exact native ICE + DTLS wire mode first.

Bundle a maintained DTLS provider in the build/package, with pinned dependencies
and notices. Current cURL use does not prove we already have a usable DTLS or
WebSocket backend. Avoid browser SCTP/data-channel machinery unless the compatibility
experiment proves it necessary. Browser players are outside the first release.
If extraction becomes impractical, evaluate a maintained ICE library against the
same broker fixtures before choosing it; do not silently change the server protocol.

Adapt `NET_GetPacketEx` and `NET_SendPacketEx` in `src/net.c`. Preserve datagram
boundaries and existing QW netchan sequencing/reliability. Do not tunnel gameplay
through the signaling WebSocket or add a reliable ordered stream underneath it.
Set an explicit path-MTU/payload policy accounting for DTLS and optional TURN;
oversized packets must be handled deliberately rather than silently truncated.

Current `src/net.h` represents IPv4, loopback and invalid addresses. Add an opaque
peer address containing a session generation and peer ID. Audit comparison,
hashing, printing, bans, qport matching, connectionless requests and every raw-IP
assumption in both server and client. Never represent a guest as loopback, a shared
proxy IP, or a fabricated IPv4 address. Keep the real candidate addresses inside
the transport. IPv6 candidates are a separate tested capability; this abstraction
alone does not prove IPv6 support across the engine.

Use bounded queues and nonblocking progress; an unreachable broker must not stall
rendering or local simulation. Route selection may change without changing logical
peer identity. Disconnects, map changes, suspend/resume, stale callbacks, slot reuse
and host shutdown must release sockets and relay allocations. A lost broker should
disable new invitations while established direct sessions continue where possible.
Test this explicitly rather than inheriting FTE's disconnect behavior blindly.

### Integration points

| Existing area | Planned change |
| --- | --- |
| `src/menu_local.c` | Friends mode uses existing KTX setup; separate map start from invitation lifecycle. |
| `src/menu_ingame.c` | Host invite/guest controls; reuse existing online KTX permission checks. |
| `src/net.h`, `src/net.c`, `src/net_chan.c` | Logical peers and transport dispatch; preserve QW packet behavior. |
| `src/sv_main.c` | Session admission, spectator policy, challenge and reconnect identity checks. |
| `src/cl_main.c`, `src/sys_win.c` | Typed invite parser and running-instance handoff; preserve legacy URL/console behavior. |
| `dist/windows/starter/Launch.ps1` | Validated join argument, correct data directory and optional URI registration. |
| `src/EX_browser*.c` | Optional later room endpoints; keep conventional QW routes/proxies operational. |

Start with the Join friends screen; public room browsing is not needed for invites.
If added later, room endpoints require a tagged representation instead of forcing
them through the current IP-address parser. Ping stays “unknown” until measured.

The parser accepts only the documented version, service IDs and bounded fields.
No console-command concatenation from URLs; reject control characters, malformed
escapes, duplicate keys and unexpected fields. Broker/TURN destinations must follow
the configured service policy, not arbitrary URLs in incoming invitations. Do not
place permanent relay credentials in the executable. Defaults should not connect
to the broker during offline bot play.

## Ordered implementation and acceptance gates

1. **FRIENDS-002 — transport feasibility.** Build a disposable native harness with
   pinned FTE signaling/ICE, compatible DTLS and the existing broker. Confirm the
   `FTE-Quake` namespace and distinct project room prefix are appropriate, including
   service usage expectations before distribution. Test two owned endpoints on
   different real networks, verify public listing behavior and record which relay
   candidates, if any, are actually offered. No new relay deployment. Gate: measured
   bidirectional datagrams over direct ICE; an honest coverage report if relay
   fallback is unavailable. Do not market or package this harness as private play.
2. **FRIENDS-003 — QW transport integration.** Add logical peers and exercise a real
   embedded KTX match, multiple clients and bots. Gate: no loopback privilege leak,
   stable peer identity, ordinary UDP/LAN/QWFWD operation preserved, bounded MTU.
3. **FRIENDS-004 — session admission.** Add authenticated invitation links, host
   approval for code-only joins, rotation, close/reopen and spectator checks. Gate:
   public listing knowledge, raw UDP, stale tokens and reused peer slots cannot
   bypass admission; malformed requests and floods are bounded.
4. **FRIENDS-005 — menus and links.** Implement Local Arena, in-game Friends and
   Join friends using current widgets; Starter URI handler and running-instance
   handoff. Gate: a friend joins by pasting/clicking with no console, account,
   manually entered password, extra installation or requested router change.
   An OS firewall prompt may still be necessary; scope it to the application.
5. **FRIENDS-006 — resilience and existing relay.** Validate advertised TURN only
   where an existing compatible service is available for use. Test reconnect,
   broker outage, route loss and host shutdown. Gate: no false success, no infinite
   spinner, no surprise public access and no gameplay regression on direct routes.
6. **FRIENDS-007 — beta packaging.** Ship dependencies and notices with matching
   source, redacted diagnostics and concise player help. Test fresh Starter and
   an existing user install. Publish the measured connectivity limitations.

If phase 1 cannot establish the required existing-service compatibility, stop
that path and report the blocker. Do not substitute a new relay infrastructure.
Automatic router mapping may be investigated separately as an optional built-in
aid; it cannot be relied on for CGNAT and is not a prerequisite for this plan.

## Validation matrix

| Scenario | Required evidence |
| --- | --- |
| Two machines on one LAN | Separate peers; no duplicate local player or accidental admin rights. |
| Two ordinary home connections | Actual match over direct ICE; measured latency, loss and jitter. |
| Home connection and mobile/CGNAT | Test both host directions; record success, route or clear failure. |
| Both sides restrictive / UDP blocked | Existing relay succeeds if supported; otherwise finite actionable failure. |
| Existing TURN offered | Allocation, permission, authentication, expiry and cleanup proven with real datagrams. |
| Two guests behind one public IP | Independent challenges, approval, qport, disconnect and reconnect. |
| Map change and bots | Invitations survive appropriate map transitions; KTX permissions unchanged. |
| Broker down before/during play | Local mode works; established routes handled independently; new joins report failure. |
| Sleep, network change, loss and reordering | No stale-peer admission, stalls, memory growth or corruption. |
| Public room and direct-address attacks | No invitation-secret exposure; no bypass via legacy/spectator paths. |
| Parser and admission | Unit/fuzz tests for malformed links/signaling, replay, expired sessions and size limits. |
| Fresh/existing Windows installs | Link opens the correct build/data directory; cfg/autoexec precedence preserved. |
| Legacy QW and proxy sessions | Existing connect, browser, demo and online KTX flows retain behavior. |

Local mocks and two processes behind the same router cannot establish internet
NAT traversal. Report simulator results separately from real-network evidence.
Full FTE client interoperability is a distinct test: broker compatibility does not
automatically make our new invitation/admission protocol usable by unmodified FTE.

## Pinned references

FTE: `f937b9d88f71fc4429db5fe56c6a98d922711b2e`.
QWFWD: `576214fcc1e7efec6234a4fa360e621c0dde7c44`.
MVDSV comparison: `c982fce8c813b9150396b6ff5b05cd735677490e`.
ezQuake-Vulkan inspected: `81a9c150710800adb7e0c447beb70fef1c08f864`.
These identify inspected revisions, not promises of current upstream head.

[fte-hosting]: https://github.com/fte-team/fteqw/blob/f937b9d88f71fc4429db5fe56c6a98d922711b2e/specs/hosting.txt
[fte-net]: https://github.com/fte-team/fteqw/blob/f937b9d88f71fc4429db5fe56c6a98d922711b2e/engine/common/net_wins.c
[fte-ice]: https://github.com/fte-team/fteqw/blob/f937b9d88f71fc4429db5fe56c6a98d922711b2e/engine/common/net_ice.c
[fte-master]: https://github.com/fte-team/fteqw/blob/f937b9d88f71fc4429db5fe56c6a98d922711b2e/engine/server/sv_master.c
[qwfwd-svc]: https://github.com/QW-Group/qwfwd/blob/576214fcc1e7efec6234a4fa360e621c0dde7c44/src/svc.c
[qwfwd-peer]: https://github.com/QW-Group/qwfwd/blob/576214fcc1e7efec6234a4fa360e621c0dde7c44/src/peer.c
[ezq-settings]: https://ezquake.com/docs/settings/multiplayer.html


## Follow-up requirement: persistent, explicitly replaceable invitation

User direction, 2026-09-16: choose the simplest implementation that lets the host
reuse an invitation for as long as they want. This supersedes the earlier rotation
counter proposal. This is future gameplay integration scope, not implemented in
probe 0.1.1.

Generate the invitation once and save its room locator, host transport identity
and cryptographically random access secret in private application state. Reuse it
on restarts, new hosted matches and normal engine updates. Keep that state separate
from shared configs, autoexec, reports and source control. No MAC address or separate
rotation counter is needed. Any URI format version is only for parser compatibility.

Provide one explicit Change invitation action. Atomically replace the saved access
secret and generated invitation, and accept only the current secret for new
admissions. Old links stop admitting guests. Existing admitted peers stay unless
the host removes them. Closing a hosted game makes it unavailable; reopening with
the saved identity makes the same invitation usable again. Losing/resetting the
private state requires a new invitation. Do not allow concurrently hosted games
with the same saved identity to silently replace each other.

FTE's current broker does not provide verified permanent ownership/reservation of
a room while its host is offline. Reuse the saved opaque room ID when reconnecting,
but handle a conflicting registration explicitly without silently replacing the
user's invitation. The saved host identity lets guests reject an impostor; it does
not make an occupied room reachable. Prove this availability behavior before
promising an always-reserved permanent address. No new external service is assumed.
