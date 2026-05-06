#include "dom_node.hpp"
#include "../css/style_engine.hpp"
#include "../css/selector_matcher.hpp"
#include "../focus_manager.hpp"
#include "../event_system.hpp"
#include "../html/html_parser.hpp"
#include "../../core/string_utils.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>
#include <limits>
#include <cmath>


namespace dong::dom {

// Static member definitions
FocusManager* DOMNode::s_focus_manager_ = nullptr;
EventDispatcher* DOMNode::s_event_dispatcher_ = nullptr;
std::string DOMNode::s_current_fragment_ = "";
DOMNodeWeakPtr DOMNode::s_pointer_capture_;
DOMNodePtr DOMNode::s_modal_dialog_root_;


// ClassList implementation
ClassList::ClassList(DOMNode* node) : node_(node) {}

std::vector<std::string> ClassList::getClasses() const {
    std::vector<std::string> classes;
    std::string class_attr = node_->getAttribute("class");
    std::istringstream iss(class_attr);
    std::string cls;
    while (iss >> cls) {
        classes.push_back(cls);
    }
    return classes;
}

void ClassList::updateAttribute() {
    auto classes = getClasses();
    std::string result;
    for (size_t i = 0; i < classes.size(); ++i) {
        if (i > 0) result += " ";
        result += classes[i];
    }
    if (result.empty()) {
        node_->removeAttribute("class");
    } else {
        node_->setAttribute("class", result);
    }
}

void ClassList::add(const std::string& className) {
    if (contains(className)) return;
    std::string current = node_->getAttribute("class");
    if (current.empty()) {
        node_->setAttribute("class", className);
    } else {
        node_->setAttribute("class", current + " " + className);
    }
}

void ClassList::remove(const std::string& className) {
    auto classes = getClasses();
    classes.erase(std::remove(classes.begin(), classes.end(), className), classes.end());
    std::string result;
    for (size_t i = 0; i < classes.size(); ++i) {
        if (i > 0) result += " ";
        result += classes[i];
    }
    if (result.empty()) {
        node_->removeAttribute("class");
    } else {
        node_->setAttribute("class", result);
    }
}

bool ClassList::toggle(const std::string& className) {
    if (contains(className)) {
        remove(className);
        return false;
    } else {
        add(className);
        return true;
    }
}

bool ClassList::toggle(const std::string& className, bool force) {
    if (force) {
        add(className);
        return true;
    } else {
        remove(className);
        return false;
    }
}

bool ClassList::contains(const std::string& className) const {
    auto classes = getClasses();
    return std::find(classes.begin(), classes.end(), className) != classes.end();
}

void ClassList::replace(const std::string& oldClass, const std::string& newClass) {
    auto classes = getClasses();
    auto it = std::find(classes.begin(), classes.end(), oldClass);
    if (it != classes.end()) {
        *it = newClass;
        std::string result;
        for (size_t i = 0; i < classes.size(); ++i) {
            if (i > 0) result += " ";
            result += classes[i];
        }
        node_->setAttribute("class", result);
    }
}

std::string ClassList::item(size_t index) const {
    auto classes = getClasses();
    if (index < classes.size()) {
        return classes[index];
    }
    return "";
}

size_t ClassList::length() const {
    return getClasses().size();
}

std::string ClassList::value() const {
    return node_->getAttribute("class");
}

void ClassList::setValue(const std::string& value) {
    node_->setAttribute("class", value);
}

// DOMNode implementation
DOMNode::DOMNode(NodeType type, const std::string& name)
    : type_(type), tag_name_(name) {}

std::string DOMNode::getNodeName() const {
    switch (type_) {
        case NodeType::ELEMENT:
            return tag_name_;
        case NodeType::TEXT:
            return "#text";
        case NodeType::COMMENT:
            return "#comment";
        case NodeType::DOCUMENT:
            return "#document";
        case NodeType::DOCUMENT_FRAGMENT:
            return "#document-fragment";
        default:
            return "";
    }
}

std::string DOMNode::getNodeValue() const {
    switch (type_) {
        case NodeType::TEXT:
        case NodeType::COMMENT:
            return text_content_;
        default:
            return "";
    }
}

void DOMNode::setNodeValue(const std::string& value) {
    if (type_ == NodeType::TEXT || type_ == NodeType::COMMENT) {
        text_content_ = value;
        markLayoutDirty();
    }
}

std::string DOMNode::getTextContent() const {
    if (type_ == NodeType::TEXT) {
        return text_content_;
    }
    
    std::string result;
    for (const auto& child : children_) {
        if (!child) continue;
        result += child->getTextContent();
    }
    return result;
}

void DOMNode::setTextContent(const std::string& text) {
    if (type_ == NodeType::TEXT) {
        text_content_ = text;
    } else {
        // Remove all children and add a text node
        children_.clear();
        if (!text.empty()) {
            auto text_node = std::make_shared<DOMNode>(NodeType::TEXT);
            text_node->text_content_ = text;
            appendChild(text_node);
        }
    }
    markLayoutDirty();
}

void DOMNode::appendChild(DOMNodePtr child) {
    if (!child) return;
    
    // Remove from old parent
    if (auto old_parent = child->parent_.lock()) {
        old_parent->removeChild(child);
    }
    
    children_.push_back(child);
    child->parent_ = std::static_pointer_cast<DOMNode>(shared_from_this());

    // Adding/removing element children can affect selector matching and computed styles.
    if (child->type_ == NodeType::ELEMENT) {
        markStyleDirty();
    }

    markLayoutDirty();
}


void DOMNode::insertBefore(DOMNodePtr newChild, DOMNodePtr refChild) {
    if (!newChild) return;

    if (!refChild) {
        appendChild(newChild);
        return;
    }

    // Remove from old parent
    if (auto old_parent = newChild->parent_.lock()) {
        old_parent->removeChild(newChild);
    }

    auto it = std::find(children_.begin(), children_.end(), refChild);
    if (it == children_.end()) {
        // Be robust: if the reference child isn't found, fall back to append.
        // This prevents silent drops in APIs like insertAdjacentHTML/document.write.
        appendChild(newChild);
        return;
    }

    children_.insert(it, newChild);
    newChild->parent_ = std::static_pointer_cast<DOMNode>(shared_from_this());

    if (newChild->type_ == NodeType::ELEMENT) {
        markStyleDirty();
    }

    markLayoutDirty();
}


void DOMNode::removeChild(DOMNodePtr child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        const bool removed_element = (child && child->type_ == NodeType::ELEMENT);

        children_.erase(it);
        child->parent_.reset();

        if (removed_element) {
            markStyleDirty();
        }

        markLayoutDirty();
    }
}


