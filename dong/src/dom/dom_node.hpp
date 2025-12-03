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
    // Default to PIXEL (0px) for margin/padding properties.
    // This matches CSS behavior where unset margins default to 0, not auto.
    // Properties that should default to auto (like width/height) must be
    // explicitly initialized with Unit::AUTO.
    Unit unit = Unit::PIXEL;

    CSSValue() = default;
    CSSValue(float v, Unit u) : value(v), unit(u) {}

    bool isAuto() const { return unit == Unit::AUTO; }
    bool isPercent() const { return unit == Unit::PERCENT; }
    bool isPixel() const { return unit == Unit::PIXEL; }
};

// box-shadow 描述（样式层），由 Painter 解释为阴影绘制
struct BoxShadow {
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    float blur_radius = 0.0f;
    float spread_radius = 0.0f;
    std::string color; // 原始 CSS 颜色字符串
};

// Layout modes used by the layout engine (derived from CSS display/position)
enum class LayoutMode {
    Block,   // block-level boxes, including outer box of inline-block
    Inline,  // inline formatting context / inline-level boxes
    Flex,    // flex containers/items
    None     // display:none
};

// Computed style properties
struct ComputedStyle {
    // Box Model
    // width/height default to AUTO (auto-sizing)
    CSSValue width = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue height = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue min_width = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue max_width = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue min_height = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue max_height = CSSValue(0.0f, CSSValue::Unit::AUTO);
    // margins/paddings default to 0px (PIXEL), not auto
    CSSValue margin_top;
    CSSValue margin_right;
    CSSValue margin_bottom;
    CSSValue margin_left;
    CSSValue padding_top;
    CSSValue padding_right;
    CSSValue padding_bottom;
    CSSValue padding_left;
    std::string box_sizing = "content-box"; // content-box (default) or border-box

    // Layout
    std::string display = "block";  // block, inline, inline-block, flex, none
    LayoutMode layout_mode = LayoutMode::Block;
    std::string position = "static"; // static, relative, absolute, fixed
    // Position offsets default to AUTO (not set)
    CSSValue top = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue right = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue bottom = CSSValue(0.0f, CSSValue::Unit::AUTO);
    CSSValue left = CSSValue(0.0f, CSSValue::Unit::AUTO);
    int z_index = 0;

    // Visual
    std::string background_color = "transparent";  // 默认透明，不绘制背景
    std::string color = "#000000";
    float border_radius = 0.0f;
    std::string border_color = "#000000";
    float border_width = 0.0f;
    std::string overflow = "visible"; // visible, hidden, scroll
    float opacity = 1.0f;              // 0.0 ~ 1.0
    bool isolation_isolate = false;     // isolation: auto | isolate (default auto)
    std::vector<BoxShadow> box_shadows; // box-shadow 列表

    // Text
    std::string font_family = "Arial";
    float font_size = 16.0f;
    std::string font_weight = "normal"; // normal, bold
    std::string text_align = "left";    // left, center, right
    float letter_spacing_em = 0.0f;      // letter-spacing，以 em 为单位，0 表示 normal
    float word_spacing_px = 0.0f;        // word-spacing，以像素为单位，0 表示 normal
    float line_height = -1.0f;           // line-height，-1 表示 normal（使用字体度量），>0 为倍数或像素值
    bool line_height_is_unitless = true; // true: line_height 是倍数；false: line_height 是像素值
    std::string text_transform = "none"; // none, uppercase, lowercase, capitalize
    std::string vertical_align = "baseline"; // baseline, top, middle, bottom

    // Flexbox properties
    std::string flex_direction = "row";     // row, column
    std::string justify_content = "flex-start";
    std::string align_items = "stretch";
    float flex = 0.0f;
    float flex_grow = 0.0f;
    float flex_shrink = 1.0f;
    CSSValue flex_basis = CSSValue(0.0f, CSSValue::Unit::AUTO);  // default to auto
    float gap = 0.0f;                       // gap 属性（flex/grid 容器间距）

    // Transform（极简版，仅 translate/scale，用于 LayerTree + 缓存覆盖）
    float transform_translate_x = 0.0f;
    float transform_translate_y = 0.0f;
    float transform_scale_x = 1.0f;
    float transform_scale_y = 1.0f;
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
    void setTextContent(const std::string& text) {
        text_content = text;
        markLayoutDirty();
    }

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

    // Layout dirtiness for incremental layout
    void markLayoutDirty();
    void clearLayoutDirtyRecursive();
    bool isLayoutDirty() const { return layout_dirty_; }

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
    bool layout_dirty_ = true;

    std::weak_ptr<DOMNode> parent;
    std::vector<DOMNodePtr> children;
};

} // namespace dong::dom
