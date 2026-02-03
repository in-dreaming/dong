#include "resource_manager.hpp"
#include "../core/log.h"
#include <fstream>

#include <sstream>
#include <iostream>
#include <cstring>
#include <memory>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <cstdlib>



// Optional: use SDL's built-in PNG loader in the legacy (SDL-coupled) build.
#if defined(DONG_USE_SDL_IMAGE)
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#endif



// TODO: 使用 FreeType + GlyphAtlas 替代字体管理


namespace dong::render {

// ImageResource destructor
ImageResource::~ImageResource() {
    // TODO: 清理图片资源
}

// FontResource destructor
FontResource::~FontResource() {
    // TODO: 清理字体资源
}

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager() {
    clear();
}

bool ResourceManager::getImagePixelsRGBA(const std::string& file_path,
                                         std::vector<uint8_t>& out_pixels,
                                         uint32_t& out_width,
                                         uint32_t& out_height) {
    out_pixels.clear();
    out_width = 0;
    out_height = 0;

    auto trimCopy = [](std::string s) -> std::string {
        auto not_space = [](unsigned char c) { return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
        s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
        return s;
    };

    std::string path = trimCopy(file_path);
    if (path.empty()) {
        return false;
    }

    auto isAbsolutePath = [](const std::string& p) -> bool {
        if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':') {
            return true; // Windows drive path
        }
        if (!p.empty() && (p[0] == '/' || p[0] == '\\')) {
            return true; // Unix absolute or UNC-like
        }
        return false;
    };


    // Basic URL handling: support file://, reject http(s)/data.
    if (path.rfind("http://", 0) == 0 || path.rfind("https://", 0) == 0 || path.rfind("data:", 0) == 0) {
        DONG_LOG_WARN("[ResourceManager] unsupported image URL: %s", path.c_str());

        return false;
    }
    if (path.rfind("file://", 0) == 0) {
        path = path.substr(std::string("file://").size());
        // Windows file URL often looks like "/d:/xxx"; strip the leading '/' if it precedes a drive letter.
        if (path.size() >= 3 && path[0] == '/' && std::isalpha(static_cast<unsigned char>(path[1])) && path[2] == ':') {
            path.erase(path.begin());
        }
    }

    // Resolve relative paths against optional resource root.
    if (!resource_root_.empty() && !isAbsolutePath(path)) {
        const std::string orig = path;
        try {
            namespace fs = std::filesystem;
            fs::path resolved = fs::path(resource_root_) / fs::path(path);
            path = resolved.lexically_normal().string();
        } catch (...) {
            // Best-effort; fall back to the original path.
        }
        if (orig != path) {
            const char* env = std::getenv("DONG_DEBUG_RESOURCE_PATHS");
            if (env && env[0] == '1') {
                DONG_LOG_INFO("[ResourceManager] resolve: root=%s path=%s -> %s", resource_root_.c_str(), orig.c_str(), path.c_str());

            }
        }

    }



#if defined(DONG_USE_SDL_IMAGE)
    SDL_Surface* surface = SDL_LoadPNG(path.c_str());
    if (!surface) {
        DONG_LOG_ERROR("[ResourceManager] SDL_LoadPNG failed: %s (%s)", path.c_str(), SDL_GetError());

        return false;
    }

    SDL_Surface* rgba = surface;
    if (surface->format != SDL_PIXELFORMAT_RGBA32) {
        rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(surface);
        surface = nullptr;
        if (!rgba) {
            DONG_LOG_ERROR("[ResourceManager] SDL_ConvertSurface(RGBA32) failed: %s (%s)", path.c_str(), SDL_GetError());

            return false;
        }
    }

    if (SDL_MUSTLOCK(rgba)) {
        if (!SDL_LockSurface(rgba)) {
            DONG_LOG_ERROR("[ResourceManager] SDL_LockSurface failed: %s (%s)", path.c_str(), SDL_GetError());

            SDL_DestroySurface(rgba);
            return false;
        }
    }

    const int w = rgba->w;
    const int h = rgba->h;
    if (!rgba->pixels || w <= 0 || h <= 0) {
        if (SDL_MUSTLOCK(rgba)) {
            SDL_UnlockSurface(rgba);
        }
        SDL_DestroySurface(rgba);
        return false;
    }

    out_width = static_cast<uint32_t>(w);
    out_height = static_cast<uint32_t>(h);
    out_pixels.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);

    const uint8_t* src = static_cast<const uint8_t*>(rgba->pixels);
    const int src_pitch = rgba->pitch;
    const size_t dst_pitch = static_cast<size_t>(w) * 4;

    for (int y = 0; y < h; ++y) {
        std::memcpy(out_pixels.data() + static_cast<size_t>(y) * dst_pitch,
                    src + static_cast<size_t>(y) * static_cast<size_t>(src_pitch),
                    dst_pitch);
    }

    if (SDL_MUSTLOCK(rgba)) {
        SDL_UnlockSurface(rgba);
    }
    SDL_DestroySurface(rgba);
    return true;
#else
    // dong core build intentionally avoids SDL linkage. Image decoding is not available here.
    DONG_LOG_WARN("[ResourceManager] image decoding disabled (build without DONG_USE_SDL_IMAGE): %s", path.c_str());

    return false;
#endif


}

ImageResource* ResourceManager::loadImage(const std::string& file_path) {
    // Check cache first
    auto it = image_cache_.find(file_path);
    if (it != image_cache_.end()) {
        return it->second.get();
    }
    
    // TODO: 使用 stb_image 加载图片
    DONG_LOG_WARN("[ResourceManager] loadImage not implemented: %s", file_path.c_str());

    return nullptr;
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
    
    // TODO: 使用 stb_image 解码
    DONG_LOG_WARN("[ResourceManager] loadImageFromMemory not implemented: %s", name.c_str());

    return nullptr;
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
    
    // TODO: 使用 FreeType 加载字体
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
    // TODO: 使用 FreeType + 系统字体查找
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
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return "Failed to open file";
    }
    
    auto size = file.tellg();
    if (size <= 0) {
        return "File is empty or invalid";
    }
    
    file.seekg(0, std::ios::beg);
    out_data.resize(static_cast<size_t>(size));
    
    if (!file.read(reinterpret_cast<char*>(out_data.data()), size)) {
        out_data.clear();
        return "Failed to read file";
    }
    
    return ""; // Success
}

std::string ResourceManager::makeFontCacheKey(const std::string& family, float size) const {
    std::ostringstream oss;
    oss << family << "_" << static_cast<int>(size);
    return oss.str();
}

} // namespace dong::render