void DOMNode::replaceChild(DOMNodePtr newChild, DOMNodePtr oldChild) {
    if (!newChild || !oldChild) return;
    
    auto it = std::find(children_.begin(), children_.end(), oldChild);
    if (it != children_.end()) {
        // Remove newChild from old parent
        if (auto old_parent = newChild->parent_.lock()) {
            old_parent->removeChild(newChild);
        }
        
        *it = newChild;
        oldChild->parent_.reset();
        newChild->parent_ = std::static_pointer_cast<DOMNode>(shared_from_this());

        // Replacing element children can affect selector matching.
        if (newChild->type_ == NodeType::ELEMENT || oldChild->type_ == NodeType::ELEMENT) {
            markStyleDirty();
        }

        markLayoutDirty();
    }
}


DOMNodePtr DOMNode::cloneNode(bool deep) const {
    auto clone = std::make_shared<DOMNode>(type_, tag_name_);
    clone->text_content_ = text_content_;
    clone->attributes_ = attributes_;
    clone->inline_styles_ = inline_styles_;
    clone->computed_style_ = computed_style_;
    
    if (deep) {
        for (const auto& child : children_) {
            clone->appendChild(child->cloneNode(true));
        }
    }
    
    return clone;
}

bool DOMNode::contains(DOMNodePtr node) const {
    if (!node) return false;
    if (node.get() == this) return true;
    
    for (const auto& child : children_) {
        if (child == node || child->contains(node)) {
            return true;
        }
    }
    return false;
}

void DOMNode::normalize() {
    std::vector<DOMNodePtr> new_children;
    DOMNodePtr prev_text = nullptr;
    
    for (const auto& child : children_) {
        if (child->type_ == NodeType::TEXT) {
            if (child->text_content_.empty()) {
                continue;  // Remove empty text nodes
            }
            if (prev_text) {
                // Merge with previous text node
                prev_text->text_content_ += child->text_content_;
            } else {
                prev_text = child;
                new_children.push_back(child);
            }
        } else {
            prev_text = nullptr;
            child->normalize();
            new_children.push_back(child);
        }
    }
    
    children_ = std::move(new_children);
}

bool DOMNode::isEqualNode(DOMNodePtr other) const {
    if (!other) return false;
    if (type_ != other->type_) return false;
    if (tag_name_ != other->tag_name_) return false;
    if (text_content_ != other->text_content_) return false;
    if (attributes_ != other->attributes_) return false;
    if (children_.size() != other->children_.size()) return false;
    
    for (size_t i = 0; i < children_.size(); ++i) {
        if (!children_[i]->isEqualNode(other->children_[i])) {
            return false;
        }
    }
    
    return true;
}

bool DOMNode::isSameNode(DOMNodePtr other) const {
    return other.get() == this;
}

int DOMNode::compareDocumentPosition(DOMNodePtr other) const {
    if (!other) return 0;
    if (other.get() == this) return 0;
    
    // Simplified implementation
    if (contains(other)) return 16;  // DOCUMENT_POSITION_CONTAINED_BY
    if (other->contains(std::const_pointer_cast<DOMNode>(shared_from_this()))) {
        return 8;  // DOCUMENT_POSITION_CONTAINS
    }
    
    return 1;  // DOCUMENT_POSITION_DISCONNECTED
}

DOMNodePtr DOMNode::getParentElement() const {
    auto parent = parent_.lock();
    while (parent) {
        if (parent->type_ == NodeType::ELEMENT) {
            return parent;
        }
        parent = parent->parent_.lock();
    }
    return nullptr;
}

DOMNodePtr DOMNode::getFirstChild() const {
    return children_.empty() ? nullptr : children_.front();
}

DOMNodePtr DOMNode::getLastChild() const {
    return children_.empty() ? nullptr : children_.back();
}

DOMNodePtr DOMNode::getPreviousSibling() const {
    auto parent = parent_.lock();
    if (!parent) return nullptr;
    
    const auto& siblings = parent->children_;
    for (size_t i = 1; i < siblings.size(); ++i) {
        if (siblings[i].get() == this) {
            return siblings[i - 1];
        }
    }
    return nullptr;
}

DOMNodePtr DOMNode::getNextSibling() const {
    auto parent = parent_.lock();
    if (!parent) return nullptr;
    
    const auto& siblings = parent->children_;
    for (size_t i = 0; i + 1 < siblings.size(); ++i) {
        if (siblings[i].get() == this) {
            return siblings[i + 1];
        }
    }
    return nullptr;
}

DOMNodePtr DOMNode::getFirstElementChild() const {
    for (const auto& child : children_) {
        if (child->type_ == NodeType::ELEMENT) {
            return child;
        }
    }
    return nullptr;
}

DOMNodePtr DOMNode::getLastElementChild() const {
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->type_ == NodeType::ELEMENT) {
            return *it;
        }
    }
    return nullptr;
}

DOMNodePtr DOMNode::getPreviousElementSibling() const {
    auto parent = parent_.lock();
    if (!parent) return nullptr;
    
    const auto& siblings = parent->children_;
    bool found_self = false;
    
    for (auto it = siblings.rbegin(); it != siblings.rend(); ++it) {
        if ((*it).get() == this) {
            found_self = true;
            continue;
        }
        if (found_self && (*it)->type_ == NodeType::ELEMENT) {
            return *it;
        }
    }
    return nullptr;
}

