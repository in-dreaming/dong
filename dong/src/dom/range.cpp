// DOM Range implementation

#include "range.hpp"
#include <algorithm>
#include <sstream>
#include <vector>

namespace dong::dom {

// Collect all text nodes in document order within a subtree.
static void collectTextNodes(const DOMNodePtr& node, std::vector<DOMNodePtr>& out) {
    if (!node) return;
    if (node->getType() == DOMNode::NodeType::TEXT) {
        out.push_back(node);
        return;
    }
    for (const auto& child : node->getChildren()) {
        collectTextNodes(child, out);
    }
}

Range::Range() = default;

bool Range::isCollapsed() const {
    return start_container_ == end_container_ && start_offset_ == end_offset_;
}

DOMNodePtr Range::getCommonAncestorContainer() const {
    if (!start_container_ || !end_container_) return nullptr;
    if (start_container_ == end_container_) return start_container_;

    // Build ancestor chain for start
    std::vector<DOMNode*> start_ancestors;
    auto current = start_container_;
    while (current) {
        start_ancestors.push_back(current.get());
        current = current->getParent();
    }

    // Walk up from end, find first match
    current = end_container_;
    while (current) {
        for (auto* ancestor : start_ancestors) {
            if (ancestor == current.get()) {
                return current;
            }
        }
        current = current->getParent();
    }

    return nullptr;
}

void Range::setStart(DOMNodePtr container, uint32_t offset) {
    start_container_ = container;
    start_offset_ = offset;
}

void Range::setEnd(DOMNodePtr container, uint32_t offset) {
    end_container_ = container;
    end_offset_ = offset;
}

void Range::setStartBefore(DOMNodePtr node) {
    if (!node || !node->getParent()) return;
    start_container_ = node->getParent();
    start_offset_ = findChildIndex(node->getParent(), node);
}

void Range::setStartAfter(DOMNodePtr node) {
    if (!node || !node->getParent()) return;
    start_container_ = node->getParent();
    start_offset_ = findChildIndex(node->getParent(), node) + 1;
}

void Range::setEndBefore(DOMNodePtr node) {
    if (!node || !node->getParent()) return;
    end_container_ = node->getParent();
    end_offset_ = findChildIndex(node->getParent(), node);
}

void Range::setEndAfter(DOMNodePtr node) {
    if (!node || !node->getParent()) return;
    end_container_ = node->getParent();
    end_offset_ = findChildIndex(node->getParent(), node) + 1;
}

void Range::collapse(bool to_start) {
    if (to_start) {
        end_container_ = start_container_;
        end_offset_ = start_offset_;
    } else {
        start_container_ = end_container_;
        start_offset_ = end_offset_;
    }
}

void Range::selectNodeContents(DOMNodePtr node) {
    if (!node) return;
    start_container_ = node;
    start_offset_ = 0;
    end_container_ = node;

    if (node->getType() == DOMNode::NodeType::TEXT) {
        end_offset_ = static_cast<uint32_t>(node->getRawTextContent().size());
    } else {
        end_offset_ = static_cast<uint32_t>(node->getChildren().size());
    }
}

void Range::selectNode(DOMNodePtr node) {
    if (!node || !node->getParent()) return;
    auto parent = node->getParent();
    uint32_t index = findChildIndex(parent, node);
    start_container_ = parent;
    start_offset_ = index;
    end_container_ = parent;
    end_offset_ = index + 1;
}

std::string Range::toString() const {
    if (!start_container_ || !end_container_) return "";

    // Simple case: both in same text node
    if (start_container_ == end_container_ &&
        start_container_->getType() == DOMNode::NodeType::TEXT) {
        const std::string& text = start_container_->getRawTextContent();
        uint32_t s = std::min(start_offset_, static_cast<uint32_t>(text.size()));
        uint32_t e = std::min(end_offset_, static_cast<uint32_t>(text.size()));
        if (s > e) std::swap(s, e);
        return text.substr(s, e - s);
    }

    // Cross-node: collect text from all text nodes in range
    auto ancestor = getCommonAncestorContainer();
    if (!ancestor) return "";

    // Resolve start/end to text nodes if they are element containers
    DOMNodePtr start_text = start_container_;
    uint32_t s_off = start_offset_;
    DOMNodePtr end_text = end_container_;
    uint32_t e_off = end_offset_;

    if (start_text->getType() != DOMNode::NodeType::TEXT) {
        // Element container: find first text node from child at offset
        std::vector<DOMNodePtr> texts;
        collectTextNodes(ancestor, texts);
        if (!texts.empty()) { start_text = texts.front(); s_off = 0; }
        else return "";
    }
    if (end_text->getType() != DOMNode::NodeType::TEXT) {
        std::vector<DOMNodePtr> texts;
        collectTextNodes(ancestor, texts);
        if (!texts.empty()) {
            end_text = texts.back();
            e_off = static_cast<uint32_t>(end_text->getRawTextContent().size());
        } else return "";
    }

    // Collect all text nodes under ancestor
    std::vector<DOMNodePtr> all_text;
    collectTextNodes(ancestor, all_text);

    int start_idx = -1, end_idx = -1;
    for (int i = 0; i < static_cast<int>(all_text.size()); ++i) {
        if (all_text[i] == start_text) start_idx = i;
        if (all_text[i] == end_text) end_idx = i;
    }
    if (start_idx < 0 || end_idx < 0 || start_idx > end_idx) return "";

    std::string result;
    for (int i = start_idx; i <= end_idx; ++i) {
        const std::string& text = all_text[i]->getRawTextContent();
        if (i == start_idx && i == end_idx) {
            uint32_t s = std::min(s_off, static_cast<uint32_t>(text.size()));
            uint32_t e = std::min(e_off, static_cast<uint32_t>(text.size()));
            result += text.substr(s, e - s);
        } else if (i == start_idx) {
            uint32_t s = std::min(s_off, static_cast<uint32_t>(text.size()));
            result += text.substr(s);
        } else if (i == end_idx) {
            uint32_t e = std::min(e_off, static_cast<uint32_t>(text.size()));
            result += text.substr(0, e);
        } else {
            result += text;
        }
    }
    return result;
}

Range Range::cloneRange() const {
    Range r;
    r.start_container_ = start_container_;
    r.start_offset_ = start_offset_;
    r.end_container_ = end_container_;
    r.end_offset_ = end_offset_;
    return r;
}

void Range::deleteContents() {
    if (!start_container_ || !end_container_) return;

    // Simple case: both in same text node
    if (start_container_ == end_container_ &&
        start_container_->getType() == DOMNode::NodeType::TEXT) {
        std::string text = start_container_->getRawTextContent();
        uint32_t s = std::min(start_offset_, static_cast<uint32_t>(text.size()));
        uint32_t e = std::min(end_offset_, static_cast<uint32_t>(text.size()));
        if (s > e) std::swap(s, e);
        text.erase(s, e - s);
        start_container_->setTextContent(text);
        end_container_ = start_container_;
        end_offset_ = s;
        start_offset_ = s;
        return;
    }

    // Cross-node deletion
    auto ancestor = getCommonAncestorContainer();
    if (!ancestor) return;

    // Resolve start/end to text nodes
    DOMNodePtr start_text = start_container_;
    uint32_t s_off = start_offset_;
    DOMNodePtr end_text = end_container_;
    uint32_t e_off = end_offset_;

    std::vector<DOMNodePtr> all_text;
    collectTextNodes(ancestor, all_text);
    if (all_text.empty()) return;

    if (start_text->getType() != DOMNode::NodeType::TEXT) {
        start_text = all_text.front();
        s_off = 0;
    }
    if (end_text->getType() != DOMNode::NodeType::TEXT) {
        end_text = all_text.back();
        e_off = static_cast<uint32_t>(end_text->getRawTextContent().size());
    }

    int start_idx = -1, end_idx = -1;
    for (int i = 0; i < static_cast<int>(all_text.size()); ++i) {
        if (all_text[i] == start_text) start_idx = i;
        if (all_text[i] == end_text) end_idx = i;
    }
    if (start_idx < 0 || end_idx < 0 || start_idx > end_idx) return;

    // Process from end to start to avoid index shifts
    for (int i = end_idx; i >= start_idx; --i) {
        auto& text_node = all_text[i];
        std::string text = text_node->getRawTextContent();

        if (i == start_idx && i == end_idx) {
            // Same node (shouldn't reach here, but handle it)
            uint32_t s = std::min(s_off, static_cast<uint32_t>(text.size()));
            uint32_t e = std::min(e_off, static_cast<uint32_t>(text.size()));
            text.erase(s, e - s);
            text_node->setTextContent(text);
        } else if (i == start_idx) {
            // Keep text before s_off
            uint32_t s = std::min(s_off, static_cast<uint32_t>(text.size()));
            text_node->setTextContent(text.substr(0, s));
        } else if (i == end_idx) {
            // Keep text after e_off
            uint32_t e = std::min(e_off, static_cast<uint32_t>(text.size()));
            text_node->setTextContent(text.substr(e));
        } else {
            // Middle node: remove entirely
            auto parent = text_node->getParent();
            if (parent) {
                parent->removeChild(text_node);
                // If the parent element is now empty (was a styled span with only this text),
                // remove it too
                if (parent->getChildren().empty() && parent->getTagName() == "span") {
                    auto grandparent = parent->getParent();
                    if (grandparent) {
                        grandparent->removeChild(parent);
                    }
                }
            }
        }
    }

    // Remove empty end text node
    if (end_idx != start_idx) {
        auto& end_node = all_text[end_idx];
        if (end_node->getRawTextContent().empty()) {
            auto parent = end_node->getParent();
            if (parent) {
                parent->removeChild(end_node);
                if (parent->getChildren().empty() && parent->getTagName() == "span") {
                    auto grandparent = parent->getParent();
                    if (grandparent) grandparent->removeChild(parent);
                }
            }
        }
    }

    // Remove empty start text node
    if (start_text->getRawTextContent().empty() && start_idx != end_idx) {
        auto parent = start_text->getParent();
        if (parent) {
            parent->removeChild(start_text);
            if (parent->getChildren().empty() && parent->getTagName() == "span") {
                auto grandparent = parent->getParent();
                if (grandparent) grandparent->removeChild(parent);
            }
        }
        // Collapse to first remaining text node
        std::vector<DOMNodePtr> remaining;
        collectTextNodes(ancestor, remaining);
        if (!remaining.empty()) {
            start_container_ = remaining.front();
            start_offset_ = 0;
        }
    } else {
        // Collapse to start position
        start_offset_ = s_off;
    }

    end_container_ = start_container_;
    end_offset_ = start_offset_;

    // Mark ancestor layout dirty
    if (ancestor) {
        ancestor->markLayoutDirty();
    }
}

int Range::compareBoundaryPoints(HowCompare how, const Range& other) const {
    DOMNodePtr this_container, other_container;
    uint32_t this_offset, other_offset;

    switch (how) {
        case START_TO_START:
            this_container = start_container_; this_offset = start_offset_;
            other_container = other.start_container_; other_offset = other.start_offset_;
            break;
        case START_TO_END:
            this_container = start_container_; this_offset = start_offset_;
            other_container = other.end_container_; other_offset = other.end_offset_;
            break;
        case END_TO_END:
            this_container = end_container_; this_offset = end_offset_;
            other_container = other.end_container_; other_offset = other.end_offset_;
            break;
        case END_TO_START:
            this_container = end_container_; this_offset = end_offset_;
            other_container = other.start_container_; other_offset = other.start_offset_;
            break;
    }

    if (!this_container || !other_container) return 0;
    if (this_container == other_container) {
        if (this_offset < other_offset) return -1;
        if (this_offset > other_offset) return 1;
        return 0;
    }

    // Different containers: check document order
    if (this_container->contains(other_container->shared_from_this())) return -1;
    if (other_container->contains(this_container->shared_from_this())) return 1;
    return 0;
}

uint32_t Range::findChildIndex(DOMNodePtr parent, DOMNodePtr child) {
    if (!parent || !child) return 0;
    const auto& children = parent->getChildren();
    for (uint32_t i = 0; i < children.size(); ++i) {
        if (children[i] == child) return i;
    }
    return 0;
}

} // namespace dong::dom
