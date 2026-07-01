#include "gpu_gpu_driver.hpp"
#include "gpu_gpu_driver_ui_graph.hpp"
#include "gpu_pipeline_registry.hpp"
#include "dong_gpu_backend_driver.h"

#include "../../src/core/log.h"

#include <cstring>

namespace dong::gpu_backend {

void gpu_register_full_vtable(DongGPUDriverVTable* vtable);

static DongGPUDriverVTable g_gpu_vtable{};

GpuGPUDriverImpl* gpu_driver_impl(DongGPUDriver* driver) {
    return driver ? static_cast<GpuGPUDriverImpl*>(driver->user_data) : nullptr;
}

static void ensure_vtable() {
    if (g_gpu_vtable.initialize) {
        return;
    }
    gpu_register_full_vtable(&g_gpu_vtable);
    gpu_register_ui_graph_vtable(&g_gpu_vtable);
}

GpuGPUDriverImpl::GpuGPUDriverImpl() {
    ensure_vtable();
    std::memset(&api_driver_, 0, sizeof(api_driver_));
    api_driver_.vtable = &g_gpu_vtable;
    api_driver_.user_data = this;
    api_driver_.embedded_mode = 1;
    pipeline_registry_ = std::make_unique<PipelineRegistry>();
    execute_dispatcher_ = std::make_unique<ExecuteDispatcher>();
}

GpuGPUDriverImpl::~GpuGPUDriverImpl() {
    shutdown();
}

void GpuGPUDriverImpl::setExternalSwapchain(void* surface, void* queue, int owns_surface) {
#ifdef DONG_HAS_IN_DREAMING_GPU
    gpu_surface_ = surface;
    gpu_queue_ = queue;
    owns_external_surface_ = owns_surface ? 1 : 0;
    // Surfaces set via this path came from the host; only destroy them in shutdown() if the host
    // explicitly transfers ownership (owns_surface=1). Otherwise the host destroys it itself.
    owns_gpu_surface_ = owns_external_surface_;
    if (gpu_surface_ && gpu_device_) {
        swapchain_format_ = stripSrgbFormat(gpuSurfaceGetPreferredFormat(static_cast<GpuSurface>(gpu_surface_)));
    }
#else
    (void)surface;
    (void)queue;
    (void)owns_surface;
#endif
}

void GpuGPUDriverImpl::setExternalWindow(void* window) {
#ifdef DONG_HAS_IN_DREAMING_GPU
    gpu_window_ = window;
#else
    (void)window;
#endif
}

void GpuGPUDriverImpl::setResourceRoot(const char* root) {
    if (root) {
        resource_root_ = root;
        image_resource_manager_.setResourceRoot(resource_root_);
    }
}

} // namespace dong::gpu_backend

extern "C" DONG_GPU_BACKEND_API DongGPUDriver* dong_gpu_backend_create_driver(void) {
    auto* impl = new (std::nothrow) dong::gpu_backend::GpuGPUDriverImpl();
    return impl ? impl->apiDriver() : nullptr;
}

extern "C" DONG_GPU_BACKEND_API void dong_gpu_backend_destroy_driver(DongGPUDriver* driver) {
    if (!driver) {
        return;
    }
    auto* impl = dong::gpu_backend::gpu_driver_impl(driver);
    delete impl;
}

extern "C" DONG_GPU_BACKEND_API DongUiGraphContext* dong_gpu_backend_get_ui_graph(DongGPUDriver* driver) {
    auto* impl = dong::gpu_backend::gpu_driver_impl(driver);
    return impl ? impl->uiGraph() : nullptr;
}

extern "C" DONG_GPU_BACKEND_API void dong_gpu_driver_set_external_swapchain(DongGPUDriver* driver,
                                                                              void* surface,
                                                                              void* queue,
                                                                              int owns_surface) {
    auto* impl = dong::gpu_backend::gpu_driver_impl(driver);
    if (impl) {
        impl->setExternalSwapchain(surface, queue, owns_surface);
    }
}

extern "C" DONG_GPU_BACKEND_API void dong_gpu_driver_set_external_window(DongGPUDriver* driver, void* window) {
    auto* impl = dong::gpu_backend::gpu_driver_impl(driver);
    if (impl) {
        impl->setExternalWindow(window);
    }
}

extern "C" DONG_GPU_BACKEND_API int dong_gpu_backend_get_texture_shader_view(DongGPUDriver* driver,
                                                                             DongGPUTexture texture,
                                                                             DongGpuTextureViewHandle* out_view) {
    if (!out_view) {
        return 0;
    }
    out_view->index = 0;
    out_view->generation = 0;
    auto* impl = dong::gpu_backend::gpu_driver_impl(driver);
    if (!impl || !texture) {
        return 0;
    }
#ifdef DONG_HAS_IN_DREAMING_GPU
    // NOTE: despite the name, callers (e.g. scene3d_world_gpu_render -> gpuPassBindTexture) need
    // the *plain texture* handle here, not the SRV `TextureView` handle from
    // gpuTextureShaderView(). gpuPassBindTexture() resolves its input through
    // GpuDevice::texturePool (rhi::ITexture) and derives the view itself via
    // tex->getDefaultView(); GpuDevice::textureViewPool (rhi::ITextureView, what
    // gpuTextureShaderView() returns) is a completely separate handle pool/index space, so
    // passing that here made every lookup silently fail (resolve() returns null) and no 3D
    // scene screen ever drew - just the background clear color ("3d_screens_simple 黑屏").
    GpuTextureHandle tex = impl->resources().gpuTextureHandle(texture);
    if (tex.index == 0) {
        return 0;
    }
    out_view->index = tex.index;
    out_view->generation = tex.generation;
    return 1;
#else
    (void)texture;
    return 0;
#endif
}

extern "C" DONG_GPU_BACKEND_API int dong_gpu_backend_readback_texture_rgba(DongGPUDriver* driver,
                                                                           DongGPUTexture texture,
                                                                           uint32_t width,
                                                                           uint32_t height,
                                                                           uint8_t* out_rgba,
                                                                           size_t out_rgba_bytes) {
    auto* impl = dong::gpu_backend::gpu_driver_impl(driver);
    if (!impl || !texture || !out_rgba) {
        return -1;
    }
#ifdef DONG_HAS_IN_DREAMING_GPU
    return impl->resources().readbackTextureRGBA(texture, width, height, out_rgba, out_rgba_bytes);
#else
    (void)width;
    (void)height;
    (void)out_rgba_bytes;
    return -1;
#endif
}
