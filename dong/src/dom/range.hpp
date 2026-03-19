#pragma once

#include "dom/dom_node.hpp"
#include <string>
#include <cstdint>

namespace dong::dom {

// DOM Range: represents a contiguous portion of a document.
// Simplified implementation focused on text selection use cases.
class Range {
public:
    Range();
    ~Range() = default;

    // Boundary points
    DOMNodePtr getStartContainer() const { return start_container_; }
    uint32_t getStartOffset() const { return start_offset_; }
    DOMNodePtr getEndContainer() const { return end_container_; }
    uint32_t getEndOffset() const { return end_offset_; }

    // Collapsed: start == end
    bool isCollapsed() const;

    // Common ancestor container
    DOMNodePtr getCommonAncestorContainer() const;

    // Set boundary points
    void setStart(DOMNodePtr container, uint32_t offset);
    void setEnd(DOMNodePtr container, uint32_t offset);
    void setStartBefore(DOMNodePtr node);
    void setStartAfter(DOMNodePtr node);
    void setEndBefore(DOMNodePtr node);
    void setEndAfter(DOMNodePtr node);

    // Collapse to start or end
    void collapse(bool to_start = true);

    // Select entire node contents
    void selectNodeContents(DOMNodePtr node);

    // Select entire node (including the node itself)
    void selectNode(DOMNodePtr node);

    // Extract text content of the range
    std::string toString() const;

    // Clone this range
    Range cloneRange() const;

    // Delete contents (for editing operations)
    void deleteContents();

    // Compare boundary points (returns -1, 0, or 1)
    enum HowCompare {
        START_TO_START = 0,
        START_TO_END = 1,
        END_TO_END = 2,
        END_TO_START = 3,
    };
    int compareBoundaryPoints(HowCompare how, const Range& other) const;

private:
    DOMNodePtr start_container_;
    uint32_t start_offset_ = 0;
    DOMNodePtr end_container_;
    uint32_t end_offset_ = 0;

    // Helper: find parent index of a child in its parent
    static uint32_t findChildIndex(DOMNodePtr parent, DOMNodePtr child);
};

} // namespace dong::dom
