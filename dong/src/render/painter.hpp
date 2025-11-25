#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../dom/dom_node.hpp"
#include "../layout/layout_engine.hpp"
#include "render_surface.hpp"
#include "display_list.hpp"

namespace dong::render {

// Forward declaration
class SkiaBackend;
class ResourceManager;

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

    // 从 DisplayList 绘制一帧（用于新 GPU 管线以及 CPU 调试）
    void renderDisplayList(const DisplayList& list);

    // 访问最近一帧构建好的 DisplayList（只读视图，用于 GPU 管线编译）
    const DisplayList& getDisplayList() const { return display_list_builder_.get(); }

    // GPU 管线用：访问 Skia 后端内部的资源管理器（图片像素复用 CPU 解码缓存）
    ResourceManager* getResourceManager() const;

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
    layout::Engine* layout_engine_;
    bool in_frame_;
    float current_opacity_;
    layout::DirtyRect current_dirty_rect_;
    bool use_dirty_rect_ = true;

    // DisplayList 构建器（从 DOM/Layout 生成渲染指令）
    DisplayListBuilder display_list_builder_;

    // Skia 对象
    void* sk_canvas_;  // SkCanvas*
    void* sk_surface_; // SkSurface*
    std::unique_ptr<SkiaBackend> skia_backend_;

    // 递归绘制 DOM 节点（直接 Skia 渲染路径）
    void renderNode(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node);

    // 根据样式绘制背景、边框等（Skia 路径）
    void drawNodeBackground(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node);
    void drawNodeBorder(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node);
    void drawNodeContent(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node);

    // 从 DOM/Layout 构建 DisplayList（供 GPU 管线和 CPU 回放共用）
    void buildDisplayListNode(const dom::DOMNodePtr& node,
                              const layout::LayoutNode* layout_node,
                              DisplayListBuilder& builder);

    // 脏矩形优化
    bool isNodeInDirtyRect(const layout::LayoutNode* layout_node) const;
};

using PainterPtr = std::unique_ptr<Painter>;

} // namespace dong::render
