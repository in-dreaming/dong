#include "resource_manager.hpp"
#include "../core/log.h"
#include "dong_platform.h"
#include "dong_file_system.h"
#include "dong_image_decoder.h"

#include <sstream>
#include <cstring>
#include <algorithm>
#include <cctype>

namespace dong::render {

// ImageResource destructor
ImageResource::~ImageResource() {
    // Pixel data is managed separately (usually by atlas or caller)
}

// FontResource destructor
FontResource::~FontResource() {
    // Font resources are managed by FreeType/HarfBuzz
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
    if (path.rfind("http://", 0) == 0 || path.rfind("https://", 0) == 0 || path.rfind("data:", 0) == 0) {
        DONG_LOG_WARN("[ResourceManager] unsupported image URL: %s", path.c_str());
        return false;
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

    // TODO: Use FreeType to load font (requires IFontLoader abstraction)
    DONG_LOG_WARN("[ResourceManager] loadFont not implemented: %s", font_name.c_str());
    return nullptr;
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
    // TODO: Use FontFinder + FreeType
    DONG_LOG_WARN("[ResourceManager] getSystemFont not implemented: %s", font_family.c_str());
    return nullptr;
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
