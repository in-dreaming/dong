// Editing commands implementation (document.execCommand)

#include "editing_commands.hpp"
#include "contenteditable.hpp"
#include "range.hpp"
#include "../core/log.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <vector>
namespace dong::dom {

// execCommand wraps text in <span style="font-weight:...">. Bold changes font metrics; if we do not
// pin line-height to the containing block, IFC can use a taller line box than plain text and the
// contenteditable block (min-height + border) visibly grows after bold — compare headless f0 vs f1.
//
// Must use an explicit px value on the span: `line-height: inherit` hits CSSParser's global-keyword
// branch and never sets ComputedStyle::has_line_height, so layout still keys off bold font metrics.
static void copyParentLineHeightOntoInlineWrapSpan(const DOMNodePtr& span, const DOMNodePtr& line_metrics_source) {
    if (!span || !line_metrics_source) {
        return;
    }
    const auto& cs = line_metrics_source->getComputedStyle();
    const float font_px = cs.font_size > 0.0f ? cs.font_size : 16.0f;
    // Match layout_engine.cpp: when no usable CSS line-height, line box is at least 1.2 * font-size.
    const float fallback_px = font_px * 1.2f;

    float line_height_px = fallback_px;
    if (cs.has_line_height && cs.line_height > 0.0f) {
        if (cs.line_height_is_unitless) {
            line_height_px = cs.line_height * font_px;
        } else {
            line_height_px = cs.line_height;
        }
    } else if (cs.has_line_height && cs.line_height <= 0.0f) {
        // Parsed `line-height: normal` stores -1; pin to the same minimum as layout_engine.
        line_height_px = fallback_px;
    }

    char buf[96];
    std::snprintf(buf, sizeof(buf), "%.1fpx", static_cast<double>(line_height_px));
    span->setInlineStyleProperty("line-height", std::string(buf));
}

EditingCommand parseEditingCommand(const std::string& command) {
    std::string lower;
    lower.resize(command.size());
    std::transform(command.begin(), command.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lower == "bold") return EditingCommand::BOLD;
    if (lower == "italic") return EditingCommand::ITALIC;
    if (lower == "underline") return EditingCommand::UNDERLINE;
    if (lower == "inserttext") return EditingCommand::INSERT_TEXT;
    if (lower == "delete") return EditingCommand::DELETE;
    if (lower == "selectall") return EditingCommand::SELECT_ALL;
    if (lower == "undo") return EditingCommand::UNDO;
    if (lower == "redo") return EditingCommand::REDO;
    return EditingCommand::UNKNOWN;
}

bool queryCommandSupported(const std::string& command) {
    auto cmd = parseEditingCommand(command);
    switch (cmd) {
        case EditingCommand::BOLD:
        case EditingCommand::ITALIC:
        case EditingCommand::UNDERLINE:
        case EditingCommand::INSERT_TEXT:
        case EditingCommand::DELETE:
        case EditingCommand::SELECT_ALL:
            return true;
        default:
            return false;
    }
}

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

// Resolve a Range whose containers may be element nodes into concrete text-node
// boundaries (start_text_node + char_offset, end_text_node + char_offset).
// This handles the case of selectAllChildren which sets container = element node.
struct ResolvedRange {
    DOMNodePtr start_text;
    uint32_t start_offset = 0;
    DOMNodePtr end_text;
    uint32_t end_offset = 0;
    bool valid = false;
};

static ResolvedRange resolveRange(const Range* range, const DOMNodePtr& editable_root) {
    ResolvedRange r;
    if (!range) return r;

    auto sc = range->getStartContainer();
    auto ec = range->getEndContainer();
    uint32_t so = range->getStartOffset();
    uint32_t eo = range->getEndOffset();
    if (!sc || !ec) return r;

    // If start container is a text node, use directly
    if (sc->getType() == DOMNode::NodeType::TEXT) {
        r.start_text = sc;
        r.start_offset = so;
    } else {
        // Element container: offset is child index. Find first text node at/after that child.
        const auto& children = sc->getChildren();
        DOMNodePtr search_from = (so < children.size()) ? children[so] : nullptr;
        if (search_from) {
            // Find first text node in this subtree
            std::vector<DOMNodePtr> texts;
            collectTextNodes(search_from, texts);
            if (!texts.empty()) {
                r.start_text = texts.front();
                r.start_offset = 0;
            }
        }
        if (!r.start_text) {
            // Fall back: first text node in the editable root
            std::vector<DOMNodePtr> texts;
            collectTextNodes(editable_root, texts);
            if (!texts.empty()) {
                r.start_text = texts.front();
                r.start_offset = 0;
            }
        }
    }

    // If end container is a text node, use directly
    if (ec->getType() == DOMNode::NodeType::TEXT) {
        r.end_text = ec;
        r.end_offset = eo;
    } else {
        // Element container: offset is child index. Find last text node before that index.
        const auto& children = ec->getChildren();
        if (eo > 0 && eo <= children.size()) {
            auto search_in = children[eo - 1];
            std::vector<DOMNodePtr> texts;
            collectTextNodes(search_in, texts);
            if (!texts.empty()) {
                r.end_text = texts.back();
                r.end_offset = static_cast<uint32_t>(r.end_text->getRawTextContent().size());
            }
        }
        if (!r.end_text) {
            std::vector<DOMNodePtr> texts;
            collectTextNodes(editable_root, texts);
            if (!texts.empty()) {
                r.end_text = texts.back();
                r.end_offset = static_cast<uint32_t>(r.end_text->getRawTextContent().size());
            }
        }
    }

    r.valid = (r.start_text && r.end_text);
    return r;
}

// Wrap a single text node (or a portion of it) in a styled <span>.
// Returns the text node inside the new span (for selection tracking).
static DOMNodePtr wrapTextNodePortion(const DOMNodePtr& text_node,
                                       uint32_t from, uint32_t to,
                                       const std::string& css_property,
                                       const std::string& css_value) {
    if (!text_node || text_node->getType() != DOMNode::NodeType::TEXT) return nullptr;
    auto parent = text_node->getParent();
    if (!parent) return nullptr;

    const std::string full_text = text_node->getRawTextContent();
    if (from >= to || to > full_text.size()) {
        return nullptr;
    }

    // Check toggle-off: if parent is a <span> with this exact style and we're wrapping the entire text
    if (parent->getTagName() == "span" && from == 0 && to == full_text.size()) {
        std::string existing = parent->getInlineStyleProperty(css_property);
        if (existing == css_value) {
            // Toggle off: remove the style
            parent->setInlineStyleProperty(css_property, "");
            parent->markStyleDirty();
            parent->markLayoutDirty();
            return text_node; // Return the same text node
        }
        // If parent is a span and we're wrapping the entire text, just add
        // the style to the existing span instead of creating a nested span.
        // This avoids deeply nested spans like <span bold><span italic>text</span></span>
        // which cause layout/hit-testing issues with inline elements.
        parent->setInlineStyleProperty(css_property, css_value);
        if (auto gp = parent->getParent()) {
            copyParentLineHeightOntoInlineWrapSpan(parent, gp);
        }
        parent->markStyleDirty();
        parent->markLayoutDirty();
        return text_node;
    }

    std::string before_text = full_text.substr(0, from);
    std::string selected_text = full_text.substr(from, to - from);
    std::string after_text = to < full_text.size() ? full_text.substr(to) : "";

    // Create styled span
    auto span = std::make_shared<DOMNode>(DOMNode::NodeType::ELEMENT, "span");
    span->setInlineStyleProperty(css_property, css_value);
    span->setInlineStyleProperty("display", "inline");
    copyParentLineHeightOntoInlineWrapSpan(span, parent);

    auto span_text = std::make_shared<DOMNode>(DOMNode::NodeType::TEXT);
    span_text->setTextContent(selected_text);
    span->appendChild(span_text);

    // Get next sibling before modifying
    auto next_sibling = text_node->getNextSibling();

    if (before_text.empty()) {
        // Replace the text node with the span
        parent->insertBefore(span, text_node);
        parent->removeChild(text_node);
    } else {
        // If before_text is only whitespace (e.g., "\n    " from HTML indentation),
        // discard it to avoid leaving orphan whitespace-only text nodes.
        bool all_ws = true;
        for (char c : before_text) {
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '\f') {
                all_ws = false;
                break;
            }
        }
        if (all_ws) {
            parent->insertBefore(span, text_node);
            parent->removeChild(text_node);
        } else {
            text_node->setTextContent(before_text);
            parent->insertBefore(span, next_sibling);
        }
    }

