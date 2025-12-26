#include "text_shaper.hpp"

#include "font_metrics.hpp"
#include "font_resolver.hpp"

#include <SDL3/SDL_log.h>
#include <hb-ft.h>
#include <hb.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>

namespace dong::render {

namespace {

// Approximate CSS `line-height: normal` as ~1.2 * font-size.
constexpr float kDefaultLineHeightMultiplier = 1.2f;

// 从 UTF-8 字符串中提取指定位置的 Unicode 码点
// 返回码点值，并更新 byte_index 到下一个字符的起始位置
uint32_t extractCodepoint(const std::string& text, size_t byte_index) {
    if (byte_index >= text.size()) {
        return 0;
    }
    
    const uint8_t* p = reinterpret_cast<const uint8_t*>(text.data() + byte_index);
    uint32_t codepoint = 0;
    
    if ((p[0] & 0x80) == 0) {
        // 1-byte (ASCII)
        codepoint = p[0];
    } else if ((p[0] & 0xE0) == 0xC0) {
        // 2-byte
        codepoint = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
    } else if ((p[0] & 0xF0) == 0xE0) {
        // 3-byte
        codepoint = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
    } else if ((p[0] & 0xF8) == 0xF0) {
        // 4-byte
        codepoint = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | 
                    ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
    }
    
    return codepoint;
}

// 缓存的字体信息
struct CachedFontInfo {
    FT_Face face = nullptr;
    hb_font_t* hb_font = nullptr;
    uint32_t units_per_em = 0;
};

// 字体缓存（避免重复加载）
std::unordered_map<std::string, CachedFontInfo> g_font_cache;

// 获取或创建字体信息
CachedFontInfo* getOrCreateFontInfo(const std::string& font_path) {
    auto it = g_font_cache.find(font_path);
    if (it != g_font_cache.end()) {
        return &it->second;
    }
    
    FT_Face face = getOrCreateDesignUnitsFace(font_path);
    if (!face) {
        return nullptr;
    }
    
    hb_font_t* hb_font = hb_ft_font_create_referenced(face);
    if (!hb_font) {
        return nullptr;
    }
    
    uint32_t units_per_em = face->units_per_EM;
    hb_font_set_scale(hb_font, units_per_em, units_per_em);
    hb_ft_font_set_funcs(hb_font);
    
    CachedFontInfo info;
    info.face = face;
    info.hb_font = hb_font;
    info.units_per_em = units_per_em;
    
    g_font_cache[font_path] = info;
    return &g_font_cache[font_path];
}

// 对单个字符进行 shaping
bool shapeChar(const std::string& text, size_t byte_start, size_t byte_end,
               CachedFontInfo* font_info, const std::string& font_path,
               float& pen_x_units, float& pen_y_units,
               std::vector<ShapedGlyph>& out_glyphs) {
    if (!font_info || !font_info->hb_font) {
        return false;
    }
    
    hb_buffer_t* buffer = hb_buffer_create();
    hb_buffer_add_utf8(buffer, text.c_str() + byte_start, 
                       static_cast<int>(byte_end - byte_start), 0, -1);
    hb_buffer_guess_segment_properties(buffer);
    
    hb_shape(font_info->hb_font, buffer, nullptr, 0);
    
    unsigned glyph_count = 0;
    hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, &glyph_count);
    
    constexpr float kHbPosScale = 1.0f / 64.0f;
    
    for (unsigned i = 0; i < glyph_count; ++i) {
        const hb_glyph_info_t& info = infos[i];
        const hb_glyph_position_t& pos = positions[i];
        
        const float x_offset_units = static_cast<float>(pos.x_offset) * kHbPosScale;
        const float y_offset_units = static_cast<float>(pos.y_offset) * kHbPosScale;
        const float x_advance_units = static_cast<float>(pos.x_advance);
        const float y_advance_units = static_cast<float>(pos.y_advance);
        
        ShapedGlyph glyph{};
        glyph.glyph_id = info.codepoint;
        glyph.pen_x_units = pen_x_units + x_offset_units;
        glyph.pen_y_units = pen_y_units - y_offset_units;
        glyph.advance_x_units = x_advance_units;
        glyph.cluster = static_cast<uint32_t>(byte_start + info.cluster);
        glyph.font_path = font_path;
        glyph.units_per_em = font_info->units_per_em;
        out_glyphs.push_back(glyph);
        
        pen_x_units += x_advance_units;
        pen_y_units -= y_advance_units;
    }
    
