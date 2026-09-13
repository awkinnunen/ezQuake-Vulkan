/* RT-00, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#include "rt_probe.h"
#include "rt_requirements.h"
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void report(rt_log_fn log, void* user, const char* prefix, const char* value)
{
    char line[512];
    snprintf(line,sizeof(line),"%s%s",prefix,value);
    if (log) log(line,user);
}

int RT_ProbeDevice(rt_device_t* selected, rt_log_fn log, void* user)
{
    VkApplicationInfo app = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    VkInstanceCreateInfo create = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice* devices = NULL;
    uint32_t count = 0, i;
    int found = 0;
    memset(selected,0,sizeof(*selected));
    app.pApplicationName = "ezQuake RT preflight";
    app.apiVersion = VK_API_VERSION_1_2;
    create.pApplicationInfo = &app;
    if (vkCreateInstance(&create,NULL,&instance)!=VK_SUCCESS) {
        report(log,user,"ERROR: ","Vulkan 1.2 instance unavailable"); return 0;
    }
    if (vkEnumeratePhysicalDevices(instance,&count,NULL)!=VK_SUCCESS || !count) goto done;
    devices = calloc(count,sizeof(*devices));
    if (!devices || vkEnumeratePhysicalDevices(instance,&count,devices)!=VK_SUCCESS) goto done;
    for (i=0;i<count;i++) {
        VkPhysicalDeviceIDProperties identity = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};
        VkPhysicalDeviceProperties2 props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        VkPhysicalDeviceFeatures2 features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        VkPhysicalDeviceVulkan12Features v12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        VkPhysicalDeviceMultiviewFeatures multiview = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES};
        VkPhysicalDevice16BitStorageFeatures storage16 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES};
        VkPhysicalDeviceSynchronization2FeaturesKHR sync2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR pipeline = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
        VkPhysicalDeviceMemoryProperties memory;
        VkExtensionProperties* extensions;
        uint32_t extensionCount=0,j;
        unsigned missing=0;
        uint64_t localBytes=0;
        char info[400];
        props.pNext=&identity;
        vkGetPhysicalDeviceProperties2(devices[i],&props);
        features.pNext=&v12; v12.pNext=&multiview; multiview.pNext=&storage16;
        storage16.pNext=&sync2; sync2.pNext=&pipeline; pipeline.pNext=&acceleration;
        vkGetPhysicalDeviceFeatures2(devices[i],&features);
        vkGetPhysicalDeviceMemoryProperties(devices[i],&memory);
        for(j=0;j<memory.memoryHeapCount;j++) if(memory.memoryHeaps[j].flags&VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) localBytes+=memory.memoryHeaps[j].size;
        snprintf(info,sizeof(info),"GPU[%u] %s, Vulkan %u.%u.%u, driver %u, device memory %llu MiB",i,props.properties.deviceName,
            VK_VERSION_MAJOR(props.properties.apiVersion),VK_VERSION_MINOR(props.properties.apiVersion),VK_VERSION_PATCH(props.properties.apiVersion),
            props.properties.driverVersion,(unsigned long long)(localBytes/(1024*1024)));
        report(log,user,"",info);
        if(props.properties.apiVersion<VK_API_VERSION_1_2) { ++missing; report(log,user,"Missing: ","Vulkan 1.2"); }
#define CHECK_BASE(field) if(!features.features.field) { ++missing; report(log,user,"Missing feature: ",#field); }
        RT_REQUIRED_BASE(CHECK_BASE)
#undef CHECK_BASE
#define CHECK12(field) if(!v12.field) { ++missing; report(log,user,"Missing Vulkan12 feature: ",#field); }
        RT_REQUIRED_V12(CHECK12)
#undef CHECK12
#define CHECK_MULTI(field) if(!multiview.field) { ++missing; report(log,user,"Missing feature: ",#field); }
        RT_REQUIRED_MULTIVIEW(CHECK_MULTI)
#undef CHECK_MULTI
#define CHECK16(field) if(!storage16.field) { ++missing; report(log,user,"Missing feature: ",#field); }
        RT_REQUIRED_STORAGE16(CHECK16)
#undef CHECK16
#define CHECK_SYNC(field) if(!sync2.field) { ++missing; report(log,user,"Missing feature: ",#field); }
        RT_REQUIRED_SYNC2(CHECK_SYNC)
#undef CHECK_SYNC
#define CHECK_RT(field) if(!pipeline.field) { ++missing; report(log,user,"Missing feature: ",#field); }
        RT_REQUIRED_PIPELINE(CHECK_RT)
#undef CHECK_RT
#define CHECK_AS(field) if(!acceleration.field) { ++missing; report(log,user,"Missing feature: ",#field); }
        RT_REQUIRED_ACCELERATION(CHECK_AS)
#undef CHECK_AS
        if(vkEnumerateDeviceExtensionProperties(devices[i],NULL,&extensionCount,NULL)!=VK_SUCCESS) continue;
        extensions=calloc(extensionCount?extensionCount:1,sizeof(*extensions));
        if(!extensions) continue;
        if(vkEnumerateDeviceExtensionProperties(devices[i],NULL,&extensionCount,extensions)!=VK_SUCCESS) { free(extensions); continue; }
#define CHECK_EXT(name) { int exists=0; for(j=0;j<extensionCount;j++) if(!strcmp(extensions[j].extensionName,name)) exists=1; if(!exists) { ++missing; report(log,user,"Missing extension: ",name); } }
        RT_REQUIRED_EXTENSIONS(CHECK_EXT)
#undef CHECK_EXT
        free(extensions);
        if(!missing && !found) {
            found=1; memcpy(selected->uuid,identity.deviceUUID,16);
            snprintf(selected->name,sizeof(selected->name),"%s",props.properties.deviceName);
            selected->vendor=props.properties.vendorID; selected->device=props.properties.deviceID;
            selected->driver=props.properties.driverVersion; selected->api=props.properties.apiVersion; selected->local_bytes=localBytes;
            report(log,user,"Selected compatible device: ",selected->name);
        }
    }
done:
    free(devices); vkDestroyInstance(instance,NULL);
    if(!found) report(log,user,"RT unavailable: ","no device meets the pinned RTGL1 requirements; use Vulkan raster rendering.");
    return found;
}
