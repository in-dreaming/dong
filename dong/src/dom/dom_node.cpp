#include "dom_node.hpp"
#include <iostream>
#include <algorithm>

namespace dong::dom {

DOMNode::DOMNode(NodeType type, const std::string& name)
    : type(type), tag_name(name) {}

void DOMNode::appendChild(DOMNodePtr child) {
    if (!child) return;
    children.push_back(child);
    child->parent = std::weak_ptr<DOMNode>(
        std::static_pointer_cast<DOMNode>(shared_from_this())
    );
}

void DOMNode::removeChild(DOMNodePtr child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
        child->parent.reset();
    }
}

void DOMNode::setAttribute(const std::string& key, const std::string& value) {
    attributes[key] = value;
}

std::string DOMNode::getAttribute(const std::string& key) const {
    auto it = attributes.find(key);
    return it != attributes.end() ? it->second : "";
}

bool DOMNode::hasAttribute(const std::string& key) const {
    return attributes.find(key) != attributes.end();
}

DOMNodePtr DOMNode::getElementById(const std::string& id) {
    if (hasAttribute("id") && getAttribute("id") == id) {
        return std::static_pointer_cast<DOMNode>(shared_from_this());
    }

    for (const auto& child : children) {
        auto result = child->getElementById(id);
        if (result) return result;
    }

    return nullptr;
}

std::vector<DOMNodePtr> DOMNode::getElementsByTagName(const std::string& tag) {
    std::vector<DOMNodePtr> result;

    if (tag_name == tag) {
        result.push_back(std::static_pointer_cast<DOMNode>(shared_from_this()));
    }

    for (const auto& child : children) {
        auto child_results = child->getElementsByTagName(tag);
        result.insert(result.end(), child_results.begin(), child_results.end());
    }

    return result;
}

std::vector<DOMNodePtr> DOMNode::getElementsByClassName(const std::string& cls) {
    std::vector<DOMNodePtr> result;

    if (hasAttribute("class")) {
        std::string classes = getAttribute("class");
        if (classes.find(cls) != std::string::npos) {
            result.push_back(std::static_pointer_cast<DOMNode>(shared_from_this()));
        }
    }

    for (const auto& child : children) {
        auto child_results = child->getElementsByClassName(cls);
        result.insert(result.end(), child_results.begin(), child_results.end());
    }

    return result;
}

void DOMNode::print(int depth) const {
    for (int i = 0; i < depth; ++i) std::cout << "  ";

    switch (type) {
        case NodeType::ELEMENT:
            std::cout << "<" << tag_name;
            for (const auto& [key, value] : attributes) {
                std::cout << " " << key << "=\"" << value << "\"";
            }
            std::cout << ">" << std::endl;
            break;
        case NodeType::TEXT:
            if (!text_content.empty()) {
                std::cout << "\"" << text_content << "\"" << std::endl;
            }
            break;
        case NodeType::COMMENT:
            std::cout << "<!-- comment -->" << std::endl;
            break;
        case NodeType::DOCUMENT:
            std::cout << "#document" << std::endl;
            break;
    }

    for (const auto& child : children) {
        child->print(depth + 1);
    }
}

} // namespace dong::dom
