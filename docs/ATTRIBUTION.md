> Updated 2026-09-15: the current native menu/preset behavior is documented in
> [Unified menus](history/UNIFIED-MENUS.md). Graphics presets contain 170 values;
> Quick WASD is default; Quick ESDF and nQuake are alternatives. Earlier menu coordinates and the
> 53-setting profile counts below are historical. Use the current
> [TODO](history/TIBAZERA-TODO.md) and
> [validation evidence](../provenance/unified-menus-validation.json).

# Attribution and provenance

## Raster follow-up and latest defaults (2026-09-15)

OpenAI Codex implemented the deferred scene, HDR/SSAO/MSAA, raster shadow and
multiview changes, including the user-reported DM6 baked-light correction.
Ordered patches 18–22 and provenance/source-changes.json identify these edits;
upstream notices and history are retained. The user selected the current
71 graphics and 55 conditional-effect defaults, packaged by Codex and verified
through the real engine. Earlier default values below are historical.


## MAINT-005 correction: conditional particle settings (2026-09-13)

OpenAI Codex incorrectly classified 55 settings as unsupported during MAINT-001.
All 55 are registered by InitVXStuff when QMB particle assets initialize. The
earlier isolated fixture lacked the ezQuake particle resource pack; its unknown
command messages did not establish missing engine support. Removing those
settings reset enhanced lightning and other effects. The removal has been reversed
from the per-file backups, and the destructive cleanup script is disabled.

The user's requested bloom restoration is 0.1 strength, 0.7 threshold and 2.75 radius.
Lightning is restored to 1 with sparks 0.4. Other current graphics and bindings
are preserved. The public defaults include a separate 55-setting particle module;
the visual-only profile still contains 53 values. Runtime fixtures now include
the installed ezquake.pk3 before initialization. Earlier claims that these 55
settings were unsupported or safe to remove are superseded by this correction.

Debug weapon/shaft fire and release tests pass with the restored settings.
The combined public profile test verifies all 53 visual values and all 55 conditional
values after saving through the real engine. Engine source/binaries are unchanged.
Earlier performance measurements used the former bloom/particle state and are
historical evidence, not measurements of the restored defaults.

Menu paths: Options > Graphics > Visual Effects > Effects for Bloom strength,
threshold and radius; Options > Graphics > Advanced Options > Lighting > Particle
Shaft for the enhanced lightning switch. Lightning color, size and sparks do not
yet have dedicated menu rows.


This project retains the complete ancestry of tibazera's Vulkan branch at
`91859a996daede0df166e2a55a5f08d56b052b21`. Original Git authors, committers,
copyright notices and licenses are retained.

| Contribution | Credit |
|---|---|
| Quake, QuakeWorld and inherited engine code | Original notices and Git history, including id Software, FuhQuake and ezQuake contributors |
| SDL3/Vulkan base | tibazera/ezquake-source, feature/sdl3-vulkan-pr and its recorded contributors |
| Changes above that base in this project's first source snapshot | Implemented by OpenAI Codex on 2026-09-13 under user direction |
| Feature priorities, approved visual values and requested control layout | User direction; no claim of human source review or implementation is implied |
| Legacy control aliases | Adapted from a user-customized FuhQuake config; earlier snippet authors are not established |
| Independent KTX gameplay patch | Codex C adaptations of GPL id Software rerelease QuakeC fixes; original source credit retained |

The first project commit groups the local development snapshot. It does not
reattribute the inherited code or fabricate individual historical commits for
each development step. The ordered engine patches in `patches/` and
`provenance/source-changes.json` distinguish local changes by ID and source path.
`docs/history/ATTRIBUTION.md` and `docs/history/DEVELOPMENT.md` preserve the English
development ledger. Their references to uncommitted changes and private test
paths describe the original development workspace, not additional published files.

The inherited `CONTINUE.md` documents earlier assistant-assisted work and remains
unchanged. Its references to Claude, Opus, Fable or an earlier Codex session do
not refer to this project's 2026-09-13 implementation session, nor do they establish
line-by-line authorship.

