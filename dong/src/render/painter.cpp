#include "painter.hpp"
#include "skia_backend.hpp"
#include <cstring>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <cstdio>
#include <algorithm>
#include <sstream>
#include <cctype>

namespace dong::render {

Painter::Painter(RenderSurface* surface)
    : surface_(surface), layout_engine_(nullptr), in_frame_(false), current_opacity_(1.0f),
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

    layout_engine_ = layout_engine;

    std::fprintf(stderr, "[Painter] renderDOM begin\n");
    beginFrame();

    if (layout_engine_) {
        const auto* layout_root = layout_engine_->getLayout(root);
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
    layout_engine_ = nullptr;
    std::fprintf(stderr, "[Painter] renderDOM end\n");
}

void Painter::renderNode(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node) return;

    const auto& style = node->getComputedStyle();
    std::fprintf(stderr, "[Painter] renderNode type=%d tag=%s layout=%p bg=%s\n",
        static_cast<int>(node->getType()), node->getTagName().c_str(), static_cast<const void*>(layout_node),
        style.background_color.c_str());

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
        const layout::LayoutNode* child_layout = nullptr;
        if (layout_engine_ && child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            child_layout = layout_engine_->getLayout(child);
        }
        renderNode(child, child_layout);
    }
}

namespace {

// 简单 CSS 颜色解析器，支持 #rgb/#rrggbb 和少量命名色
static void parseCssColor(const std::string& css, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    std::string s = css;
    a = 255;

    // 去空格并转小写
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    if (s.empty() || s == "transparent") {
        r = g = b = 0;
        a = 0;
        return;
    }

    if (!s.empty() && s[0] == '#') {
        if (s.size() == 4) { // #rgb
            auto hex = [](char c) -> uint8_t {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
                return 0;
            };
            uint8_t r4 = hex(s[1]);
            uint8_t g4 = hex(s[2]);
            uint8_t b4 = hex(s[3]);
            r = (r4 << 4) | r4;
            g = (g4 << 4) | g4;
            b = (b4 << 4) | b4;
            return;
        } else if (s.size() == 7) { // #rrggbb
            auto hex2 = [](char c1, char c2) -> uint8_t {
                auto hex = [](char c) -> uint8_t {
                    if (c >= '0' && c <= '9') return c - '0';
                    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
                    return 0;
                };
                return static_cast<uint8_t>((hex(c1) << 4) | hex(c2));
            };
            r = hex2(s[1], s[2]);
            g = hex2(s[3], s[4]);
            b = hex2(s[5], s[6]);
            return;
        }
    }

    // 简单命名色
    if (s == "white")      { r = g = b = 255; return; }
    if (s == "black")      { r = g = b = 0;   return; }
    if (s == "red")        { r = 255; g = 0;   b = 0;   return; }
    if (s == "green")      { r = 0;   g = 128; b = 0;   return; }
    if (s == "blue")       { r = 0;   g = 0;   b = 255; return; }
    if (s == "gray" || s == "grey") { r = g = b = 128; return; }
    if (s == "lightgray" || s == "lightgrey") { r = g = b = 211; return; }

    // 兜底：浅灰
    r = g = b = 240;
}

} // anonymous namespace

void Painter::drawNodeBackground(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node || !layout_node || !skia_backend_) return;

    const auto& style = node->getComputedStyle();
    if (style.background_color == "transparent") return;

    float x = layout_node->layout.position[0];
    float y = layout_node->layout.position[1];
    float width = layout_node->layout.dimensions[0];
    float height = layout_node->layout.dimensions[1];

    // Special-case root/html/body to fill the viewport horizontally
    const std::string& tag = node->getTagName();
    if ((tag.empty() || tag == "html" || tag == "body") && surface_) {
        x = 0.0f;
        y = 0.0f;
        width = static_cast<float>(surface_->getWidth());
        height = static_cast<float>(surface_->getHeight());
    }

    if (width <= 0.0f || height <= 0.0f) return;

    uint8_t r = 240, g = 240, b = 240, a = 255;
    parseCssColor(style.background_color, r, g, b, a);

    std::fprintf(stderr,
        "[Painter] background tag=%s color=%s rgba=(%u,%u,%u,%u) rect=(%.1f,%.1f,%.1f,%.1f)\n",
        node->getTagName().c_str(), style.background_color.c_str(), r, g, b, a,
        x, y, width, height);

    if (style.border_radius > 0.0f) {
        skia_backend_->drawRoundRect(x, y, width, height, style.border_radius,
                                     r, g, b, a, 0.0f);
    } else {
        skia_backend_->drawRect(x, y, width, height, r, g, b, a);
    }
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
    if (!node || !layout_node || !skia_backend_) return;

    std::string tag = node->getTagName();

    // For now, render text content for block elements like h1 / p using their own box
    if (node->getType() == dom::DOMNode::NodeType::ELEMENT && (tag == "h1" || tag == "p")) {
        // Collect direct text children
        std::string text;
        for (const auto& child : node->getChildren()) {
            if (child && child->getType() == dom::DOMNode::NodeType::TEXT) {
                text += child->getTextContent();
            }
        }

        if (!text.empty()) {
            const auto& style = node->getComputedStyle();
            float x = layout_node->layout.position[0];
            float y = layout_node->layout.position[1];
            float width = layout_node->layout.dimensions[0];
            float height = layout_node->layout.dimensions[1];

            // Apply simple padding based on computed style
            float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
            float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;

            float font_size = style.font_size;
            if (font_size <= 0.0f) {
                font_size = 16.0f; // fallback font size
            }

            // Text color from style.color
            uint8_t r = 0, g = 0, b = 0, a = 255;
            parseCssColor(style.color, r, g, b, a);

            float text_x = x + pad_left;
            float text_y;
            // Try to keep baseline inside the element box: if the box太矮，就大致垂直居中
            if (height > font_size + pad_top) {
                text_y = y + pad_top + font_size;
            } else {
                text_y = y + height * 0.5f + font_size * 0.35f;
            }

            std::fprintf(stderr,
                "[Painter] draw text tag=%s text='%s' box=(%.1f,%.1f,%.1f,%.1f) pad=(%.1f,%.1f) font=%.1f rgba=(%u,%u,%u,%u)\n",
                node->getTagName().c_str(), text.c_str(), x, y, width, height,
                pad_left, pad_top, font_size, r, g, b, a);

            // Debug helper: small red dot at the baseline start so we can see位置
            skia_backend_->drawRect(text_x, text_y, 2.0f, 2.0f, 255, 0, 0, 255);

            // 使用 CSS 的 font-family / font-weight 信息进行美化渲染
            const std::string& font_family = style.font_family;
            const std::string& font_weight = style.font_weight;

            // 简单的文字阴影，让对比度更好
            uint8_t shadow_a = static_cast<uint8_t>(a * 0.6f);
            skia_backend_->drawTextStyled(text, text_x + 1.5f, text_y + 1.5f,
                                          font_size, font_family, font_weight,
                                          0, 0, 0, shadow_a);

            // 主文字
            skia_backend_->drawTextStyled(text, text_x, text_y,
                                          font_size, font_family, font_weight,
                                          r, g, b, a);
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
