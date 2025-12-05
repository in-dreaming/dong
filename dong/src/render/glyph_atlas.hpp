#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <SDL3/SDL_gpu.h>

namespace dong::render {

// 字形度量信息（design units 空间）
struct GlyphMetrics {
    float advance_x_units = 0.0f;
    float bearing_x_units = 0.0f;
    float bearing_y_units = 0.0f;
    float width_units = 0.0f;
    float height_units = 0.0f;
    
    // MSDF 生成元数据（字号无关）
    float msdf_scale = 1.0f;
    float msdf_translate_x = 0.0f;
    float msdf_translate_y = 0.0f;
    
    // 字形轮廓的实际边界（design units，用于 MSDF 投影）
    // 这些值来自 msdfgen::Shape::getBounds()，可能与 FreeType metrics 略有不同
    float bounds_left = 0.0f;
    float bounds_bottom = 0.0f;
    float bounds_right = 0.0f;
    float bounds_top = 0.0f;
    
    // 逻辑 bbox（design units，基线坐标系）
    // 这是用于排版和渲染的统一坐标系：
    // - baseline 在 y=0
    // - logical_top > 0 表示字形顶部在基线上方
    // - logical_bottom < 0 表示字形底部在基线下方（descender）
    // - logical_left 通常 >= 0，表示字形左边缘相对于 pen position
    // - logical_right 表示字形右边缘相对于 pen position
    //
    // 这些值直接用于计算屏幕空间矩形：
    // screen_top = baseline_y - logical_top * scale
    // screen_bottom = baseline_y - logical_bottom * scale
    // screen_left = pen_x + logical_left * scale
    // screen_right = pen_x + logical_right * scale
    float logical_left = 0.0f;
    float logical_bottom = 0.0f;
    float logical_right = 0.0f;
    float logical_top = 0.0f;
    
    uint32_t units_per_em = 0;     // 字体的 EM 单位
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

    // 获取 Atlas GPU 纹理（用于绑定到 shader）
    // 为了兼容旧代码，此接口返回第 0 页的纹理；
    // 新代码应优先使用 getAtlasTextureForPage/ getPageCount 组合。
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

    GPUDevice* gpu_device_ = nullptr;

    uint32_t atlas_width_ = 2048;
    uint32_t atlas_height_ = 2048;

    uint32_t glyph_bitmap_size_ = 64;
    float glyph_distance_range_ = 8.0f;

    // 页集合与总体容量限制
    std::vector<AtlasPage> pages_;
    uint32_t max_pages_ = 4;
    uint64_t usage_counter_ = 0;

    // 缓存：font_path#glyph_id -> AtlasEntry
    std::unordered_map<std::string, AtlasEntry> cache_;

    // 反向索引：page_index -> 该页上承载的 glyph key 列表
    std::unordered_map<uint32_t, std::vector<std::string> > page_to_keys_;

    std::string makeGlyphKey(uint32_t codepoint, const std::string& font_path) const;

    // 页管理
    bool createPage();
    AtlasPage* selectPageForGlyph(uint32_t glyph_width, uint32_t glyph_height);
    AtlasPage* evictAndRecyclePage();

    // 内部辅助：为指定字符生成 MSDF 位图（design units 模式）
    bool generateMSDF(uint32_t codepoint, const std::string& font_path,
                     std::vector<uint8_t>& out_bitmap,
                     uint32_t& out_width, uint32_t& out_height,
                     GlyphMetrics& out_metrics);
};

} // namespace dong::render
