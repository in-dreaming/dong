#include "scene_compiler.hpp"
#include "../core/log.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace dong::dom {

// --- Color parsing (subset of CSS colors used in game UIs) ---

render::Color SceneCompiler::parseCSSColor(const std::string& s) {
    if (s.empty() || s == "transparent") return {0, 0, 0, 0};

    if (s[0] == '#') {
        auto hex1f = [](char c) -> float {
            int v = (c >= 'a' ? c - 'a' + 10 : c >= 'A' ? c - 'A' + 10 : c - '0');
            return (v * 17) / 255.0f;
        };
        auto hex2f = [](const char* p) -> float {
            int v = 0;
            for (int i = 0; i < 2; i++) {
                char c = p[i];
                v = v * 16 + (c >= 'a' ? c - 'a' + 10 : c >= 'A' ? c - 'A' + 10 : c - '0');
            }
            return v / 255.0f;
        };

        if (s.size() == 4 || s.size() == 5) {
            render::Color c;
            c.r = hex1f(s[1]);
            c.g = hex1f(s[2]);
            c.b = hex1f(s[3]);
            c.a = s.size() == 5 ? hex1f(s[4]) : 1.0f;
            return c;
        }
        if (s.size() >= 7) {
            render::Color c;
            c.r = hex2f(s.c_str() + 1);
            c.g = hex2f(s.c_str() + 3);
            c.b = hex2f(s.c_str() + 5);
            c.a = s.size() >= 9 ? hex2f(s.c_str() + 7) : 1.0f;
            return c;
        }
    }

    if (s.rfind("rgba(", 0) == 0) {
        float r = 0, g = 0, b = 0, a = 1;
        sscanf(s.c_str(), "rgba(%f,%f,%f,%f)", &r, &g, &b, &a);
        if (r > 1 || g > 1 || b > 1) { r /= 255.0f; g /= 255.0f; b /= 255.0f; }
        return {r, g, b, a};
    }

    if (s.rfind("rgb(", 0) == 0) {
        float r = 0, g = 0, b = 0;
        sscanf(s.c_str(), "rgb(%f,%f,%f)", &r, &g, &b);
        if (r > 1 || g > 1 || b > 1) { r /= 255.0f; g /= 255.0f; b /= 255.0f; }
        return {r, g, b, 1.0f};
    }

    if (s == "white") return {1, 1, 1, 1};
    if (s == "black") return {0, 0, 0, 1};
    if (s == "red") return {1, 0, 0, 1};
    if (s == "green") return {0, 0.5f, 0, 1};
    if (s == "blue") return {0, 0, 1, 1};

    return {0, 0, 0, 0};
}

// --- Helper to read inline style property ---

static std::string getIS(const DOMNodePtr& node, const std::string& prop) {
    return node->getInlineStyleProperty(prop);
}

static float parsePx(const std::string& val, float fallback = 0) {
    if (val.empty()) return fallback;
    return (float)atof(val.c_str());
}

// --- Detection ---

static bool isSceneCandidate(const DOMNodePtr& node) {
    if (!node) return true;
    if (node->getNodeType() != DOMNode::NodeType::ELEMENT) return true;

    auto tag = node->getTagName();
    if (tag == "html" || tag == "head" || tag == "body" || tag == "meta" ||
        tag == "title" || tag == "script" || tag == "link") {
        for (auto& child : node->getChildren()) {
            if (!isSceneCandidate(child)) return false;
        }
        return true;
    }

    // Read position from inline styles
    auto pos = getIS(node, "position");
    auto display = getIS(node, "display");

    if (display == "flex" || display == "grid" ||
        display == "inline-flex" || display == "inline-grid") {
        return false;
    }

    if (pos != "absolute" && pos != "relative" && pos != "fixed") {
        if (!node->getChildren().empty()) {
            auto w = getIS(node, "width");
            auto h = getIS(node, "height");
            if (w.empty() && h.empty()) return false;
        }
    }

    for (auto& child : node->getChildren()) {
        if (!isSceneCandidate(child)) return false;
    }
    return true;
}

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

    // Only compile when explicitly opted-in via meta tag.
    // Heuristic auto-detection is too error-prone for mixed HTML.
    if (findMetaSceneMode(root)) {
        DONG_LOG_INFO("[SceneCompiler] Forced scene mode via meta tag");
        return true;
    }

    return false;
}

