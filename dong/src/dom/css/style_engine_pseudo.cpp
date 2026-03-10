#include "style_engine.hpp"
#include "style_engine_internal.hpp"
#include "../dom/dom_node.hpp"
#include <algorithm>

namespace dong::dom {

static bool hasGeneratedContent(const ComputedStyle& style) {
    if (!style.isExplicitlySet("content")) {
        return false;
    }
    // `content: none/normal` clears content_raw/tokens.
    if (!style.content_raw.empty()) return true;
    if (!style.content_tokens.empty()) return true;
    // Back-compat: literal-only string content may end up here.
    return !style.content.empty();
}

void StyleEngine::processPseudoElements(DOMNodePtr node) {
    if (!node || node->getType() != DOMNode::NodeType::ELEMENT) return;

    // Pseudo-element cascade must respect specificity + source order.
    // The previous implementation applied rules in discovery order, which is wrong for cases like:
    //   ol.nested ol > li::before  (more specific)
    //   .q::before                (less specific but later)
    // Browsers keep the more specific rule.

    struct PseudoRuleRef {
        const CSSRule* rule = nullptr;
    };

    auto extractBaseSelector = [&](const std::string& selector, const char* pseudo_a, const char* pseudo_b) -> std::string {
        size_t pos = selector.find(pseudo_a);
        if (pos == std::string::npos) pos = selector.find(pseudo_b);
        if (pos == std::string::npos) return {};
        return trimWhitespace(selector.substr(0, pos));
    };

    auto ruleLess = [](const PseudoRuleRef& a, const PseudoRuleRef& b) {
        if (a.rule->specificity != b.rule->specificity) return a.rule->specificity < b.rule->specificity;
        return a.rule->source_order < b.rule->source_order;
    };

    std::vector<PseudoRuleRef> before_rules;
    std::vector<PseudoRuleRef> after_rules;
    std::vector<PseudoRuleRef> marker_rules;
    std::vector<PseudoRuleRef> placeholder_rules;
    std::vector<PseudoRuleRef> selection_rules;

    for (const auto& sheet : stylesheets_) {
        for (const auto& rule : sheet.getRules()) {
            // ::before
            if (rule.selector.find("::before") != std::string::npos || rule.selector.find(":before") != std::string::npos) {
                std::string base_selector = extractBaseSelector(rule.selector, "::before", ":before");
                if (!base_selector.empty() && matcher_.matches(base_selector, node)) {
                    before_rules.push_back(PseudoRuleRef{&rule});
                }
            }

            // ::after
            if (rule.selector.find("::after") != std::string::npos || rule.selector.find(":after") != std::string::npos) {
                std::string base_selector = extractBaseSelector(rule.selector, "::after", ":after");
                if (!base_selector.empty() && matcher_.matches(base_selector, node)) {
                    after_rules.push_back(PseudoRuleRef{&rule});
                }
            }

            // ::marker
            if (node->getTagName() == "li" &&
                (rule.selector.find("::marker") != std::string::npos || rule.selector.find(":marker") != std::string::npos)) {
                std::string base_selector = extractBaseSelector(rule.selector, "::marker", ":marker");
                if (!base_selector.empty() && matcher_.matches(base_selector, node)) {
                    marker_rules.push_back(PseudoRuleRef{&rule});
                }
            }

            // ::placeholder
            if ((node->getTagName() == "input" || node->getTagName() == "textarea") &&
                (rule.selector.find("::placeholder") != std::string::npos || rule.selector.find(":placeholder") != std::string::npos)) {
                std::string base_selector = extractBaseSelector(rule.selector, "::placeholder", ":placeholder");
                if (!base_selector.empty() && matcher_.matches(base_selector, node)) {
                    placeholder_rules.push_back(PseudoRuleRef{&rule});
                }
            }

            // ::selection
            if (rule.selector.find("::selection") != std::string::npos || rule.selector.find(":selection") != std::string::npos) {
                std::string base_selector = extractBaseSelector(rule.selector, "::selection", ":selection");
                if (!base_selector.empty() && matcher_.matches(base_selector, node)) {
                    selection_rules.push_back(PseudoRuleRef{&rule});
                }
            }
        }
    }

    auto buildPseudoStyle = [&](const std::vector<PseudoRuleRef>& rules) -> std::pair<bool, ComputedStyle> {
        if (rules.empty()) return {false, ComputedStyle{}};

        std::vector<PseudoRuleRef> sorted = rules;
        std::sort(sorted.begin(), sorted.end(), ruleLess);

        // Pseudo-elements inherit from their originating element.
        ComputedStyle s = node->getComputedStyle();
        // But counters are not inherited; keep them off the pseudo so we don't double-apply.
        s.counter_resets.clear();
        s.counter_increments.clear();
        // Clear inherited explicitly_set_properties_ so that only the pseudo rule's own
        // properties are considered explicitly set. This prevents inherited background-color
        // etc. from blocking inline rendering of ::before/::after content.
        s.clearExplicitlySet();

        for (const auto& rr : sorted) {
            style_engine_internal::applyRuleProperties(rr.rule->style, s);
        }
        return {true, s};
    };

    bool has_before = false;
    bool has_after = false;
    bool has_marker = !marker_rules.empty();
    bool has_placeholder = !placeholder_rules.empty();
    bool has_selection = !selection_rules.empty();
    ComputedStyle before_style;
    ComputedStyle after_style;
    ComputedStyle selection_style;

    {
        auto r = buildPseudoStyle(before_rules);
        has_before = r.first;
        if (has_before) before_style = std::move(r.second);
    }
    {
        auto r = buildPseudoStyle(after_rules);
        has_after = r.first;
        if (has_after) after_style = std::move(r.second);
    }
    {
        auto r = buildPseudoStyle(selection_rules);
        has_selection = r.first;
        if (has_selection) selection_style = std::move(r.second);
    }

    // UA stylesheet: <q> elements get open-quote/close-quote pseudo-elements by default.
    // If no explicit ::before/::after rule is defined for a <q> element, inject UA defaults.
    if (node->getTagName() == "q") {
        if (!has_before) {
            before_style = node->getComputedStyle();
            before_style.clearExplicitlySet();
            before_style.counter_resets.clear();
            before_style.counter_increments.clear();
            ComputedStyle::ContentToken t;
            t.type = ComputedStyle::ContentToken::Type::OpenQuote;
            before_style.content_tokens = {t};
            before_style.markExplicitlySet("content");
            has_before = true;
        }
        if (!has_after) {
            after_style = node->getComputedStyle();
            after_style.clearExplicitlySet();
            after_style.counter_resets.clear();
            after_style.counter_increments.clear();
            ComputedStyle::ContentToken t;
            t.type = ComputedStyle::ContentToken::Type::CloseQuote;
            after_style.content_tokens = {t};
            after_style.markExplicitlySet("content");
            has_after = true;
        }
    }

    // Create ::before pseudo-element if needed
    // NOTE: `content` may be token-based (counter()/open-quote/etc) and not reflected in `style.content`.
    // Use token-aware detection so generated content actually appears.
    if (has_before && hasGeneratedContent(before_style)) {
        // Pseudo-elements default to inline in CSS.
        before_style.setDisplay("inline");
        style_engine_internal::applyLogicalProperties(before_style);
        auto pseudo = createPseudoElement(node, "before");
        if (pseudo) {
            pseudo->getComputedStyle() = before_style;
            pseudo->getComputedStyle().is_pseudo_element = true;
            pseudo->getComputedStyle().pseudo_type = "before";

            // Best-effort textual representation (rendering uses computed tokens).
            pseudo->setTextContent(!before_style.content.empty() ? before_style.content : before_style.content_raw);

            node->setPseudoBefore(pseudo);
        }
    } else {
        node->setPseudoBefore(nullptr);
    }


    // Create ::after pseudo-element if needed
    if (has_after && hasGeneratedContent(after_style)) {
        // Pseudo-elements default to inline in CSS.
        after_style.setDisplay("inline");
        style_engine_internal::applyLogicalProperties(after_style);
        auto pseudo = createPseudoElement(node, "after");

        if (pseudo) {
            pseudo->getComputedStyle() = after_style;
            pseudo->getComputedStyle().is_pseudo_element = true;
            pseudo->getComputedStyle().pseudo_type = "after";
            node->setPseudoAfter(pseudo);
        }
    } else {
        node->setPseudoAfter(nullptr);
    }


    // Create ::marker pseudo-element for li elements.
    // Unlike ::before/::after, ::marker is created automatically for all <li> elements
    // (unless list-style-type is "none"). Explicit ::marker CSS rules override defaults.
    if (node->getTagName() == "li") {
        const auto& node_style = node->getComputedStyle();
        const bool needs_marker = (node_style.list_style_type != "none");
        if (needs_marker) {
            auto pseudo = createPseudoElement(node, "marker");
            if (pseudo) {
                pseudo->getComputedStyle() = node_style;
                if (has_marker) {
                    std::vector<PseudoRuleRef> sorted = marker_rules;
                    std::sort(sorted.begin(), sorted.end(), ruleLess);
                    for (const auto& rr : sorted) {
                        style_engine_internal::applyRuleProperties(rr.rule->style, pseudo->getComputedStyle());
                    }
                }

                pseudo->getComputedStyle().is_pseudo_element = true;
                pseudo->getComputedStyle().pseudo_type = "marker";
                node->setPseudoMarker(pseudo);
            }
        } else {
            node->setPseudoMarker(nullptr);
        }
    }

    // Create ::placeholder pseudo-element for input and textarea elements.
    // ::placeholder is shown when the element has no content (empty value).
    if (node->getTagName() == "input" || node->getTagName() == "textarea") {
        const bool has_placeholder_attr = node->hasAttribute("placeholder");
        const bool value_is_empty = !node->hasAttribute("value") || node->getAttribute("value").empty();

        if (has_placeholder_attr && value_is_empty) {
            auto pseudo = createPseudoElement(node, "placeholder");
            if (pseudo) {
                // Start with parent's styles for inheritance
                pseudo->getComputedStyle() = node->getComputedStyle();
                // Apply explicit ::placeholder rules (with proper cascade)
                if (has_placeholder) {
                    std::vector<PseudoRuleRef> sorted = placeholder_rules;
                    std::sort(sorted.begin(), sorted.end(), ruleLess);
                    for (const auto& rr : sorted) {
                        style_engine_internal::applyRuleProperties(rr.rule->style, pseudo->getComputedStyle());
                    }
                }

                // Set placeholder text as content
                pseudo->getComputedStyle().is_pseudo_element = true;
                pseudo->getComputedStyle().pseudo_type = "placeholder";
                pseudo->getComputedStyle().content = node->getAttribute("placeholder");
                pseudo->setTextContent(node->getAttribute("placeholder"));
                node->setPseudoPlaceholder(pseudo);
            }
        } else {
            node->setPseudoPlaceholder(nullptr);
        }
    }

    // Create ::selection pseudo-element for text selection styling
    // ::selection is automatically created for all elements that support text selection
    if (has_selection) {
        auto pseudo = createPseudoElement(node, "selection");
        if (pseudo) {
            // Start with parent's styles for inheritance
            pseudo->getComputedStyle() = selection_style;
            pseudo->getComputedStyle().is_pseudo_element = true;
            pseudo->getComputedStyle().pseudo_type = "selection";
            node->setPseudoSelection(pseudo);
        }
    } else {
        node->setPseudoSelection(nullptr);
    }
}

DOMNodePtr StyleEngine::createPseudoElement(DOMNodePtr parent, const std::string& pseudo_type) {
    if (!parent) return nullptr;

    auto pseudo = std::make_shared<DOMNode>(DOMNode::NodeType::ELEMENT, "::" + pseudo_type);

    // Pseudo-elements inherit from their originating element.
    // Note: pseudo nodes are not inserted into the DOM tree's children list, so we cannot
    // rely on DOMNode::getParent() inside inheritFromParent().
    pseudo->getComputedStyle() = parent->getComputedStyle();

    return pseudo;
}

// 优化策略3：重建规则索�?

} // namespace dong::dom