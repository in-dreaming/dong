#include "font_metrics.hpp"
#include "../core/log.h"

#include <cmath>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_TRUETYPE_TABLES_H

namespace dong::render {

namespace {

FT_Library g_ft_library = nullptr;
bool g_ft_initialized = false;

// 鏂扮紦瀛橈細design units face锛坘ey: font_path锛?
std::unordered_map<std::string, FT_Face> g_design_face_cache;

// 鏃х紦瀛橈細鍍忕礌 face锛坘ey: font_path#pixel_size锛岀敤浜庤繃娓℃湡锛?
std::unordered_map<std::string, FT_Face> g_pixel_face_cache;

// 绠€鍗?LRU锛氳褰曟渶杩戜娇鐢ㄥ抚娆℃垨璁℃暟
std::unordered_map<std::string, uint64_t> g_design_face_last_used;
std::unordered_map<std::string, uint64_t> g_pixel_face_last_used;
uint64_t g_face_usage_counter = 0;

constexpr std::size_t kMaxDesignFaces = 32;
constexpr std::size_t kMaxPixelFaces = 64;

std::string makePixelFaceKey(const std::string& font_path, uint32_t pixel_size) {
    std::string key = font_path;
    key.push_back('#');
    key.append(std::to_string(pixel_size));
    return key;
}

bool ensureLibraryInitialized() {
    if (g_ft_initialized) {
        return g_ft_library != nullptr;
    }

    if (FT_Init_FreeType(&g_ft_library) != 0) {
        DONG_LOG_INFO("FontMetrics: failed to initialize FreeType library");
        g_ft_library = nullptr;
        g_ft_initialized = true;
        return false;
    }

    // 设置 LCD filter，优化子像素渲染质量
    // FT_LCD_FILTER_DEFAULT: 默认滤波，平衡清晰度和颜色边缘
    // FT_LCD_FILTER_LIGHT: 轻度滤波，更锐利
    // FT_LCD_FILTER_LEGACY: 传统滤波，最锐利但可能有颜色边缘
    FT_Library_SetLcdFilter(g_ft_library, FT_LCD_FILTER_LIGHT);

    g_ft_initialized = true;
    return true;
}

} // namespace

bool initializeFontLibrary() {
    return ensureLibraryInitialized();
}

FT_Library getSharedFreeTypeLibrary() {
    if (!ensureLibraryInitialized()) {
        return nullptr;
    }
    return g_ft_library;
}

FT_Face getOrCreateDesignUnitsFace(const std::string& font_path) {
    if (font_path.empty()) {
        return nullptr;
    }
    if (!ensureLibraryInitialized()) {
        return nullptr;
    }

    ++g_face_usage_counter;

    auto it = g_design_face_cache.find(font_path);
    if (it != g_design_face_cache.end()) {
        g_design_face_last_used[font_path] = g_face_usage_counter;
        return it->second;
    }

    FT_Face face = nullptr;
    FT_Error error = FT_New_Face(g_ft_library, font_path.c_str(), 0, &face);
    if (error != 0) {
        DONG_LOG_ERROR("FontMetrics: FT_New_Face failed for '%s': error=%d", font_path.c_str(), error);
        return nullptr;
    }
    if (!face) {
        DONG_LOG_ERROR("FontMetrics: FT_New_Face returned NULL face for '%s'", font_path.c_str());
        return nullptr;
    }

    DONG_LOG_INFO("FontMetrics: Loaded design face '%s' (family='%s', style='%s')",
                  font_path.c_str(),
                  face->family_name ? face->family_name : "?",
                  face->style_name ? face->style_name : "?");

    // NOTE:
    // We primarily use this face in design-units mode (FT_LOAD_NO_SCALE) so that metrics and
    // MSDF generation are font-size independent.
    // However, some integrations (notably hb-ft) assume an active FT_Size exists and may
    // return zero advances if the face has never had a size set.
    // Setting a deterministic size here keeps the face usable for both paths.
    (void)FT_Set_Pixel_Sizes(face, 0, face->units_per_EM > 0 ? face->units_per_EM : 16);

    g_design_face_cache.emplace(font_path, face);
    g_design_face_last_used[font_path] = g_face_usage_counter;

    if (g_design_face_cache.size() > kMaxDesignFaces) {
        // 绠€鍗曟窐姹帮細绉婚櫎鏈€涔呮湭浣跨敤鐨勪竴涓?face
        std::string oldest_key;
        uint64_t oldest_use = UINT64_MAX;
        for (const auto& kv : g_design_face_last_used) {
            if (kv.second < oldest_use) {
                oldest_use = kv.second;
                oldest_key = kv.first;
            }
        }
        if (!oldest_key.empty()) {
            auto it_face = g_design_face_cache.find(oldest_key);
            if (it_face != g_design_face_cache.end()) {
                if (it_face->second) {
                    FT_Done_Face(it_face->second);
                }
                g_design_face_cache.erase(it_face);
            }
            g_design_face_last_used.erase(oldest_key);
            DONG_LOG_INFO("FontMetrics: evicted design-units face '%s' from cache", oldest_key.c_str());
        }
    }

    return face;
}

bool getFontMetrics(FT_Face face, UnifiedFontMetrics& out_metrics) {
    if (!face) {
        return false;
    }

    out_metrics.units_per_em = face->units_per_EM;

    // 浼樺厛浣跨敤 OS/2 sTypo* 搴﹂噺锛屼娇琛岄珮/ascent 鏇存帴杩戞祻瑙堝櫒瀹炵幇
    TT_OS2* os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(face, ft_sfnt_os2));
    if (os2 && os2->sTypoAscender != 0 && os2->sTypoDescender != 0) {
        out_metrics.ascent_units = static_cast<float>(os2->sTypoAscender);
        out_metrics.descent_units = static_cast<float>(os2->sTypoDescender);
        out_metrics.line_gap_units = static_cast<float>(os2->sTypoLineGap);
        out_metrics.height_units = out_metrics.ascent_units - out_metrics.descent_units + out_metrics.line_gap_units;
    } else {
        // 鍥為€€鍒?FreeType 鎻愪緵鐨?ascender/descender/height
        out_metrics.ascent_units = static_cast<float>(face->ascender);
        out_metrics.descent_units = static_cast<float>(face->descender);
        out_metrics.line_gap_units = static_cast<float>(face->height - face->ascender + face->descender);
        out_metrics.height_units = static_cast<float>(face->height);
    }