The renderer is derived from the ezQuake Vulkan fork. The local vkQuake-RT and
RTGL1 experiments are separate; their presence in the development notes does
not mean ray tracing has been merged here. Related standalone patches are
included as reference material, with their own pinned bases in `sources.lock.json`.

Commercial game data, user logs/demos, full personal configs, machine credentials
and custom crosshair images are excluded. This ledger records provenance; it
does not change licenses or transfer ownership.

Latest update: Explosion without the ring (11) is implemented alongside Big
explosion (7). Portable defaults match the latest saved settings. See
[the RT feasibility assessment](history/RTX-FEASIBILITY.md); RTX 3060 rendering
validation and the optional RT backend are still future work.

RT-PLAN-002: the user selected broad host/donor effect migration with a distinct
RT appearance. OpenAI Codex authored the source-reviewed
[implementation plan](history/RT-IMPLEMENTATION-PLAN.md) and related documentation.
This is planning work only; proposed adapters and RTGL1 extensions are not yet code.

## Optional RT foundation

OpenAI Codex authored the RT device/package adapter, SDL3 hardware harness,
CPU geometry adapters and tests. The vendored RTGL1 SDK header retains its MIT
notice and API; one comment dash was normalized to ASCII for portable patches.
RTGL1 changes are supplied as attributed patches against the pinned upstream
source. Existing RTGL1 code and shaders retain their original authorship.
This foundation is not a completed ezQuake RT gameplay renderer.


## 2026-09-15: CFG-PRESETS-002 — preset activation

User report: Apply was unclear and graphics presets could not be loaded intuitively.
OpenAI Codex diagnosed and fixed the native browser: Enter previously only previewed
the focused file. Moving to Apply could preview other rows along the way; leaving
the browser then cancelled the preview. Enter or a click now commits the focused
valid preset and returns to Graphics. F3 keeps the preview without moving focus;
Escape cancels uncommitted changes. Empty/invalid confirmation cannot report success.
Pending video changes still require F5 after loading, with an on-screen reminder.

Patch 24 records the implementation separately from patch 23. Debug (800x600) and
Release (640x480) tests exercise native Enter, mouse down/up, arrows and F3, validate
all 170 preset values after menu exit, and check cancel/invalid-file behavior plus
audio/binding isolation. The existing Release save/backup/scope/restart regression
also passes. See provenance/preset-activation-validation.json for binary/source
hashes and limitations. Earlier unified-menu evidence describes the preceding build.


## DIST-001 — Windows x64 test distribution, 2026-09-15

User requested a deploy package and GitHub publication. OpenAI Codex packaged the
Release Vulkan raster client with static runtime libraries, all dependency copyright
notices, portable graphics/keyboard presets, setup/diagnostic launchers, an exact
source-commit manifest and a matching source ZIP. Game assets, private configurations,
KTX binaries, RT harnesses and debug symbols are excluded. Requires existing nQuake
data. Quick WASD/Quick ESDF/nQuake and the updated Balanced profile are included.

A launcher-managed first-run flag loads ezv-dist-first-run.cfg before the existing
config chain, including after startup resets. User config and autoexec values win;
normal launches are unchanged. Setup preserves existing files and installs missing
namespaced profiles. Normal engine saving retains its existing cfg_save_onquit policy.
The distribution marker is written only after exit code 0. Diagnose.cmd collects
qconsole.log and package identity. No autoexec or full user config is rewritten by setup.

Patch 26 records the startup hook. Isolated packaged-binary tests cover fresh,
existing-config/autoexec, and normal starts, all 170 graphics values and repeated
setup preservation. Debug/Release compilation and source patch replay pass. See
provenance/distribution-validation.json. Earlier records describe earlier binaries.
The public test is version 0.1.0-beta.1; no RTX functionality is claimed.


## DIST-002 - downloadable Windows Starter (2026-09-16)

At the user's request, OpenAI Codex implemented an unofficial nQuake-based
installer with verified upstream downloads, fresh-directory staging, optional
owned classic PAK import, Balanced / Quick WASD first-run presets and relative
launchers. Original nQuake autoexec and subsequent user config precedence are
preserved. GUI-process exit waiting fixes the beta.1 launcher's premature exit
report. Engine binary and source remain the pinned 7bb8a686 release. No commercial
assets, private configs or local demo files enter the source or Starter archive.
See docs/STARTER.md and provenance/starter-validation.json in the public tree.


