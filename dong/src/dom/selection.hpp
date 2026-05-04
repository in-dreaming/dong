#pragma once

#include "dom/dom_node.hpp"
#include "range.hpp"
#include <string>
#include <vector>
#include <memory>

namespace dong::dom {

// DOM Selection: represents the range of text selected by the user or the
// current position of the caret.
class Selection {
public:
    Selection();
    ~Selection() = default;

    // Anchor: where the selection started
    DOMNodePtr getAnchorNode() const { return anchor_node_; }
    uint32_t getAnchorOffset() const { return anchor_offset_; }

    // Focus: where the selection ended (current position)
    DOMNodePtr getFocusNode() const { return focus_node_; }
    uint32_t getFocusOffset() const { return focus_offset_; }

    // Is collapsed (caret, no selection extent)
    bool isCollapsed() const;

    // Number of ranges (typically 0 or 1)
    uint32_t getRangeCount() const;

    // Get the range at index
    Range* getRangeAt(uint32_t index);

    // Add a range (replaces current selection)
    void addRange(const Range& range);

    // Remove all ranges
    void removeAllRanges();

    // Collapse to a position
    void collapse(DOMNodePtr node, uint32_t offset);

    // Collapse to start/end of current range
    void collapseToStart();
    void collapseToEnd();

    // Extend selection to a point
    void extend(DOMNodePtr node, uint32_t offset);

    // Select all children of a node
    void selectAllChildren(DOMNodePtr node);

    // Get string representation of the selection
    std::string toString() const;

    // Set selection from anchor/focus (used by mouse drag)
    void setBaseAndExtent(DOMNodePtr anchor_node, uint32_t anchor_offset,
                          DOMNodePtr focus_node, uint32_t focus_offset);

private:
    DOMNodePtr anchor_node_;
    uint32_t anchor_offset_ = 0;
    DOMNodePtr focus_node_;
    uint32_t focus_offset_ = 0;

    // Current range (we support a single range)
    std::unique_ptr<Range> range_;

    // Sync range from anchor/focus
    void syncRange();
};

// Selection render info for painter
struct SelectionRenderInfo {
    DOMNodePtr start_node;
    uint32_t start_offset = 0;
    DOMNodePtr end_node;
    uint32_t end_offset = 0;
    bool has_selection = false;
};

} // namespace dong::dom
