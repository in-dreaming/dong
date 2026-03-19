// DOM Selection implementation

#include "selection.hpp"
#include <vector>

namespace dong::dom {

Selection::Selection() = default;

bool Selection::isCollapsed() const {
    if (!range_) return true;
    return range_->isCollapsed();
}

uint32_t Selection::getRangeCount() const {
    return range_ ? 1 : 0;
}

Range* Selection::getRangeAt(uint32_t index) {
    if (index != 0 || !range_) return nullptr;
    return range_.get();
}

void Selection::addRange(const Range& range) {
    range_ = std::make_unique<Range>(range);
    anchor_node_ = range.getStartContainer();
    anchor_offset_ = range.getStartOffset();
    focus_node_ = range.getEndContainer();
    focus_offset_ = range.getEndOffset();
}

void Selection::removeAllRanges() {
    range_.reset();
    anchor_node_.reset();
    focus_node_.reset();
    anchor_offset_ = 0;
    focus_offset_ = 0;
}

void Selection::collapse(DOMNodePtr node, uint32_t offset) {
    anchor_node_ = node;
    anchor_offset_ = offset;
    focus_node_ = node;
    focus_offset_ = offset;
    syncRange();
}

void Selection::collapseToStart() {
    if (!range_) return;
    collapse(range_->getStartContainer(), range_->getStartOffset());
}

void Selection::collapseToEnd() {
    if (!range_) return;
    collapse(range_->getEndContainer(), range_->getEndOffset());
}

void Selection::extend(DOMNodePtr node, uint32_t offset) {
    focus_node_ = node;
    focus_offset_ = offset;
    syncRange();
}

void Selection::selectAllChildren(DOMNodePtr node) {
    if (!node) return;
    Range r;
    r.selectNodeContents(node);
    addRange(r);
}

std::string Selection::toString() const {
    if (!range_) return "";
    return range_->toString();
}

void Selection::setBaseAndExtent(DOMNodePtr anchor, uint32_t a_offset,
                                  DOMNodePtr focus, uint32_t f_offset) {
    anchor_node_ = anchor;
    anchor_offset_ = a_offset;
    focus_node_ = focus;
    focus_offset_ = f_offset;
    syncRange();
}

// Determine if node_a comes before node_b in document (depth-first) order.
// Returns true if a is before b, false otherwise.
static bool isNodeBefore(const DOMNodePtr& node_a, const DOMNodePtr& node_b) {
    if (!node_a || !node_b) return true;
    if (node_a == node_b) return true;

    // Build ancestor chains (node → root)
    auto buildChain = [](DOMNodePtr n) {
        std::vector<DOMNode*> chain;
        auto cur = n;
        while (cur) {
            chain.push_back(cur.get());
            cur = cur->getParent();
        }
        return chain;
    };

    auto chain_a = buildChain(node_a);
    auto chain_b = buildChain(node_b);

    // Walk from root (end of chains) towards leaves to find divergence point
    int ia = static_cast<int>(chain_a.size()) - 1;
    int ib = static_cast<int>(chain_b.size()) - 1;

    // Skip matching ancestors
    while (ia > 0 && ib > 0 && chain_a[ia] == chain_b[ib]) {
        --ia;
        --ib;
    }

    // If one is ancestor of the other
    if (chain_a[ia] == chain_b[ib]) return ia >= ib; // a is higher or same level → comes first

    // chain_a[ia] and chain_b[ib] are siblings under chain_a[ia+1]
    // Find which comes first in parent's children
    DOMNode* parent = (ia + 1 < static_cast<int>(chain_a.size())) ? chain_a[ia + 1] : nullptr;
    if (!parent) return true;

    // Walk from parent to find common parent shared_ptr
    auto parent_ptr = node_a;
    while (parent_ptr && parent_ptr.get() != parent) {
        parent_ptr = parent_ptr->getParent();
    }
    if (!parent_ptr) {
        parent_ptr = node_b;
        while (parent_ptr && parent_ptr.get() != parent) {
            parent_ptr = parent_ptr->getParent();
        }
    }
    if (!parent_ptr) return true;

    for (const auto& child : parent_ptr->getChildren()) {
        if (child.get() == chain_a[ia]) return true;  // a's branch comes first
        if (child.get() == chain_b[ib]) return false;  // b's branch comes first
    }

    return true; // fallback
}

void Selection::syncRange() {
    if (!anchor_node_ || !focus_node_) {
        range_.reset();
        return;
    }

    range_ = std::make_unique<Range>();

    bool anchor_first;
    if (anchor_node_ == focus_node_) {
        anchor_first = (anchor_offset_ <= focus_offset_);
    } else {
        anchor_first = isNodeBefore(anchor_node_, focus_node_);
    }

    if (anchor_first) {
        range_->setStart(anchor_node_, anchor_offset_);
        range_->setEnd(focus_node_, focus_offset_);
    } else {
        range_->setStart(focus_node_, focus_offset_);
        range_->setEnd(anchor_node_, anchor_offset_);
    }
}

} // namespace dong::dom
