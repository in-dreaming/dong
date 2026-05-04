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

    // lang attribute inheritance - get effective lang (inherited from ancestors)
    std::string getEffectiveLang() const;

    // dir attribute - get effective direction (ltr/rtl)
    // dir="auto" detects direction from first strong directional character in text content
    std::string getEffectiveDirection() const;

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
    const std::unordered_map<std::string, std::string>& getInlineStyles() const { return inline_styles_; }
    // Returns a serialized cssText with properties in declaration order.
    std::string getInlineStyleCssText() const;
    // Called by the HTML parser to set the raw style attribute string (preserves order).
    void setInlineStyleAttrString(const std::string& s) { inline_style_attr_ = s; }
    ComputedStyle& getComputedStyle() { return computed_style_; }
    const ComputedStyle& getComputedStyle() const { return computed_style_; }
    
    // Pseudo-elements (::before/::after/::marker/::placeholder/::selection/::backdrop)
    DOMNodePtr getPseudoBefore() const { return pseudo_before_; }
    DOMNodePtr getPseudoAfter() const { return pseudo_after_; }
    DOMNodePtr getPseudoMarker() const { return pseudo_marker_; }
    DOMNodePtr getPseudoPlaceholder() const { return pseudo_placeholder_; }
    DOMNodePtr getPseudoSelection() const { return pseudo_selection_; }
    DOMNodePtr getPseudoBackdrop() const { return pseudo_backdrop_; }
    void setPseudoBefore(DOMNodePtr node) { pseudo_before_ = node; }
    void setPseudoAfter(DOMNodePtr node) { pseudo_after_ = node; }
    void setPseudoMarker(DOMNodePtr node) { pseudo_marker_ = node; }
    void setPseudoPlaceholder(DOMNodePtr node) { pseudo_placeholder_ = node; }
    void setPseudoSelection(DOMNodePtr node) { pseudo_selection_ = node; }
    void setPseudoBackdrop(DOMNodePtr node) { pseudo_backdrop_ = node; }
    bool hasPseudoElements() const { return pseudo_before_ || pseudo_after_ || pseudo_marker_ || pseudo_placeholder_ || pseudo_selection_ || pseudo_backdrop_; }
    bool isPseudoElement() const { return computed_style_.is_pseudo_element; }

    // Layout
    void markLayoutDirty();
    void clearLayoutDirtyRecursive();
    bool isLayoutDirty() const { return layout_dirty_; }

    // Style dirty (used to skip full style recompute when only layout/text changed)
    void markStyleDirty();
    void markStyleSubtreeDirty();
    void clearStyleDirtyRecursive();
    bool isStyleDirty() const { return style_dirty_; }
    bool isStyleSubtreeDirty() const { return style_subtree_dirty_; }

    // Scroll

    float getScrollX() const { return scroll_x_; }
    float getScrollY() const { return scroll_y_; }
    float getScrollTop() const { return scroll_y_; }
    float getScrollLeft() const { return scroll_x_; }

    // Note: must clamp to valid scroll range when sizes are known.
    // When layout hasn't produced client/content sizes yet, preserve the raw value and let
    // setClientRect()/setContentSize() clamp later (so early scrollTop isn't lost).
    void setScrollX(float x);
    void setScrollY(float y);
    void setScrollTop(float y);
    void setScrollLeft(float x);

    // Scroll result structure
    struct ScrollResult {
        bool consumed_x = false;  // Whether horizontal scroll was consumed
        bool consumed_y = false;  // Whether vertical scroll was consumed
    };

    ScrollResult scrollBy(float dx, float dy);
    void scrollTo(float x, float y);

    // Smooth scroll animation (driven by EngineView::tick)
    bool hasActiveSmoothScroll() const { return smooth_scroll_active_; }
    bool updateSmoothScroll(double current_time_sec);

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

    // Inert: returns true if this element or any ancestor has the "inert" attribute,
    // or if a modal dialog is active and this element is outside it.
    bool isInert() const;

    // ContentEditable: returns true if this element or any ancestor has contenteditable="true"
    bool isContentEditable() const;

    // Modal dialog root tracking (set by EngineView when modal dialog is active)
    static void setModalDialogRoot(DOMNodePtr root) { s_modal_dialog_root_ = root; }
    static DOMNodePtr getModalDialogRoot() { return s_modal_dialog_root_; }
    
    // Static accessors for focus manager and event dispatcher (set by View)
    static void setFocusManager(class FocusManager* fm) { s_focus_manager_ = fm; }
    static void setEventDispatcher(class EventDispatcher* ed) { s_event_dispatcher_ = ed; }

    // Set current URL fragment for :target pseudo-class matching
    static void setCurrentFragment(const std::string& fragment) { s_current_fragment_ = fragment; }

    // Get current URL fragment for :target pseudo-class matching
    static const std::string& getCurrentFragment() { return s_current_fragment_; }

    // Pointer capture (minimal, mouse-only for now)
    static void setPointerCapture(const DOMNodePtr& element);
    static void releasePointerCapture(const DOMNodePtr& element);
    static DOMNodePtr getPointerCapture();


    // Runtime interaction states for pseudo-classes (:hover/:active/:focus/:focus-visible)
    bool isHovered() const { return hovered_; }
    bool isActive() const { return active_; }
    bool isFocused() const { return focused_; }
    bool isFocusVisible() const { return focus_visible_; }

    void setHovered(bool v) {
        if (hovered_ == v) return;
        hovered_ = v;
        markStyleDirty();
    }
    void setActive(bool v) {
        if (active_ == v) return;
        active_ = v;
        markStyleDirty();
    }
    void setFocused(bool v) {
        if (focused_ == v) return;
        focused_ = v;
        // Note: focus-visible is set separately by FocusManager
        markStyleDirty();
    }
    void setFocusVisible(bool v) {
        if (focus_visible_ == v) return;
        focus_visible_ = v;
        markStyleDirty();
    }


    // Debug
    void print(int depth = 0) const;

