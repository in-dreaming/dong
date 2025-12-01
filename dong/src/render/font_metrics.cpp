#include "font_metrics.hpp"

#include <SDL3/SDL_log.h>
#include <cmath>

namespace dong::render {

namespace {

FT_Library g_ft_library = nullptr;
bool g_ft_initialized = false;

// 新缓存：design units face（key: font_path）
std::unordered_map<std::string, FT_Face> g_design_face_cache;

// 旧缓存：像素 face（key: font_path#pixel_size，用于过渡期）
std::unordered_map<std::string, FT_Face> g_pixel_face_cache;

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
        SDL_Log("FontMetrics: failed to initialize FreeType library");
        g_ft_library = nullptr;
        g_ft_initialized = true;
        return false;
    }

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

    auto it = g_design_face_cache.find(font_path);
    if (it != g_design_face_cache.end()) {
        return it->second;
    }

    FT_Face face = nullptr;
    if (FT_New_Face(g_ft_library, font_path.c_str(), 0, &face) != 0) {
        SDL_Log("FontMetrics: failed to load design units face '%s'", font_path.c_str());
        return nullptr;
    }

    // 不设置 pixel size，保持 design units 模式
    g_design_face_cache.emplace(font_path, face);
    return face;
}

bool getFontMetrics(FT_Face face, UnifiedFontMetrics& out_metrics) {
    if (!face) {
        return false;
    }

    out_metrics.units_per_em = face->units_per_EM;
    out_metrics.ascent_units = static_cast<float>(face->ascender);
    out_metrics.descent_units = static_cast<float>(face->descender);
    out_metrics.line_gap_units = static_cast<float>(face->height - face->ascender + face->descender);
    out_metrics.height_units = static_cast<float>(face->height);

    return true;
}

bool getGlyphMetrics(FT_Face face, uint32_t glyph_id, UnifiedGlyphMetrics& out_metrics) {
    if (!face) {
        return false;
    }

    // 使用 FT_LOAD_NO_SCALE 加载原始 design units
    if (FT_Load_Glyph(face, glyph_id, FT_LOAD_NO_SCALE) != 0) {
        SDL_Log("FontMetrics: failed to load glyph %u in design units", glyph_id);
        return false;
    }

    out_metrics.glyph_id = glyph_id;
    out_metrics.advance_x_units = static_cast<float>(face->glyph->advance.x);
    out_metrics.bearing_x_units = static_cast<float>(face->glyph->metrics.horiBearingX);
    out_metrics.bearing_y_units = static_cast<float>(face->glyph->metrics.horiBearingY);
    out_metrics.width_units = static_cast<float>(face->glyph->metrics.width);
    out_metrics.height_units = static_cast<float>(face->glyph->metrics.height);

    return true;
}

FT_Face getOrCreateFontFace(const std::string& font_path, uint32_t pixel_size) {
    if (font_path.empty()) {
        return nullptr;
    }
    if (!ensureLibraryInitialized()) {
        return nullptr;
    }

    const std::string key = makePixelFaceKey(font_path, pixel_size);
    auto it = g_pixel_face_cache.find(key);
    if (it != g_pixel_face_cache.end()) {
        return it->second;
    }

    FT_Face face = nullptr;
    if (FT_New_Face(g_ft_library, font_path.c_str(), 0, &face) != 0) {
        SDL_Log("FontMetrics: failed to load font face '%s'", font_path.c_str());
        return nullptr;
    }

    FT_Set_Pixel_Sizes(face, 0, pixel_size);

    g_pixel_face_cache.emplace(key, face);
    return face;
}

} // namespace dong::render
