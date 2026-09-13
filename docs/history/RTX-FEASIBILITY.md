# RT integration feasibility

Author: OpenAI Codex. Investigation requested by the user, 2026-09-13.
Applies to the pinned sources in sources.lock.json and the current local fork.

## Decision

Proceed with an optional RTGL1 renderer prototype. Source inspection found a
credible integration path, but successful RT output in ezQuake is not yet proven.
The existing ezQuake executable remains a Vulkan raster renderer on an RTX GPU.
This investigation makes no engine or user-configuration changes.

The user subsequently selected direct integration with broad host/donor effect
coverage and accepted the RT renderer's own appearance. The detailed next steps
are in [RT-PLAN-002](RT-IMPLEMENTATION-PLAN.md), including RTX 3060 hardware gates,
RTGL1 shader extensions and separate visual profiles. This supersedes the limited
prototype scope below; the source/device findings remain applicable.

## Evidence checked

- ezquake-vulkan/src/r_main.c currently selects classic GL (0), modern GL (1),
  or Vulkan (2). The source/CMake audit found no RTGL1 integration or Vulkan
  ray-tracing pipeline/acceleration-structure implementation.
- Local vkquake-rt Debug executable, RTGL1 Debug DLL, 51 compiled RT shader
  binaries and the blue-noise resource exist. Their presence is build preparation,
  not proof of a complete, working runtime package.
- Re-ran build/vulkan-probe/vulkan-probe.exe successfully. It enumerated one
  AMD Radeon(TM) Graphics device, Vulkan 1.2.188, with neither
  VK_KHR_ray_tracing_pipeline nor VK_KHR_acceleration_structure advertised.
  Result: cache/rt-feasibility-device.txt. No RT frame was attempted on this GPU.
  This test checks extension advertisement, not the complete RTGL1 feature set.
- RTGL1's quake-branch RgInstanceCreateInfo takes a native window surface and
  creates its own device (Source/VulkanDevice_Init.cpp). It does not expose
  injection of ezQuake's existing VkDevice or command buffer through this API.
- RTGL1 provides static/dynamic geometry, material, light and raster-overlay
  upload APIs. vkquake-rt demonstrates their use in r_world.c, r_brush.c,
  r_alias.c, gl_rlight.c and gl_draw.c. The donor remains SDL2; ezQuake is SDL3.
- RgMaterialCreateInfo accepts original RGBA texture data and optional overrides;
  new game textures are not a prerequisite for the initial scene prototype.
- ezQuake's server explicitly filters player/entity updates using PVS in
  SV_PlayerVisibleToClient and SV_EntityVisibleToClient (src/sv_ents.c).
- The local RTGL1 build has RG_WITH_NVIDIA_DLSS=OFF. Ray tracing and DLSS are
  separate capabilities; enabling RT does not automatically enable DLSS.

Upstream also describes vkquake-rt as a path-tracing integration using RTGL1:
https://github.com/sultim-t/vkquake-rt . Local API conclusions are based on the
pinned files, not an assumption that the default RTGL1 branch is compatible.

## Architecture and scope

Give the RT backend exclusive ownership of its device and presentation while
selected. Adapt the CPU-side scene and SDL3 native window to RTGL1. Keep the
existing raster backend available. Do not merge vkQuake's gameplay/network code.
The renderer function table is a useful starting point, but its texture/buffer
and lifecycle APIs still need adaptation; it is not a drop-in RT interface.

Upload the static BSP beyond the raster view's visible surface list so secondary
rays have scene geometry. Use stable IDs for moving brush/alias models and lights,
with explicit deletion and history reset on map changes, teleports and demo seek.
Start with one view. Independent temporal history for ezQuake multiview remains
an unresolved integration requirement.

The current competitive visual shader effects, FXAA/MSAA and postprocessing do
not automatically transfer to a separate RTGL1 frame pipeline. Map equivalent
controls deliberately or show their unsupported state. Particles, coronas and HUD
can use the raster overlay API, but still need an adapter and visual verification.

Original Quake geometry/textures can serve as the baseline. Material roughness,
emission and light intensity still need sensible defaults. A client cannot render
a current off-screen opponent reflection if the server did not send that entity;
do not retain stale opponents or change the network protocol as a hidden fix.

## Smallest useful next milestone

1. On the intended RT-capable machine, check all requested device features and
   load the exact RTGL1 DLL with its shaders and required resources. Render a
   known triangle/light and verify normal shutdown. Check the native-window ABI.
2. Add an optional ezQuake RT backend that renders one static single-player map
   with original textures, a camera and HUD. Verify switching back to raster and
   map reload. No dynamic multiplayer scene is required for this milestone.
3. Add animated entities, dynamic lights, water and particles; validate demo seek,
   entity removal, settings and moving-camera quality on real RT hardware.

The first milestone is a compatibility experiment. Full multiplayer-quality RT
is a renderer integration project with several stages, not a small effects patch.
The user identified the target GPU as GeForce RTX 3060. NVIDIA documents hardware
ray tracing for this family, making it a suitable prototype target:
https://www.nvidia.com/en-us/geforce/graphics-cards/30-series/rtx-3060-3060ti/ .
The exact VRAM variant and installed driver were not inspected. Start validation
at 1280x720, then measure 1920x1080; these are proposed test resolutions, not an
FPS guarantee. Actual target-machine device checks and rendered frames are still
required before making performance or complete runtime-support claims.
