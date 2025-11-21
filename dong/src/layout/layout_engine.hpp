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

private:
    YGConfig* yoga_config = nullptr;
    std::unordered_map<void*, std::unique_ptr<LayoutNode>> layout_cache;

    // Yoga node creation and style mapping
    YGNode* createYogaNode(dom::DOMNodePtr dom_node);
    void applyDOMStylesToYoga(dom::DOMNodePtr dom_node, YGNode* yoga_node);
    void mapComputedStylesToYoga(const dom::ComputedStyle& style, YGNode* yoga_node);

    // CSS unit conversion
    float parsePixelValue(const dom::CSSValue& value, float parent_size = 0);
    float parsePercentValue(const dom::CSSValue& value, float parent_size);

    // Cleanup
    void destroyYogaNode(YGNode* node);
};

} // namespace dong::layout
