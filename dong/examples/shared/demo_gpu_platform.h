#ifndef DEMO_GPU_PLATFORM_H
#define DEMO_GPU_PLATFORM_H

#include <stdint.h>
#include "dong.h"
#include "dong_gpu_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DongDemoGpu {
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
    int32_t mouse_x;
    int32_t mouse_y;
} DongDemoGpu;

typedef struct DongDemoGpuConfig {
    const char* title;
    uint32_t width;
    uint32_t height;
} DongDemoGpuConfig;

/* Create window/device/surface, register DongGPUDriver (embedded_mode=0). */
int dong_demo_gpu_init(DongDemoGpu* ctx, const DongDemoGpuConfig* cfg);

void dong_demo_gpu_shutdown(DongDemoGpu* ctx);

/* Poll platform events; forwards resize to engine. Returns 0 when quit requested. */
int dong_demo_gpu_poll_events(DongDemoGpu* ctx, dong_engine_t* engine);

/* One frame: dong_engine_tick (renders via registered GPU driver). */
void dong_demo_gpu_present(DongDemoGpu* ctx, dong_engine_t* engine);

dong_engine_t* dong_demo_gpu_create_engine(DongDemoGpu* ctx);

void dong_demo_gpu_destroy_engine(dong_engine_t* engine);

/* Load HTML from disk; sets engine resource root to the file directory. */
int dong_demo_gpu_load_html_file(dong_engine_t* engine, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_GPU_PLATFORM_H */
