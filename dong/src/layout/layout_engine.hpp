#pragma once

#include "../dom/dom/dom_node.hpp"


#include <memory>
#include <unordered_map>
#include <vector>

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

    // Anonymous block wrappers: maps an anonymous YGNode* to its inline-level DOM children.
    // Created when a block container has mixed block + inline-level children (CSS anonymous
    // block box generation). The anonymous wrapper uses row + wrap direction so that
    // consecutive inline-level elements are laid out horizontally.
    struct AnonBlockInfo {
        YGNode* yoga_node = nullptr;
        dom::DOMNodePtr parent;  // The DOM parent that owns this anonymous wrapper
        std::vector<dom::DOMNodePtr> children;
    };
    std::vector<AnonBlockInfo> anon_blocks_;

    // Viewport size for resolving vw/vh units during Yoga style mapping
    float viewport_width_ = 0.0f;
    float viewport_height_ = 0.0f;
    float root_font_size_ = 16.0f;

    // Yoga node creation and style mapping
    YGNode* createYogaNode(dom::DOMNodePtr dom_node);
    void buildChildYogaNodes(dom::DOMNodePtr dom_node, YGNode* yoga_node);
    void applyDOMStylesToYoga(dom::DOMNodePtr dom_node, YGNode* yoga_node);
    void mapComputedStylesToYoga(const dom::ComputedStyle& style, YGNode* yoga_node,
                                float parent_content_width_px, float parent_content_height_px);

    // applyDOMStylesToYoga helpers (refactored)
    void applyYogaInlineAlignmentFixes(dom::DOMNodePtr dom_node, YGNode* yoga_node,
                                       const dom::ComputedStyle& style);
    void applyYogaFlexBasisFromWidth(dom::DOMNodePtr dom_node, YGNode* yoga_node,
                                     const dom::ComputedStyle& style,
                                     bool width_converted_to_max, float converted_width_value);
    void applyYogaInputElementStyles(dom::DOMNodePtr dom_node, YGNode* yoga_node,
                                     const dom::ComputedStyle& style);
    void applyYogaInlineElementMinSizes(dom::DOMNodePtr dom_node, YGNode* yoga_node,
                                        const dom::ComputedStyle& style);


    // CSS unit conversion
    float parsePixelValue(const dom::CSSValue& value, float parent_size = 0);
    float parsePercentValue(const dom::CSSValue& value, float parent_size);

    // Cleanup
    void destroyYogaNode(YGNode* node);

    // Block formatting context layout (margin: auto centering, etc.)
    void layoutBlockFormattingContext(dom::DOMNodePtr root);

    // Inline formatting context layout
    void layoutInlineFormattingContexts(dom::DOMNodePtr root);

    // IFC helper structures and functions (refactored from layoutInlineFormattingContexts)
    struct InlineItem {
        dom::DOMNodePtr node;
        float margin_left = 0.0f;
        float margin_right = 0.0f;
        float preferred_width = 0.0f;
        float preferred_height = 0.0f;
        float baseline_from_border_top = 0.0f;
        float line_height_px = 0.0f;
        float offset_x_in_content = 0.0f;
        std::string vertical_align = "baseline";
        bool is_line_break = false;
    };

    struct LineInfo {
        std::vector<size_t> item_indices;
        float max_baseline_from_border_top = 0.0f;
        float max_line_height_px = 0.0f;
    };

    struct IFCContext {
        float container_x = 0.0f;
        float container_y = 0.0f;
        float content_x = 0.0f;
        float content_y = 0.0f;
        float content_w = 0.0f;
        float pad_top = 0.0f;
        float pad_bottom = 0.0f;
        float container_baseline_from_border_top = 0.0f;
        float container_line_height_px = 0.0f;
        bool has_container_text_metrics = false;
    };

    void collectInlineItems(const dom::DOMNodePtr& container,
                           const IFCContext& ctx,
                           std::vector<InlineItem>& items);
    void breakIntoLines(std::vector<InlineItem>& items,
                       const IFCContext& ctx,
                       std::vector<LineInfo>& lines);
    void layoutLineItems(std::vector<InlineItem>& items,
                        const std::vector<LineInfo>& lines,
                        const IFCContext& ctx,
                        const dom::DOMNodePtr& container);
    float propagateIFCHeights(const dom::DOMNodePtr& node);
    void adjustPositionsAfterIFC(const dom::DOMNodePtr& node, float parent_delta_y);

    // Positioned layout (position: absolute)
    void layoutPositionedElements(dom::DOMNodePtr root);

    // Third-pass sibling Y adjustment helpers
    void shiftSubtreeY(const dom::DOMNodePtr& n, float dy);
    void shiftAnonWrapperY(const AnonBlockInfo& ab, float dy);
    void adjustSiblingYPositions(const dom::DOMNodePtr& node,
                                 LayoutNode* layout,
                                 const dom::ComputedStyle& style);

    // Incremental layout helpers
    bool calculateLayoutIncremental(dom::DOMNodePtr dom_node, YGNode* yoga_node,
                                    float parent_x, float parent_y);
};

} // namespace dong::layout
