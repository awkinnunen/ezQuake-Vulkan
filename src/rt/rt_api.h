/* RT-00/01, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#ifndef EZQ_RT_API_H
#define EZQ_RT_API_H
#define RG_USE_SURFACE_WIN32
#include "RTGL1.h"
#include "rt_probe.h"
#define RT_EXTENSION_VERSION 1u
typedef struct rt_api_s {
    HMODULE library;
    RgInstance instance;
    RgResult last_result;
    char directory[1024];
    rt_log_fn log;
    void* log_user;
#define RT_EXPORT(result,name,args) result (RGCONV *name) args;
#include "rt_exports.h"
#undef RT_EXPORT
    uint32_t (RGCONV *get_extension_version)(void);
    RgBool32 (RGCONV *select_device)(const uint8_t*);
} rt_api_t;
int RT_LoadAPI(rt_api_t* api, const char* directory, rt_log_fn log, void* user);
void RT_UnloadAPI(rt_api_t* api);
int RT_CreateInstance(rt_api_t* api, void* sdl_window, const rt_device_t* device);
void RT_DestroyInstance(rt_api_t* api);
int RT_Check(rt_api_t* api, RgResult result, const char* operation);
#endif
