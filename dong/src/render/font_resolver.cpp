#include "font_resolver.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace dong::render {

namespace {

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

const std::unordered_map<std::string, std::vector<std::string>> kFontCandidates = {
    // 系统 UI 字体族：-apple-system / BlinkMacSystemFont / system-ui
    {"-apple-system", {
        // macOS 优先选择 SF 系列，其次 Helvetica/Arial
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/SFNSRounded.ttf",
        "/System/Library/Fonts/SF-Pro.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        // Linux 常见替代
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        // Windows 常见替代
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"system-ui", {
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/SFNSRounded.ttf",
        "/System/Library/Fonts/SF-Pro.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"blinkmacsystemfont", {
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/SFNSRounded.ttf",
        "/System/Library/Fonts/SF-Pro.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"sans-serif", {
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"arial", {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"helvetica", {
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

std::string resolveFontPath(const std::string& requested_family) {
    auto families = splitFontFamilies(requested_family);
    if (families.empty()) {
        families.push_back("sans-serif");
    }

    std::vector<std::string> candidate_paths;
    candidate_paths.reserve(families.size() * 3);

    for (const auto& family : families) {
        std::string canonical = canonicalFontFamily(family);
        auto it = kFontCandidates.find(canonical);
        if (it != kFontCandidates.end()) {
            candidate_paths.insert(candidate_paths.end(), it->second.begin(), it->second.end());
            continue;
        }
        auto exact = kFontCandidates.find(toLowerAscii(family));
        if (exact != kFontCandidates.end()) {
            candidate_paths.insert(candidate_paths.end(), exact->second.begin(), exact->second.end());
        }
    }

    if (candidate_paths.empty()) {
        candidate_paths = kFontCandidates.at("sans-serif");
    }

    return findExistingFont(candidate_paths);
}

} // namespace dong::render
