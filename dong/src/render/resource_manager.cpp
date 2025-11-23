#include "resource_manager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <memory>

#include <core/SkImage.h>
#include <core/SkData.h>
#include <core/SkTypeface.h>
#include <core/SkFontMgr.h>
#include <core/SkStream.h>
#include <codec/SkCodec.h>
#include <core/SkPixmap.h>
#include <core/SkColor.h>
#if defined(__APPLE__)
#include <ports/SkFontMgr_mac_ct.h>
#endif

namespace dong::render {

// ImageResource destructor
ImageResource::~ImageResource() {
    if (sk_image) {
        SkImage* img = reinterpret_cast<SkImage*>(sk_image);
        SkSafeUnref(img);
    }
}

// FontResource destructor
FontResource::~FontResource() {
    if (sk_typeface) {
        SkTypeface* tf = reinterpret_cast<SkTypeface*>(sk_typeface);
        SkSafeUnref(tf);
    }
}

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager() {
    clear();
}

ImageResource* ResourceManager::loadImage(const std::string& file_path) {
    // Check cache first
    auto it = image_cache_.find(file_path);
    if (it != image_cache_.end()) {
        return it->second.get();
    }
    
    // Try to read file
    std::vector<uint8_t> data;
    if (readFileBytes(file_path, data).empty() == false) {
        return nullptr;
    }
    
    // Try to load from memory
    return loadImageFromMemory(file_path, data.data(), data.size());
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
    
    // Create SkData from buffer
    sk_sp<SkData> sk_data = SkData::MakeWithCopy(data, data_size);
    if (!sk_data) {
        std::cerr << "[ResourceManager] Failed to create SkData for: " << name << std::endl;
        return nullptr;
    }
    
    // Try to decode image directly from SkData using DeferredFromEncodedData
    sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(sk_data);
    if (!image) {
        std::cerr << "[ResourceManager] Failed to decode image: " << name << std::endl;
        // Create placeholder: 32x32 red rectangle
        SkImageInfo info = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
        std::vector<uint32_t> red_pixels(32 * 32, 0xFFFF0000);
        SkPixmap pixmap(info, red_pixels.data(), 32 * 4);
        image = SkImages::RasterFromPixmapCopy(pixmap);
        
        if (image) {
            auto resource = std::make_unique<ImageResource>();
            resource->path = name;
            resource->width = 32;
            resource->height = 32;
            resource->sk_image = image.release();
            ImageResource* ptr = resource.get();
            image_cache_[name] = std::move(resource);
            std::cout << "[ResourceManager] Created placeholder for: " << name << std::endl;
            return ptr;
        }
        return nullptr;
    }
    
    // Cache the decoded image
    int width = image->width();
    int height = image->height();
    
    auto resource = std::make_unique<ImageResource>();
    resource->path = name;
    resource->width = width;
    resource->height = height;
    resource->sk_image = image.release();
    
    ImageResource* ptr = resource.get();
    image_cache_[name] = std::move(resource);
    
    std::cout << "[ResourceManager] Loaded image: " << name << " (" << width << "x" << height << ")" << std::endl;
    
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
    
    // Try to load font file
    std::vector<uint8_t> font_data;
    std::string err = readFileBytes(file_path, font_data);
    if (!err.empty()) {
        std::cerr << "[ResourceManager] Failed to load font file: " << file_path << std::endl;
        return nullptr;
    }
    
    // Create SkData
    sk_sp<SkData> sk_data = SkData::MakeWithCopy(font_data.data(), font_data.size());
    if (!sk_data) {
        return nullptr;
    }
    
    // Get system font manager
#if defined(__APPLE__)
    sk_sp<SkFontMgr> mgr = SkFontMgr_New_CoreText(nullptr);
#else
    sk_sp<SkFontMgr> mgr = SkFontMgr::RefEmpty();
#endif
    
    if (!mgr) {
        return nullptr;
    }
    
    // Try to create typeface from data
    sk_sp<SkTypeface> typeface = mgr->makeFromData(sk_data);
    if (!typeface) {
        // Fallback to default
        typeface = mgr->legacyMakeTypeface(font_name.c_str(), SkFontStyle::Normal());
    }
    
    if (!typeface) {
        return nullptr;
    }
    
    // Store in cache
    auto resource = std::make_unique<FontResource>();
    resource->font_name = font_name;
    resource->font_size = size;
    resource->sk_typeface = typeface.release();
    
    FontResource* ptr = resource.get();
    font_cache_[cache_key] = std::move(resource);
    
    std::cout << "[ResourceManager] Loaded font: " << font_name << " (" << size << "pt)" << std::endl;
    
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
    
    // Check cache
    auto it = font_cache_.find(cache_key);
    if (it != font_cache_.end()) {
        return it->second.get();
    }
    
    // Get system font manager
#if defined(__APPLE__)
    sk_sp<SkFontMgr> mgr = SkFontMgr_New_CoreText(nullptr);
#else
    sk_sp<SkFontMgr> mgr = SkFontMgr::RefEmpty();
#endif
    
    if (!mgr) {
        return nullptr;
    }
    
    // Try to find typeface by family name
    sk_sp<SkTypeface> typeface = mgr->matchFamilyStyle(font_family.c_str(), SkFontStyle::Normal());
    if (!typeface) {
        // Fallback to system sans-serif
        typeface = mgr->matchFamilyStyle("system-ui", SkFontStyle::Normal());
    }
    if (!typeface) {
        // Last resort: default
        typeface = mgr->legacyMakeTypeface(nullptr, SkFontStyle::Normal());
    }
    
    if (!typeface) {
        return nullptr;
    }
    
    // Store in cache
    auto resource = std::make_unique<FontResource>();
    resource->font_name = font_family;
    resource->font_size = size;
    resource->sk_typeface = typeface.release();
    
    FontResource* ptr = resource.get();
    font_cache_[cache_key] = std::move(resource);
    
    return ptr;
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
        return "Cannot open file: " + path;
    }
    
    std::streamsize size = file.tellg();
    if (size <= 0) {
        return "Invalid file size: " + path;
    }
    
    file.seekg(0, std::ios::beg);
    out_data.resize(size);
    
    if (!file.read(reinterpret_cast<char*>(out_data.data()), size)) {
        return "Failed to read file: " + path;
    }
    
    return "";  // Success
}

std::string ResourceManager::makeFontCacheKey(const std::string& family, float size) const {
    std::ostringstream oss;
    oss << family << ":" << size;
    return oss.str();
}

} // namespace dong::render
