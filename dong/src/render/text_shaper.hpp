#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

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
    TextShaper();
    ~TextShaper();

    bool initialize();
    bool shape(const TextShapeRequest& request, ShapedText& out_text);

private:
    bool ensureInitialized();
    FT_Face getOrCreateFace(const std::string& font_path, uint32_t pixel_size);

    FT_Library ft_library_ = nullptr;
    std::unordered_map<std::string, FT_Face> face_cache_;
    bool initialized_ = false;
};

} // namespace dong::render
