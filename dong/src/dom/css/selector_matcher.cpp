#include "selector_matcher.hpp"
#include "../dom/dom_node.hpp"
#include "../details_element.hpp"
#include "../input_element.hpp"
#include "../select_element.hpp"


#include <algorithm>
#include <sstream>
#include <cctype>
#include <functional>

namespace dong::dom {

// Helper function to check if an element is valid
static bool isElementValid(const DOMNodePtr& node) {
    if (!node) return true;

    const std::string& tag = node->getTagName();

    // For input and textarea elements
    if (tag == "input" || tag == "textarea") {
        auto* state = dong::dom::getInputState(node);

        // Check custom validity
        if (state && state->hasCustomError()) return false;

        // Check required field
        if (node->hasAttribute("required")) {
            std::string value = state ? state->getValue() : node->getAttribute("value");
            if (value.empty()) return false;
        }

        // Check pattern validation
        if (state && state->hasPattern()) {
            std::string value = state->getValue();
            if (!state->matchesPattern(value)) return false;
        }

        // Check range validation
        if (state && (state->hasMin() || state->hasMax() || state->hasStep())) {
            std::string value = state->getValue();
            if (!state->isInRange(value)) return false;
        }

        return true;
    }

    // For select elements
    if (tag == "select") {
        if (node->hasAttribute("required")) {
            auto* state = dong::dom::getSelectState(node);
            if (state) {
                return !state->getSelectedValue().empty();
            }
            return !node->getAttribute("value").empty();
        }
        return true;
    }

    // For other elements, assume valid
    return true;
}

std::string SelectorMatcher::trimWhitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

bool SelectorMatcher::matches(const std::string& selector, DOMNodePtr node) {
    if (!node || selector.empty()) return false;
    return matchesComplexSelector(selector, node);
}

bool SelectorMatcher::matchesComplexSelector(const std::string& selector, DOMNodePtr node) {
    if (!node || selector.empty()) return false;
    
    std::string trimmed = trimWhitespace(selector);
    
    // Find the last combinator in a way that ignores whitespace around explicit combinators.
    // Example: `.a > .b` should split on `>` (not on the trailing space after `>`).
    size_t last_combinator = std::string::npos;
    std::string combinator;

    auto is_space = [](char c) {
        return std::isspace(static_cast<unsigned char>(c)) != 0;
    };

    int paren_depth = 0;
    int bracket_depth = 0;

    size_t i = trimmed.size();
    while (i > 0) {
        char c = trimmed[i - 1];

        // Skip anything inside (...) or [...].
        if (c == ')') { ++paren_depth; --i; continue; }
        if (c == '(') { if (paren_depth > 0) --paren_depth; --i; continue; }
        if (c == ']') { ++bracket_depth; --i; continue; }
        if (c == '[') { if (bracket_depth > 0) --bracket_depth; --i; continue; }
        if (paren_depth > 0 || bracket_depth > 0) { --i; continue; }

        // Explicit combinators
        if (c == '>' || c == '+' || c == '~') {
            last_combinator = i - 1;
            combinator = std::string(1, c);
            break;
        }

        // Descendant combinator (whitespace), but ignore whitespace that is merely around an explicit combinator.
        if (is_space(c)) {
            const size_t run_end = i - 1;
            size_t run_start = run_end;
            while (run_start > 0 && is_space(trimmed[run_start - 1])) {
                --run_start;
            }

            // Find nearest non-space neighbors around this whitespace run.
            char prev = '\0';
            if (run_start > 0) {
                prev = trimmed[run_start - 1];
            }

            size_t next_pos = run_end + 1;
            while (next_pos < trimmed.size() && is_space(trimmed[next_pos])) {
                ++next_pos;
            }
            char next = '\0';
            if (next_pos < trimmed.size()) {
                next = trimmed[next_pos];
            }

            const bool adjacent_to_explicit =
                (prev == '>' || prev == '+' || prev == '~' || next == '>' || next == '+' || next == '~');

            if (!adjacent_to_explicit) {
                last_combinator = run_start;
                combinator = " ";
                break;
            }

            // Skip over this whitespace run and continue scanning left.
            i = run_start;
            continue;
        }

        --i;
    }

    if (last_combinator != std::string::npos) {
        std::string right_selector = trimWhitespace(trimmed.substr(last_combinator + 1));
        std::string left_selector = trimWhitespace(trimmed.substr(0, last_combinator));

        if (!matchesSimpleSelector(right_selector, node)) {
            return false;
        }

        auto parent = node->getParent();
        if (!parent) return false;

        if (combinator == " ") {
            return matchesDescendant(left_selector, parent);
        } else if (combinator == ">") {
            return matchesComplexSelector(left_selector, parent);
        } else if (combinator == "+") {
            return matchesAdjacentSibling(left_selector, node);
        } else if (combinator == "~") {
            return matchesGeneralSibling(left_selector, node);
        }
    }
    
    return matchesSimpleSelector(trimmed, node);
}

bool SelectorMatcher::matchesSimpleSelector(const std::string& selector, DOMNodePtr node) {
    std::string trimmed = trimWhitespace(selector);
    
    if (trimmed.empty() || !node) return false;
    
    size_t pos = 0;
    
    // Parse tag, class, id, attribute, pseudo selectors
    size_t dot_pos = trimmed.find('.');
    size_t hash_pos = trimmed.find('#');
    size_t bracket_pos = trimmed.find('[');
    size_t colon_pos = trimmed.find(':');
    
    // Find where tag name ends
    size_t tag_end = std::string::npos;
    if (dot_pos != std::string::npos) tag_end = std::min(tag_end, dot_pos);
    if (hash_pos != std::string::npos) tag_end = std::min(tag_end, hash_pos);
    if (bracket_pos != std::string::npos) tag_end = std::min(tag_end, bracket_pos);
    if (colon_pos != std::string::npos) tag_end = std::min(tag_end, colon_pos);
    
    // Check tag name
    if (trimmed[0] != '.' && trimmed[0] != '#' && trimmed[0] != '[' && trimmed[0] != ':') {
        std::string tag = tag_end != std::string::npos ? trimmed.substr(0, tag_end) : trimmed;
        if (tag != "*" && !matchesTagSelector(tag, node)) {
            return false;
        }
        pos = tag_end != std::string::npos ? tag_end : trimmed.length();
    }
    
    // Check remaining parts
    while (pos < trimmed.length()) {
        if (trimmed[pos] == '.') {
            // Class selector
            size_t next_pos = std::string::npos;
            for (size_t i = pos + 1; i < trimmed.length(); ++i) {
                if (trimmed[i] == '.' || trimmed[i] == '#' || trimmed[i] == '[' || trimmed[i] == ':') {
                    next_pos = i;
                    break;
                }
            }
            
            std::string cls = next_pos != std::string::npos ? 
                trimmed.substr(pos + 1, next_pos - pos - 1) : trimmed.substr(pos + 1);
            
            if (!matchesClassSelector(cls, node)) {
                return false;
            }
            
            pos = next_pos != std::string::npos ? next_pos : trimmed.length();
        } else if (trimmed[pos] == '#') {
            // ID selector
            size_t next_pos = std::string::npos;
            for (size_t i = pos + 1; i < trimmed.length(); ++i) {
                if (trimmed[i] == '.' || trimmed[i] == '#' || trimmed[i] == '[' || trimmed[i] == ':') {
                    next_pos = i;
                    break;
                }
            }
            
            std::string id = next_pos != std::string::npos ? 
                trimmed.substr(pos + 1, next_pos - pos - 1) : trimmed.substr(pos + 1);
            
            if (!matchesIdSelector(id, node)) {
                return false;
            }
            
            pos = next_pos != std::string::npos ? next_pos : trimmed.length();
        } else if (trimmed[pos] == '[') {
            // Attribute selector
            size_t bracket_close = trimmed.find(']', pos);
            if (bracket_close != std::string::npos) {
                std::string attr_sel = trimmed.substr(pos + 1, bracket_close - pos - 1);
                if (!matchesAttributeSelector(attr_sel, node)) {
                    return false;
                }
                pos = bracket_close + 1;
            } else {
                break;
            }
        } else if (trimmed[pos] == ':') {
            // Pseudo selector
            bool is_pseudo_element = (pos + 1 < trimmed.length() && trimmed[pos + 1] == ':');
            size_t start = is_pseudo_element ? pos + 2 : pos + 1;
            
            // Find end of pseudo selector
            size_t next_pos = start;
            int paren_depth = 0;
            while (next_pos < trimmed.length()) {
                if (trimmed[next_pos] == '(') {
                    ++paren_depth;
                } else if (trimmed[next_pos] == ')') {
                    --paren_depth;
                    if (paren_depth == 0) {
                        ++next_pos;
                        break;
                    }
                } else if (paren_depth == 0 && 
                           (trimmed[next_pos] == '.' || trimmed[next_pos] == '#' || 
                            trimmed[next_pos] == '[' || trimmed[next_pos] == ':')) {
                    break;
                }
                ++next_pos;
            }
            
            std::string pseudo = trimmed.substr(start, next_pos - start);
            
            if (is_pseudo_element) {
                if (!matchesPseudoElement(pseudo, node)) {
                    return false;
                }
            } else {
                if (!matchesPseudoClass(pseudo, node)) {
                    return false;
                }
            }
            
            pos = next_pos;
        } else {
            ++pos;
        }
    }
    
    return true;
}

bool SelectorMatcher::matchesTagSelector(const std::string& tag, DOMNodePtr node) {
    if (!node) return false;
    if (tag == "*") return true;
    return node->getTagName() == tag;
}

bool SelectorMatcher::matchesClassSelector(const std::string& cls, DOMNodePtr node) {
    if (!node || !node->hasAttribute("class")) return false;
    
    std::string classes = node->getAttribute("class");
    std::istringstream iss(classes);
    std::string class_name;
    
    while (iss >> class_name) {
        if (class_name == cls) return true;
    }
    
    return false;
}

bool SelectorMatcher::matchesIdSelector(const std::string& id, DOMNodePtr node) {
    if (!node || !node->hasAttribute("id")) return false;
    return node->getAttribute("id") == id;
}

bool SelectorMatcher::matchesAttributeSelector(const std::string& attr_sel, DOMNodePtr node) {
    if (!node) return false;
    
    size_t eq_pos = attr_sel.find('=');
    
    if (eq_pos == std::string::npos) {
        // Just [attr]
        std::string attr_name = trimWhitespace(attr_sel);
        return node->hasAttribute(attr_name);
    }
    
    // Determine operator
    std::string op = "=";
    size_t attr_end = eq_pos;
    
    if (eq_pos > 0) {
        char prev = attr_sel[eq_pos - 1];
        if (prev == '~' || prev == '|' || prev == '^' || prev == '$' || prev == '*') {
            op = std::string(1, prev) + "=";
            attr_end = eq_pos - 1;
        }
    }
    
    std::string attr_name = trimWhitespace(attr_sel.substr(0, attr_end));
    
    if (!node->hasAttribute(attr_name)) return false;
    
    std::string attr_value = node->getAttribute(attr_name);
    
    // Extract expected value (remove quotes)
    size_t val_start = eq_pos + 1;
    while (val_start < attr_sel.length() && 
           (attr_sel[val_start] == ' ' || attr_sel[val_start] == '"' || attr_sel[val_start] == '\'')) {
        ++val_start;
    }
    size_t val_end = attr_sel.length();
    while (val_end > val_start && 
           (attr_sel[val_end - 1] == ' ' || attr_sel[val_end - 1] == '"' || attr_sel[val_end - 1] == '\'')) {
        --val_end;
    }
    std::string expected_value = attr_sel.substr(val_start, val_end - val_start);
    
    if (op == "=") {
        return attr_value == expected_value;
    } else if (op == "~=") {
        std::istringstream iss(attr_value);
        std::string word;
        while (iss >> word) {
            if (word == expected_value) return true;
        }
        return false;
    } else if (op == "|=") {
        return attr_value == expected_value || 
               (attr_value.length() > expected_value.length() &&
                attr_value.substr(0, expected_value.length() + 1) == expected_value + "-");
    } else if (op == "^=") {
        return attr_value.find(expected_value) == 0;
    } else if (op == "$=") {
        return attr_value.length() >= expected_value.length() &&
               attr_value.substr(attr_value.length() - expected_value.length()) == expected_value;
    } else if (op == "*=") {
        return attr_value.find(expected_value) != std::string::npos;
    }
    
    return false;
}

bool SelectorMatcher::matchesPseudoClass(const std::string& pseudo, DOMNodePtr node) {
    if (!node) return false;
    
    // Extract argument if present
    std::string name = pseudo;
    std::string arg;
    size_t paren_pos = pseudo.find('(');
    if (paren_pos != std::string::npos) {
        name = pseudo.substr(0, paren_pos);
        size_t close_paren = pseudo.rfind(')');
        if (close_paren != std::string::npos && close_paren > paren_pos) {
            arg = pseudo.substr(paren_pos + 1, close_paren - paren_pos - 1);
        }
    }
    
    // Structural pseudo-classes
    if (name == "root") {
        return node->getParent() == nullptr;
    } else if (name == "first-child") {
        auto parent = node->getParent();
        if (!parent) return false;
        const auto& children = parent->getChildren();
        // Find first element child
        for (const auto& child : children) {
            if (child->getType() == DOMNode::NodeType::ELEMENT) {
                return child == node;
            }
        }
        return false;
    } else if (name == "last-child") {
        auto parent = node->getParent();
        if (!parent) return false;
        const auto& children = parent->getChildren();
        // Find last element child
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->getType() == DOMNode::NodeType::ELEMENT) {
                return *it == node;
            }
        }
        return false;
    } else if (name == "only-child") {
        auto parent = node->getParent();
        if (!parent) return false;
        const auto& children = parent->getChildren();
        int element_count = 0;
        for (const auto& child : children) {
            if (child->getType() == DOMNode::NodeType::ELEMENT) {
                ++element_count;
                if (element_count > 1) return false;
            }
        }
        return element_count == 1;
    } else if (name == "nth-child") {
        return matchesNthChild(arg, node);
    } else if (name == "nth-last-child") {
        return matchesNthLastChild(arg, node);
    } else if (name == "nth-of-type") {
        return matchesNthOfType(arg, node);
    } else if (name == "nth-last-of-type") {
        return matchesNthLastOfType(arg, node);
    } else if (name == "first-of-type") {
        auto parent = node->getParent();
        if (!parent) return false;
        const auto& children = parent->getChildren();
        std::string tag = node->getTagName();
        for (const auto& child : children) {
            if (child->getTagName() == tag) {
                return child == node;
            }
        }
        return false;
    } else if (name == "last-of-type") {
        auto parent = node->getParent();
        if (!parent) return false;
        const auto& children = parent->getChildren();
        std::string tag = node->getTagName();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            if ((*it)->getTagName() == tag) {
                return *it == node;
            }
        }
        return false;
    } else if (name == "only-of-type") {
        auto parent = node->getParent();
        if (!parent) return false;
        const auto& children = parent->getChildren();
        std::string tag = node->getTagName();
        int count = 0;
        for (const auto& child : children) {
            if (child->getTagName() == tag) {
                ++count;
                if (count > 1) return false;
            }
        }
        return count == 1;
    } else if (name == "empty") {
        const auto& children = node->getChildren();
        for (const auto& child : children) {
            if (child->getType() == DOMNode::NodeType::ELEMENT) {
                return false;
            }
            if (child->getType() == DOMNode::NodeType::TEXT && 
                !child->getTextContent().empty()) {
                return false;
            }
        }
        return true;
    }
    // Negation and logical pseudo-classes
    else if (name == "not") {
        return matchesNot(arg, node);
    } else if (name == "is" || name == "matches") {
        return matchesIs(arg, node);
    } else if (name == "where") {
        return matchesWhere(arg, node);
    } else if (name == "has") {
        return matchesHas(arg, node);
    }
    // Form pseudo-classes
    else if (name == "checked") {
        return node->hasAttribute("checked");
    } else if (name == "disabled") {
        return node->hasAttribute("disabled");
    } else if (name == "enabled") {
        return !node->hasAttribute("disabled");
    } else if (name == "required") {
        return node->hasAttribute("required");
    } else if (name == "optional") {
        return !node->hasAttribute("required");
    } else if (name == "read-only") {
        return node->hasAttribute("readonly");
    } else if (name == "read-write") {
        return !node->hasAttribute("readonly");
    } else if (name == "placeholder-shown") {
        // Check if input has placeholder and value is empty
        if (node->hasAttribute("placeholder")) {
            std::string value = node->getAttribute("value");
            return value.empty();
        }
        return false;
    } else if (name == "indeterminate") {
        // :indeterminate matches checkboxes/radios in indeterminate state
        // Checkbox: has indeterminate attribute
        // Radio: no radio in group is selected
        std::string tag = node->getTagName();
        if (tag == "input") {
            std::string type = node->getAttribute("type");
            if (type == "checkbox") {
                return node->hasAttribute("indeterminate");
            } else if (type == "radio") {
                // For radio groups, check if no radio in group is selected
                std::string name = node->getAttribute("name");
                if (name.empty()) return false;

                auto parent = node->getParent();
                if (!parent) return false;

                // Check if any radio in same group is checked
                bool found_checked = false;
                std::vector<DOMNodePtr> nodes_to_check;
                nodes_to_check.push_back(parent);

                while (!nodes_to_check.empty()) {
                    auto current = nodes_to_check.back();
                    nodes_to_check.pop_back();

                    for (const auto& child : current->getChildren()) {
                        if (child->getTagName() == "input") {
                            std::string child_type = child->getAttribute("type");
                            std::string child_name = child->getAttribute("name");
                            if (child_type == "radio" && child_name == name && child->hasAttribute("checked")) {
                                found_checked = true;
                                break;
                            }
                        }
                        nodes_to_check.push_back(child);
                    }
                    if (found_checked) break;
                }
                return !found_checked;
            }
        }
        return false;
    }
    // Directional pseudo-class (:dir())
    else if (name == "dir") {
        // :dir(ltr) or :dir(rtl) matches based on the element's resolved direction
        // The direction is inherited, so this uses the getEffectiveDirection() method
        std::string effective_dir = node->getEffectiveDirection();
        return arg == effective_dir;
    }
    // Link pseudo-classes
    else if (name == "link") {
        return node->getTagName() == "a" && node->hasAttribute("href");
    } else if (name == "any-link") {
        // :any-link matches any element that would be matched by :link or :visited
        // This includes <a>, <area>, <link> elements with href attribute
        std::string tag = node->getTagName();
        return (tag == "a" || tag == "area" || tag == "link") && node->hasAttribute("href");
    } else if (name == "target") {
        // :target matches the element whose ID matches the URL fragment identifier
        if (node->hasAttribute("id")) {
            std::string id = node->getAttribute("id");
            std::string current_fragment = DOMNode::getCurrentFragment();
            return id == current_fragment;
        }
        return false;
    }
    // Focus/interaction pseudo-classes (runtime state)
    else if (name == "hover") {
        return node->isHovered();
    } else if (name == "active") {
        return node->isActive();
    } else if (name == "focus") {
        return node->isFocused();
    } else if (name == "focus-visible") {
        // :focus-visible matches when element is focused and should show focus ring
        // Heuristic: keyboard focus shows ring, mouse click typically doesn't
        return node->isFocusVisible();
    } else if (name == "focus-within") {
        // :focus-within matches if element or any descendant has focus
        if (node->isFocused()) return true;
        // Check descendants
        std::function<bool(DOMNodePtr)> checkDescendants = [&](DOMNodePtr n) -> bool {
            for (const auto& child : n->getChildren()) {
                if (child->isFocused() || checkDescendants(child)) {
                    return true;
                }
            }
            return false;
        };
        return checkDescendants(node);
    }

    // Valid/invalid pseudo-classes
    else if (name == "valid") {
        return isElementValid(node);
    } else if (name == "invalid") {
        return !isElementValid(node);
    }
    // :open / :closed pseudo-classes for <details> element
    else if (name == "open") {
        // :open matches <details> elements with the open attribute
        return node->getTagName() == "details" && node->hasAttribute("open");
    } else if (name == "closed") {
        // :closed matches <details> elements without the open attribute
        return node->getTagName() == "details" && !node->hasAttribute("open");
    }

    return false;
}

