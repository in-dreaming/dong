#include "html_parser.hpp"
#include "../css/style_engine.hpp"
#include "../../core/log.h"
#include "../../core/profiler.h"
#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>
#include <lexbor/css/css.h>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <string_view>

namespace dong::dom {

HTMLParser::HTMLParser() {
}

HTMLParser::~HTMLParser() {
    if (doc_) {
        lxb_html_document_destroy(doc_);
        doc_ = nullptr;
    }
}

DOMNodePtr HTMLParser::parse(const std::string& html) {
    DONG_PROFILE_SCOPE_CAT("HTMLParser::parse", "parse");
    DONG_LOG_INFO("[HTMLParser::parse] Entry, html length=%zu", html.length());
    
    doc_ = lxb_html_document_create();
    if (!doc_) {
        DONG_LOG_INFO("[HTMLParser::parse] Failed to create Lexbor HTML document");
        std::cerr << "Failed to create Lexbor HTML document" << std::endl;
        return nullptr;
    }
    DONG_LOG_INFO("[HTMLParser::parse] Document created");

    lxb_status_t status = lxb_html_document_parse(
        doc_,
        reinterpret_cast<const uint8_t*>(html.c_str()),
        html.length()
    );
    DONG_LOG_INFO("[HTMLParser::parse] lxb_html_document_parse returned status=%d", (int)status);

    if (status != LXB_STATUS_OK) {
        std::cerr << "Failed to parse HTML: status " << status << std::endl;
        lxb_html_document_destroy(doc_);
        doc_ = nullptr;
        return nullptr;
    }

    lxb_dom_node_t* root = lxb_dom_interface_node(doc_);
    if (!root) {
        DONG_LOG_INFO("[HTMLParser::parse] Failed to get root node");
        std::cerr << "Failed to get root node from Lexbor document" << std::endl;
        return nullptr;
    }

    auto dom_root = lexborNodeToDOMNode(root);
    
    if (dom_root) {
        applyDefaultStyles(dom_root);
        
        DONG_LOG_INFO("[HTMLParser::parse] Creating StyleEngine...");
        auto style_engine = std::make_unique<StyleEngine>();
        DONG_LOG_INFO("[HTMLParser::parse] Extracting and applying styles...");
        extractAndApplyStyles(dom_root, style_engine.get());
        DONG_LOG_INFO("[HTMLParser::parse] Computing styles...");
        style_engine->computeStyles(dom_root);
        
        DONG_LOG_INFO("[HTMLParser::parse] Parsing inline styles...");
        parseInlineStyles(dom_root);
        DONG_LOG_INFO("[HTMLParser::parse] Done");
    }

    return dom_root;
}

DOMNodePtr HTMLParser::parseWithCSS(const std::string& html, const std::string& css) {
    DONG_PROFILE_SCOPE_CAT("HTMLParser::parseWithCSS", "parse");
    auto root = parse(html);
    if (root) {
        parseCSSAndApply(root, css);
    }
    return root;
}

DOMNodePtr HTMLParser::parseFragment(const std::string& html, DOMNodePtr context) {
    DONG_PROFILE_SCOPE_CAT("HTMLParser::parseFragment", "parse");
    // Create a temporary document to parse the fragment
    lxb_html_document_t* temp_doc = lxb_html_document_create();
    if (!temp_doc) {
        return nullptr;
    }
    
    // Wrap in body to ensure proper parsing
    std::string wrapped = "<body>" + html + "</body>";
    
    lxb_status_t status = lxb_html_document_parse(
        temp_doc,
        reinterpret_cast<const uint8_t*>(wrapped.c_str()),
        wrapped.length()
    );
    
    if (status != LXB_STATUS_OK) {
        lxb_html_document_destroy(temp_doc);
        return nullptr;
    }
    
    lxb_dom_node_t* root = lxb_dom_interface_node(temp_doc);
    if (!root) {
        lxb_html_document_destroy(temp_doc);
        return nullptr;
    }
    
    // Find body and extract its children
    auto dom_root = lexborNodeToDOMNode(root);
    
    // Create a document fragment to hold the children
    auto fragment = std::make_shared<DOMNode>(DOMNode::NodeType::DOCUMENT_FRAGMENT);
    
    // Find body element and move its children to fragment
    std::function<void(DOMNodePtr)> findBody = [&](DOMNodePtr node) {
        if (node->getTagName() == "body") {
            for (const auto& child : node->getChildren()) {
                fragment->appendChild(child->cloneNode(true));
            }
            return;
        }
        for (const auto& child : node->getChildren()) {
            findBody(child);
        }
    };
    
    if (dom_root) {
        findBody(dom_root);
    }
    
    lxb_html_document_destroy(temp_doc);
    
    return fragment;
}

DOMNodePtr HTMLParser::lexborNodeToDOMNode(lxb_dom_node_t* lexbor_node) {
    if (!lexbor_node) return nullptr;

    lxb_dom_node_type_t node_type = lexbor_node->type;

    std::string tag_name;
    if (node_type == LXB_DOM_NODE_TYPE_ELEMENT) {
        auto element = lxb_dom_interface_element(lexbor_node);
        if (element) {
            size_t tag_len = 0;
            const lxb_char_t* tag = lxb_dom_element_tag_name(element, &tag_len);
            if (tag && tag_len > 0) {
                tag_name = std::string(reinterpret_cast<const char*>(tag), tag_len);
                std::transform(tag_name.begin(), tag_name.end(), tag_name.begin(),
                    [](unsigned char c) { return std::tolower(c); });
            }
        }
    }

    auto dom_node = std::make_shared<DOMNode>(
        node_type == LXB_DOM_NODE_TYPE_ELEMENT ? DOMNode::NodeType::ELEMENT :
        node_type == LXB_DOM_NODE_TYPE_TEXT ? DOMNode::NodeType::TEXT :
        node_type == LXB_DOM_NODE_TYPE_COMMENT ? DOMNode::NodeType::COMMENT :
        DOMNode::NodeType::DOCUMENT,
        tag_name
    );

    if (node_type == LXB_DOM_NODE_TYPE_ELEMENT) {
        auto element = lxb_dom_interface_element(lexbor_node);
        if (element) {
            lxb_dom_attr_t* attr = element->first_attr;
            while (attr) {
                size_t name_len = 0;
                const lxb_char_t* name_ptr = lxb_dom_attr_qualified_name(attr, &name_len);
                
                if (name_ptr && name_len > 0) {
                    std::string attr_name(reinterpret_cast<const char*>(name_ptr), name_len);
                    
                    size_t value_len = 0;
                    const lxb_char_t* value_ptr = lxb_dom_attr_value(attr, &value_len);
                    std::string attr_value;
                    if (value_ptr && value_len > 0) {
                        attr_value = std::string(reinterpret_cast<const char*>(value_ptr), value_len);
                    }
                    
                    dom_node->setAttribute(attr_name, attr_value);
                }
                
                attr = attr->next;
            }
        }
    }
    else if (node_type == LXB_DOM_NODE_TYPE_TEXT) {
        lxb_dom_text_t* text_node = lxb_dom_interface_text(lexbor_node);
        if (text_node) {
            size_t text_len = 0;
            const lxb_char_t* text_ptr = lxb_dom_node_text_content(lexbor_node, &text_len);
            if (text_ptr && text_len > 0) {
                std::string text_content(reinterpret_cast<const char*>(text_ptr), text_len);
                dom_node->setTextContent(text_content);
            }
        }
    }

    // Recursively add child nodes
    if (lexbor_node->first_child) {
        for (lxb_dom_node_t* child = lexbor_node->first_child; child; child = child->next) {
            auto child_node = lexborNodeToDOMNode(child);
            if (child_node) {
                dom_node->appendChild(child_node);
            }
        }
    }

    return dom_node;
}

void HTMLParser::applyDefaultStyles(DOMNodePtr node) {
    if (!node) return;

    auto& style = node->getComputedStyle();
    const std::string& tag = node->getTagName();

    // ========================================================================
    // Tag-based default style dispatch table - O(1) lookup
    // ========================================================================
    using TagStyleHandler = void(*)(ComputedStyle&);
    
    static const std::unordered_map<std::string_view, TagStyleHandler> tag_handlers = {
        // Block-level elements
        {"div", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"p", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"body", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"html", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"main", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"section", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"article", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"nav", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"header", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"footer", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"aside", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"address", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"blockquote", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"figure", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"figcaption", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"ul", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"ol", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"li", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"dl", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"dt", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"dd", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"form", [](ComputedStyle& s) { s.setDisplay("block"); }},
        {"fieldset", [](ComputedStyle& s) { s.setDisplay("block"); }},
        
        // Inline elements
        {"span", [](ComputedStyle& s) { s.setDisplay("inline"); }},
        {"a", [](ComputedStyle& s) { s.setDisplay("inline"); }},
        {"b", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_weight = "bold"; }},
        {"i", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; }},
        {"strong", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_weight = "bold"; }},
        {"em", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; }},
        {"code", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_family = "Menlo, Consolas, monospace"; }},
        {"kbd", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_family = "Menlo, Consolas, monospace"; }},
        {"samp", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_family = "Menlo, Consolas, monospace"; }},
        {"var", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; }},
        {"small", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_size = 12.0f; }},
        {"s", [](ComputedStyle& s) { s.setDisplay("inline"); s.text_decoration = "line-through"; }},
        {"cite", [](ComputedStyle& s) { s.setDisplay("inline"); s.font_style = "italic"; }},
        {"q", [](ComputedStyle& s) { s.setDisplay("inline"); }},
        {"mark", [](ComputedStyle& s) { s.setDisplay("inline"); s.background_color = "#ffff00"; }},
        {"sub", [](ComputedStyle& s) { s.setDisplay("inline"); s.vertical_align = "sub"; s.font_size = 12.0f; }},
        {"sup", [](ComputedStyle& s) { s.setDisplay("inline"); s.vertical_align = "super"; s.font_size = 12.0f; }},
        {"u", [](ComputedStyle& s) { s.setDisplay("inline"); s.text_decoration = "underline"; }},
        {"abbr", [](ComputedStyle& s) { s.setDisplay("inline"); }},
        {"time", [](ComputedStyle& s) { s.setDisplay("inline"); }},
        {"data", [](ComputedStyle& s) { s.setDisplay("inline"); }},
        {"wbr", [](ComputedStyle& s) { s.setDisplay("inline"); }},
        
        // Headings
        {"h1", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 32.0f; }},
        {"h2", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 28.0f; }},
        {"h3", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 24.0f; }},
        {"h4", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 20.0f; }},
        {"h5", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 18.0f; }},
        {"h6", [](ComputedStyle& s) { s.setDisplay("block"); s.font_weight = "bold"; s.font_size = 16.0f; }},
        
        // Form elements (inline-block)
        {"button", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
        {"input", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
        {"select", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
        {"textarea", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
        
        // Media elements (inline-block)
        {"img", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
        {"video", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
        {"canvas", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
        {"svg", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
        {"picture", [](ComputedStyle& s) { s.setDisplay("inline-block"); }},
        
        // Special elements
        {"br", [](ComputedStyle& s) { 
            s.setDisplay("block"); 
            s.height = CSSValue(0.0f, CSSValue::Unit::PIXEL); 
        }},
        {"hr", [](ComputedStyle& s) { 
            s.setDisplay("block"); 
            s.height = CSSValue(1.0f, CSSValue::Unit::PIXEL);
            s.margin_top = CSSValue(8.0f, CSSValue::Unit::PIXEL);
            s.margin_bottom = CSSValue(8.0f, CSSValue::Unit::PIXEL);
            s.border_width = 0.0f;
            s.background_color = "#cccccc";
        }},
        {"pre", [](ComputedStyle& s) { 
            s.setDisplay("block"); 
            s.font_family = "Menlo, Consolas, monospace"; 
        }},
        
        // Table elements
        {"table", [](ComputedStyle& s) { s.setDisplay("table"); }},
        {"tr", [](ComputedStyle& s) { s.setDisplay("table-row"); }},
        {"td", [](ComputedStyle& s) { s.setDisplay("table-cell"); }},
        {"th", [](ComputedStyle& s) { s.setDisplay("table-cell"); }},
        {"thead", [](ComputedStyle& s) { s.setDisplay("table-row-group"); }},
        {"tbody", [](ComputedStyle& s) { s.setDisplay("table-row-group"); }},
        {"tfoot", [](ComputedStyle& s) { s.setDisplay("table-row-group"); }},
    };

    // O(1) lookup in dispatch table
    auto it = tag_handlers.find(tag);
    if (it != tag_handlers.end()) {
        it->second(style);
    } else {
        // Default: block display for unknown elements
        style.setDisplay("block");
    }

    // Recursively apply to children
    for (const auto& child : node->getChildren()) {
        applyDefaultStyles(child);
    }
}

void HTMLParser::parseInlineStyles(DOMNodePtr node) {
    if (!node) return;

    if (node->hasAttribute("style")) {
        std::string style_str = node->getAttribute("style");
        
        size_t pos = 0;
        while (pos < style_str.length()) {
            size_t semicolon = style_str.find(';', pos);
            if (semicolon == std::string::npos) semicolon = style_str.length();
            
            std::string declaration = style_str.substr(pos, semicolon - pos);
            pos = semicolon + 1;
            
            size_t colon = declaration.find(':');
            if (colon == std::string::npos) continue;
            
            std::string property = declaration.substr(0, colon);
            std::string value = declaration.substr(colon + 1);
            
            property.erase(0, property.find_first_not_of(" \t"));
            property.erase(property.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            node->setInlineStyleProperty(property, value);
        }
    }

    for (const auto& child : node->getChildren()) {
        parseInlineStyles(child);
    }
}

void HTMLParser::parseCSSAndApply(DOMNodePtr node, const std::string& css) {
    if (!node) return;
    
    auto style_engine = std::make_unique<StyleEngine>();
    style_engine->addStylesheet(css);
    style_engine->computeStyles(node);
}

void HTMLParser::extractAndApplyStyles(DOMNodePtr node, StyleEngine* style_engine) {
    if (!node || !style_engine) return;
    
    // Look for <style> tags
    if (node->getTagName() == "style") {
        if (node->getChildren().size() > 0) {
            const auto& children = node->getChildren();
            std::string css_text;
            for (const auto& child : children) {
                if (child->getType() == DOMNode::NodeType::TEXT) {
                    css_text += child->getTextContent();
                }
            }
            
            if (!css_text.empty()) {
                style_engine->addStylesheet(css_text);
            }
        }
    }
    
    // Look for <link rel="stylesheet">
    if (node->getTagName() == "link") {
        std::string rel = node->getAttribute("rel");
        if (rel == "stylesheet") {
            // TODO: Load external stylesheet
            std::string href = node->getAttribute("href");
            DONG_LOG_INFO("[HTMLParser] External stylesheet: %s (not loaded)", href.c_str());
        }
    }
    
    // Recursively search for style tags
    for (const auto& child : node->getChildren()) {
        extractAndApplyStyles(child, style_engine);
    }
}

CSSValue HTMLParser::parseCSSValue(const std::string& value) {
    if (value == "auto") {
        return CSSValue(0, CSSValue::Unit::AUTO);
    }
    
    if (value.find('%') != std::string::npos) {
        try {
            float val = std::stof(value);
            return CSSValue(val, CSSValue::Unit::PERCENT);
        } catch (...) {
            return CSSValue(0, CSSValue::Unit::AUTO);
        }
    }
    
    try {
        size_t pos = 0;
        float val = std::stof(value, &pos);
        return CSSValue(val, CSSValue::Unit::PIXEL);
    } catch (...) {
        return CSSValue(0, CSSValue::Unit::AUTO);
    }
}

float HTMLParser::parseLength(const std::string& value) {
    try {
        size_t pos = 0;
        return std::stof(value, &pos);
    } catch (...) {
        return 0.0f;
    }
}

void HTMLParser::parseMarginShorthand(const std::string& value, ComputedStyle& style) {
    std::vector<CSSValue> values;
    size_t pos = 0;
    
    while (pos < value.length()) {
        size_t space = value.find(' ', pos);
        if (space == std::string::npos) space = value.length();
        
        std::string val_str = value.substr(pos, space - pos);
        if (!val_str.empty()) {
            values.push_back(parseCSSValue(val_str));
        }
        pos = space + 1;
    }
    
    if (values.size() == 1) {
        style.margin_top = style.margin_right = style.margin_bottom = style.margin_left = values[0];
    } else if (values.size() == 2) {
        style.margin_top = style.margin_bottom = values[0];
        style.margin_left = style.margin_right = values[1];
    } else if (values.size() >= 4) {
        style.margin_top = values[0];
        style.margin_right = values[1];
        style.margin_bottom = values[2];
        style.margin_left = values[3];
    }
}

void HTMLParser::parsePaddingShorthand(const std::string& value, ComputedStyle& style) {
    std::vector<CSSValue> values;
    size_t pos = 0;
    
    while (pos < value.length()) {
        size_t space = value.find(' ', pos);
        if (space == std::string::npos) space = value.length();
        
        std::string val_str = value.substr(pos, space - pos);
        if (!val_str.empty()) {
            values.push_back(parseCSSValue(val_str));
        }
        pos = space + 1;
    }
    
    if (values.size() == 1) {
        style.padding_top = style.padding_right = style.padding_bottom = style.padding_left = values[0];
    } else if (values.size() == 2) {
        style.padding_top = style.padding_bottom = values[0];
        style.padding_left = style.padding_right = values[1];
    } else if (values.size() >= 4) {
        style.padding_top = values[0];
        style.padding_right = values[1];
        style.padding_bottom = values[2];
        style.padding_left = values[3];
    }
}

} // namespace dong::dom