## INPUT-005 - original Quick crosshairs (2026-09-16)

The user explicitly requested inclusion of the five PNGs from their legacy Quake
installation. The images retain their original bytes; their original authors are
not established. Neither the user nor OpenAI Codex is credited as image creator,
and no new license or GPL relicensing is asserted for the images.
OpenAI Codex integrated the shared crosshair CFG, Quick profiles, Starter 0.1.1
payload, separate update ZIP, tests and documentation. Engine code/binary unchanged.
Crosshair size 2.5 and the original weapon aliases replace public built-in substitutes.
User config/autoexec files are excluded; installation does not rewrite them.
See provenance/crosshairs.json and docs/CONTROLS.md in the public tree.


## DIST-003 - simplify the undistributed Starter release (2026-09-16)

At the user's request, OpenAI Codex removed the separate crosshair update package
and its packaging step, and simplified the current release notes and installation
guides to direct players to Starter. Original crosshairs remain bundled. The old
engine archive and matching source remain available because Starter pins that
download; download hashes, engine code and installed configuration behavior are
unchanged. Historical implementation/test records are retained as provenance.


## FRIENDS-001 - existing FTE infrastructure research (2026-09-16)

The user requested easy friend-hosted games without a newly operated external
relay and preferably without additional player-installed software. OpenAI Codex
inspected pinned FTE, QWFWD and MVDSV sources and this fork's networking/menu code,
then wrote docs/history/FRIENDS-HOSTING-PLAN.md. The proposal uses existing FTE room signaling,
native ICE, authenticated invitations and the embedded KTX server. Short room
codes require host approval because broker rooms may be publicly listed.

Read-only verified TLS/HTTP and one STUN probe against the existing Frag-Net master
succeeded. No room was registered, no TURN allocation was requested, and no game
was joined. Direct gameplay across separate networks and usable existing TURN
coverage are unverified acceptance gates. Ordinary QWFWD/Qizmo client forwarding
was not established as a reverse-hosting solution. No new service was deployed.

This change adds planning, TODO and provenance documentation only. Engine source,
binaries, installed configuration, launchers and published releases are unchanged.
FTE networking work remains credited to its upstream authors; no donor code was
ported. See provenance/friends-hosting-research.json for inspected source revisions,
hashes, sanitized checks and limitations. Implementation tasks FRIENDS-002 through
FRIENDS-007 remain open.


## FRIENDS-002 - native connection prototype and test package (2026-09-16)

OpenAI Codex implemented a standalone Windows x64 FTE-broker adapter, native
libjuice ICE, OpenSSL DTLS, host certificate pinning and secret invitation admission.
No FTE implementation files were copied. The user stated that a second computer
on another network is unavailable now, so the planned two-network gate remains
open and engine/menu integration is deferred pending that evidence.

Debug/Release self-tests, real-broker local pair tests, wrong-key/wrong-fingerprint
rejection, host timeout, expired invitation and extracted-package tests pass.
Host.cmd and Join.cmd require no separately installed runtime. The local ZIP has
matching source/dependency archives, notices and hashes. No new external relay or
other server was deployed; no TURN allocation was attempted. No GitHub release was
published, and no current game binary, launcher or user configuration was changed.
See FRIENDS-HOSTING-STATUS.md and provenance/friends-probe-validation.json.


## FRIENDS-002C - wait for a guest without a deadline (2026-09-16)

The user requested leaving the probe open for a guest joining on a later day.
OpenAI Codex made --host default to unlimited waiting and Host.cmd select
--seconds 0 explicitly. Room metadata and WebSocket keepalives refresh every
30 seconds. Idle polling uses 100 ms; active test polling remains 5 ms. Explicit
finite test durations, console cancellation and the bounded active attempt remain.
A certificate-date regression check verifies the existing invitation-pinning policy
continues working after the ephemeral certificate's date window. No host identity
or invitation is rotated just because time passes. Sleep/network-loss reconnection
is still outside the prototype. The updated standalone package is 0.1.1; engine,
user configuration and published Starter are unchanged. See the wait validation
record in provenance/friends-probe-wait-validation.json.


