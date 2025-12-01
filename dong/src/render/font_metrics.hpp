#pragma once

#include <string>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace dong::render {

// 统一字体度量信息（design units 空间）
struct UnifiedFontMetrics {
    uint32_t units_per_em = 0;      // 字体的 EM 单位（如 2048）
    float ascent_units = 0.0f;      // design units
    float descent_units = 0.0f;     // design units（通常为负值）
    float line_gap_units = 0.0f;    // design units
    float height_units = 0.0f;      // ascent - descent + line_gap
};

// 统一字形度量信息（design units 空间）
struct UnifiedGlyphMetrics {
    uint32_t glyph_id = 0;
    float advance_x_units = 0.0f;   // design units
    float bearing_x_units = 0.0f;   // design units
    float bearing_y_units = 0.0f;   // design units
    float width_units = 0.0f;       // design units
    float height_units = 0.0f;      // design units
    
    // MSDF 生成元数据（与字号无关）
    float msdf_scale = 1.0f;
    float msdf_translate_x = 0.0f;
    float msdf_translate_y = 0.0f;
};

// 确保 FreeType 库已初始化，失败返回 false。
bool initializeFontLibrary();

// 获取共享的 FreeType 库句柄，如未初始化则返回 nullptr。
FT_Library getSharedFreeTypeLibrary();

// 获取或创建指定字体的 FT_Face（无缩放，design units 模式）
// 返回的 FT_Face 由本模块持有与回收，调用方不应调用 FT_Done_Face。
FT_Face getOrCreateDesignUnitsFace(const std::string& font_path);

// 获取字体的统一度量（design units）
bool getFontMetrics(FT_Face face, UnifiedFontMetrics& out_metrics);

// 获取字形的统一度量（design units）
bool getGlyphMetrics(FT_Face face, uint32_t glyph_id, UnifiedGlyphMetrics& out_metrics);

// 【兼容旧接口】获取或创建指定字体与像素大小的 FT_Face（用于过渡期）
FT_Face getOrCreateFontFace(const std::string& font_path, uint32_t pixel_size);

} // namespace dong::render
