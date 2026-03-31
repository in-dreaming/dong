#include "scene_compiler.hpp"
#include "../core/log.h"
#include "../render/painter/painter_style_utils.hpp"

namespace dong::dom {

using dong::render::painter_detail::makeColorFromCss;

// --- Detection ---

static bool findMetaSceneMode(const DOMNodePtr& node, int depth = 0) {
    if (!node || depth > 4) return false;
    if (node->getTagName() == "meta") {
        if (node->getAttribute("name") == "dong-render-mode" &&
            node->getAttribute("content") == "scene") {
            return true;
        }
    }
    for (auto& child : node->getChildren()) {
        if (findMetaSceneMode(child, depth + 1)) return true;
    }
    return false;
}

bool SceneCompiler::canCompile(const DOMNodePtr& root) {
    if (!root) return false;

    if (findMetaSceneMode(root)) {
        DONG_LOG_INFO("[SceneCompiler] Forced scene mode via meta tag");
        return true;
    }

    return false;
}

// --- Compilation (reads ComputedStyle populated by StyleEngine) ---

static std::string collectDirectText(const DOMNodePtr& node) {
    std::string result;
    for (auto& child : node->getChildren()) {
        if (child && child->getNodeType() == DOMNode::NodeType::TEXT) {
            auto t = child->getRawTextContent();
            size_t start = t.find_first_not_of(" \t\n\r");
            size_t end = t.find_last_not_of(" \t\n\r");
            if (start != std::string::npos && end != std::string::npos) {
                if (!result.empty()) result += " ";
                result += t.substr(start, end - start + 1);
            }
        }
    }
    return result;
}

static float resolveLength(const CSSValue& v, float fallback = 0) {
    if (v.isPixel()) return v.value;
    return fallback;
}

static float effectiveBorderWidth(const ComputedStyle& cs, float side_width, const std::string& side_style) {
    const std::string& st = !side_style.empty() ? side_style : cs.border_style;
    if (st == "none" || st == "hidden") return 0.0f;
    return (side_width >= 0.0f) ? side_width : std::max(0.0f, cs.border_width);
}

void SceneCompiler::compileNode(const DOMNodePtr& node, render::SceneGraph& sg,
                                uint32_t parent_id, float parent_x, float parent_y) {
    if (!node) return;
    if (node->getNodeType() != DOMNode::NodeType::ELEMENT) return;

    auto tag = node->getTagName();
    if (tag == "html" || tag == "head" || tag == "meta" || tag == "title" ||
        tag == "script" || tag == "link" || tag == "style") {
        if (tag == "html" || tag == "head") {
            for (auto& child : node->getChildren()) {
                compileNode(child, sg, parent_id, parent_x, parent_y);
            }
        }
        return;
    }

    const auto& cs = node->getComputedStyle();

    if (cs.display == "none") return;

    render::SceneNode sn;
    sn.name = node->getAttribute("id");

    float x = resolveLength(cs.left);
    float y = resolveLength(cs.top);
    float w = resolveLength(cs.width);
    float h = resolveLength(cs.height);

    // Background
    sn.background_color = makeColorFromCss(cs.background_color);

    // Uniform border
    sn.border_width = cs.border_width;
    sn.border_color = makeColorFromCss(cs.border_color);

    // Per-side border
    float bt = effectiveBorderWidth(cs, cs.border_top_width, cs.border_top_style);
    float br = effectiveBorderWidth(cs, cs.border_right_width, cs.border_right_style);
    float bb = effectiveBorderWidth(cs, cs.border_bottom_width, cs.border_bottom_style);
    float bl = effectiveBorderWidth(cs, cs.border_left_width, cs.border_left_style);

    sn.border_top_width = bt;
    sn.border_right_width = br;
    sn.border_bottom_width = bb;
    sn.border_left_width = bl;

    // content-box: borders (and padding) expand outside declared dimensions
    if (cs.box_sizing != "border-box") {
        float pl = resolveLength(cs.padding_left);
        float pr = resolveLength(cs.padding_right);
        float pt = resolveLength(cs.padding_top);
        float pb = resolveLength(cs.padding_bottom);
        w += bl + br + pl + pr;
        h += bt + bb + pt + pb;
    }

    sn.x = parent_x + x;
    sn.y = parent_y + y;
    sn.width = w;
    sn.height = h;

    auto sideColor = [&](const std::string& side_color) -> render::Color {
        if (!side_color.empty()) return makeColorFromCss(side_color);
        return sn.border_color;
    };
    sn.border_top_color = sideColor(cs.border_top_color);
    sn.border_right_color = sideColor(cs.border_right_color);
    sn.border_bottom_color = sideColor(cs.border_bottom_color);
    sn.border_left_color = sideColor(cs.border_left_color);

    sn.border_radius = cs.border_radius;
    sn.opacity = cs.opacity;

    // Text properties
    sn.text_color = makeColorFromCss(cs.color);
    sn.font_size = cs.font_size;
    sn.font_family = cs.font_family;
    sn.font_weight = cs.font_weight;
    sn.text_align = cs.text_align;
    sn.line_height = cs.has_line_height ? cs.line_height : 0;

    sn.text = collectDirectText(node);
    sn.parent = parent_id;

    float abs_x = sn.x;
    float abs_y = sn.y;

    uint32_t my_id = sg.addNode(std::move(sn));

    for (auto& child : node->getChildren()) {
        if (child && child->getNodeType() == DOMNode::NodeType::ELEMENT) {
            compileNode(child, sg, my_id, abs_x, abs_y);
        }
    }
}

bool SceneCompiler::compile(const DOMNodePtr& root, render::SceneGraph& sg) {
    if (!root) return false;

    sg.clear();

    DOMNodePtr body;
    std::function<void(const DOMNodePtr&)> findBody = [&](const DOMNodePtr& n) {
        if (!n) return;
        if (n->getTagName() == "body") { body = n; return; }
        for (auto& c : n->getChildren()) {
            if (body) return;
            findBody(c);
        }
    };
    findBody(root);

    if (!body) {
        DONG_LOG_WARN("[SceneCompiler] No <body> found");
        return false;
    }

    const auto& body_cs = body->getComputedStyle();
    float bw = resolveLength(body_cs.width);
    float bh = resolveLength(body_cs.height);

    uint32_t body_node_id = UINT32_MAX;
    if (bw > 0 && bh > 0) {
        render::SceneNode bg;
        bg.name = "__body__";
        bg.x = 0; bg.y = 0; bg.width = bw; bg.height = bh;
        bg.background_color = makeColorFromCss(body_cs.background_color);
        body_node_id = sg.addNode(std::move(bg));
    }

    for (auto& child : body->getChildren()) {
        compileNode(child, sg, body_node_id, 0, 0);
    }

    DONG_LOG_INFO("[SceneCompiler] Compiled %zu nodes from DOM", sg.nodeCount());
    return sg.nodeCount() > 0;
}

} // namespace dong::dom
