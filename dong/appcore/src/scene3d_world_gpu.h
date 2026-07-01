#ifndef SCENE3D_WORLD_GPU_H
#define SCENE3D_WORLD_GPU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Scene3DWorldGpu Scene3DWorldGpu;

typedef struct Scene3DWorldScreenDraw {
    void* texture;
    float mvp[16];
    float model[16];
    int highlighted;
} Scene3DWorldScreenDraw;

typedef struct Scene3DWorldFrameDesc {
    void* driver;
    void* device;
    void* queue;
    void* surface;
    uint32_t width;
    uint32_t height;
    float bg_r;
    float bg_g;
    float bg_b;
    float bg_a;
    int depth_test;
    int screen_count;
    const Scene3DWorldScreenDraw* screens;
} Scene3DWorldFrameDesc;

Scene3DWorldGpu* scene3d_world_gpu_create(void* device, uint32_t color_format, const char* shader_dir);
void scene3d_world_gpu_destroy(Scene3DWorldGpu* world);
int scene3d_world_gpu_render(Scene3DWorldGpu* world, const Scene3DWorldFrameDesc* frame);

#ifdef __cplusplus
}
#endif

#endif
