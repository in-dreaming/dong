#include "painter.hpp"
#include <cstring>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <sstream>
#include <cctype>
#include <cmath>
#include "../core/log.h"
namespace dong::render {

namespace {

// CSS 棰滆壊瑙ｆ瀽鍣紙姝ｅ紡鐗堬級锛氭敮鎸?#rgb/#rgba/#rrggbb/#rrggbbaa 涓?rgb()/rgba() 瀛愰泦
static void parseCssColor(const std::string& css, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
    auto clampToByte = [](int v) -> uint8_t {
        if (v < 0) return 0;
        if (v > 255) return 255;
        return static_cast<uint8_t>(v);
    };

    auto parseHexNibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return 0;
    };

    auto parseHex2 = [&](char c1, char c2) -> uint8_t {
        int hi = parseHexNibble(c1);
        int lo = parseHexNibble(c2);
        return clampToByte((hi << 4) | lo);
    };

    auto parseComponent = [&](const std::string& s, bool is_alpha, int& out_int, float& out_alpha) {
        std::string v = s;
        // 鍘婚灏剧┖鐧?
        v.erase(v.begin(), std::find_if(v.begin(), v.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        while (!v.empty() && std::isspace(static_cast<unsigned char>(v.back()))) {
            v.pop_back();
        }

        bool is_percent = false;
        if (!v.empty() && v.back() == '%') {
            is_percent = true;
            v.pop_back();
        }

        float f = 0.0f;
        try {
            f = std::stof(v);
        } catch (...) {
            f = 0.0f;
        }

        if (!is_alpha) {
            if (is_percent) {
                f = f * 255.0f / 100.0f;
            }
            out_int = static_cast<int>(std::round(f));
            out_alpha = 1.0f;
        } else {
            if (is_percent) {
                f = f / 100.0f;
            }
            if (f < 0.0f) f = 0.0f;
            if (f > 1.0f) f = 1.0f;
            out_int = static_cast<int>(std::round(f * 255.0f));
            out_alpha = f;
        }
    };

    // 棰勫鐞嗭細鍘荤┖鐧藉苟杞皬鍐?
    std::string s = css;
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch); }), s.end());
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    // 榛樿鍊硷細涓嶅悎娉曟椂缁欎竴涓彲瑙佺殑娴呯伆鑹诧紝鏂逛究璋冭瘯
    r = g = b = 240;
    a = 255;

    if (s.empty() || s == "transparent") {
        r = g = b = 0;
        a = 0;
        return;
    }

    // 鍗佸叚杩涘埗褰㈠紡
    if (!s.empty() && s[0] == '#') {
        if (s.size() == 4) {          // #rgb
            int r4 = parseHexNibble(s[1]);
            int g4 = parseHexNibble(s[2]);
            int b4 = parseHexNibble(s[3]);
            r = clampToByte((r4 << 4) | r4);
            g = clampToByte((g4 << 4) | g4);
            b = clampToByte((b4 << 4) | b4);
            a = 255;
            return;
        } else if (s.size() == 5) {  // #rgba
            int r4 = parseHexNibble(s[1]);
            int g4 = parseHexNibble(s[2]);
            int b4 = parseHexNibble(s[3]);
            int a4 = parseHexNibble(s[4]);
            r = clampToByte((r4 << 4) | r4);
            g = clampToByte((g4 << 4) | g4);
            b = clampToByte((b4 << 4) | b4);
            a = clampToByte((a4 << 4) | a4);
            return;
        } else if (s.size() == 7) {  // #rrggbb
            r = parseHex2(s[1], s[2]);
            g = parseHex2(s[3], s[4]);
            b = parseHex2(s[5], s[6]);
            a = 255;
            return;
        } else if (s.size() == 9) {  // #rrggbbaa
            r = parseHex2(s[1], s[2]);
            g = parseHex2(s[3], s[4]);
            b = parseHex2(s[5], s[6]);
            a = parseHex2(s[7], s[8]);
            return;
        }
    }

    // rgb()/rgba()
    auto startsWith = [](const std::string& str, const char* prefix) {
        const size_t len = std::strlen(prefix);
        return str.size() >= len && std::equal(prefix, prefix + len, str.begin());
    };

    if (startsWith(s, "rgb(") || startsWith(s, "rgba(")) {
        bool has_alpha = startsWith(s, "rgba(");
        size_t lparen = s.find('(');
        size_t rparen = s.rfind(')');
        if (lparen != std::string::npos && rparen != std::string::npos && rparen > lparen + 1) {
            std::string args = s.substr(lparen + 1, rparen - lparen - 1);
            std::vector<std::string> parts;
            std::string current;
            for (char c : args) {
                if (c == ',') {
                    if (!current.empty()) {
                        parts.push_back(current);
                        current.clear();
                    }
                } else {
                    current.push_back(c);
                }
            }
            if (!current.empty()) {
                parts.push_back(current);
            }

            int r_int = 0, g_int = 0, b_int = 0, a_int = 255;
            float a_float = 1.0f;
            if ((has_alpha && parts.size() == 4) || (!has_alpha && parts.size() == 3)) {
                int dummy_int = 0;
                float dummy_alpha = 1.0f;
                parseComponent(parts[0], false, r_int, dummy_alpha);
                parseComponent(parts[1], false, g_int, dummy_alpha);
                parseComponent(parts[2], false, b_int, dummy_alpha);
                if (has_alpha) {
                    parseComponent(parts[3], true, a_int, a_float);
                }
                r = clampToByte(r_int);
                g = clampToByte(g_int);
                b = clampToByte(b_int);
                a = clampToByte(a_int);
                return;
            }
        }
    }

    // 鍛藉悕棰滆壊锛堝瓙闆嗭級锛屽叾浠栨湭瑕嗙洊鐨勯鑹插悕鍙互鎸夐渶瑕佺户缁墿鍏?
    if (s == "white")      { r = g = b = 255; a = 255; return; }
    if (s == "black")      { r = g = b = 0;   a = 255; return; }
    if (s == "red")        { r = 255; g = 0;   b = 0;   a = 255; return; }
    if (s == "green")      { r = 0;   g = 128; b = 0;   a = 255; return; }
    if (s == "blue")       { r = 0;   g = 0;   b = 255; a = 255; return; }
    if (s == "gray" || s == "grey") { r = g = b = 128; a = 255; return; }
    if (s == "lightgray" || s == "lightgrey") { r = g = b = 211; a = 255; return; }

    // 鍏跺畠鎯呭喌淇濈暀榛樿娴呯伆锛屾柟渚垮悗缁皟璇曞畾浣嶆湭瀹炵幇鐨勯鑹叉牸寮?
}

static Color makeColorFromCss(const std::string& css) {
    uint8_t r8 = 255, g8 = 255, b8 = 255, a8 = 255;
    parseCssColor(css, r8, g8, b8, a8);
    Color c;
    c.r = r8 / 255.0f;
    c.g = g8 / 255.0f;
    c.b = b8 / 255.0f;
    c.a = a8 / 255.0f;
    return c;
}

