#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dong::render {

struct TextShapeRequest {
    std::string text;
    std::string font_family;
    std::string font_weight;
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
    std::string font_path;          // 该 glyph 使用的字体路径（支持字体回退）
    uint32_t units_per_em = 0;      // 该字体的 units_per_em
};

struct ShapedText {
    std::string font_path;          // 主字体路径（可能部分 glyph 使用回退字体）
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

} // namespace dong::render
