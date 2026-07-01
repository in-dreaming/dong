#include "gpu_gpu_driver.hpp"
#include "gpu_pipeline_registry.hpp"
#include "gpu_image_atlas.hpp"
#include "gpu_gtc_service.hpp"

#include "../../src/core/log.h"
#include "dong_gtc.h"

#include <cstdio>

#ifdef DONG_HAS_IN_DREAMING_GPU
#include "gpu/gpu.h"
#include "gpu/platform/gpu_platform.h"
#include "gpu/platform/gpu_surface.h"
#endif

#ifndef DONG_GPU_SHADER_DIR
#define DONG_GPU_SHADER_DIR "shaders"
#endif

namespace dong::gpu_backend {

bool GpuGPUDriverImpl::initialize() {
    if (initialized_) {
        return true;
    }

#ifdef DONG_HAS_IN_DREAMING_GPU
    const bool external_device_provided = api_driver_.external_device != nullptr;
    if (!external_device_provided) {
        if (gpuPlatformInit() != GPU_SUCCESS) {
            DONG_LOG_ERROR("GpuGPUDriver: gpuPlatformInit failed");
            return false;
        }
        owns_platform_ = 1;
    }

    if (external_device_provided) {
        gpu_device_ = api_driver_.external_device;
        // owns_external_device is a host opt-in ("1 if Dong should destroy external_device");
        // by default (0) the host retains ownership and will destroy the device itself.
        owns_gpu_device_ = api_driver_.owns_external_device ? 1 : 0;
        if (gpu_surface_) {
            swapchain_format_ = stripSrgbFormat(gpuSurfaceGetPreferredFormat(static_cast<GpuSurface>(gpu_surface_)));
        } else {
            swapchain_format_ = GPU_FORMAT_RGBA8_UNORM;
        }
    } else {
        GpuWindow window = nullptr;
        GpuWindowDesc win_desc{};
        win_desc.title = "Dong GPU Backend";
        win_desc.width = 1280;
        win_desc.height = 720;
        win_desc.vsync = true;
        if (gpuCreateWindow(&win_desc, &window) != GPU_SUCCESS) {
            DONG_LOG_ERROR("GpuGPUDriver: gpuCreateWindow failed");
            return false;
        }
        gpu_window_ = window;
        owns_gpu_window_ = 1;

        GpuDevice device = nullptr;
        GpuDeviceDesc dev_desc{};
        dev_desc.appName = "dong_gpu_backend";
        if (gpuCreateDevice(&dev_desc, &device) != GPU_SUCCESS) {
            DONG_LOG_ERROR("GpuGPUDriver: gpuCreateDevice failed");
            return false;
        }
        gpu_device_ = device;
        api_driver_.external_device = device;
        owns_gpu_device_ = 1;

        GpuSurface surface = nullptr;
        if (gpuCreateSurface(device, window, GPU_SURFACE_TYPE_VULKAN, &surface) == GPU_SUCCESS) {
            gpu_surface_ = surface;
            owns_gpu_surface_ = 1;
            swapchain_format_ = stripSrgbFormat(gpuSurfaceGetPreferredFormat(surface));
            gpuSurfaceConfigure(surface, win_desc.width, win_desc.height, swapchain_format_, true);
        } else {
            DONG_LOG_WARN("GpuGPUDriver: surface create failed (offscreen-only)");
            swapchain_format_ = GPU_FORMAT_RGBA8_UNORM;
        }
    }

    if (!gpu_queue_ && gpu_device_) {
        GpuCommandQueue queue = nullptr;
        if (gpuGetQueue(static_cast<GpuDevice>(gpu_device_), GPU_QUEUE_TYPE_GRAPHICS, &queue) == GPU_SUCCESS) {
            gpu_queue_ = queue;
        }
    }

    upload_queue_.initialize(static_cast<GpuDevice>(gpu_device_));
    resources_.initialize(static_cast<GpuDevice>(gpu_device_), &upload_queue_);

    render_format_ = GPU_FORMAT_RGBA8_UNORM;

    if (!pipeline_registry_->initialize(static_cast<GpuDevice>(gpu_device_), render_format_, DONG_GPU_SHADER_DIR,
                                        gpu_surface_ ? swapchain_format_ : GPU_FORMAT_UNDEFINED)) {
        return false;
    }
    DONG_LOG_INFO("GpuGPUDriver: pipelines render_format=%u swapchain_format=%u",
                  (unsigned)render_format_, (unsigned)swapchain_format_);
    std::fprintf(stderr, "[GpuGPUDriver] render_format=%u swapchain_format=%u surface=%p window=%p\n",
                 (unsigned)render_format_, (unsigned)swapchain_format_, gpu_surface_, gpu_window_);

    if (gpu_device_) {
        gtc_ctx_ = dong_gtc_create(gpu_device_, DONG_GTC_BACKEND_IN_DREAMING_GPU);
        if (gtc_ctx_) {
            dong_gtc_set_default(static_cast<DongGtcContext*>(gtc_ctx_));
            gpu_gtc_service_register();
            DONG_LOG_INFO("GpuGPUDriver: dong_gtc (in-dreaming/gpu) initialized");
        }
    }

    DongAtlasConfig atlas_cfg{};
    atlas_cfg.width = 2048;
    atlas_cfg.height = 2048;
    atlas_cfg.max_pages = 4;
    atlas_cfg.format = DONG_IMAGE_FORMAT_RGBA8;
    atlas_cfg.padding = 2;
    image_atlas_ = gpu_image_atlas_create(&resources_, &atlas_cfg);
    if (!image_atlas_) {
        DONG_LOG_WARN("GpuGPUDriver: image atlas creation failed");
    }

    static const uint32_t kGlyphTiers[] = {32, 48, 64, 96, 128};
    for (uint32_t tier : kGlyphTiers) {
        auto atlas = std::make_unique<render::GlyphAtlas>(&api_driver_);
        if (atlas->initialize(2048, 2048, tier, 8.0f)) {
            glyph_atlas_tiers_.push_back(std::move(atlas));
        }
    }

    slug_font_cache_ = std::make_unique<render::slug::SlugFontCache>();
    if (pipeline_registry_->pipeline(GpuPipelineKind::Slug)) {
        text_renderer_selector_.setSlugAvailable(true);
        DONG_LOG_INFO("GpuGPUDriver: Slug text pipeline initialized");
        std::fprintf(stderr, "[GpuGPUDriver] text_renderer=Auto (Slug when available)\n");
    } else {
        DONG_LOG_WARN("GpuGPUDriver: Slug pipeline unavailable, MSDF fallback only");
        std::fprintf(stderr, "[GpuGPUDriver] text_renderer=MSDF only (Slug pipeline unavailable)\n");
    }
#endif

    dong_ui_graph_context_init(&ui_graph_);
    ui_graph_.driver = &api_driver_;
    ui_graph_.device = api_driver_.external_device;

    initialized_ = 1;
    DONG_LOG_INFO("GpuGPUDriver: initialized (embedded_mode=%d)", api_driver_.embedded_mode);
    return true;
}

void GpuGPUDriverImpl::shutdown() {
    if (!initialized_) {
        return;
    }
#ifdef DONG_HAS_IN_DREAMING_GPU
    glyph_atlas_tiers_.clear();
    if (slug_curve_texture_) {
        resources_.destroyTexture(slug_curve_texture_);
        slug_curve_texture_ = nullptr;
        slug_curve_texture_height_ = 0;
    }
    if (slug_band_texture_) {
        resources_.destroyTexture(slug_band_texture_);
        slug_band_texture_ = nullptr;
        slug_band_texture_height_ = 0;
    }
    if (slug_vertex_buffer_) {
        resources_.destroyBuffer(slug_vertex_buffer_);
        slug_vertex_buffer_ = nullptr;
        slug_vertex_buffer_capacity_ = 0;
        slug_vertex_write_offset_ = 0;
    }
    for (DongGPUBuffer buf : retired_slug_vertex_buffers_) {
        resources_.destroyBuffer(buf);
    }
    retired_slug_vertex_buffers_.clear();
    slug_font_cache_.reset();
    text_renderer_selector_.setSlugAvailable(false);
    if (image_atlas_) {
        gpu_image_atlas_destroy(image_atlas_);
        image_atlas_ = nullptr;
    }
    image_atlas_entries_.clear();
    external_images_.clear();

    if (intermediate_texture_) {
        resources_.destroyTexture(intermediate_texture_);
        intermediate_texture_ = nullptr;
        intermediate_width_ = 0;
        intermediate_height_ = 0;
    }

    if (gtc_ctx_) {
        if (dong_gtc_get_default() == static_cast<DongGtcContext*>(gtc_ctx_)) {
            dong_gtc_set_default(nullptr);
        }
        dong_gtc_destroy(static_cast<DongGtcContext*>(gtc_ctx_));
        gtc_ctx_ = nullptr;
    }

    // Only destroy the surface if *this driver* created it internally, or the host explicitly
    // transferred ownership via owns_surface=1 in setExternalSwapchain(). Real callers
    // (dong_gpu_app_platform.c, examples/shared/demo_gpu_platform.c) always pass owns_surface=0
    // and destroy the surface themselves after dong_gpu_driver_shutdown() returns; destroying it
    // here too was a double-destroy that crashed those embedders on shutdown.
    if (gpu_surface_ && gpu_device_ && owns_gpu_surface_) {
        gpuDestroySurface(static_cast<GpuDevice>(gpu_device_), static_cast<GpuSurface>(gpu_surface_));
    }
    gpu_surface_ = nullptr;
    owns_gpu_surface_ = 0;
    // Only destroy the device/window if *this driver* created them (owns_gpu_device_/
    // owns_gpu_window_); when the host provided them via setExternalDevice()/setExternalWindow()
    // without opting into ownership transfer, the host is responsible for destroying them itself
    // (double-destroying here was crashing embedders like dong_app_gpu on shutdown).
    if (gpu_device_ && owns_gpu_device_) {
        gpuDestroyDevice(static_cast<GpuDevice>(gpu_device_));
    }
    if (gpu_window_ && owns_gpu_window_) {
        gpuDestroyWindow(static_cast<GpuWindow>(gpu_window_));
    }
    gpu_window_ = nullptr;
    gpu_device_ = nullptr;
    owns_gpu_device_ = 0;
    owns_gpu_window_ = 0;
    if (owns_platform_) {
        gpuPlatformShutdown();
        owns_platform_ = 0;
    }
    gpu_queue_ = nullptr;
    pipeline_registry_->shutdown();
    resources_.shutdown();
    upload_queue_.shutdown();
#endif
    dong_ui_graph_reset(&ui_graph_);
    initialized_ = 0;
}

int GpuGPUDriverImpl::beginFrame() {
#ifdef DONG_HAS_IN_DREAMING_GPU
    if (initialized_) {
        upload_queue_.beginFrame();
        ++frame_index_;
    }
#endif
    return initialized_ ? 1 : 0;
}

int GpuGPUDriverImpl::endFrame() {
#ifdef DONG_HAS_IN_DREAMING_GPU
    if (initialized_) {
        upload_queue_.endFrame();
    }
#endif
    return initialized_ ? 1 : 0;
}

} // namespace dong::gpu_backend
