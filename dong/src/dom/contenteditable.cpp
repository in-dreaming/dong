// ContentEditable implementation

#include "contenteditable.hpp"
#include "range.hpp"
#include "../core/log.h"
#include <algorithm>
#include <cctype>
#include <cstdint>

namespace dong::dom {

// --- UTF-8 helpers ---

// Returns the byte length of the UTF-8 character starting at s[offset].
// If offset is in the middle of a multi-byte sequence, returns 1 (error recovery).
static uint32_t utf8CharLenAt(const std::string& s, uint32_t offset) {
    if (offset >= s.size()) return 0;
    uint8_t c = static_cast<uint8_t>(s[offset]);
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1; // Invalid lead byte, treat as 1
}

// Returns the byte offset of the start of the character before s[offset].
// If offset is 0, returns 0.
static uint32_t utf8PrevCharStart(const std::string& s, uint32_t offset) {
    if (offset == 0 || s.empty()) return 0;
    // Walk backwards past continuation bytes (10xxxxxx)
    uint32_t pos = offset - 1;
    while (pos > 0 && (static_cast<uint8_t>(s[pos]) & 0xC0) == 0x80) {
        pos--;
    }
    return pos;
}

// Snap a byte offset to the nearest UTF-8 character boundary (backward).
static uint32_t utf8SnapToCharBoundary(const std::string& s, uint32_t offset) {
    if (offset == 0 || offset >= s.size()) return offset;
    // If we're on a continuation byte, walk backwards
    while (offset > 0 && (static_cast<uint8_t>(s[offset]) & 0xC0) == 0x80) {
        offset--;
    }
    return offset;
}

bool ContentEditableState::isContentEditable(const DOMNodePtr& node) {
    if (!node) return false;

    auto current = node;
    while (current) {
        if (current->hasAttribute("contenteditable")) {
            std::string val = current->getAttribute("contenteditable");
            if (val == "false") return false;
            // "true", "", or "plaintext-only" all count as editable
            return true;
        }
        current = current->getParent();
    }
    return false;
}

DOMNodePtr ContentEditableState::findEditableRoot(const DOMNodePtr& node) {
    if (!node) return nullptr;

    DOMNodePtr editable_root;
    auto current = node;
    while (current) {
        if (current->hasAttribute("contenteditable")) {
            std::string val = current->getAttribute("contenteditable");
            if (val == "false") break;
            editable_root = current;
        }
        current = current->getParent();
    }
    return editable_root;
}

bool ContentEditableState::insertText(const DOMNodePtr& editable_root,
                                       Selection& selection,
                                       const std::string& text) {
    if (!editable_root || text.empty()) return false;

    // Get caret position from selection
    if (selection.getRangeCount() == 0) return false;
    auto* range = selection.getRangeAt(0);
    if (!range) return false;

    // If selection is not collapsed, delete the selected content first
    if (!range->isCollapsed()) {
        range->deleteContents();
    }

    auto container = range->getStartContainer();
    uint32_t offset = range->getStartOffset();

    if (!container) return false;

    // If container is a text node, insert directly
    if (container->getType() == DOMNode::NodeType::TEXT) {
        std::string content = container->getRawTextContent();
        uint32_t insert_pos = std::min(offset, static_cast<uint32_t>(content.size()));
        content.insert(insert_pos, text);
        container->setTextContent(content);

        // Move caret past insertion
        uint32_t new_offset = insert_pos + static_cast<uint32_t>(text.size());
        selection.collapse(container, new_offset);

        // Mark layout dirty
        if (auto parent = container->getParent()) {
            parent->markLayoutDirty();
        }
        return true;
    }

    // Container is an element: create or use existing text node
    const auto& children = container->getChildren();
    if (offset < children.size()) {
        auto child = children[offset];
        if (child->getType() == DOMNode::NodeType::TEXT) {
            std::string content = child->getRawTextContent();
            content.insert(0, text);
            child->setTextContent(content);
            selection.collapse(child, static_cast<uint32_t>(text.size()));
            container->markLayoutDirty();
            return true;
        }
    }

    // Create new text node
    auto text_node = std::make_shared<DOMNode>(DOMNode::NodeType::TEXT);
    text_node->setTextContent(text);

    if (offset < children.size()) {
        container->insertBefore(text_node, children[offset]);
    } else {
        container->appendChild(text_node);
    }

    selection.collapse(text_node, static_cast<uint32_t>(text.size()));
    container->markLayoutDirty();
    return true;
}

bool ContentEditableState::deleteBackward(const DOMNodePtr& editable_root,
                                           Selection& selection) {
    if (!editable_root) return false;
    if (selection.getRangeCount() == 0) return false;
    auto* range = selection.getRangeAt(0);
    if (!range) return false;

    // If selection is not collapsed, delete the selection
    if (!range->isCollapsed()) {
        range->deleteContents();
        selection.collapse(range->getStartContainer(), range->getStartOffset());
        if (editable_root) editable_root->markLayoutDirty();
        return true;
    }

    auto container = range->getStartContainer();
    uint32_t offset = range->getStartOffset();
    if (!container) return false;

    if (container->getType() == DOMNode::NodeType::TEXT) {
        std::string content = container->getRawTextContent();
        if (offset == 0 || content.empty()) return false;

        // UTF-8 aware: find start of character before offset
        uint32_t char_start = utf8PrevCharStart(content, offset);
        uint32_t char_len = offset - char_start;
        content.erase(char_start, char_len);
        container->setTextContent(content);
        selection.collapse(container, char_start);

        if (auto parent = container->getParent()) {
            parent->markLayoutDirty();
        }
        return true;
    }

    return false;
}

bool ContentEditableState::deleteForward(const DOMNodePtr& editable_root,
                                          Selection& selection) {
    if (!editable_root) return false;
    if (selection.getRangeCount() == 0) return false;
    auto* range = selection.getRangeAt(0);
    if (!range) return false;

    // If selection is not collapsed, delete the selection
    if (!range->isCollapsed()) {
        range->deleteContents();
        selection.collapse(range->getStartContainer(), range->getStartOffset());
        if (editable_root) editable_root->markLayoutDirty();
        return true;
    }

    auto container = range->getStartContainer();
    uint32_t offset = range->getStartOffset();
    if (!container) return false;

    if (container->getType() == DOMNode::NodeType::TEXT) {
        std::string content = container->getRawTextContent();
        if (offset >= content.size()) return false;

        // UTF-8 aware: delete entire character at offset
        uint32_t char_len = utf8CharLenAt(content, offset);
        content.erase(offset, char_len);
        container->setTextContent(content);
        // Caret stays at same position

        if (auto parent = container->getParent()) {
            parent->markLayoutDirty();
        }
        return true;
    }

    return false;
}

bool ContentEditableState::insertParagraph(const DOMNodePtr& editable_root,
                                            Selection& selection) {
    if (!editable_root) return false;
    if (selection.getRangeCount() == 0) return false;
    auto* range = selection.getRangeAt(0);
    if (!range) return false;

    // Delete selection if not collapsed
    if (!range->isCollapsed()) {
        range->deleteContents();
    }

    auto container = range->getStartContainer();
    uint32_t offset = range->getStartOffset();
    if (!container) return false;

    // For text nodes: split into two parts with a <br> in between
    if (container->getType() == DOMNode::NodeType::TEXT) {
        auto parent = container->getParent();
        if (!parent) return false;

        std::string content = container->getRawTextContent();
        DONG_LOG_WARN("[insertParagraph] offset=%u content_size=%zu content=\"%s\"",
                       offset, content.size(), content.c_str());
        std::string before = content.substr(0, std::min(offset, static_cast<uint32_t>(content.size())));
        std::string after = offset < content.size() ? content.substr(offset) : "";

        container->setTextContent(before);

        // Create <br>
        auto br = std::make_shared<DOMNode>(DOMNode::NodeType::ELEMENT, "br");
        parent->insertBefore(br, container->getNextSibling());

        // Create text node for the rest
        if (!after.empty()) {
            auto after_node = std::make_shared<DOMNode>(DOMNode::NodeType::TEXT);
            after_node->setTextContent(after);
            parent->insertBefore(after_node, br->getNextSibling());
            selection.collapse(after_node, 0);
        } else {
            // Place caret after the BR
            auto after_node = std::make_shared<DOMNode>(DOMNode::NodeType::TEXT);
            after_node->setTextContent("");
            parent->insertBefore(after_node, br->getNextSibling());
            selection.collapse(after_node, 0);
        }

        parent->markLayoutDirty();
        return true;
    }

    return false;
}

// --- Caret movement helpers ---

DOMNodePtr ContentEditableState::findFirstTextNode(const DOMNodePtr& node) {
    if (!node) return nullptr;
    if (node->getType() == DOMNode::NodeType::TEXT) return node;
    for (const auto& child : node->getChildren()) {
        auto result = findFirstTextNode(child);
        if (result) return result;
    }
    return nullptr;
}

DOMNodePtr ContentEditableState::findLastTextNode(const DOMNodePtr& node) {
    if (!node) return nullptr;
    if (node->getType() == DOMNode::NodeType::TEXT) return node;
    const auto& children = node->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto result = findLastTextNode(*it);
        if (result) return result;
    }
    return nullptr;
}

