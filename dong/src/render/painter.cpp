#include "painter.hpp"
#include <cstring>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <sstream>
#include <cctype>

namespace dong::render {

namespace {

// CSS 颜色解析器
static void parseCssColor(const std::string& css, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    std::string s = css;
    a = 255;

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

    // 命名颜色
    if (s == "white")      { r = g = b = 255; return; }
    if (s == "black")      { r = g = b = 0;   return; }
    if (s == "red")        { r = 255; g = 0;   b = 0;   return; }
    if (s == "green")      { r = 0;   g = 128; b = 0;   return; }
    if (s == "blue")       { r = 0;   g = 0;   b = 255; return; }
    if (s == "gray" || s == "grey") { r = g = b = 128; return; }
    if (s == "lightgray" || s == "lightgrey") { r = g = b = 211; return; }

    r = g = b = 240;
}

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

static std::string collapseWhitespace(const std::string& input) {
    if (input.empty()) return "";

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
    if (first == std::string::npos) return "";
    size_t last = output.find_last_not_of(' ');
    return output.substr(first, last - first + 1);
}

// 简单的文本宽度估算（后续用 HarfBuzz 替换）
static float estimateTextWidth(const std::string& text, float font_size) {
    return static_cast<float>(text.size()) * font_size * 0.55f;
}

} // anonymous namespace

Painter::Painter(RenderSurface* surface)
    : surface_(surface), layout_engine_(nullptr), current_opacity_(1.0f) {
}

Painter::~Painter() {
}

const DisplayList& Painter::buildDisplayList(const dom::DOMNodePtr& root, layout::Engine* layout_engine) {
    layout_engine_ = layout_engine;
    current_opacity_ = 1.0f;

    display_list_builder_.clear();

    if (layout_engine_) {
        current_dirty_rect_ = layout_engine_->getDirtyRect();
        const auto* layout_root = layout_engine_->getLayout(root);
        if (layout_root) {
            buildDisplayListNode(root, layout_root, display_list_builder_);
        }
    }

    layout_engine_ = nullptr;
    return display_list_builder_.get();
}

void Painter::buildDisplayListNode(const dom::DOMNodePtr& node,
                                   const layout::LayoutNode* layout_node,
                                   DisplayListBuilder& builder) {
    if (!node) return;

    const auto& style = node->getComputedStyle();
    if (style.display == "none") return;

    if (node->getType() == dom::DOMNode::NodeType::TEXT) return;

    // Dirty rect 优化
    if (use_dirty_rect_ && !current_dirty_rect_.isEmpty() && layout_node) {
        if (!isNodeInDirtyRect(layout_node)) {
            return;
        }
    }

    // Opacity 累乘
    float prev_opacity = current_opacity_;
    current_opacity_ *= style.opacity;

    const std::string tag = node->getTagName();

    // 1. 背景
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

    // 2. 图片
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

    // 3. 文本内容
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
                float x = layout_node->layout.position[0];
                float y = layout_node->layout.position[1];
                float width = layout_node->layout.dimensions[0];
                float height = layout_node->layout.dimensions[1];

                float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
                float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
                float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;

                float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
                float line_height = font_size * (tag == "h1" ? 1.25f : (tag == "h2" ? 1.2f : 1.35f));

                Color text_color = makeColorFromCss(style.color, current_opacity_);

                float inner_width = width - pad_left - pad_right;
                if (inner_width <= 0.0f) inner_width = width > 0.0f ? width : 0.0f;

                // 简单的换行逻辑
                std::vector<std::string> lines;
                if (inner_width <= 0.0f) {
                    lines.push_back(text);
                } else {
                    std::istringstream stream(text);
                    std::string word;
                    std::string current_line;
                    while (stream >> word) {
                        std::string candidate = current_line.empty() ? word : current_line + " " + word;
                        float candidate_width = estimateTextWidth(candidate, font_size);
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

                float first_baseline = y + pad_top + font_size;

                for (size_t i = 0; i < lines.size(); ++i) {
                    const std::string& line = lines[i];
                    float line_width = estimateTextWidth(line, font_size);
                    float text_x = x + pad_left;
                    
                    if (style.text_align == "center") {
                        text_x = x + pad_left + std::max(0.0f, (inner_width - line_width) * 0.5f);
                    } else if (style.text_align == "right") {
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
                    glyph.font_family = style.font_family;
                    glyph.font_weight = style.font_weight;
                    builder.addGlyphRun(glyph);
                }
            }
        }
    }

    // 4. 递归子节点
    const auto& children = node->getChildren();
    for (const auto& child : children) {
        const layout::LayoutNode* child_layout = nullptr;
        if (layout_engine_ && child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            child_layout = layout_engine_->getLayout(child);
        }
        buildDisplayListNode(child, child_layout, builder);
    }

    current_opacity_ = prev_opacity;
}

bool Painter::isNodeInDirtyRect(const layout::LayoutNode* layout_node) const {
    if (!layout_node || current_dirty_rect_.isEmpty()) {
        return true;
    }

    float node_x = layout_node->layout.position[0];
    float node_y = layout_node->layout.position[1];
    float node_w = layout_node->layout.dimensions[0];
    float node_h = layout_node->layout.dimensions[1];

    float dirty_x = current_dirty_rect_.x;
    float dirty_y = current_dirty_rect_.y;
    float dirty_w = current_dirty_rect_.width;
    float dirty_h = current_dirty_rect_.height;

    bool x_overlap = (node_x < dirty_x + dirty_w) && (node_x + node_w > dirty_x);
    bool y_overlap = (node_y < dirty_y + dirty_h) && (node_y + node_h > dirty_y);

    return x_overlap && y_overlap;
}

} // namespace dong::render
