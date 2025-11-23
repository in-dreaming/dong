#include "parser.hpp"
#include "style_engine.hpp"
#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>
#include <lexbor/css/css.h>
#include <iostream>
#include <cctype>
#include <algorithm>

namespace dong::dom {

Parser::Parser() {
}

Parser::~Parser() {
    if (doc) {
        lxb_html_document_destroy(doc);
        doc = nullptr;
    }
}

DOMNodePtr Parser::parse(const std::string& html) {
    doc = lxb_html_document_create();
    if (!doc) {
        std::cerr << "Failed to create Lexbor HTML document" << std::endl;
        return nullptr;
    }

    lxb_status_t status = lxb_html_document_parse(
        doc,
        reinterpret_cast<const uint8_t*>(html.c_str()),
        html.length()
    );

    if (status != LXB_STATUS_OK) {
        std::cerr << "Failed to parse HTML: status " << status << std::endl;
        lxb_html_document_destroy(doc);
        doc = nullptr;
        return nullptr;
    }

    lxb_dom_node_t* root = lxb_dom_interface_node(doc);
    if (!root) {
        std::cerr << "Failed to get root node from Lexbor document" << std::endl;
        return nullptr;
    }

    auto dom_root = lexborNodeToDOMNode(root);
    if (dom_root) {
        applyDefaultStyles(dom_root);
        
        // Extract styles from <style> tags and compute CSS-based styles
        auto style_engine = std::make_unique<StyleEngine>();
        extractAndApplyStyles(dom_root, style_engine.get());
        style_engine->computeStyles(dom_root);
        
        // Apply inline styles (override stylesheet rules)
        parseInlineStyles(dom_root);
    }

    return dom_root;
}

DOMNodePtr Parser::parseWithCSS(const std::string& html, const std::string& css) {
    auto root = parse(html);
    if (root) {
        parseCSSAndApply(root, css);
    }
    return root;
}

DOMNodePtr Parser::lexborNodeToDOMNode(lxb_dom_node_t* lexbor_node) {
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
        // Get text content from text node
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

void Parser::applyDefaultStyles(DOMNodePtr node) {
    if (!node) return;

    auto& style = node->getComputedStyle();
    std::string tag = node->getTagName();

    if (tag == "div" || tag == "p" || tag == "body" || tag == "html" || tag == "main" || 
        tag == "section" || tag == "article" || tag == "nav" || tag == "header" || tag == "footer") {
        style.display = "block";
    }
    else if (tag == "span" || tag == "a" || tag == "b" || tag == "i" || tag == "strong" || tag == "em") {
        style.display = "inline";
    }
    else if (tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" || tag == "h5" || tag == "h6") {
        style.display = "block";
        style.font_weight = "bold";
        if (tag == "h1") style.font_size = 32.0f;
        else if (tag == "h2") style.font_size = 28.0f;
        else if (tag == "h3") style.font_size = 24.0f;
        else if (tag == "h4") style.font_size = 20.0f;
        else if (tag == "h5") style.font_size = 18.0f;
        else if (tag == "h6") style.font_size = 16.0f;
    }
    else if (tag == "button" || tag == "input") {
        style.display = "inline-block";
    }
    else if (tag == "img" || tag == "video") {
        style.display = "inline-block";
    }
    else {
        style.display = "block";
    }

    for (const auto& child : node->getChildren()) {
        applyDefaultStyles(child);
    }
}

void Parser::parseInlineStyles(DOMNodePtr node) {
    if (!node) return;

    if (node->hasAttribute("style")) {
        std::string style_str = node->getAttribute("style");
        auto& computed_style = node->getComputedStyle();
        
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
            
            if (property == "color") {
                computed_style.color = value;
            }
            else if (property == "background-color" || property == "background") {
                computed_style.background_color = value;
            }
            else if (property == "font-size") {
                computed_style.font_size = parseLength(value);
            }
            else if (property == "font-weight") {
                computed_style.font_weight = value;
            }
            else if (property == "text-align") {
                computed_style.text_align = value;
            }
            else if (property == "width") {
                computed_style.width = parseCSSValue(value);
            }
            else if (property == "height") {
                computed_style.height = parseCSSValue(value);
            }
            else if (property == "margin") {
                parseMarginShorthand(value, computed_style);
            }
            else if (property == "margin-top") {
                computed_style.margin_top = parseCSSValue(value);
            }
            else if (property == "margin-right") {
                computed_style.margin_right = parseCSSValue(value);
            }
            else if (property == "margin-bottom") {
                computed_style.margin_bottom = parseCSSValue(value);
            }
            else if (property == "margin-left") {
                computed_style.margin_left = parseCSSValue(value);
            }
            else if (property == "padding") {
                parsePaddingShorthand(value, computed_style);
            }
            else if (property == "padding-top") {
                computed_style.padding_top = parseCSSValue(value);
            }
            else if (property == "padding-right") {
                computed_style.padding_right = parseCSSValue(value);
            }
            else if (property == "padding-bottom") {
                computed_style.padding_bottom = parseCSSValue(value);
            }
            else if (property == "padding-left") {
                computed_style.padding_left = parseCSSValue(value);
            }
            else if (property == "display") {
                computed_style.display = value;
            }
            else if (property == "flex-direction") {
                computed_style.flex_direction = value;
            }
            else if (property == "justify-content") {
                computed_style.justify_content = value;
            }
            else if (property == "align-items") {
                computed_style.align_items = value;
            }
        }
    }

    for (const auto& child : node->getChildren()) {
        parseInlineStyles(child);
    }
}

void Parser::parseCSSAndApply(DOMNodePtr node, const std::string& css) {
    if (!node) return;
    
    auto style_engine = std::make_unique<StyleEngine>();
    style_engine->addStylesheet(css);
    style_engine->computeStyles(node);
}

void Parser::extractAndApplyStyles(DOMNodePtr node, StyleEngine* style_engine) {
    if (!node || !style_engine) return;
    
    // Look for <style> tags
    if (node->getTagName() == "style") {
        if (node->getChildren().size() > 0) {
            // Get text content of style tag
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
    
    // Recursively search for style tags
    for (const auto& child : node->getChildren()) {
        extractAndApplyStyles(child, style_engine);
    }
}

CSSValue Parser::parseCSSValue(const std::string& value) {
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

float Parser::parseLength(const std::string& value) {
    try {
        size_t pos = 0;
        return std::stof(value, &pos);
    } catch (...) {
        return 0.0f;
    }
}

void Parser::parseMarginShorthand(const std::string& value, ComputedStyle& style) {
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

void Parser::parsePaddingShorthand(const std::string& value, ComputedStyle& style) {
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
