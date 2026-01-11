#include "dom_node.hpp"
#include "../css/style_engine.hpp"
#include "../css/selector_matcher.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>
#include <limits>


namespace dong::dom {

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
    if (it != children_.end()) {
        children_.insert(it, newChild);
        newChild->parent_ = std::static_pointer_cast<DOMNode>(shared_from_this());
        markLayoutDirty();
    }
}

void DOMNode::removeChild(DOMNodePtr child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        children_.erase(it);
        child->parent_.reset();
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
    attributes_[key] = value;

    // Internal attributes used by the renderer should not affect layout.
    // (e.g. Painter's transient markers)
    if (key.size() >= 2 && key[0] == '_' && key[1] == '_') {
        return;
    }

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
    attributes_.erase(key);
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
    // TODO: Parse HTML and add children
    // For now, just add as text
    if (!html.empty()) {
        auto text_node = std::make_shared<DOMNode>(NodeType::TEXT);
        text_node->text_content_ = html;
        appendChild(text_node);
    }
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
    // TODO: Parse HTML and replace self
    auto parent = parent_.lock();
    if (parent) {
        // For now, just replace with text
        auto text_node = std::make_shared<DOMNode>(NodeType::TEXT);
        text_node->text_content_ = html;
        parent->replaceChild(text_node, shared_from_this());
    }
}

void DOMNode::insertAdjacentHTML(const std::string& position, const std::string& html) {
    // TODO: Parse HTML
    auto text_node = std::make_shared<DOMNode>(NodeType::TEXT);
    text_node->text_content_ = html;
    insertAdjacentElement(position, text_node);
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

namespace {

static std::string toLowerCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

static bool isScrollOverflowValue(const std::string& v) {
    const std::string lowered = toLowerCopy(v);
    return lowered == "scroll" || lowered == "auto";
}

static float computeMaxScroll(float client, float content) {
    if (client <= 0.0f || content <= client) {
        return 0.0f;
    }
    return content - client;
}

} // namespace

void DOMNode::scrollBy(float dx, float dy) {
    if (!isScrollContainer()) return;

    const float max_x = computeMaxScroll(client_width_, content_width_);
    const float max_y = computeMaxScroll(client_height_, content_height_);

    scroll_x_ = std::clamp(scroll_x_ + dx, 0.0f, max_x);
    scroll_y_ = std::clamp(scroll_y_ + dy, 0.0f, max_y);
}

void DOMNode::scrollTo(float x, float y) {
    if (!isScrollContainer()) return;

    const float max_x = computeMaxScroll(client_width_, content_width_);
    const float max_y = computeMaxScroll(client_height_, content_height_);

    scroll_x_ = std::clamp(x, 0.0f, max_x);
    scroll_y_ = std::clamp(y, 0.0f, max_y);
}


void DOMNode::scrollIntoView(bool alignToTop) {
    // TODO: Implement scroll into view
    (void)alignToTop;
}

bool DOMNode::isScrollContainer() const {
    // CSS: overflow-x / overflow-y can differ; treat any axis with scroll/auto as scrollable.
    return isScrollOverflowValue(computed_style_.overflow) ||
           isScrollOverflowValue(computed_style_.overflow_x) ||
           isScrollOverflowValue(computed_style_.overflow_y);
}


void DOMNode::setClientRect(float top, float left, float width, float height) {
    client_top_ = top;
    client_left_ = left;
    client_width_ = width;
    client_height_ = height;

    // If content size was updated before client size (or vice versa), ensure scroll offsets remain valid.
    if (isScrollContainer()) {
        const float max_x = computeMaxScroll(client_width_, content_width_);
        const float max_y = computeMaxScroll(client_height_, content_height_);
        scroll_x_ = std::clamp(scroll_x_, 0.0f, max_x);
        scroll_y_ = std::clamp(scroll_y_, 0.0f, max_y);
    }
}

void DOMNode::setContentSize(float w, float h) {
    content_width_ = w;
    content_height_ = h;

    if (isScrollContainer()) {
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
        const auto& pos = current->computed_style_.position;
        if (pos != "static") {
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
    // TODO: Implement focus management
}

void DOMNode::blur() {
    // TODO: Implement focus management
}

void DOMNode::click() {
    // TODO: Dispatch click event
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
