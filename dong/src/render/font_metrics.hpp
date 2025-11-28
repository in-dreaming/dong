#pragma once

#include <string>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace dong::render {

// 统一管理 FreeType 库与按 (font_path, pixel_size) 缓存的 FT_Face。
// 目前仍以像素尺寸为 key，后续可以演进到设计单位模式。

// 确保 FreeType 库已初始化，失败返回 false。
bool initializeFontLibrary();

// 获取共享的 FreeType 库句柄，如未初始化则返回 nullptr。
FT_Library getSharedFreeTypeLibrary();

// 获取或创建指定字体与像素大小的 FT_Face。
// 返回的 FT_Face 由本模块持有与回收，调用方不应调用 FT_Done_Face/FT_Done_FreeType。
FT_Face getOrCreateFontFace(const std::string& font_path, uint32_t pixel_size);

} // namespace dong::render
