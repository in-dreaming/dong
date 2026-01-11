#pragma once

#include "../css/computed_style.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace dong::dom {

// Forward declarations
class DOMNode;
using DOMNodePtr = std::shared_ptr<DOMNode>;
using DOMNodeWeakPtr = std::weak_ptr<DOMNode>;

// ClassList helper class for classList API
class ClassList {
public:
    explicit ClassList(DOMNode* node);
    
    void add(const std::string& className);
    void remove(const std::string& className);
    bool toggle(const std::string& className);
    bool toggle(const std::string& className, bool force);
    bool contains(const std::string& className) const;
    void replace(const std::string& oldClass, const std::string& newClass);
    std::string item(size_t index) const;
    size_t length() const;
    std::string value() const;
    void setValue(const std::string& value);

private:
    DOMNode* node_;
    void updateAttribute();
    std::vector<std::string> getClasses() const;
};

// DOM Node representation
class DOMNode : public std::enable_shared_from_this<DOMNode> {
public:
    enum class NodeType {
        ELEMENT = 1,
        ATTRIBUTE = 2,
        TEXT = 3,
        CDATA_SECTION = 4,
        PROCESSING_INSTRUCTION = 7,
        COMMENT = 8,
        DOCUMENT = 9,
        DOCUMENT_TYPE = 10,
        DOCUMENT_FRAGMENT = 11
    };

    DOMNode(NodeType type, const std::string& name = "");
    virtual ~DOMNode() = default;

    // Node interface
    NodeType getNodeType() const { return type_; }
    NodeType getType() const { return type_; }  // Alias for compatibility
    std::string getNodeName() const;
    std::string getNodeValue() const;
    void setNodeValue(const std::string& value);
    
    // Tree structure
    void appendChild(DOMNodePtr child);
    void insertBefore(DOMNodePtr newChild, DOMNodePtr refChild);
    void removeChild(DOMNodePtr child);
    void replaceChild(DOMNodePtr newChild, DOMNodePtr oldChild);
    DOMNodePtr cloneNode(bool deep = false) const;
    bool contains(DOMNodePtr node) const;
    bool hasChildNodes() const { return !children_.empty(); }
    void normalize();
    bool isEqualNode(DOMNodePtr other) const;
    bool isSameNode(DOMNodePtr other) const;
    int compareDocumentPosition(DOMNodePtr other) const;
    
    DOMNodePtr getParent() const { return parent_.lock(); }
    DOMNodePtr getParentNode() const { return parent_.lock(); }
    DOMNodePtr getParentElement() const;
    const std::vector<DOMNodePtr>& getChildren() const { return children_; }
    const std::vector<DOMNodePtr>& getChildNodes() const { return children_; }
    DOMNodePtr getFirstChild() const;
    DOMNodePtr getLastChild() const;
    DOMNodePtr getPreviousSibling() const;
    DOMNodePtr getNextSibling() const;
    DOMNodePtr getFirstElementChild() const;
    DOMNodePtr getLastElementChild() const;
    DOMNodePtr getPreviousElementSibling() const;
    DOMNodePtr getNextElementSibling() const;
    size_t getChildElementCount() const;
    DOMNodePtr getOwnerDocument() const;

    // Element interface
    const std::string& getTagName() const { return tag_name_; }
    std::string getId() const { return getAttribute("id"); }
    void setId(const std::string& id) { setAttribute("id", id); }
    std::string getClassName() const { return getAttribute("class"); }
    void setClassName(const std::string& className) { setAttribute("class", className); }
    ClassList& getClassList();
    
    // Attributes
    void setAttribute(const std::string& key, const std::string& value);
    std::string getAttribute(const std::string& key) const;
    bool hasAttribute(const std::string& key) const;
    void removeAttribute(const std::string& key);
    bool toggleAttribute(const std::string& name);
    bool toggleAttribute(const std::string& name, bool force);
    std::vector<std::string> getAttributeNames() const;
    const std::unordered_map<std::string, std::string>& getAttributes() const { return attributes_; }

    // Text content
    std::string getTextContent() const;
    const std::string& getRawTextContent() const { return text_content_; }
    void setTextContent(const std::string& text);
    
    // innerHTML/outerHTML
    std::string getInnerHTML() const;
    void setInnerHTML(const std::string& html);
    std::string getOuterHTML() const;
    void setOuterHTML(const std::string& html);
    
    // insertAdjacentHTML/Element/Text
    void insertAdjacentHTML(const std::string& position, const std::string& html);
    void insertAdjacentElement(const std::string& position, DOMNodePtr element);
    void insertAdjacentText(const std::string& position, const std::string& text);
    
    // ChildNode interface
    void remove();
    void before(DOMNodePtr node);
    void after(DOMNodePtr node);
    void replaceWith(DOMNodePtr node);
    
