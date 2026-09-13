# Validation status

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
