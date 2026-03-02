#include "resource_manager.hpp"
#include "../core/log.h"
#include "font_resolver.hpp"
#include "font_metrics.hpp"
#include "dong_platform.h"
#include "dong_file_system.h"
#include "dong_image_decoder.h"

#include <sstream>
#include <cstring>
#include <algorithm>
#include <cctype>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace dong::render {

// ImageResource destructor
ImageResource::~ImageResource() {
    // Pixel data is managed separately (usually by atlas or caller)
}

// FontResource destructor
FontResource::~FontResource() {
    // Release FreeType face if loaded
    if (ft_face) {
        FT_Done_Face(ft_face);
        ft_face = nullptr;
    }
}

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager() {
    clear();
}

// =============================================================================
// Path Utilities
// =============================================================================

static std::string trimString(const std::string& s) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    auto begin = std::find_if(s.begin(), s.end(), not_space);
    auto end = std::find_if(s.rbegin(), s.rend(), not_space).base();
    return (begin < end) ? std::string(begin, end) : std::string();
}

static bool isAbsolutePath(const std::string& p) {
    if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':') {
        return true; // Windows drive path
    }
    if (!p.empty() && (p[0] == '/' || p[0] == '\\')) {
        return true; // Unix absolute or UNC-like
    }
    return false;
}

static std::string normalizeUrl(const std::string& url) {
    std::string path = url;

    // Handle file:// URLs
    if (path.rfind("file://", 0) == 0) {
        path = path.substr(7);
        // Windows file URL: file:///C:/... -> C:/...
        if (path.size() >= 3 && path[0] == '/' &&
            std::isalpha(static_cast<unsigned char>(path[1])) && path[2] == ':') {
            path.erase(path.begin());
        }
    }

    return path;
}

static std::string resolvePath(const std::string& base, const std::string& relative) {
    if (base.empty() || isAbsolutePath(relative)) {
        return relative;
    }

    // Simple path joining
    std::string result = base;
    if (!result.empty() && result.back() != '/' && result.back() != '\\') {
        result += '/';
    }
    result += relative;

    return result;
}

// =============================================================================
// data: URL helpers (minimal; supports data:image/*;base64,...)
// =============================================================================

