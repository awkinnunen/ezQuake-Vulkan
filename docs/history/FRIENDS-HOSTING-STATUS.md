# Friends hosting implementation status

Updated 2026-09-16. Engine integration and reusable invitations are implemented.
Implementation/tests: OpenAI Codex; requirements/direction: AWK.

Current instructions and implementation details: [Friends games](../FRIENDS.md).
The user confirmed a successful probe between different cities. Native engine
tests now cover two guests plus host, bot startup through Local Arena, chat,
same-process rejoin, DM6 -> DM2, ordinary UDP and private admission rejection.
Persistence, wrong keys/pins, closed invitations and rotation have separate tests.
See [current evidence](../../provenance/friends-engine-validation.json).

Windows URI registration, cold start and running-instance routing are implemented
and tested with actual ShellExecute dispatch, confirmation/cancel and redaction.
Starter distribution integration, wider NAT coverage, existing TURN allocation,
sleep/network recovery and cross-city gameplay with the integrated engine remain
pending. The text below records the earlier standalone-probe milestones.

## Implemented

The standalone Windows x64 probe registers a random room through the existing FTE
broker, exchanges raw SDP/candidates, creates native ICE using libjuice, pins the
host certificate from a secret-bearing invitation and authenticates an invitation
key inside DTLS. It echoes 100 protected datagrams between one host and one guest.
Version 0.1.1 waits indefinitely for a guest by default, with 30-second WebSocket
keepalives and room metadata refreshes. Active attempts remain bounded; cancellation,
bounded queues and sanitized JSON results are retained.

The chosen prototype uses a maintained ICE library rather than copying FTE's
engine-dependent networking implementation. FTE signaling constants and protocol
behavior are implemented in a new adapter. There is no browser SCTP, no new broker,
no newly deployed relay and no game engine replacement.

Source and player instructions: [probe README](../../tools/friends-probe/README.md).
Package: `ezQuake-Vulkan-friends-probe-windows-x64.zip`, with matching source ZIP
and SHA-256 files. Both are local artifacts, not a GitHub release. Host.cmd copies
an invitation; Join.cmd accepts it as text. No additional software installation is
needed by testers. The executable imports Windows system DLLs only.

## Verified

- Release and Debug compile with the pinned manifest; native protocol/DTLS tests pass.
- Real Frag-Net TLS/WebSocket room registration and signaling work with two owned
  processes. Direct ICE and DTLS succeed; both receive all 100 expected datagrams.
- A wrong invitation key is rejected before test payload admission.
- A wrong invitation fingerprint is rejected; the crypto self-test also rejects
  a certificate mismatch during the actual DTLS handshake.
- An explicitly time-limited host exits and cleans up its blocking receive operation.
  Unlimited hosting is now the default; Ctrl+C/Break still cancels it.
- A guest cannot use the invitation after that host exits.
- The extracted package passes self-tests and the real-broker local pair test.
- Windows PowerShell 5.1 rejects malformed join text; the temporary invitation file
  is removed. Matching binary/source hashes and dependency notices were verified.

Evidence: [validation record](../../provenance/friends-probe-validation.json).
Observed relay offers were zero. This does not prove that no relay can ever be
available. The prototype counts offered metadata but does not allocate TURN.

## Historical pending gates (before the user's cross-city result)

The user confirmed that a second computer on another internet connection is not
available now. All successful live tests were on one Windows machine; they verify
real broker interoperability, not NAT traversal between separate networks. This is
why FRIENDS-002 remains open as a whole despite the completed prototype and package.

Run the two-computer test in the README, including reversing the hosting roles.
Only then proceed to FRIENDS-003 (engine transport), FRIENDS-004 (production session
admission), FRIENDS-005 (menus and system links), FRIENDS-006 (resilience/existing
TURN) and FRIENDS-007 (gameplay beta). The prototype's `probe` URI intentionally
cannot be mistaken for an implemented production `join` URI.

Multi-guest behavior, code-only host approval, hardened admission/resource limits,
IPv6 coverage, full FTE-client interoperability and a usable existing TURN fallback
remain unverified or unimplemented. Public room visibility is still expected.
Current engine binaries, launchers, user configs and the published Starter are
unchanged. No remote service is installed or administered by the prototype.

## Dependencies and attribution

vcpkg baseline: `66c0373dc7fca549e5803087b9487edfe3aca0a1`.
libjuice 1.7.0 provides ICE/STUN; OpenSSL 3.6.0 provides DTLS and cryptography.
Their exact source archives, vcpkg patches and license texts accompany the source
package. Check dependency updates before a production release. The actual libjuice
source license is MPL-2.0; the pinned port's LGPL metadata is inaccurate.

FTE authors retain credit for the existing broker and protocol. New adapter,
packaging and test code is identified as OpenAI Codex work. No FTE renderer or
network implementation files were copied into the engine.


## FRIENDS-002C - unlimited host waiting (0.1.1)

At the user's request, Host.cmd uses `--seconds 0`, and bare `--host` has the same
unlimited-wait default. A friend can use the unchanged invitation later while the
host process, computer and broker connection remain available. The probe still
exits when its one-peer test completes. Keep the computer awake; reconnecting
after sleep or a broken broker connection is not implemented.

An active peer attempt has a 180-second limit in unlimited mode. Explicit finite
limits remain available for automated tests, and Join.cmd retains its 90-second
attempt limit. WebSocket keepalive is explicitly set to 30 seconds, matching the
room metadata refresh. Idle polling uses 100 ms rather than the active 5 ms loop.
The pinned-certificate self-test now covers an expired date window, because the
invitation fingerprint anchors identity for the lifetime of the process.

The 0.1.0 validation record is historical. See the separate 0.1.1 wait validation
record for the current binary, source and test evidence. No 24-hour wall-clock test
or test between separate internet connections is claimed.

Validated: real-broker join after 330 idle seconds with the unchanged invitation,
100 protected datagrams each way, explicit-zero cancellation, finite deadline,
Debug/Release crypto tests including an aged certificate, and the extracted 0.1.1
package. See [wait validation](../../provenance/friends-probe-wait-validation.json).
