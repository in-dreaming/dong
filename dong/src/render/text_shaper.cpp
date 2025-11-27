#include "text_shaper.hpp"

#include "font_resolver.hpp"

#include <SDL3/SDL_log.h>
#include <hb-ft.h>
#include <hb.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace dong::render {

namespace {

constexpr float kDefaultLineHeightMultiplier = 1.35f;

uint32_t clampPixelSize(float font_size) {
    float clamped = std::max(1.0f, font_size);
    return static_cast<uint32_t>(std::ceil(clamped));
}

} // namespace

TextShaper::TextShaper() {
    initialize();
}

TextShaper::~TextShaper() {
    for (auto& entry : face_cache_) {
        if (entry.second) {
            FT_Done_Face(entry.second);
        }
    }
    face_cache_.clear();

    if (ft_library_) {
        FT_Done_FreeType(ft_library_);
        ft_library_ = nullptr;
    }
}

bool TextShaper::initialize() {
    if (initialized_) {
        return true;
    }
    if (FT_Init_FreeType(&ft_library_) != 0) {
        SDL_Log("TextShaper: failed to initialize FreeType");
        ft_library_ = nullptr;
        initialized_ = false;
        return false;
    }
    initialized_ = true;
    return true;
}

bool TextShaper::ensureInitialized() {
    if (initialized_) {
        return true;
    }
    return initialize();
}

FT_Face TextShaper::getOrCreateFace(const std::string& font_path, uint32_t pixel_size) {
    if (!ensureInitialized() || font_path.empty()) {
        return nullptr;
    }

    std::string key = font_path + "#" + std::to_string(pixel_size);
    auto it = face_cache_.find(key);
    if (it != face_cache_.end()) {
        return it->second;
    }

    FT_Face face = nullptr;
    if (FT_New_Face(ft_library_, font_path.c_str(), 0, &face) != 0) {
        SDL_Log("TextShaper: failed to load font face '%s'", font_path.c_str());
        return nullptr;
    }

    FT_Set_Pixel_Sizes(face, 0, pixel_size);
    face_cache_[key] = face;
    return face;
}

bool TextShaper::shape(const TextShapeRequest& request, ShapedText& out_text) {
    out_text = {};

    if (request.text.empty()) {
        return false;
    }

    if (!ensureInitialized()) {
        return false;
    }

    std::string font_path = resolveFontPath(request.font_family);
    if (font_path.empty()) {
        SDL_Log("TextShaper: failed to resolve font family '%s'", request.font_family.c_str());
        return false;
    }

    uint32_t pixel_size = clampPixelSize(request.font_size);
    FT_Face face = getOrCreateFace(font_path, pixel_size);
    if (!face) {
        return false;
    }

    hb_font_t* hb_font = hb_ft_font_create_referenced(face);
    if (!hb_font) {
        SDL_Log("TextShaper: failed to create hb_font for '%s'", font_path.c_str());
        return false;
    }

    const int32_t scale = static_cast<int32_t>(std::round(request.font_size * 64.0f));
    hb_font_set_scale(hb_font, scale, scale);

    hb_ft_font_set_load_flags(hb_font, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
    hb_ft_font_set_funcs(hb_font);

    hb_buffer_t* buffer = hb_buffer_create();
    hb_buffer_add_utf8(buffer, request.text.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties(buffer);

    hb_shape(hb_font, buffer, nullptr, 0);

    unsigned glyph_count = 0;
    hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, &glyph_count);

    out_text.font_path = font_path;
    out_text.glyphs.clear();
    out_text.glyphs.reserve(glyph_count);

    float pen_x = request.origin_x;
    float pen_y = request.origin_y;

    for (unsigned i = 0; i < glyph_count; ++i) {
        const hb_glyph_info_t& info = infos[i];
        const hb_glyph_position_t& pos = positions[i];

        float x_offset = pos.x_offset / 64.0f;
        float y_offset = pos.y_offset / 64.0f;
        float x_advance = pos.x_advance / 64.0f;
        float y_advance = pos.y_advance / 64.0f;

        ShapedGlyph glyph{};
        glyph.glyph_id = info.codepoint;
        glyph.pen_x = pen_x + x_offset;
        glyph.pen_y = pen_y - y_offset;
        out_text.glyphs.push_back(glyph);

        pen_x += x_advance;
        pen_y -= y_advance;
    }

    if (face->size) {
        out_text.ascent = static_cast<float>(face->size->metrics.ascender) / 64.0f;
        out_text.descent = std::fabs(static_cast<float>(face->size->metrics.descender) / 64.0f);
        out_text.line_height = static_cast<float>(face->size->metrics.height) / 64.0f;
    }

    if (out_text.line_height <= 0.0f) {
        out_text.line_height = request.font_size * kDefaultLineHeightMultiplier;
    }

    if (out_text.ascent <= 0.0f) {
        out_text.ascent = request.font_size;
    }

    out_text.width = pen_x - request.origin_x;

    hb_buffer_destroy(buffer);
    hb_font_destroy(hb_font);
    return true;
}

} // namespace dong::render
