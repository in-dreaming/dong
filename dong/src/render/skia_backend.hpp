#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "render_surface.hpp"

namespace dong::render {

// Forward declarations
class CPUBufferSurface;

// Skia 后端管理器，处理文本、图片、绘制等
class SkiaBackend {
public:
    SkiaBackend(RenderSurface* surface);
    ~SkiaBackend();

    // 初始化 Skia 表面和画布
    bool initialize();

    // 获取画布指针
    void* getCanvas() const { return sk_canvas_; }
    void* getSurface() const { return sk_surface_; }

    // 设置默认字体
    void setDefaultFont(const std::string& font_name, float font_size);

    // 文本渲染 - 使用 SkTextBlob 进行简单文本渲染
    void drawText(const std::string& text, float x, float y,
                  float font_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // 带字体信息的文本渲染（支持 font-family / font-weight 等）
    void drawTextStyled(const std::string& text, float x, float y,
                        float font_size, const std::string& font_family,
                        const std::string& font_weight,
                        uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // 使用 SkParagraph 进行高级文本布局（换行、对齐等）
    void drawParagraph(const std::string& text, float x, float y, float max_width,
                       float font_size, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // 矩形绘制
    void drawRect(float x, float y, float width, float height,
                  uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255, float stroke_width = 0);

    // 圆角矩形
    void drawRoundRect(float x, float y, float width, float height, float radius,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255, float stroke_width = 0);

    // 边框绘制
    void drawStroke(float x, float y, float width, float height, float stroke_width,
                    uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // 阴影绘制（使用 Skia 的 MaskFilter）
    void drawShadow(float x, float y, float width, float height, float blur_radius,
                    float offset_x, float offset_y, uint8_t shadow_color_alpha = 100);

    // 图片加载和绘制
    void drawImage(const std::string& image_path, float x, float y,
                   float width, float height, uint8_t alpha = 255);

    // 状态管理
    void saveState();
    void restoreState();
    void clipRect(float x, float y, float width, float height);

    // 提交绘制到表面
    void flush();

private:
    RenderSurface* render_surface_;
    void* sk_canvas_;    // SkCanvas* (opaque)
    void* sk_surface_;   // SkSurface* (opaque)
    void* default_font_; // SkFont* (opaque)
    float default_font_size_;

    // 缓存存储（使用 opaque pointers）
    std::unordered_map<std::string, void*> font_cache_;   // SkTypeface* cache
    std::unordered_map<std::string, void*> image_cache_;  // SkImage* cache

    // 辅助函数
    void* loadTypeface(const std::string& font_name);
    void* loadImage(const std::string& image_path);
};

using SkiaBackendPtr = std::unique_ptr<SkiaBackend>;

} // namespace dong::render
