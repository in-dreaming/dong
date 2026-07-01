#pragma once

#include "dong_gpu_driver.h"
#include "dong_image_atlas.h"
#include "dong_ui_graph.h"
#include "gpu_gpu_driver_execute_dispatch.hpp"
#include "gpu_image_atlas.hpp"
#include "gpu_pipeline_registry.hpp"
#include "gpu_resource_manager.hpp"
#include "gpu_upload_queue.hpp"

#include "../../src/render/glyph_atlas.hpp"
#include "../../src/render/resource_manager.hpp"
#include "../../src/render/text_renderer_selector.hpp"
#include "../../src/render/slug/slug_font_cache.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct DongImageAtlas;

namespace dong::gpu_backend {

class ExecuteDispatcher;

#ifdef DONG_HAS_IN_DREAMING_GPU
// Dong's UI pipeline stores/produces already display-ready (gamma-encoded) colors, not linear
// light values. Configuring the swapchain with an _SRGB surface format would make the GPU's
// render-output stage re-apply a linear->sRGB encode on every write, double-gamma-correcting the
// already-encoded UI colors into a washed-out/hazy image. Surfaces are queried for their
// "preferred" format, which on many platforms defaults to an _SRGB variant, so callers must
// strip it back to the plain UNORM equivalent before using it to configure the surface/swapchain.
inline GpuFormat stripSrgbFormat(GpuFormat fmt) {
    switch (fmt) {
        case GPU_FORMAT_RGBA8_UNORM_SRGB: return GPU_FORMAT_RGBA8_UNORM;
        case GPU_FORMAT_BGRA8_UNORM_SRGB: return GPU_FORMAT_BGRA8_UNORM;
        default: return fmt;
    }
}

struct ExternalImageGpu {
    DongGPUTexture texture = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct ImageAtlasEntryGpu {
    DongAtlasEntry entry{};
    uint32_t width = 0;
    uint32_t height = 0;
};
#endif

class GpuGPUDriverImpl {
public:
    GpuGPUDriverImpl();
    ~GpuGPUDriverImpl();

    bool initialize();
    void shutdown();

    int beginFrame();
    int endFrame();
    int execute(const void* command_list);
    void prepareResources(const void* command_list);

    int beginFrameOffscreen(DongGPUTexture target, uint32_t width, uint32_t height);
    int endFrameOffscreen();

    DongGPUDriver* apiDriver() { return &api_driver_; }
    DongUiGraphContext* uiGraph() { return &ui_graph_; }
    void* nativeDevice() const { return gpu_device_; }

    void setExternalSwapchain(void* surface, void* queue, int owns_surface);
    void setExternalWindow(void* window);
    void setResourceRoot(const char* root);

#ifdef DONG_HAS_IN_DREAMING_GPU
    GpuResourceManager& resources() { return resources_; }
    PipelineRegistry& pipelines() { return *pipeline_registry_; }
    GpuUploadQueue& uploadQueue() { return upload_queue_; }
    ExecuteDispatcher& dispatcher() { return *execute_dispatcher_; }
    GpuOffscreenState& offscreen() { return offscreen_; }
    DongImageAtlas* imageAtlas() { return image_atlas_; }
    render::ResourceManager& imageResourceManager() { return image_resource_manager_; }

    bool ensureImageInAtlas(const std::string& src, ImageAtlasEntryGpu& out_entry);
    render::GlyphAtlas* glyphAtlasForFontSize(float font_size);
    void prepareResourcesImpl(const void* command_list);
    void prepareSlugResources(const render::GPUCommandList& commands);
    void uploadSlugTextures();
    void drawTextSlug(GpuExecuteContext& ctx, const render::GPUCommand& cmd);
    render::TextRendererSelector& textRendererSelector() { return text_renderer_selector_; }
    void flushUploadPass(GpuGraphPassContext* ctx);
    void executeUiMainPass(GpuGraphPassContext* ctx, const DongUiPassBundle* bundle);

