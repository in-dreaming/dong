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

    // 获取 design units face（无像素缩放）
    FT_Face face = getOrCreateDesignUnitsFace(font_path);
    if (!face) {
        SDL_Log("TextShaper: failed to get design units face for '%s'", font_path.c_str());
        return false;
    }

    const uint32_t units_per_em = face->units_per_EM;
    if (units_per_em == 0) {
        SDL_Log("TextShaper: invalid units_per_em for '%s'", font_path.c_str());
        return false;
    }

    // 创建 HarfBuzz 字体，配置为 design units 空间
    hb_font_t* hb_font = hb_ft_font_create_referenced(face);
    if (!hb_font) {
        SDL_Log("TextShaper: failed to create hb_font for '%s'", font_path.c_str());
        return false;
    }

    // 关键：设置 HarfBuzz 字体缩放为 unitsPerEm（而非像素）
    hb_font_set_scale(hb_font, units_per_em, units_per_em);
    
    // 使用 FT funcs 但不设置像素尺寸
    hb_ft_font_set_funcs(hb_font);

    hb_buffer_t* buffer = hb_buffer_create();
    hb_buffer_add_utf8(buffer, request.text.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties(buffer);

    hb_shape(hb_font, buffer, nullptr, 0);

    unsigned glyph_count = 0;
    hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, &glyph_count);

    out_text.font_path = font_path;
    out_text.units_per_em = units_per_em;
    out_text.scale_to_pixels = request.font_size / static_cast<float>(units_per_em);
    out_text.glyphs.clear();
    out_text.glyphs.reserve(glyph_count);
    
    // 调试日志
    static int shape_count = 0;
    if (shape_count < 5) {
        SDL_Log("[TextShaper] Request #%d: font_size=%.1f units_per_em=%u scale=%.4f text='%s'",
               shape_count, request.font_size, units_per_em, out_text.scale_to_pixels,
               request.text.substr(0, 30).c_str());
        ++shape_count;
    }

    float pen_x_units = 0.0f;
    float pen_y_units = 0.0f;

    for (unsigned i = 0; i < glyph_count; ++i) {
        const hb_glyph_info_t& info = infos[i];
        const hb_glyph_position_t& pos = positions[i];

        // 这里我们将 HarfBuzz 的位置直接视为 design units：
        // hb_font_set_scale(hb_font, units_per_em, units_per_em) 之后，
        // pos.x_advance 等已经按字体 design units 缩放，无需再除以 64。
        const float x_offset_units = static_cast<float>(pos.x_offset);
        const float y_offset_units = static_cast<float>(pos.y_offset);
        const float x_advance_units = static_cast<float>(pos.x_advance);
        const float y_advance_units = static_cast<float>(pos.y_advance);

        ShapedGlyph glyph{};
        glyph.glyph_id = info.codepoint;
        glyph.pen_x_units = pen_x_units + x_offset_units;
        glyph.pen_y_units = pen_y_units - y_offset_units;
        out_text.glyphs.push_back(glyph);

        pen_x_units += x_advance_units;
        pen_y_units -= y_advance_units;
    }

    // 获取字体度量（design units）
    UnifiedFontMetrics font_metrics;
    if (getFontMetrics(face, font_metrics)) {
        out_text.ascent_units = font_metrics.ascent_units;
        out_text.descent_units = font_metrics.descent_units;
        out_text.line_height_units = font_metrics.height_units;
    }

    // 如果度量无效，使用默认值
    if (out_text.line_height_units <= 0.0f) {
        out_text.line_height_units = static_cast<float>(units_per_em) * kDefaultLineHeightMultiplier;
    }

    if (out_text.ascent_units <= 0.0f) {
        out_text.ascent_units = static_cast<float>(units_per_em);
    }

    out_text.width_units = pen_x_units;

    hb_buffer_destroy(buffer);
    hb_font_destroy(hb_font);
    return true;
}

} // namespace dong::render
