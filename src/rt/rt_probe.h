/* RT-00, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#ifndef EZQ_RT_PROBE_H
#define EZQ_RT_PROBE_H
#include <stdint.h>
typedef void (*rt_log_fn)(const char*, void*);
typedef struct rt_device_s {
    uint8_t uuid[16];
    char name[256];
    uint32_t vendor, device, driver, api;
    uint64_t local_bytes;
} rt_device_t;
/* Returns 1 only when a device meets ALL pinned CreateDevice requirements. */
int RT_ProbeDevice(rt_device_t* selected, rt_log_fn log, void* user);
#endif