    return true;
}

bool getGlyphMetrics(FT_Face face, uint32_t glyph_id, UnifiedGlyphMetrics& out_metrics) {
    if (!face) {
        return false;
    }

    // 浣跨敤 FT_LOAD_NO_SCALE 鍔犺浇鍘熷 design units
    if (FT_Load_Glyph(face, glyph_id, FT_LOAD_NO_SCALE) != 0) {
        DONG_LOG_INFO("FontMetrics: failed to load glyph %u in design units", glyph_id);
        return false;
    }

    // FreeType glyph metrics are 26.6 fixed point.
    constexpr float kFtFixedToUnits = 1.0f / 64.0f;

    out_metrics.glyph_id = glyph_id;
    out_metrics.advance_x_units = static_cast<float>(face->glyph->advance.x) * kFtFixedToUnits;
    out_metrics.bearing_x_units = static_cast<float>(face->glyph->metrics.horiBearingX) * kFtFixedToUnits;
    out_metrics.bearing_y_units = static_cast<float>(face->glyph->metrics.horiBearingY) * kFtFixedToUnits;
    out_metrics.width_units = static_cast<float>(face->glyph->metrics.width) * kFtFixedToUnits;
    out_metrics.height_units = static_cast<float>(face->glyph->metrics.height) * kFtFixedToUnits;


    return true;
}

FT_Face getOrCreateFontFace(const std::string& font_path, uint32_t pixel_size) {
    if (font_path.empty()) {
        return nullptr;
    }
    if (!ensureLibraryInitialized()) {
        return nullptr;
    }

    ++g_face_usage_counter;

    const std::string key = makePixelFaceKey(font_path, pixel_size);
    auto it = g_pixel_face_cache.find(key);
    if (it != g_pixel_face_cache.end()) {
        g_pixel_face_last_used[key] = g_face_usage_counter;
        return it->second;
    }

    FT_Face face = nullptr;
    if (FT_New_Face(g_ft_library, font_path.c_str(), 0, &face) != 0) {
        DONG_LOG_INFO("FontMetrics: failed to load font face '%s'", font_path.c_str());
        return nullptr;
    }

    FT_Set_Pixel_Sizes(face, 0, pixel_size);

    g_pixel_face_cache.emplace(key, face);
    g_pixel_face_last_used[key] = g_face_usage_counter;

    if (g_pixel_face_cache.size() > kMaxPixelFaces) {
        std::string oldest_key;
        uint64_t oldest_use = UINT64_MAX;
        for (const auto& kv : g_pixel_face_last_used) {
            if (kv.second < oldest_use) {
                oldest_use = kv.second;
                oldest_key = kv.first;
            }
        }
        if (!oldest_key.empty()) {
            auto it_face = g_pixel_face_cache.find(oldest_key);
            if (it_face != g_pixel_face_cache.end()) {
                if (it_face->second) {
                    FT_Done_Face(it_face->second);
                }
                g_pixel_face_cache.erase(it_face);
            }
            g_pixel_face_last_used.erase(oldest_key);
            DONG_LOG_INFO("FontMetrics: evicted pixel face '%s' from cache", oldest_key.c_str());
        }
    }

    return face;
}

} // namespace dong::render
