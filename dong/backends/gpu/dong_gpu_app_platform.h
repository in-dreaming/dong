#ifndef DONG_GPU_APP_PLATFORM_H
#define DONG_GPU_APP_PLATFORM_H

#include <stdint.h>

#include "dong_app.h"
#include "dong_gpu_driver.h"
#include "dong.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DongGpuAppPlatform {
    void* window;
    void* device;
    void* surface;
    void* queue;
    uint32_t surface_fmt;
    uint32_t width;
    uint32_t height;
    DongGPUDriver* driver;
    void* image_decoder;
    int quit;
    int platform_ready;
    int text_input_enabled;
    int right_mouse_down;
} DongGpuAppPlatform;

typedef struct DongGpuAppPlatformConfig {
    const char* title;
    uint32_t width;
    uint32_t height;
    int vsync;
    int resizable;
    int fullscreen;
} DongGpuAppPlatformConfig;

int dong_gpu_app_platform_init(DongGpuAppPlatform* ctx, const DongGpuAppPlatformConfig* cfg);
void dong_gpu_app_platform_shutdown(DongGpuAppPlatform* ctx);

int dong_gpu_app_platform_poll_event(DongGpuAppPlatform* ctx, dong_app_event_t* out_event,
                                     dong_engine_t* engine);

void dong_gpu_app_platform_present_clear(DongGpuAppPlatform* ctx);

void dong_gpu_app_platform_set_text_input(DongGpuAppPlatform* ctx, int enable);

#ifdef __cplusplus
}
#endif

#endif
