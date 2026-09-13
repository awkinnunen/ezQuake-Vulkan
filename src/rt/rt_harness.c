/* RT-01, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#define SDL_MAIN_HANDLED
#include "rt_geometry.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void log_line(const char* text, void* unused) { (void)unused; puts(text); fflush(stdout); }
static int triangle(rt_api_t* api)
{
    RgVertex vertices[3]={0};
    RgGeometryUploadInfo geometry={0};
    unsigned i;
    vertices[0].position[0]=-1; vertices[0].position[1]=-1; vertices[0].position[2]=-3;
    vertices[1].position[0]=1; vertices[1].position[1]=-1; vertices[1].position[2]=-3;
    vertices[2].position[1]=1; vertices[2].position[2]=-3;
    for(i=0;i<3;i++) { vertices[i].normal[2]=1; vertices[i].packedColor=0xffffffffu; }
    geometry.uniqueID=RT_GeometryID(1,1,0,0); geometry.geomType=RG_GEOMETRY_TYPE_STATIC;
    geometry.vertexCount=3; geometry.pVertices=vertices;
    geometry.defaultRoughness=.8f;
    geometry.layerColors[0]=(RgFloat4D){{.8f,.3f,.1f,1}};
    geometry.transform=(RgTransform){{{1,0,0,0},{0,1,0,0},{0,0,1,0}}};
    return RT_Check(api,api->rgBeginStaticGeometries(api->instance),"begin scene") &&
        RT_SubmitGeometry(api,&geometry) &&
        RT_Check(api,api->rgSubmitStaticGeometries(api->instance),"submit scene");
}
int main(int argc,char** argv)
{
    rt_device_t device;
    rt_api_t api={0};
    SDL_Window* window=NULL;
    unsigned frame=0, maxframes=600;
    int supported, result=1, quit=0, reject_device=0;
    const char* directory=argc>1?argv[1]:"rt-runtime";
    setvbuf(stdout,NULL,_IONBF,0);
    SDL_SetMainReady();
    puts("ezQuake RT-01 harness: package/API and device preflight, then triangle/light. Not an engine integration test.");
    supported=RT_ProbeDevice(&device,log_line,NULL);
    if(!strcmp(directory,"--probe")) return supported?0:2;
    if(!RT_LoadAPI(&api,directory,log_line,NULL)) return 3;
    if(argc>2 && !strcmp(argv[2],"--check-package")) { RT_UnloadAPI(&api); return 0; }
    reject_device=argc>2 && !strcmp(argv[2],"--reject-device");
    if(!supported && !reject_device) { RT_UnloadAPI(&api); return 2; }
    if(argc>2 && !reject_device) maxframes=(unsigned)strtoul(argv[2],NULL,10);
    if(!maxframes || maxframes>36000) { puts("Frame count must be 1..36000"); goto done; }
    if(!SDL_Init(SDL_INIT_VIDEO)) { puts(SDL_GetError()); goto done; }
    window=SDL_CreateWindow("ezQuake RT triangle - Escape to exit",960,540,SDL_WINDOW_VULKAN|SDL_WINDOW_RESIZABLE);
    if(!window) { puts(SDL_GetError()); goto done; }
    if(reject_device) {
        unsigned attempt;
        /* A deliberately different UUID must not silently choose another GPU. */
        device.uuid[0]^=255;
        for(attempt=0;attempt<16;attempt++) {
            if(RT_CreateInstance(&api,window,&device) || api.last_result!=RG_CANT_FIND_PHYSICAL_DEVICE) goto done;
        }
        puts("PASS: missing selected UUID rejected 16 times, no alternate GPU used."); result=0;
        goto done;
    }
    if(!RT_CreateInstance(&api,window,&device) || !triangle(&api)) goto done;
    while(!quit && frame<maxframes) {
        SDL_Event event;
        RgStartFrameInfo start={0};
        RgDrawFrameInfo draw={0};
        RgSphericalLightUploadInfo light={0};
        RgDrawFrameSkyParams sky={0};
        while(SDL_PollEvent(&event)) {
            if(event.type==SDL_EVENT_QUIT || (event.type==SDL_EVENT_KEY_DOWN && event.key.key==SDLK_ESCAPE)) quit=1;
        }
        if(quit) break;
        if(SDL_GetWindowFlags(window)&SDL_WINDOW_MINIMIZED) { SDL_Delay(10); continue; }
        start.requestVSync=RG_TRUE;
        if(!RT_Check(&api,api.rgStartFrame(api.instance,&start),"start frame")) goto done;
        light.uniqueID=2; light.position=(RgFloat3D){{0,1,-1}}; light.color=(RgFloat3D){{20,20,20}}; light.radius=.1f;
        if(!RT_Check(&api,api.rgUploadSphericalLight(api.instance,&light),"light")) goto done;
        draw.view[0]=draw.view[5]=draw.view[10]=draw.view[15]=1;
        draw.worldUpVector=(RgFloat3D){{0,1,0}};
        draw.fovYRadians=1.04719755f; draw.cameraNear=.05f; draw.cameraFar=100; draw.rayLength=100;
        draw.rayCullMaskWorld=RG_DRAW_FRAME_RAY_CULL_WORLD_0_BIT;
        draw.currentTime=(double)SDL_GetTicksNS()/1000000000.0;
        sky.skyColorDefault=(RgFloat3D){{.1f,.15f,.2f}}; sky.skyColorMultiplier=1; sky.skyColorSaturation=1;
        draw.pSkyParams=&sky;
        if(!RT_Check(&api,api.rgDrawFrame(api.instance,&draw),"draw frame")) goto done;
        ++frame;
    }
    printf("SUBMITTED %u RT frames on %s. Visual correctness requires inspection.\n",frame,device.name);
    result=frame?0:1;
done:
    RT_UnloadAPI(&api);
    if(window) SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