    if (!after_text.empty()) {
        // Don't create a separate text node for trailing-only whitespace
        // (e.g., the "\n" before </div> in HTML source). It would cause
        // IFC layout to create an extra empty line.
        bool all_ws = true;
        for (char c : after_text) {
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '\f') {
                all_ws = false;
                break;
            }
        }
        if (!all_ws) {
            auto after_node = std::make_shared<DOMNode>(DOMNode::NodeType::TEXT);
            after_node->setTextContent(after_text);
            parent->insertBefore(after_node, span->getNextSibling());
        }
    }

    parent->markStyleDirty();
    parent->markLayoutDirty();

    return span_text;
}

static bool isWhitespaceOnlyString(const std::string& s) {
    for (char c : s) {
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r' && c != '\f') {
            return false;
        }
    }
    return true;
}

static bool hasLineBreakChar(const std::string& s) {
    for (char c : s) {
        if (c == '\n' || c == '\r') {
            return true;
        }
    }
    return false;
}

// HTML parsing leaves newline+indent TEXT nodes before/after block content. Inserting inline
// <span> leaves them as siblings; IFC treats embedded newlines as line breaks, yielding an
// extra empty line after execCommand(bold). Strip only edge TEXT nodes that are whitespace
// with at least one newline (do not remove space-only indent without \n).
static void stripNewlineWhitespaceEdgeTextNodes(const DOMNodePtr& editable_root) {
    if (!editable_root) {
        return;
    }

    while (true) {
        const auto& ch = editable_root->getChildren();
        if (ch.empty()) {
            break;
        }
        const auto& fc = ch.front();
        if (!fc || fc->getType() != DOMNode::NodeType::TEXT) {
            break;
        }
        const std::string& t = fc->getRawTextContent();
        if (!isWhitespaceOnlyString(t) || !hasLineBreakChar(t)) {
            break;
        }
        editable_root->removeChild(fc);
    }

    while (true) {
        const auto& ch = editable_root->getChildren();
        if (ch.empty()) {
            break;
        }
        const auto& lc = ch.back();
        if (!lc || lc->getType() != DOMNode::NodeType::TEXT) {
            break;
        }
        const std::string& t = lc->getRawTextContent();
        if (!isWhitespaceOnlyString(t) || !hasLineBreakChar(t)) {
            break;
        }
        editable_root->removeChild(lc);
    }
}

