#include "painter.hpp"
#include "skia_backend.hpp"
#include <cstring>
#include <iostream>

namespace dong::render {

Painter::Painter(RenderSurface* surface)
    : surface_(surface), in_frame_(false), current_opacity_(1.0f),
      sk_canvas_(nullptr), sk_surface_(nullptr) {
    // 初始化 Skia 后端
    auto backend = std::make_unique<SkiaBackend>(surface);
    if (backend && backend->initialize()) {
        sk_canvas_ = reinterpret_cast<void*>(backend->getCanvas());
        sk_surface_ = reinterpret_cast<void*>(backend->getSurface());
        skia_backend_ = std::move(backend);
    }
}

Painter::~Painter() {
    // Skia 资源由 SkiaBackend 析构函数处理
    skia_backend_.reset();
}

void Painter::beginFrame() {
    if (!surface_) return;

    in_frame_ = true;
    current_opacity_ = 1.0f;

    // 清空表面
    surface_->lock();
    surface_->clear(255, 255, 255, 255); // 白色背景
    surface_->unlock();
}

void Painter::endFrame() {
    if (!surface_) return;

    in_frame_ = false;

    // 提交 Skia 绘制
    if (skia_backend_) {
        skia_backend_->flush();
    }

    // 标记表面已更新
    surface_->unlock();
}

void Painter::renderDOM(const dom::DOMNodePtr& root, layout::Engine* layout_engine) {
    if (!root) return;

    beginFrame();

    // If no layout engine provided, we still render with nullptr layout info
    // This allows testing DOM rendering without layout
    if (layout_engine) {
        const auto* layout_root = layout_engine->getLayout(root);
        if (layout_root) {
            renderNode(root, layout_root);
        } else {
            // Layout not computed yet, skip rendering
            std::cerr << "Warning: Layout not computed for DOM root\n";
        }
    } else {
        // Render with dummy layout (all zeros)
        // This is fallback behavior if layout_engine is not provided
        renderNode(root, nullptr);
    }

    endFrame();
}

void Painter::renderNode(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node) return;

    // If layout_node is provided, use it for positioning and sizing
    if (layout_node) {
        // 绘制背景
        drawNodeBackground(node, layout_node);

        // 绘制边框
        drawNodeBorder(node, layout_node);

        // 绘制内容（文本、图片等）
        drawNodeContent(node, layout_node);
    }

    // 递归绘制子节点
    const auto& children = node->getChildren();
    if (layout_node && layout_node->first_child) {
        const auto* child_layout = layout_node->first_child;
        for (size_t i = 0; i < children.size(); ++i) {
            if (child_layout) {
                renderNode(children[i], child_layout);
                child_layout = child_layout->next_sibling;
            }
        }
    } else {
        // No layout info, render children without layout
        for (const auto& child : children) {
            renderNode(child, nullptr);
        }
    }
}

void Painter::drawNodeBackground(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node || !layout_node) return;

    const auto& style = node->getComputedStyle();
    float x = layout_node->layout.position[0];
    float y = layout_node->layout.position[1];
    float width = layout_node->layout.dimensions[0];
    float height = layout_node->layout.dimensions[1];

    // 绘制背景色
    if (style.background_color != "transparent") {
        // 简单的颜色解析（仅支持 #RRGGBB 格式）
        uint8_t r = 200, g = 200, b = 200; // 默认灰色
        if (style.background_color == "red") {
            r = 255; g = 0; b = 0;
        } else if (style.background_color == "blue") {
            r = 0; g = 0; b = 255;
        } else if (style.background_color == "green") {
            r = 0; g = 255; b = 0;
        }

        drawRect(x, y, width, height, r, g, b);
    }
}

void Painter::drawNodeBorder(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node || !layout_node || !skia_backend_) return;

    const auto& style = node->getComputedStyle();
    float x = layout_node->layout.position[0];
    float y = layout_node->layout.position[1];
    float width = layout_node->layout.dimensions[0];
    float height = layout_node->layout.dimensions[1];

    // 简单的边框支持（仅支持实线边框）
    if (style.border_width > 0 && style.border_color != "transparent") {
        uint8_t r = 0, g = 0, b = 0; // 默认黑色
        if (style.border_color == "red") {
            r = 255; g = 0; b = 0;
        } else if (style.border_color == "blue") {
            r = 0; g = 0; b = 255;
        } else if (style.border_color == "green") {
            r = 0; g = 255; b = 0;
        }

        skia_backend_->drawStroke(x, y, width, height, style.border_width, r, g, b);
    }

    // 圆角支持
    if (style.border_radius > 0) {
        // 使用 Skia 的圆角矩形（绘制在背景之后作为装饰）
        // 这里我们绘制一个透明的圆角边框
        skia_backend_->drawRoundRect(x, y, width, height, style.border_radius,
                                      50, 50, 50, 0, 0.5f);
    }
}

void Painter::drawNodeContent(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node || !layout_node) return;

    std::string tag = node->getTagName();

    // 如果是文本节点或包含文本内容
    if (node->getType() == dom::DOMNode::NodeType::TEXT) {
        std::string text = node->getTextContent();
        if (!text.empty()) {
            const auto& style = node->getComputedStyle();
            float x = layout_node->layout.position[0];
            float y = layout_node->layout.position[1];
            float font_size = style.font_size;

            drawText(text, x, y, font_size, 0, 0, 0); // 黑色文本
        }
    }
    // 如果是图片标签
    else if (tag == "img") {
        if (node->hasAttribute("src")) {
            std::string src = node->getAttribute("src");
            float x = layout_node->layout.position[0];
            float y = layout_node->layout.position[1];
            float width = layout_node->layout.dimensions[0];
            float height = layout_node->layout.dimensions[1];

            drawImage(src, x, y, width, height);
        }
    }
}

void Painter::drawRect(float x, float y, float width, float height,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (skia_backend_) {
        skia_backend_->drawRect(x, y, width, height, r, g, b, a);
    }
}

void Painter::drawText(const std::string& text, float x, float y, float font_size,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (skia_backend_) {
        skia_backend_->drawText(text, x, y, font_size, r, g, b, a);
    }
}

void Painter::drawImage(const std::string& image_path, float x, float y,
                        float width, float height) {
    if (skia_backend_) {
        skia_backend_->drawImage(image_path, x, y, width, height);
    }
}

void Painter::setClipRect(float x, float y, float width, float height) {
    if (skia_backend_) {
        skia_backend_->saveState();
        skia_backend_->clipRect(x, y, width, height);
    }
}

void Painter::clearClipRect() {
    if (skia_backend_) {
        skia_backend_->restoreState();
    }
}

void Painter::setOpacity(float opacity) {
    current_opacity_ = opacity;
}

} // namespace dong::render
