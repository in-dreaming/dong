#include "font_finder.hpp"

#include <SDL3/SDL_log.h>
#include <algorithm>
#include <filesystem>
#include <unordered_set>
#include <vector>
#include <limits>

#if defined(__APPLE__) && defined(__MACH__)
#include <CoreText/CoreText.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/param.h> // for PATH_MAX
#endif

namespace dong::render {

namespace {

// 用户自定义字体路径列表
std::vector<std::string> g_custom_font_paths;
bool g_font_finder_initialized = false;

// 支持的字体文件扩展名
const std::unordered_set<std::string> kFontExtensions = {
    ".ttf", ".otf", ".ttc", ".otc", ".woff", ".woff2"
};

// 检查文件是否为字体文件
bool isFontFile(const std::string& path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::is_regular_file(path, ec) || ec) {
        return false;
    }
    
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return kFontExtensions.find(ext) != kFontExtensions.end();
}

// 扫描目录中的字体文件
void scanDirectoryForFonts(const std::string& dir_path, std::vector<std::string>& out_fonts) {
    namespace fs = std::filesystem;
    std::error_code ec;
    
    if (!fs::is_directory(dir_path, ec) || ec) {
        return;
    }
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(dir_path, ec)) {
            if (!ec && isFontFile(entry.path().string())) {
                out_fonts.push_back(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        SDL_Log("FontFinder: error scanning directory '%s': %s", dir_path.c_str(), e.what());
    }
}

// 从自定义路径收集字体文件
std::vector<std::string> collectCustomFonts() {
    std::vector<std::string> fonts;
    namespace fs = std::filesystem;
    
    for (const auto& path : g_custom_font_paths) {
        std::error_code ec;
        if (fs::is_directory(path, ec)) {
            scanDirectoryForFonts(path, fonts);
        } else if (fs::is_regular_file(path, ec) && isFontFile(path)) {
            fonts.push_back(path);
        }
    }
    
    return fonts;
}

} // namespace

std::vector<FontMatch> findSystemFonts(const std::string& family_name, int weight) {
    std::vector<FontMatch> matches;
    
    if (!g_font_finder_initialized) {
        SDL_Log("FontFinder: not initialized, falling back to hardcoded paths");
        return matches;
    }
    
#if defined(__APPLE__) && defined(__MACH__)
    // macOS implementation using CoreText
    CFStringRef family_name_cf = CFStringCreateWithCString(
        kCFAllocatorDefault,
        family_name.c_str(),
        kCFStringEncodingUTF8
    );
    if (!family_name_cf) {
        return matches;
    }
    
    // Create font descriptor with family name and weight
    CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(
        kCFAllocatorDefault,
        0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );
    
    CFDictionarySetValue(attributes, kCTFontFamilyNameAttribute, family_name_cf);
    
    // Convert CSS weight (100-900) to CoreText weight (-1.0 to 1.0)
    // CSS 400 = normal, 700 = bold
    // CoreText: -1.0 (thin) to 1.0 (bold), 0.0 = normal
    float ct_weight = (weight - 400) / 300.0f;
    ct_weight = std::max(-1.0f, std::min(1.0f, ct_weight));
    CFNumberRef weight_value = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloatType, &ct_weight);
    
    CFMutableDictionaryRef traits = CFDictionaryCreateMutable(
        kCFAllocatorDefault,
        0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );
    CFDictionarySetValue(traits, kCTFontWeightTrait, weight_value);
    CFDictionarySetValue(attributes, kCTFontTraitsAttribute, traits);
    CFRelease(traits);
    
    CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
    
    // Get matching font descriptors
    CFArrayRef matching_descriptors = CTFontDescriptorCreateMatchingFontDescriptors(
        descriptor,
        nullptr
    );
    
