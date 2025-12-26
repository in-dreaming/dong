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

// Approximate CSS `line-height: normal` as ~1.2 * font-size.
constexpr float kDefaultLineHeightMultiplier = 1.2f;

} // namespace

bool TextShaper::shape(const TextShapeRequest& request, ShapedText& out_text) {
    out_text = {};

    if (request.text.empty()) {
        return false;
    }

    std::string font_path = resolveFontPath(request.font_family, request.font_weight);
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
    float pen_x_units = 0.0f;
    float pen_y_units = 0.0f;

    // 注意：当 hb_font_set_scale 设置为 units_per_em 时，
    // HarfBuzz 返回的 x_advance/y_advance 已经是 design units，不需要再除以 64。
    // 只有 x_offset/y_offset 仍然是 26.6 fixed point 格式。
    constexpr float kHbPosScale = 1.0f / 64.0f; // 仅用于 offset

    for (unsigned i = 0; i < glyph_count; ++i) {
        const hb_glyph_info_t& info = infos[i];
        const hb_glyph_position_t& pos = positions[i];

        // x_offset/y_offset 是 26.6 fixed point，需要除以 64
        const float x_offset_units = static_cast<float>(pos.x_offset) * kHbPosScale;
        const float y_offset_units = static_cast<float>(pos.y_offset) * kHbPosScale;
        
        // x_advance/y_advance 在 hb_font_set_scale 设置为 units_per_em 后，
        // 直接就是 design units，不需要除以 64
        const float x_advance_units = static_cast<float>(pos.x_advance);
        const float y_advance_units = static_cast<float>(pos.y_advance);

        ShapedGlyph glyph{};
        glyph.glyph_id = info.codepoint;
        glyph.pen_x_units = pen_x_units + x_offset_units;
        glyph.pen_y_units = pen_y_units - y_offset_units;
        glyph.advance_x_units = x_advance_units;
        glyph.cluster = info.cluster; // UTF-8 字节偏移
        out_text.glyphs.push_back(glyph);

        pen_x_units += x_advance_units;
        pen_y_units -= y_advance_units;
    }


    // 获取字体度量（design units），并直接使用 OS/2 提供的 height_units
    // 作为行高基础（接近现代浏览器对 `line-height: normal` 的实现），避免
    // 自己拍一个常数放大 EM 导致额外的空白。
    UnifiedFontMetrics font_metrics;
    if (getFontMetrics(face, font_metrics)) {
        out_text.ascent_units = font_metrics.ascent_units;
        out_text.descent_units = font_metrics.descent_units;
        out_text.line_height_units = font_metrics.height_units;
    }

    // 如果 height_units 无效，再退回到基于 EM 的默认倍数。
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
