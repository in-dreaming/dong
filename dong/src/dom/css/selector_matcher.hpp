#pragma once

#include <string>
#include <vector>
#include <memory>

namespace dong::dom {

class DOMNode;
using DOMNodePtr = std::shared_ptr<DOMNode>;

// Selector token for complex selector parsing
struct SelectorPart {
    enum class Type {
        ELEMENT,
        CLASS,
        ID,
        ATTRIBUTE,
        PSEUDO_CLASS,
        PSEUDO_ELEMENT,
        COMBINATOR
    };
    
    Type type;
    std::string value;
    std::string combinator;  // " " for descendant, ">" for child, "+" for adjacent, "~" for general
    std::string argument;    // for :nth-child(2n+1), :not(.class), etc.
};

// Selector matcher - matches CSS selectors against DOM nodes
class SelectorMatcher {
public:
    SelectorMatcher() = default;
    
    // Check if selector matches node
    bool matches(const std::string& selector, DOMNodePtr node);
    
    // Find all matching elements
    std::vector<DOMNodePtr> querySelectorAll(const std::string& selector, DOMNodePtr root);
    
    // Find first matching element
    DOMNodePtr querySelector(const std::string& selector, DOMNodePtr root);
    
    // Check if element matches selector (for Element.matches())
    bool elementMatches(const std::string& selector, DOMNodePtr element);
    
    // Find closest ancestor matching selector
    DOMNodePtr closest(const std::string& selector, DOMNodePtr element);

private:
    // Parse selector into parts
    std::vector<SelectorPart> parseSelector(const std::string& selector);
    
    // Match different selector types
    bool matchesSimpleSelector(const std::string& selector, DOMNodePtr node);
    bool matchesComplexSelector(const std::string& selector, DOMNodePtr node);
    bool matchesTagSelector(const std::string& tag, DOMNodePtr node);
    bool matchesClassSelector(const std::string& cls, DOMNodePtr node);
    bool matchesIdSelector(const std::string& id, DOMNodePtr node);
    bool matchesAttributeSelector(const std::string& attr_sel, DOMNodePtr node);
    bool matchesPseudoClass(const std::string& pseudo, DOMNodePtr node);
    bool matchesPseudoElement(const std::string& pseudo, DOMNodePtr node);
    
    // Combinator matching
    bool matchesDescendant(const std::string& ancestor_sel, DOMNodePtr node);
    bool matchesChild(const std::string& parent_sel, DOMNodePtr node);
    bool matchesAdjacentSibling(const std::string& prev_sel, DOMNodePtr node);
    bool matchesGeneralSibling(const std::string& prev_sel, DOMNodePtr node);
    
    // Pseudo-class helpers
    bool matchesNthChild(const std::string& arg, DOMNodePtr node);
    bool matchesNthLastChild(const std::string& arg, DOMNodePtr node);
    bool matchesNthOfType(const std::string& arg, DOMNodePtr node);
    bool matchesNot(const std::string& arg, DOMNodePtr node);
    bool matchesIs(const std::string& arg, DOMNodePtr node);
    bool matchesWhere(const std::string& arg, DOMNodePtr node);
    bool matchesHas(const std::string& arg, DOMNodePtr node);
    
    // Helper
    std::string trimWhitespace(const std::string& str);
    std::string extractSelectorComponent(const std::string& selector, size_t& pos);
    std::pair<int, int> parseNthExpression(const std::string& expr);  // returns (a, b) for an+b
};

} // namespace dong::dom
