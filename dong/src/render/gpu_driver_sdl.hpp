#pragma once

#include "gpu_driver.hpp"
#include <SDL3/SDL_gpu.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace dong::render {

class GPUDevice;
class ShaderManager;
class ResourceManager;
class GlyphAtlas;

// 基于 SDL_gpu 的 GPUDriver 实现（最小骨架）
class GPUDriverSDL : public GPUDriver {
public:
    GPUDriverSDL(GPUDevice* device, SDL_Window* window, ShaderManager* shader_manager);
    ~GPUDriverSDL() override;

    // 目前只做基本有效性检查
    bool initialize() override;

    void beginFrame() override;
    void endFrame() override;
    void execute(const GPUCommandList& commands) override;
    
    // 预处理命令列表中的资源（如 glyph 纹理上传）
    void prepareResources(const GPUCommandList& commands) override;
    
    // Offscreen rendering support
    void beginFrameOffscreen(SDL_GPUTexture* target, uint32_t width, uint32_t height) override;
    void endFrameOffscreen() override;

    // 供外部注入图片资源管理器（用于从 Skia/CPU 侧取得图片像素）
    void setImageResourceManager(ResourceManager* manager) { image_resource_manager_ = manager; }

    // Debug: 启用时在 execute 末尾按 draw_batches 做一次批次遍历并输出日志
    void setDebugLogDrawBatches(bool enable) { debug_log_draw_batches_ = enable; }

    // Debug: 打印隔离图层缓存（重栅格 / 缓存复用）情况
    void setDebugLogLayerCache(bool enable) { debug_log_layer_cache_ = enable; }

    // MSDF 文本：是否启用 subpixel 渲染路径
    void setMsdfSubpixelEnabled(bool enable) { msdf_subpixel_enabled_ = enable; }

    // 启用或关闭图层缓存复用（默认关闭，确保正确性优先）
    void setLayerCacheEnabled(bool enable) { layer_cache_enabled_ = enable; }

    // Workaround: 在隔离层切换时拆分 command buffer，避免同一纹理在同一 command buffer 内多次 Begin/End render pass 导致内容丢失。
    void setSplitCommandBufferForIsolatedLayers(bool enable) { split_cmd_buf_for_isolated_layers_ = enable; }


private:
    GPUDevice* gpu_device_;
    SDL_Window* window_;
    ShaderManager* shader_manager_;
    ResourceManager* image_resource_manager_ = nullptr;
    SDL_GPUCommandBuffer* current_cmd_buf_ = nullptr;
    bool in_frame_ = false;
    bool debug_log_draw_batches_ = false;
    bool debug_log_layer_cache_ = false;
    bool msdf_subpixel_enabled_ = false;
    bool layer_cache_enabled_ = false; // 控制是否复用隔离层缓存纹理（默认关闭，确保正确性优先）
    bool split_cmd_buf_for_isolated_layers_ = true; // Workaround: 拆分 command buffer 以规避 LOADOP/多 pass 问题
    bool debug_rt_enabled_ = false;    // 调试：打印帧级 / RenderTarget / 图层合成日志

    unsigned long long frame_index_ = 0;
    
    // Offscreen rendering support
    SDL_GPUTexture* offscreen_target_ = nullptr;
    uint32_t offscreen_width_ = 0;
    uint32_t offscreen_height_ = 0;

    // 中间渲染纹理：用于避免直接在 swapchain 上进行多次 render pass
    // 所有渲染先在此纹理上进行，最后 blit 到 swapchain
    SDL_GPUTexture* intermediate_texture_ = nullptr;
    uint32_t intermediate_width_ = 0;
    uint32_t intermediate_height_ = 0;

    // 纯色矩形绘制管线
    SDL_GPUShader* rect_vs_ = nullptr;
    SDL_GPUShader* rect_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* rect_pipeline_ = nullptr;

    // 圆角矩形绘制管线（analytic SDF）
    SDL_GPUShader* round_rect_vs_ = nullptr;
    SDL_GPUShader* round_rect_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* round_rect_pipeline_ = nullptr;

    // 阴影绘制管线（SDF + blur）
    SDL_GPUShader* shadow_vs_ = nullptr;
    SDL_GPUShader* shadow_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* shadow_pipeline_ = nullptr;

    // 图片绘制相关：基于一个简单的 2D atlas 纹理
    SDL_GPUShader* image_vs_ = nullptr;
    SDL_GPUShader* image_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* image_pipeline_ = nullptr;
    // 专用于“备份/恢复父 render target”的 copy 管线：关闭 blending，避免把颜色再乘一次 alpha
    SDL_GPUGraphicsPipeline* image_copy_pipeline_ = nullptr;
    SDL_GPUTexture* image_atlas_texture_ = nullptr;
    SDL_GPUSampler* image_sampler_ = nullptr;

    Uint32 image_atlas_width_ = 0;
    Uint32 image_atlas_height_ = 0;
    Uint32 atlas_cursor_x_ = 0;
    Uint32 atlas_cursor_y_ = 0;
    Uint32 atlas_row_height_ = 0;

    // 图层离屏渲染目标缓存池：按尺寸复用纹理，减少反复创建/销毁
    struct LayerRenderTarget {
        SDL_GPUTexture* texture = nullptr;
        Uint32 width = 0;
        Uint32 height = 0;
        uint64_t layer_id = 0;       // 绑定的逻辑图层 ID（0 表示未绑定）
        bool in_use = false;         // 本帧是否作为 render target 正在使用
        bool valid_for_cache = false;// 纹理内容是否可作为缓存复用
    };
    std::vector<LayerRenderTarget> layer_render_targets_;

    struct ImageAtlasEntry {
        float u0;
        float v0;
        float u1;
        float v1;
        uint32_t width;
        uint32_t height;
    };
    std::unordered_map<std::string, ImageAtlasEntry> image_atlas_entries_;

    bool ensureImageInAtlas(const std::string& src, ImageAtlasEntry& out_entry);

    // MSDF 文字渲染
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

    GlyphAtlasTier* selectGlyphAtlasTier(float font_size);
    FT_Face getOrCreateFace(const std::string& font_path, uint32_t pixel_size);
};

} // namespace dong::render
