#pragma once

#include "dom/dom_node.hpp"
#include "parser.hpp"
#include <memory>

namespace dong::dom {

// Forward declaration
class StyleEngine;

// Manages DOM tree and provides query/modification APIs
class Manager {
public:
    Manager();
    ~Manager();

    // Load and parse HTML
    bool loadHTML(const std::string& html);
    bool loadHTMLWithCSS(const std::string& html, const std::string& css);

    void setResourceRoot(const std::string& root) { resource_root_ = root; }
    const std::string& getResourceRoot() const { return resource_root_; }

    // DOM queries
    DOMNodePtr getRoot() const { return root; }
    DOMNodePtr getElementById(const std::string& id);
    std::vector<DOMNodePtr> getElementsByTagName(const std::string& tag);
    std::vector<DOMNodePtr> getElementsByClassName(const std::string& cls);

    // DOM manipulation
    void appendChild(DOMNodePtr parent, DOMNodePtr child);
    void removeChild(DOMNodePtr parent, DOMNodePtr child);

    // Style queries
    const ComputedStyle& getComputedStyle(DOMNodePtr node);

    // Runtime style recomputation (for class changes)
    void recomputeNodeStyle(DOMNodePtr node);
    StyleEngine* getStyleEngine() const { return style_engine.get(); }

    // Debug
    void printTree();

private:
    DOMNodePtr root;
    std::unique_ptr<Parser> parser;
    std::unique_ptr<StyleEngine> style_engine;
    std::string resource_root_;

    // Helper function to handle autofocus attribute after HTML loading
    void handleAutofocus(DOMNodePtr root_node);
};

} // namespace dong::dom
