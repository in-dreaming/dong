#include "font_resolver.hpp"
#include "font_finder.hpp"
#include "../core/log.h"
#include "../../third_party/freetype/include/freetype/freetype.h"
#include "../../third_party/freetype/include/freetype/tttables.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace dong::render {

namespace {

// Forward declaration (defined later in this file)
std::string findClosestWeightFont(const std::vector<std::string>& candidates, int requested_weight);

// Ensure font finder is initialized (lazy init)
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

// Preset font candidate list: key is canonical family (or generic family), value is candidate paths
const std::unordered_map<std::string, std::vector<std::string>> kFontCandidates = {
    // Inter font - modern sans-serif for English/numbers
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
    // System UI font family: -apple-system / BlinkMacSystemFont / system-ui
    // Prefer Inter, then fallback to system fonts
    {"-apple-system", {
        // Inter first (English/numbers)
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // macOS system fonts
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        // Linux common alternatives
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        // Windows common alternatives
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"system-ui", {
        // Inter first
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // System fonts
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"blinkmacsystemfont", {
        // Inter first
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // System fonts
        "/System/Library/Fonts/SFNS.ttf",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"sans-serif", {
        // Inter first (English/numbers)
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // Fallback to system sans-serif
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"arial", {
        // Inter first
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // Arial fallback
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf"
    }},
    {"helvetica", {
        // Inter first
        "C:/Windows/Fonts/Inter-Regular.ttf",
        "/Library/Fonts/Inter-Regular.ttf",
        "/usr/share/fonts/truetype/inter/Inter-Regular.ttf",
        // Helvetica fallback
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
        // Inter first
        "C:/Windows/Fonts/Inter-Regular.ttf",
        // Segoe UI fallback
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
    // Chinese fonts
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

// CJK font fallback list (sorted by priority)
// Platform-specific fonts are listed first for better performance
const std::vector<std::string> kCJKFallbackFonts = {
#if defined(__APPLE__) && defined(__MACH__)
    // macOS Chinese fonts - prioritize fonts that exist on most macOS systems
    "/System/Library/Fonts/STHeiti Light.ttc",      // STHeiti (macOS 10.6+, always present)
    "/System/Library/Fonts/STHeiti Medium.ttc",     // STHeiti Medium
    "/System/Library/Fonts/PingFang.ttc",           // PingFang SC (macOS 10.11+)
    "/Library/Fonts/PingFang.ttc",                  // PingFang SC (user-installed)
    "/System/Library/Fonts/Hiragino Sans GB.ttc",   // Hiragino Sans GB
    "/Library/Fonts/Songti.ttc",                    // Songti SC
    "/Library/Fonts/Kaiti.ttc",                     // Kaiti SC
    "/System/Library/Fonts/CJKSymbolsFallback.ttc", // CJK Symbols (fallback)
#elif defined(_WIN32) || defined(_WIN64)
    // Windows Chinese fonts - Microsoft YaHei UI first
    "C:/Windows/Fonts/msyh.ttc",      // Microsoft YaHei UI
    "C:/Windows/Fonts/msyhbd.ttc",    // Microsoft YaHei UI Bold
    "C:/Windows/Fonts/msjh.ttc",      // Microsoft JhengHei
    "C:/Windows/Fonts/simsun.ttc",    // SimSun
    "C:/Windows/Fonts/simhei.ttf",    // SimHei
    "C:/Windows/Fonts/simkai.ttf",    // KaiTi
    "C:/Windows/Fonts/STXIHEI.TTF",   // STXihei
    "C:/Windows/Fonts/mingliu.ttc",   // MingLiU
#else
    // Linux Chinese fonts
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
    "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
    "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
#endif
};

const std::vector<std::string> kEmojiFallbackFonts = {
#if defined(__APPLE__) && defined(__MACH__)
    "/System/Library/Fonts/Apple Color Emoji.ttc",
    "/Library/Fonts/Apple Color Emoji.ttc",
#elif defined(_WIN32) || defined(_WIN64)
    "C:/Windows/Fonts/seguisym.ttf",   // Segoe UI Symbol
    "C:/Windows/Fonts/seguiemj.ttf",   // Segoe UI Emoji (color glyphs may fallback to monochrome path)
    "C:/Windows/Fonts/segoeuiemoji.ttf",
#else
    "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf",
    "/usr/share/fonts/noto/NotoColorEmoji.ttf",
    "/usr/share/fonts/truetype/emoji/NotoColorEmoji.ttf",
#endif
};

// Normalize CSS font-weight to numeric 100-900, unknown values fallback to 400
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

    // Numeric form, e.g. "500", "700"
    int value = 400;
    try {
        value = std::stoi(lower);
    } catch (...) {
        return 400;
    }
    if (value < 100) value = 100;
    if (value > 900) value = 900;
    // Normalize to multiples of 100
    int rem = value % 100;
    if (rem != 0) {
        value = value - rem + (rem >= 50 ? 100 : 0);
    }
    return value;
}

// Weight-based file paths for common font families.
// Returns candidates based on actual weight value (100-900), not just bold vs regular.
// Each entry is checked for existence at runtime; missing paths are silently
// skipped by findExistingFont().
void appendWeightedCandidatesForFamily(const std::string& canonical_family,
                                       int numeric_weight,
                                       std::vector<std::string>& out) {
    // Inter family - has multiple weights available
    auto appendInterWeights = [&] {
        switch (numeric_weight) {
            case 100:
                out.push_back("C:/Windows/Fonts/Inter-Thin.ttf");
                out.push_back("/Library/Fonts/Inter-Thin.ttf");
                out.push_back("/usr/share/fonts/truetype/inter/Inter-Thin.ttf");
                break;
            case 200:
                out.push_back("C:/Windows/Fonts/Inter-ExtraLight.ttf");
                out.push_back("/Library/Fonts/Inter-ExtraLight.ttf");
                out.push_back("/usr/share/fonts/truetype/inter/Inter-ExtraLight.ttf");
                break;
            case 300:
                out.push_back("C:/Windows/Fonts/Inter-Light.ttf");
                out.push_back("/Library/Fonts/Inter-Light.ttf");
                out.push_back("/usr/share/fonts/truetype/inter/Inter-Light.ttf");
                break;
            case 400:
            default:
                out.push_back("C:/Windows/Fonts/Inter-Regular.ttf");
                out.push_back("/Library/Fonts/Inter-Regular.ttf");
                out.push_back("/usr/share/fonts/truetype/inter/Inter-Regular.ttf");
                break;
            case 500:
                out.push_back("C:/Windows/Fonts/Inter-Medium.ttf");
                out.push_back("/Library/Fonts/Inter-Medium.ttf");
                out.push_back("/usr/share/fonts/truetype/inter/Inter-Medium.ttf");
                break;
            case 600:
                out.push_back("C:/Windows/Fonts/Inter-SemiBold.ttf");
                out.push_back("/Library/Fonts/Inter-SemiBold.ttf");
                out.push_back("/usr/share/fonts/truetype/inter/Inter-SemiBold.ttf");
                break;
            case 700:
                out.push_back("C:/Windows/Fonts/Inter-Bold.ttf");
                out.push_back("/Library/Fonts/Inter-Bold.ttf");
                out.push_back("/usr/share/fonts/truetype/inter/Inter-Bold.ttf");
                break;
            case 800:
                out.push_back("C:/Windows/Fonts/Inter-ExtraBold.ttf");
                out.push_back("/Library/Fonts/Inter-ExtraBold.ttf");
                out.push_back("/usr/share/fonts/truetype/inter/Inter-ExtraBold.ttf");
                break;
            case 900:
                out.push_back("C:/Windows/Fonts/Inter-Black.ttf");
                out.push_back("/Library/Fonts/Inter-Black.ttf");
                out.push_back("/usr/share/fonts/truetype/inter/Inter-Black.ttf");
                break;
        }
    };

    // Sans-serif families
    if (canonical_family == "-apple-system" || canonical_family == "sans-serif"
        || canonical_family == "system-ui" || canonical_family == "blinkmacsystemfont"
        || canonical_family == "arial") {
        appendInterWeights();
        // Fallback to Arial weights when Inter not available
        if (numeric_weight >= 700) {
            // Bold weights
            out.push_back("/System/Library/Fonts/Supplemental/Arial Bold.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");
            out.push_back("C:/Windows/Fonts/arialbd.ttf");
        } else {
            // Regular weights
            out.push_back("/System/Library/Fonts/Supplemental/Arial.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
            out.push_back("C:/Windows/Fonts/arial.ttf");
        }
    } else if (canonical_family == "helvetica") {
        appendInterWeights();
        out.push_back("/System/Library/Fonts/Helvetica.ttc");
        out.push_back("/System/Library/Fonts/Supplemental/Arial.ttf");
        out.push_back("C:/Windows/Fonts/arial.ttf");
    } else if (canonical_family == "inter") {
        appendInterWeights();
    } else if (canonical_family == "segoe ui") {
        if (numeric_weight >= 700) {
            out.push_back("C:/Windows/Fonts/segoeuib.ttf");
        } else {
            out.push_back("C:/Windows/Fonts/segoeui.ttf");
        }
        // Fallback
        out.push_back("C:/Windows/Fonts/arialbd.ttf");
        out.push_back("C:/Windows/Fonts/arial.ttf");
    } else if (canonical_family == "serif" || canonical_family == "times"
               || canonical_family == "times new roman") {
        if (numeric_weight >= 700) {
            out.push_back("/System/Library/Fonts/Supplemental/Times New Roman Bold.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf");
            out.push_back("C:/Windows/Fonts/timesbd.ttf");
        } else {
            out.push_back("/System/Library/Fonts/Supplemental/Times New Roman.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf");
            out.push_back("C:/Windows/Fonts/times.ttf");
        }
    } else if (canonical_family == "monospace" || canonical_family == "courier"
               || canonical_family == "courier new") {
        if (numeric_weight >= 700) {
            out.push_back("/System/Library/Fonts/Supplemental/Courier New Bold.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf");
            out.push_back("C:/Windows/Fonts/consolab.ttf");
        } else {
            out.push_back("/System/Library/Fonts/Supplemental/Courier New.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
            out.push_back("C:/Windows/Fonts/consola.ttf");
        }
    }
}

// Bold-variant file paths for common font families (kept for fallback compatibility)
// Each entry is checked for existence at runtime; missing paths are silently
// skipped by findExistingFont().
void appendBoldCandidatesForFamily(const std::string& canonical_family,
                                   std::vector<std::string>& out) {
    appendWeightedCandidatesForFamily(canonical_family, 700, out);
}

// Append more refined candidate paths based on family + weight (e.g. prefer Bold font files for bold weight),
// if the corresponding file is not found, it will naturally fallback to the generic list in kFontCandidates.
void appendCandidatesForFamilyAndWeight(const std::string& canonical_family,
                                        int numeric_weight,
                                        std::vector<std::string>& out) {
    auto it = kFontCandidates.find(canonical_family);

    // Use weight-specific candidates for better font matching
    // This handles all weights (100-900) properly, not just a binary bold/regular split
    appendWeightedCandidatesForFamily(canonical_family, numeric_weight, out);

    // Generic candidates (including SF/Helvetica/Arial etc), always as fallback
    if (it != kFontCandidates.end()) {
        out.insert(out.end(), it->second.begin(), it->second.end());
    }
}

std::string findExistingFont(const std::vector<std::string>& candidates, int requested_weight = 400) {
    if (candidates.empty()) {
        return {};
    }

    namespace fs = std::filesystem;

    // First collect all existing font files
    std::vector<std::string> existing_fonts;
    for (const auto& path : candidates) {
        std::error_code ec;
        if (!path.empty() && fs::exists(path, ec) && !ec) {
            existing_fonts.push_back(path);
        }
    }

    if (existing_fonts.empty()) {
        return {};
    }

    // Use closest weight matching for proper font selection, even for weight==400
    // This ensures we select the correct font when multiple fonts exist
    if (existing_fonts.size() == 1) {
        return existing_fonts[0];
    }

    return findClosestWeightFont(existing_fonts, requested_weight);
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
    // Remove leading/trailing whitespace and quotes
    std::string trimmed = trimWhitespace(name);
    if (!trimmed.empty() && (trimmed.front() == '"' || trimmed.front() == '\'') && trimmed.back() == trimmed.front()) {
        if (trimmed.size() > 2) {
            trimmed = trimmed.substr(1, trimmed.size() - 2);
        } else {
            trimmed.clear();
        }
    }

    std::string lower = toLowerAscii(trimmed);

    // Normalize CSS generic family
    if (lower == "sans" || lower == "sans-serif") {
        return "sans-serif";
    }
    if (lower == "serif") {
        return "serif";
    }
    if (lower == "monospace") {
        return "monospace";
    }

    // Map system UI font families to unified key
    if (lower == "-apple-system" || lower == "blinkmacsystemfont" || lower == "system-ui") {
        return "-apple-system";
    }

    // Common specific family names kept in lowercase, e.g. "arial", "helvetica", "times new roman", "segoe ui"
    return lower;
}

std::string resolveFontPath(const std::string& requested_family,
                            const std::string& font_weight) {
    // Ensure font finder is initialized
    ensureFontFinderInitialized();

    auto families = splitFontFamilies(requested_family);
    if (families.empty()) {
        families.push_back("sans-serif");
    }

    const int numeric_weight = normalizeFontWeight(font_weight);

    DONG_LOG_INFO("[FontResolver] Resolving font: family='%s', weight=%d", requested_family.c_str(), numeric_weight);

    // 1. First try system font lookup API
    for (const auto& family : families) {
        std::string canonical = canonicalFontFamily(family);
        DONG_LOG_INFO("[FontResolver] Trying family '%s' (canonical: '%s')", family.c_str(), canonical.c_str());
        std::vector<FontMatch> system_matches = findSystemFonts(canonical, numeric_weight);

        if (!system_matches.empty()) {
            // Return first matched font path
            std::string path = system_matches[0].path;
            DONG_LOG_INFO("[FontResolver] System font matched: '%s'", path.c_str());
            namespace fs = std::filesystem;
            std::error_code ec;
            if (fs::exists(path, ec) && !ec) {
                DONG_LOG_INFO("[FontResolver] Using font: '%s'", path.c_str());
                return path;
            } else {
                DONG_LOG_WARN("[FontResolver] System font does not exist: '%s'", path.c_str());
            }
        }
    }

    // 2. If system font lookup fails, check user-defined font paths
    // (this is already handled in findSystemFonts as it checks custom paths)

    // 3. Finally fallback to existing hardcoded path list
    std::vector<std::string> candidate_paths;
    candidate_paths.reserve(families.size() * 4);

    for (const auto& family : families) {
        std::string canonical = canonicalFontFamily(family);

        // First append refined candidates based on canonical family + weight
        appendCandidatesForFamilyAndWeight(canonical, numeric_weight, candidate_paths);

        // Then try exact key (for future per-family configuration)
        auto exact = kFontCandidates.find(toLowerAscii(family));
        if (exact != kFontCandidates.end()) {
            candidate_paths.insert(candidate_paths.end(), exact->second.begin(), exact->second.end());
        }
    }

    if (candidate_paths.empty()) {
        // Fallback to sans-serif candidates
        DONG_LOG_INFO("[FontResolver] No candidates found, falling back to sans-serif");
        appendCandidatesForFamilyAndWeight("sans-serif", numeric_weight, candidate_paths);
    }

    std::string result = findExistingFont(candidate_paths, numeric_weight);
    if (result.empty()) {
        DONG_LOG_ERROR("[FontResolver] Failed to find any font for '%s', weight=%d", requested_family.c_str(), numeric_weight);
    } else {
        DONG_LOG_INFO("[FontResolver] Using font: '%s'", result.c_str());
    }
    return result;
}

namespace {

bool wantsItalicOrOblique(const std::string& font_style) {
    std::string style = trimWhitespace(font_style);
    style = toLowerAscii(style);
    if (style.empty() || style == "normal") {
        return false;
    }
    // oblique 目前按 italic 处理（优先选 italic face；没有再回退到 normal）
    return style == "italic" || style == "oblique";
}

void appendCandidatesForFamilyWeightStyle(const std::string& canonical_family,
                                         int numeric_weight,
                                         bool italic_or_oblique,
                                         std::vector<std::string>& out) {
    if (italic_or_oblique) {
        // When italic/oblique is requested, add italic-specific fonts FIRST
        // so they are preferred over regular-weight fonts by findExistingFont.
        if (canonical_family == "arial") {
            if (numeric_weight >= 600) {
                out.push_back("C:/Windows/Fonts/arialbi.ttf");
            } else {
                out.push_back("C:/Windows/Fonts/ariali.ttf");
            }
        } else if (canonical_family == "segoe ui") {
            if (numeric_weight >= 600) {
                out.push_back("C:/Windows/Fonts/segoeuiz.ttf");
            } else {
                out.push_back("C:/Windows/Fonts/segoeuii.ttf");
            }
        } else if (canonical_family == "sans-serif" || canonical_family == "-apple-system") {
            // Common sans-serif italic variants - weight-appropriate first
            if (numeric_weight >= 600) {
                out.push_back("C:/Windows/Fonts/segoeuiz.ttf");
                out.push_back("C:/Windows/Fonts/arialbi.ttf");
            } else {
                out.push_back("C:/Windows/Fonts/segoeuii.ttf");
                out.push_back("C:/Windows/Fonts/ariali.ttf");
            }
        }

        // Linux: DejaVu italic variants
        if (numeric_weight >= 600) {
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans-BoldOblique.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSerif-BoldItalic.ttf");
        } else {
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans-Oblique.ttf");
            out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSerif-Italic.ttf");
        }

        // macOS: Arial/Times italic variants
        if (numeric_weight >= 600) {
            out.push_back("/System/Library/Fonts/Supplemental/Arial Bold Italic.ttf");
            out.push_back("/System/Library/Fonts/Supplemental/Times New Roman Bold Italic.ttf");
        } else {
            out.push_back("/System/Library/Fonts/Supplemental/Arial Italic.ttf");
            out.push_back("/System/Library/Fonts/Supplemental/Times New Roman Italic.ttf");
        }
    }

    // Weight-specific candidates as fallback (or primary for non-italic)
    appendWeightedCandidatesForFamily(canonical_family, numeric_weight, out);

    // For italic: add regular-weight fonts as final fallback when no italic font exists
    if (italic_or_oblique && canonical_family == "arial") {
        out.push_back(numeric_weight >= 600 ? "C:/Windows/Fonts/arialbd.ttf" : "C:/Windows/Fonts/arial.ttf");
    }
}

} // namespace

std::string resolveFontPath(const std::string& requested_family,
                            const std::string& font_weight,
                            const std::string& font_style) {
    // 先走系统查找/自定义字体路径（如果未来实现 DirectWrite/Fontconfig，可以在这里进一步按 style 过滤）
    // 当前 Windows/Linux 的 system lookup 是 stub，主要靠路径候选。

    const bool italic_or_oblique = wantsItalicOrOblique(font_style);

    // 对 normal 样式，保持原有行为，减少对现有用例的影响
    if (!italic_or_oblique) {
        return resolveFontPath(requested_family, font_weight);
    }

    // Ensure font finder is initialized
    ensureFontFinderInitialized();

    auto families = splitFontFamilies(requested_family);
    if (families.empty()) {
        families.push_back("sans-serif");
    }

    const int numeric_weight = normalizeFontWeight(font_weight);

    // 由于 system lookup 目前无法按 italic 过滤，这里直接走候选路径策略
    std::vector<std::string> candidate_paths;
    candidate_paths.reserve(families.size() * 6);

    for (const auto& family : families) {
        std::string canonical = canonicalFontFamily(family);
        appendCandidatesForFamilyWeightStyle(canonical, numeric_weight, true, candidate_paths);

        // exact key candidates
        auto exact = kFontCandidates.find(toLowerAscii(family));
        if (exact != kFontCandidates.end()) {
            candidate_paths.insert(candidate_paths.end(), exact->second.begin(), exact->second.end());
        }
    }

    if (candidate_paths.empty()) {
        appendCandidatesForFamilyWeightStyle("sans-serif", numeric_weight, true, candidate_paths);
    }

    return findExistingFont(candidate_paths, numeric_weight);
}

std::string resolveFontPath(const std::string& requested_family) {
    // Compatibility: no weight equals normal/400
    return resolveFontPath(requested_family, "normal");
}

const std::vector<std::string>& getCJKFallbackFonts() {
    // Cache the result to avoid repeated filesystem checks
    static std::vector<std::string> cached_result;
    static bool cached = false;

    if (!cached) {
        cached_result.reserve(kCJKFallbackFonts.size());

        namespace fs = std::filesystem;
        for (const auto& path : kCJKFallbackFonts) {
            std::error_code ec;
            if (fs::exists(path, ec) && !ec) {
                cached_result.push_back(path);
            }
        }

        cached = true;
    }

    return cached_result;
}

const std::vector<std::string>& getEmojiFallbackFonts() {
    static std::vector<std::string> existing_emoji_fonts;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;
        namespace fs = std::filesystem;
        for (const auto& font_path : kEmojiFallbackFonts) {
            std::error_code ec;
            if (fs::exists(font_path, ec) && !ec) {
                existing_emoji_fonts.push_back(font_path);
            }
        }
    }

    return existing_emoji_fonts;
}

namespace {

// Lazy-initialized FreeType library
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

// Extract the actual weight (100-900) from a font file using FreeType
int getFontFileWeight(const std::string& font_path) {
    FT_Library library = getFTLibrary();
    if (!library) {
        return 400; // Default to normal
    }

    FT_Face face = nullptr;
    if (FT_New_Face(library, font_path.c_str(), 0, &face) != 0) {
        return 400;
    }

    int weight = 400; // Default fallback
    bool has_os2_weight = false;

    // Try to get usWeightClass from OS/2 table (most reliable, authoritative)
    FT_ULong os2_offset = 0;
    FT_ULong os2_length = 0;
    if (FT_Load_Sfnt_Table(face, FT_MAKE_TAG('O', 'S', '/', '2'), 0, nullptr, &os2_length) == 0) {
        std::vector<uint8_t> os2_table(os2_length);
        if (FT_Load_Sfnt_Table(face, FT_MAKE_TAG('O', 'S', '/', '2'), 0, os2_table.data(), &os2_length) == 0) {
            // usWeightClass is at offset 4 (2 bytes, big-endian)
            if (os2_length >= 6) {
                uint16_t weightClass = static_cast<uint16_t>(os2_table[4]) << 8 | os2_table[5];
                // Validate weightClass is in expected range for OS/2
                if (weightClass >= 100 && weightClass <= 900) {
                    weight = weightClass;
                    has_os2_weight = true;
                }
            }
        }
    }

    // If OS/2 weightClass not available, try style flags and style_name as fallback
    if (!has_os2_weight) {
        // Fallback to style flags first
        if (face->style_flags & FT_STYLE_FLAG_BOLD) {
            weight = 700;
        }

        // Check style name for weight hints (only if OS/2 weight not available)
        std::string style_name = face->style_name ? face->style_name : "";
        std::transform(style_name.begin(), style_name.end(), style_name.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (style_name.find("thin") != std::string::npos) {
            weight = 100;
        } else if (style_name.find("extralight") != std::string::npos || style_name.find("extra light") != std::string::npos) {
            weight = 200;
        } else if (style_name.find("light") != std::string::npos) {
            weight = 300;
        } else if (style_name.find("medium") != std::string::npos) {
            weight = 500;
        } else if (style_name.find("semibold") != std::string::npos || style_name.find("semi bold") != std::string::npos) {
            weight = 600;
        } else if (style_name.find("bold") != std::string::npos) {
            weight = 700;
        } else if (style_name.find("extrabold") != std::string::npos || style_name.find("extra bold") != std::string::npos) {
            weight = 800;
        } else if (style_name.find("black") != std::string::npos || style_name.find("heavy") != std::string::npos) {
            weight = 900;
        }
    }

    FT_Done_Face(face);

    // Clamp to valid range
    if (weight < 100) weight = 100;
    if (weight > 900) weight = 900;

    return weight;
}

// Find the font with the closest actual weight to the requested weight
std::string findClosestWeightFont(const std::vector<std::string>& candidates, int requested_weight) {
    if (candidates.empty()) {
        return {};
    }

    struct WeightedCandidate {
        std::string path;
        int actual_weight;
        int distance;
    };

    std::vector<WeightedCandidate> weighted_candidates;

    // Extract actual weights from existing font files
    for (const auto& path : candidates) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (fs::exists(path, ec) && !ec) {
            int actual_weight = getFontFileWeight(path);
            int distance = std::abs(actual_weight - requested_weight);
            weighted_candidates.push_back({path, actual_weight, distance});
        }
    }

    if (weighted_candidates.empty()) {
        // No existing font files found, return first candidate (will be checked later)
        return candidates[0];
    }

    // Sort by distance (closest first), then by actual weight (prefer higher weight for tie)
    std::sort(weighted_candidates.begin(), weighted_candidates.end(),
              [requested_weight](const WeightedCandidate& a, const WeightedCandidate& b) {
                  if (a.distance != b.distance) {
                      return a.distance < b.distance;
                  }
                  // For ties, prefer the weight on the same side of the requested weight
                  // If both are on the same side, prefer the closer one (already handled by distance)
                  // If one is above and one is below, prefer the heavier one (CSS spec)
                  if ((a.actual_weight >= requested_weight) && (b.actual_weight < requested_weight)) {
                      return true;
                  }
                  if ((b.actual_weight >= requested_weight) && (a.actual_weight < requested_weight)) {
                      return false;
                  }
                  return a.actual_weight > b.actual_weight;
              });

    DONG_LOG_INFO("[FontResolver] Requested weight %d, selected '%s' (actual weight %d, distance %d)",
                  requested_weight,
                  weighted_candidates[0].path.c_str(),
                  weighted_candidates[0].actual_weight,
                  weighted_candidates[0].distance);

    return weighted_candidates[0].path;
}

// Check if font supports specified Unicode codepoint
bool fontSupportsCodepoint(const ::std::string& font_path, uint32_t codepoint) {
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

::std::string findFontForCodepoint(uint32_t codepoint, 
                                  const ::std::string& primary_font) {
    // First check primary font
    if (!primary_font.empty() && fontSupportsCodepoint(primary_font, codepoint)) {
        return primary_font;
    }
    
    // Try CJK fallback fonts
    for (const auto& fallback : kCJKFallbackFonts) {
        namespace fs = ::std::filesystem;
        ::std::error_code ec;
        if (fs::exists(fallback, ec) && !ec) {
            if (fontSupportsCodepoint(fallback, codepoint)) {
                return fallback;
            }
        }
    }
    
    // No font found supporting this character
    return {};
}

} // namespace dong::render
