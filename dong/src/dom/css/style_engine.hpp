#pragma once

#include "css_parser.hpp"
#include "selector_matcher.hpp"
#include <vector>
#include <unordered_map>
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
    
    void addKeyframes(const KeyframesRule& keyframes);
    const KeyframesRule* getKeyframes(const std::string& name) const;
    
    void addFontFace(const FontFaceRule& font_face);
    const std::vector<FontFaceRule>& getFontFaces() const { return font_faces_; }

private:
    std::vector<CSSRule> rules_;
    std::unordered_map<std::string, KeyframesRule> keyframes_;
    std::vector<FontFaceRule> font_faces_;
};

// Style engine for CSS matching and cascading
class StyleEngine {
public:
    StyleEngine();

    // Add stylesheet from CSS text
    void addStylesheet(const std::string& css);
    
    // Add stylesheet object
    void addStylesheet(const Stylesheet& sheet);

    // Parse CSS and extract rules
    std::vector<CSSRule> parseCSS(const std::string& css);

    // Apply inline style property (static for use without engine instance)
    static void applyInlineStyleProperty(const std::string& property,
                                         const std::string& value,
                                         ComputedStyle& style);

    // Calculate computed styles for a node (considering cascade)
    void computeStyles(DOMNodePtr node);
    
    // Recompute styles for a single node (for dynamic updates)
    void recomputeNodeStyle(DOMNodePtr node);

    // Check if selector matches node
    bool matches(const std::string& selector, DOMNodePtr node);
    
    // Get keyframes by name
    const KeyframesRule* getKeyframes(const std::string& name) const;
    
    // Media query support
    void setViewportSize(float width, float height);
    bool evaluateMediaQuery(const std::string& query) const;

private:
    std::vector<Stylesheet> stylesheets_;
    CSSParser parser_;
    SelectorMatcher matcher_;
    int rule_order_counter_ = 0;
    
    // Viewport for media queries
    float viewport_width_ = 800.0f;
    float viewport_height_ = 600.0f;
    
    // Apply matching rules to node
    void applyMatchingRules(DOMNodePtr node);
    
    // Inherit properties from parent
    void inheritFromParent(DOMNodePtr node);
    
    // Process pseudo-elements (::before/::after)
    void processPseudoElements(DOMNodePtr node);
    
    // Create pseudo-element node
    DOMNodePtr createPseudoElement(DOMNodePtr parent, const std::string& pseudo_type);
    
    // Derive layout mode from display
    LayoutMode deriveLayoutMode(const ComputedStyle& style);
    
    // Selector matching helpers (delegated to SelectorMatcher)
    bool matchesSelector(const std::string& selector, DOMNodePtr node);
    
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
};

} // namespace dong::dom
