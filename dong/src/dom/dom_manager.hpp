#pragma once

#include "dom_node.hpp"
#include "parser.hpp"
#include <memory>

namespace dong::dom {

// Manages DOM tree and provides query/modification APIs
class Manager {
public:
    Manager();
    ~Manager();

    // Load and parse HTML
    bool loadHTML(const std::string& html);
    bool loadHTMLWithCSS(const std::string& html, const std::string& css);

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

    // Debug
    void printTree();

private:
    DOMNodePtr root;
    std::unique_ptr<Parser> parser;
};

} // namespace dong::dom
