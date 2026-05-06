#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include "../dom/dom/dom_node.hpp"
#include "../render/display_list.hpp"
#include "../render/text_shaper.hpp"

namespace dong::devtools {

// Minimal DevTools overlay state.
// Renders a DOM tree inspector as display list items overlaid on the main content.
class DevToolsOverlay {
public:
    DevToolsOverlay() = default;

    void toggle() { visible_ = !visible_; }
    bool isVisible() const { return visible_; }
    void show() { visible_ = true; }
    void hide() { visible_ = false; }

    // Set the DOM root to inspect
    void setRoot(const dom::DOMNodePtr& root) { root_ = root; }

    // Set viewport size (for panel positioning)
    void setViewport(float w, float h) { viewport_w_ = w; viewport_h_ = h; }

    // Set frame metrics
    void setMetrics(uint32_t draw_calls, uint32_t display_items, float frame_ms) {
        draw_calls_ = draw_calls;
        display_items_ = display_items;
        frame_ms_ = frame_ms;
    }

    // Generate overlay display items (call each frame when visible)
    void buildOverlay(render::DisplayListBuilder& builder);

    // Handle mouse interaction with the overlay
    // Returns true if the event was consumed by the overlay
    bool handleMouseMove(float x, float y);
    bool handleMouseButton(float x, float y, bool pressed, int32_t button);
    bool handleMouseWheel(float x, float y, float delta_y);

    // Get the currently selected/hovered node for highlight
    const dom::DOMNodePtr& getSelectedNode() const { return selected_node_; }

private:
    bool visible_ = false;
    dom::DOMNodePtr root_;
    dom::DOMNodePtr selected_node_;
    dom::DOMNodePtr hovered_node_;
    float viewport_w_ = 800.0f;
    float viewport_h_ = 600.0f;

    // Metrics
    uint32_t draw_calls_ = 0;
    uint32_t display_items_ = 0;
    float frame_ms_ = 0.0f;

    // Panel layout
    float panel_width_ = 300.0f;  // right-side panel width
    float scroll_y_ = 0.0f;
    render::TextShaper text_shaper_;

    // Tree rendering state
    struct TreeLine {
        std::string text;
        int depth = 0;
        dom::DOMNodePtr node;
        float y_pos = 0.0f;
        float arrow_x = 0.0f;
        bool has_children = false;
        bool expanded = true;
    };
    std::vector<TreeLine> tree_lines_;
    std::unordered_map<const dom::DOMNode*, bool> expanded_state_;

    void buildTreeLines();
    void collectTreeLines(const dom::DOMNodePtr& node, int depth);
    bool isWhitespaceOnly(const std::string& s) const;
    bool isPointInPanel(float x, float y) const;
    bool findLineAtY(float y, TreeLine& out_line) const;
};

} // namespace dong::devtools