// Apply a style to the selected range. Handles single-node and multi-node selections.
static bool wrapSelectionWithStyle(const DOMNodePtr& editable_root,
                                    Selection& selection,
                                    const std::string& css_property,
                                    const std::string& css_value,
                                    const std::string& css_value_off) {
    if (!editable_root || selection.getRangeCount() == 0) {
        return false;
    }
    auto* range = selection.getRangeAt(0);
    if (!range || range->isCollapsed()) {
        return false;
    }

    // Resolve the range to text-node boundaries (handles element-level selections)
    auto resolved = resolveRange(range, editable_root);
    if (!resolved.valid) {
        return false;
    }

    // Collect all text nodes in the editable root
    std::vector<DOMNodePtr> all_text_nodes;
    collectTextNodes(editable_root, all_text_nodes);
    if (all_text_nodes.empty()) return false;

    // Find indices of start/end text nodes
    int start_idx = -1, end_idx = -1;
    for (int i = 0; i < static_cast<int>(all_text_nodes.size()); ++i) {
        if (all_text_nodes[i] == resolved.start_text) start_idx = i;
        if (all_text_nodes[i] == resolved.end_text) end_idx = i;
    }
    if (start_idx < 0 || end_idx < 0 || start_idx > end_idx) {
        return false;
    }

    // Track first and last wrapped text nodes for selection update
    DOMNodePtr first_wrapped, last_wrapped;
    uint32_t first_wrapped_offset = 0, last_wrapped_length = 0;

    // Process text nodes from last to first (so indices don't shift)
    for (int i = end_idx; i >= start_idx; --i) {
        auto& text_node = all_text_nodes[i];
        const std::string& text = text_node->getRawTextContent();

        uint32_t wrap_from = 0;
        uint32_t wrap_to = static_cast<uint32_t>(text.size());

        if (i == start_idx) {
            wrap_from = resolved.start_offset;
        }
        if (i == end_idx) {
            wrap_to = resolved.end_offset;
        }

        // Skip empty ranges
        if (wrap_from >= wrap_to) continue;

        auto wrapped_text = wrapTextNodePortion(text_node, wrap_from, wrap_to,
                                                 css_property, css_value);
        if (wrapped_text) {
            if (i == start_idx) {
                first_wrapped = wrapped_text;
                first_wrapped_offset = 0;
            }
            if (i == end_idx) {
                last_wrapped = wrapped_text;
                last_wrapped_length = static_cast<uint32_t>(wrapped_text->getRawTextContent().size());
            }
        }
    }

    // Update selection to cover the wrapped text
    if (first_wrapped && last_wrapped) {
        selection.setBaseAndExtent(first_wrapped, first_wrapped_offset,
                                    last_wrapped, last_wrapped_length);
    }

    stripNewlineWhitespaceEdgeTextNodes(editable_root);

    editable_root->markStyleDirty();
    editable_root->markLayoutDirty();

    // Debug: dump DOM tree after style wrapping
    DONG_LOG_WARN("[BOLD-DOM] editable_root children=%zu", editable_root->getChildren().size());
    for (size_t i = 0; i < editable_root->getChildren().size(); ++i) {
        const auto& ch = editable_root->getChildren()[i];
        if (!ch) continue;
        if (ch->getType() == DOMNode::NodeType::TEXT) {
            const std::string& raw = ch->getRawTextContent();
            DONG_LOG_WARN("[BOLD-DOM]   [%zu] TEXT len=%zu \"%s\"",
                          i, raw.size(), raw.substr(0, 60).c_str());
        } else if (ch->getType() == DOMNode::NodeType::ELEMENT) {
            const auto& cs = ch->getComputedStyle();
            DONG_LOG_WARN("[BOLD-DOM]   [%zu] <%s> display=%s layout_mode=%d style=\"%s\"",
                          i, ch->getTagName().c_str(), cs.display.c_str(), (int)cs.layout_mode,
                          ch->hasAttribute("style") ? ch->getAttribute("style").c_str() : "");
            for (size_t j = 0; j < ch->getChildren().size(); ++j) {
                const auto& gc = ch->getChildren()[j];
                if (gc && gc->getType() == DOMNode::NodeType::TEXT) {
                    DONG_LOG_WARN("[BOLD-DOM]     [%zu] TEXT \"%s\"", j, gc->getRawTextContent().c_str());
                }
            }
        }
    }

    return true;
}

