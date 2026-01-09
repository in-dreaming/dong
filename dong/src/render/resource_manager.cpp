#include "resource_manager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <memory>

// TODO: 集成 stb_image 用于图片加载
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
    // TODO: 使用 stb_image 实现
    std::cerr << "[ResourceManager] getImagePixelsRGBA not implemented: " << file_path << std::endl;
    out_width = 0;
    out_height = 0;
    return false;
}

ImageResource* ResourceManager::loadImage(const std::string& file_path) {
    // Check cache first
    auto it = image_cache_.find(file_path);
    if (it != image_cache_.end()) {
        return it->second.get();
    }
    
    // TODO: 使用 stb_image 加载图片
    std::cerr << "[ResourceManager] loadImage not implemented: " << file_path << std::endl;
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
    std::cerr << "[ResourceManager] loadImageFromMemory not implemented: " << name << std::endl;
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
    std::cerr << "[ResourceManager] loadFont not implemented: " << font_name << std::endl;
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
    std::cerr << "[ResourceManager] getSystemFont not implemented: " << font_family << std::endl;
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