DOMNodePtr ContentEditableState::findNextTextNode(const DOMNodePtr& root,
                                                    const DOMNodePtr& current) {
    if (!root || !current) return nullptr;

    // Walk siblings after current, then up and forward
    auto node = current;
    while (node && node != root) {
        auto next = node->getNextSibling();
        while (next) {
            auto text = findFirstTextNode(next);
            if (text) return text;
            next = next->getNextSibling();
        }
        node = node->getParent();
    }
    return nullptr;
}

DOMNodePtr ContentEditableState::findPrevTextNode(const DOMNodePtr& root,
                                                    const DOMNodePtr& current) {
    if (!root || !current) return nullptr;

    auto node = current;
    while (node && node != root) {
        auto prev = node->getPreviousSibling();
        while (prev) {
            auto text = findLastTextNode(prev);
            if (text) return text;
            prev = prev->getPreviousSibling();
        }
        node = node->getParent();
    }
    return nullptr;
}

bool ContentEditableState::moveCaret(const DOMNodePtr& editable_root,
                                      Selection& selection,
                                      int direction) {
    if (!editable_root || selection.getRangeCount() == 0) return false;

    // If there's a non-collapsed selection, collapse to the appropriate edge
    if (!selection.isCollapsed()) {
        if (direction < 0) {
            selection.collapseToStart();
        } else {
            selection.collapseToEnd();
        }
        return true;
    }

    auto* range = selection.getRangeAt(0);
    if (!range) return false;

    auto container = range->getStartContainer();
    uint32_t offset = range->getStartOffset();
    if (!container) return false;

    if (container->getType() == DOMNode::NodeType::TEXT) {
        const std::string& text = container->getRawTextContent();

        if (direction < 0) {
            if (offset > 0) {
                // UTF-8 aware: move to start of previous character
                uint32_t prev = utf8PrevCharStart(text, offset);
                selection.collapse(container, prev);
                return true;
            }
            // Move to end of previous text node
            auto prev = findPrevTextNode(editable_root, container);
            if (prev) {
                selection.collapse(prev, static_cast<uint32_t>(prev->getRawTextContent().size()));
                return true;
            }
        } else {
            if (offset < text.size()) {
                // UTF-8 aware: move past current character
                uint32_t char_len = utf8CharLenAt(text, offset);
                selection.collapse(container, offset + char_len);
                return true;
            }
            // Move to start of next text node
            auto next = findNextTextNode(editable_root, container);
            if (next) {
                selection.collapse(next, 0);
                return true;
            }
        }
    }

    return false;
}

