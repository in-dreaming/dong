#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <SDL3/SDL_gpu.h>

// Forward declaration for ImageAtlas
struct DongImageAtlas;

// Forward declaration for C API GPU Driver
typedef struct DongGPUDriver DongGPUDriver;

// SDL types (backend internal use)
struct SDL_Window;

namespace dong {
namespace sdl_backend {
class GPUDevice;
class ShaderManager;
class GPUTextureCompressor;
}

namespace render {

// Core types (forward declarations, will be decoupled in future phases)
class ResourceManager;
class GlyphAtlas;
struct GPUCommandList;
struct GPUCommand;
struct Color;
struct Rect;

// =============================================================================
// SDL GPU Driver Implementation (Backend)
// =============================================================================
// This is the SDL-specific implementation of GPU rendering.
// It is part of the dong_sdl_backend DLL, not the core dong DLL.
//
// Responsibilities:
//   - Manage SDL GPU resources (shaders, pipelines, textures)
//   - Execute GPUCommandList from core engine
//   - Manage glyph atlas and image atlas
//   - Handle offscreen rendering
//
// Note: This class uses core types (GPUDevice, ShaderManager, etc.) which
// will be gradually decoupled in future refactoring phases.
// =============================================================================

class SDLGPUDriver {
public:
    SDLGPUDriver(sdl_backend::GPUDevice* device, SDL_Window* window, sdl_backend::ShaderManager* shader_manager);
    ~SDLGPUDriver();

    // Initialize pipelines and resources
    bool initialize();

    // Frame management
    void beginFrame();
    void endFrame();
    bool isInFrame() const { return in_frame_; }

    // Execute GPU command list from core
    void execute(const GPUCommandList& commands);

    // Resource preparation (upload textures, glyphs before render pass)
    void prepareResources(const GPUCommandList& commands);

    // Offscreen rendering
    void beginFrameOffscreen(SDL_GPUTexture* target, uint32_t width, uint32_t height);
    void endFrameOffscreen();

    // External image updates (video frames)
    bool updateExternalImageRGBA(const std::string& key,
                                 const uint8_t* rgba,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t stride_bytes);

    bool updateExternalImageYUV420P(const std::string& key,
                                    const uint8_t* plane_y,
                                    uint32_t stride_y,
                                    const uint8_t* plane_u,
                                    uint32_t stride_u,
                                    const uint8_t* plane_v,
                                    uint32_t stride_v,
                                    uint32_t width,
                                    uint32_t height);

    // Upload RGBA pixels into a sub-rectangle of an existing texture.
    // IMPORTANT: This tries to reuse the current frame command buffer when possible,
    // so uploads are ordered before subsequent draws in the same frame.
    bool uploadTextureSubrectRGBA(SDL_GPUTexture* texture,
                                 const void* rgba,
                                 uint32_t dest_x,
                                 uint32_t dest_y,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t src_stride_bytes);

    // Resource manager injection
    void setImageResourceManager(ResourceManager* manager) { image_resource_manager_ = manager; }

    // DongGPUDriver injection (required for GlyphAtlas creation)
    void setDongGPUDriver(DongGPUDriver* driver) { dong_gpu_driver_ = driver; }
    DongGPUDriver* getDongGPUDriver() const { return dong_gpu_driver_; }

    // Debug options
    void setDebugLogDrawBatches(bool enable) { debug_log_draw_batches_ = enable; }
    void setDebugLogLayerCache(bool enable) { debug_log_layer_cache_ = enable; }
    void setMsdfSubpixelEnabled(bool enable) { msdf_subpixel_enabled_ = enable; }
    void setLayerCacheEnabled(bool enable) { layer_cache_enabled_ = enable; }
    void setSplitCommandBufferForIsolatedLayers(bool enable) { split_cmd_buf_for_isolated_layers_ = enable; }