DOMNodePtr DOMNode::getNextElementSibling() const {
    auto parent = parent_.lock();
    if (!parent) return nullptr;
    
    const auto& siblings = parent->children_;
    bool found_self = false;
    
    for (const auto& sibling : siblings) {
        if (sibling.get() == this) {
            found_self = true;
            continue;
        }
        if (found_self && sibling->type_ == NodeType::ELEMENT) {
            return sibling;
        }
    }
    return nullptr;
}

size_t DOMNode::getChildElementCount() const {
    size_t count = 0;
    for (const auto& child : children_) {
        if (child->type_ == NodeType::ELEMENT) {
            ++count;
        }
    }
    return count;
}

DOMNodePtr DOMNode::getOwnerDocument() const {
    auto current = parent_.lock();
    while (current) {
        if (current->type_ == NodeType::DOCUMENT) {
            return current;
        }
        current = current->parent_.lock();
    }
    return nullptr;
}

ClassList& DOMNode::getClassList() {
    if (!class_list_) {
        class_list_ = std::make_unique<ClassList>(this);
    }
    return *class_list_;
}

void DOMNode::setAttribute(const std::string& key, const std::string& value) {
    const bool had_attr = hasAttribute(key);
    attributes_[key] = value;

    // Internal attributes used by the renderer should not affect style/layout.
    // (e.g. Painter's transient markers)
    if (key.size() >= 2 && key[0] == '_' && key[1] == '_') {
        return;
    }

    // <details open> toggles dispatch a 'toggle' event when the open state changes.
    if (tag_name_ == "details" && key == "open" && !had_attr) {
        if (s_event_dispatcher_) {
            Event ev = s_event_dispatcher_->createEvent(EventType::TOGGLE);
            ev.target = shared_from_this();
            ev.current_target = shared_from_this();
            ev.is_trusted = false;
            s_event_dispatcher_->dispatch(ev);
        }
    }

    markStyleDirty();
    markLayoutDirty();
}


std::string DOMNode::getAttribute(const std::string& key) const {
    auto it = attributes_.find(key);
    return it != attributes_.end() ? it->second : "";
}

bool DOMNode::hasAttribute(const std::string& key) const {
    return attributes_.find(key) != attributes_.end();
}

void DOMNode::removeAttribute(const std::string& key) {
    const bool had_attr = hasAttribute(key);
    attributes_.erase(key);

    // Internal attributes used by the renderer should not affect style/layout.
    if (key.size() >= 2 && key[0] == '_' && key[1] == '_') {
        return;
    }

    // <details open> toggles dispatch a 'toggle' event when the open state changes.
    if (tag_name_ == "details" && key == "open" && had_attr) {
        if (s_event_dispatcher_) {
            Event ev = s_event_dispatcher_->createEvent(EventType::TOGGLE);
            ev.target = shared_from_this();
            ev.current_target = shared_from_this();
            ev.is_trusted = false;
            s_event_dispatcher_->dispatch(ev);
        }
    }

    markStyleDirty();
    markLayoutDirty();
}


bool DOMNode::toggleAttribute(const std::string& name) {
    if (hasAttribute(name)) {
        removeAttribute(name);
        return false;
    } else {
        setAttribute(name, "");
        return true;
    }
}

bool DOMNode::toggleAttribute(const std::string& name, bool force) {
    if (force) {
        setAttribute(name, "");
        return true;
    } else {
        removeAttribute(name);
        return false;
    }
}

std::vector<std::string> DOMNode::getAttributeNames() const {
    std::vector<std::string> names;
    names.reserve(attributes_.size());
    for (const auto& [key, _] : attributes_) {
        names.push_back(key);
    }
    return names;
}

