#pragma once

#include <string>
#include <vector>

namespace dong::render {

// 系统字体查找结果
struct FontMatch {
    std::string path;           // 字体文件路径
    std::string family_name;    // 字体族名
    int weight;                 // 字重 (100-900)
    bool is_system_font;        // 是否为系统字体
};

// 使用系统 API 查找字体
// 返回匹配的字体列表，按匹配度排序（最佳匹配在前）
std::vector<FontMatch> findSystemFonts(const std::string& family_name, int weight);

// 添加用户自定义字体路径（可以是文件或目录）
// 如果是目录，会扫描其中的字体文件
void addCustomFontPath(const std::string& path);

// 清除所有自定义字体路径
void clearCustomFontPaths();

// 初始化字体查找系统（平台特定初始化）
bool initializeFontFinder();

// 清理字体查找系统（平台特定清理）
void cleanupFontFinder();

} // namespace dong::render

