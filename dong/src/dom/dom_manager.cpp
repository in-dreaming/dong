#include "dom_manager.hpp"

namespace dong::dom {

Manager::Manager() : parser(std::make_unique<Parser>()) {}

Manager::~Manager() = default;

bool Manager::loadHTML(const std::string& html) {
    root = parser->parse(html);
    return root != nullptr;
}

bool Manager::loadHTMLWithCSS(const std::string& html, const std::string& css) {
    root = parser->parseWithCSS(html, css);
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

void Manager::printTree() {
    if (root) {
        root->print();
    }
}

} // namespace dong::dom
