> Updated 2026-09-15: the current native menu/preset behavior is documented in
> [Unified menus](history/UNIFIED-MENUS.md). Graphics presets contain 170 values;
> Quick WASD is default; Quick ESDF and nQuake are alternatives. Earlier menu coordinates and the
> 53-setting profile counts below are historical. Use the current
> [TODO](history/TIBAZERA-TODO.md) and
> [validation evidence](../provenance/unified-menus-validation.json).

# Validation status

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


Recorded by OpenAI Codex, 2026-09-13. Windows x64, MSVC 2022, integrated
AMD Radeon(TM) Graphics, Vulkan API 1.2.188. Debug and Release builds passed.

| Area | Evidence |
|---|---|
| Immediate video-restart screenshots | Debug and Release, F5/MSAA transitions and clamped samples; five nonblank captures each, no validation errors |
| Moving gameplay | Three maps, six-player demo, forward/backward seek, pause, skin reload and video restart |
| Local Arena | Real KTX modes/rules, moving bots, join/leave, restart and single-player isolation |
| VSync timedemo | Three complete 1,290-frame FIFO runs; historical hang not reproduced on this GPU |
| Effect costs | Eight cases, warm-up plus two samples, identical frame counts and closing default baseline; see PERFORMANCE.md |
| Public WASD/defaults | Actual combined load and save preserves all 53 approved graphics values, W/A/S/D, dynamic Caps Lock jump and 8/8 pickup selection |
| Visual menu behavior | Earlier 48-control A/A/B/A image evidence, 25 dependency gates and requested/applied restart state checks |
| Source provenance | All 13 ordered engine patches reproduce the 61 changed/new source files exactly; source-path attribution and whitespace checks pass |

The first benchmark fixture was cut without its EndOfDemo marker and waited for
more data. Correcting that fixture made it complete; this was not a VSync fix.
Private baseline startup can reference custom images/skins absent in isolated
test profiles. The public controls use built-in crosshairs. KTX's inherited
`sv_enableprofile` warning is distinguished from unknown test commands.

These are bounded checks on one GPU. There is no new OpenGL parity pass, complete
campaign playthrough, long-session leak result, dedicated automatic-match/movie
screenshot regression, or RTX integration. See [testing](TESTING.md) for the
portable runtime harness and [historical validation](history/VALIDATION.md) for
earlier evidence, attribution and limitations.

## PUBLIC-002 - Publication cfg validation (2026-09-13, OpenAI Codex)

Vulkan Debug test publication-latest-cfg passed with normal exit 0 and no
validation errors. All 53 current visual settings and 55 conditional effects
survived the public profile load/save. WASD/CapsLock bindings, automatic pickup
selection 8/8 and built-in crosshair size 2.5 also passed. The latest settings
include Explosion 11, MSAA 4, model outline opacity 0.5 and contact AO 0.125.


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