    // ParentNode interface
    void prepend(DOMNodePtr node);
    void append(DOMNodePtr node);

    // Style
    void setInlineStyleProperty(const std::string& property, const std::string& value);
    std::string getInlineStyleProperty(const std::string& property) const;
    ComputedStyle& getComputedStyle() { return computed_style_; }
    const ComputedStyle& getComputedStyle() const { return computed_style_; }
    
    // Pseudo-elements (::before/::after)
    DOMNodePtr getPseudoBefore() const { return pseudo_before_; }
    DOMNodePtr getPseudoAfter() const { return pseudo_after_; }
    void setPseudoBefore(DOMNodePtr node) { pseudo_before_ = node; }
    void setPseudoAfter(DOMNodePtr node) { pseudo_after_ = node; }
    bool hasPseudoElements() const { return pseudo_before_ || pseudo_after_; }
    bool isPseudoElement() const { return computed_style_.is_pseudo_element; }

    // Layout
    void markLayoutDirty();
    void clearLayoutDirtyRecursive();
    bool isLayoutDirty() const { return layout_dirty_; }

    // Scroll
    float getScrollX() const { return scroll_x_; }
    float getScrollY() const { return scroll_y_; }
    float getScrollTop() const { return scroll_y_; }
    float getScrollLeft() const { return scroll_x_; }
    void setScrollX(float x) { scroll_x_ = x; }
    void setScrollY(float y) { scroll_y_ = y; }
    void setScrollTop(float y) { scroll_y_ = y; }
    void setScrollLeft(float x) { scroll_x_ = x; }
    void scrollBy(float dx, float dy);
    void scrollTo(float x, float y);
    void scrollIntoView(bool alignToTop = true);
    bool isScrollContainer() const;
    
    // Content size
    float getContentWidth() const { return content_width_; }
    float getContentHeight() const { return content_height_; }
    float getScrollWidth() const { return content_width_; }
    float getScrollHeight() const { return content_height_; }
    void setContentSize(float w, float h);
    
    // Client dimensions (set by layout)
    float getClientTop() const { return client_top_; }
    float getClientLeft() const { return client_left_; }
    float getClientWidth() const { return client_width_; }
    float getClientHeight() const { return client_height_; }
    void setClientRect(float top, float left, float width, float height);
    
    // Offset dimensions (set by layout)
    float getOffsetTop() const { return offset_top_; }
    float getOffsetLeft() const { return offset_left_; }
    float getOffsetWidth() const { return offset_width_; }
    float getOffsetHeight() const { return offset_height_; }
    DOMNodePtr getOffsetParent() const;
    void setOffsetRect(float top, float left, float width, float height);
    
    // Bounding rect
    struct DOMRect {
        float x = 0, y = 0, width = 0, height = 0;
        float top = 0, right = 0, bottom = 0, left = 0;
    };
    DOMRect getBoundingClientRect() const;
    std::vector<DOMRect> getClientRects() const;

    // Traversal helpers
    DOMNodePtr getElementById(const std::string& id);
    std::vector<DOMNodePtr> getElementsByTagName(const std::string& tag);
    std::vector<DOMNodePtr> getElementsByClassName(const std::string& cls);
    std::vector<DOMNodePtr> getElementsByName(const std::string& name);
    DOMNodePtr querySelector(const std::string& selector);
    std::vector<DOMNodePtr> querySelectorAll(const std::string& selector);
    bool matches(const std::string& selector) const;
    DOMNodePtr closest(const std::string& selector);

    // Focus
    void focus();
    void blur();
    void click();

    // Debug
    void print(int depth = 0) const;

protected:
    NodeType type_;
    std::string tag_name_;
    std::string text_content_;
    std::unordered_map<std::string, std::string> attributes_;
    std::unordered_map<std::string, std::string> inline_styles_;
    ComputedStyle computed_style_;
    bool layout_dirty_ = true;
    
    // Scroll state
    float scroll_x_ = 0.0f;
    float scroll_y_ = 0.0f;
    float content_width_ = 0.0f;
    float content_height_ = 0.0f;
    
    // Client rect
    float client_top_ = 0.0f;
    float client_left_ = 0.0f;
    float client_width_ = 0.0f;
    float client_height_ = 0.0f;
    
    // Offset rect
    float offset_top_ = 0.0f;
    float offset_left_ = 0.0f;
    float offset_width_ = 0.0f;
    float offset_height_ = 0.0f;

    DOMNodeWeakPtr parent_;
    std::vector<DOMNodePtr> children_;
    
    // Pseudo-elements
    DOMNodePtr pseudo_before_;
    DOMNodePtr pseudo_after_;
    
    std::unique_ptr<ClassList> class_list_;
};

} // namespace dong::dom