    bool updateExternalImageRGBA(const std::string& key, const uint8_t* rgba, uint32_t width, uint32_t height,
                                 uint32_t stride_bytes);
    bool updateExternalImageYUV420P(const std::string& key, const uint8_t* plane_y, uint32_t stride_y,
                                    const uint8_t* plane_u, uint32_t stride_u, const uint8_t* plane_v,
                                    uint32_t stride_v, uint32_t width, uint32_t height);
    const ExternalImageGpu* findExternalImage(const std::string& key) const;
    void syncSurfaceToWindow();
    void queryFramebufferSize(uint32_t& out_w, uint32_t& out_h) const;
    bool ensureIntermediateTexture(uint32_t width, uint32_t height);
    void executeSwapchainBlitPass(GpuGraphPassContext* ctx, DongGPUTexture intermediate, uint32_t width,
                                  uint32_t height);
    bool shouldDumpThisFrame(uint64_t* out_frame_index);
    void maybeDumpWindowFrame(uint32_t width, uint32_t height, uint64_t frame_index);
    void maybeDumpBackbuffer(GpuSurfaceTexture backbuffer, uint32_t width, uint32_t height, uint64_t frame_index);
#endif

private:
    DongGPUDriver api_driver_{};
    std::unique_ptr<PipelineRegistry> pipeline_registry_;
    std::unique_ptr<ExecuteDispatcher> execute_dispatcher_;
    DongUiGraphContext ui_graph_{};
    GpuResourceManager resources_;
    GpuUploadQueue upload_queue_;
    render::ResourceManager image_resource_manager_;
    int initialized_ = 0;

#ifdef DONG_HAS_IN_DREAMING_GPU
    void* gpu_device_ = nullptr;
    void* gpu_window_ = nullptr;
    void* gpu_surface_ = nullptr;
    void* gpu_queue_ = nullptr;
    int owns_external_surface_ = 0;
    int owns_platform_ = 0;
    // Whether *this driver* created gpu_device_/gpu_window_ itself (and must destroy them), as
    // opposed to receiving them from the host via setExternalDevice()/setExternalWindow() (where
    // the host retains ownership unless it explicitly opts in via owns_external_device).
    int owns_gpu_device_ = 0;
    int owns_gpu_window_ = 0;
    int owns_gpu_surface_ = 0;
    void* gtc_ctx_ = nullptr;
    GpuFormat swapchain_format_ = GPU_FORMAT_RGBA8_UNORM;
    GpuFormat render_format_ = GPU_FORMAT_RGBA8_UNORM;

    GpuOffscreenState offscreen_{};
    DongGPUTexture intermediate_texture_ = nullptr;
    uint32_t intermediate_width_ = 0;
    uint32_t intermediate_height_ = 0;
    DongImageAtlas* image_atlas_ = nullptr;
    std::vector<std::unique_ptr<render::GlyphAtlas>> glyph_atlas_tiers_;
    std::unique_ptr<render::slug::SlugFontCache> slug_font_cache_;
    render::TextRendererSelector text_renderer_selector_;
    DongGPUTexture slug_curve_texture_ = nullptr;
    DongGPUTexture slug_band_texture_ = nullptr;
    uint32_t slug_curve_texture_height_ = 0;
    uint32_t slug_band_texture_height_ = 0;
    // Bump-allocated per-frame vertex buffer for Slug text draws. Buffers are recorded into the
    // render graph but only actually consumed by the GPU once the whole graph is submitted, so a
    // freshly created-and-destroyed-per-draw buffer would be released (via the immediate,
    // non-frame-context release path) before the GPU ever reads it, corrupting the rest of the
    // pass. Instead we keep one growable buffer alive for the driver's lifetime and hand out
    // non-overlapping offsets within it, resetting the offset once per execute() call.
    DongGPUBuffer slug_vertex_buffer_ = nullptr;
    uint32_t slug_vertex_buffer_capacity_ = 0;
    uint32_t slug_vertex_write_offset_ = 0;
    std::vector<DongGPUBuffer> retired_slug_vertex_buffers_;
    std::unordered_map<std::string, ImageAtlasEntryGpu> image_atlas_entries_;
    std::unordered_map<std::string, ExternalImageGpu> external_images_;
    std::string resource_root_;
    uint64_t frame_index_ = 0;
    uint64_t dump_frame_counter_ = 0;
#endif
};

GpuGPUDriverImpl* gpu_driver_impl(DongGPUDriver* driver);

} // namespace dong::gpu_backend
