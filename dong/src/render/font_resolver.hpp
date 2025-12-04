#pragma once

#include <string>
#include <vector>

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

} // namespace dong::render