static std::string collapseWhitespace(const std::string& input) {
    if (input.empty()) return "";

    std::string output;
    output.reserve(input.size());
    bool in_space = false;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!in_space) {
                output.push_back(' ');
                in_space = true;
            }
        } else {
            output.push_back(c);
            in_space = false;
        }
    }

    size_t first = output.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = output.find_last_not_of(' ');
    return output.substr(first, last - first + 1);
}

// 绠€鍗曠殑鏂囨湰瀹藉害浼扮畻锛堝悗缁敤 HarfBuzz 鏇挎崲锛?
static float estimateTextWidth(const std::string& text, float font_size) {
    return static_cast<float>(text.size()) * font_size * 0.55f;
}

static bool shouldClipOverflow(const std::string& overflow_value) {
    std::string lowered = overflow_value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    // CSS 璇箟锛歰verflow:hidden/scroll/auto 閮介渶瑕佸缓绔嬭鍓笂涓嬫枃
    return lowered == "hidden" || lowered == "scroll" || lowered == "auto";
}

} // anonymous namespace

Painter::Painter(RenderSurface* surface)
    : surface_(surface), layout_engine_(nullptr) {
}

Painter::~Painter() {
}

const DisplayList& Painter::buildDisplayList(const dom::DOMNodePtr& root, layout::Engine* layout_engine) {
    layout_engine_ = layout_engine;

    display_list_builder_.clear();
    layer_tree_.clear();
    layer_stack_.clear();

    if (layout_engine_) {
        current_dirty_rect_ = layout_engine_->getDirtyRect();
        const auto* layout_root = layout_engine_->getLayout(root);
        if (layout_root) {
            buildDisplayListNode(root, layout_root, display_list_builder_);
        }
    }

    layout_engine_ = nullptr;
    return display_list_builder_.get();
}

