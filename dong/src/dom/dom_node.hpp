#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace dong::dom {

// Forward declarations
class DOMNode;
using DOMNodePtr = std::shared_ptr<DOMNode>;

// CSS style value - can be pixel, percent, or auto
struct CSSValue {
    enum class Unit {
        PIXEL,
        PERCENT,
        AUTO,
        INHERIT
    };

    float value = 0.0f;
    Unit unit = Unit::AUTO;

    CSSValue() = default;
    CSSValue(float v, Unit u) : value(v), unit(u) {}

    bool isAuto() const { return unit == Unit::AUTO; }
    bool isPercent() const { return unit == Unit::PERCENT; }
    bool isPixel() const { return unit == Unit::PIXEL; }
};

// Computed style properties
struct ComputedStyle {
    // Box Model
    CSSValue width;
    CSSValue height;
    CSSValue margin_top;
    CSSValue margin_right;
    CSSValue margin_bottom;
    CSSValue margin_left;
    CSSValue padding_top;
    CSSValue padding_right;
    CSSValue padding_bottom;
    CSSValue padding_left;

    // Layout
    std::string display = "block";  // block, inline, flex, none
    std::string position = "static"; // static, relative, absolute, fixed

    // Visual
    std::string background_color = "#ffffff";
    std::string color = "#000000";
    float border_radius = 0.0f;
    std::string border_color = "#000000";
    float border_width = 0.0f;

    // Text
    std::string font_family = "Arial";
    float font_size = 16.0f;
    std::string font_weight = "normal"; // normal, bold
    std::string text_align = "left";    // left, center, right

    // Flexbox properties
    std::string flex_direction = "row";     // row, column
    std::string justify_content = "flex-start";
    std::string align_items = "stretch";
    float flex = 0.0f;
    float flex_grow = 0.0f;
    float flex_shrink = 1.0f;
    CSSValue flex_basis;
};

// DOM Node representation
class DOMNode : public std::enable_shared_from_this<DOMNode> {
public:
    enum class NodeType {
        ELEMENT,
        TEXT,
        COMMENT,
        DOCUMENT
    };

    DOMNode(NodeType type, const std::string& name = "");
    ~DOMNode() = default;

    // Tree structure
    void appendChild(DOMNodePtr child);
    void removeChild(DOMNodePtr child);
    DOMNodePtr getParent() const { return parent.lock(); }
    const std::vector<DOMNodePtr>& getChildren() const { return children; }

    // Node properties
    NodeType getType() const { return type; }
    const std::string& getTagName() const { return tag_name; }
    const std::string& getTextContent() const { return text_content; }
    void setTextContent(const std::string& text) { text_content = text; }

    // Attributes
    void setAttribute(const std::string& key, const std::string& value);
    std::string getAttribute(const std::string& key) const;
    bool hasAttribute(const std::string& key) const;
    const std::unordered_map<std::string, std::string>& getAttributes() const {
        return attributes;
    }

    // Style
    ComputedStyle& getComputedStyle() { return computed_style; }
    const ComputedStyle& getComputedStyle() const { return computed_style; }

    // Traversal helpers
    DOMNodePtr getElementById(const std::string& id);
    std::vector<DOMNodePtr> getElementsByTagName(const std::string& tag);
    std::vector<DOMNodePtr> getElementsByClassName(const std::string& cls);

    // Debug
    void print(int depth = 0) const;

private:
    NodeType type;
    std::string tag_name;
    std::string text_content;
    std::unordered_map<std::string, std::string> attributes;
    ComputedStyle computed_style;

    std::weak_ptr<DOMNode> parent;
    std::vector<DOMNodePtr> children;
};

} // namespace dong::dom
