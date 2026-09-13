/* RT-GEOMETRY-001, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#include "rt_geometry.h"
#include <math.h>
#include <string.h>
#include <limits.h>

uint32_t RT_Triangulate(rt_primitive_t primitive, uint32_t n, uint32_t* out, uint32_t capacity)
{
    uint32_t count, i;
    if (!out || n < 3) return 0;
    if (primitive == RT_TRIANGLES) {
        if (n % 3) return 0;
        count = n;
    } else if (primitive == RT_TRIANGLE_FAN || primitive == RT_TRIANGLE_STRIP) {
        if (n - 2 > UINT32_MAX / 3) return 0;
        count = (n - 2) * 3;
    } else return 0;
    if (capacity < count) return 0;
    if (primitive == RT_TRIANGLES) {
        for (i = 0; i < n; ++i) out[i] = i;
    } else for (i = 0; i < n - 2; ++i) {
        out[3*i] = primitive == RT_TRIANGLE_FAN ? 0 : i + (i & 1);
        out[3*i+1] = primitive == RT_TRIANGLE_FAN ? i + 1 : i + 1 - (i & 1);
        out[3*i+2] = i + 2;
    }
    return count;
}

int RT_AffineTransform(const float m[16], RgTransform* result)
{
    unsigned r, c;
    if (!m || !result) return 0;
    for (r = 0; r < 16; ++r) if (!isfinite(m[r])) return 0;
    if (m[3] != 0 || m[7] != 0 || m[11] != 0 || m[15] != 1) return 0;
    for (r = 0; r < 3; ++r) for (c = 0; c < 4; ++c) result->matrix[r][c] = m[c*4+r];
    return 1;
}

static int valid_vertex(const RgVertex* v)
{
    unsigned i;
    for (i = 0; i < 3; ++i) if (!isfinite(v->position[i]) || !isfinite(v->normal[i])) return 0;
    for (i = 0; i < 2; ++i) if (!isfinite(v->texCoord[i]) ||
        !isfinite(v->texCoordLayer1[i]) || !isfinite(v->texCoordLayer2[i])) return 0;
    return 1;
}

int RT_InterpolatePose(const RgVertex* a, const RgVertex* b, uint32_t count, float lerp, RgVertex* out)
{
    uint32_t i;
    unsigned j;
    double normal[3], len;
    if (!a || !b || !out || !count || !isfinite(lerp)) return 0;
    for (i = 0; i < count; ++i) if (!valid_vertex(a+i) || !valid_vertex(b+i)) return 0;
    if (lerp < 0) lerp = 0;
    if (lerp > 1) lerp = 1;
    for (i = 0; i < count; ++i) {
        RgVertex v = a[i];
        for (j = 0; j < 3; ++j) {
            v.position[j] = (float)((1.0-lerp)*a[i].position[j] + lerp*(double)b[i].position[j]);
            normal[j] = (1.0-lerp)*a[i].normal[j] + lerp*(double)b[i].normal[j];
        }
        len = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
        /* Opposing normals at the midpoint have no interpolated direction. */
        if (len < 1e-12) {
            for (j = 0; j < 3; ++j) normal[j] = a[i].normal[j];
            len = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
        }
        for (j = 0; j < 3; ++j) v.normal[j] = len < 1e-12 ? (j == 2 ? 1.0f : 0.0f) : (float)(normal[j]/len);
        out[i] = v;
    }
    return 1;
}

int RT_HUDProjection(float width, float height, float m[16])
{
    if (!m || !isfinite(width) || !isfinite(height) || width < 1 || height < 1) return 0;
    memset(m, 0, 16*sizeof(*m));
    m[0] = 2/width; m[5] = 2/height; m[10] = 1;
    m[12] = -1; m[13] = -1; m[15] = 1;
    return 1;
}

uint64_t RT_GeometryID(uint32_t domain, uint32_t epoch, uint32_t slot, uint32_t generation)
{
    if (!domain || domain > 255 || !epoch || epoch > 65535 || slot > 0xffffff || generation > 65535) return 0;
    return ((uint64_t)domain << 56) | ((uint64_t)epoch << 40) | ((uint64_t)slot << 16) | generation;
}

static int valid_mesh(const RgVertex* vertices, uint32_t n, const uint32_t* indices, uint32_t count)
{
    uint32_t i;
    if (!vertices || n < 3 || (count && !indices) || (!count && indices)) return 0;
    if ((count ? count : n) % 3) return 0;
    for (i = 0; i < n; ++i) if (!valid_vertex(vertices+i)) return 0;
    for (i = 0; i < count; ++i) if (indices[i] >= n) return 0;
    return 1;
}

int RT_SubmitGeometry(rt_api_t* api, const RgGeometryUploadInfo* g)
{
    if (!api || !api->rgUploadGeometry || !g || !g->uniqueID ||
        !valid_mesh(g->pVertices, g->vertexCount, g->pIndices, g->indexCount)) return 0;
    return RT_Check(api, api->rgUploadGeometry(api->instance, g), "geometry upload");
}

int RT_SubmitOverlay(rt_api_t* api, const RgVertex* vertices, uint32_t n,
    const uint32_t* indices, uint32_t count, RgMaterial material,
    rt_blend_t blend, int hud, int depth_test, int depth_write, const float* projection)
{
    RgRasterizedGeometryUploadInfo g = {0};
    unsigned i;
    if (!api || !api->rgUploadRasterizedGeometry || !valid_mesh(vertices,n,indices,count) ||
        (hud && (depth_test || depth_write || !projection)) ||
        blend < RT_BLEND_REPLACE || blend > RT_BLEND_MULTIPLY) return 0;
    if (projection) for (i = 0; i < 16; ++i) if (!isfinite(projection[i])) return 0;
    g.renderType = hud ? RG_RASTERIZED_GEOMETRY_RENDER_TYPE_SWAPCHAIN : RG_RASTERIZED_GEOMETRY_RENDER_TYPE_DEFAULT;
    g.vertexCount=n; g.pVertices=vertices; g.indexCount=count; g.pIndices=indices;
    g.transform.matrix[0][0]=g.transform.matrix[1][1]=g.transform.matrix[2][2]=1;
    g.color=(RgFloat4D){{1,1,1,1}}; g.material=material;
    g.blendFuncSrc=RG_BLEND_FACTOR_ONE; g.blendFuncDst=RG_BLEND_FACTOR_ZERO;
    if (depth_test) g.pipelineState |= RG_RASTERIZED_GEOMETRY_STATE_DEPTH_TEST;
    if (depth_write) g.pipelineState |= RG_RASTERIZED_GEOMETRY_STATE_DEPTH_WRITE;
    if (blend != RT_BLEND_REPLACE) g.pipelineState |= RG_RASTERIZED_GEOMETRY_STATE_BLEND_ENABLE;
    switch (blend) {
    case RT_BLEND_ALPHA: g.blendFuncSrc=RG_BLEND_FACTOR_SRC_ALPHA; g.blendFuncDst=RG_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; break;
    case RT_BLEND_PREMULTIPLIED: g.blendFuncDst=RG_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; break;
    case RT_BLEND_ADD: g.blendFuncDst=RG_BLEND_FACTOR_ONE; break;
    case RT_BLEND_MULTIPLY: g.blendFuncSrc=RG_BLEND_FACTOR_DST_COLOR; break;
    default: break;
    }
    return RT_Check(api,api->rgUploadRasterizedGeometry(api->instance,&g,projection,NULL),"overlay upload");
}