void Painter::buildDisplayListNode(const dom::DOMNodePtr& node,
                                   const layout::LayoutNode* layout_node,
                                   DisplayListBuilder& builder) {
    if (!node) return;

    const auto& style = node->getComputedStyle();
    if (style.display == "none") return;

    if (node->getType() == dom::DOMNode::NodeType::TEXT) return;

    // Debug: log all elements
    const std::string tag = node->getTagName();
    std::string dbg_class = node->getAttribute("class");
    DONG_LOG_DEBUG("[Painter] buildDisplayListNode: tag='%s' class='%s'", tag.c_str(), dbg_class.c_str());

    // Dirty rect 浼樺寲
    if (use_dirty_rect_ && !current_dirty_rect_.isEmpty() && layout_node) {
        if (!isNodeInDirtyRect(layout_node)) {
            return;
        }
    }


    Rect node_rect{};;
    bool has_layout_rect = false;
    if (layout_node) {
        node_rect.x = layout_node->layout.position[0];
        node_rect.y = layout_node->layout.position[1];
        node_rect.width = layout_node->layout.dimensions[0];
        node_rect.height = layout_node->layout.dimensions[1];
        has_layout_rect = node_rect.width > 0.0f && node_rect.height > 0.0f;
    }

    const bool should_apply_clip = has_layout_rect && shouldClipOverflow(style.overflow);

    DisplayListBuilder::ScopedLayer opacity_scope;
    float clamped_opacity = std::clamp(style.opacity, 0.0f, 1.0f);

    Rect layer_bounds = node_rect;
    if (!has_layout_rect && surface_) {
        layer_bounds.x = 0.0f;
        layer_bounds.y = 0.0f;
        layer_bounds.width = static_cast<float>(surface_->getWidth());
        layer_bounds.height = static_cast<float>(surface_->getHeight());
    }

    const bool is_scroll_container =
        should_apply_clip &&
        (style.overflow == "scroll" || style.overflow == "auto");

    const bool has_transform =
        (style.transform_translate_x != 0.0f || style.transform_translate_y != 0.0f ||
         style.transform_scale_x != 1.0f || style.transform_scale_y != 1.0f);

    bool has_isolation = (style.isolation_isolate || is_scroll_container || has_transform) &&
                         (layer_bounds.width > 0.0f && layer_bounds.height > 0.0f);
    bool needs_layer = has_isolation || clamped_opacity < 0.999f;
    int parent_layer_index = layer_stack_.empty() ? -1 : layer_stack_.back();
    bool pushed_layer_node = false;
    if (needs_layer) {
        uint64_t layer_id = reinterpret_cast<uint64_t>(node.get());
        bool layer_dirty = node->isLayoutDirty();
        // 婊氬姩瀹瑰櫒濮嬬粓鏍囪涓鸿剰锛屽洜涓烘粴鍔ㄥ唴瀹逛細闅忕潃婊氬姩浣嶇疆鏀瑰彉
        if (is_scroll_container) {
            layer_dirty = true;
        }
        if (!layer_dirty && use_dirty_rect_ && !current_dirty_rect_.isEmpty()) {
            layer_dirty = isRectInDirtyRect(layer_bounds);
        }

        // 鍦?LayerTree 涓褰曡繖涓€灞?
        LayerNode layer_node;
        layer_node.id = layer_id;
        layer_node.type = has_isolation ? LayerType::Surface : LayerType::Opacity;
        layer_node.bounds = layer_bounds;
        layer_node.opacity = clamped_opacity;
        layer_node.transform = LayerTransform::identity();
        layer_node.transform.m[0] = style.transform_scale_x;
        layer_node.transform.m[4] = style.transform_scale_y;
        layer_node.transform.m[2] = style.transform_translate_x;
        layer_node.transform.m[5] = style.transform_translate_y;
        layer_node.scroll_x = node->getScrollX();
        layer_node.scroll_y = node->getScrollY();
        layer_node.is_surface = has_isolation;
        layer_node.content_dirty = layer_dirty;
        layer_node.transform_dirty = false;
        layer_node.opacity_dirty = false;
        layer_node.scroll_dirty = false;

        const int this_index = layer_tree_.addNode(layer_node, parent_layer_index);
        layer_stack_.push_back(this_index);
        pushed_layer_node = true;

        opacity_scope = builder.pushLayer(clamped_opacity, has_isolation, layer_bounds, layer_id, layer_dirty);
    }

    // 1. 鑳屾櫙涓庨槾褰?
    if (layout_node) {
        Rect rect = node_rect;

        // root/html/body 濉弧 viewport
        if ((tag.empty() || tag == "html" || tag == "body") && surface_) {
            rect.x = 0.0f;
            rect.y = 0.0f;
            rect.width = static_cast<float>(surface_->getWidth());
            rect.height = static_cast<float>(surface_->getHeight());
        }

        // 1.1 box-shadow锛堝厛鐢诲湪鑳屾櫙涔嬩笅锛?
        if (!style.box_shadows.empty() && rect.width > 0.0f && rect.height > 0.0f) {
            for (const auto& shadow : style.box_shadows) {
                if (shadow.color.empty()) continue;
                Color sc = makeColorFromCss(shadow.color);

                Rect shadow_rect = rect;
                shadow_rect.x += shadow.offset_x;
                shadow_rect.y += shadow.offset_y;
                shadow_rect.x -= shadow.spread_radius;
                shadow_rect.y -= shadow.spread_radius;
                shadow_rect.width += shadow.spread_radius * 2.0f;
                shadow_rect.height += shadow.spread_radius * 2.0f;

                if (shadow_rect.width <= 0.0f || shadow_rect.height <= 0.0f) {
                    continue;
                }

                float radius = style.border_radius;
                if (radius < 0.0f) radius = 0.0f;

                // 浣跨敤甯︽ā绯婄殑闃村奖缁樺埗
                float blur = shadow.blur_radius;
                if (blur > 0.0f) {
                    builder.addShadow(shadow_rect, sc, radius, blur);
                } else {
                    builder.addRoundedRect(shadow_rect, sc, radius);
                }
            }
        }

        // 1.2 鑳屾櫙濉厖
        if (style.background_color != "transparent" && rect.width > 0.0f && rect.height > 0.0f) {
            Color c = makeColorFromCss(style.background_color);
            if (style.border_radius > 0.0f) {
                builder.addRoundedRect(rect, c, style.border_radius);
            } else {
                builder.addRect(rect, c);
            }
        }

        // 1.3 杈规缁樺埗
        if (style.border_width > 0.0f && rect.width > 0.0f && rect.height > 0.0f) {
            Color border_color = makeColorFromCss(style.border_color);
            float bw = style.border_width;
            float radius = style.border_radius;
            if (radius < 0.0f) radius = 0.0f;

            // 缁樺埗鍥涙潯杈规锛堢畝鍖栧疄鐜帮細浣跨敤鍥涗釜鐭╁舰锛?
            // 涓婅竟妗?
            Rect top_border{rect.x, rect.y, rect.width, bw};
            // 涓嬭竟妗?
            Rect bottom_border{rect.x, rect.y + rect.height - bw, rect.width, bw};
            // 宸﹁竟妗?
            Rect left_border{rect.x, rect.y + bw, bw, rect.height - 2 * bw};
            // 鍙宠竟妗?
            Rect right_border{rect.x + rect.width - bw, rect.y + bw, bw, rect.height - 2 * bw};

            if (radius > 0.0f) {
                // 瀵逛簬鍦嗚杈规锛屼娇鐢ㄦ弿杈规柟寮忥紙鐩墠绠€鍖栦负鍥涗釜鐭╁舰锛屽悗缁彲鏀硅繘锛?
                builder.addRect(top_border, border_color);
                builder.addRect(bottom_border, border_color);
                builder.addRect(left_border, border_color);
                builder.addRect(right_border, border_color);
            } else {
                builder.addRect(top_border, border_color);
                builder.addRect(bottom_border, border_color);
                builder.addRect(left_border, border_color);
                builder.addRect(right_border, border_color);
            }
        }
    }

    DisplayListBuilder::ScopedClip clip_scope;
    if (should_apply_clip) {
        if (style.border_radius > 0.0f) {
            clip_scope = builder.pushRoundedClip(node_rect, style.border_radius);
        } else {
            clip_scope = builder.pushClipRect(node_rect);
        }
    }

    // 2. 鍥剧墖
    if (layout_node && tag == "img") {
        std::string src = node->getAttribute("src");
        if (!src.empty()) {
            Rect rect{};
            rect.x = layout_node->layout.position[0];
            rect.y = layout_node->layout.position[1];
            rect.width = layout_node->layout.dimensions[0];
            rect.height = layout_node->layout.dimensions[1];

            if (rect.width > 0.0f && rect.height > 0.0f) {
                builder.addImage(rect, src, 1.0f);
            }
        }
    }

    // 3. 鏂囨湰鍐呭
    if (layout_node && tag != "script" && tag != "style" && tag != "head" && tag != "img") {
        std::string debug_class = node->getAttribute("class");
        if (debug_class.find("overlay-row") != std::string::npos) {
            DONG_LOG_INFO("[Painter] overlay-row: checking children, count=%zu", node->getChildren().size());
            for (const auto& child : node->getChildren()) {
                if (child) {
                    DONG_LOG_INFO("[Painter] overlay-row child type=%d text='%s'",
                            static_cast<int>(child->getType()), child->getTextContent().c_str());
                }
            }
        }
        bool has_text_child = false;
        bool has_inline_element_child = false;
        std::string raw_text;
        
        for (const auto& child : node->getChildren()) {
            if (!child) continue;
            if (child->getType() == dom::DOMNode::NodeType::TEXT) {
                raw_text += child->getTextContent();
                has_text_child = true;
            } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                const auto& child_style = child->getComputedStyle();
                // 妫€鏌ユ槸鍚︿负 inline/inline-block 鍏冪礌
                if (child_style.display == "inline" || child_style.display == "inline-block") {
                    has_inline_element_child = true;
                }
            }
        }

        const bool tag_prefers_text =
            tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
            tag == "h5" || tag == "h6" || tag == "p" || tag == "span" ||
            tag == "button" || tag == "code" || tag == "div" || tag == "footer";

        if (debug_class.find("overlay-row") != std::string::npos) {
            DONG_LOG_INFO("[Painter] overlay-row: has_text=%d has_inline=%d tag_prefers=%d raw='%s'",
                    has_text_child, has_inline_element_child, tag_prefers_text, raw_text.c_str());
        }

        // 褰撴湁 inline 瀛愬厓绱犳椂锛岄渶瑕佹寜椤哄簭澶勭悊姣忎釜瀛愯妭鐐癸紝閬垮厤鏂囨湰閲嶅彔
        // 浣跨敤瀹瑰櫒灞傚畬鍏ㄦ帴绠℃贩鍚堝唴瀹圭殑甯冨眬鍜岀粯鍒?
        // 娉ㄦ剰锛氬鏋滃鍣ㄦ槸 flex 甯冨眬锛屼笉搴旇浣跨敤娣峰悎鍐呭璺緞锛屽洜涓?flex 浼氭纭绠楀瓙鍏冪礌浣嶇疆
        const bool is_flex_container = (style.display == "flex" || style.display == "inline-flex");
        if (has_text_child && has_inline_element_child && tag_prefers_text && !is_flex_container) {
            if (debug_class.find("overlay-row") != std::string::npos) {
                DONG_LOG_INFO("[Painter] overlay-row MIXED PATH raw_text='%s'", raw_text.c_str());
            }
            // 娣峰悎鍐呭锛氭湁 TEXT 鑺傜偣鍜?inline 鍏冪礌
            // 鎸夊瓙鑺傜偣椤哄簭鍒嗗埆缁樺埗姣忎釜鍐呭锛岃绠楁纭殑璧峰 X 浣嶇疆
            float x = layout_node->layout.position[0];
            float y = layout_node->layout.position[1];
            float width = layout_node->layout.dimensions[0];

            float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
            float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
            float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;

            float container_font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
            Color container_text_color = makeColorFromCss(style.color);

            float inner_width = width - pad_left - pad_right;
            if (inner_width <= 0.0f) inner_width = width > 0.0f ? width : 0.0f;

            // 璁＄畻瀹瑰櫒鐨?baseline 搴﹂噺
            float container_baseline_offset = 0.0f;
            float container_ascent_px = 0.0f;
            float container_line_height_px = 0.0f;
            {
                TextShapeRequest req{};
                req.text = "X";  // 浣跨敤 X 浣滀负鍩哄噯瀛楃
                req.font_family = style.font_family;
                req.font_weight = style.font_weight;
                req.font_size = container_font_size;
                ShapedText shaped{};
                if (text_shaper_.shape(req, shaped)) {
                    float scale = shaped.scale_to_pixels;
                    float ascent_units = shaped.ascent_units > 0.0f ? shaped.ascent_units : container_font_size / scale;
                    float descent_units = shaped.descent_units;
                    float line_height_units = shaped.line_height_units;
                    
                    if (style.line_height > 0.0f) {
                        if (style.line_height_is_unitless) {
                            line_height_units = (style.line_height * container_font_size) / std::max(scale, 1e-3f);
                        } else {
                            line_height_units = style.line_height / std::max(scale, 1e-3f);
                        }
                    }
                    if (line_height_units <= 0.0f) line_height_units = container_font_size / scale;
                    
                    float descent_abs_units = descent_units < 0.0f ? -descent_units : 0.0f;
                    float metrics_height_units = ascent_units + descent_abs_units;
                    float extra_leading_units = std::max(line_height_units - metrics_height_units, 0.0f);
                    float top_leading_units = extra_leading_units * 0.5f;
                    
                    container_baseline_offset = (top_leading_units + ascent_units) * scale;
                    container_ascent_px = ascent_units * scale;
                    container_line_height_px = line_height_units * scale;
                }
            }

            float cumulative_x_offset = 0.0f;
            float baseline_y = y + pad_top + container_baseline_offset;

            // 鎸夊瓙鑺傜偣椤哄簭澶勭悊鎵€鏈夊唴瀹癸紙TEXT 鍜?inline ELEMENT锛?
            for (const auto& child : node->getChildren()) {
                if (!child) continue;

                if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                    const auto& child_style = child->getComputedStyle();
                    if (child_style.display == "inline" || child_style.display == "inline-block") {
                        // 鑾峰彇 inline 鍏冪礌鐨勬枃鏈唴瀹瑰苟鐩存帴鍦ㄦ缁樺埗
                        // 鑰屼笉鏄緷璧栭€掑綊缁樺埗锛堝洜涓?layout engine 娌℃湁涓烘璁＄畻姝ｇ‘浣嶇疆锛?
                        std::string child_text;
                        for (const auto& grandchild : child->getChildren()) {
                            if (grandchild && grandchild->getType() == dom::DOMNode::NodeType::TEXT) {
                                child_text += grandchild->getTextContent();
                            }
                        }
                        child_text = collapseWhitespace(child_text);
                        
                        if (!child_text.empty()) {
                            // 浣跨敤 inline 鍏冪礌鐨勬牱寮?
                            float child_font_size = child_style.font_size > 0.0f ? child_style.font_size : container_font_size;
                            std::string child_font_family = !child_style.font_family.empty() ? child_style.font_family : style.font_family;
                            std::string child_font_weight = !child_style.font_weight.empty() ? child_style.font_weight : style.font_weight;
                            Color child_color = makeColorFromCss(!child_style.color.empty() ? child_style.color : style.color);
                            
                            TextShapeRequest req{};
                            req.text = child_text;
                            req.font_family = child_font_family;
                            req.font_weight = child_font_weight;
                            req.font_size = child_font_size;
                            
                            ShapedText shaped{};
                            if (text_shaper_.shape(req, shaped) && !shaped.glyphs.empty()) {
                                float scale = shaped.scale_to_pixels;
                                float text_width_px = shaped.width_units * scale;
                                float ascent_units = shaped.ascent_units > 0.0f ? shaped.ascent_units : child_font_size / scale;
                                float ascent_px = ascent_units * scale;
                                
                                // 璁＄畻 inline 鍏冪礌鐨?padding
                                float child_pad_left = child_style.padding_left.isPixel() ? child_style.padding_left.value : 0.0f;
                                float child_pad_right = child_style.padding_right.isPixel() ? child_style.padding_right.value : 0.0f;
                                
                                float text_x = x + pad_left + cumulative_x_offset + child_pad_left;
                                
                                // 缁樺埗 inline 鍏冪礌鐨勮儗鏅紙濡傛灉鏈夛級
                                if (!child_style.background_color.empty() && child_style.background_color != "transparent") {
                                    Color bg_color = makeColorFromCss(child_style.background_color);
                                    float bg_radius = child_style.border_radius > 0.0f ? child_style.border_radius : 0.0f;
                                    Rect bg_rect{};
                                    bg_rect.x = x + pad_left + cumulative_x_offset;
                                    bg_rect.y = baseline_y - container_ascent_px;
                                    bg_rect.width = text_width_px + child_pad_left + child_pad_right;
                                    bg_rect.height = container_line_height_px;
                                    if (bg_radius > 0.0f) {
                                        builder.addRoundedRect(bg_rect, bg_color, bg_radius);
                                    } else {
                                        builder.addRect(bg_rect, bg_color);
                                    }
                                }
                                
                                DrawGlyphRunData glyph_run{};
                                glyph_run.rect.x = text_x;
                                glyph_run.rect.y = baseline_y - ascent_px;
                                glyph_run.rect.width = text_width_px;
                                glyph_run.rect.height = container_line_height_px;
                                glyph_run.color = child_color;
                                glyph_run.font_size = child_font_size;
                                glyph_run.font_family = child_font_family;
                                glyph_run.font_weight = child_font_weight;
                                glyph_run.font_path = shaped.font_path;
                                glyph_run.baseline_x = text_x;
                                glyph_run.baseline_y = baseline_y;
                                glyph_run.units_per_em = shaped.units_per_em;
                                glyph_run.scale_to_pixels = shaped.scale_to_pixels;
                                
                                for (const auto& sg : shaped.glyphs) {
                                    GlyphInstance inst{};
                                    inst.glyph_id = sg.glyph_id;
                                    inst.pen_x_units = sg.pen_x_units;
                                    inst.pen_y_units = sg.pen_y_units;
                                    inst.font_path = sg.font_path;
                                    inst.units_per_em = sg.units_per_em;
                                    glyph_run.glyphs.push_back(inst);
                                }
                                
                                builder.addGlyphRun(std::move(glyph_run));
                                cumulative_x_offset += text_width_px + child_pad_left + child_pad_right;
                            }
                            
                            // 鍙湁褰撴枃鏈唴瀹硅鎴愬姛缁樺埗鏃讹紝鎵嶆爣璁拌 inline 鍏冪礌宸茬粡鍦ㄥ鍣ㄥ眰缁樺埗
                            // 瀵逛簬娌℃湁鏂囨湰鍐呭鐨勫厓绱狅紙濡?input锛夛紝璁╁畠閫氳繃姝ｅ父閫掑綊鏉ョ粯鍒?
                            child->setAttribute("__inline_rendered__", "1");
                        }
                    }
                } else if (child->getType() == dom::DOMNode::NodeType::TEXT) {
                    std::string text_content = child->getTextContent();
                    std::string text = collapseWhitespace(text_content);
                    if (text.empty()) continue;

                    TextShapeRequest req{};
                    req.text = text;
                    req.font_family = style.font_family;
                    req.font_weight = style.font_weight;
                    req.font_size = container_font_size;

                    ShapedText shaped{};
                    if (!text_shaper_.shape(req, shaped) || shaped.glyphs.empty()) {
                        continue;
                    }

                    float scale = shaped.scale_to_pixels;
                    float text_width_px = shaped.width_units * scale;
                    float ascent_units = shaped.ascent_units > 0.0f ? shaped.ascent_units : container_font_size / scale;
                    float ascent_px = ascent_units * scale;

                    float text_x = x + pad_left + cumulative_x_offset;

                    DrawGlyphRunData glyph_run{};
                    glyph_run.rect.x = text_x;
                    glyph_run.rect.y = baseline_y - ascent_px;
                    glyph_run.rect.width = text_width_px;
                    glyph_run.rect.height = container_line_height_px;
                    glyph_run.color = container_text_color;
                    glyph_run.font_size = container_font_size;
                    glyph_run.font_family = style.font_family;
                    glyph_run.font_weight = style.font_weight;
                    glyph_run.font_path = shaped.font_path;
                    glyph_run.baseline_x = text_x;
                    glyph_run.baseline_y = baseline_y;
                    glyph_run.units_per_em = shaped.units_per_em;
                    glyph_run.scale_to_pixels = shaped.scale_to_pixels;

                    for (const auto& sg : shaped.glyphs) {
                        GlyphInstance inst{};
                        inst.glyph_id = sg.glyph_id;
                        inst.pen_x_units = sg.pen_x_units;
                        inst.pen_y_units = sg.pen_y_units;
                        inst.font_path = sg.font_path;
                        inst.units_per_em = sg.units_per_em;
                        glyph_run.glyphs.push_back(inst);
                    }

                    builder.addGlyphRun(std::move(glyph_run));
                    cumulative_x_offset += text_width_px;
                }
            }
        } else if (has_text_child && (tag_prefers_text || !has_inline_element_child)) {
            std::string debug_class = node->getAttribute("class");
            if (debug_class.find("abs-badge") != std::string::npos) {
                DONG_LOG_INFO("[Painter] ABS badge text raw='%s'", raw_text.c_str());
            }
            if (debug_class.find("overlay-row") != std::string::npos) {
                DONG_LOG_INFO("[Painter] overlay-row raw_text='%s' (len=%zu)", raw_text.c_str(), raw_text.size());
            }
            std::string text = collapseWhitespace(raw_text);
            if (debug_class.find("overlay-row") != std::string::npos) {
                DONG_LOG_INFO("[Painter] overlay-row after collapse='%s' (len=%zu)", text.c_str(), text.size());
            }

            // 搴旂敤 text-transform
            if (!text.empty() && !style.text_transform.empty() && style.text_transform != "none") {
                if (style.text_transform == "uppercase") {
                    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
                        return static_cast<char>(std::toupper(c));
                    });
                } else if (style.text_transform == "lowercase") {
                    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    });
                }
                // capitalize 鏆備笉鏀寔锛堥渶瑕佹洿澶嶆潅鐨?Unicode 澶勭悊锛?
            }

            if (!text.empty()) {
                float x = layout_node->layout.position[0];
                float y = layout_node->layout.position[1];
                float width = layout_node->layout.dimensions[0];
                float height = layout_node->layout.dimensions[1];

                float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
                float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
                float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;

                float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

                Color text_color = makeColorFromCss(style.color);

                float inner_width = width - pad_left - pad_right;
                if (inner_width <= 0.0f) inner_width = width > 0.0f ? width : 0.0f;

                // 浣跨敤 HarfBuzz 鍋氫竴娆″畬鏁?shaping锛屽苟鍩轰簬 glyph 瀹藉害杩涜鎹㈣
                TextShapeRequest full_req{};
                full_req.text = text;
                full_req.font_family = style.font_family;
                full_req.font_weight = style.font_weight;
                full_req.font_size = font_size;

                ShapedText shaped_full{};
                if (!text_shaper_.shape(full_req, shaped_full) || shaped_full.glyphs.empty()) {
                    return;
                }

                const float scale = shaped_full.scale_to_pixels;

                float letter_spacing_em = style.letter_spacing_em;
                float letter_spacing_px = 0.0f;
                float letter_spacing_units = 0.0f;
                if (letter_spacing_em != 0.0f) {
                    letter_spacing_px = letter_spacing_em * font_size;
                    if (scale > 0.0f) {
                        letter_spacing_units = letter_spacing_px / scale;
                    }
                }

                // word-spacing: 棰濆鐨勫崟璇嶉棿璺濓紙搴旂敤浜庣┖鏍煎瓧绗︼級
                float word_spacing_px = style.word_spacing_px;
                float word_spacing_units = 0.0f;
                if (word_spacing_px != 0.0f && scale > 0.0f) {
                    word_spacing_units = word_spacing_px / scale;
                }

                // 琛岄珮/鍩虹嚎搴﹂噺锛堣璁″崟浣嶏級
                // 浼樺厛浣跨敤 CSS line-height锛屽鏋滄湭璁剧疆鍒欎娇鐢ㄥ瓧浣撳害閲?
                float line_height_units = shaped_full.line_height_units;
                float ascent_units = shaped_full.ascent_units;
                float descent_units = shaped_full.descent_units;

                // 搴旂敤 CSS line-height 灞炴€?
                if (style.line_height > 0.0f) {
                    if (style.line_height_is_unitless) {
                        // 鍊嶆暟锛歭ine-height * font-size
                        float css_line_height_px = style.line_height * font_size;
                        line_height_units = css_line_height_px / std::max(scale, 1e-3f);
                    } else {
                        // 鍍忕礌鍊?
                        line_height_units = style.line_height / std::max(scale, 1e-3f);
                    }
                }

                if (line_height_units <= 0.0f) {
                    line_height_units = font_size / std::max(scale, 1e-3f);
                }
                if (ascent_units <= 0.0f) {
                    ascent_units = font_size / std::max(scale, 1e-3f);
                }
                float descent_abs_units = descent_units < 0.0f ? -descent_units : 0.0f;
                float metrics_height_units = ascent_units + descent_abs_units;
                if (metrics_height_units <= 0.0f) {
                    metrics_height_units = line_height_units;
                }
                float extra_leading_units = line_height_units - metrics_height_units;
                if (extra_leading_units < 0.0f) {
                    extra_leading_units = 0.0f;
                }
                float top_leading_units = extra_leading_units * 0.5f;

                float effective_line_height = line_height_units * scale;
                float ascent_px = ascent_units * scale;
                float baseline_offset = (top_leading_units + ascent_units) * scale;

                // 鍩轰簬 UTF-8 瀛楄妭鍋忕Щ鏋勫缓娼滃湪鎹㈣鐐癸紙绌烘牸 + CJK 瀛楃鍚庯級
                std::vector<size_t> break_points;
                break_points.reserve(text.size() + 1);

                const auto push_break = [&](size_t byte_index) {
                    if (!break_points.empty() && break_points.back() == byte_index) {
                        return;
                    }
                    break_points.push_back(byte_index);
                };

                size_t i_byte = 0;
                while (i_byte < text.size()) {
                    unsigned char c = static_cast<unsigned char>(text[i_byte]);
                    size_t char_start = i_byte;
                    size_t char_len = 1;
                    if ((c & 0b10000000) == 0) {
                        // ASCII
                        i_byte += 1;
                        if (c == ' ') {
                            push_break(i_byte); // 鍦ㄧ┖鏍间箣鍚庡厑璁告崲琛?
                        }
                        continue;
                    } else if ((c & 0b11100000) == 0b11000000) {
                        char_len = 2;
                    } else if ((c & 0b11110000) == 0b11100000) {
                        char_len = 3;
                    } else if ((c & 0b11111000) == 0b11110000) {
                        char_len = 4;
                    }
                    i_byte += char_len;
                    // 瀵逛簬闈?ASCII锛堝 CJK锛夛紝鍦ㄥ瓧绗﹀悗鍏佽鎹㈣
                    push_break(i_byte);
                }

                // 淇濊瘉鏂囨湰鏈熬鏄竴涓崲琛屽€欓€?
                if (break_points.empty() || break_points.back() != text.size()) {
                    break_points.push_back(text.size());
                }

                const auto measure_range_units = [&](size_t byte_start, size_t byte_end) -> float {
                    const auto& glyphs = shaped_full.glyphs;
                    int first = -1;
                    int last = -1;
                    int glyph_count = 0;
                    int space_count = 0;
                    for (size_t gi = 0; gi < glyphs.size(); ++gi) {
                        uint32_t cluster = glyphs[gi].cluster;
                        if (cluster < byte_start) {
                            continue;
                        }
                        if (cluster >= byte_end) {
                            break;
                        }
                        if (first == -1) {
                            first = static_cast<int>(gi);
                        }
                        last = static_cast<int>(gi);
                        ++glyph_count;
                        // 缁熻绌烘牸鏁伴噺鐢ㄤ簬 word-spacing
                        if (cluster < text.size() && text[cluster] == ' ') {
                            ++space_count;
                        }
                    }
                    if (first == -1 || last == -1 || glyph_count == 0) {
                        return 0.0f;
                    }
                    const ShapedGlyph& g_first = shaped_full.glyphs[static_cast<size_t>(first)];
                    const ShapedGlyph& g_last = shaped_full.glyphs[static_cast<size_t>(last)];
                    float left = g_first.pen_x_units;
                    float right = g_last.pen_x_units + g_last.advance_x_units;
                    float w = right - left;
                    if (glyph_count > 1 && letter_spacing_units != 0.0f) {
                        w += letter_spacing_units * static_cast<float>(glyph_count - 1);
                    }
                    // 搴旂敤 word-spacing
                    if (space_count > 0 && word_spacing_units != 0.0f) {
                        w += word_spacing_units * static_cast<float>(space_count);
                    }
                    return w > 0.0f ? w : 0.0f;
                };

                const auto glyph_range_for_bytes = [&](size_t byte_start, size_t byte_end,
                                                       int& out_first, int& out_last) {
                    out_first = -1;
                    out_last = -1;
                    const auto& glyphs = shaped_full.glyphs;
                    for (size_t gi = 0; gi < glyphs.size(); ++gi) {
                        uint32_t cluster = glyphs[gi].cluster;
                        if (cluster < byte_start) {
                            continue;
                        }
                        if (cluster >= byte_end) {
                            break;
                        }
                        if (out_first == -1) {
                            out_first = static_cast<int>(gi);
                        }
                        out_last = static_cast<int>(gi);
                    }
                };

                const float max_line_width_px = inner_width;
                size_t text_len = text.size();
                size_t line_start = 0;
                int line_index = 0;

                while (line_start < text_len) {
                    // 璺宠繃琛岄绌烘牸
                    while (line_start < text_len && text[line_start] == ' ') {
                        ++line_start;
                    }
                    if (line_start >= text_len) {
                        break;
                    }

                    float best_width_units = 0.0f;
                    size_t best_break = text_len;
                    bool found_any = false;

                    for (size_t bp : break_points) {
                        if (bp <= line_start) {
                            continue;
                        }
                        float w_units = measure_range_units(line_start, bp);
                        float w_px = w_units * scale;
                        if (w_px <= max_line_width_px + 0.1f) {
                            found_any = true;
                            best_break = bp;
                            best_width_units = w_units;
                        } else {
                            // 鍚庣画 break point 鍙細鏇村锛岀洿鎺ュ仠姝?
                            break;
                        }
                    }

                    if (!found_any) {
                        // 鏃犳硶鍦ㄥ綋鍓嶈瀹藉唴鎵惧埌 break point锛屽己鍒跺湪绗竴涓彲鐢?break 鎴栨枃鏈湯灏炬崲琛?
                        size_t fallback_break = text_len;
                        for (size_t bp : break_points) {
                            if (bp > line_start) {
                                fallback_break = bp;
                                break;
                            }
                        }
                        best_break = fallback_break;
                        best_width_units = measure_range_units(line_start, best_break);
                    }

                    int first_glyph = -1;
                    int last_glyph = -1;
                    glyph_range_for_bytes(line_start, best_break, first_glyph, last_glyph);
                    if (first_glyph == -1 || last_glyph == -1) {
                        // 娌℃湁 glyph锛堝彲鑳芥槸鍏ㄧ┖鏍硷級锛岀洿鎺ョ粨鏉?
                        line_start = best_break;
                        continue;
                    }

                    float line_width_px = best_width_units * scale;

                    float text_x = x + pad_left;
                    if (style.text_align == "center") {
                        text_x = x + pad_left + std::max(0.0f, (inner_width - line_width_px) * 0.5f);
                    } else if (style.text_align == "right") {
                        text_x = x + pad_left + std::max(0.0f, inner_width - line_width_px);
                    }

                    float base_baseline = y + pad_top + baseline_offset;
                    float baseline_y = base_baseline + static_cast<float>(line_index) * effective_line_height;

                    DrawGlyphRunData glyph_run{};
                    glyph_run.rect.x = text_x;
                    glyph_run.rect.y = baseline_y - ascent_px;
                    glyph_run.rect.width = line_width_px;
                    glyph_run.rect.height = effective_line_height;
                    glyph_run.color = text_color;
                    glyph_run.font_size = font_size;
                    glyph_run.font_family = style.font_family;
                    glyph_run.font_weight = style.font_weight;
                    glyph_run.font_path = shaped_full.font_path;
                    glyph_run.baseline_x = text_x;
                    glyph_run.baseline_y = baseline_y;
                    glyph_run.units_per_em = shaped_full.units_per_em;
                    glyph_run.scale_to_pixels = shaped_full.scale_to_pixels;

                    const auto& glyphs = shaped_full.glyphs;
                    glyph_run.glyphs.reserve(static_cast<size_t>(last_glyph - first_glyph + 1));

                    float first_pen_x_units = glyphs[static_cast<size_t>(first_glyph)].pen_x_units;

                    int glyph_index_in_run = 0;
                    int accumulated_spaces = 0;  // 绱绌烘牸鏁扮敤浜?word-spacing
                    for (int gi = first_glyph; gi <= last_glyph; ++gi) {
                        const ShapedGlyph& sg = glyphs[static_cast<size_t>(gi)];
                        GlyphInstance inst{};
                        inst.glyph_id = sg.glyph_id;
                        float base_x_units = sg.pen_x_units - first_pen_x_units;
                        if (letter_spacing_units != 0.0f && glyph_index_in_run > 0) {
                            base_x_units += letter_spacing_units * static_cast<float>(glyph_index_in_run);
                        }
                        // 搴旂敤 word-spacing锛氬湪绌烘牸涔嬪悗鐨勫瓧绗︽坊鍔犻澶栭棿璺?
                        if (word_spacing_units != 0.0f && accumulated_spaces > 0) {
                            base_x_units += word_spacing_units * static_cast<float>(accumulated_spaces);
                        }
                        inst.pen_x_units = base_x_units;
                        inst.pen_y_units = sg.pen_y_units;
                        inst.font_path = sg.font_path;
                        inst.units_per_em = sg.units_per_em;
                        glyph_run.glyphs.push_back(inst);
                        ++glyph_index_in_run;
                        // 妫€鏌ュ綋鍓?glyph 鏄惁瀵瑰簲绌烘牸瀛楃
                        if (sg.cluster < text.size() && text[sg.cluster] == ' ') {
                            ++accumulated_spaces;
                        }
                    }

                    builder.addGlyphRun(std::move(glyph_run));

                    line_start = best_break;
                    ++line_index;
                }
            }
        }
    }

    // 3.5 Input 鍏冪礌鐗规畩娓叉煋锛氭樉绀?value 鎴?placeholder
    if (layout_node && tag == "input") {
        float x = layout_node->layout.position[0];
        float y = layout_node->layout.position[1];
        float width = layout_node->layout.dimensions[0];
        float height = layout_node->layout.dimensions[1];
        
        float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
        float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
        
        float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
        
        // 鑾峰彇 value 鎴?placeholder
        std::string display_text = node->getAttribute("value");
        Color text_color = makeColorFromCss(style.color);
        
        if (display_text.empty()) {
            display_text = node->getAttribute("placeholder");
            // placeholder 浣跨敤鍗婇€忔槑棰滆壊
            text_color.a *= 0.5f;
        }
        
        if (!display_text.empty() && width > 0.0f && height > 0.0f) {
            TextShapeRequest req{};
            req.text = display_text;
            req.font_family = style.font_family;
            req.font_weight = style.font_weight;
            req.font_size = font_size;
            
            ShapedText shaped{};
            if (text_shaper_.shape(req, shaped) && !shaped.glyphs.empty()) {
                float scale = shaped.scale_to_pixels;
                float ascent_units = shaped.ascent_units > 0.0f ? shaped.ascent_units : font_size / scale;
                float descent_units = shaped.descent_units;
                float line_height_units = shaped.line_height_units;
                
                if (line_height_units <= 0.0f) line_height_units = font_size / scale;
                
                float descent_abs_units = descent_units < 0.0f ? -descent_units : 0.0f;
                float metrics_height_units = ascent_units + descent_abs_units;
                float extra_leading_units = std::max(line_height_units - metrics_height_units, 0.0f);
                float top_leading_units = extra_leading_units * 0.5f;
                
                float baseline_offset = (top_leading_units + ascent_units) * scale;
                float text_height_px = line_height_units * scale;
                
                // 鍨傜洿灞呬腑
                float text_y = y + (height - text_height_px) * 0.5f;
                float baseline_y = text_y + baseline_offset;
                float text_x = x + pad_left;
                
                DrawGlyphRunData glyph_run{};
                glyph_run.rect.x = text_x;
                glyph_run.rect.y = text_y;
                glyph_run.rect.width = shaped.width_units * scale;
                glyph_run.rect.height = text_height_px;
                glyph_run.color = text_color;
                glyph_run.font_size = font_size;
                glyph_run.font_family = style.font_family;
                glyph_run.font_weight = style.font_weight;
                glyph_run.font_path = shaped.font_path;
                glyph_run.baseline_x = text_x;
                glyph_run.baseline_y = baseline_y;
                glyph_run.units_per_em = shaped.units_per_em;
                glyph_run.scale_to_pixels = shaped.scale_to_pixels;
                
                for (const auto& sg : shaped.glyphs) {
                    GlyphInstance inst{};
                    inst.glyph_id = sg.glyph_id;
                    inst.pen_x_units = sg.pen_x_units;
                    inst.pen_y_units = sg.pen_y_units;
                    inst.font_path = sg.font_path;
                    inst.units_per_em = sg.units_per_em;
                    glyph_run.glyphs.push_back(inst);
                }
                
                builder.addGlyphRun(std::move(glyph_run));
            }
        }
    }

    // 4. 閫掑綊瀛愯妭鐐癸紙鎸?z-index 鎺掑簭锛?
    const auto& children = node->getChildren();
    
    // 濡傛灉鏄粴鍔ㄥ鍣紝搴旂敤婊氬姩鍋忕Щ鍒板瓙鍏冪礌
    bool applied_scroll_translate = false;
    if (is_scroll_container) {
        float scroll_x = node->getScrollX();
        float scroll_y = node->getScrollY();
        if (scroll_x != 0.0f || scroll_y != 0.0f) {
            builder.pushTranslate(-scroll_x, -scroll_y);
            applied_scroll_translate = true;
        }
    }
    
    // 鏀堕泦闇€瑕佺粯鍒剁殑瀛愬厓绱犲強鍏?z-index
    struct ChildWithZIndex {
        dom::DOMNodePtr child;
        int z_index;
        size_t original_order;
    };
    std::vector<ChildWithZIndex> sorted_children;
    sorted_children.reserve(children.size());
    
    size_t order = 0;
    for (const auto& child : children) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
            continue;
        }
        const auto& child_style = child->getComputedStyle();
        ChildWithZIndex item{};
        item.child = child;
        item.z_index = child_style.z_index;
        item.original_order = order++;
        sorted_children.push_back(item);
    }
    
    // 鎸?z-index 鍗囧簭鎺掑簭锛岀浉鍚?z-index 淇濇寔 DOM 椤哄簭
    std::sort(sorted_children.begin(), sorted_children.end(),
              [](const ChildWithZIndex& a, const ChildWithZIndex& b) {
                  if (a.z_index != b.z_index) {
                      return a.z_index < b.z_index;
                  }
                  return a.original_order < b.original_order;
              });
    
    for (const auto& item : sorted_children) {
        // 璺宠繃宸茬粡鍦ㄥ鍣ㄥ眰缁樺埗杩囩殑 inline 鍏冪礌
        if (item.child->getAttribute("__inline_rendered__") == "1") {
            item.child->setAttribute("__inline_rendered__", "");  // 娓呴櫎鏍囪
            continue;
        }
        const layout::LayoutNode* child_layout = nullptr;
        if (layout_engine_) {
            child_layout = layout_engine_->getLayout(item.child);
        }
        buildDisplayListNode(item.child, child_layout, builder);
    }
    
    // 鎭㈠婊氬姩鍋忕Щ
    if (applied_scroll_translate) {
        builder.popTranslate();
    }

    // 5. 婊氬姩鏉℃覆鏌擄紙鍦ㄥ瓙鑺傜偣涔嬪悗缁樺埗锛岀‘淇濇粴鍔ㄦ潯鍦ㄥ唴瀹逛箣涓婏級
    if (is_scroll_container && layout_node && node_rect.width > 0.0f && node_rect.height > 0.0f) {
        // 璁＄畻鍐呭楂樺害锛堟墍鏈夊瓙鍏冪礌鐨勬渶澶у簳閮ㄤ綅缃級
        float content_bottom = node_rect.y;
        for (const auto& child : node->getChildren()) {
            if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                continue;
            }
            const layout::LayoutNode* child_layout = layout_engine_ ? layout_engine_->getLayout(child) : nullptr;
            if (child_layout) {
                float child_bottom = child_layout->layout.position[1] + child_layout->layout.dimensions[1];
                if (child_bottom > content_bottom) {
                    content_bottom = child_bottom;
                }
            }
        }
        
        float content_height = content_bottom - node_rect.y;
        float visible_height = node_rect.height;
        
        // 鍙湁褰撳唴瀹归珮搴﹀ぇ浜庡彲瑙嗛珮搴︽椂鎵嶆樉绀烘粴鍔ㄦ潯
        if (content_height > visible_height + 1.0f) {
            // 婊氬姩鏉″弬鏁?
            constexpr float kScrollbarWidth = 8.0f;
            constexpr float kScrollbarMinThumbHeight = 20.0f;
            constexpr float kScrollbarPadding = 2.0f;
            
            // 婊氬姩鏉¤建閬撲綅缃紙鍦ㄥ鍣ㄥ彸渚э級
            Rect track_rect{};
            track_rect.x = node_rect.x + node_rect.width - kScrollbarWidth - kScrollbarPadding;
            track_rect.y = node_rect.y + kScrollbarPadding;
            track_rect.width = kScrollbarWidth;
            track_rect.height = node_rect.height - kScrollbarPadding * 2.0f;
            
            // 缁樺埗婊氬姩鏉¤建閬擄紙鍗婇€忔槑鐏拌壊鑳屾櫙锛?
            Color track_color{};
            track_color.r = 0.0f;
            track_color.g = 0.0f;
            track_color.b = 0.0f;
            track_color.a = 0.1f;
            builder.addRoundedRect(track_rect, track_color, kScrollbarWidth * 0.5f);
            
            // 璁＄畻婊戝潡楂樺害鍜屼綅缃?
            float thumb_height_ratio = visible_height / content_height;
            float thumb_height = std::max(kScrollbarMinThumbHeight, track_rect.height * thumb_height_ratio);
            
            // 浠庤妭鐐硅幏鍙栧綋鍓嶆粴鍔ㄤ綅缃?
            float scroll_position = node->getScrollY();
            float max_scroll = content_height - visible_height;
            float scroll_ratio = (max_scroll > 0.0f) ? (scroll_position / max_scroll) : 0.0f;
            scroll_ratio = std::clamp(scroll_ratio, 0.0f, 1.0f);
            
            float thumb_y = track_rect.y + (track_rect.height - thumb_height) * scroll_ratio;
            
            Rect thumb_rect{};
            thumb_rect.x = track_rect.x;
            thumb_rect.y = thumb_y;
            thumb_rect.width = kScrollbarWidth;
            thumb_rect.height = thumb_height;
            
            // 缁樺埗婊戝潡锛堝崐閫忔槑娣辩伆鑹诧級
            Color thumb_color{};
            thumb_color.r = 0.4f;
            thumb_color.g = 0.4f;
            thumb_color.b = 0.4f;
            thumb_color.a = 0.5f;
            builder.addRoundedRect(thumb_rect, thumb_color, kScrollbarWidth * 0.5f);
        }
    }

    if (pushed_layer_node && !layer_stack_.empty()) {
        layer_stack_.pop_back();
    }

}

bool Painter::isNodeInDirtyRect(const layout::LayoutNode* layout_node) const {
    if (!layout_node || current_dirty_rect_.isEmpty()) {
        return true;
    }

    Rect rect{};
    rect.x = layout_node->layout.position[0];
    rect.y = layout_node->layout.position[1];
    rect.width = layout_node->layout.dimensions[0];
    rect.height = layout_node->layout.dimensions[1];
    return isRectInDirtyRect(rect);
}

bool Painter::isRectInDirtyRect(const Rect& rect) const {
    if (current_dirty_rect_.isEmpty()) {
        return true;
    }

    float node_x = rect.x;
    float node_y = rect.y;
    float node_w = rect.width;
    float node_h = rect.height;

    float dirty_x = current_dirty_rect_.x;
    float dirty_y = current_dirty_rect_.y;
    float dirty_w = current_dirty_rect_.width;
    float dirty_h = current_dirty_rect_.height;

    bool x_overlap = (node_x < dirty_x + dirty_w) && (node_x + node_w > dirty_x);
    bool y_overlap = (node_y < dirty_y + dirty_h) && (node_y + node_h > dirty_y);

    return x_overlap && y_overlap;
}

} // namespace dong::render