    hb_buffer_destroy(buffer);
    return true;
}

// 获取 UTF-8 字符的字节长度
size_t utf8CharLen(uint8_t first_byte) {
    if ((first_byte & 0x80) == 0) return 1;
    if ((first_byte & 0xE0) == 0xC0) return 2;
    if ((first_byte & 0xF0) == 0xE0) return 3;
    if ((first_byte & 0xF8) == 0xF0) return 4;
    return 1; // 无效字节，跳过
}

} // namespace

bool TextShaper::shape(const TextShapeRequest& request, ShapedText& out_text) {
    out_text = {};

    if (request.text.empty()) {
        return false;
    }

    // 解析主字体
    std::string primary_font_path = resolveFontPath(request.font_family, request.font_weight);
    if (primary_font_path.empty()) {
        SDL_Log("TextShaper: failed to resolve font family '%s'", request.font_family.c_str());
        return false;
    }

    CachedFontInfo* primary_font = getOrCreateFontInfo(primary_font_path);
    if (!primary_font) {
        SDL_Log("TextShaper: failed to load primary font '%s'", primary_font_path.c_str());
        return false;
    }

    out_text.font_path = primary_font_path;
    out_text.units_per_em = primary_font->units_per_em;
    out_text.scale_to_pixels = request.font_size / static_cast<float>(primary_font->units_per_em);
    out_text.glyphs.clear();
    
    float pen_x_units = 0.0f;
    float pen_y_units = 0.0f;
    
    const std::string& text = request.text;
    size_t i = 0;
    
    // 预加载 CJK 回退字体列表
    std::vector<std::string> cjk_fallbacks = getCJKFallbackFonts();
    
    while (i < text.size()) {
        size_t char_len = utf8CharLen(static_cast<uint8_t>(text[i]));
        size_t char_end = std::min(i + char_len, text.size());
        uint32_t codepoint = extractCodepoint(text, i);
        
        // 检查主字体是否支持该字符
        FT_UInt glyph_index = FT_Get_Char_Index(primary_font->face, codepoint);
        
        std::string font_to_use = primary_font_path;
        CachedFontInfo* font_info = primary_font;
        
        if (glyph_index == 0 && codepoint > 127) {
            // 主字体不支持，尝试回退字体
            for (const auto& fallback_path : cjk_fallbacks) {
                CachedFontInfo* fallback_font = getOrCreateFontInfo(fallback_path);
                if (fallback_font) {
                    FT_UInt fallback_glyph = FT_Get_Char_Index(fallback_font->face, codepoint);
                    if (fallback_glyph != 0) {
                        font_to_use = fallback_path;
                        font_info = fallback_font;
                        break;
                    }
                }
            }
        }
        
        // 对该字符进行 shaping
        shapeChar(text, i, char_end, font_info, font_to_use,
                  pen_x_units, pen_y_units, out_text.glyphs);
        
        i = char_end;
    }

    // 获取主字体度量
    UnifiedFontMetrics font_metrics;
    if (getFontMetrics(primary_font->face, font_metrics)) {
        out_text.ascent_units = font_metrics.ascent_units;
        out_text.descent_units = font_metrics.descent_units;
        out_text.line_height_units = font_metrics.height_units;
    }

    if (out_text.line_height_units <= 0.0f) {
        out_text.line_height_units = static_cast<float>(primary_font->units_per_em) * kDefaultLineHeightMultiplier;
    }

    if (out_text.ascent_units <= 0.0f) {
        out_text.ascent_units = static_cast<float>(primary_font->units_per_em);
    }

    out_text.width_units = pen_x_units;

    return true;
}

} // namespace dong::render