bool ContentEditableState::moveCaretToLineEdge(const DOMNodePtr& editable_root,
                                                 Selection& selection,
                                                 bool to_end) {
    if (!editable_root || selection.getRangeCount() == 0) return false;

    auto* range = selection.getRangeAt(0);
    if (!range) return false;

    auto container = range->getStartContainer();
    if (!container) return false;

    if (container->getType() == DOMNode::NodeType::TEXT) {
        const std::string& text = container->getRawTextContent();
        if (to_end) {
            selection.collapse(container, static_cast<uint32_t>(text.size()));
        } else {
            selection.collapse(container, 0);
        }
        return true;
    }

    // For element containers, go to first/last text node
    if (to_end) {
        auto last = findLastTextNode(editable_root);
        if (last) {
            selection.collapse(last, static_cast<uint32_t>(last->getRawTextContent().size()));
            return true;
        }
    } else {
        auto first = findFirstTextNode(editable_root);
        if (first) {
            selection.collapse(first, 0);
            return true;
        }
    }

    return false;
}

bool ContentEditableState::extendSelection(const DOMNodePtr& editable_root,
                                             Selection& selection,
                                             int direction) {
    if (!editable_root || selection.getRangeCount() == 0) return false;

    auto focus = selection.getFocusNode();
    uint32_t focus_offset = selection.getFocusOffset();
    if (!focus) return false;

    if (focus->getType() == DOMNode::NodeType::TEXT) {
        const std::string& text = focus->getRawTextContent();

        if (direction < 0) {
            if (focus_offset > 0) {
                // UTF-8 aware: extend to start of previous character
                uint32_t prev = utf8PrevCharStart(text, focus_offset);
                selection.extend(focus, prev);
                return true;
            }
            auto prev = findPrevTextNode(editable_root, focus);
            if (prev) {
                selection.extend(prev, static_cast<uint32_t>(prev->getRawTextContent().size()));
                return true;
            }
        } else {
            if (focus_offset < text.size()) {
                // UTF-8 aware: extend past current character
                uint32_t char_len = utf8CharLenAt(text, focus_offset);
                selection.extend(focus, focus_offset + char_len);
                return true;
            }
            auto next = findNextTextNode(editable_root, focus);
            if (next) {
                selection.extend(next, 0);
                return true;
            }
        }
    }

    return false;
}

