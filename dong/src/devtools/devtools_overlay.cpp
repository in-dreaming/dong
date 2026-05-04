#include "devtools_overlay.hpp"
#include <cstdio>
#include <algorithm>

namespace dong::devtools {

void DevToolsOverlay::buildTreeLines() {
    tree_lines_.clear();
    if (root_) {
        collectTreeLines(root_, 0);
    }
}

void DevToolsOverlay::collectTreeLines(const dom::DOMNodePtr& node, int depth) {
    if (!node) return;
    if (depth > 20) return;  // prevent infinite recursion

    const auto type = node->getType();
    if (type == dom::DOMNode::NodeType::TEXT) {
        std::string text = node->getRawTextContent();
        if (text.size() > 30) text = text.substr(0, 30) + "...";
        // Collapse whitespace-only text nodes
        bool all_ws = true;
        for (char c : text) { if (c != ' ' && c != '\n' && c != '\t' && c != '\r') { all_ws = false; break; } }
        if (all_ws) return;
        tree_lines_.push_back({"\"" + text + "\"", depth, node, 0.0f});
        return;
    }

    if (type != dom::DOMNode::NodeType::ELEMENT) return;

    std::string tag = node->getTagName();
    std::string id = node->getAttribute("id");
    std::string cls = node->getAttribute("class");

    std::string label = "<" + tag;
    if (!id.empty()) label += " #" + id;
    if (!cls.empty()) {
        // Show first class only
        auto sp = cls.find(' ');
        label += " ." + (sp != std::string::npos ? cls.substr(0, sp) : cls);
    }
    label += ">";

    tree_lines_.push_back({label, depth, node, 0.0f});

    for (const auto& child : node->getChildren()) {
        collectTreeLines(child, depth + 1);
    }
}

void DevToolsOverlay::buildOverlay(render::DisplayListBuilder& builder) {
    if (!visible_) return;

    // Rebuild tree lines from current DOM
    buildTreeLines();

    // Panel background (right side)
    float panel_x = viewport_w_ - panel_width_;
    float panel_h = viewport_h_;

    // Semi-transparent dark background
    render::Rect bg_rect{panel_x, 0.0f, panel_width_, panel_h};
    render::Color bg_color{0.05f, 0.05f, 0.12f, 0.92f};
    builder.addRect(bg_rect, bg_color);

    // Title bar
    render::Rect title_rect{panel_x, 0.0f, panel_width_, 28.0f};
    render::Color title_bg{0.1f, 0.1f, 0.2f, 1.0f};
    builder.addRect(title_rect, title_bg);

    // Metrics bar at bottom
    float metrics_h = 24.0f;
    render::Rect metrics_rect{panel_x, panel_h - metrics_h, panel_width_, metrics_h};
    render::Color metrics_bg{0.08f, 0.08f, 0.15f, 1.0f};
    builder.addRect(metrics_rect, metrics_bg);

    // Title text: "DevTools"
    render::DrawGlyphRunData title_glyph{};
    title_glyph.rect = {panel_x + 8.0f, 4.0f, 100.0f, 20.0f};
    title_glyph.color = {1.0f, 0.84f, 0.0f, 1.0f};  // gold
    title_glyph.font_size = 12.0f;
    title_glyph.font_family = "sans-serif";
    title_glyph.baseline_x = panel_x + 8.0f;
    title_glyph.baseline_y = 18.0f;
    // We can't easily emit shaped glyphs here without access to TextShaper.
    // For the minimal v1, we'll render placeholder rects for DOM tree nodes
    // and use the text shaper integration later.

    // DOM tree entries (simplified: colored rects per node for now)
    float y = 32.0f;
    float line_h = 16.0f;
    float indent_px = 12.0f;

    for (size_t i = 0; i < tree_lines_.size() && y < panel_h - metrics_h - 4.0f; ++i) {
        auto& line = tree_lines_[i];
        line.y_pos = y;

        float x = panel_x + 8.0f + line.depth * indent_px;

        // Highlight selected node
        bool is_selected = (selected_node_ && line.node && line.node.get() == selected_node_.get());
        if (is_selected) {
            render::Rect hl_rect{panel_x, y, panel_width_, line_h};
            render::Color hl_color{0.2f, 0.3f, 0.5f, 0.6f};
            builder.addRect(hl_rect, hl_color);
        }

        // Tag indicator dot
        render::Color dot_color;
        if (line.text.front() == '<') {
            dot_color = {0.4f, 0.7f, 1.0f, 1.0f};  // blue for elements
        } else {
            dot_color = {0.6f, 0.6f, 0.6f, 0.8f};  // grey for text
        }
        render::Rect dot_rect{x, y + 5.0f, 6.0f, 6.0f};
        builder.addRect(dot_rect, dot_color);

        // Simple text representation: draw a small rect as text placeholder
        // (full text rendering requires TextShaper access — will be added in S2)
        float text_w = std::min(static_cast<float>(line.text.size()) * 6.0f, panel_width_ - 40.0f);
        render::Rect text_rect{x + 10.0f, y + 4.0f, text_w, 8.0f};
        render::Color text_color{0.8f, 0.8f, 0.8f, 0.5f};
        builder.addRect(text_rect, text_color);

        y += line_h;
    }

    // Metrics text placeholder (draw calls / items)
    float mx = panel_x + 8.0f;
    float my = panel_h - metrics_h + 4.0f;

    // Draw call indicator (green dot = count)
    render::Rect dc_dot{mx, my + 5.0f, 8.0f, 8.0f};
    render::Color dc_color{0.2f, 0.8f, 0.2f, 1.0f};  // green
    builder.addRect(dc_dot, dc_color);

    // Items indicator (blue dot)
    render::Rect items_dot{mx + 60.0f, my + 5.0f, 8.0f, 8.0f};
    render::Color items_color{0.3f, 0.6f, 1.0f, 1.0f};  // blue
    builder.addRect(items_dot, items_color);
}

bool DevToolsOverlay::handleClick(float x, float y) {
    if (!visible_) return false;

    float panel_x = viewport_w_ - panel_width_;
    if (x < panel_x) return false;  // Click outside panel

    // Find which tree line was clicked
    float line_h = 16.0f;
    for (const auto& line : tree_lines_) {
        if (y >= line.y_pos && y < line.y_pos + line_h) {
            selected_node_ = line.node;
            return true;
        }
    }

    return true;  // Consume click within panel area
}

} // namespace dong::devtools
