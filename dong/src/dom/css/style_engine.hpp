#pragma once

#include "css_parser.hpp"
#include "selector_matcher.hpp"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace dong::dom {

class DOMNode;
using DOMNodePtr = std::shared_ptr<DOMNode>;

// CSS stylesheet
class Stylesheet {
public:
    void addRule(const std::string& selector, const ComputedStyle& style, 
                 int specificity, int order);
    const std::vector<CSSRule>& getRules() const { return rules_; }
    std::vector<CSSRule>& getRulesMutable() { return rules_; }
    size_t ruleCount() const { return rules_.size(); }

    bool insertRuleAt(size_t index, const CSSRule& rule);
    bool deleteRuleAt(size_t index);
    
    void addKeyframes(const KeyframesRule& keyframes);
    const KeyframesRule* getKeyframes(const std::string& name) const;
    
    void addFontFace(const FontFaceRule& font_face);
    const std::vector<FontFaceRule>& getFontFaces() const { return font_faces_; }

private:
    std::vector<CSSRule> rules_;
    std::unordered_map<std::string, KeyframesRule> keyframes_;
    std::vector<FontFaceRule> font_faces_;
};


// 优化策略3：规则索引结构，用于快速查找匹配规则
struct RuleIndex {
    // 按 tag 名索引的规则（如 "div", "button"）
    std::unordered_map<std::string, std::vector<size_t>> by_tag;
    // 按 class 名索引的规则（如 ".container", ".btn"）
    std::unordered_map<std::string, std::vector<size_t>> by_class;
    // 按 id 索引的规则（如 "#header"）
    std::unordered_map<std::string, std::vector<size_t>> by_id;
    // 通用规则（如 "*", 复杂选择器）
    std::vector<size_t> universal;
    
    void clear() {
        by_tag.clear();
        by_class.clear();
        by_id.clear();
        universal.clear();
    }
};

// Style engine for CSS matching and cascading
class StyleEngine {
public:
    StyleEngine();

    // Add stylesheet from CSS text
    void addStylesheet(const std::string& css);
    
    // Add stylesheet object
    void addStylesheet(const Stylesheet& sheet);

    size_t stylesheetCount() const { return stylesheets_.size(); }
    Stylesheet* stylesheetAt(size_t index);
    const Stylesheet* stylesheetAt(size_t index) const;

    // Parse CSS and extract rules
    std::vector<CSSRule> parseCSS(const std::string& css);


    // Apply inline style property (static for use without engine instance)
    static void applyInlineStyleProperty(const std::string& property,
                                         const std::string& value,
                                         ComputedStyle& style);

    // Apply default tag styles (shared by parser + runtime recompute)
    static void applyDefaultStyleForNode(DOMNodePtr node);
    static void applyDefaultStylesRecursive(DOMNodePtr node);

    // Calculate computed styles for a node (considering cascade)
    void computeStyles(DOMNodePtr node);

    
    // 优化策略3：增量样式计算 - 只计算 dirty 节点
    void computeStylesIncremental(DOMNodePtr node);
    
    // Recompute styles for a single node (for dynamic updates)
    void recomputeNodeStyle(DOMNodePtr node);

    // Full style recompute for a single node (used by incremental path)
    void recomputeNodeStyleFull(DOMNodePtr node);

    // Check if selector matches node
    bool matches(const std::string& selector, DOMNodePtr node);
    
    // Get keyframes by name
    const KeyframesRule* getKeyframes(const std::string& name) const;
    
    // Media query support
    void setViewportSize(float width, float height);
    bool evaluateMediaQuery(const std::string& query) const;
    
    // 优化策略3：重建规则索引（在添加样式表后调用）
    void rebuildRuleIndex();

private:
    std::vector<Stylesheet> stylesheets_;
    CSSParser parser_;
    SelectorMatcher matcher_;
    int rule_order_counter_ = 0;
    
    // 优化策略3：规则索引
    RuleIndex rule_index_;
    std::vector<CSSRule> all_rules_;  // 所有规则的扁平列表
    bool index_dirty_ = true;
    
    // Viewport for media queries
    float viewport_width_ = 800.0f;
    float viewport_height_ = 600.0f;

    // Environment variables for env() function
    CSSEnvironment env_variables_;
    
    // Apply matching rules to node
    void applyMatchingRules(DOMNodePtr node);
    
    // 优化策略3：使用索引快速查找匹配规则
    void applyMatchingRulesIndexed(DOMNodePtr node);
    
    // Inherit properties from parent
    void inheritFromParent(DOMNodePtr node);

    // Incremental style computation helper.
    // If an ancestor style was recomputed, descendants must also be recomputed because
    // selector matching and inheritance may change (e.g. body[data-x] .child).
    void computeStylesIncrementalImpl(DOMNodePtr node, bool ancestor_dirty);

    // Process CSS global keywords (inherit, initial, unset)
    void processGlobalKeywords(DOMNodePtr node, DOMNodePtr parent);

    // Copy a single property value from parent to child
    void copyPropertyFromParent(const std::string& prop,
                                ComputedStyle& child_style,
                                const ComputedStyle& parent_style);

    // Process pseudo-elements (::before/::after)
    void processPseudoElements(DOMNodePtr node);
    
    // Create pseudo-element node
    DOMNodePtr createPseudoElement(DOMNodePtr parent, const std::string& pseudo_type);
    
    // Derive layout mode from display
    LayoutMode deriveLayoutMode(const ComputedStyle& style);
    
    // Selector matching helpers (delegated to SelectorMatcher)
    bool matchesSelector(const std::string& selector, DOMNodePtr node);

    // Layer cascade support
    struct LayerContext {
        std::vector<LayerRule> layers; // 所有层（按声明顺序）
        std::unordered_map<std::string, int> layer_order_map; // 层名到优先级的映射
    };

    LayerContext layer_context_;
    int anon_layer_counter_ = 0; // Counter for generating unique anonymous layer names

    // Layer cascade methods
    void processLayerRules(const std::vector<LayerRule>& layer_rules);
    void sortRulesWithLayerPriority(std::vector<CSSRule>& rules);
    int getLayerPriority(const std::string& layer_name) const;
    
    // Specificity calculation
    int calculateSpecificity(const std::string& selector);
    int countIdSelectors(const std::string& selector);
    int countClassSelectors(const std::string& selector);
    int countElementSelectors(const std::string& selector);

    // CSS parsing helpers
    std::string trimWhitespace(const std::string& str);
    std::vector<std::string> splitDeclarations(const std::string& css);
    std::pair<std::string, ComputedStyle> parseRule(const std::string& rule_str);
    void applyStyleProperty(const std::string& property, const std::string& value, 
                            ComputedStyle& style);
    
    // Selector parsing
    std::vector<SelectorPart> parseSelector(const std::string& selector);
    std::string extractSelectorComponent(const std::string& selector, size_t& pos);
    
    // 优化策略3：从选择器中提取索引键
    void extractIndexKeys(const std::string& selector,
                          std::string& out_tag,
                          std::vector<std::string>& out_classes,
                          std::string& out_id);

    // Light-dark() function resolution
    void resolveLightDarkFunctions(DOMNodePtr node);
    std::string resolveLightDarkColor(const std::string& light_dark_value, const std::string& color_scheme);

    // env() function resolution
    void resolveEnvFunctions(DOMNodePtr node);
    CSSValue resolveEnvValue(const CSSValue& env_value, const CSSEnvironment& env) const;
};

} // namespace dong::dom