    // HDR support
    void setHDREnabled(bool enable);
    bool isHDREnabled() const { return hdr_enabled_; }
    SDL_GPUTextureFormat getRenderTargetFormat() const { return render_target_format_; }

private:
    // Dependencies (to be decoupled in future phases)
    sdl_backend::GPUDevice* gpu_device_;
    SDL_Window* window_;
    sdl_backend::ShaderManager* shader_manager_;
    ResourceManager* image_resource_manager_ = nullptr;
    DongGPUDriver* dong_gpu_driver_ = nullptr;  // C API driver for GlyphAtlas
    std::unique_ptr<sdl_backend::GPUTextureCompressor> gpu_compressor_;  // GPU texture compression

    // SDL GPU state
    SDL_GPUCommandBuffer* current_cmd_buf_ = nullptr;
    bool in_frame_ = false;

    // Debug flags
    bool debug_log_draw_batches_ = false;
    bool debug_log_layer_cache_ = false;
    bool msdf_subpixel_enabled_ = false;
    bool layer_cache_enabled_ = false;
    bool split_cmd_buf_for_isolated_layers_ = true;
    bool debug_rt_enabled_ = false;

    // HDR support
    bool hdr_enabled_ = false;
    SDL_GPUTextureFormat render_target_format_ = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    // Frame counter
    unsigned long long frame_index_ = 0;

    // Offscreen rendering
    SDL_GPUTexture* offscreen_target_ = nullptr;
    uint32_t offscreen_width_ = 0;
    uint32_t offscreen_height_ = 0;

    // Intermediate render target
    SDL_GPUTexture* intermediate_texture_ = nullptr;
    uint32_t intermediate_width_ = 0;
    uint32_t intermediate_height_ = 0;
    bool intermediate_valid_ = false;

    // Pipelines and shaders
    SDL_GPUShader* rect_vs_ = nullptr;
    SDL_GPUShader* rect_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* rect_pipeline_ = nullptr;

    SDL_GPUShader* round_rect_vs_ = nullptr;
    SDL_GPUShader* round_rect_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* round_rect_pipeline_ = nullptr;

    SDL_GPUShader* shadow_vs_ = nullptr;
    SDL_GPUShader* shadow_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* shadow_pipeline_ = nullptr;

    SDL_GPUShader* gradient_vs_ = nullptr;
    SDL_GPUShader* gradient_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* gradient_pipeline_ = nullptr;

    SDL_GPUShader* image_vs_ = nullptr;
    SDL_GPUShader* image_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* image_pipeline_ = nullptr;

    SDL_GPUShader* video_yuv_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* video_yuv_pipeline_ = nullptr;
    SDL_GPUGraphicsPipeline* image_copy_pipeline_ = nullptr;

    // Image atlas
    DongImageAtlas* image_atlas_ = nullptr;
    SDL_GPUSampler* image_sampler_ = nullptr;
    SDL_GPUSampler* image_sampler_nearest_ = nullptr;


    // Layer render target pool
    struct LayerRenderTarget {
        SDL_GPUTexture* texture = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        uint64_t layer_id = 0;
        bool in_use = false;
        bool valid_for_cache = false;
    };
    std::vector<LayerRenderTarget> layer_render_targets_;

    struct ImageAtlasEntry {
        float u0, v0, u1, v1;
        uint32_t width, height;
    };
    std::unordered_map<std::string, ImageAtlasEntry> image_atlas_entries_;

    // External images (video frames)
    enum class ExternalImageFormat : uint8_t { RGBA8 = 0, YUV420P = 1 };
    struct ExternalImage {
        ExternalImageFormat format = ExternalImageFormat::RGBA8;
        SDL_GPUTexture* texture = nullptr;
        SDL_GPUTexture* texture_u = nullptr;
        SDL_GPUTexture* texture_v = nullptr;
        uint32_t width = 0, height = 0;
    };
    std::unordered_map<std::string, ExternalImage> external_images_;

