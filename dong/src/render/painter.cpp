#include "painter.hpp"
#include "skia_backend.hpp"
#include <cstring>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <sstream>
#include <cctype>

namespace dong::render {

namespace {

// 简单 CSS 颜色解析器，支持 #rgb/#rrggbb 和少量命名色
static void parseCssColor(const std::string& css, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);
static std::string collapseWhitespace(const std::string& input);

// 将 CSS 颜色转换为 DisplayList::Color
static Color makeColorFromCss(const std::string& css, float opacity_multiplier = 1.0f) {
    uint8_t r8 = 255, g8 = 255, b8 = 255, a8 = 255;
    parseCssColor(css, r8, g8, b8, a8);
    float a = (a8 / 255.0f) * opacity_multiplier;
    Color c;
    c.r = r8 / 255.0f;
    c.g = g8 / 255.0f;
    c.b = b8 / 255.0f;
    c.a = a;
    return c;
}

} // anonymous namespace

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

ResourceManager* Painter::getResourceManager() const {
    if (skia_backend_) {
        return skia_backend_->getResourceManager();
    }
    return nullptr;
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

    beginFrame();

    if (layout_engine_) {
        current_dirty_rect_ = layout_engine_->getDirtyRect();
        const auto* layout_root = layout_engine_->getLayout(root);
        if (layout_root) {
            // 1) 先从 DOM/Layout 构建 DisplayList（供 GPU 管线使用）
            display_list_builder_.clear();
            buildDisplayListNode(root, layout_root, display_list_builder_);

            // 2) 仍然走原有 Skia renderNode 路径，确保现有行为不变
            renderNode(root, layout_root);
        }
    } else {
        current_dirty_rect_ = layout::DirtyRect();
        renderNode(root, nullptr);
    }

    endFrame();
    layout_engine_ = nullptr;
}

void Painter::renderDisplayList(const DisplayList& list) {
    if (!skia_backend_ || !surface_) return;

    beginFrame();

    for (const auto& item : list.items) {
        switch (item.type) {
        case DisplayItemType::DrawRect: {
            const auto& d = item.rect;
            uint8_t r = static_cast<uint8_t>(std::clamp(d.color.r * 255.0f, 0.0f, 255.0f));
            uint8_t g = static_cast<uint8_t>(std::clamp(d.color.g * 255.0f, 0.0f, 255.0f));
            uint8_t b = static_cast<uint8_t>(std::clamp(d.color.b * 255.0f, 0.0f, 255.0f));
            uint8_t a = static_cast<uint8_t>(std::clamp(d.color.a * 255.0f, 0.0f, 255.0f));
            skia_backend_->drawRect(d.rect.x, d.rect.y, d.rect.width, d.rect.height, r, g, b, a);
            break;
        }
        case DisplayItemType::DrawRoundedRect: {
            const auto& d = item.rounded_rect;
            uint8_t r = static_cast<uint8_t>(std::clamp(d.color.r * 255.0f, 0.0f, 255.0f));
            uint8_t g = static_cast<uint8_t>(std::clamp(d.color.g * 255.0f, 0.0f, 255.0f));
            uint8_t b = static_cast<uint8_t>(std::clamp(d.color.b * 255.0f, 0.0f, 255.0f));
            uint8_t a = static_cast<uint8_t>(std::clamp(d.color.a * 255.0f, 0.0f, 255.0f));
            skia_backend_->drawRoundRect(d.rect.x, d.rect.y, d.rect.width, d.rect.height,
                                         d.radius, r, g, b, a, 0.0f);
            break;
        }
        case DisplayItemType::DrawImage: {
            const auto& d = item.image;
            uint8_t a = static_cast<uint8_t>(std::clamp(d.opacity * 255.0f, 0.0f, 255.0f));
            skia_backend_->drawImage(d.src, d.rect.x, d.rect.y,
                                     d.rect.width, d.rect.height, a);
            break;
        }
        case DisplayItemType::DrawGlyphRun: {
            const auto& d = item.glyph_run;
            uint8_t r = static_cast<uint8_t>(std::clamp(d.color.r * 255.0f, 0.0f, 255.0f));
            uint8_t g = static_cast<uint8_t>(std::clamp(d.color.g * 255.0f, 0.0f, 255.0f));
            uint8_t b = static_cast<uint8_t>(std::clamp(d.color.b * 255.0f, 0.0f, 255.0f));
            uint8_t a = static_cast<uint8_t>(std::clamp(d.color.a * 255.0f, 0.0f, 255.0f));
            skia_backend_->drawTextStyled(d.text,
                                          d.rect.x,
                                          d.rect.y,
                                          d.font_size,
                                          d.font_family,
                                          d.font_weight,
                                          r, g, b, a);
            break;
        }
        }
    }

    endFrame();
}

void Painter::renderNode(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node) return;

