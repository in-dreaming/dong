#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace dong::render {

struct TextShapeRequest {
    std::string text;
    std::string font_family;
    std::string font_weight;
    std::string font_style;        // CSS font-style: normal/italic/oblique
    float font_size = 16.0f;       // 目标像素字号

    float origin_x = 0.0f;
    float origin_y = 0.0f;
};

struct ShapedGlyph {
    uint32_t glyph_id = 0;
    float pen_x_units = 0.0f;       // design units，基线起点 x
    float pen_y_units = 0.0f;       // design units，基线起点 y
    float advance_x_units = 0.0f;   // design units，水平方向 advance
    uint32_t cluster = 0;           // 输入文本中的 UTF-8 字节偏移
    uint16_t font_path_index = 0;   // 指向 ShapedText::font_paths 的索引（支持字体回退）
    uint32_t units_per_em = 0;      // 该字体的 units_per_em
};

struct ShapedText {
    // 字体路径表：索引 0 为主字体；后续为回退字体。
    std::vector<std::string> font_paths;

    // 为了兼容旧代码：主字体路径（等价于 font_paths[0]）。
    std::string font_path;

    uint32_t units_per_em = 0;      // 主字体的 EM 单位
    float scale_to_pixels = 1.0f;   // font_size / units_per_em（用于下游缩放）
    
    float ascent_units = 0.0f;     // design units
    float descent_units = 0.0f;    // design units
    float line_height_units = 0.0f;// design units
    float width_units = 0.0f;      // design units
    
    std::vector<ShapedGlyph> glyphs;
};

class TextShaper {
public:
    TextShaper() = default;
    ~TextShaper() = default;

    // 对给定文本做 shaping，将结果写入 out_text（design units 空间）
    bool shape(const TextShapeRequest& request, ShapedText& out_text);
};

// 优化策略4：文本测量缓存
// 用于缓存 layout 阶段的文本宽度/高度测量结果，避免重复 shaping
struct TextMeasureCacheKey {
    std::string text;
    std::string font_family;
    std::string font_weight;
    std::string font_style;
    float font_size;
    float letter_spacing_em;
    float word_spacing_px;
    
    bool operator==(const TextMeasureCacheKey& other) const {
        return text == other.text &&
               font_family == other.font_family &&
               font_weight == other.font_weight &&
               font_style == other.font_style &&
               font_size == other.font_size &&
               letter_spacing_em == other.letter_spacing_em &&
               word_spacing_px == other.word_spacing_px;
    }
};

struct TextMeasureCacheKeyHash {
    std::size_t operator()(const TextMeasureCacheKey& k) const {
        std::size_t h = std::hash<std::string>{}(k.text);
        h ^= std::hash<std::string>{}(k.font_family) << 1;
        h ^= std::hash<std::string>{}(k.font_weight) << 2;
        h ^= std::hash<std::string>{}(k.font_style) << 3;
        h ^= std::hash<float>{}(k.font_size) << 4;
        h ^= std::hash<float>{}(k.letter_spacing_em) << 5;
        h ^= std::hash<float>{}(k.word_spacing_px) << 6;
        return h;
    }
};

struct TextMeasureResult {
    float content_width_px = 0.0f;
    float line_height_px = 0.0f;
    float baseline_from_top_px = 0.0f;
    float ascent_units = 0.0f;
    float descent_units = 0.0f;
    float line_height_units = 0.0f;
    float scale_to_pixels = 1.0f;
    size_t glyph_count = 0;
    int space_count = 0;
    bool valid = false;
};

class TextMeasureCache {
public:
    static TextMeasureCache& instance() {
        static TextMeasureCache cache;
        return cache;
    }
    
    // 查找缓存，如果命中返回 true 并填充 result
    bool lookup(const TextMeasureCacheKey& key, TextMeasureResult& result) const {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            result = it->second;
            ++hit_count_;
            return true;
        }
        ++miss_count_;
        return false;
    }
    
    // 插入缓存
    void insert(const TextMeasureCacheKey& key, const TextMeasureResult& result) {
        // 简单的 LRU 策略：如果缓存太大，清空一半
        if (cache_.size() > max_entries_) {
            cache_.clear();  // 简单起见直接清空，可以改成真正的 LRU
        }
        cache_[key] = result;
    }
    
    // 清空缓存
    void clear() {
        cache_.clear();
    }
    
    // 统计信息
    size_t size() const { return cache_.size(); }
    size_t hitCount() const { return hit_count_; }
    size_t missCount() const { return miss_count_; }
    float hitRate() const {
        size_t total = hit_count_ + miss_count_;
        return total > 0 ? static_cast<float>(hit_count_) / total : 0.0f;
    }
    
    void setMaxEntries(size_t max) { max_entries_ = max; }
    
private:
    TextMeasureCache() = default;
    
    std::unordered_map<TextMeasureCacheKey, TextMeasureResult, TextMeasureCacheKeyHash> cache_;
    size_t max_entries_ = 10000;
    mutable size_t hit_count_ = 0;
    mutable size_t miss_count_ = 0;
};

} // namespace dong::render