bool SelectorMatcher::matchesPseudoElement(const std::string& pseudo, DOMNodePtr node) {
    // Pseudo-elements are handled during rendering, not matching
    // Return true to allow the selector to match
    if (pseudo == "before" || pseudo == "after" || 
        pseudo == "first-line" || pseudo == "first-letter" ||
        pseudo == "placeholder" || pseudo == "selection") {
        return true;
    }
    return false;
}

bool SelectorMatcher::matchesDescendant(const std::string& ancestor_sel, DOMNodePtr node) {
    DOMNodePtr current = node;
    while (current) {
        if (matchesComplexSelector(ancestor_sel, current)) {
            return true;
        }
        current = current->getParent();
    }
    return false;
}

bool SelectorMatcher::matchesAdjacentSibling(const std::string& prev_sel, DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return false;
    
    const auto& siblings = parent->getChildren();
    DOMNodePtr prev_element = nullptr;
    
    for (const auto& sibling : siblings) {
        if (sibling == node) {
            if (prev_element && matchesComplexSelector(prev_sel, prev_element)) {
                return true;
            }
            return false;
        }
        if (sibling->getType() == DOMNode::NodeType::ELEMENT) {
            prev_element = sibling;
        }
    }
    return false;
}

bool SelectorMatcher::matchesGeneralSibling(const std::string& prev_sel, DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return false;
    
    const auto& siblings = parent->getChildren();
    
    for (const auto& sibling : siblings) {
        if (sibling == node) {
            return false;  // No matching previous sibling found
        }
        if (sibling->getType() == DOMNode::NodeType::ELEMENT && 
            matchesComplexSelector(prev_sel, sibling)) {
            return true;
        }
    }
    return false;
}

