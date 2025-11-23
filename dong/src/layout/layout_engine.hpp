#pragma once

#include "../dom/dom_node.hpp"
#include <memory>
#include <unordered_map>

// Forward declare Yoga types
extern "C" {
    typedef struct YGNode YGNode;
    typedef struct YGConfig YGConfig;
}

namespace dong::layout {

// Dirty rectangle for incremental rendering
struct DirtyRect {
    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;
    bool is_empty = true;

    bool isEmpty() const { return is_empty; }
    void expand(float px, float py, float pw, float ph);
    bool intersects(float px, float py, float pw, float ph) const;
};

// Layout node properties
struct LayoutNode {
    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;
    YGNode* yoga_node = nullptr;
    LayoutNode* parent = nullptr;
    LayoutNode* first_child = nullptr;
    LayoutNode* next_sibling = nullptr;
    bool layout_recalculated = false;  // Whether this node's layout was recalculated in current pass

    // Layout result structure
    struct Layout {
        float position[2] = {0, 0};  // x, y
        float dimensions[2] = {0, 0}; // width, height
    } layout;

    LayoutNode() = default;
    ~LayoutNode();
};

// Yoga-based layout engine
class Engine {
public:
    Engine();
    ~Engine();

    // Calculate layout for DOM tree
    void calculateLayout(dom::DOMNodePtr root, float width, float height);

    // Get layout info for a node
    const LayoutNode* getLayout(dom::DOMNodePtr node) const;

    // Update layout when DOM changes
    void markDirty(dom::DOMNodePtr node);

    // Get the dirty region (union of all changed areas)
    const DirtyRect& getDirtyRect() const { return dirty_rect_; }
    bool hasDirtyRegion() const { return !dirty_rect_.isEmpty(); }

private:
    YGConfig* yoga_config = nullptr;
    std::unordered_map<void*, std::unique_ptr<LayoutNode>> layout_cache;
    DirtyRect dirty_rect_;  // Accumulates all regions that changed this frame

    // Yoga node creation and style mapping
    YGNode* createYogaNode(dom::DOMNodePtr dom_node);
    void applyDOMStylesToYoga(dom::DOMNodePtr dom_node, YGNode* yoga_node);
    void mapComputedStylesToYoga(const dom::ComputedStyle& style, YGNode* yoga_node);

    // CSS unit conversion
    float parsePixelValue(const dom::CSSValue& value, float parent_size = 0);
    float parsePercentValue(const dom::CSSValue& value, float parent_size);

    // Cleanup
    void destroyYogaNode(YGNode* node);

    // Incremental layout helpers
    bool calculateLayoutIncremental(dom::DOMNodePtr dom_node, YGNode* yoga_node,
                                    float parent_x, float parent_y);
};

} // namespace dong::layout
