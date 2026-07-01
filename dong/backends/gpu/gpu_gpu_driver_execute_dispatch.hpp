#pragma once

#include "dong_gpu_driver.h"
#include "../../src/render/gpu_ir.hpp"

#include <vector>

#ifdef DONG_HAS_IN_DREAMING_GPU
#include "gpu/gpu.h"
#include "gpu/rendergraph/gpu_render_graph.h"
#endif

namespace dong::gpu_backend {

class GpuGPUDriverImpl;

#ifdef DONG_HAS_IN_DREAMING_GPU

struct GpuOffscreenState {
    DongGPUTexture target = nullptr;
    GpuTextureHandle target_view{};
    uint32_t width = 0;
    uint32_t height = 0;
    bool active = false;
};

struct GpuExecuteContext {
    GpuRenderPassEncoder pass = nullptr;
    GpuDevice device = nullptr;
    uint32_t viewport_w = 1;
    uint32_t viewport_h = 1;
    float transform[6] = {1, 0, 0, 0, 1, 0};
    bool in_pass = false;
    int skip_draw_depth = 0;

    struct ClipEntry {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
        bool has_rounded = false;
        dong::render::Rect rounded_rect{};
        float rounded_radius = 0.0f;
    };
    std::vector<ClipEntry> clip_stack;

    float layer_opacity = 1.0f;
    std::vector<float> layer_opacity_stack;

    void writeViewport(float out[4]) const;
    const float* currentTransform() const;
    void fillClipUniform(float clip_rects[4][4], float clip_radii[4], float clip_meta[4]) const;
};

class ExecuteDispatcher {
public:
    void dispatch(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommandList& list);
};

void drawTextMsdf(GpuGPUDriverImpl& driver, GpuExecuteContext& ctx, const dong::render::GPUCommand& cmd);

#endif

} // namespace dong::gpu_backend