std::pair<int, int> SelectorMatcher::parseNthExpression(const std::string& expr) {
    std::string e = expr;
    std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    e.erase(std::remove_if(e.begin(), e.end(), [](unsigned char c) {
        return std::isspace(c);
    }), e.end());
    
    if (e == "odd") return {2, 1};
    if (e == "even") return {2, 0};
    
    int a = 0, b = 0;
    
    size_t n_pos = e.find('n');
    if (n_pos != std::string::npos) {
        if (n_pos == 0) {
            a = 1;
        } else if (n_pos == 1 && e[0] == '-') {
            a = -1;
        } else if (n_pos == 1 && e[0] == '+') {
            a = 1;
        } else {
            try {
                a = std::stoi(e.substr(0, n_pos));
            } catch (...) {
                a = 1;
            }
        }
        
        if (n_pos + 1 < e.length()) {
            try {
                b = std::stoi(e.substr(n_pos + 1));
            } catch (...) {
                b = 0;
            }
        }
    } else {
        try {
            b = std::stoi(e);
        } catch (...) {
            b = 0;
        }
    }
    
    return {a, b};
}

bool SelectorMatcher::matchesNthChild(const std::string& arg, DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return false;
    
    auto [a, b] = parseNthExpression(arg);
    
    const auto& children = parent->getChildren();
    int index = 1;
    for (const auto& child : children) {
        if (child->getType() == DOMNode::NodeType::ELEMENT) {
            if (child == node) {
                if (a == 0) {
                    return index == b;
                }
                return (index - b) % a == 0 && (index - b) / a >= 0;
            }
            ++index;
        }
    }
    return false;
}