bool ContentEditableState::selectWordAtCaret(const DOMNodePtr& editable_root,
                                               Selection& selection) {
    if (!editable_root || selection.getRangeCount() == 0) return false;

    auto* range = selection.getRangeAt(0);
    if (!range) return false;

    auto container = range->getStartContainer();
    uint32_t offset = range->getStartOffset();
    if (!container || container->getType() != DOMNode::NodeType::TEXT) return false;

    const std::string& text = container->getRawTextContent();
    if (text.empty()) return false;

    // Clamp offset
    if (offset > text.size()) offset = static_cast<uint32_t>(text.size());

    // Find word boundaries (whitespace/punctuation)
    auto isWordChar = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    };

    // Find start of word
    uint32_t word_start = offset;
    if (word_start > 0 && word_start <= text.size()) {
        // If at a non-word char, look backward one more
        if (word_start == text.size() || !isWordChar(text[word_start])) {
            if (word_start > 0) word_start--;
        }
    }
    while (word_start > 0 && isWordChar(text[word_start - 1])) {
        word_start--;
    }

    // Find end of word
    uint32_t word_end = word_start;
    while (word_end < text.size() && isWordChar(text[word_end])) {
        word_end++;
    }

    // If we didn't find a word, select at least one character
    if (word_start == word_end && offset < text.size()) {
        word_start = offset;
        word_end = offset + 1;
    }

    selection.setBaseAndExtent(container, word_start, container, word_end);
    return true;
}

} // namespace dong::dom
