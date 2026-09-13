/* Instrumented ABI fixtures; OpenAI Codex, 2026-09-13. GPL-2.0-or-later.
 * These check conversion and submission only, never ray-traced pixels. */
#include "rt_geometry.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); return 1; } } while (0)
static unsigned geometry_calls, overlay_calls;
static RgGeometryUploadInfo captured_geometry;
static RgRasterizedGeometryUploadInfo captured_overlay;
static RgResult upload_result=RG_SUCCESS;
static const char* RGCONV result_description(RgResult result)
{ (void)result; return "instrumented upload failure"; }
static RgResult RGCONV geometry_upload(RgInstance instance, const RgGeometryUploadInfo* g)
{ (void)instance; ++geometry_calls; captured_geometry=*g; return upload_result; }
static RgResult RGCONV overlay_upload(RgInstance instance, const RgRasterizedGeometryUploadInfo* g,
    const float* projection, const RgViewport* viewport)
{ (void)instance; (void)projection; (void)viewport; ++overlay_calls; captured_overlay=*g; return upload_result; }
static void quiet_log(const char* text, void* user) { (void)text; (void)user; }
static float area(const RgVertex* v, uint32_t a, uint32_t b, uint32_t c)
{
    return (v[b].position[0]-v[a].position[0])*(v[c].position[1]-v[a].position[1])-
        (v[b].position[1]-v[a].position[1])*(v[c].position[0]-v[a].position[0]);
}
int main(void)
{
    uint32_t indices[12]={0}, saved[12], i;
    RgVertex vertices[4]={0}, pose[4], output[4];
    float matrix[16]={0,1,0,0,-1,0,0,0,0,0,1,0,10,20,30,1};
    float projection[16], sum;
    RgTransform transform;
    RgGeometryUploadInfo geometry={0};
    rt_api_t api={0};
    setvbuf(stdout,NULL,_IONBF,0);
    api.rgUploadGeometry=geometry_upload; api.rgUploadRasterizedGeometry=overlay_upload; api.log=quiet_log;
    api.rgGetResultDescription=result_description;
    vertices[1].position[0]=2; vertices[2].position[0]=2;
    vertices[2].position[1]=2; vertices[3].position[1]=2;
    for(i=0;i<4;i++) { vertices[i].normal[2]=1; vertices[i].packedColor=0x80402010; vertices[i].texCoord[0]=(float)i; }
    CHECK(RT_Triangulate(RT_TRIANGLE_FAN,4,indices,12)==6);
    sum=0;
    for(i=0;i<6;i+=3) { CHECK(area(vertices,indices[i],indices[i+1],indices[i+2])>0); sum+=area(vertices,indices[i],indices[i+1],indices[i+2])/2; }
    CHECK(sum==4); /* The convex square is covered exactly with preserved winding. */
    memcpy(saved,indices,sizeof(saved));
    CHECK(!RT_Triangulate(RT_TRIANGLE_FAN,4,indices,5));
    CHECK(!memcmp(saved,indices,sizeof(saved)));
    CHECK(!RT_Triangulate(RT_TRIANGLE_STRIP,UINT32_MAX,indices,12));
    CHECK(!RT_Triangulate(RT_TRIANGLES,4,indices,12));
    CHECK(!RT_Triangulate((rt_primitive_t)99,4,indices,12));
    CHECK(RT_Triangulate(RT_TRIANGLES,3,indices,12)==3 && indices[2]==2);
    { RgVertex temporary=vertices[2]; vertices[2]=vertices[3]; vertices[3]=temporary; }
    CHECK(RT_Triangulate(RT_TRIANGLE_STRIP,4,indices,12)==6);
    for(i=0;i<6;i+=3) CHECK(area(vertices,indices[i],indices[i+1],indices[i+2])>0);
    puts("PASS: fan/strip/list topology, area, winding, capacity and overflow");

    CHECK(RT_AffineTransform(matrix,&transform));
    CHECK(transform.matrix[0][0]==0 && transform.matrix[0][1]==-1 && transform.matrix[0][3]==10);
    CHECK(transform.matrix[1][0]==1 && transform.matrix[1][3]==20 && transform.matrix[2][3]==30);
    matrix[15]=0; CHECK(!RT_AffineTransform(matrix,&transform)); matrix[15]=1;
    matrix[0]=NAN; CHECK(!RT_AffineTransform(matrix,&transform));
    puts("PASS: Quake-axis affine rotation/translation and invalid matrices");

    memcpy(pose,vertices,sizeof(pose));
    for(i=0;i<4;i++) { pose[i].position[2]=8; pose[i].normal[2]=0; pose[i].normal[1]=1; }
    CHECK(RT_InterpolatePose(vertices,pose,4,.25f,output));
    CHECK(output[3].position[2]==2 && output[3].texCoord[0]==vertices[3].texCoord[0]);
    CHECK(output[3].packedColor==0x80402010);
    CHECK(fabsf(output[0].normal[1]*output[0].normal[1]+output[0].normal[2]*output[0].normal[2]-1)<1e-5f);
    CHECK(RT_InterpolatePose(vertices,pose,4,2,output) && output[0].position[2]==8);
    CHECK(RT_InterpolatePose(vertices,pose,4,-1,output) && output[0].position[2]==0);
    pose[0].normal[1]=0; pose[0].normal[2]=-1;
    CHECK(RT_InterpolatePose(vertices,pose,4,.5f,output) && output[0].normal[2]==1);
    memset(output,0x5a,sizeof(output));
    pose[3].position[0]=NAN;
    CHECK(!RT_InterpolatePose(vertices,pose,4,.5f,output));
    for(i=0;i<sizeof(output);i++) CHECK(((unsigned char*)output)[i]==0x5a);
    CHECK(!RT_InterpolatePose(vertices,pose,4,NAN,output));
    puts("PASS: interpolated poses, normalized/opposed normals, UV/color retention, atomic rejection");

    CHECK(RT_HUDProjection(960,540,projection));
    CHECK(projection[12]==-1 && projection[13]==-1);
    CHECK(fabsf(projection[0]*960+projection[12]-1)<1e-5f);
    CHECK(fabsf(projection[5]*540+projection[13]-1)<1e-5f);
    CHECK(!RT_HUDProjection(0,540,projection));
    CHECK(!RT_HUDProjection(NAN,540,projection));
    CHECK(RT_HUDProjection(960,540,projection));
    puts("PASS: HUD logical corners and invalid dimensions");

    CHECK(RT_GeometryID(255,65535,0xffffff,65535)==UINT64_MAX);
    CHECK(!RT_GeometryID(0,1,1,1) && !RT_GeometryID(1,0,1,1));
    CHECK(!RT_GeometryID(256,1,1,1) && !RT_GeometryID(1,65536,1,1));
    CHECK(!RT_GeometryID(1,1,0x1000000,1) && !RT_GeometryID(1,1,1,65536));
    CHECK(RT_GeometryID(2,1,3,4)!=RT_GeometryID(2,1,3,5));
    CHECK(RT_GeometryID(2,1,3,4)!=RT_GeometryID(2,2,3,4));
    puts("PASS: map/entity generation ID separation and wrap rejection");

    geometry.uniqueID=RT_GeometryID(1,1,0,0); geometry.vertexCount=4; geometry.pVertices=vertices;
    geometry.indexCount=6; geometry.pIndices=indices; geometry.transform=transform;
    geometry.geomType=RG_GEOMETRY_TYPE_STATIC;
    geometry.geomMaterial.layerMaterials[0]=123;
    CHECK(RT_SubmitGeometry(&api,&geometry));
    CHECK(geometry_calls==1 && captured_geometry.geomMaterial.layerMaterials[0]==123);
    CHECK(captured_geometry.uniqueID==geometry.uniqueID && captured_geometry.indexCount==6);
    geometry.geomType=RG_GEOMETRY_TYPE_DYNAMIC;
    geometry.visibilityType=RG_GEOMETRY_VISIBILITY_TYPE_FIRST_PERSON;
    CHECK(RT_SubmitGeometry(&api,&geometry));
    CHECK(captured_geometry.geomType==RG_GEOMETRY_TYPE_DYNAMIC && captured_geometry.visibilityType==RG_GEOMETRY_VISIBILITY_TYPE_FIRST_PERSON);
    indices[5]=4; CHECK(!RT_SubmitGeometry(&api,&geometry) && geometry_calls==2); indices[5]=3;
    vertices[0].position[2]=INFINITY; CHECK(!RT_SubmitGeometry(&api,&geometry) && geometry_calls==2); vertices[0].position[2]=0;
    upload_result=RG_GRAPHICS_API_ERROR;
    CHECK(!RT_SubmitGeometry(&api,&geometry) && api.last_result==RG_GRAPHICS_API_ERROR);
    upload_result=RG_SUCCESS;
    puts("PASS: static/dynamic/viewmodel ABI payload, material IDs, invalid indices and upload failure");

    for(i=RT_BLEND_REPLACE;i<=RT_BLEND_MULTIPLY;i++) {
        CHECK(RT_SubmitOverlay(&api,vertices,4,indices,6,123,(rt_blend_t)i,0,1,0,NULL));
        CHECK(captured_overlay.renderType==RG_RASTERIZED_GEOMETRY_RENDER_TYPE_DEFAULT);
        CHECK(captured_overlay.pipelineState & RG_RASTERIZED_GEOMETRY_STATE_DEPTH_TEST);
        CHECK(!(captured_overlay.pipelineState & RG_RASTERIZED_GEOMETRY_STATE_DEPTH_WRITE));
        CHECK(captured_overlay.material==123);
        CHECK(!!(captured_overlay.pipelineState & RG_RASTERIZED_GEOMETRY_STATE_BLEND_ENABLE)==(i!=RT_BLEND_REPLACE));
    }
    CHECK(captured_overlay.blendFuncSrc==RG_BLEND_FACTOR_DST_COLOR && captured_overlay.blendFuncDst==RG_BLEND_FACTOR_ZERO);
    CHECK(RT_SubmitOverlay(&api,vertices,4,indices,6,0,RT_BLEND_ALPHA,1,0,0,projection));
    CHECK(captured_overlay.renderType==RG_RASTERIZED_GEOMETRY_RENDER_TYPE_SWAPCHAIN);
    CHECK(captured_overlay.blendFuncSrc==RG_BLEND_FACTOR_SRC_ALPHA && captured_overlay.blendFuncDst==RG_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
    CHECK(!(captured_overlay.pipelineState & (RG_RASTERIZED_GEOMETRY_STATE_DEPTH_TEST|RG_RASTERIZED_GEOMETRY_STATE_DEPTH_WRITE)));
    i=overlay_calls;
    CHECK(!RT_SubmitOverlay(&api,vertices,4,indices,6,0,RT_BLEND_ALPHA,1,1,0,projection));
    CHECK(!RT_SubmitOverlay(&api,vertices,4,indices,6,0,RT_BLEND_ALPHA,1,0,0,NULL));
    CHECK(!RT_SubmitOverlay(&api,vertices,4,indices,6,0,(rt_blend_t)99,0,1,0,NULL));
    CHECK(overlay_calls==i);
    puts("PASS: particle blend/depth and output-resolution HUD submission guards");
    puts("All RT geometry CPU fixtures passed. Host scene callbacks and GPU pixels are not tested.");
    return 0;
}