    const auto& style = node->getComputedStyle();
    if (style.display == "none") {
        return;
    }

    // Skip text nodes (rendered as part of parent element content)
    if (node->getType() == dom::DOMNode::NodeType::TEXT) {
        return;
    }
    


    // Dirty rect optimization: skip nodes outside dirty region (but skip for img tags)
    if (use_dirty_rect_ && !current_dirty_rect_.isEmpty() && layout_node && node->getTagName() != "img") {
        if (!isNodeInDirtyRect(layout_node)) {
            return;
        }
    }

    // Opacity stack: 累乘当前节点的 opacity
    float prev_opacity = current_opacity_;
    current_opacity_ *= style.opacity;

    bool pushed_clip = false;
    
    // Draw background and borders if layout provided
    if (layout_node) {
        // Apply overflow clipping for this node if requested
        if (style.overflow == "hidden") {
            float x = layout_node->layout.position[0];
            float y = layout_node->layout.position[1];
            float w = layout_node->layout.dimensions[0];
            float h = layout_node->layout.dimensions[1];
            if (w > 0.0f && h > 0.0f) {
                setClipRect(x, y, w, h);
                pushed_clip = true;
            }
        }

        drawNodeBackground(node, layout_node);
        drawNodeBorder(node, layout_node);
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

    if (pushed_clip) {
        clearClipRect();
    }

    current_opacity_ = prev_opacity;
}

// 下面这两个工具函数在前面声明过，这里给出实现
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

static std::string collapseWhitespace(const std::string& input) {
    if (input.empty()) {
        return "";
    }

    std::string output;
    output.reserve(input.size());
    bool in_space = false;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!in_space) {
                output.push_back(' ');
                in_space = true;
            }
        } else {
            output.push_back(c);
            in_space = false;
        }
    }

    size_t first = output.find_first_not_of(' ');
    if (first == std::string::npos) {
        return "";
    }
    size_t last = output.find_last_not_of(' ');
    return output.substr(first, last - first + 1);
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
    a = static_cast<uint8_t>(a * current_opacity_);

    if (style.border_radius > 0.0f) {
        skia_backend_->drawRoundRect(x, y, width, height, style.border_radius,
                                     r, g, b, a, 0.0f);
    } else {
        skia_backend_->drawRect(x, y, width, height, r, g, b, a);
    }
}

