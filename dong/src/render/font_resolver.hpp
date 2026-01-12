#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace dong::render {

// 解析 CSS font-family 字符串并拆分候选字体族
std::vector<std::string> splitFontFamilies(const std::string& css_value);

// 将字体族名映射到标准化别名（sans-serif / serif / monospace 等）
std::string canonicalFontFamily(const std::string& name);

// 根据 font-family 声明返回本地可用的字体文件路径（不含 font-weight 信息，等价于 CSS 的 400/normal）
std::string resolveFontPath(const std::string& requested_family);

// 根据 font-family + font-weight 返回本地可用的字体文件路径
// font_weight 支持 "normal"/"bold"/数值 100–900 等 CSS 写法
std::string resolveFontPath(const std::string& requested_family,
                            const std::string& font_weight);

// 根据 font-family + font-weight + font-style 返回本地可用的字体文件路径
// font_style 支持 "normal"/"italic"/"oblique"（oblique 目前按 italic 处理）
std::string resolveFontPath(const std::string& requested_family,
                            const std::string& font_weight,
                            const std::string& font_style);


// 获取 CJK（中日韩）字体回退列表
// 返回系统中存在的 CJK 字体路径列表，按优先级排序
std::vector<std::string> getCJKFallbackFonts();

// 查找支持指定 Unicode 码点的字体
// 首先检查 primary_font，如果不支持则尝试 CJK 回退字体
// 返回支持该字符的字体路径，如果都不支持则返回空字符串
std::string findFontForCodepoint(uint32_t codepoint, 
                                  const std::string& primary_font);

} // namespace dong::render