static int base64Value(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return static_cast<int>(c - 'A');
    if (c >= 'a' && c <= 'z') return static_cast<int>(c - 'a') + 26;
    if (c >= '0' && c <= '9') return static_cast<int>(c - '0') + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static bool decodeBase64ToBytes(const std::string& in, std::vector<uint8_t>& out) {
    out.clear();
    int vals[4] = {0, 0, 0, 0};
    int val_count = 0;

    for (unsigned char c : in) {
        if (std::isspace(c)) continue;
        if (c == '=') break;

        int v = base64Value(c);
        if (v < 0) {
            return false;
        }
        vals[val_count++] = v;
        if (val_count == 4) {
            out.push_back(static_cast<uint8_t>((vals[0] << 2) | (vals[1] >> 4)));
            out.push_back(static_cast<uint8_t>(((vals[1] & 0x0F) << 4) | (vals[2] >> 2)));
            out.push_back(static_cast<uint8_t>(((vals[2] & 0x03) << 6) | vals[3]));
            val_count = 0;
        }
    }

    if (val_count == 2) {
        out.push_back(static_cast<uint8_t>((vals[0] << 2) | (vals[1] >> 4)));
    } else if (val_count == 3) {
        out.push_back(static_cast<uint8_t>((vals[0] << 2) | (vals[1] >> 4)));
        out.push_back(static_cast<uint8_t>(((vals[1] & 0x0F) << 4) | (vals[2] >> 2)));
    }

    return !out.empty();
}

static bool decodeDataImageBase64(const std::string& url,
                                 std::string& out_mime,
                                 std::vector<uint8_t>& out_bytes) {
    out_mime.clear();
    out_bytes.clear();

    if (url.rfind("data:", 0) != 0) {
        return false;
    }

    const size_t comma = url.find(',');
    if (comma == std::string::npos || comma <= 5) {
        return false;
    }

    std::string meta = url.substr(5, comma - 5);
    std::string data = url.substr(comma + 1);

    // Normalize meta to lowercase for scheme parsing
    std::string meta_lower = meta;
    std::transform(meta_lower.begin(), meta_lower.end(), meta_lower.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    const size_t semi = meta_lower.find(';');
    out_mime = (semi == std::string::npos) ? meta_lower : meta_lower.substr(0, semi);
    if (out_mime.empty()) {
        return false;
    }
    if (out_mime.rfind("image/", 0) != 0) {
        return false;
    }
    if (meta_lower.find(";base64") == std::string::npos) {
        return false;
    }

    return decodeBase64ToBytes(data, out_bytes);
}

// =============================================================================
// Image Loading via Platform IImageDecoder
// =============================================================================

bool ResourceManager::getImagePixelsRGBA(const std::string& file_path,
                                         std::vector<uint8_t>& out_pixels,
                                         uint32_t& out_width,
                                         uint32_t& out_height) {
    out_pixels.clear();
    out_width = 0;
    out_height = 0;

    std::string path = trimString(file_path);
    if (path.empty()) {
        return false;
    }

    // Reject unsupported URL schemes
    if (path.rfind("http://", 0) == 0 || path.rfind("https://", 0) == 0) {
        DONG_LOG_WARN("[ResourceManager] unsupported image URL: %s", path.c_str());
        return false;
    }

    // data: URLs (supported subset: data:image/*;base64,...)
    if (path.rfind("data:", 0) == 0) {
        DongPlatform* platform = dong_platform_get();
        DongImageDecoder* decoder = dong_platform_get_image_decoder(platform);
        if (!decoder) {
            DONG_LOG_WARN("[ResourceManager] no image decoder registered");
            return false;
        }

        std::string mime;
        std::vector<uint8_t> bytes;
        if (!decodeDataImageBase64(path, mime, bytes)) {
            DONG_LOG_WARN("[ResourceManager] unsupported image URL: %s", path.c_str());
            return false;
        }

        DongDecodedImage image = {};
        DongImageDecoderResult decode_result = dong_image_decode(decoder, bytes.data(), bytes.size(), &image);
        if (decode_result != DONG_IMAGE_OK) {
            DONG_LOG_ERROR("[ResourceManager] failed to decode data URL image (error=%d)", (int)decode_result);
            return false;
        }

        if (image.format != DONG_IMAGE_FORMAT_RGBA8) {
            DONG_LOG_WARN("[ResourceManager] unexpected image format %s, expected RGBA8 (data URL)",
                          dong_image_format_name(image.format));
            dong_image_free(decoder, &image);
            return false;
        }

        out_width = image.width;
        out_height = image.height;
        out_pixels.resize(image.data_size);
        std::memcpy(out_pixels.data(), image.data, image.data_size);
        dong_image_free(decoder, &image);
        return true;
    }

    // Normalize file:// URLs
    path = normalizeUrl(path);

    // Resolve relative paths
    if (!resource_root_.empty() && !isAbsolutePath(path)) {
        std::string resolved = resolvePath(resource_root_, path);
        const char* env = std::getenv("DONG_DEBUG_RESOURCE_PATHS");
        if (env && env[0] == '1') {
            DONG_LOG_INFO("[ResourceManager] resolve: root=%s path=%s -> %s",
                          resource_root_.c_str(), path.c_str(), resolved.c_str());
        }
        path = resolved;
    }

    // Get platform services
    DongPlatform* platform = dong_platform_get();
    DongFileSystem* fs = dong_platform_get_file_system(platform);
    DongImageDecoder* decoder = dong_platform_get_image_decoder(platform);

    // Read file via FileSystem
    DongFileData file_data = {nullptr, 0};
    DongFileSystemResult fs_result = dong_fs_read_all(fs, path.c_str(), &file_data);

    if (fs_result != DONG_FS_OK || !file_data.data || file_data.size == 0) {
        DONG_LOG_ERROR("[ResourceManager] failed to read file: %s (error=%d)", path.c_str(), (int)fs_result);
        return false;
    }

    // Check if we have an image decoder
    if (!decoder) {
        DONG_LOG_WARN("[ResourceManager] no image decoder registered, cannot decode: %s", path.c_str());
        dong_fs_free_data(fs, &file_data);
        return false;
    }

    // Decode image
    DongDecodedImage image = {};
    DongImageDecoderResult decode_result = dong_image_decode(decoder, file_data.data, file_data.size, &image);

    // Free file data (no longer needed)
    dong_fs_free_data(fs, &file_data);

    if (decode_result != DONG_IMAGE_OK) {
        DONG_LOG_ERROR("[ResourceManager] failed to decode image: %s (error=%d)", path.c_str(), (int)decode_result);
        return false;
    }

    // Verify format is RGBA8
    if (image.format != DONG_IMAGE_FORMAT_RGBA8) {
        DONG_LOG_WARN("[ResourceManager] unexpected image format %s, expected RGBA8: %s",
                      dong_image_format_name(image.format), path.c_str());
        dong_image_free(decoder, &image);
        return false;
    }

    // Copy pixels to output vector
    out_width = image.width;
    out_height = image.height;
    out_pixels.resize(image.data_size);
    std::memcpy(out_pixels.data(), image.data, image.data_size);

    // Free decoded image
    dong_image_free(decoder, &image);

    return true;
}

ImageResource* ResourceManager::loadImage(const std::string& file_path) {
    // Check cache first
    auto it = image_cache_.find(file_path);
    if (it != image_cache_.end()) {
        return it->second.get();
    }

    // Load via getImagePixelsRGBA
    std::vector<uint8_t> pixels;
    uint32_t width = 0, height = 0;

    if (!getImagePixelsRGBA(file_path, pixels, width, height)) {
        return nullptr;
    }

    // Create ImageResource
    auto resource = std::make_unique<ImageResource>();
    resource->path = file_path;
    resource->width = width;
    resource->height = height;
    // Note: pixels are not stored in ImageResource; caller should use getImagePixelsRGBA

    ImageResource* ptr = resource.get();
    image_cache_[file_path] = std::move(resource);

    return ptr;
}

ImageResource* ResourceManager::loadImageFromMemory(const std::string& name,
                                                    const uint8_t* data,
                                                    size_t data_size) {
    if (!data || data_size == 0) {
        return nullptr;
    }

    // Check cache first
    auto it = image_cache_.find(name);
    if (it != image_cache_.end()) {
        return it->second.get();
    }

    // Get decoder from platform
    DongPlatform* platform = dong_platform_get();
    DongImageDecoder* decoder = dong_platform_get_image_decoder(platform);

    if (!decoder) {
        DONG_LOG_WARN("[ResourceManager] no image decoder registered");
        return nullptr;
    }

    // Decode image
    DongDecodedImage image = {};
    DongImageDecoderResult result = dong_image_decode(decoder, data, data_size, &image);

    if (result != DONG_IMAGE_OK) {
        DONG_LOG_ERROR("[ResourceManager] failed to decode image from memory: %s", name.c_str());
        return nullptr;
    }

    // Create ImageResource
    auto resource = std::make_unique<ImageResource>();
    resource->path = name;
    resource->width = image.width;
    resource->height = image.height;

    // Free decoded image (we only needed dimensions)
    dong_image_free(decoder, &image);

    ImageResource* ptr = resource.get();
    image_cache_[name] = std::move(resource);

    return ptr;
}

ImageResource* ResourceManager::getImage(const std::string& file_path) const {
    auto it = image_cache_.find(file_path);
    if (it != image_cache_.end()) {
        return it->second.get();
    }
    return nullptr;
}

FontResource* ResourceManager::loadFont(const std::string& font_name,
                                        const std::string& file_path,
                                        float size) {
    std::string cache_key = makeFontCacheKey(font_name, size);

    // Check cache
    auto it = font_cache_.find(cache_key);
    if (it != font_cache_.end()) {
        return it->second.get();
    }

    // Resolve font path using font resolver
    std::string resolved_path = resolveFontPath(file_path.empty() ? font_name : file_path);
    if (resolved_path.empty()) {
        DONG_LOG_WARN("[ResourceManager] could not resolve font path: %s", 
                      file_path.empty() ? font_name.c_str() : file_path.c_str());
        return nullptr;
    }

    // Load font using FreeType via font_metrics
    FT_Face face = getOrCreateDesignUnitsFace(resolved_path);
    if (!face) {
        DONG_LOG_WARN("[ResourceManager] failed to load font: %s", resolved_path.c_str());
        return nullptr;
    }

    // Create FontResource
    auto resource = std::make_unique<FontResource>();
    resource->font_name = font_name;
    resource->font_size = size;
    resource->file_path = resolved_path;
    
    // Create a sized face for this font size
    // Note: We reference the design face and create a new sized instance
    FT_Face sized_face = nullptr;
    FT_Library ft_lib = getSharedFreeTypeLibrary();
    if (ft_lib) {
        FT_Error error = FT_New_Face(ft_lib, resolved_path.c_str(), 0, &sized_face);
        if (error != 0) {
            DONG_LOG_ERROR("[ResourceManager] FT_New_Face failed for '%s': error=%d",
                           resolved_path.c_str(), error);
            sized_face = nullptr;
        } else {
            DONG_LOG_INFO("[ResourceManager] FT_New_Face succeeded for '%s'", resolved_path.c_str());
        }
    } else {
        DONG_LOG_ERROR("[ResourceManager] FreeType library not initialized");
    }
    if (sized_face) {
        FT_Set_Pixel_Sizes(sized_face, 0, static_cast<FT_UInt>(size));
        resource->ft_face = sized_face;
    } else {
        // Fall back to design face if sized face creation fails
        DONG_LOG_WARN("[ResourceManager] Falling back to design face for '%s'", resolved_path.c_str());
        resource->ft_face = face;
    }

    FontResource* ptr = resource.get();
    font_cache_[cache_key] = std::move(resource);

    DONG_LOG_DEBUG("[ResourceManager] loaded font: %s @ %.1fpx -> %s", 
                   font_name.c_str(), size, resolved_path.c_str());

    return ptr;
}

FontResource* ResourceManager::getFont(const std::string& font_name, float size) const {
    std::string cache_key = makeFontCacheKey(font_name, size);
    auto it = font_cache_.find(cache_key);
    if (it != font_cache_.end()) {
        return it->second.get();
    }
    return nullptr;
}

FontResource* ResourceManager::getSystemFont(const std::string& font_family, float size) {
    std::string cache_key = makeFontCacheKey(font_family, size);

    // Check cache first
    auto it = font_cache_.find(cache_key);
    if (it != font_cache_.end()) {
        return it->second.get();
    }

    // Use font resolver to find system font
    std::string font_path = resolveFontPath(font_family);
    if (font_path.empty()) {
        DONG_LOG_WARN("[ResourceManager] system font not found: %s", font_family.c_str());
        return nullptr;
    }

    // Load the resolved font
    return loadFont(font_family, font_path, size);
}

void ResourceManager::clearImageCache() {
    image_cache_.clear();
}

void ResourceManager::clearFontCache() {
    font_cache_.clear();
}

void ResourceManager::clear() {
    clearImageCache();
    clearFontCache();
}

std::string ResourceManager::readFileBytes(const std::string& path, std::vector<uint8_t>& out_data) const {
    out_data.clear();

    // Use platform FileSystem
    DongPlatform* platform = dong_platform_get();
    DongFileSystem* fs = dong_platform_get_file_system(platform);

    DongFileData file_data = {nullptr, 0};
    DongFileSystemResult result = dong_fs_read_all(fs, path.c_str(), &file_data);

    if (result != DONG_FS_OK) {
        switch (result) {
            case DONG_FS_ERR_NOT_FOUND: return "File not found";
            case DONG_FS_ERR_IO: return "IO error";
            case DONG_FS_ERR_INVALID_ARG: return "Invalid argument";
            default: return "Unknown error";
        }
    }

    if (file_data.data && file_data.size > 0) {
        out_data.resize(file_data.size);
        std::memcpy(out_data.data(), file_data.data, file_data.size);
    }

    dong_fs_free_data(fs, &file_data);
    return ""; // Success
}

std::string ResourceManager::makeFontCacheKey(const std::string& family, float size) const {
    std::ostringstream oss;
    oss << family << "_" << static_cast<int>(size);
    return oss.str();
}

} // namespace dong::render
