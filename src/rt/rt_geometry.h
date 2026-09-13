/* RT-GEOMETRY-001, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#ifndef EZQ_RT_GEOMETRY_H
#define EZQ_RT_GEOMETRY_H
#include "rt_api.h"

typedef enum rt_primitive_e { RT_TRIANGLES, RT_TRIANGLE_FAN, RT_TRIANGLE_STRIP } rt_primitive_t;
typedef enum rt_blend_e { RT_BLEND_REPLACE, RT_BLEND_ALPHA, RT_BLEND_PREMULTIPLIED,
    RT_BLEND_ADD, RT_BLEND_MULTIPLY } rt_blend_t;

/* Convex polygons only. The caller supplies storage; failure never writes it.
 * Degenerate strip triangles are retained to preserve strip parity.
 * A zero result means invalid input or insufficient capacity. */
uint32_t RT_Triangulate(rt_primitive_t primitive, uint32_t vertex_count,
    uint32_t* indices, uint32_t capacity);
/* Column-major affine host model matrix -> row-major 3x4 RTGL1 matrix.
 * Coordinates retain Quake units and axes; the camera uses the same space. */
int RT_AffineTransform(const float matrix[16], RgTransform* result);
/* Interpolates position and normalized normals; takes UV/layers/color from a.
 * Lerp is clamped to [0,1]. The entire input is validated before writing. */
int RT_InterpolatePose(const RgVertex* a, const RgVertex* b, uint32_t count,
    float lerp, RgVertex* output);
/* Top-left logical HUD coordinates, Vulkan depth range [0,1]. */
int RT_HUDProjection(float width, float height, float matrix[16]);
/* Collision-free ID packing. The host must supply real entity lifetime
 * generations, not pointer/visibility-list indices; zero signals bad input.
 * Epoch/generation wrap requires a new scene and temporal-history reset. */
uint64_t RT_GeometryID(uint32_t domain, uint32_t epoch, uint32_t slot, uint32_t generation);
/* These validate CPU geometry before forwarding to the real or instrumented ABI.
 * Static begin/submit and frame ownership remain with the caller. */
int RT_SubmitGeometry(rt_api_t* api, const RgGeometryUploadInfo* geometry);
int RT_SubmitOverlay(rt_api_t* api, const RgVertex* vertices, uint32_t vertex_count,
    const uint32_t* indices, uint32_t index_count, RgMaterial material,
    rt_blend_t blend, int hud, int depth_test, int depth_write,
    const float* view_projection);
#endif
