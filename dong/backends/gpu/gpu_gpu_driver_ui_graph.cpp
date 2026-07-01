#include "dong_gpu_driver.h"
#include "gpu_gpu_driver_ui_graph.hpp"
#include "gpu_gpu_driver.hpp"

#include "../../src/core/log.h"

#include "dong_ui_graph.h"

#ifdef DONG_HAS_IN_DREAMING_GPU
#include "gpu/gpu.h"
#include "gpu/rendergraph/gpu_render_graph.h"
#include "gpu/platform/gpu_surface.h"
#endif

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU
static void upload_pass_callback(GpuGraphPassContext* ctx, void* user_data) {
    auto* impl = static_cast<GpuGPUDriverImpl*>(user_data);
    if (impl) {
        impl->flushUploadPass(ctx);
    }
}

struct UiMainPassPayload {
    GpuGPUDriverImpl* impl = nullptr;
    const DongUiPassBundle* bundle = nullptr;
};

static void ui_main_pass_callback(GpuGraphPassContext* ctx, void* user_data) {
    const auto* payload = static_cast<const UiMainPassPayload*>(user_data);
    if (payload && payload->impl && payload->bundle) {
        payload->impl->executeUiMainPass(ctx, payload->bundle);
    }
}

void gpu_register_ui_passes_on_graph(GpuGraph graph,
                                     const DongUiPassBundle* bundle,
                                     GpuGraphResource color_target,
                                     GpuGPUDriverImpl* impl) {
    if (!graph || !bundle) {
        return;
    }

    // The swapchain backbuffer (and a freshly imported offscreen target) has undefined
    // contents at frame start. The FIRST render pass that writes the color target must
    // CLEAR it; subsequent passes LOAD to composite on top. Using LOAD on the first pass
    // reads garbage from the previous frame / present engine, causing flicker and a hazy
    // "overlay" look when semi-transparent UI is composited over the stale contents.
    bool color_target_initialized = false;
    auto bindColorTarget = [&](GpuGraphPass pass) {
        if (color_target == GPU_GRAPH_NULL_RESOURCE) {
            return;
        }
        GpuGraphColorAttachment ca{};
        ca.resource = color_target;
        ca.storeOp = GPU_STORE_OP_STORE;
        if (!color_target_initialized) {
            ca.loadOp = GPU_LOAD_OP_CLEAR;
            ca.clearColor[0] = 0.0f;
            ca.clearColor[1] = 0.0f;
            ca.clearColor[2] = 0.0f;
            // Swapchain surfaces need opaque alpha; clearing to 0 lets the compositor
            // treat the buffer as transparent and blend stale desktop pixels underneath.
            ca.clearColor[3] = 1.0f;
            color_target_initialized = true;
        } else {
            ca.loadOp = GPU_LOAD_OP_LOAD;
        }
        gpuGraphPassSetColorAttachments(pass, 1, &ca);
    };

    if (bundle->has_upload && impl) {
        GpuGraphPass upload = gpuGraphAddCopyPass(graph, "Dong.ResourceUpload");
        gpuGraphPassSetCallback(upload, upload_pass_callback, impl);
    }
    if (bundle->has_layer_raster) {
        GpuGraphPass layer = gpuGraphAddRenderPass(graph, "Dong.LayerRaster");
        bindColorTarget(layer);
    }
    if (bundle->has_ui_main && impl) {
        static thread_local UiMainPassPayload payload;
        payload.impl = impl;
        payload.bundle = bundle;
        GpuGraphPass ui = gpuGraphAddRenderPass(graph, "Dong.UIMain");
        bindColorTarget(ui);
        gpuGraphPassSetCallback(ui, ui_main_pass_callback, &payload);
    }
}
#endif

static void gpu_ui_graph_add_passes(DongGPUDriver* driver, DongUiGraphContext* ctx) {
    auto* impl = gpu_driver_impl(driver);
    if (!impl || !ctx) {
        return;
    }

#ifdef DONG_HAS_IN_DREAMING_GPU
    const DongUiPassBundle* bundle = dong_ui_graph_get_pass_bundle(ctx);
    auto* graph = static_cast<GpuGraph>(ctx->host_graph);
    GpuGraphResource target = ctx->composite_target
        ? static_cast<GpuGraphResource>(reinterpret_cast<uintptr_t>(ctx->composite_target))
        : GPU_GRAPH_NULL_RESOURCE;
    gpu_register_ui_passes_on_graph(graph, bundle, target, impl);
#else
    (void)impl;
    (void)ctx;
#endif
}

void gpu_register_ui_graph_vtable(DongGPUDriverVTable* vtable) {
    if (vtable) {
        vtable->ui_graph_add_passes = gpu_ui_graph_add_passes;
    }
}

} // namespace dong::gpu_backend
