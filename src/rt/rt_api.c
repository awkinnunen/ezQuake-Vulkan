/* RT-00/01, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#include "rt_api.h"
#include "rt_build_identity.h"
#include <SDL3/SDL.h>
#include <jansson.h>
#include <bcrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void message(rt_api_t* api, const char* text) { if(api->log) api->log(text,api->log_user); }
static int fail(rt_api_t* api, const char* text) { message(api,text); return 0; }
static int path_join(char* dest, size_t size, const char* dir, const char* relative)
{
    int length;
    if(!relative || !*relative || relative[0]=='/' || relative[0]=='\\' || strstr(relative,"..") || strchr(relative,':')) return 0;
    length=snprintf(dest,size,"%s/%s",dir,relative);
    return length>0 && (size_t)length<size;
}
static int hash_file(const char* path, char hex[65])
{
    BCRYPT_ALG_HANDLE algorithm=NULL;
    BCRYPT_HASH_HANDLE hash=NULL;
    unsigned char data[65536], digest[32];
    FILE* file=NULL;
    size_t bytes;
    int result=0,i;
    if(BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,NULL,0)<0) goto done;
    if(BCryptCreateHash(algorithm,&hash,NULL,0,NULL,0,0)<0) goto done;
    if(fopen_s(&file,path,"rb") || !file) goto done;
    while((bytes=fread(data,1,sizeof(data),file))>0) if(BCryptHashData(hash,data,(ULONG)bytes,0)<0) goto done;
    if(ferror(file) || BCryptFinishHash(hash,digest,sizeof(digest),0)<0) goto done;
    for(i=0;i<32;i++) snprintf(hex+2*i,3,"%02x",digest[i]);
    result=1;
done:
    if(file) fclose(file);
    if(hash) BCryptDestroyHash(hash);
    if(algorithm) BCryptCloseAlgorithmProvider(algorithm,0);
    return result;
}
static int validate_package(rt_api_t* api)
{
    char path[1200], actual[65];
    const char* name;
    json_t *manifest=NULL,*files,*value;
    json_error_t error;
    unsigned shaders=0;
    int ok=0;
    path_join(path,sizeof(path),api->directory,"manifest.json");
    manifest=json_load_file(path,JSON_REJECT_DUPLICATES,&error);
    if(!manifest) return fail(api,"RT package manifest missing or invalid.");
    if(!json_is_object(manifest) || json_integer_value(json_object_get(manifest,"schema"))!=1) {
        fail(api,"Unsupported RT package manifest schema."); goto done;
    }
    value=json_object_get(manifest,"headerSHA256");
    if(!json_is_string(value) || strcmp(json_string_value(value),RT_HEADER_SHA256)) {
        fail(api,"RT package header/ABI mismatch."); goto done;
    }
    if(json_integer_value(json_object_get(manifest,"extensionVersion"))!=RT_EXTENSION_VERSION) {
        fail(api,"RT package extension version mismatch."); goto done;
    }
    files=json_object_get(manifest,"files");
    if(!json_is_object(files) || json_object_size(files)>256 || !json_object_get(files,"RayTracedGL1.dll") || !json_object_get(files,"BlueNoise_LDR_RGBA_128.ktx2") || !json_object_get(files,"RayTracedGL1.txt")) {
        fail(api,"RT package manifest lacks required runtime files."); goto done;
    }
#define REQUIRE_SHADER(name) if(!json_object_get(files,"shaders/" name)) { fail(api,"RT manifest lacks required shader: " name); goto done; }
    RT_REQUIRED_SHADERS(REQUIRE_SHADER)
#undef REQUIRE_SHADER
    json_object_foreach(files,name,value) {
        if(!json_is_string(value) || !path_join(path,sizeof(path),api->directory,name) ||
           !hash_file(path,actual) || strcmp(actual,json_string_value(value))) {
            message(api,name); fail(api,"RT runtime file missing or SHA-256 mismatch."); goto done;
        }
        if(!strncmp(name,"shaders/",8)) ++shaders;
    }
    if(shaders!=RT_SHADER_COUNT) { fail(api,"RT shader set does not match the build."); goto done; }
    ok=1;
done:
    json_decref(manifest); return ok;
}
int RT_LoadAPI(rt_api_t* api, const char* directory, rt_log_fn log, void* user)
{
    char path[1200];
    DWORD length;
    memset(api,0,sizeof(*api)); api->log=log; api->log_user=user;
    length=GetFullPathNameA(directory,sizeof(api->directory),api->directory,NULL);
    if(!length || length>=sizeof(api->directory)) return fail(api,"RT runtime directory is invalid or too long.");
    if(!validate_package(api)) return 0;
    path_join(path,sizeof(path),api->directory,"RayTracedGL1.dll");
    api->library=LoadLibraryExA(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!api->library) {
        char text[256];
        snprintf(text,sizeof(text),"RT DLL load failed (Windows error %lu). Check the x64 Visual C++ runtime and DLL dependencies.",GetLastError());
        return fail(api,text);
    }
#define RT_EXPORT(result,name,args) api->name=(result (RGCONV*) args)GetProcAddress(api->library,#name); if(!api->name) { message(api,"Missing RT export: " #name); goto failed; }
#include "rt_exports.h"
#undef RT_EXPORT
    api->get_extension_version=(uint32_t (RGCONV*)(void))GetProcAddress(api->library,"rgEzqGetApiVersion");
    api->select_device=(RgBool32 (RGCONV*)(const uint8_t*))GetProcAddress(api->library,"rgEzqSelectDevice");
    if(!api->get_extension_version || !api->select_device || api->get_extension_version()!=RT_EXTENSION_VERSION) {
        message(api,"RT DLL lacks the matching ezQuake extension ABI."); goto failed;
    }
    message(api,"RT package hashes and 26 required exports verified."); return 1;
failed:
    RT_UnloadAPI(api); return 0;
}
int RT_Check(rt_api_t* api, RgResult result, const char* operation)
{
    char text[512];
    api->last_result=result;
    if(result==RG_SUCCESS) return 1;
    snprintf(text,sizeof(text),"RT %s failed: %s",operation,api->rgGetResultDescription(result));
    return fail(api,text);
}
static void print_library(const char* text, void* user) { message((rt_api_t*)user,text); }
int RT_CreateInstance(rt_api_t* api, void* window, const rt_device_t* device)
{
    SDL_PropertiesID properties=SDL_GetWindowProperties((SDL_Window*)window);
    RgWin32SurfaceCreateInfo surface={0};
    RgInstanceCreateInfo create={0};
    char shaders[1200],noise[1200],water[1200],config[1200],materials[1200];
    surface.hwnd=SDL_GetPointerProperty(properties,SDL_PROP_WINDOW_WIN32_HWND_POINTER,NULL);
    surface.hinstance=GetModuleHandle(NULL);
    if(!surface.hwnd || !api->select_device(device->uuid)) return fail(api,"RT window/device selection failed.");
    path_join(shaders,sizeof(shaders),api->directory,"shaders/");
    path_join(noise,sizeof(noise),api->directory,"BlueNoise_LDR_RGBA_128.ktx2");
    path_join(water,sizeof(water),api->directory,"WaterNormal_n.ktx2");
    path_join(config,sizeof(config),api->directory,"RayTracedGL1.txt");
    path_join(materials,sizeof(materials),api->directory,"materials/");
    create.pAppName="ezQuake Vulkan RT"; create.pAppGUID="bde35278-b7e6-48f7-b408-09f822da8369";
    create.pWin32SurfaceInfo=&surface; create.pConfigPath=config;
    create.pfnPrint=print_library; create.pUserPrintData=api;
    create.pShaderFolderPath=shaders; create.pBlueNoiseFilePath=noise;
    create.pWaterNormalTexturePath=water; create.pOverridenTexturesFolderPath=materials;
    create.primaryRaysMaxAlbedoLayers=2; create.indirectIlluminationMaxAlbedoLayers=1;
    create.rayCullBackFacingTriangles=RG_FALSE; create.allowGeometryWithSkyFlag=RG_TRUE;
    create.rasterizedMaxVertexCount=1<<19; create.rasterizedMaxIndexCount=1<<20;
    create.rasterizedVertexColorGamma=RG_TRUE; create.rasterizedSkyCubemapSize=256;
    create.maxTextureCount=4096; create.textureSamplerForceMinificationFilterLinear=RG_TRUE;
    create.textureSamplerForceNormalMapFilterLinear=RG_TRUE;
    create.originalAlbedoAlphaTextureIsSRGB=RG_TRUE; create.overridenAlbedoAlphaTextureIsSRGB=RG_TRUE;
    return RT_Check(api,api->rgCreateInstance(&create,&api->instance),"create instance");
}
void RT_DestroyInstance(rt_api_t* api)
{
    if(api->instance) { RT_Check(api,api->rgDestroyInstance(api->instance),"destroy instance"); api->instance=RG_NULL_HANDLE; }
}
void RT_UnloadAPI(rt_api_t* api)
{
    RT_DestroyInstance(api);
    if(api->library) FreeLibrary(api->library);
    memset(api,0,sizeof(*api));
}
