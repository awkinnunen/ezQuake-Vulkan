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

## Current validation

Implementation and packaging are in progress. No native Mac runtime pass is
claimed. Build and test results will be recorded here before release.
