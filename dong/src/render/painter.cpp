#include "painter.hpp"
#include "skia_backend.hpp"
#include <cstring>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <cstdio>

namespace dong::render {

Painter::Painter(RenderSurface* surface)
    : surface_(surface), in_frame_(false), current_opacity_(1.0f),
      sk_canvas_(nullptr), sk_surface_(nullptr) {
    // Initialize Skia backend
    auto backend = std::make_unique<SkiaBackend>(surface);
    if (backend && backend->initialize()) {
        sk_canvas_ = reinterpret_cast<void*>(backend->getCanvas());
        sk_surface_ = reinterpret_cast<void*>(backend->getSurface());
        skia_backend_ = std::move(backend);
    }
}

Painter::~Painter() {
    // Skia resources cleaned up by SkiaBackend destructor
    skia_backend_.reset();
}

void Painter::beginFrame() {
    if (!surface_) return;

    in_frame_ = true;
    current_opacity_ = 1.0f;

    // Clear surface
    surface_->lock();
    surface_->clear(255, 255, 255, 255); // White background
    surface_->unlock();
}

void Painter::endFrame() {
    if (!surface_) return;

    in_frame_ = false;

    // Commit Skia drawing
    if (skia_backend_) {
        skia_backend_->flush();
    }

    // Mark surface as updated
    surface_->unlock();
}

void Painter::renderDOM(const dom::DOMNodePtr& root, layout::Engine* layout_engine) {
    if (!root) return;

    std::fprintf(stderr, "[Painter] renderDOM begin\n");
    beginFrame();

    if (layout_engine) {
        const auto* layout_root = layout_engine->getLayout(root);
        if (layout_root) {
            std::fprintf(stderr, "[Painter] layout root ready\n");
            renderNode(root, layout_root);
            std::fprintf(stderr, "[Painter] renderNode finished\n");
        } else {
            std::fprintf(stderr, "[Painter] missing layout info\n");
        }
    } else {
        std::fprintf(stderr, "[Painter] layout engine missing\n");
        renderNode(root, nullptr);
    }

    endFrame();
    std::fprintf(stderr, "[Painter] renderDOM end\n");
}

void Painter::renderNode(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node) return;

    std::fprintf(stderr, "[Painter] renderNode type=%d tag=%s layout=%p\n",
        static_cast<int>(node->getType()), node->getTagName().c_str(), static_cast<const void*>(layout_node));

    // Skip text nodes (rendered as part of parent element content)
    if (node->getType() == dom::DOMNode::NodeType::TEXT) {
        return;
    }

    // Draw background and borders if layout provided
    if (layout_node) {
        std::fprintf(stderr, "[Painter] draw background\n");
        drawNodeBackground(node, layout_node);
        std::fprintf(stderr, "[Painter] draw border\n");
        drawNodeBorder(node, layout_node);
        std::fprintf(stderr, "[Painter] draw content\n");
        drawNodeContent(node, layout_node);
    }

    // Recursively render children
    const auto& children = node->getChildren();
    for (const auto& child : children) {
        renderNode(child, nullptr);
    }
}

void Painter::drawNodeBackground(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    // Temporarily disabled background painting to stabilize demo
    (void)node;
    (void)layout_node;
}

void Painter::drawNodeBorder(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node || !layout_node || !skia_backend_) return;

    const auto& style = node->getComputedStyle();
    float x = layout_node->layout.position[0];
    float y = layout_node->layout.position[1];
    float width = layout_node->layout.dimensions[0];
    float height = layout_node->layout.dimensions[1];

    // Draw border
    if (style.border_width > 0 && style.border_color != "transparent") {
        uint8_t r = 0, g = 0, b = 0;
        if (style.border_color == "red") { r = 255; }
        else if (style.border_color == "blue") { b = 255; }
        else if (style.border_color == "green") { g = 255; }

        skia_backend_->drawStroke(x, y, width, height, style.border_width, r, g, b);
    }

    // Draw rounded corners
    if (style.border_radius > 0) {
        skia_backend_->drawRoundRect(x, y, width, height, style.border_radius,
                                      50, 50, 50, 0, 0.5f);
    }
}

void Painter::drawNodeContent(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node || !layout_node) return;

    std::string tag = node->getTagName();

    // Handle text nodes
    if (node->getType() == dom::DOMNode::NodeType::TEXT) {
        std::string text = node->getTextContent();
        if (!text.empty()) {
            const auto& style = node->getComputedStyle();
            float x = layout_node->layout.position[0];
            float y = layout_node->layout.position[1];
            float font_size = style.font_size;

            drawText(text, x, y, font_size, 0, 0, 0);
        }
    }
    // Handle image elements
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