void Painter::buildDisplayListNode(const dom::DOMNodePtr& node,
                                   const layout::LayoutNode* layout_node,
                                   DisplayListBuilder& builder) {
    if (!node) return;

    const auto& style = node->getComputedStyle();
    if (style.display == "none") {
        return;
    }

    // 暂时与 Skia 路径共享同样的 text/img 过滤逻辑
    const std::string tag = node->getTagName();

    // 背景
    if (layout_node && style.background_color != "transparent") {
        Rect rect{};
        rect.x = layout_node->layout.position[0];
        rect.y = layout_node->layout.position[1];
        rect.width = layout_node->layout.dimensions[0];
        rect.height = layout_node->layout.dimensions[1];

        // root/html/body 填满 viewport
        if ((tag.empty() || tag == "html" || tag == "body") && surface_) {
            rect.x = 0.0f;
            rect.y = 0.0f;
            rect.width = static_cast<float>(surface_->getWidth());
            rect.height = static_cast<float>(surface_->getHeight());
        }

        if (rect.width > 0.0f && rect.height > 0.0f) {
            Color c = makeColorFromCss(style.background_color, current_opacity_);
            if (style.border_radius > 0.0f) {
                builder.addRoundedRect(rect, c, style.border_radius);
            } else {
                builder.addRect(rect, c);
            }
        }
    }

    // img 节点
    if (layout_node && tag == "img") {
        std::string src = node->getAttribute("src");
        if (!src.empty()) {
            Rect rect{};
            rect.x = layout_node->layout.position[0];
            rect.y = layout_node->layout.position[1];
            rect.width = layout_node->layout.dimensions[0];
            rect.height = layout_node->layout.dimensions[1];

            if (rect.width > 0.0f && rect.height > 0.0f) {
                builder.addImage(rect, src, current_opacity_);
            }
        }
    }

    // 文本内容（复用 Skia 路径中的布局逻辑，转为 GlyphRun）
    if (layout_node && tag != "script" && tag != "style" && tag != "head" && tag != "img") {
        bool has_text_child = false;
        bool has_element_child = false;
        std::string raw_text;
        for (const auto& child : node->getChildren()) {
            if (!child) continue;
            if (child->getType() == dom::DOMNode::NodeType::TEXT) {
                raw_text += child->getTextContent();
                has_text_child = true;
            } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                has_element_child = true;
            }
        }

        const bool tag_prefers_text =
            tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
            tag == "h5" || tag == "h6" || tag == "p" || tag == "span" ||
            tag == "button" || tag == "code" || tag == "div" || tag == "footer";

        if (has_text_child && (tag_prefers_text || !has_element_child)) {
            std::string text = collapseWhitespace(raw_text);
            if (!text.empty()) {
                const auto& style2 = node->getComputedStyle();
                float x = layout_node->layout.position[0];
                float y = layout_node->layout.position[1];
                float width = layout_node->layout.dimensions[0];
                float height = layout_node->layout.dimensions[1];

                float pad_left = style2.padding_left.isPixel() ? style2.padding_left.value : 0.0f;
                float pad_right = style2.padding_right.isPixel() ? style2.padding_right.value : 0.0f;
                float pad_top = style2.padding_top.isPixel() ? style2.padding_top.value : 0.0f;
                float pad_bottom = style2.padding_bottom.isPixel() ? style2.padding_bottom.value : 0.0f;

                float font_size = style2.font_size > 0.0f ? style2.font_size : 16.0f;
                float line_height = font_size * (tag == "h1" ? 1.25f : (tag == "h2" ? 1.2f : 1.35f));
                if (tag == "button" || tag == "span") {
                    line_height = font_size * 1.2f;
                }

                Color text_color = makeColorFromCss(style2.color, current_opacity_);

                auto measure = [&](const std::string& line) -> float {
                    if (!skia_backend_) {
                        return static_cast<float>(line.size()) * font_size * 0.5f;
                    }
                    return skia_backend_->measureTextWidth(line, font_size, style2.font_family, style2.font_weight);
                };

                float inner_width = width - pad_left - pad_right;
                if (inner_width <= 0.0f) {
                    inner_width = width > 0.0f ? width : 0.0f;
                }

                std::vector<std::string> lines;
                if (inner_width <= 0.0f) {
                    lines.push_back(text);
                } else {
                    std::istringstream stream(text);
                    std::string word;
                    std::string current_line;
                    while (stream >> word) {
                        std::string candidate = current_line.empty() ? word : current_line + " " + word;
                        float candidate_width = measure(candidate);
                        if (candidate_width <= inner_width || current_line.empty()) {
                            current_line = candidate;
                        } else {
                            lines.push_back(current_line);
                            current_line = word;
                        }
                    }
                    if (!current_line.empty()) {
                        lines.push_back(current_line);
                    }
                }

                if (lines.empty()) {
                    lines.push_back(text);
                }

                float first_baseline;
                if (height > font_size + pad_top + pad_bottom) {
                    first_baseline = y + pad_top + font_size;
                } else {
                    first_baseline = y + height * 0.5f + font_size * 0.35f;
                }

                for (size_t i = 0; i < lines.size(); ++i) {
                    const std::string& line = lines[i];
                    float line_width = measure(line);
                    float text_x = x + pad_left;
                    if (style2.text_align == "center") {
                        text_x = x + pad_left + std::max(0.0f, (inner_width - line_width) * 0.5f);
                    } else if (style2.text_align == "right") {
                        text_x = x + pad_left + std::max(0.0f, inner_width - line_width);
                    }

                    float text_y = first_baseline + static_cast<float>(i) * line_height;

                    DrawGlyphRunData glyph{};
                    glyph.text = line;
                    glyph.rect.x = text_x;
                    glyph.rect.y = text_y;
                    glyph.rect.width = line_width;
                    glyph.rect.height = line_height;
                    glyph.color = text_color;
                    glyph.font_size = font_size;
                    glyph.font_family = style2.font_family;
                    glyph.font_weight = style2.font_weight;
                    builder.addGlyphRun(glyph);
                }
            }
        }
    }

    // 递归子节点，沿用当前 opacity（与 renderNode 行为匹配）
    const auto& children = node->getChildren();
    for (const auto& child : children) {
        const layout::LayoutNode* child_layout = nullptr;
        if (layout_engine_ && child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            child_layout = layout_engine_->getLayout(child);
        }
        buildDisplayListNode(child, child_layout, builder);
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
        uint8_t r = 0, g = 0, b = 0, a = 255;
        if (style.border_color == "red") { r = 255; }
        else if (style.border_color == "blue") { b = 255; }
        else if (style.border_color == "green") { g = 255; }
        a = static_cast<uint8_t>(a * current_opacity_);

        skia_backend_->drawStroke(x, y, width, height, style.border_width, r, g, b, a);
    }

    // Draw rounded corners
    if (style.border_radius > 0) {
        skia_backend_->drawRoundRect(x, y, width, height, style.border_radius,
                                      50, 50, 50, 0, 0.5f);
    }
}

