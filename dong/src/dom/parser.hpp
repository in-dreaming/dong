#pragma once

#include "dom_node.hpp"
#include <string>
#include <memory>

// Forward declare Lexbor types
extern "C" {
    typedef struct lxb_html_document lxb_html_document_t;
    typedef struct lxb_dom_node lxb_dom_node_t;
}

namespace dong::dom {

// HTML Parser using Lexbor
class Parser {
public:
    Parser();
    ~Parser();

    // Parse HTML string and return DOM tree
    DOMNodePtr parse(const std::string& html);

    // Parse HTML with inline CSS
    DOMNodePtr parseWithCSS(const std::string& html, const std::string& css);
    
    // Extract styles from <style> tags and add to StyleEngine
    void extractAndApplyStyles(DOMNodePtr node, class StyleEngine* style_engine);

private:
    DOMNodePtr lexborNodeToDOMNode(lxb_dom_node_t* lexbor_node);
    void applyDefaultStyles(DOMNodePtr node);
    void parseInlineStyles(DOMNodePtr node);
    void parseCSSAndApply(DOMNodePtr node, const std::string& css);
    
    // CSS value parsing helpers
    CSSValue parseCSSValue(const std::string& value);
    float parseLength(const std::string& value);
    void parseMarginShorthand(const std::string& value, ComputedStyle& style);
    void parsePaddingShorthand(const std::string& value, ComputedStyle& style);

    lxb_html_document_t* doc = nullptr;
};

} // namespace dong::dom