protected:
    NodeType type_;
    std::string tag_name_;
    std::string text_content_;
    std::unordered_map<std::string, std::string> attributes_;
    std::unordered_map<std::string, std::string> inline_styles_;
    std::string inline_style_attr_; // Raw style attribute string (preserves declaration order)
    ComputedStyle computed_style_;
    bool layout_dirty_ = true;
    bool style_dirty_ = true;
    bool style_subtree_dirty_ = false;


    // Interaction states (used by selector matcher for :hover/:active/:focus/:focus-visible)
    bool hovered_ = false;
    bool active_ = false;
    bool focused_ = false;
    bool focus_visible_ = false;

    // Scroll state
    float scroll_x_ = 0.0f;
    float scroll_y_ = 0.0f;
    float content_width_ = 0.0f;
    float content_height_ = 0.0f;

    // Smooth scrolling state (for scroll-behavior: smooth)
    bool smooth_scroll_active_ = false;
    double smooth_scroll_start_time_ = -1.0;   // EngineView::tick timebase; -1 means "not started yet"
    double smooth_scroll_duration_sec_ = 0.35; // Default duration; tuned to be close to browser feel.
    float smooth_scroll_from_x_ = 0.0f;
    float smooth_scroll_from_y_ = 0.0f;
    float smooth_scroll_target_x_raw_ = 0.0f;  // Raw targets (may be clamped once metrics are ready)
    float smooth_scroll_target_y_raw_ = 0.0f;

    // Scroll geometry bookkeeping: avoid clamping scroll offsets until both
    // client rect + content size have been initialized at least once.
    bool has_client_rect_ = false;
    bool has_content_size_ = false;

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
    DOMNodePtr pseudo_marker_;
    DOMNodePtr pseudo_placeholder_;
    DOMNodePtr pseudo_selection_;
    DOMNodePtr pseudo_backdrop_;

    std::unique_ptr<ClassList> class_list_;
    
    // Static pointers to FocusManager and EventDispatcher (set by View)
    static class FocusManager* s_focus_manager_;
    static class EventDispatcher* s_event_dispatcher_;

    // Current URL fragment for :target pseudo-class matching
    static std::string s_current_fragment_;

    // Pointer capture (mouse-only)
    static DOMNodeWeakPtr s_pointer_capture_;

    // Modal dialog root (for implicit inert)
    static DOMNodePtr s_modal_dialog_root_;
};

} // namespace dong::dom