bool SelectorMatcher::matchesNthLastChild(const std::string& arg, DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return false;
    
    auto [a, b] = parseNthExpression(arg);
    
    const auto& children = parent->getChildren();
    int total = 0;
    int node_index = 0;
    int current = 0;
    
    for (const auto& child : children) {
        if (child->getType() == DOMNode::NodeType::ELEMENT) {
            ++total;
            ++current;
            if (child == node) {
                node_index = current;
            }
        }
    }
    
    int index = total - node_index + 1;
    
    if (a == 0) {
        return index == b;
    }
    return (index - b) % a == 0 && (index - b) / a >= 0;
}

bool SelectorMatcher::matchesNthOfType(const std::string& arg, DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return false;

    auto [a, b] = parseNthExpression(arg);

    const auto& children = parent->getChildren();
    std::string tag = node->getTagName();
    int index = 1;

    for (const auto& child : children) {
        if (child->getTagName() == tag) {
            if (child == node) {
                if (a == 0) {
                    return index == b;
                }
                return (index - b) % a == 0 && (index - b) / a >= 0;
            }
            ++index;
        }
    }
    return false;
}

bool SelectorMatcher::matchesNthLastOfType(const std::string& arg, DOMNodePtr node) {
    auto parent = node->getParent();
    if (!parent) return false;

    auto [a, b] = parseNthExpression(arg);

    const auto& children = parent->getChildren();
    std::string tag = node->getTagName();

    // First count total elements of same type
    int total = 0;
    for (const auto& child : children) {
        if (child->getTagName() == tag) {
            ++total;
        }
    }

    // Now find the reverse index
    int reverse_index = 0;
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->getTagName() == tag) {
            ++reverse_index;
            if (*it == node) {
                if (a == 0) {
                    return reverse_index == b;
                }
                return (reverse_index - b) % a == 0 && (reverse_index - b) / a >= 0;
            }
        }
    }
    return false;
}

