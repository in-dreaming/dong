#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <SDL3/SDL_gpu.h>

namespace dong {
namespace render {

// 字形度量信息（design units 空间）
struct GlyphMetrics {
    float advance_x_units = 0.0f;
    float bearing_x_units = 0.0f;
    float bearing_y_units = 0.0f;
    float width_units = 0.0f;
    float height_units = 0.0f;
    float msdf_scale = 1.0f;
    float msdf_translate_x = 0.0f;
    float msdf_translate_y = 0.0f;
    float bounds_left = 0.0f;
    float bounds_bottom = 0.0f;
    float bounds_right = 0.0f;
    float bounds_top = 0.0f;
    float logical_left = 0.0f;
    float logical_bottom = 0.0f;
    float logical_right = 0.0f;
    float logical_top = 0.0f;
    uint32_t units_per_em = 0;
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
    bool initialize(uint32_t width = 2048,
                    uint32_t height = 2048,
                    uint32_t glyph_bitmap_size = 64,
                    float glyph_distance_range = 8.0f);

    // 查询字形是否已缓存
    const AtlasEntry* getGlyph(uint32_t glyph_id, const std::string& font_path);

    // 添加字形到 Atlas（生成 MSDF 并上传）
    const AtlasEntry* addGlyph(uint32_t glyph_id, const std::string& font_path);

    // 批量添加字形到 Atlas（单次 GPU 同步）
    struct GlyphRequest {
        uint32_t glyph_id;
        std::string font_path;
    };
    void addGlyphsBatched(const std::vector<GlyphRequest>& requests);

    // 获取 Atlas GPU 纹理（用于绑定到 shader）
    SDL_GPUTexture* getAtlasTexture() const;

    // 按页获取 Atlas 纹理；若页索引越界或纹理不存在则返回 nullptr。
    SDL_GPUTexture* getAtlasTextureForPage(uint32_t page_index) const;

    // 当前已分配的页数
    uint32_t getPageCount() const { return static_cast<uint32_t>(pages_.size()); }

    // 获取 Atlas 页的逻辑尺寸（所有页共享相同尺寸）
    uint32_t getWidth() const { return atlas_width_; }
    uint32_t getHeight() const { return atlas_height_; }

    uint32_t getGlyphBitmapSize() const { return glyph_bitmap_size_; }
    float getGlyphDistanceRange() const { return glyph_distance_range_; }

private:
    struct AtlasPage {
        SDL_GPUTexture* texture = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t cursor_x = 0;
        uint32_t cursor_y = 0;
        uint32_t row_height = 0;
        uint32_t page_index = 0;
        uint32_t glyph_count = 0;
        uint64_t last_used = 0;
    };

    struct PendingUpload {
        SDL_GPUFence* fence = nullptr;
        std::vector<SDL_GPUTransferBuffer*> buffers;
    };

    GPUDevice* gpu_device_ = nullptr;

    uint32_t atlas_width_ = 2048;
    uint32_t atlas_height_ = 2048;
    uint32_t glyph_bitmap_size_ = 64;
    float glyph_distance_range_ = 8.0f;

    std::vector<AtlasPage> pages_;
    uint32_t max_pages_ = 4;
    uint64_t usage_counter_ = 0;

    std::unordered_map<std::string, AtlasEntry> cache_;
    std::unordered_map<uint32_t, std::vector<std::string> > page_to_keys_;
    std::vector<PendingUpload> pending_uploads_;

    std::string makeGlyphKey(uint32_t codepoint, const std::string& font_path) const;
    void reapPendingUploads(SDL_GPUDevice* dev);
    void waitAllPendingUploads(SDL_GPUDevice* dev);
    bool createPage();
    AtlasPage* selectPageForGlyph(uint32_t glyph_width, uint32_t glyph_height);
    AtlasPage* evictAndRecyclePage();
    bool generateMSDF(uint32_t codepoint, const std::string& font_path,
                     std::vector<uint8_t>& out_bitmap,
                     uint32_t& out_width, uint32_t& out_height,
                     GlyphMetrics& out_metrics);
};

} // namespace render
} // namespace dong
