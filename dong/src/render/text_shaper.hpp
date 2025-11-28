#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dong::render {

struct TextShapeRequest {
    std::string text;
    std::string font_family;
    std::string font_weight;
    float font_size = 16.0f;
    float origin_x = 0.0f;
    float origin_y = 0.0f;
};

struct ShapedGlyph {
    uint32_t glyph_id = 0;
    float pen_x = 0.0f;
    float pen_y = 0.0f;
};

struct ShapedText {
    std::string font_path;
    float ascent = 0.0f;
    float descent = 0.0f;
    float line_height = 0.0f;
    float width = 0.0f;
    std::vector<ShapedGlyph> glyphs;
};

class TextShaper {
public:
    TextShaper() = default;
    ~TextShaper() = default;

    // 对给定文本做 shaping，将结果写入 out_text。
    bool shape(const TextShapeRequest& request, ShapedText& out_text);
};

} // namespace dong::render
