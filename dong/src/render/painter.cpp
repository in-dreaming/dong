#include "painter.hpp"
#include <SDL3/SDL_log.h>
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

static Color makeColorFromCss(const std::string& css) {
    uint8_t r8 = 255, g8 = 255, b8 = 255, a8 = 255;
    parseCssColor(css, r8, g8, b8, a8);
    Color c;
    c.r = r8 / 255.0f;
    c.g = g8 / 255.0f;
    c.b = b8 / 255.0f;
    c.a = a8 / 255.0f;
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

static bool shouldClipOverflow(const std::string& overflow_value) {
    std::string lowered = overflow_value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lowered == "hidden" || lowered == "scroll";
}

} // anonymous namespace

Painter::Painter(RenderSurface* surface)
    : surface_(surface), layout_engine_(nullptr) {
}

Painter::~Painter() {
}

const DisplayList& Painter::buildDisplayList(const dom::DOMNodePtr& root, layout::Engine* layout_engine) {
    layout_engine_ = layout_engine;

    display_list_builder_.clear();
    layer_tree_.clear();
    layer_stack_.clear();

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


    const std::string tag = node->getTagName();

    Rect node_rect{};
    bool has_layout_rect = false;
    if (layout_node) {
        node_rect.x = layout_node->layout.position[0];
        node_rect.y = layout_node->layout.position[1];
        node_rect.width = layout_node->layout.dimensions[0];
        node_rect.height = layout_node->layout.dimensions[1];
        has_layout_rect = node_rect.width > 0.0f && node_rect.height > 0.0f;
    }

    const bool should_apply_clip = has_layout_rect && shouldClipOverflow(style.overflow);

    DisplayListBuilder::ScopedLayer opacity_scope;
    float clamped_opacity = std::clamp(style.opacity, 0.0f, 1.0f);

    Rect layer_bounds = node_rect;
    if (!has_layout_rect && surface_) {
        layer_bounds.x = 0.0f;
        layer_bounds.y = 0.0f;
        layer_bounds.width = static_cast<float>(surface_->getWidth());
        layer_bounds.height = static_cast<float>(surface_->getHeight());
    }

    const bool is_scroll_container =
        should_apply_clip &&
        (style.overflow == "scroll" || style.overflow == "auto");

    const bool has_transform =
        (style.transform_translate_x != 0.0f || style.transform_translate_y != 0.0f ||
         style.transform_scale_x != 1.0f || style.transform_scale_y != 1.0f);

    bool has_isolation = (style.isolation_isolate || is_scroll_container || has_transform) &&
                         (layer_bounds.width > 0.0f && layer_bounds.height > 0.0f);
    bool needs_layer = has_isolation || clamped_opacity < 0.999f;
    int parent_layer_index = layer_stack_.empty() ? -1 : layer_stack_.back();
    bool pushed_layer_node = false;
    if (needs_layer) {
        uint64_t layer_id = reinterpret_cast<uint64_t>(node.get());
        bool layer_dirty = node->isLayoutDirty();
        if (!layer_dirty && use_dirty_rect_ && !current_dirty_rect_.isEmpty()) {
            layer_dirty = isRectInDirtyRect(layer_bounds);
        }

        // 在 LayerTree 中记录这一层
        LayerNode layer_node;
        layer_node.id = layer_id;
        layer_node.type = has_isolation ? LayerType::Surface : LayerType::Opacity;
        layer_node.bounds = layer_bounds;
        layer_node.opacity = clamped_opacity;
        layer_node.transform = LayerTransform::identity();
        layer_node.transform.m[0] = style.transform_scale_x;
        layer_node.transform.m[4] = style.transform_scale_y;
        layer_node.transform.m[2] = style.transform_translate_x;
        layer_node.transform.m[5] = style.transform_translate_y;
        layer_node.scroll_x = 0.0f;
        layer_node.scroll_y = 0.0f;
        layer_node.is_surface = has_isolation;
        layer_node.content_dirty = layer_dirty;
        layer_node.transform_dirty = false;
        layer_node.opacity_dirty = false;
        layer_node.scroll_dirty = false;

        const int this_index = layer_tree_.addNode(layer_node, parent_layer_index);
        layer_stack_.push_back(this_index);
        pushed_layer_node = true;

        opacity_scope = builder.pushLayer(clamped_opacity, has_isolation, layer_bounds, layer_id, layer_dirty);
    }

    // 1. 背景
    if (layout_node && style.background_color != "transparent") {
        Rect rect = node_rect;

        // root/html/body 填满 viewport
        if ((tag.empty() || tag == "html" || tag == "body") && surface_) {
            rect.x = 0.0f;
            rect.y = 0.0f;
            rect.width = static_cast<float>(surface_->getWidth());
            rect.height = static_cast<float>(surface_->getHeight());
        }

        if (rect.width > 0.0f && rect.height > 0.0f) {
            Color c = makeColorFromCss(style.background_color);
            if (style.border_radius > 0.0f) {
                builder.addRoundedRect(rect, c, style.border_radius);
            } else {
                builder.addRect(rect, c);
            }
        }
    }

    DisplayListBuilder::ScopedClip clip_scope;
    if (should_apply_clip) {
        if (style.border_radius > 0.0f) {
            clip_scope = builder.pushRoundedClip(node_rect, style.border_radius);
        } else {
            clip_scope = builder.pushClipRect(node_rect);
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
                builder.addImage(rect, src, 1.0f);
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

                // 调试：输出布局和字体大小
                static int text_debug_count = 0;
                if (text_debug_count < 5) {
                    SDL_Log("[Painter] Text #%d: layout=(%.1f,%.1f) size=%.1fx%.1f font_size=%.1f",
                           text_debug_count, x, y, width, height, font_size);
                    ++text_debug_count;
                }

                Color text_color = makeColorFromCss(style.color);

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

                for (size_t i = 0; i < lines.size(); ++i) {
                    const std::string& line = lines[i];

                    TextShapeRequest request{};
                    request.text = line;
                    request.font_family = style.font_family;
                    request.font_weight = style.font_weight;
                    request.font_size = font_size;

                    ShapedText shaped{};
                    if (!text_shaper_.shape(request, shaped) || shaped.glyphs.empty()) {
                        continue;
                    }

                    // 将 design units 转换为像素用于布局
                    const float scale = shaped.scale_to_pixels;
                    float line_width = shaped.width_units * scale;
                    if (line_width <= 0.0f) {
                        line_width = estimateTextWidth(line, font_size);
                    }

                    // 使用 TextShaper 提供的 line-height（design units）作为 CSS `line-height: normal` 的近似，
                    // 再结合字体 ascent/descender，将多余的 leading 在行盒顶部/底部平均分摊，
                    // 使得文本在灰色块内的垂直位置更接近浏览器行为。
                    float line_height_units = shaped.line_height_units;
                    float ascent_units = shaped.ascent_units;
                    float descent_units = shaped.descent_units;

                    if (line_height_units <= 0.0f) {
                        line_height_units = font_size / std::max(scale, 1e-3f);
                    }
                    if (ascent_units <= 0.0f) {
                        ascent_units = font_size / std::max(scale, 1e-3f);
                    }
                    // descent_units 一般为负值，如果缺失则按 0 处理
                    float descent_abs_units = descent_units < 0.0f ? -descent_units : 0.0f;

                    float metrics_height_units = ascent_units + descent_abs_units;
                    if (metrics_height_units <= 0.0f) {
                        metrics_height_units = line_height_units;
                    }

                    float extra_leading_units = line_height_units - metrics_height_units;
                    if (extra_leading_units < 0.0f) {
                        extra_leading_units = 0.0f;
                    }
                    float top_leading_units = extra_leading_units * 0.5f;

                    float effective_line_height = line_height_units * scale;
                    float ascent = ascent_units * scale;

                    float baseline_offset = (top_leading_units + ascent_units) * scale;

                    float text_x = x + pad_left;
                    if (style.text_align == "center") {
                        text_x = x + pad_left + std::max(0.0f, (inner_width - line_width) * 0.5f);
                    } else if (style.text_align == "right") {
                        text_x = x + pad_left + std::max(0.0f, inner_width - line_width);
                    }

                    float base_baseline = y + pad_top + baseline_offset;
                    float baseline_y = base_baseline + static_cast<float>(i) * effective_line_height;

                    DrawGlyphRunData glyph{};
                    glyph.rect.x = text_x;
                    glyph.rect.y = baseline_y - ascent;
                    glyph.rect.width = line_width;
                    glyph.rect.height = effective_line_height;
                    glyph.color = text_color;
                    glyph.font_size = font_size;
                    glyph.font_family = style.font_family;
                    glyph.font_weight = style.font_weight;
                    glyph.font_path = shaped.font_path;
                    glyph.baseline_x = text_x;
                    glyph.baseline_y = baseline_y;
                    
                    // 传递 design units 元数据
                    glyph.units_per_em = shaped.units_per_em;
                    glyph.scale_to_pixels = shaped.scale_to_pixels;

                    glyph.glyphs.reserve(shaped.glyphs.size());
                    for (const auto& shaped_glyph : shaped.glyphs) {
                        GlyphInstance instance{};
                        instance.glyph_id = shaped_glyph.glyph_id;
                        // 保持 design units，不在此处缩放
                        instance.pen_x_units = shaped_glyph.pen_x_units;
                        instance.pen_y_units = shaped_glyph.pen_y_units;
                        glyph.glyphs.push_back(instance);
                    }

                    builder.addGlyphRun(std::move(glyph));
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

    if (pushed_layer_node && !layer_stack_.empty()) {
        layer_stack_.pop_back();
    }

}

bool Painter::isNodeInDirtyRect(const layout::LayoutNode* layout_node) const {
    if (!layout_node || current_dirty_rect_.isEmpty()) {
        return true;
    }

    Rect rect{};
    rect.x = layout_node->layout.position[0];
    rect.y = layout_node->layout.position[1];
    rect.width = layout_node->layout.dimensions[0];
    rect.height = layout_node->layout.dimensions[1];
    return isRectInDirtyRect(rect);
}

bool Painter::isRectInDirtyRect(const Rect& rect) const {
    if (current_dirty_rect_.isEmpty()) {
        return true;
    }

    float node_x = rect.x;
    float node_y = rect.y;
    float node_w = rect.width;
    float node_h = rect.height;

    float dirty_x = current_dirty_rect_.x;
    float dirty_y = current_dirty_rect_.y;
    float dirty_w = current_dirty_rect_.width;
    float dirty_h = current_dirty_rect_.height;

    bool x_overlap = (node_x < dirty_x + dirty_w) && (node_x + node_w > dirty_x);
    bool y_overlap = (node_y < dirty_y + dirty_h) && (node_y + node_h > dirty_y);

    return x_overlap && y_overlap;
}

} // namespace dong::render