    if (matching_descriptors && CFArrayGetCount(matching_descriptors) > 0) {
        for (CFIndex i = 0; i < CFArrayGetCount(matching_descriptors); ++i) {
            CTFontDescriptorRef match_desc = (CTFontDescriptorRef)CFArrayGetValueAtIndex(
                matching_descriptors, i
            );
            
            // Get font URL
            CFURLRef font_url = (CFURLRef)CTFontDescriptorCopyAttribute(
                match_desc,
                kCTFontURLAttribute
            );
            
            if (font_url) {
                char path_buffer[PATH_MAX];
                if (CFURLGetFileSystemRepresentation(font_url, true, 
                    (UInt8*)path_buffer, PATH_MAX)) {
                    FontMatch match;
                    match.path = path_buffer;
                    
                    // Get family name
                    CFStringRef match_family = (CFStringRef)CTFontDescriptorCopyAttribute(
                        match_desc,
                        kCTFontFamilyNameAttribute
                    );
                    if (match_family) {
                        CFIndex length = CFStringGetLength(match_family);
                        CFIndex max_size = CFStringGetMaximumSizeForEncoding(
                            length, kCFStringEncodingUTF8) + 1;
                        std::vector<char> buffer(max_size);
                        if (CFStringGetCString(match_family, buffer.data(), max_size,
                            kCFStringEncodingUTF8)) {
                            match.family_name = buffer.data();
                        }
                        CFRelease(match_family);
                    }
                    
                    match.weight = weight;
                    match.is_system_font = true;
                    matches.push_back(match);
                }
                CFRelease(font_url);
            }
        }
        CFRelease(matching_descriptors);
    }
    
    if (descriptor) CFRelease(descriptor);
    if (weight_value) CFRelease(weight_value);
    if (attributes) CFRelease(attributes);
    if (family_name_cf) CFRelease(family_name_cf);
#elif defined(_WIN32) || defined(_WIN64)
    // Windows implementation using DirectWrite
    // Note: DirectWrite requires COM initialization and is complex
    // For now, we'll rely on fallback to hardcoded paths
    // TODO: Full DirectWrite implementation when needed
    (void)family_name;
    (void)weight;
#else
    // Linux implementation using Fontconfig
    // Note: Fontconfig requires linking against libfontconfig
    // For now, we'll rely on fallback to hardcoded paths
    // TODO: Full Fontconfig implementation when needed
    (void)family_name;
    (void)weight;
#endif
    
    // Also check custom font paths
    std::vector<std::string> custom_fonts = collectCustomFonts();
    for (const auto& font_path : custom_fonts) {
        FontMatch match;
        match.path = font_path;
        match.family_name = family_name; // Will be filled by platform-specific code
        match.weight = weight;
        match.is_system_font = false;
        matches.push_back(match);
    }
    
    return matches;
}

void addCustomFontPath(const std::string& path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    
    if (path.empty()) {
        return;
    }
    
    // 检查路径是否存在
    if (!fs::exists(path, ec) || ec) {
        SDL_Log("FontFinder: custom font path does not exist: '%s'", path.c_str());
        return;
    }
    
    // 检查是否已添加
    auto it = std::find(g_custom_font_paths.begin(), g_custom_font_paths.end(), path);
    if (it == g_custom_font_paths.end()) {
        g_custom_font_paths.push_back(path);
        SDL_Log("FontFinder: added custom font path: '%s'", path.c_str());
    }
}

void clearCustomFontPaths() {
    g_custom_font_paths.clear();
    SDL_Log("FontFinder: cleared all custom font paths");
}

bool initializeFontFinder() {
    if (g_font_finder_initialized) {
        return true;
    }
    
#if defined(__APPLE__) && defined(__MACH__)
    // macOS: CoreText is always available, no initialization needed
    g_font_finder_initialized = true;
    return true;
#elif defined(_WIN32) || defined(_WIN64)
    // Windows: DirectWrite initialization
    // TODO: Initialize DirectWrite
    g_font_finder_initialized = true;
    return true;
#else
    // Linux: Fontconfig initialization
    // TODO: Initialize Fontconfig
    g_font_finder_initialized = true;
    return true;
#endif
}

void cleanupFontFinder() {
    if (!g_font_finder_initialized) {
        return;
    }
    
    clearCustomFontPaths();
    
#if defined(__APPLE__) && defined(__MACH__)
    // macOS: No cleanup needed for CoreText
#elif defined(_WIN32) || defined(_WIN64)
    // Windows: DirectWrite cleanup
    // TODO: Cleanup DirectWrite
#else
    // Linux: Fontconfig cleanup
    // TODO: Cleanup Fontconfig
#endif
    
    g_font_finder_initialized = false;
}

} // namespace dong::render

