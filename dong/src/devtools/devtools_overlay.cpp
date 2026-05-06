#include "devtools_overlay.hpp"
#include <cstdio>
#include <algorithm>
#include <string>

namespace dong::devtools {

namespace {

void appendOverlayText(render::DisplayListBuilder& builder,
                       render::TextShaper& shaper,
                       const std::string& text,
                       float x,
                       float y,
                       float font_size,
                       const render::Color& color) {
    if (text.empty()) return;

    render::TextShapeRequest request;
    request.text = text;
    request.font_family = "sans-serif";
    request.font_weight = "400";
    request.font_style = "normal";
    request.font_size = font_size;
    request.origin_x = x;
    request.origin_y = y;

    render::ShapedText shaped;
    if (!shaper.shape(request, shaped) || shaped.glyphs.empty()) {
        return;
    }

    const float scale = shaped.scale_to_pixels;
    const float text_width_px = shaped.width_units * scale;
    const float text_height_px = shaped.line_height_units * scale;
    const float baseline_y = y + shaped.ascent_units * scale;

    render::DrawGlyphRunData glyph_run{};
    glyph_run.rect = {x, y, text_width_px, text_height_px};
    glyph_run.color = color;
    glyph_run.font_size = font_size;
    glyph_run.font_family = "sans-serif";
    glyph_run.font_weight = "400";
    glyph_run.font_style = "normal";
    glyph_run.font_paths = shaped.font_paths;
    glyph_run.font_path = shaped.font_path;
    glyph_run.baseline_x = x;
    glyph_run.baseline_y = baseline_y;
    glyph_run.units_per_em = shaped.units_per_em;
    glyph_run.scale_to_pixels = shaped.scale_to_pixels;

    for (const auto& sg : shaped.glyphs) {
        render::GlyphInstance inst{};
        inst.glyph_id = sg.glyph_id;
        inst.pen_x_units = sg.pen_x_units;
        inst.pen_y_units = sg.pen_y_units;
        inst.font_path_index = sg.font_path_index;
        inst.units_per_em = sg.units_per_em;
        glyph_run.glyphs.push_back(inst);
    }

    builder.addGlyphRun(std::move(glyph_run));
}

} // namespace

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

    if (type != dom::DOMNode::NodeType::ELEMENT) {
        // Root is often a DOCUMENT-like container node. It has no visual tag
        // line, but DevTools tree should still include its element/text children.
        for (const auto& child : node->getChildren()) {
            collectTreeLines(child, depth);
        }
        return;
    }

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

    appendOverlayText(
        builder, text_shaper_, "DevTools",
        panel_x + 8.0f, 6.0f, 12.0f,
        {1.0f, 0.84f, 0.0f, 1.0f});

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

        appendOverlayText(
            builder, text_shaper_, line.text,
            x + 10.0f, y + 2.0f, 11.0f,
            {0.85f, 0.85f, 0.9f, 0.95f});

        y += line_h;
    }

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

    appendOverlayText(
        builder, text_shaper_, "draw " + std::to_string(draw_calls_),
        mx + 12.0f, my + 1.0f, 10.0f,
        {0.75f, 0.9f, 0.75f, 0.95f});
    appendOverlayText(
        builder, text_shaper_, "items " + std::to_string(display_items_),
        mx + 72.0f, my + 1.0f, 10.0f,
        {0.75f, 0.82f, 1.0f, 0.95f});
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