    // Upload buffer management
    struct UploadBuffer {
        void* buf = nullptr;  // SDL_GPUTransferBuffer*
        uint32_t size = 0;
    };
    std::vector<UploadBuffer> free_upload_buffers_;
    std::vector<UploadBuffer> frame_upload_buffers_;
    struct PendingUploadBuffers {
        void* fence = nullptr;  // SDL_GPUFence*
        std::vector<UploadBuffer> buffers;
    };
    std::vector<PendingUploadBuffers> pending_upload_buffers_;

    void reapUploadBuffers(SDL_GPUDevice* dev);
    UploadBuffer acquireUploadBuffer(SDL_GPUDevice* dev, uint32_t size);

    SDL_GPUCommandBuffer* acquireCommandBufferForUploads(SDL_GPUDevice* dev, bool& out_temp);
    void submitStandaloneUploadCommandBuffer(SDL_GPUDevice* dev, SDL_GPUCommandBuffer* cmd_buf);

    bool ensureImageInAtlas(const std::string& src, ImageAtlasEntry& out_entry);


    // Text rendering
    SDL_GPUShader* text_vs_ = nullptr;
    SDL_GPUShader* text_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* text_pipeline_ = nullptr;
    SDL_GPUSampler* text_sampler_ = nullptr;

    struct GlyphAtlasTier {
        uint32_t bitmap_px = 0;
        float distance_range = 0.0f;
        std::unique_ptr<GlyphAtlas> atlas;
    };
    std::vector<GlyphAtlasTier> glyph_atlas_tiers_;
    FT_Library ft_library_ = nullptr;
    std::unordered_map<std::string, FT_Face> ft_face_cache_;

    // GlobalShared 集成
    bool use_global_shared_glyph_atlas_ = false;
    
    // 获取适合字号的 GlyphAtlas
    // 如果使用 GlobalShared，则返回 GlobalShared 的 atlas
    GlyphAtlas* getGlyphAtlasForFontSize(float font_size);
    
    // 根据 bitmap_px 获取 GlyphAtlas（用于 prepareResources）
    // 如果使用 GlobalShared，则返回 GlobalShared 的 atlas
    GlyphAtlas* getGlyphAtlasForBitmapPx(uint32_t bitmap_px);
    
    FT_Face getOrCreateFace(const std::string& font_path, uint32_t pixel_size);

    // Execute context (refactored from monolithic execute)
    struct ExecuteContext;
    bool executeSetupMainTarget(ExecuteContext& ctx);
    void executePreuploadImages(const GPUCommandList& commands);
    void executeDispatchCommand(const GPUCommand& cmd, ExecuteContext& ctx);

    void executeBeginPass(ExecuteContext& ctx);
    void executeEndPass(ExecuteContext& ctx);

    void executePushClipRect(ExecuteContext& ctx, const GPUCommand& cmd);
    void executePopClip(ExecuteContext& ctx);

    void executeBeginIsolatedLayer(ExecuteContext& ctx, const GPUCommand& cmd);
    void executeEndIsolatedLayer(ExecuteContext& ctx, const GPUCommand& cmd);

    void executeDrawRect(ExecuteContext& ctx, const GPUCommand& cmd);
    void executeDrawRoundedRect(ExecuteContext& ctx, const GPUCommand& cmd);
    void executeDrawShadow(ExecuteContext& ctx, const GPUCommand& cmd);
    void executeDrawGradient(ExecuteContext& ctx, const GPUCommand& cmd);
    void executeDrawImage(ExecuteContext& ctx, const GPUCommand& cmd);
    void executeDrawText(ExecuteContext& ctx, const GPUCommand& cmd);
};

// Factory function (backend internal)
std::unique_ptr<SDLGPUDriver> CreateSDLGPUDriver(
    sdl_backend::GPUDevice* device,
    SDL_Window* window,
    sdl_backend::ShaderManager* shader_manager
);

} // namespace render
} // namespace dong
