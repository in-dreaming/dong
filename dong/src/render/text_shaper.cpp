#include "text_shaper.hpp"

#include "font_metrics.hpp"
#include "font_resolver.hpp"

#include <hb-ft.h>
#include <hb.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>
#include "../core/log.h"

namespace dong::render {

namespace {

// Approximate CSS `line-height: normal` as ~1.2 * font-size.
constexpr float kDefaultLineHeightMultiplier = 1.2f;

// 浠?UTF-8 瀛楃涓蹭腑鎻愬彇鎸囧畾浣嶇疆鐨?Unicode 鐮佺偣
// 杩斿洖鐮佺偣鍊硷紝骞舵洿鏂?byte_index 鍒颁笅涓€涓瓧绗︾殑璧峰浣嶇疆
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

// 缂撳瓨鐨勫瓧浣撲俊鎭?
struct CachedFontInfo {
    FT_Face face = nullptr;
    hb_font_t* hb_font = nullptr;
    uint32_t units_per_em = 0;
};

// 瀛椾綋缂撳瓨锛堥伩鍏嶉噸澶嶅姞杞斤級
std::unordered_map<std::string, CachedFontInfo> g_font_cache;

// 鑾峰彇鎴栧垱寤哄瓧浣撲俊鎭?
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

// 瀵瑰崟涓瓧绗﹁繘琛?shaping
// primary_units_per_em: 涓诲瓧浣撶殑 units_per_em锛岀敤浜庣粺涓€鍧愭爣绯?
bool shapeChar(const std::string& text, size_t byte_start, size_t byte_end,
               CachedFontInfo* font_info, const std::string& font_path,
               uint32_t primary_units_per_em,
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
    
    // HarfBuzz 鐨?position 鍊奸渶瑕佹牴鎹瓧浣撶殑 scale 璁剧疆鏉ヨВ閲?
    // 鎴戜滑鍦?hb_font_set_scale 涓缃簡 (units_per_em, units_per_em)
    // 鎵€浠ヨ繑鍥炵殑鍊煎凡缁忔槸 design units
    
    // 褰撲娇鐢ㄥ洖閫€瀛椾綋鏃讹紝闇€瑕佸皢 advance 浠庡洖閫€瀛椾綋鐨?units 杞崲涓轰富瀛椾綋鐨?units
    // 杩欐牱鎵€鏈夊瓧绗︾殑浣嶇疆閮藉湪鍚屼竴涓潗鏍囩郴涓?
    const float units_scale = (font_info->units_per_em > 0 && primary_units_per_em > 0)
        ? static_cast<float>(primary_units_per_em) / static_cast<float>(font_info->units_per_em)
        : 1.0f;
    
    for (unsigned i = 0; i < glyph_count; ++i) {
        const hb_glyph_info_t& info = infos[i];
        const hb_glyph_position_t& pos = positions[i];
        
        // HarfBuzz 杩斿洖鐨勫€煎凡缁忔槸 design units锛堝洜涓烘垜浠缃簡 scale = units_per_em锛?
        const float x_offset_units = static_cast<float>(pos.x_offset) * units_scale;
        const float y_offset_units = static_cast<float>(pos.y_offset) * units_scale;
        const float x_advance_units = static_cast<float>(pos.x_advance) * units_scale;
        const float y_advance_units = static_cast<float>(pos.y_advance) * units_scale;
        
        // Debug: check for abnormal values
        if (x_advance_units > 100000.0f || x_advance_units < -100000.0f) {
            DONG_LOG_INFO("[shapeChar] WARN: glyph_id=%u x_advance_units=%.1f (raw=%d scale=%.3f)",
                    info.codepoint, x_advance_units, pos.x_advance, units_scale);
        }
        
        ShapedGlyph glyph{};
        glyph.glyph_id = info.codepoint;
        glyph.pen_x_units = pen_x_units + x_offset_units;
        glyph.pen_y_units = pen_y_units - y_offset_units;
        glyph.advance_x_units = x_advance_units;
        glyph.cluster = static_cast<uint32_t>(byte_start + info.cluster);
        glyph.font_path = font_path;
        glyph.units_per_em = font_info->units_per_em;  // 淇濈暀鍘熷瀛椾綋鐨?units_per_em
        out_glyphs.push_back(glyph);
        
        pen_x_units += x_advance_units;
        pen_y_units -= y_advance_units;
    }
    
    hb_buffer_destroy(buffer);
    return true;
}

// 鑾峰彇 UTF-8 瀛楃鐨勫瓧鑺傞暱搴?
size_t utf8CharLen(uint8_t first_byte) {
    if ((first_byte & 0x80) == 0) return 1;
    if ((first_byte & 0xE0) == 0xC0) return 2;
    if ((first_byte & 0xF0) == 0xE0) return 3;
    if ((first_byte & 0xF8) == 0xF0) return 4;
    return 1; // 鏃犳晥瀛楄妭锛岃烦杩?
}

} // namespace

bool TextShaper::shape(const TextShapeRequest& request, ShapedText& out_text) {
    out_text = {};

    if (request.text.empty()) {
        return false;
    }

    // 解析主字体（需要考虑 font-style：italic/oblique）
    std::string primary_font_path = resolveFontPath(request.font_family, request.font_weight, request.font_style);
    if (primary_font_path.empty()) {
        DONG_LOG_INFO("TextShaper: failed to resolve font family '%s'", request.font_family.c_str());
        return false;
    }

    CachedFontInfo* primary_font = getOrCreateFontInfo(primary_font_path);
    if (!primary_font) {
        DONG_LOG_INFO("TextShaper: failed to load primary font '%s'", primary_font_path.c_str());
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
    
    // 棰勫姞杞?CJK 鍥為€€瀛椾綋鍒楄〃
    std::vector<std::string> cjk_fallbacks = getCJKFallbackFonts();
    
    while (i < text.size()) {
        size_t char_len = utf8CharLen(static_cast<uint8_t>(text[i]));
        size_t char_end = std::min(i + char_len, text.size());
        uint32_t codepoint = extractCodepoint(text, i);
        
        // 妫€鏌ヤ富瀛椾綋鏄惁鏀寔璇ュ瓧绗?
        FT_UInt glyph_index = FT_Get_Char_Index(primary_font->face, codepoint);
        
        std::string font_to_use = primary_font_path;
        CachedFontInfo* font_info = primary_font;
        
        if (glyph_index == 0 && codepoint > 127) {
            // 涓诲瓧浣撲笉鏀寔锛屽皾璇曞洖閫€瀛椾綋
            for (const auto& fallback_path : cjk_fallbacks) {
                CachedFontInfo* fallback_font = getOrCreateFontInfo(fallback_path);
                if (fallback_font) {
                    FT_UInt fallback_glyph = FT_Get_Char_Index(fallback_font->face, codepoint);
                    if (fallback_glyph != 0) {
                        DONG_LOG_DEBUG("[TextShaper] Fallback for U+%04X: '%s' (units_per_em=%u, primary=%u)",
                                codepoint, fallback_path.c_str(), 
                                fallback_font->units_per_em, primary_font->units_per_em);
                        font_to_use = fallback_path;
                        font_info = fallback_font;
                        break;
                    }
                }
            }
        }
        
        // 瀵硅瀛楃杩涜 shaping锛屼紶鍏ヤ富瀛椾綋鐨?units_per_em 浠ョ粺涓€鍧愭爣绯?
        shapeChar(text, i, char_end, font_info, font_to_use,
                  primary_font->units_per_em,
                  pen_x_units, pen_y_units, out_text.glyphs);
        
        i = char_end;
    }

    // 鑾峰彇涓诲瓧浣撳害閲?
    UnifiedFontMetrics font_metrics;
    if (getFontMetrics(primary_font->face, font_metrics)) {
        out_text.ascent_units = font_metrics.ascent_units;
        out_text.descent_units = font_metrics.descent_units;
        out_text.line_height_units = font_metrics.height_units;
        
        DONG_LOG_DEBUG("[TextShaper] font='%s' units_per_em=%u ascent=%.1f descent=%.1f line_height=%.1f",
                primary_font_path.c_str(),
                font_metrics.units_per_em,
                font_metrics.ascent_units,
                font_metrics.descent_units,
                font_metrics.height_units);
    }

    if (out_text.line_height_units <= 0.0f) {
        out_text.line_height_units = static_cast<float>(primary_font->units_per_em) * kDefaultLineHeightMultiplier;
    }

    if (out_text.ascent_units <= 0.0f) {
        out_text.ascent_units = static_cast<float>(primary_font->units_per_em);
    }

    out_text.width_units = pen_x_units;
    
    DONG_LOG_DEBUG("[TextShaper] result: width_units=%.1f line_height_units=%.1f scale=%.6f",
            out_text.width_units, out_text.line_height_units, out_text.scale_to_pixels);

    return true;
}

} // namespace dong::render
