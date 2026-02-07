#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <vector>

// Forward declare FreeType types
struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;

namespace dong::render {

// Image cache entry
struct ImageResource {
    std::string path;
    uint32_t width = 0;
    uint32_t height = 0;
    void* sk_image = nullptr;  // SkImage* (opaque)
    
    ~ImageResource();
};

// Font resource entry
struct FontResource {
    std::string font_name;
    float font_size = 16.0f;
    std::string file_path;      // Font file path
    FT_Face ft_face = nullptr;  // FreeType face (loaded at specific size)
    
    ~FontResource();
};

// Resource manager for images, fonts, and other assets
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    // Optional: set a base directory for resolving relative resource paths.
    // Example: if base is ".../examples/data/tests", then "../images/bg.png" works.
    void setResourceRoot(const std::string& root) { resource_root_ = root; }
    const std::string& getResourceRoot() const { return resource_root_; }
    
    // Image loading and caching

    // Returns nullptr if file cannot be loaded or decoded
    ImageResource* loadImage(const std::string& file_path);
    
    // Get cached image without loading
    ImageResource* getImage(const std::string& file_path) const;
    
    // Load image from memory buffer
    ImageResource* loadImageFromMemory(const std::string& name, 
                                       const uint8_t* data, 
                                       size_t data_size);
    
    // Font management
    FontResource* loadFont(const std::string& font_name, const std::string& file_path, float size);
    FontResource* getFont(const std::string& font_name, float size) const;
    
    // System font lookup (platform-specific)
    FontResource* getSystemFont(const std::string& font_family, float size);
    
    // Cache clearing
    void clearImageCache();
    void clearFontCache();
    void clear();
    
    // Statistics
    size_t getImageCacheSize() const { return image_cache_.size(); }
    size_t getFontCacheSize() const { return font_cache_.size(); }

    // 获取指定图片的 RGBA 像素数据（用于 GPU atlas 构建）
    // 返回 true 表示成功填充 out_pixels / out_width / out_height
    bool getImagePixelsRGBA(const std::string& file_path,
                            std::vector<uint8_t>& out_pixels,
                            uint32_t& out_width,
                            uint32_t& out_height);

private:
    // Image cache: path -> ImageResource
    std::unordered_map<std::string, std::unique_ptr<ImageResource>> image_cache_;
    
    // Font cache: "family:size" -> FontResource  
    std::unordered_map<std::string, std::unique_ptr<FontResource>> font_cache_;
    
    // Helper functions
    std::string readFileBytes(const std::string& path, std::vector<uint8_t>& out_data) const;
    std::string makeFontCacheKey(const std::string& family, float size) const;

    std::string resource_root_;
};


using ResourceManagerPtr = std::unique_ptr<ResourceManager>;

} // namespace dong::render