static bool execBold(const DOMNodePtr& editable_root, Selection& selection) {
    return wrapSelectionWithStyle(editable_root, selection,
                                  "font-weight", "bold", "normal");
}

static bool execItalic(const DOMNodePtr& editable_root, Selection& selection) {
    return wrapSelectionWithStyle(editable_root, selection,
                                  "font-style", "italic", "normal");
}

static bool execUnderline(const DOMNodePtr& editable_root, Selection& selection) {
    return wrapSelectionWithStyle(editable_root, selection,
                                  "text-decoration", "underline", "none");
}

static bool execSelectAll(const DOMNodePtr& editable_root, Selection& selection) {
    if (!editable_root) return false;
    selection.selectAllChildren(editable_root);
    return true;
}

bool execCommand(const DOMNodePtr& editable_root,
                 Selection& selection,
                 const std::string& command,
                 const std::string& value) {
    auto cmd = parseEditingCommand(command);

    switch (cmd) {
        case EditingCommand::BOLD:
            return execBold(editable_root, selection);
        case EditingCommand::ITALIC:
            return execItalic(editable_root, selection);
        case EditingCommand::UNDERLINE:
            return execUnderline(editable_root, selection);
        case EditingCommand::INSERT_TEXT:
            return ContentEditableState::insertText(editable_root, selection, value);
        case EditingCommand::DELETE:
            return ContentEditableState::deleteBackward(editable_root, selection);
        case EditingCommand::SELECT_ALL:
            return execSelectAll(editable_root, selection);
        default:
            return false;
    }
}

} // namespace dong::dom
