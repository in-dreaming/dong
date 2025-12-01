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
    bool initialize();

    void beginFrame() override;
    void endFrame() override;
    void execute(const GPUCommandList& commands) override;
    
    // Offscreen rendering support
    void beginFrameOffscreen(SDL_GPUTexture* target, uint32_t width, uint32_t height) override;
    void endFrameOffscreen() override;

    // 供外部注入图片资源管理器（用于从 Skia/CPU 侧取得图片像素）
    void setImageResourceManager(ResourceManager* manager) { image_resource_manager_ = manager; }

private:
    GPUDevice* gpu_device_;
    SDL_Window* window_;
    ShaderManager* shader_manager_;
    ResourceManager* image_resource_manager_ = nullptr;
    SDL_GPUCommandBuffer* current_cmd_buf_ = nullptr;
    bool in_frame_ = false;
    
    // Offscreen rendering support
    SDL_GPUTexture* offscreen_target_ = nullptr;
    uint32_t offscreen_width_ = 0;
    uint32_t offscreen_height_ = 0;

    // 纯色矩形绘制管线
    SDL_GPUShader* rect_vs_ = nullptr;
    SDL_GPUShader* rect_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* rect_pipeline_ = nullptr;

    // 圆角矩形绘制管线（analytic SDF）
    SDL_GPUShader* round_rect_vs_ = nullptr;
    SDL_GPUShader* round_rect_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* round_rect_pipeline_ = nullptr;

    // 图片绘制相关：基于一个简单的 2D atlas 纹理
    SDL_GPUShader* image_vs_ = nullptr;
    SDL_GPUShader* image_fs_ = nullptr;
    SDL_GPUGraphicsPipeline* image_pipeline_ = nullptr;
    SDL_GPUTexture* image_atlas_texture_ = nullptr;
    SDL_GPUSampler* image_sampler_ = nullptr;
    Uint32 image_atlas_width_ = 0;
    Uint32 image_atlas_height_ = 0;
    Uint32 atlas_cursor_x_ = 0;
    Uint32 atlas_cursor_y_ = 0;
    Uint32 atlas_row_height_ = 0;

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
