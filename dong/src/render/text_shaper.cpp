#include "text_shaper.hpp"

#include "font_metrics.hpp"
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

bool TextShaper::shape(const TextShapeRequest& request, ShapedText& out_text) {
    out_text = {};

    if (request.text.empty()) {
        return false;
    }

    std::string font_path = resolveFontPath(request.font_family);
    if (font_path.empty()) {
        SDL_Log("TextShaper: failed to resolve font family '%s'", request.font_family.c_str());
        return false;
    }

    const uint32_t pixel_size = clampPixelSize(request.font_size);
    FT_Face face = getOrCreateFontFace(font_path, pixel_size);
    if (!face) {
        SDL_Log("TextShaper: failed to get FT_Face for '%s'", font_path.c_str());
        return false;
    }

    // 在像素空间下 shape：HarfBuzz 返回的 26.6 定点值直接代表像素坐标
    hb_font_t* hb_font = hb_ft_font_create_referenced(face);
    if (!hb_font) {
        SDL_Log("TextShaper: failed to create hb_font for '%s'", font_path.c_str());
        return false;
    }

    // 使用无 hinting / 无 bitmap，但保持缩放，让 HarfBuzz 给出像素级几何
    hb_ft_font_set_load_flags(hb_font,
                              FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
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

        // HarfBuzz 返回的位置信息是 26.6 定点数（像素单位），直接转为像素
        const float x_offset_px = static_cast<float>(pos.x_offset) / 64.0f;
        const float y_offset_px = static_cast<float>(pos.y_offset) / 64.0f;
        const float x_advance_px = static_cast<float>(pos.x_advance) / 64.0f;
        const float y_advance_px = static_cast<float>(pos.y_advance) / 64.0f;

        ShapedGlyph glyph{};
        glyph.glyph_id = info.codepoint;
        glyph.pen_x = pen_x + x_offset_px;
        glyph.pen_y = pen_y - y_offset_px;
        out_text.glyphs.push_back(glyph);

        pen_x += x_advance_px;
        pen_y -= y_advance_px;
    }

    // 使用 FreeType size->metrics 的像素度量
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
