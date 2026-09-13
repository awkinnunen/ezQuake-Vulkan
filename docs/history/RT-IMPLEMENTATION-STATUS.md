# RT implementation status

Author: OpenAI Codex, 2026-09-13. Scope requested by the user: steps 1 through 7
of the summary of RT-PLAN-002, corresponding to milestones RT-00 through RT-06.

## Current deliverable

The first dependency test is implemented and packaged. This is **not a playable
ezQuake RT renderer**, and steps 1 through 7 are not complete. The user reported
that the RTX 3060 machine is unavailable and will run the first hardware test.

| Requested step | Plan milestone | Actual status |
|---|---|---|
| 1: inventory and dependencies | RT-00 | Extracted 446 feature candidates, visual menu references, all 108 saved settings, 45 device feature bits, seven extensions and 26 API exports. Runtime validation implemented. Semantic feature mapping remains open. |
| 2: first RT image | RT-01 | SDL3 triangle/light harness builds in Debug and Release; package and failure tests pass. Actual RT image, resize and successful GPU shutdown await RTX 3060 testing. |
| 3: backend lifecycle and HUD | RT-02 | Library initialization cleanup and Release error reporting started. CPU overlay submission and logical HUD projection now have instrumented tests. Host renderer dispatch, actual 2D drawing callbacks and renderer-switch fallback are not implemented. |
| 4: maps and materials | RT-03 | Convex polygon triangulation and affine transforms implemented and CPU-tested. BSP/material callbacks and map uploads are pending. |
| 5: models and lights | RT-04 | Pose interpolation and bounded ID packing implemented and CPU-tested. Dynamic entity IDs still require host lifetime tracking; moving-model/light callbacks are pending. |
| 6: all effects | RT-05 | Primitive and blend/depth submission helpers implemented and CPU-tested. Particle/emitter and donor-effect connections remain pending. |
| 7: Competitive Visuals and menus | RT-06 | Planned RTGL1 shader/API extensions and separate profiles not implemented. |

No requested effect has been removed from scope. The first real-device gate is
still open, so later milestones must not be marked hardware-verified.

## Implemented source

- `src/rt/rt_probe.c`: queries all enabled feature bits and device extensions
  from the pinned library's CreateDevice requirements. Reports each GPU, missing
  capabilities and a compatible device UUID. Local AMD rejection is expected.
- `src/rt/rt_api.c`: validates package schema, header identity, required shader
  filenames and SHA-256 file hashes; loads the DLL from an explicit directory;
  resolves all required exports and the additive extension API. Uses SDL3 native
  window properties, with no SDL2 dependency.
- `src/rt/rt_harness.c`: creates a standalone SDL3 window, uploads a static
  triangle and per-frame spherical light, presents frames and handles close/resize.
  Exit zero means submitted frames, not a verified correct image.
- Optional `RENDERER_RTGL1` CMake setting builds the runtime adapter and harness.
  It is off by default and does not add `vid_renderer 3` to the game yet.
- RTGL1 additive exports `rgEzqGetApiVersion` and `rgEzqSelectDevice` bind creation
  to the preflighted UUID instead of silently choosing another GPU.
- RTGL1 central construction failure cleanup releases successfully constructed
  members and owned Vulkan instance/surface/device/synchronization handles.
  Thirteen initialization Vulkan results are checked in Release as well as Debug.
  Null C API creation arguments and C++ initialization exceptions return errors.

The central cleanup does not prove exception safety inside every individual
RTGL1 component constructor. Failures during shader/texture/pipeline allocation
still need supported-device fault testing before claiming reliable host fallback.

## CPU geometry preparation (RT-GEOMETRY-001)

`src/rt/rt_geometry.c` implements checked convex-fan and triangle-strip conversion,
Quake-axis affine transforms, normalized pose interpolation, domain/map/slot/lifetime
ID packing, logical HUD projection and instrumentable geometry/overlay submission.
The standalone triangle uses this submission path. The host scene, texture, HUD
and particle callbacks do not yet call it. The ID helper does not itself track
entity lifetimes, and raster overlays do not become ray-traced geometry.

`ezquake-rt-cpu-tests` exercises seven fixture groups in Debug and Release, without
loading an RT DLL or opening a GPU window. Fixtures check winding/area, invalid
capacity/overflow, model interpolation and UV/color retention, HUD corners, ID
bounds/generations, upload-error propagation and depth/blend flags. These establish
conversion behavior, not the completeness or visual correctness of steps 3-6.

## Validation

- MSVC x64 Debug/Release runtime adapter, harness and RTGL1 library builds.
- Thirteen package/API boundary tests: valid package, rejected selected GPU,
  header/extension/schema mismatch, omitted/corrupt/missing shader, invalid path,
  malformed/missing manifest, null UUID and null instance arguments.
- Sixteen consecutive rejected-device creation attempts in one process, including
  Debug Vulkan validation with no VUID or Vulkan error messages.
- Local device: AMD Radeon(TM) Graphics, Vulkan 1.2.188, 2048 MiB device-local heap.
  Neither RT pipeline nor acceleration structure support is available.
- RT-disabled host Debug/Release build and a separate Vulkan raster smoke fixture
  are run to protect the existing executable. Detailed results are in VALIDATION.md.
- CPU geometry fixtures pass in Debug and Release; logs: `cache/rt-geometry-debug.log` and `cache/rt-geometry-release.log`.
- All 51 RT shader binaries pass `spirv-val --target-env vulkan1.2`.
- Actual RT rendering, effect output, successful RT resize/shutdown and performance
  are not verified. There is no substitute raster screenshot presented as RT proof.

Local evidence: `cache/rt-foundation-results.json`, `cache/rt-device-probe.log`,
`cache/rt-rejection-validation.log` and the `runtime-rt-foundation-raster` fixture.

## First RTX test

Extract `ezQuake-RT-first-test.zip`, or copy the complete `runtime/rt-test-release` package to the RTX 3060 machine
and run `Test-RTX.cmd`. No game assets or Vulkan SDK are needed. The Release DLL
requires the Microsoft Visual C++ x64 runtime. DLSS is disabled.

Expected output: an orange triangle against a blue-grey background. Check that
resizing works and Escape closes the window. Return `RTX-test.log` and describe
what was visible. Exit 2 means no compatible device; exit 3 means package loading
failed. The launcher preserves the exit code and leaves the console open.

The water normal file is deliberately absent in this minimal fixture; the pinned
library supplies its built-in neutral fallback. The blue-noise texture comes from
RTGL1 Tools, whose README permits its use. No Quake/nQuake assets are packaged.

## Rebuilding

In the development workspace, run `scripts/Prepare-RTIntegration.py`, enable
`RENDERER_RTGL1=ON` in the existing CMake build, and build `ezquake-rt-harness`
for Debug and Release. Build the pinned RTGL1 quake branch with the attributed
patches, Windows surfaces enabled and DLSS disabled. Compile/validate the shaders
with `scripts/Build-RTShaders.ps1`, then run `scripts/Package-RTTest.py`.

The public source contains the generated ABI header, requirements and inventory.
Its harness builds with `-DRENDERER_RTGL1=ON -DRENDERER_VULKAN=ON` using the normal
Windows build instructions. `tools/Package-RTTest.py` accepts `--library-source`,
`--library-binary`, `--shader-directory` and `--output` for separate RTGL1 checkouts.
Keep the original RTGL1 notices when applying the library patch series.

CPU tests can also be built as target `ezquake-rt-cpu-tests` and run with
`ctest --test-dir build-msvc-x64 -C Release -R rt-geometry-cpu --output-on-failure`.
