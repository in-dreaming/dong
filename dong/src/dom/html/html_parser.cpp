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

    // Determine the context tag so we can pick the right wrapper.
    // When parsing innerHTML for table parts, we must wrap in the appropriate
    // table ancestor so the HTML5 parser recognises the content as valid.
    // Without this, <td>/<th>/<tr> content gets foster-parented to <body>.
    std::string ctx_tag;
    if (context) {
        ctx_tag = context->getTagName();
    }

    std::string wrapped;
    std::string extract_tag;     // the element whose children we want
    if (ctx_tag == "tr") {
        // Fragment is a list of <td>/<th> cells
        wrapped = "<table><tbody><tr>" + html + "</tr></tbody></table>";
        extract_tag = "tr";
    } else if (ctx_tag == "tbody" || ctx_tag == "thead" || ctx_tag == "tfoot") {
        // Fragment is a list of <tr> rows
        wrapped = "<table>" + html + "</table>";
        extract_tag = ctx_tag;
    } else if (ctx_tag == "table") {
        wrapped = "<table>" + html + "</table>";
        extract_tag = "table";
    } else {
        // Default: body context
        wrapped = "<body>" + html + "</body>";
        extract_tag = "body";
    }

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

    // Find the target element and move its children to fragment
    std::function<bool(DOMNodePtr)> findTarget = [&](DOMNodePtr node) -> bool {
        if (node->getTagName() == extract_tag) {
            for (const auto& child : node->getChildren()) {
                fragment->appendChild(child->cloneNode(true));
            }
            return true;
        }
        for (const auto& child : node->getChildren()) {
            if (findTarget(child)) return true;
        }
        return false;
    };

    if (dom_root) {
        findTarget(dom_root);
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
    StyleEngine::applyDefaultStylesRecursive(node);
}


void HTMLParser::parseInlineStyles(DOMNodePtr node) {
    if (!node) return;

    if (node->hasAttribute("style")) {
        std::string style_str = node->getAttribute("style");

        // Store the raw attribute string so cssText can return it in declaration order.
        node->setInlineStyleAttrString(style_str);

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
        // Restore the raw attr string since setInlineStyleProperty clears it.
        node->setInlineStyleAttrString(style_str);
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
