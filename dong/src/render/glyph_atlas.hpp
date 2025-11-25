#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <SDL3/SDL_gpu.h>

namespace dong::render {

// 字形度量信息
struct GlyphMetrics {
    float advance_x = 0.0f;
    float bearing_x = 0.0f;
    float bearing_y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

// Atlas 条目（UV 坐标 + 度量）
struct AtlasEntry {
    uint32_t atlas_page = 0;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
    GlyphMetrics metrics;
};

class GPUDevice;

// MSDF 字形 Atlas 管理器
class GlyphAtlas {
public:
    explicit GlyphAtlas(GPUDevice* gpu_device);
    ~GlyphAtlas();

    // 初始化 Atlas（创建 GPU 纹理）
    bool initialize(uint32_t width = 2048, uint32_t height = 2048);

    // 查询字形是否已缓存
    const AtlasEntry* getGlyph(uint32_t codepoint);

    // 添加字形到 Atlas（生成 MSDF 并上传）
    const AtlasEntry* addGlyph(uint32_t codepoint, const std::string& font_path);

    // 获取 Atlas GPU 纹理（用于绑定到 shader）
    SDL_GPUTexture* getAtlasTexture() const { return atlas_texture_; }

    // 获取 Atlas 尺寸
    uint32_t getWidth() const { return atlas_width_; }
    uint32_t getHeight() const { return atlas_height_; }

private:
    GPUDevice* gpu_device_ = nullptr;
    SDL_GPUTexture* atlas_texture_ = nullptr;

    uint32_t atlas_width_ = 2048;
    uint32_t atlas_height_ = 2048;

    // 简单装箱：行优先
    uint32_t cursor_x_ = 0;
    uint32_t cursor_y_ = 0;
    uint32_t row_height_ = 0;

    // 缓存：codepoint -> AtlasEntry
    std::unordered_map<uint32_t, AtlasEntry> cache_;

    // 内部辅助：为指定字符生成 MSDF 位图
    bool generateMSDF(uint32_t codepoint, const std::string& font_path,
                     std::vector<uint8_t>& out_bitmap,
                     uint32_t& out_width, uint32_t& out_height,
                     GlyphMetrics& out_metrics);
};

} // namespace dong::render
