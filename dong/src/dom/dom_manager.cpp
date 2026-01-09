#include "dom_manager.hpp"
#include "style_engine.hpp"

namespace dong::dom {

Manager::Manager() 
    : parser(std::make_unique<Parser>()),
      style_engine(std::make_unique<StyleEngine>()) {}

Manager::~Manager() = default;

bool Manager::loadHTML(const std::string& html) {
    root = parser->parse(html);
    if (root) {
        // Extract stylesheets from the parsed DOM and add to our persistent StyleEngine
        // The parser already computed initial styles, but we need to keep the stylesheets
        // for runtime recomputation
        style_engine = std::make_unique<StyleEngine>();
        parser->extractAndApplyStyles(root, style_engine.get());
    }
    return root != nullptr;
}

bool Manager::loadHTMLWithCSS(const std::string& html, const std::string& css) {
    root = parser->parseWithCSS(html, css);
    if (root) {
        style_engine = std::make_unique<StyleEngine>();
        parser->extractAndApplyStyles(root, style_engine.get());
        style_engine->addStylesheet(css);
    }
    return root != nullptr;
}

DOMNodePtr Manager::getElementById(const std::string& id) {
    if (!root) return nullptr;
    return root->getElementById(id);
}

std::vector<DOMNodePtr> Manager::getElementsByTagName(const std::string& tag) {
    if (!root) return {};
    return root->getElementsByTagName(tag);
}

std::vector<DOMNodePtr> Manager::getElementsByClassName(const std::string& cls) {
    if (!root) return {};
    return root->getElementsByClassName(cls);
}

void Manager::appendChild(DOMNodePtr parent, DOMNodePtr child) {
    if (parent && child) {
        parent->appendChild(child);
    }
}

void Manager::removeChild(DOMNodePtr parent, DOMNodePtr child) {
    if (parent && child) {
        parent->removeChild(child);
    }
}

const ComputedStyle& Manager::getComputedStyle(DOMNodePtr node) {
    static ComputedStyle empty;
    if (!node) return empty;
    return node->getComputedStyle();
}

void Manager::recomputeNodeStyle(DOMNodePtr node) {
    if (!node || !style_engine) return;
    
    // Recompute styles for this single node using the persistent StyleEngine
    // This is called when class attribute changes at runtime
    style_engine->computeStyles(node);
    
    // Re-apply inline styles (they should override stylesheet rules)
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
            
            // Trim whitespace
            auto trim = [](std::string& s) {
                size_t start = s.find_first_not_of(" \t");
                size_t end = s.find_last_not_of(" \t");
                if (start == std::string::npos) s.clear();
                else s = s.substr(start, end - start + 1);
            };
            trim(property);
            trim(value);
            
            StyleEngine::applyInlineStyleProperty(property, value, node->getComputedStyle());
        }
    }
    
    node->markLayoutDirty();
}

void Manager::printTree() {
    if (root) {
        root->print();
    }
}

} // namespace dong::dom