## FRIENDS-003/004/005A - native gameplay and reusable invitations (2026-09-16)

Requirements/product choices: AWK. Implementation, tests and documentation:
OpenAI Codex. The user supplied join-result.json and confirmed host/guest were in
different cities: direct ICE, pinned DTLS, admission and 100/100 echo packets pass.
That evidence opened the engine-integration gate for this network pair.

New code in src/friends.c and src/friends/* implements asynchronous native
multi-guest transport, opaque NA_FRIENDS addressing, protected persistent identity,
secret rotation, admission gates and existing-widget menus. Local Arena can open
the saved room after starting KTX; online guests cannot administer another host.
OpenSSL symbols are kept separate from the inherited unrelated SHA1 functions.
Broker slot reuse, stale join generations, graceful departure and map sign-on were
covered during integration. No existing graphics/key presets or autoexec changed.

Release gameplay validation uses a host, two guests and one arena bot through
the real Frag-Net service. Rejoining in the same client, chat and DM6 -> DM2 work.
Separate tests cover normal UDP before hosting, rejection of plaintext bypass,
binary fragmentation, persistent identities, wrong keys/pins, close/reopen and
rotation. See provenance/friends-engine-validation.json for exact source/binary
hashes and the scope of each result. Wider network coverage and distribution
integration remain TODO items; no new external service or GitHub release was made.

New adapter code is GPL-2.0-or-later. libjuice remains MPL-2.0; OpenSSL remains
Apache-2.0. Distribute the combined Friends-enabled binary under GPL-3.0-or-later
with matching sources and dependency notices. FTE broker/protocol authors and
ezQuake/QW/KTX authors retain their existing credit; no FTE implementation files
were copied. See docs/FRIENDS.md for current player instructions and limitations.

## 2026-09-16 — Native Windows invitation registration (FRIENDS-005B)

Requested explicitly by AWK; implementation, test automation and notes by OpenAI
Codex. Friends -> Windows links registers the current executable and data folder
for ezquake-vulkan:// in HKCU. A matching running instance receives validated data
through a bounded local mailslot; otherwise the engine starts and asks the player
to confirm. Existing qw:// registration and personal configuration are preserved.

Actual Windows shell dispatch revealed the OS inserts a slash before the query.
The parser now accepts that equivalent spelling while preserving old invitations
and strict validation elsewhere. Cold start, warm handoff, cancel/confirm, secret
redaction and rejection of appended console arguments pass in the native engine.
The test config polling was moved out of startup's Cbuf_Flush loop so tests now
exercise completed initialization. Two guests, a bot, same-process rejoin, chat
and DM6 -> DM2 were then revalidated through the real broker.

The development installation is registered at AWK's request. Distribution/Starter
updates and cross-city engine gameplay remain separate pending work. See
provenance/friends-engine-validation.json for sanitized evidence.

## 2026-09-16 — Friends beta distribution

AWK requested a new test package, GitHub push/release, refreshed installer and
retirement of the unused old Starter. OpenAI Codex prepared version 0.2.0-beta.1
and Starter 0.2.0, with an exact source commit, pinned engine download, upstream
notices and matching Friends dependency sources. The installer offers registration
only after the final directory exists; interactive choice or -RegisterLinks is
required. Package tests use nQuake's distributed KTX QVM as well as the development
DLL. Personal configs and commercial assets are excluded from release archives.
Final release verification is recorded separately under provenance.

Release verification: the published engine is built from commit 4e70d587.
The extracted Starter passed first/second launch, E1M1 save, all 170 Balanced
values, Quick crosshairs, bot arena and installer integrity checks. The packaged
KTX QVM also passed host/two-guest Friends gameplay, rejoin, DM6 -> DM2, URI
cold/warm start, confirmation and redaction. This remains local network-path
evidence through the real broker, not cross-city engine gameplay evidence.
