#include "font_resolver.hpp"
#include "font_finder.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace dong::render {

namespace {

// 确保字体查找系统已初始化（延迟初始化）
void ensureFontFinderInitialized() {
    static bool initialized = false;
    if (!initialized) {
        initializeFontFinder();
        initialized = true;
    }
}


std::string trimWhitespace(const std::string& input) {
    size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
        ++start;
    }
    size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }
    return input.substr(start, end - start);
}

std::string toLowerAscii(std::string input) {
    std::transform(input.begin(), input.end(), input.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return input;
}

// 预设字体候选列表：key 为 canonical family（或 generic family），value 为候选路径
const std::unordered_map<std::string, std::vector<std::string>> kFontCandidates = {
    // Inter 字体 - 现代无衬线字体，用于英文/数字
    {"inter", {
        // Windows
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "C:/Windows/Fonts/Inter-Medium.ttf",
        "C:/Windows/Fonts/inter.ttf",
        // macOS
        "/Library/Fonts/Inter-Regular.ttf",
        "/System/Library/Fonts/Inter.ttf",
        // Linux
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        "/usr/share/fonts/opentype/inter/Inter-Regular.otf",
    }},
    // 系统 UI 字体族：-apple-system / BlinkMacSystemFont / system-ui
    // 优先使用 Inter，然后回退到系统字体
    {"-apple-system", {
        // Inter 优先（英文/数字）
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // macOS 系统字体
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        // Linux 常见替代
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        // Windows 常见替代
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"system-ui", {
        // Inter 优先
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // 系统字体
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"blinkmacsystemfont", {
        // Inter 优先
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // 系统字体
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"sans-serif", {
        // Inter 优先（英文/数字）
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // 回退到系统无衬线字体
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"arial", {
        // Inter 优先
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // Arial 回退
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"helvetica", {
        // Inter 优先
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // Helvetica 回退
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf"
    }},
    {"serif", {
        "/System/Library/Fonts/Supplemental/Times New Roman.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "C:/Windows/Fonts/times.ttf"
    }},
    {"times", {
        "/System/Library/Fonts/Supplemental/Times New Roman.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "C:/Windows/Fonts/times.ttf"
    }},
    {"times new roman", {
        "/System/Library/Fonts/Supplemental/Times New Roman.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "C:/Windows/Fonts/times.ttf"
    }},
    {"segoe ui", {
        // Inter 优先
        "C:/Windows/Fonts/Inter-Regular.ttf",
        // Segoe UI 回退
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"
    }},
    {"monospace", {
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/System/Library/Fonts/Supplemental/Menlo.ttc",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf"
    }},
    {"courier", {
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "C:/Windows/Fonts/cour.ttf"
    }},
    {"courier new", {
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "C:/Windows/Fonts/cour.ttf"
    }},
    {"menlo", {
        "/System/Library/Fonts/Supplemental/Menlo.ttc"
    }},
    {"consolas", {
        "/System/Library/Fonts/consola.ttf"
    }},
    // 中文字体
    {"pingfang sc", {
        "/System/Library/Fonts/PingFang.ttc",
        "/Library/Fonts/PingFang.ttc",
    }},
    {"microsoft yahei ui", {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyhbd.ttc",
    }},
    {"microsoft yahei", {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyhbd.ttc",
    }},
};

// CJK 字体回退列表（按优先级排序）
// Windows 优先使用 Microsoft YaHei UI，macOS 优先使用 PingFang SC
const std::vector<std::string> kCJKFallbackFonts = {
    // Windows 中文字体 - Microsoft YaHei UI 优先
    "C:/Windows/Fonts/msyh.ttc",      // Microsoft YaHei UI (微软雅黑)
    "C:/Windows/Fonts/msyhbd.ttc",    // Microsoft YaHei UI Bold
    "C:/Windows/Fonts/msjh.ttc",      // Microsoft JhengHei (微软正黑)
    "C:/Windows/Fonts/simsun.ttc",    // SimSun (宋体)
    "C:/Windows/Fonts/simhei.ttf",    // SimHei (黑体)
    "C:/Windows/Fonts/simkai.ttf",    // KaiTi (楷体)
    "C:/Windows/Fonts/STXIHEI.TTF",   // STXihei
    "C:/Windows/Fonts/mingliu.ttc",   // MingLiU (细明体)
    // macOS 中文字体 - PingFang SC 优先
    "/System/Library/Fonts/PingFang.ttc",           // PingFang SC (苹方)
    "/Library/Fonts/PingFang.ttc",
    "/System/Library/Fonts/STHeiti Light.ttc",      // STHeiti (华文黑体)
    "/System/Library/Fonts/STHeiti Medium.ttc",
    "/System/Library/Fonts/Hiragino Sans GB.ttc",   // Hiragino Sans GB (冬青黑体)
    "/Library/Fonts/Songti.ttc",                    // Songti SC (宋体)
    "/Library/Fonts/Kaiti.ttc",                     // Kaiti SC (楷体)
    // Linux 中文字体
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
    "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
    "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
};

std::string findExistingFont(const std::vector<std::string>& candidates) {
    namespace fs = std::filesystem;
    for (const auto& path : candidates) {
        std::error_code ec;
        if (!path.empty() && fs::exists(path, ec) && !ec) {
            return path;
        }
    }
    return {};
}

// 将 CSS font-weight 规范化为数值 100–900，未知值回退为 400
int normalizeFontWeight(const std::string& css_weight) {
    std::string trimmed = trimWhitespace(css_weight);
    if (trimmed.empty()) {
        return 400;
    }
    std::string lower = toLowerAscii(trimmed);
    if (lower == "normal") {
        return 400;
    }
    if (lower == "bold") {
        return 700;
    }
    if (lower == "bolder") {
        return 700;
    }
    if (lower == "lighter") {
        return 300;
    }

    // 数值形式，如 "500"、"700"
    int value = 400;
    try {
        value = std::stoi(lower);
    } catch (...) {
        return 400;
    }
    if (value < 100) value = 100;
    if (value > 900) value = 900;
    // 规范到 100 的倍数
    int rem = value % 100;
    if (rem != 0) {
        value = value - rem + (rem >= 50 ? 100 : 0);
    }
    return value;
}

// 根据 family + weight 追加更精细的候选路径（例如为粗体优先尝试 Bold 字体文件），
// 若找不到对应文件，会自然回退到 kFontCandidates 中的通用列表。
void appendCandidatesForFamilyAndWeight(const std::string& canonical_family,
                                        int numeric_weight,
                                        std::vector<std::string>& out) {
    auto it = kFontCandidates.find(canonical_family);

    const bool is_bold = numeric_weight >= 600;
    if (is_bold) {
        // 针对粗体优先尝试常见 Bold 变体路径（若不存在会被忽略）
        if (canonical_family == "-apple-system" || canonical_family == "sans-serif") {
            out.push_back("/System/Library/Fonts/Supplemental/Arial Bold.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");
            out.push_back("C:/Windows/Fonts/arialbd.ttf");
        } else if (canonical_family == "serif") {
            out.push_back("/System/Library/Fonts/Supplemental/Times New Roman Bold.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf");
            out.push_back("C:/Windows/Fonts/timesbd.ttf");
        } else if (canonical_family == "monospace") {
            out.push_back("/System/Library/Fonts/Supplemental/Courier New Bold.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf");
            out.push_back("C:/Windows/Fonts/consolab.ttf");
        }
    }

    // 通用候选（包含 SF/Helvetica/Arial 等），始终作为回退
    if (it != kFontCandidates.end()) {
        out.insert(out.end(), it->second.begin(), it->second.end());
    }
}

} // namespace

std::vector<std::string> splitFontFamilies(const std::string& css_value) {
    std::vector<std::string> result;
    if (css_value.empty()) {
        return result;
    }

    size_t start = 0;
    while (start < css_value.size()) {
        size_t comma = css_value.find(',', start);
        std::string token = css_value.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
        token = trimWhitespace(token);
        if (!token.empty()) {
            result.push_back(token);
        }
        if (comma == std::string::npos) {
            break;
        }
        start = comma + 1;
    }
    return result;
}

std::string canonicalFontFamily(const std::string& name) {
    // 去掉首尾空白与引号
    std::string trimmed = trimWhitespace(name);
    if (!trimmed.empty() && (trimmed.front() == '"' || trimmed.front() == '\'') && trimmed.back() == trimmed.front()) {
        if (trimmed.size() > 2) {
            trimmed = trimmed.substr(1, trimmed.size() - 2);
        } else {
            trimmed.clear();
        }
    }

    std::string lower = toLowerAscii(trimmed);

    // 归一化 CSS generic family
    if (lower == "sans" || lower == "sans-serif") {
        return "sans-serif";
    }
    if (lower == "serif") {
        return "serif";
    }
    if (lower == "monospace") {
        return "monospace";
    }

    // 系统 UI 字体族映射到统一 key，方便在映射表中维护候选路径
    if (lower == "-apple-system" || lower == "blinkmacsystemfont" || lower == "system-ui") {
        return "-apple-system";
    }

    // 常见具体家族名保持小写形式，例如 "arial"、"helvetica"、"times new roman"、"segoe ui" 等
    return lower;
}

std::string resolveFontPath(const std::string& requested_family,
                            const std::string& font_weight) {
    // 确保字体查找系统已初始化
    ensureFontFinderInitialized();
    
    auto families = splitFontFamilies(requested_family);
    if (families.empty()) {
        families.push_back("sans-serif");
    }

    const int numeric_weight = normalizeFontWeight(font_weight);

    // 1. 首先尝试系统字体查找 API
    for (const auto& family : families) {
        std::string canonical = canonicalFontFamily(family);
        std::vector<FontMatch> system_matches = findSystemFonts(canonical, numeric_weight);
        
        if (!system_matches.empty()) {
            // 返回第一个匹配的字体路径
            std::string path = system_matches[0].path;
            namespace fs = std::filesystem;
            std::error_code ec;
            if (fs::exists(path, ec) && !ec) {
                return path;
            }
        }
    }

    // 2. 如果系统字体查找失败，检查用户自定义字体路径
    // (这部分已经在 findSystemFonts 中处理，因为它会检查自定义路径)

    // 3. 最后回退到现有的硬编码路径列表
    std::vector<std::string> candidate_paths;
    candidate_paths.reserve(families.size() * 4);

    for (const auto& family : families) {
        std::string canonical = canonicalFontFamily(family);

        // 优先根据 canonical family + weight 追加更精细的候选
        appendCandidatesForFamilyAndWeight(canonical, numeric_weight, candidate_paths);

        // 其次尝试 exact key（方便未来为具体 family 单独配置）
        auto exact = kFontCandidates.find(toLowerAscii(family));
        if (exact != kFontCandidates.end()) {
            candidate_paths.insert(candidate_paths.end(), exact->second.begin(), exact->second.end());
        }
    }

    if (candidate_paths.empty()) {
        // 兜底使用 sans-serif 的候选列表
        appendCandidatesForFamilyAndWeight("sans-serif", numeric_weight, candidate_paths);
    }

    return findExistingFont(candidate_paths);
}

std::string resolveFontPath(const std::string& requested_family) {
    // 兼容旧接口：不传 weight 等价于 normal/400
    return resolveFontPath(requested_family, "normal");
}

std::vector<std::string> getCJKFallbackFonts() {
    std::vector<std::string> result;
    result.reserve(kCJKFallbackFonts.size());
    
    namespace fs = std::filesystem;
    for (const auto& path : kCJKFallbackFonts) {
        std::error_code ec;
        if (fs::exists(path, ec) && !ec) {
            result.push_back(path);
        }
    }
    
    return result;
}

// FreeType 库实例（用于检查字符支持）
// 注意：这里使用静态变量，实际项目中应该考虑更好的生命周期管理
#include <ft2build.h>
#include FT_FREETYPE_H

namespace {

// 延迟初始化的 FreeType 库
FT_Library getFTLibrary() {
    static FT_Library library = nullptr;
    static bool initialized = false;
    
    if (!initialized) {
        if (FT_Init_FreeType(&library) != 0) {
            library = nullptr;
        }
        initialized = true;
    }
    
    return library;
}

// 检查字体是否支持指定的 Unicode 码点
bool fontSupportsCodepoint(const std::string& font_path, uint32_t codepoint) {
    FT_Library library = getFTLibrary();
    if (!library) {
        return false;
    }
    
    FT_Face face = nullptr;
    if (FT_New_Face(library, font_path.c_str(), 0, &face) != 0) {
        return false;
    }
    
    FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
    FT_Done_Face(face);
    
    return glyph_index != 0;
}

} // namespace

std::string findFontForCodepoint(uint32_t codepoint, 
                                  const std::string& primary_font) {
    // 首先检查主字体
    if (!primary_font.empty() && fontSupportsCodepoint(primary_font, codepoint)) {
        return primary_font;
    }
    
    // 尝试 CJK 回退字体
    for (const auto& fallback : kCJKFallbackFonts) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (fs::exists(fallback, ec) && !ec) {
            if (fontSupportsCodepoint(fallback, codepoint)) {
                return fallback;
            }
        }
    }
    
    // 没有找到支持该字符的字体
    return {};
}

} // namespace dong::render