// --- Compilation (reads inline styles directly, no need for computed style) ---

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

    auto display = getIS(node, "display");
    if (display == "none") return;

    render::SceneNode sn;
    sn.name = node->getAttribute("id");

    float x = parsePx(getIS(node, "left"));
    float y = parsePx(getIS(node, "top"));
    float w = parsePx(getIS(node, "width"));
    float h = parsePx(getIS(node, "height"));

    sn.x = parent_x + x;
    sn.y = parent_y + y;
    sn.width = w;
    sn.height = h;

    // Visual properties from inline styles
    auto bg = getIS(node, "background");
    if (bg.empty()) bg = getIS(node, "background-color");
    sn.background_color = parseCSSColor(bg);

    auto border = getIS(node, "border");
    if (!border.empty()) {
        // Parse "Npx solid #color" format
        float bw = parsePx(border);
        sn.border_width = bw;
        // Extract color from border shorthand
        auto hash_pos = border.find('#');
        auto rgba_pos = border.find("rgba(");
        auto rgb_pos = border.find("rgb(");
        if (hash_pos != std::string::npos) {
            sn.border_color = parseCSSColor(border.substr(hash_pos));
        } else if (rgba_pos != std::string::npos) {
            auto end = border.find(')', rgba_pos);
            if (end != std::string::npos) {
                sn.border_color = parseCSSColor(border.substr(rgba_pos, end - rgba_pos + 1));
            }
        } else if (rgb_pos != std::string::npos) {
            auto end = border.find(')', rgb_pos);
            if (end != std::string::npos) {
                sn.border_color = parseCSSColor(border.substr(rgb_pos, end - rgb_pos + 1));
            }
        }
    }
    auto border_left = getIS(node, "border-left");
    auto border_right = getIS(node, "border-right");
    if (!border_left.empty() && sn.border_width < 0.01f) {
        sn.border_width = parsePx(border_left);
    }

    sn.border_radius = parsePx(getIS(node, "border-radius"));
    auto opacity_str = getIS(node, "opacity");
    sn.opacity = opacity_str.empty() ? 1.0f : parsePx(opacity_str, 1.0f);

    // Text properties
    auto color = getIS(node, "color");
    sn.text_color = color.empty() ? render::Color{0.93f, 0.93f, 0.93f, 1.0f} : parseCSSColor(color);

    auto fs = getIS(node, "font-size");
    sn.font_size = fs.empty() ? 16.0f : parsePx(fs, 16.0f);

    auto ff = getIS(node, "font-family");
    sn.font_family = ff.empty() ? "sans-serif" : ff;

    auto fw = getIS(node, "font-weight");
    sn.font_weight = fw.empty() ? "normal" : fw;

    auto ta = getIS(node, "text-align");
    sn.text_align = ta.empty() ? "left" : ta;

    auto lh = getIS(node, "line-height");
    sn.line_height = lh.empty() ? 0 : parsePx(lh);

    sn.text = collectDirectText(node);
    sn.parent = parent_id;

    // Store absolute position for children
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

    // Extract body background from inline style
    auto body_bg = getIS(body, "background");
    if (body_bg.empty()) body_bg = getIS(body, "background-color");
    float bw = parsePx(getIS(body, "width"));
    float bh = parsePx(getIS(body, "height"));

    uint32_t body_node_id = UINT32_MAX;
    if (bw > 0 && bh > 0) {
        render::SceneNode bg;
        bg.name = "__body__";
        bg.x = 0; bg.y = 0; bg.width = bw; bg.height = bh;
        bg.background_color = parseCSSColor(body_bg);
        body_node_id = sg.addNode(std::move(bg));
    }

    for (auto& child : body->getChildren()) {
        compileNode(child, sg, body_node_id, 0, 0);
    }

    DONG_LOG_INFO("[SceneCompiler] Compiled %zu nodes from DOM", sg.nodeCount());
    return sg.nodeCount() > 0;
}

} // namespace dong::dom
