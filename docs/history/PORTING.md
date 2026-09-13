# Vulkan and RT integration notes

Author: OpenAI Codex. Updated: 2026-09-13. These are local analysis and planning
notes, not statements by upstream maintainers.

## Current direction

Use tibazera's existing SDL3/Vulkan ezQuake branch. It is checked out in
`ezquake-vulkan/`; its initial Debug build succeeded without source changes.
LOCAL-007 subsequently added and tested the first corrections: compact checked
shader parameters, NPOT reporting, skybox bounds and alias directional lighting.
Both Debug and Release now include those changes. See VALIDATION.md and the
CODE-005..010 attribution entries for exact scope.
[PR #1145](https://github.com/QW-Group/ezquake-source/pull/1145) was open when
inspected in this session. This assessment applies to the pinned revision in
`sources.lock.json`, not later upstream changes.

The original plan to implement Vulkan from old ezQuake stubs is superseded.
See [TIBAZERA-TODO.md](TIBAZERA-TODO.md) for the active task list.

## Initial investigation (historical)

The original comparison checkout, `ezquake/` at `c0e22c4`, offered classic and
modern OpenGL CMake options. Its renderer function table in `r_renderer.h` and
`r_renderer_structure.h` included OpenGL types and concepts, so it was not a fully
API-independent abstraction.

`r_main.c` reserved renderer value 2 for Vulkan; `vk_*.c` contained incomplete
2018 code. Empty setup functions and inconsistent signatures meant a preprocessor
definition alone would not create a working backend. Git attributes old Vulkan
commit `5e582127709dca919d203df52d105b1c61a8520c` to meag; its message describes
the implementation as not yet coherent. No documented reason for stopping that
work was established here.

That base used SDL2, whereas current vkQuake uses SDL3. Discovering tibazera's
SDL3/Vulkan branch changed the plan. The initial estimate had not accounted for
the open PR and should not be used to estimate continuing the existing fork.

## Recommended sequence (Codex proposal)

1. Run the compiled fork with Quake data and Vulkan validation enabled.
2. Compare map changes, renderer changes, vid_restart, demos, seeking, multiview,
   skins, player colours and network play against OpenGL.
3. Reproduce historical reports and fix confirmed stability bugs.
4. Address verified visual differences and measure proposed world bindless work.
5. Design RTGL1 integration as a separate renderer phase.

Preserve source history and copyright notices when importing changes. A wholesale
merge of vkQuake is not the proposed approach.

## RT integration observations

vkquake-rt's README selects RTGL1's `quake` branch; the default `hl1` branch has a
different integration interface. RT calls remain in files named `Quake/gl_*.c`:
`gl_draw.c`, for example, uses `rgUploadRasterizedGeometry` for HUD geometry.
The Windows project expects `RTGL1_SDK_PATH`.

RT integration needs geometry, materials, lights, stable object identifiers and
temporal history for each view. QuakeWorld may omit entities outside the client's
visibility set. Determine available scene data before promising complete
off-screen reflections or indirect lighting in network play.

The local driver lacks advertised `VK_KHR_ray_tracing_pipeline` and
`VK_KHR_acceleration_structure` support. Installing an SDK does not add device
features. No driver update or RT runtime test was performed. The DLL alone is
not a complete shader/material resource package. Game data is not included.