std::string DOMNode::getEffectiveLang() const {
    // Check if this element has a lang attribute
    auto it = attributes_.find("lang");
    if (it != attributes_.end() && !it->second.empty()) {
        // Normalize the lang tag: "en-US" -> "en-us" (lowercase language, lowercase country)
        std::string lang = it->second;
        std::transform(lang.begin(), lang.end(), lang.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return lang;
    }

    // Inherit from parent (recursively)
    if (auto p = parent_.lock()) {
        return p->getEffectiveLang();
    }

    // No lang attribute in any ancestor, return empty string
    return "";
}

// Helper function to detect direction from text content
// Returns direction of first strong directional character (L or R type)
static std::string detectTextDirection(const std::string& text) {
    // RFC 9386: Bidi character classes
    // R: Right-to-Left letters (Arabic, Hebrew)
    // AL: Arabic Letter (strong RTL)
    // L: Left-to-Right letters
    //
    // These ranges are simplified but cover the most common characters.
    // For full accuracy, we would use the full Unicode bidi database.
    for (size_t i = 0; i < text.length(); ) {
        unsigned char ch = static_cast<unsigned char>(text[i]);

        // Handle multi-byte UTF-8 sequences
        uint32_t codepoint = 0;
        int bytes = 0;
        if ((ch & 0x80) == 0) {
            // 1-byte ASCII
            codepoint = ch;
            bytes = 1;
        } else if ((ch & 0xE0) == 0xC0) {
            // 2-byte sequence
            if (i + 1 < text.length()) {
                codepoint = ((ch & 0x1F) << 6) | (text[i + 1] & 0x3F);
                bytes = 2;
            } else {
                break;
            }
        } else if ((ch & 0xF0) == 0xE0) {
            // 3-byte sequence
            if (i + 2 < text.length()) {
                codepoint = ((ch & 0x0F) << 12) | ((text[i + 1] & 0x3F) << 6) | (text[i + 2] & 0x3F);
                bytes = 3;
            } else {
                break;
            }
        } else if ((ch & 0xF8) == 0xF0) {
            // 4-byte sequence
            if (i + 3 < text.length()) {
                codepoint = ((ch & 0x07) << 18) | ((text[i + 1] & 0x3F) << 12) |
                           ((text[i + 2] & 0x3F) << 6) | (text[i + 3] & 0x3F);
                bytes = 4;
            } else {
                break;
            }
        } else {
            i++;
            continue;
        }

        // RTL characters (simplified ranges)
        // Hebrew: U+0590-U+05FF
        if (codepoint >= 0x0590 && codepoint <= 0x05FF) {
            return "rtl";
        }
        // Arabic: U+0600-U+06FF
        if (codepoint >= 0x0600 && codepoint <= 0x06FF) {
            return "rtl";
        }
        // Arabic Extended-A: U+0750-U+077F
        if (codepoint >= 0x0750 && codepoint <= 0x077F) {
            return "rtl";
        }
        // Arabic Presentation Forms: U+FB50-U+FDFF, U+FE70-U+FEFF
        if (codepoint >= 0xFB50 && codepoint <= 0xFDFF) {
            return "rtl";
        }
        if (codepoint >= 0xFE70 && codepoint <= 0xFEFF) {
            return "rtl";
        }
        // Syriac: U+0700-U+074F
        if (codepoint >= 0x0700 && codepoint <= 0x074F) {
            return "rtl";
        }
        // Thaana: U+0780-U+07BF
        if (codepoint >= 0x0780 && codepoint <= 0x07BF) {
            return "rtl";
        }
        // N'Ko: U+07C0-U+07FF
        if (codepoint >= 0x07C0 && codepoint <= 0x07FF) {
            return "rtl";
        }
        // LTR characters (Latin, Greek, Cyrillic, etc.)
        // Basic Latin: U+0041-U+005A, U+0061-U+007A
        if ((codepoint >= 0x0041 && codepoint <= 0x005A) ||
            (codepoint >= 0x0061 && codepoint <= 0x007A)) {
            return "ltr";
        }
        // Greek: U+0370-U+03FF
        if (codepoint >= 0x0370 && codepoint <= 0x03FF) {
            return "ltr";
        }
        // Cyrillic: U+0400-U+04FF
        if (codepoint >= 0x0400 && codepoint <= 0x04FF) {
            return "ltr";
        }
        // Armenian: U+0530-U+058F
        if (codepoint >= 0x0530 && codepoint <= 0x058F) {
            return "ltr";
        }
        // Georgian: U+10A0-U+10FF
        if (codepoint >= 0x10A0 && codepoint <= 0x10FF) {
            return "ltr";
        }
        // CJK: U+4E00-U+9FFF (CJK Unified Ideographs)
        if (codepoint >= 0x4E00 && codepoint <= 0x9FFF) {
            return "ltr";
        }
        // Hiragana: U+3040-U+309F
        if (codepoint >= 0x3040 && codepoint <= 0x309F) {
            return "ltr";
        }
        // Katakana: U+30A0-U+30FF
        if (codepoint >= 0x30A0 && codepoint <= 0x30FF) {
            return "ltr";
        }
        // Hangul: U+AC00-U+D7AF
        if (codepoint >= 0xAC00 && codepoint <= 0xD7AF) {
            return "ltr";
        }

        i += bytes;
    }

    // No strong directional character found, return "ltr" as default
    return "ltr";
}

std::string DOMNode::getEffectiveDirection() const {
    // Check if this element has a dir attribute
    auto it = attributes_.find("dir");
    if (it != attributes_.end() && !it->second.empty()) {
        std::string dir = it->second;
        std::transform(dir.begin(), dir.end(), dir.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (dir == "ltr" || dir == "rtl") {
            return dir;
        } else if (dir == "auto") {
            // For dir=auto, detect direction from text content
            // Get all text content from this node and its descendants
            std::string text_content = "";
            for (const auto& child : children_) {
                if (child->getType() == NodeType::TEXT) {
                    text_content += child->getTextContent();
                } else if (child->getType() == NodeType::ELEMENT) {
                    text_content += child->getTextContent();
                }
            }
            return detectTextDirection(text_content);
        }
        // Invalid dir value, treat as if not set
    }

    // Inherit from parent (direction is inherited)
    if (auto p = parent_.lock()) {
        return p->getEffectiveDirection();
    }

    // No dir attribute in any ancestor, return default "ltr"
    return "ltr";
}

std::string DOMNode::getInnerHTML() const {
    std::string result;
    for (const auto& child : children_) {
        result += child->getOuterHTML();
    }
    return result;
}

void DOMNode::setInnerHTML(const std::string& html) {
    // Clear existing children
    children_.clear();

    // Parse HTML fragment and add children
    if (!html.empty()) {
        HTMLParser parser;
        auto fragment = parser.parseFragment(html, shared_from_this());
        if (fragment) {
            // Move all children from fragment to this node
            for (const auto& child : fragment->getChildren()) {
                appendChild(child->cloneNode(true));
            }
        }
    }
    // Mark both style and layout dirty to ensure new nodes get styled and laid out
    markStyleDirty();
    markLayoutDirty();
}

std::string DOMNode::getOuterHTML() const {
    if (type_ == NodeType::TEXT) {
        return text_content_;
    }
    if (type_ == NodeType::COMMENT) {
        return "<!--" + text_content_ + "-->";
    }
    if (type_ != NodeType::ELEMENT) {
        return "";
    }
    
    std::string result = "<" + tag_name_;
    for (const auto& [key, value] : attributes_) {
        result += " " + key + "=\"" + value + "\"";
    }
    
    // Self-closing tags
    static const std::unordered_set<std::string> void_elements = {
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr"
    };
    
    if (void_elements.count(tag_name_) > 0) {
        result += " />";
    } else {
        result += ">";
        result += getInnerHTML();
        result += "</" + tag_name_ + ">";
    }
    
    return result;
}

void DOMNode::setOuterHTML(const std::string& html) {
    auto parent = parent_.lock();
    if (!parent) return;
    
    // Parse HTML fragment
    if (!html.empty()) {
        HTMLParser parser;
        auto fragment = parser.parseFragment(html, parent);
        if (fragment && !fragment->getChildren().empty()) {
            // Replace self with the first child of fragment
            auto new_node = fragment->getChildren().front()->cloneNode(true);
            parent->replaceChild(new_node, shared_from_this());
        } else {
            // If parsing failed or empty, just remove self
            remove();
        }
    } else {
        // Empty HTML means remove self
        remove();
    }
}

void DOMNode::insertAdjacentHTML(const std::string& position, const std::string& html) {
    if (html.empty()) return;
    
    // Parse HTML fragment
    HTMLParser parser;
    auto fragment = parser.parseFragment(html, shared_from_this());
    if (!fragment) return;
    
    // Insert based on position
    if (position == "beforebegin") {
        auto parent = parent_.lock();
        if (parent) {
            for (const auto& child : fragment->getChildren()) {
                parent->insertBefore(child->cloneNode(true), shared_from_this());
            }
        }
    } else if (position == "afterbegin") {
        // Insert as first children
        auto children = fragment->getChildren();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            insertBefore((*it)->cloneNode(true), getFirstChild());
        }
    } else if (position == "beforeend") {
        // Insert as last children
        for (const auto& child : fragment->getChildren()) {
            appendChild(child->cloneNode(true));
        }
    } else if (position == "afterend") {
        auto parent = parent_.lock();
        if (parent) {
            auto next = getNextSibling();
            for (const auto& child : fragment->getChildren()) {
                if (next) {
                    parent->insertBefore(child->cloneNode(true), next);
                } else {
                    parent->appendChild(child->cloneNode(true));
                }
            }
        }
    }
}

void DOMNode::insertAdjacentElement(const std::string& position, DOMNodePtr element) {
    if (!element) return;
    
    if (position == "beforebegin") {
        before(element);
    } else if (position == "afterbegin") {
        prepend(element);
    } else if (position == "beforeend") {
        append(element);
    } else if (position == "afterend") {
        after(element);
    }
}

void DOMNode::insertAdjacentText(const std::string& position, const std::string& text) {
    auto text_node = std::make_shared<DOMNode>(NodeType::TEXT);
    text_node->text_content_ = text;
    insertAdjacentElement(position, text_node);
}

void DOMNode::remove() {
    if (auto parent = parent_.lock()) {
        parent->removeChild(shared_from_this());
    }
}

void DOMNode::before(DOMNodePtr node) {
    if (!node) return;
    if (auto parent = parent_.lock()) {
        parent->insertBefore(node, shared_from_this());
    }
}

void DOMNode::after(DOMNodePtr node) {
    if (!node) return;
    if (auto parent = parent_.lock()) {
        auto next = getNextSibling();
        if (next) {
            parent->insertBefore(node, next);
        } else {
            parent->appendChild(node);
        }
    }
}

void DOMNode::replaceWith(DOMNodePtr node) {
    if (!node) return;
    if (auto parent = parent_.lock()) {
        parent->replaceChild(node, shared_from_this());
    }
}

void DOMNode::prepend(DOMNodePtr node) {
    if (!node) return;
    if (children_.empty()) {
        appendChild(node);
    } else {
        insertBefore(node, children_.front());
    }
}

void DOMNode::append(DOMNodePtr node) {
    appendChild(node);
}

namespace {
std::string normalizePropertyKey(const std::string& property) {
    std::string key;
    key.reserve(property.size());
    for (char c : property) {
        key.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return key;
}
}

void DOMNode::setInlineStyleProperty(const std::string& property, const std::string& value) {
    const std::string key = normalizePropertyKey(property);
    if (value.empty()) {
        inline_styles_.erase(key);
    } else {
        inline_styles_[key] = value;
    }
    // Invalidate cached attribute string since CSSOM changed the properties.
    inline_style_attr_.clear();

    if (inline_styles_.empty()) {
        attributes_.erase("style");
    } else {
        std::string style_str;
        bool first = true;
        for (const auto& entry : inline_styles_) {
            if (entry.second.empty()) continue;
            if (!first) {
                style_str.append(" ");
            }
            style_str.append(entry.first);
            style_str.append(":");
            style_str.append(entry.second);
            style_str.append(";");
            first = false;
        }
        if (!style_str.empty()) {
            attributes_["style"] = style_str;
        }
    }

    StyleEngine::applyInlineStyleProperty(key, value, computed_style_);
    markStyleDirty();
    markLayoutDirty();
}


std::string DOMNode::getInlineStyleProperty(const std::string& property) const {
    const std::string key = normalizePropertyKey(property);
    auto it = inline_styles_.find(key);
    if (it == inline_styles_.end()) {
        return "";
    }
    return it->second;
}

std::string DOMNode::getInlineStyleCssText() const {
    // If we have the original attribute string (from HTML parsing), return it directly.
    // This preserves the declaration order and whitespace the author wrote.
    if (!inline_style_attr_.empty()) {
        return inline_style_attr_;
    }
    // CSSOM-modified styles: serialize in map order (not ideal but consistent).
    std::string result;
    for (const auto& entry : inline_styles_) {
        if (entry.second.empty()) continue;
        if (!result.empty()) result += " ";
        result += entry.first + ": " + entry.second + ";";
    }
    return result;
}

void DOMNode::markLayoutDirty() {
    // Always propagate to ancestors.
    // If a descendant is already dirty but its ancestors were cleared (or never marked),
    // we still need the root to become dirty so View can recompute layout/rebuild commands.
    if (!layout_dirty_) {
        layout_dirty_ = true;
    }

    if (auto p = parent_.lock()) {
        p->markLayoutDirty();
    }
}

void DOMNode::clearLayoutDirtyRecursive() {
    layout_dirty_ = false;
    for (auto& child : children_) {
        if (child) {
            child->clearLayoutDirtyRecursive();
        }
    }
}

void DOMNode::markStyleDirty() {
    if (!style_dirty_) {
        style_dirty_ = true;
    }

    // Propagate subtree-dirty flag to ancestors (without marking them as self-dirty)
    if (auto p = parent_.lock()) {
        p->markStyleSubtreeDirty();
    }
}

void DOMNode::markStyleSubtreeDirty() {
    if (style_subtree_dirty_) return;  // already propagated
    style_subtree_dirty_ = true;
    if (auto p = parent_.lock()) {
        p->markStyleSubtreeDirty();
    }
}

void DOMNode::clearStyleDirtyRecursive() {
    style_dirty_ = false;
    style_subtree_dirty_ = false;
    for (auto& child : children_) {
        if (child) {
            child->clearStyleDirtyRecursive();
        }
    }
}


namespace {

static bool isScrollOverflowValue(CSSOverflow v) {
    return v == CSSOverflow::Scroll || v == CSSOverflow::Auto;
}

static float computeMaxScroll(float client, float content) {
    if (client <= 0.0f || content <= client) {
        return 0.0f;
    }
    return content - client;
}

} // namespace

DOMNode::ScrollResult DOMNode::scrollBy(float dx, float dy) {
    ScrollResult result;
    if (!isScrollContainer()) return result;

    const float max_x = computeMaxScroll(client_width_, content_width_);
    const float max_y = computeMaxScroll(client_height_, content_height_);

    const float old_scroll_x = scroll_x_;
    const float old_scroll_y = scroll_y_;

    scroll_x_ = std::clamp(scroll_x_ + dx, 0.0f, max_x);
    scroll_y_ = std::clamp(scroll_y_ + dy, 0.0f, max_y);

    // Check if scroll was consumed (position changed)
    result.consumed_x = (scroll_x_ != old_scroll_x);
    result.consumed_y = (scroll_y_ != old_scroll_y);

    return result;
}

void DOMNode::setScrollX(float x) {
    setScrollLeft(x);
}

void DOMNode::setScrollY(float y) {
    setScrollTop(y);
}

void DOMNode::setScrollTop(float y) {
    // Always store the raw value, even if style/layout isn't ready yet.
    // The scroll offset will only be applied visually when this node is a scroll container.
    // Preserve early scrollTop before layout has established scroll ranges.
    if (!isScrollContainer() || !(has_client_rect_ && has_content_size_)) {
        scroll_y_ = y;
        return;
    }

    scrollTo(scroll_x_, y);
}

void DOMNode::setScrollLeft(float x) {
    if (!isScrollContainer() || !(has_client_rect_ && has_content_size_)) {
        scroll_x_ = x;
        return;
    }

    scrollTo(x, scroll_y_);
}

namespace {

static float smoothstep01(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static bool nearlyEqual(float a, float b, float eps = 0.01f) {
    return std::abs(a - b) <= eps;
}

} // namespace

bool DOMNode::updateSmoothScroll(double current_time_sec) {
    if (!smooth_scroll_active_) {
        return false;
    }

    // Only scroll containers can animate scroll offsets.
    if (!isScrollContainer()) {
        smooth_scroll_active_ = false;
        smooth_scroll_start_time_ = -1.0;
        return false;
    }

    // Wait until we have established scroll ranges.
    if (!(has_client_rect_ && has_content_size_)) {
        return false;
    }

    const float max_x = computeMaxScroll(client_width_, content_width_);
    const float max_y = computeMaxScroll(client_height_, content_height_);

    const float target_x = std::clamp(smooth_scroll_target_x_raw_, 0.0f, max_x);
    const float target_y = std::clamp(smooth_scroll_target_y_raw_, 0.0f, max_y);

    if (smooth_scroll_start_time_ < 0.0) {
        smooth_scroll_start_time_ = current_time_sec;
        smooth_scroll_from_x_ = scroll_x_;
        smooth_scroll_from_y_ = scroll_y_;

        // If already at target (or target collapses to current after clamping), finish immediately.
        if (nearlyEqual(smooth_scroll_from_x_, target_x) && nearlyEqual(smooth_scroll_from_y_, target_y)) {
            scroll_x_ = target_x;
            scroll_y_ = target_y;
            smooth_scroll_active_ = false;
            smooth_scroll_start_time_ = -1.0;
            return false;
        }
    }

    const double duration = std::max(0.0, smooth_scroll_duration_sec_);
    if (duration <= 1e-6) {
        const bool changed = !nearlyEqual(scroll_x_, target_x) || !nearlyEqual(scroll_y_, target_y);
        scroll_x_ = target_x;
        scroll_y_ = target_y;
        smooth_scroll_active_ = false;
        smooth_scroll_start_time_ = -1.0;
        return changed;
    }

    const double elapsed = current_time_sec - smooth_scroll_start_time_;
    const float t = static_cast<float>(elapsed / duration);
    const float eased = smoothstep01(t);

    const float new_x = smooth_scroll_from_x_ + (target_x - smooth_scroll_from_x_) * eased;
    const float new_y = smooth_scroll_from_y_ + (target_y - smooth_scroll_from_y_) * eased;

    const bool changed = !nearlyEqual(scroll_x_, new_x) || !nearlyEqual(scroll_y_, new_y);
    scroll_x_ = new_x;
    scroll_y_ = new_y;

    if (t >= 1.0f) {
        scroll_x_ = target_x;
        scroll_y_ = target_y;
        smooth_scroll_active_ = false;
        smooth_scroll_start_time_ = -1.0;
    }

    return changed;
}

void DOMNode::scrollTo(float x, float y) {
    if (!isScrollContainer()) return;

    const float max_x = computeMaxScroll(client_width_, content_width_);
    const float max_y = computeMaxScroll(client_height_, content_height_);

    const float target_x = std::clamp(x, 0.0f, max_x);
    const float target_y = std::clamp(y, 0.0f, max_y);

    if (computed_style_.scroll_behavior == CSSScrollBehavior::Smooth) {
        // Defer animation start until EngineView::tick (timebase + scroll metrics).
        smooth_scroll_active_ = true;
        smooth_scroll_start_time_ = -1.0;
        smooth_scroll_from_x_ = scroll_x_;
        smooth_scroll_from_y_ = scroll_y_;
        smooth_scroll_target_x_raw_ = target_x;
        smooth_scroll_target_y_raw_ = target_y;
        return;
    }

    // Instant scroll
    smooth_scroll_active_ = false;
    smooth_scroll_start_time_ = -1.0;
    scroll_x_ = target_x;
    scroll_y_ = target_y;
}


void DOMNode::scrollIntoView(bool alignToTop) {
    // Find the nearest scroll container ancestor
    DOMNodePtr scroll_container = nullptr;
    auto current = getParent();
    while (current) {
        if (current->isScrollContainer()) {
            scroll_container = current;
            break;
        }
        current = current->getParent();
    }
    
    if (!scroll_container) {
        return; // No scroll container found
    }
    
    // Get this node's position relative to the scroll container
    float node_top = offset_top_;
    float node_left = offset_left_;
    float node_height = offset_height_;
    float node_width = offset_width_;
    
    float container_height = scroll_container->getClientHeight();
    float container_width = scroll_container->getClientWidth();
    
    float current_scroll_x = scroll_container->getScrollLeft();
    float current_scroll_y = scroll_container->getScrollTop();
    
    float new_scroll_x = current_scroll_x;
    float new_scroll_y = current_scroll_y;
    
    // Calculate target scroll position relative to the scroll container's content
    float relative_top = node_top - scroll_container->getOffsetTop();
    float relative_left = node_left - scroll_container->getOffsetLeft();
    
    if (alignToTop) {
        // Align element's top with container's top
        new_scroll_y = relative_top;
    } else {
        // Align element's bottom with container's bottom
        new_scroll_y = relative_top + node_height - container_height;
    }
    
    // Ensure horizontal visibility
    if (relative_left < current_scroll_x) {
        new_scroll_x = relative_left;
    } else if (relative_left + node_width > current_scroll_x + container_width) {
        new_scroll_x = relative_left + node_width - container_width;
    }
    
    // Apply scroll
    scroll_container->scrollTo(new_scroll_x, new_scroll_y);
}

bool DOMNode::isScrollContainer() const {
    // CSS: overflow-x / overflow-y can differ; treat any axis with scroll/auto as scrollable.
    if (isScrollOverflowValue(computed_style_.overflow) ||
        isScrollOverflowValue(computed_style_.overflow_x) ||
        isScrollOverflowValue(computed_style_.overflow_y)) {
        return true;
    }

    // Viewport scrolling semantics: in browsers, document root can scroll even when
    // overflow is "visible" (default), unless explicitly disabled.
    if (tag_name_ == "html" || tag_name_ == "body") {
        const bool hidden_or_clip =
            computed_style_.overflow == CSSOverflow::Hidden ||
            computed_style_.overflow == CSSOverflow::Clip ||
            computed_style_.overflow_x == CSSOverflow::Hidden ||
            computed_style_.overflow_x == CSSOverflow::Clip ||
            computed_style_.overflow_y == CSSOverflow::Hidden ||
            computed_style_.overflow_y == CSSOverflow::Clip;
        return !hidden_or_clip;
    }

    return false;
}


void DOMNode::setClientRect(float top, float left, float width, float height) {
    client_top_ = top;
    client_left_ = left;
    client_width_ = width;
    client_height_ = height;
    has_client_rect_ = true;

    // If content size was updated before client size (or vice versa), ensure scroll offsets remain valid.
    // Important: do NOT clamp until both client rect and content size have been initialized at least once.
    // Otherwise, early `scrollTop = ...` (before layout) will get clamped to 0 and silently lost.
    if (isScrollContainer() && has_client_rect_ && has_content_size_) {
        const float max_x = computeMaxScroll(client_width_, content_width_);
        const float max_y = computeMaxScroll(client_height_, content_height_);
        scroll_x_ = std::clamp(scroll_x_, 0.0f, max_x);
        scroll_y_ = std::clamp(scroll_y_, 0.0f, max_y);
    }
}

void DOMNode::setContentSize(float w, float h) {
    content_width_ = w;
    content_height_ = h;
    has_content_size_ = true;

    if (isScrollContainer() && has_client_rect_ && has_content_size_) {
        const float max_x = computeMaxScroll(client_width_, content_width_);
        const float max_y = computeMaxScroll(client_height_, content_height_);
        scroll_x_ = std::clamp(scroll_x_, 0.0f, max_x);
        scroll_y_ = std::clamp(scroll_y_, 0.0f, max_y);
    }
}

void DOMNode::setOffsetRect(float top, float left, float width, float height) {
    offset_top_ = top;
    offset_left_ = left;
    offset_width_ = width;
    offset_height_ = height;
}

DOMNodePtr DOMNode::getOffsetParent() const {
    auto current = parent_.lock();
    while (current) {
        if (current->computed_style_.position != CSSPosition::Static) {
            return current;
        }
        current = current->parent_.lock();
    }
    return nullptr;
}

DOMNode::DOMRect DOMNode::getBoundingClientRect() const {
    DOMRect rect;
    rect.x = offset_left_;
    rect.y = offset_top_;
    rect.width = offset_width_;
    rect.height = offset_height_;
    rect.top = rect.y;
    rect.left = rect.x;
    rect.right = rect.x + rect.width;
    rect.bottom = rect.y + rect.height;
    return rect;
}

std::vector<DOMNode::DOMRect> DOMNode::getClientRects() const {
    // Simplified: return single rect
    return {getBoundingClientRect()};
}

DOMNodePtr DOMNode::getElementById(const std::string& id) {
    if (hasAttribute("id") && getAttribute("id") == id) {
        return std::static_pointer_cast<DOMNode>(shared_from_this());
    }

    for (const auto& child : children_) {
        auto result = child->getElementById(id);
        if (result) return result;
    }

    return nullptr;
}

std::vector<DOMNodePtr> DOMNode::getElementsByTagName(const std::string& tag) {
    std::vector<DOMNodePtr> result;
    std::string lower_tag = tag;
    std::transform(lower_tag.begin(), lower_tag.end(), lower_tag.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (tag == "*" || tag_name_ == lower_tag) {
        result.push_back(std::static_pointer_cast<DOMNode>(shared_from_this()));
    }

    for (const auto& child : children_) {
        auto child_results = child->getElementsByTagName(tag);
        result.insert(result.end(), child_results.begin(), child_results.end());
    }

    return result;
}

std::vector<DOMNodePtr> DOMNode::getElementsByClassName(const std::string& cls) {
    std::vector<DOMNodePtr> result;

    if (hasAttribute("class")) {
        std::string classes = getAttribute("class");
        std::istringstream iss(classes);
        std::string class_name;
        while (iss >> class_name) {
            if (class_name == cls) {
                result.push_back(std::static_pointer_cast<DOMNode>(shared_from_this()));
                break;
            }
        }
    }

    for (const auto& child : children_) {
        auto child_results = child->getElementsByClassName(cls);
        result.insert(result.end(), child_results.begin(), child_results.end());
    }

    return result;
}

std::vector<DOMNodePtr> DOMNode::getElementsByName(const std::string& name) {
    std::vector<DOMNodePtr> result;

    if (getAttribute("name") == name) {
        result.push_back(std::static_pointer_cast<DOMNode>(shared_from_this()));
    }

    for (const auto& child : children_) {
        auto child_results = child->getElementsByName(name);
        result.insert(result.end(), child_results.begin(), child_results.end());
    }

    return result;
}

DOMNodePtr DOMNode::querySelector(const std::string& selector) {
    SelectorMatcher matcher;
    return matcher.querySelector(selector, shared_from_this());
}

std::vector<DOMNodePtr> DOMNode::querySelectorAll(const std::string& selector) {
    SelectorMatcher matcher;
    return matcher.querySelectorAll(selector, shared_from_this());
}

bool DOMNode::matches(const std::string& selector) const {
    SelectorMatcher matcher;
    return matcher.elementMatches(selector, std::const_pointer_cast<DOMNode>(shared_from_this()));
}

DOMNodePtr DOMNode::closest(const std::string& selector) {
    SelectorMatcher matcher;
    return matcher.closest(selector, shared_from_this());
}

void DOMNode::focus() {
    if (!s_focus_manager_) return;
    if (isInert()) return;

    // Only focus if element is focusable
    if (FocusManager::isFocusable(shared_from_this())) {
        s_focus_manager_->setFocus(shared_from_this());
    }
}

void DOMNode::blur() {
    if (!s_focus_manager_) return;

    // Only blur if this element has focus
    if (s_focus_manager_->hasFocus(shared_from_this())) {
        s_focus_manager_->blur();
    }
}

void DOMNode::setPointerCapture(const DOMNodePtr& element) {
    if (!element) return;
    s_pointer_capture_ = element;
}

void DOMNode::releasePointerCapture(const DOMNodePtr& element) {
    auto cur = s_pointer_capture_.lock();
    if (!cur) return;
    if (!element || cur.get() == element.get()) {
        s_pointer_capture_.reset();
    }
}

DOMNodePtr DOMNode::getPointerCapture() {
    return s_pointer_capture_.lock();
}

void DOMNode::click() {
    // Dispatch click event
    if (s_event_dispatcher_) {
        Event event;
        event.type = EventType::CLICK;
        event.type_name = "click";
        event.target = shared_from_this();
        event.current_target = shared_from_this();
        event.is_trusted = false;
        s_event_dispatcher_->dispatch(event);
    }

    // Also try to focus if focusable
    if (FocusManager::isFocusable(shared_from_this())) {
        focus();
    }
}

bool DOMNode::isInert() const {
    // Check explicit inert attribute in ancestor chain
    auto current = const_cast<DOMNode*>(this)->shared_from_this();
    while (current) {
        if (current->hasAttribute("inert")) {
            return true;
        }
        current = current->getParent();
    }

    // If a modal dialog is active, everything outside it is implicitly inert
    if (s_modal_dialog_root_) {
        auto self = const_cast<DOMNode*>(this)->shared_from_this();
        if (self != s_modal_dialog_root_ && !s_modal_dialog_root_->contains(self)) {
            return true;
        }
    }

    return false;
}

bool DOMNode::isContentEditable() const {
    auto current = const_cast<DOMNode*>(this)->shared_from_this();
    while (current) {
        if (current->hasAttribute("contenteditable")) {
            std::string val = current->getAttribute("contenteditable");
            if (val == "false") return false;
            return true;
        }
        current = current->getParent();
    }
    return false;
}

void DOMNode::print(int depth) const {
    for (int i = 0; i < depth; ++i) std::cout << "  ";

    switch (type_) {
        case NodeType::ELEMENT:
            std::cout << "<" << tag_name_;
            for (const auto& [key, value] : attributes_) {
                std::cout << " " << key << "=\"" << value << "\"";
            }
            std::cout << ">" << std::endl;
            break;
        case NodeType::TEXT:
            if (!text_content_.empty()) {
                std::cout << "\"" << text_content_ << "\"" << std::endl;
            }
            break;
        case NodeType::COMMENT:
            std::cout << "<!-- comment -->" << std::endl;
            break;
        case NodeType::DOCUMENT:
            std::cout << "#document" << std::endl;
            break;
        default:
            break;
    }

    for (const auto& child : children_) {
        child->print(depth + 1);
    }
}

} // namespace dong::dom