bool SelectorMatcher::matchesNot(const std::string& arg, DOMNodePtr node) {
    return !matchesComplexSelector(arg, node);
}

bool SelectorMatcher::matchesIs(const std::string& arg, DOMNodePtr node) {
    // Split by comma
    std::vector<std::string> selectors;
    std::string current;
    int paren_depth = 0;
    
    for (char c : arg) {
        if (c == '(') {
            ++paren_depth;
            current += c;
        } else if (c == ')') {
            --paren_depth;
            current += c;
        } else if (c == ',' && paren_depth == 0) {
            std::string trimmed = trimWhitespace(current);
            if (!trimmed.empty()) {
                selectors.push_back(trimmed);
            }
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        std::string trimmed = trimWhitespace(current);
        if (!trimmed.empty()) {
            selectors.push_back(trimmed);
        }
    }
    
    for (const auto& sel : selectors) {
        if (matchesComplexSelector(sel, node)) {
            return true;
        }
    }
    return false;
}

bool SelectorMatcher::matchesWhere(const std::string& arg, DOMNodePtr node) {
    // :where() is like :is() but with zero specificity
    return matchesIs(arg, node);
}

bool SelectorMatcher::matchesHas(const std::string& arg, DOMNodePtr node) {
    // :has() matches if any descendant matches the argument
    std::vector<DOMNodePtr> descendants;
    std::function<void(DOMNodePtr)> collect = [&](DOMNodePtr n) {
        for (const auto& child : n->getChildren()) {
            if (child->getType() == DOMNode::NodeType::ELEMENT) {
                descendants.push_back(child);
                collect(child);
            }
        }
    };
    collect(node);
    
    for (const auto& desc : descendants) {
        if (matchesComplexSelector(arg, desc)) {
            return true;
        }
    }
    return false;
}

std::vector<DOMNodePtr> SelectorMatcher::querySelectorAll(const std::string& selector, DOMNodePtr root) {
    std::vector<DOMNodePtr> result;
    if (!root) return result;
    
    std::function<void(DOMNodePtr)> traverse = [&](DOMNodePtr node) {
        if (node->getType() == DOMNode::NodeType::ELEMENT && matches(selector, node)) {
            result.push_back(node);
        }
        for (const auto& child : node->getChildren()) {
            traverse(child);
        }
    };
    
    for (const auto& child : root->getChildren()) {
        traverse(child);
    }
    
    return result;
}

DOMNodePtr SelectorMatcher::querySelector(const std::string& selector, DOMNodePtr root) {
    if (!root) return nullptr;
    
    std::function<DOMNodePtr(DOMNodePtr)> traverse = [&](DOMNodePtr node) -> DOMNodePtr {
        if (node->getType() == DOMNode::NodeType::ELEMENT && matches(selector, node)) {
            return node;
        }
        for (const auto& child : node->getChildren()) {
            auto result = traverse(child);
            if (result) return result;
        }
        return nullptr;
    };
    
    for (const auto& child : root->getChildren()) {
        auto result = traverse(child);
        if (result) return result;
    }
    
    return nullptr;
}

bool SelectorMatcher::elementMatches(const std::string& selector, DOMNodePtr element) {
    return matches(selector, element);
}

DOMNodePtr SelectorMatcher::closest(const std::string& selector, DOMNodePtr element) {
    DOMNodePtr current = element;
    while (current) {
        if (matches(selector, current)) {
            return current;
        }
        current = current->getParent();
    }
    return nullptr;
}


} // namespace dong::dom
