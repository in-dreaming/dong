#pragma once

#include "../dom/dom_node.hpp"


#include <string>
#include <memory>

// Forward declare Lexbor types
extern "C" {
    typedef struct lxb_html_document lxb_html_document_t;
    typedef struct lxb_dom_node lxb_dom_node_t;
}

namespace dong::dom {

class StyleEngine;

// HTML Parser using Lexbor
class HTMLParser {
public:
    HTMLParser();
    ~HTMLParser();

    // Parse HTML string and return DOM tree
    DOMNodePtr parse(const std::string& html);

    // Parse HTML with inline CSS
    DOMNodePtr parseWithCSS(const std::string& html, const std::string& css);
    
    // Parse HTML fragment (for innerHTML)
    DOMNodePtr parseFragment(const std::string& html, DOMNodePtr context = nullptr);
    
    // Extract styles from <style> tags and <link rel="stylesheet"> and add to StyleEngine.
    // resource_root is used to resolve relative paths for external stylesheets.
    void extractAndApplyStyles(DOMNodePtr node, StyleEngine* style_engine,
                               const std::string& resource_root = {});

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

    lxb_html_document_t* doc_ = nullptr;
};

// Alias for backward compatibility
using Parser = HTMLParser;

} // namespace dong::dom