void Painter::drawNodeContent(const dom::DOMNodePtr& node, const layout::LayoutNode* layout_node) {
    if (!node || !layout_node || !skia_backend_) return;

    const std::string tag = node->getTagName();
    if (tag == "script" || tag == "style" || tag == "head") {
        return;
    }

    // Handle img tag specially - draw the image
    if (tag == "img") {
        std::string src = node->getAttribute("src");
        if (!src.empty()) {
            const auto& style = node->getComputedStyle();
            float x = layout_node->layout.position[0];
            float y = layout_node->layout.position[1];
            float width = layout_node->layout.dimensions[0];
            float height = layout_node->layout.dimensions[1];
            
            uint8_t alpha = static_cast<uint8_t>(255 * current_opacity_);
            skia_backend_->drawImage(src, x, y, width, height, alpha);
        }
        return;
    }

    bool has_text_child = false;
    bool has_element_child = false;
    std::string raw_text;
    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::TEXT) {
            raw_text += child->getTextContent();
            has_text_child = true;
        } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            has_element_child = true;
        }
    }

    const bool tag_prefers_text =
        tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
        tag == "h5" || tag == "h6" || tag == "p" || tag == "span" ||
        tag == "button" || tag == "code" || tag == "div" || tag == "footer";

    if (!has_text_child) {
        return;
    }
    if (!tag_prefers_text && has_element_child) {
        return;
    }

    std::string text = collapseWhitespace(raw_text);
    if (text.empty()) {
        return;
    }

    const auto& style = node->getComputedStyle();
    float x = layout_node->layout.position[0];
    float y = layout_node->layout.position[1];
    float width = layout_node->layout.dimensions[0];
    float height = layout_node->layout.dimensions[1];

    float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
    float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
    float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
    float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;

    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
    float line_height = font_size * (tag == "h1" ? 1.25f : (tag == "h2" ? 1.2f : 1.35f));
    if (tag == "button" || tag == "span") {
        line_height = font_size * 1.2f;
    }

    uint8_t r = 0, g = 0, b = 0, a = 255;
    parseCssColor(style.color, r, g, b, a);
    a = static_cast<uint8_t>(a * current_opacity_);
    uint8_t shadow_a = static_cast<uint8_t>(a * 0.6f);

    auto measure = [&](const std::string& line) -> float {
        if (!skia_backend_) {
            return static_cast<float>(line.size()) * font_size * 0.5f;
        }
        return skia_backend_->measureTextWidth(line, font_size, style.font_family, style.font_weight);
    };

    float inner_width = width - pad_left - pad_right;
    if (inner_width <= 0.0f) {
        inner_width = width > 0.0f ? width : 0.0f;
    }

    std::vector<std::string> lines;
    if (inner_width <= 0.0f) {
        lines.push_back(text);
    } else {
        std::istringstream stream(text);
        std::string word;
        std::string current_line;
        while (stream >> word) {
            std::string candidate = current_line.empty() ? word : current_line + " " + word;
            float candidate_width = measure(candidate);
            if (candidate_width <= inner_width || current_line.empty()) {
                current_line = candidate;
            } else {
                lines.push_back(current_line);
                current_line = word;
            }
        }
        if (!current_line.empty()) {
            lines.push_back(current_line);
        }
    }

    if (lines.empty()) {
        lines.push_back(text);
    }

    float first_baseline;
    if (height > font_size + pad_top + pad_bottom) {
        first_baseline = y + pad_top + font_size;
    } else {
        first_baseline = y + height * 0.5f + font_size * 0.35f;
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        float line_width = measure(line);
        float text_x = x + pad_left;
        if (style.text_align == "center") {
            text_x = x + pad_left + std::max(0.0f, (inner_width - line_width) * 0.5f);
        } else if (style.text_align == "right") {
            text_x = x + pad_left + std::max(0.0f, inner_width - line_width);
        }

        float text_y = first_baseline + static_cast<float>(i) * line_height;

        if (shadow_a > 0) {
            skia_backend_->drawTextStyled(line, text_x + 1.0f, text_y + 1.0f,
                                          font_size, style.font_family, style.font_weight,
                                          0, 0, 0, shadow_a);
        }

        skia_backend_->drawTextStyled(line, text_x, text_y,
                                      font_size, style.font_family, style.font_weight,
                                      r, g, b, a);
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

bool Painter::isNodeInDirtyRect(const layout::LayoutNode* layout_node) const {
    if (!layout_node || current_dirty_rect_.isEmpty()) {
        return true;
    }
    
    float x = layout_node->layout.position[0];
    float y = layout_node->layout.position[1];
    float w = layout_node->layout.dimensions[0];
    float h = layout_node->layout.dimensions[1];
    
    return current_dirty_rect_.intersects(x, y, w, h);
}

} // namespace dong::render
