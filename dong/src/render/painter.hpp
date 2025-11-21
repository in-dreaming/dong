#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../dom/dom_node.hpp"
#include "../layout/layout_engine.hpp"
#include "render_surface.hpp"

namespace dong::render {

// Forward declaration
class SkiaBackend;

// Skia 绘制器
class Painter {
public:
    Painter(RenderSurface* surface);
    ~Painter();

    // 开始绘制一帧
    void beginFrame();

    // 结束绘制一帧（提交到表面）
    void endFrame();

    // 绘制 DOM 树
    void renderDOM(const dom::DOMNodePtr& root, layout::Engine* layout_engine);

    // 低级绘制原语
    void drawRect(float x, float y, float width, float height,
                  uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void drawText(const std::string& text, float x, float y, float font_size,
                  uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void drawImage(const std::string& image_path, float x, float y, 
                   float width, float height);

    // 设置裁剪区域
    void setClipRect(float x, float y, float width, float height);
    void clearClipRect();

    // 设置透明度
    void setOpacity(float opacity);

private:
    RenderSurface* surface_;
    bool in_frame_;
    float current_opacity_;

    // Skia 对象
    void* sk_canvas_;  // SkCanvas*
    void* sk_surface_; // SkSurface*
    std::unique_ptr<SkiaBackend> skia_backend_;

    // 递归绘制 DOM 节点
    void renderNode(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node);

    // 根据样式绘制背景、边框等
    void drawNodeBackground(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node);
    void drawNodeBorder(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node);
    void drawNodeContent(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node);
};

using PainterPtr = std::unique_ptr<Painter>;

} // namespace dong::render
