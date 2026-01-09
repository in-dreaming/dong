#pragma once

#include "dom_node.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace dong::dom {

// CSS rule representation with specificity
struct CSSRule {
    std::string selector;
    ComputedStyle style;
    int specificity = 0;  // Specificity score: (id_count, class_count, element_count)
    int source_order = 0; // Order in stylesheet (for tie-breaking)
};

// CSS stylesheet
class Stylesheet {
public:
    void addRule(const std::string& selector, const ComputedStyle& style, int specificity, int order);
    const std::vector<CSSRule>& getRules() const { return rules; }

private:
    std::vector<CSSRule> rules;
};

// Selector token for complex selector parsing
struct SelectorPart {
    enum class Type {
        ELEMENT,    // div, span, etc.
        CLASS,      // .classname
        ID,         // #idname
        ATTRIBUTE,  // [attr], [attr="value"], etc.
        PSEUDO,     // :hover, :focus, etc.
        COMBINATOR  // (space for descendant, >, +, ~)
    };
    
    Type type;
    std::string value;
    std::string combinator;  // " " for descendant, ">" for child, "+" for adjacent, "~" for general
};

// Style engine for CSS matching and cascading
class StyleEngine {
public:
    StyleEngine();

    // Add stylesheet
    void addStylesheet(const std::string& css);

    // Parse CSS and extract rules
    std::vector<CSSRule> parseCSS(const std::string& css);

    static void applyInlineStyleProperty(const std::string& property,
                                         const std::string& value,
                                         ComputedStyle& style);

    // Calculate computed styles for a node (considering cascade)
    void computeStyles(DOMNodePtr node);


    // Check if selector matches node
    bool matches(const std::string& selector, DOMNodePtr node);

private:
    std::vector<Stylesheet> stylesheets;
    int rule_order_counter = 0;

    // Selector matching helpers
    bool matchesSelector(const std::string& selector, DOMNodePtr node);
    bool matchesComplexSelector(const std::string& selector, DOMNodePtr node);
    bool matchesSimpleSelector(const std::string& selector, DOMNodePtr node);
    bool matchesTagSelector(const std::string& tag, DOMNodePtr node);
    bool matchesClassSelector(const std::string& cls, DOMNodePtr node);
    bool matchesIdSelector(const std::string& id, DOMNodePtr node);
    bool matchesAttributeSelector(const std::string& attr_sel, DOMNodePtr node);
    bool matchesPseudoClass(const std::string& pseudo, DOMNodePtr node);

    // Descendant/combinator matching
    bool matchesDescendant(const std::string& ancestor_sel, DOMNodePtr node);
    bool matchesChild(const std::string& parent_sel, DOMNodePtr node);
    bool matchesAdjacent(const std::string& prev_sel, DOMNodePtr node);

    // Specificity calculation
    int calculateSpecificity(const std::string& selector);
    int countIdSelectors(const std::string& selector);
    int countClassSelectors(const std::string& selector);
    int countElementSelectors(const std::string& selector);

    // CSS parsing helpers
    std::string trimWhitespace(const std::string& str);
    std::vector<std::string> splitDeclarations(const std::string& css);
    std::pair<std::string, ComputedStyle> parseRule(const std::string& rule_str);
    void applyStyleProperty(const std::string& property, const std::string& value, ComputedStyle& style);
    
    // Selector parsing
    std::vector<SelectorPart> parseSelector(const std::string& selector);
    std::string extractSelectorComponent(const std::string& selector, size_t& pos);
};

} // namespace dong::dom
