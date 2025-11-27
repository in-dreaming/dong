#pragma once

#include <string>
#include <vector>

namespace dong::render {

// 解析 CSS font-family 字符串并拆分候选字体族
std::vector<std::string> splitFontFamilies(const std::string& css_value);

// 将字体族名映射到标准化别名（sans-serif / serif / monospace 等）
std::string canonicalFontFamily(const std::string& name);

// 根据 font-family 声明返回本地可用的字体文件路径
std::string resolveFontPath(const std::string& requested_family);

} // namespace dong::render
