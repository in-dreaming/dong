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
    std::string lower = toLowerAscii(name);

    // 只归一化 CSS generic family（sans-serif/serif/monospace），
    // 保留 Arial/Helvetica 等具体家族名，避免覆盖掉作者明确指定的字体顺序。
    if (lower == "sans" || lower == "sans-serif") {
        return "sans-serif";
    }
    if (lower == "serif") {
        return "serif";
    }
    if (lower == "monospace") {
        return "monospace";
    }

    // 其他一律按原样（小写）返回，例如 "arial"、"helvetica"、"times new roman" 等。
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
