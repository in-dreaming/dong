#include "painter.hpp"
#include <cstring>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <sstream>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include "../core/log.h"
#include "../core/profiler.h"
namespace dong::render {


namespace {

// CSS 颜色解析器（正式版）：支�?#rgb/#rgba/#rrggbb/#rrggbbaa �?rgb()/rgba() 子集
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
        // 去首尾空�?
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

    // 预处理：去空白并转小�?
    std::string s = css;
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch); }), s.end());
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    // 默认值：不合法时给一个可见的浅灰色，方便调试
    r = g = b = 240;
    a = 255;

    if (s.empty() || s == "transparent") {
        r = g = b = 0;
        a = 0;
        return;
    }

    // 十六进制形式
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

    // 命名颜色（子集），其他未覆盖的颜色名可以按需要继续扩�?
    if (s == "white")      { r = g = b = 255; a = 255; return; }
    if (s == "black")      { r = g = b = 0;   a = 255; return; }
    if (s == "red")        { r = 255; g = 0;   b = 0;   a = 255; return; }
    if (s == "green")      { r = 0;   g = 128; b = 0;   a = 255; return; }
    if (s == "blue")       { r = 0;   g = 0;   b = 255; a = 255; return; }
    if (s == "gray" || s == "grey") { r = g = b = 128; a = 255; return; }
    if (s == "lightgray" || s == "lightgrey") { r = g = b = 211; a = 255; return; }

    // 其它情况保留默认浅灰，方便后续调试定位未实现的颜色格�?
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

// 辅助函数：根据 ComputedStyle 填充 DrawGlyphRunData 的 text-shadow 属性
static void fillTextShadow(DrawGlyphRunData& glyph_run, const dong::dom::ComputedStyle& style) {
    if (style.text_shadow_offset_x != 0.0f || style.text_shadow_offset_y != 0.0f ||
        style.text_shadow_blur != 0.0f || !style.text_shadow_color.empty()) {
        glyph_run.has_text_shadow = true;
        glyph_run.text_shadow_offset_x = style.text_shadow_offset_x;
        glyph_run.text_shadow_offset_y = style.text_shadow_offset_y;
        glyph_run.text_shadow_blur = style.text_shadow_blur;
        if (!style.text_shadow_color.empty()) {
            glyph_run.text_shadow_color = makeColorFromCss(style.text_shadow_color);
        } else {
            // 默认阴影颜色为黑色
            glyph_run.text_shadow_color = Color{0.0f, 0.0f, 0.0f, 1.0f};
        }
    }
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

// 简单的文本宽度估算（后续用 HarfBuzz 替换�?
static float estimateTextWidth(const std::string& text, float font_size) {
    return static_cast<float>(text.size()) * font_size * 0.55f;
}

static bool shouldClipOverflow(const std::string& overflow_value) {
    std::string lowered = overflow_value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    // CSS 语义：overflow:hidden/scroll/auto 都需要建立裁剪上下文
    return lowered == "hidden" || lowered == "scroll" || lowered == "auto";
}

static bool isScrollOverflow(const std::string& overflow_value) {
    std::string lowered = overflow_value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lowered == "scroll" || lowered == "auto";
}


} // anonymous namespace

Painter::Painter(RenderSurface* surface)
    : surface_(surface), layout_engine_(nullptr) {
}

Painter::~Painter() {
}

const DisplayList& Painter::buildDisplayList(const dom::DOMNodePtr& root, layout::Engine* layout_engine) {
    DONG_PROFILE_FUNCTION();
    
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
    
    // visibility: hidden Ԫ�ز��ɼ�������ռλ
    // �� display: none ��ͬ��visibility: hidden ��Ȼ���벼��
    const bool is_hidden = (style.visibility == "hidden" || style.visibility == "collapse");

    if (node->getType() == dom::DOMNode::NodeType::TEXT) return;

    const std::string tag = node->getTagName();

    // Dirty rect 优化
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

    const float builder_tx = builder.getTranslateX();
    const float builder_ty = builder.getTranslateY();

    const bool should_apply_clip = has_layout_rect &&
        (shouldClipOverflow(style.overflow) || shouldClipOverflow(style.overflow_x) || shouldClipOverflow(style.overflow_y));


    DisplayListBuilder::ScopedLayer opacity_scope;
    float clamped_opacity = std::clamp(style.opacity, 0.0f, 1.0f);

    // layer bounds 用于 isolated layer 的采样与合成，需要落在“最终屏幕坐标系”中。
    // 注意：滚动是通过 DisplayListBuilder 的 translate 实现的，因此这里必须把 translate 计入 bounds。
    Rect layer_bounds = node_rect;
    if (!has_layout_rect && surface_) {
        layer_bounds.x = 0.0f;
        layer_bounds.y = 0.0f;
        layer_bounds.width = static_cast<float>(surface_->getWidth());
        layer_bounds.height = static_cast<float>(surface_->getHeight());
    }
    layer_bounds.x += builder_tx;
    layer_bounds.y += builder_ty;


    const bool is_scroll_container =
        should_apply_clip &&
        (isScrollOverflow(style.overflow) || isScrollOverflow(style.overflow_x) || isScrollOverflow(style.overflow_y));


    const bool has_transform =
        (style.transform_translate_x != 0.0f || style.transform_translate_y != 0.0f ||
         style.transform_scale_x != 1.0f || style.transform_scale_y != 1.0f ||
         style.transform_rotate != 0.0f || style.transform_skew_x != 0.0f || style.transform_skew_y != 0.0f);

    bool has_isolation = (style.isolation_isolate || is_scroll_container || has_transform) &&
                         (layer_bounds.width > 0.0f && layer_bounds.height > 0.0f);
    bool needs_layer = has_isolation || clamped_opacity < 0.999f;
    
    // 强制为有transform的元素创建layer，确保transform能被应用
    if (has_transform) {
        needs_layer = true;
        has_isolation = true;
    }
    int parent_layer_index = layer_stack_.empty() ? -1 : layer_stack_.back();
    bool pushed_layer_node = false;
    if (needs_layer) {
        uint64_t layer_id = reinterpret_cast<uint64_t>(node.get());
        bool layer_dirty = node->isLayoutDirty();

        // 滚动容器始终标记为脏，因为滚动内容会随着滚动位置改变
        if (is_scroll_container) {
            layer_dirty = true;
        }

        // 如果该 layer 处在 scroll translate 之下（例如：滚动容器内部的 transform 元素），
        // 它的栅格结果依赖当前滚动偏移；否则缓存会导致“控件固定不动”。
        if (!layer_dirty && (builder_tx != 0.0f || builder_ty != 0.0f)) {
            layer_dirty = true;
        }

        if (!layer_dirty && use_dirty_rect_ && !current_dirty_rect_.isEmpty()) {
            layer_dirty = isRectInDirtyRect(layer_bounds);
        }


        // �?LayerTree 中记录这一�?
        LayerNode layer_node;
        layer_node.id = layer_id;
        layer_node.type = has_isolation ? LayerType::Surface : LayerType::Opacity;
        layer_node.bounds = layer_bounds;
        layer_node.opacity = clamped_opacity;
        layer_node.transform = LayerTransform::identity();
        
        // 构建完整的 2D 变换矩阵（与 CSS transform 语义对齐）：T * R * K * S
        // - 右侧先作用：先 Scale，再 Skew，再 Rotate，最后 Translate
        // - 坐标系：DisplayList/屏幕是 Y 向下，CSS 正角度在视觉上是顺时针
        // - 需要考虑 transform-origin（也在“全局像素坐标系”里）
        
        const float sx = style.transform_scale_x;
        const float sy = style.transform_scale_y;
        const float tx = style.transform_translate_x;
        const float ty = style.transform_translate_y;
        const float angle_rad = style.transform_rotate * 3.14159265358979f / 180.0f;
        const float skew_x_rad = style.transform_skew_x * 3.14159265358979f / 180.0f;
        const float skew_y_rad = style.transform_skew_y * 3.14159265358979f / 180.0f;

        // Transform origin（必须落在全局坐标系里，否则会绕 (0,0) 错误旋转/倾斜）
        const float origin_rel_x = layer_bounds.width * style.transform_origin_x / 100.0f;
        const float origin_rel_y = layer_bounds.height * style.transform_origin_y / 100.0f;
        const float origin_x = layer_bounds.x + origin_rel_x;
        const float origin_y = layer_bounds.y + origin_rel_y;

        const float cos_r = cosf(angle_rad);
        const float sin_r = sinf(angle_rad);
        const float tan_kx = tanf(skew_x_rad);
        const float tan_ky = tanf(skew_y_rad);

        // 2x2 部分：A = R * Ky * Kx * S
        // 其中：
        //   S  = [sx 0; 0 sy]
        //   Kx = [1 tan(kx); 0 1]
        //   Ky = [1 0; tan(ky) 1]
        //   R  = [cos -sin; sin cos]
        const float a00 = sx * (cos_r - sin_r * tan_ky);
        const float a01 = sy * (cos_r * tan_kx - sin_r);
        const float a10 = sx * (sin_r + cos_r * tan_ky);
        const float a11 = sy * (sin_r * tan_kx + cos_r);


        // 2x3 平移部分：M = T(origin+translate) * A * T(-origin)
        const float final_tx = origin_x + tx - (a00 * origin_x + a01 * origin_y);
        const float final_ty = origin_y + ty - (a10 * origin_x + a11 * origin_y);

        layer_node.transform.m[0] = a00;      // m00
        layer_node.transform.m[1] = a01;      // m01
        layer_node.transform.m[2] = final_tx; // m02 (translate_x)
        layer_node.transform.m[3] = a10;      // m10
        layer_node.transform.m[4] = a11;      // m11
        layer_node.transform.m[5] = final_ty; // m12 (translate_y)

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

    // 1. ��������Ӱ (visibility: hidden ʱ��������)
    if (layout_node && !is_hidden) {
        Rect rect = node_rect;

        // root/html/body 填满 viewport
        if ((tag.empty() || tag == "html" || tag == "body") && surface_) {
            rect.x = 0.0f;
            rect.y = 0.0f;
            rect.width = static_cast<float>(surface_->getWidth());
            rect.height = static_cast<float>(surface_->getHeight());
        }

        // 1.1 background box helpers (clip/origin/attachment)
        const float bw = style.border_width;
        const float radius = style.border_radius;
        const float viewport_w = surface_ ? static_cast<float>(surface_->getWidth()) : 800.0f;
        const float viewport_h = surface_ ? static_cast<float>(surface_->getHeight()) : 600.0f;

        auto toLowerCopy = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            return s;
        };

        auto resolvePx = [&](const dong::dom::CSSValue& v, float parent_size) -> float {
            return v.resolvePixels(parent_size, 16.0f, viewport_w, viewport_h);
        };

        const float pad_l = resolvePx(style.padding_left, rect.width);
        const float pad_r = resolvePx(style.padding_right, rect.width);
        const float pad_t = resolvePx(style.padding_top, rect.height);
        const float pad_b = resolvePx(style.padding_bottom, rect.height);
        const float min_pad = std::min(std::min(pad_l, pad_r), std::min(pad_t, pad_b));


        Rect bg_border_box = rect;
        Rect bg_padding_box = rect;
        bg_padding_box.x += bw;
        bg_padding_box.y += bw;
        bg_padding_box.width = std::max(0.0f, bg_padding_box.width - 2.0f * bw);
        bg_padding_box.height = std::max(0.0f, bg_padding_box.height - 2.0f * bw);

        Rect bg_content_box = bg_padding_box;
        bg_content_box.x += pad_l;
        bg_content_box.y += pad_t;
        bg_content_box.width = std::max(0.0f, bg_content_box.width - pad_l - pad_r);
        bg_content_box.height = std::max(0.0f, bg_content_box.height - pad_t - pad_b);

        auto pickBox = [&](const std::string& keyword) -> Rect {
            if (keyword == "padding-box") return bg_padding_box;
            if (keyword == "content-box") return bg_content_box;
            return bg_border_box;
        };

        const std::string bg_clip_kw = toLowerCopy(collapseWhitespace(style.background_clip));
        const std::string bg_origin_kw = toLowerCopy(collapseWhitespace(style.background_origin));
        const std::string bg_attach_kw = toLowerCopy(collapseWhitespace(style.background_attachment));

        Rect bg_clip_rect = pickBox(bg_clip_kw);
        Rect bg_origin_rect = pickBox(bg_origin_kw);
        if (bg_attach_kw == "fixed") {
            bg_origin_rect = Rect{0.0f, 0.0f, viewport_w, viewport_h};
        }

        // 1.2 box-shadow（先画在背景之下�?


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

                // 使用带模糊的阴影绘制
                float blur = shadow.blur_radius;
                if (blur > 0.0f) {
                    builder.addShadow(shadow_rect, sc, radius, blur);
                } else {
                    builder.addRoundedRect(shadow_rect, sc, radius);
                }
            }
        }

        // 1.2 边框和背景填充
        bool has_border = bw > 0.0f && style.border_style != "none";
        bool has_background = !style.background_color.empty() && style.background_color != "transparent";

        
        // DEBUG: 打印背景信息
        if (radius > 0.0f || has_background || has_border) {
            DONG_LOG_DEBUG("[PAINT_BG] bg_color='%s' has_bg=%d radius=%.1f has_border=%d rect=(%.0f,%.0f,%.0f,%.0f)",
                style.background_color.c_str(), has_background, radius, has_border,
                rect.x, rect.y, rect.width, rect.height);
        }

        // DEBUG（可选）：定位 section 的最终布局位置，避免“后一个 section 覆盖前一个”的错位问题
        // 用法：在运行前设置环境变量 DONG_DEBUG_SECTION=1
        if (std::getenv("DONG_DEBUG_SECTION")) {
            const std::string cls = node->hasAttribute("class") ? node->getAttribute("class") : std::string();
            if (!cls.empty() && cls.find("section") != std::string::npos && radius > 0.0f) {
                std::string title;
                for (const auto& ch : node->getChildren()) {
                    if (!ch || ch->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
                    const std::string ch_cls = ch->hasAttribute("class") ? ch->getAttribute("class") : std::string();
                    if (ch_cls.find("section-title") != std::string::npos) {
                        title = ch->getTextContent();
                        break;
                    }
                }
                DONG_LOG_INFO("[PAINT_SECTION] class='%s' title='%s' rect=(%.0f,%.0f,%.0f,%.0f)",
                    cls.c_str(), title.c_str(), rect.x, rect.y, rect.width, rect.height);
            }
        }

        
        if (rect.width > 0.0f && rect.height > 0.0f) {
            if (radius > 0.0f) {
                // 圆角情况：遵循浏览器绘制顺序：先背景，后边框。
                // 另外：圆角边框不能用 "填充整块圆角矩形" 来近似，否则会把整个内容区域都染上边框颜色。

                if (has_background) {
                    Color bg_color = makeColorFromCss(style.background_color);
                    DONG_LOG_DEBUG("[PAINT_BG] Drawing rounded rect: color=(%f,%f,%f,%f)", bg_color.r, bg_color.g, bg_color.b, bg_color.a);

                    Rect inner_rect = bg_clip_rect;
                    float inset_for_radius = 0.0f;
                    if (bg_clip_kw == "padding-box") {
                        inset_for_radius = bw;
                    } else if (bg_clip_kw == "content-box") {
                        inset_for_radius = bw + min_pad;
                    }
                    float inner_radius = std::max(0.0f, radius - inset_for_radius);

                    if (inner_rect.width > 0.0f && inner_rect.height > 0.0f) {
                        builder.addRoundedRect(inner_rect, bg_color, inner_radius);
                    }
                }

                if (has_border) {
                    auto toLowerCopy = [](std::string s) {
                        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                            return static_cast<char>(std::tolower(c));
                        });
                        return s;
                    };

                    const std::string bstyle = toLowerCopy(collapseWhitespace(style.border_style));
                    const bool bevel = (bstyle == "outset" || bstyle == "inset");

                    if (!bevel) {
                        // Rounded border ring: draw analytically as a stroked rounded-rect.
                        // This avoids the "square inner corner" artifact from "clip + 4 rects".
                        Color border_color = makeColorFromCss(style.border_color);
                        builder.addRoundedRect(rect, border_color, radius, bw);
                    } else {


                        auto clamp01 = [](float v) {
                            return std::clamp(v, 0.0f, 1.0f);
                        };
                        auto lighten = [&](Color c, float a) {
                            c.r = clamp01(c.r + (1.0f - c.r) * a);
                            c.g = clamp01(c.g + (1.0f - c.g) * a);
                            c.b = clamp01(c.b + (1.0f - c.b) * a);
                            return c;
                        };
                        auto darken = [&](Color c, float a) {
                            c.r = clamp01(c.r * (1.0f - a));
                            c.g = clamp01(c.g * (1.0f - a));
                            c.b = clamp01(c.b * (1.0f - a));
                            return c;
                        };

                        const bool is_outset = (bstyle == "outset");

                        // Prefer using the element background as the base for bevel shading;
                        // this matches browsers better when author sets background-color but not border.
                        Color base = has_background ? makeColorFromCss(style.background_color)
                                                    : makeColorFromCss(style.border_color);

                        Color c_tl = is_outset ? lighten(base, 0.25f) : darken(base, 0.25f);
                        Color c_br = is_outset ? darken(base, 0.25f) : lighten(base, 0.25f);

                        DisplayListBuilder::ScopedClip border_clip = builder.pushRoundedClip(rect, radius);
                        Rect top_border{rect.x, rect.y, rect.width, bw};
                        Rect bottom_border{rect.x, rect.y + rect.height - bw, rect.width, bw};
                        Rect left_border{rect.x, rect.y + bw, bw, rect.height - 2 * bw};
                        Rect right_border{rect.x + rect.width - bw, rect.y + bw, bw, rect.height - 2 * bw};
                        builder.addRect(top_border, c_tl);
                        builder.addRect(left_border, c_tl);
                        builder.addRect(bottom_border, c_br);
                        builder.addRect(right_border, c_br);
                    }
                }

            } else {

                // 非圆角情况：先背景，后边框
                if (has_background) {
                    Color bg_color = makeColorFromCss(style.background_color);
                    builder.addRect(bg_clip_rect, bg_color);
                }

                if (has_border) {
                    auto toLowerCopy = [](std::string s) {
                        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                            return static_cast<char>(std::tolower(c));
                        });
                        return s;
                    };

                    const std::string bstyle = toLowerCopy(collapseWhitespace(style.border_style));
                    const bool bevel = (bstyle == "outset" || bstyle == "inset");

                    Rect top_border{rect.x, rect.y, rect.width, bw};
                    Rect bottom_border{rect.x, rect.y + rect.height - bw, rect.width, bw};
                    Rect left_border{rect.x, rect.y + bw, bw, rect.height - 2 * bw};
                    Rect right_border{rect.x + rect.width - bw, rect.y + bw, bw, rect.height - 2 * bw};

                    if (!bevel) {
                        Color border_color = makeColorFromCss(style.border_color);
                        builder.addRect(top_border, border_color);
                        builder.addRect(bottom_border, border_color);
                        builder.addRect(left_border, border_color);
                        builder.addRect(right_border, border_color);
                    } else {
                        auto clamp01 = [](float v) {
                            return std::clamp(v, 0.0f, 1.0f);
                        };
                        auto lighten = [&](Color c, float a) {
                            c.r = clamp01(c.r + (1.0f - c.r) * a);
                            c.g = clamp01(c.g + (1.0f - c.g) * a);
                            c.b = clamp01(c.b + (1.0f - c.b) * a);
                            return c;
                        };
                        auto darken = [&](Color c, float a) {
                            c.r = clamp01(c.r * (1.0f - a));
                            c.g = clamp01(c.g * (1.0f - a));
                            c.b = clamp01(c.b * (1.0f - a));
                            return c;
                        };

                        const bool is_outset = (bstyle == "outset");
                        Color base = has_background ? makeColorFromCss(style.background_color)
                                                    : makeColorFromCss(style.border_color);
                        Color c_tl = is_outset ? lighten(base, 0.25f) : darken(base, 0.25f);
                        Color c_br = is_outset ? darken(base, 0.25f) : lighten(base, 0.25f);

                        builder.addRect(top_border, c_tl);
                        builder.addRect(left_border, c_tl);
                        builder.addRect(bottom_border, c_br);
                        builder.addRect(right_border, c_br);
                    }
                }

            }
        }

        // 1.2.5 背景图片
        if (!style.background_image.empty() && rect.width > 0.0f && rect.height > 0.0f) {
            // 解析 url(...) 格式
            std::string image_url = style.background_image;
            if (image_url.find("url(") == 0) {
                size_t start = 4;
                size_t end = image_url.rfind(")");
                if (end != std::string::npos && end > start) {
                    image_url = image_url.substr(start, end - start);
                    // 去除引号
                    if (!image_url.empty() && (image_url[0] == '"' || image_url[0] == '\'')) {
                        image_url = image_url.substr(1);
                    }
                    if (!image_url.empty() && (image_url.back() == '"' || image_url.back() == '\'')) {
                        image_url.pop_back();
                    }
                }
            }
            
            if (!image_url.empty()) {
                auto toLowerCopy = [](std::string s) {
                    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    });
                    return s;
                };

                const std::string bg_size = toLowerCopy(collapseWhitespace(style.background_size));
                const std::string bg_repeat = toLowerCopy(collapseWhitespace(style.background_repeat));
                const std::string bg_pos = toLowerCopy(collapseWhitespace(style.background_position));

                // Backgrounds are clipped to background-clip.
                float bg_clip_radius = radius;
                if (bg_clip_kw == "padding-box") {
                    bg_clip_radius = std::max(0.0f, radius - bw);
                } else if (bg_clip_kw == "content-box") {
                    bg_clip_radius = std::max(0.0f, radius - bw - min_pad);
                }

                DisplayListBuilder::ScopedClip bg_clip;
                if (bg_clip_radius > 0.0f) {
                    bg_clip = builder.pushRoundedClip(bg_clip_rect, bg_clip_radius);
                } else {
                    bg_clip = builder.pushClipRect(bg_clip_rect);
                }


                if (bg_size.find("cover") != std::string::npos) {
                    builder.addImage(bg_clip_rect, image_url, 1.0f, ImageFitMode::Cover);
                } else if (bg_size.find("contain") != std::string::npos) {
                    builder.addImage(bg_clip_rect, image_url, 1.0f, ImageFitMode::Contain);
                } else {

                    // Handle explicit background-size like "96px 96px" (used by the tile test).
                    float tile_w = 0.0f;
                    float tile_h = 0.0f;

                    auto parsePx = [](std::string token) -> float {
                        token.erase(std::remove_if(token.begin(), token.end(), [](unsigned char c) {
                            return std::isspace(c);
                        }), token.end());
                        auto pos = token.find("px");
                        if (pos != std::string::npos) {
                            token = token.substr(0, pos);
                        }
                        try {
                            return std::stof(token);
                        } catch (...) {
                            return 0.0f;
                        }
                    };

                    {
                        std::istringstream iss(bg_size);
                        std::string t1;
                        std::string t2;
                        iss >> t1 >> t2;
                        if (!t1.empty()) {
                            tile_w = parsePx(t1);
                            tile_h = !t2.empty() ? parsePx(t2) : tile_w;
                        }
                    }

                    const bool repeat = (bg_repeat.find("repeat") != std::string::npos) && (bg_repeat.find("no-repeat") == std::string::npos);

                    // We only implement the subset used by tests:
                    // - cover/contain: centered
                    // - explicit size + repeat: tile from top-left
                    (void)bg_pos;

                    if (repeat && tile_w > 0.0f && tile_h > 0.0f) {
                        auto alignStart = [](float clip_start, float origin_start, float step) -> float {
                            if (step <= 0.0f) return clip_start;
                            const float n = std::floor((clip_start - origin_start) / step);
                            return origin_start + n * step;
                        };

                        const float x0 = alignStart(bg_clip_rect.x, bg_origin_rect.x, tile_w);
                        const float y0 = alignStart(bg_clip_rect.y, bg_origin_rect.y, tile_h);
                        const float x1 = bg_clip_rect.x + bg_clip_rect.width;
                        const float y1 = bg_clip_rect.y + bg_clip_rect.height;

                        for (float y = y0; y < y1; y += tile_h) {
                            for (float x = x0; x < x1; x += tile_w) {
                                Rect tile{ x, y, tile_w, tile_h };
                                builder.addImage(tile, image_url, 1.0f, ImageFitMode::Fill);
                            }
                        }
                    } else {
                        // Fallback: stretch to the clip rect.
                        builder.addImage(bg_clip_rect, image_url, 1.0f, ImageFitMode::Fill);
                    }

                }
            }

        }

        // 1.4 outline 绘制（在边框外，不影响布局）
        if (style.outline_width > 0.0f && style.outline_style != "none" && rect.width > 0.0f && rect.height > 0.0f) {
            Color outline_color = makeColorFromCss(style.outline_color);
            float ow = style.outline_width;
            float offset = style.outline_offset;
            
            // outline 绘制在边框外
            Rect outline_rect{};
            outline_rect.x = rect.x - ow - offset;
            outline_rect.y = rect.y - ow - offset;
            outline_rect.width = rect.width + (ow + offset) * 2.0f;
            outline_rect.height = rect.height + (ow + offset) * 2.0f;
            
            // 上边
            Rect top_outline{outline_rect.x, outline_rect.y, outline_rect.width, ow};
            // 下边
            Rect bottom_outline{outline_rect.x, outline_rect.y + outline_rect.height - ow, outline_rect.width, ow};
            // 左边
            Rect left_outline{outline_rect.x, outline_rect.y + ow, ow, outline_rect.height - 2 * ow};
            // 右边
            Rect right_outline{outline_rect.x + outline_rect.width - ow, outline_rect.y + ow, ow, outline_rect.height - 2 * ow};
            
            builder.addRect(top_outline, outline_color);
            builder.addRect(bottom_outline, outline_color);
            builder.addRect(left_outline, outline_color);
            builder.addRect(right_outline, outline_color);
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

    paintMediaElements(node, layout_node, tag, style, is_hidden, builder);

    paintTextAndInput(node, layout_node, tag, style, is_hidden, builder);

    paintChildrenAndOverlays(node, layout_node, node_rect, has_layout_rect, is_scroll_container, builder);
    // 4. 渲染 ::before 伪元素


    if (node->hasPseudoElements()) {
        auto pseudo_before = node->getPseudoBefore();
        if (pseudo_before) {
            renderPseudoElement(pseudo_before, node_rect, builder);
        }
    }

    // 5. 递归子节点（按 z-index 排序）
    const auto& children = node->getChildren();
    
    // 维护滚动容器的 client/content 尺寸（用于 scrollTo/scrollBy clamp 与滚动条绘制）
    if (is_scroll_container && has_layout_rect) {
        float content_bottom = node_rect.y;
        for (const auto& child : children) {
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
        if (content_height < 0.0f) content_height = 0.0f;

        node->setClientRect(node_rect.y, node_rect.x, node_rect.width, node_rect.height);
        node->setContentSize(node_rect.width, content_height);
    }

    // 如果是滚动容器，应用滚动偏移到子元素
    bool applied_scroll_translate = false;
    if (is_scroll_container) {
        float scroll_x = node->getScrollX();
        float scroll_y = node->getScrollY();
        if (scroll_x != 0.0f || scroll_y != 0.0f) {
            builder.pushTranslate(-scroll_x, -scroll_y);
            applied_scroll_translate = true;
        }
    }

    
    // 收集需要绘制的子元素及�?z-index
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
    
    // �?z-index 升序排序，相�?z-index 保持 DOM 顺序
    std::sort(sorted_children.begin(), sorted_children.end(),
              [](const ChildWithZIndex& a, const ChildWithZIndex& b) {
                  if (a.z_index != b.z_index) {
                      return a.z_index < b.z_index;
                  }
                  return a.original_order < b.original_order;
              });
    
    for (const auto& item : sorted_children) {
        // 跳过已经在容器层绘制过的 inline 元素
        if (item.child->getAttribute("__inline_rendered__") == "1") {
            item.child->setAttribute("__inline_rendered__", "");  // 清除标记
            continue;
        }
        const layout::LayoutNode* child_layout = nullptr;
        if (layout_engine_) {
            child_layout = layout_engine_->getLayout(item.child);
        }
        buildDisplayListNode(item.child, child_layout, builder);
    }
    
    // 恢复滚动偏移
    if (applied_scroll_translate) {
        builder.popTranslate();
    }

    // 6. 渲染 ::after 伪元素
    if (node->hasPseudoElements()) {
        auto pseudo_after = node->getPseudoAfter();
        if (pseudo_after) {
            renderPseudoElement(pseudo_after, node_rect, builder);
        }
    }

    // 7. 滚动条渲染（在子节点之后绘制，确保滚动条在内容之上）
    if (is_scroll_container && layout_node && node_rect.width > 0.0f && node_rect.height > 0.0f) {
        // 计算内容高度（所有子元素的最大底部位置）
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
        
        // 只有当内容高度大于可视高度时才显示滚动条
        if (content_height > visible_height + 1.0f) {
            // 滚动条参�?
            constexpr float kScrollbarWidth = 8.0f;
            constexpr float kScrollbarMinThumbHeight = 20.0f;
            constexpr float kScrollbarPadding = 2.0f;
            
            // 滚动条轨道位置（在容器右侧）
            Rect track_rect{};
            track_rect.x = node_rect.x + node_rect.width - kScrollbarWidth - kScrollbarPadding;
            track_rect.y = node_rect.y + kScrollbarPadding;
            track_rect.width = kScrollbarWidth;
            track_rect.height = node_rect.height - kScrollbarPadding * 2.0f;
            
            // 绘制滚动条轨道（半透明灰色背景�?
            Color track_color{};
            track_color.r = 0.0f;
            track_color.g = 0.0f;
            track_color.b = 0.0f;
            track_color.a = 0.1f;
            builder.addRoundedRect(track_rect, track_color, kScrollbarWidth * 0.5f);
            
            // 计算滑块高度和位�?
            float thumb_height_ratio = visible_height / content_height;
            float thumb_height = std::max(kScrollbarMinThumbHeight, track_rect.height * thumb_height_ratio);
            
            // 从节点获取当前滚动位�?
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
            
            // 绘制滑块（半透明深灰色）
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

void Painter::renderPseudoElement(const dom::DOMNodePtr& pseudo,
                                   const Rect& parent_rect,
                                   DisplayListBuilder& builder) {
    if (!pseudo) return;
    
    const auto& style = pseudo->getComputedStyle();
    if (style.display == "none" || style.content.empty()) return;
    
    // Calculate pseudo-element position (relative to parent)
    Rect rect = parent_rect;
    
    // Apply width/height if specified
    if (!style.width.isAuto() && style.width.value > 0.0f) {
        rect.width = style.width.value;
    }
    if (!style.height.isAuto() && style.height.value > 0.0f) {
        rect.height = style.height.value;
    }
    
    // For ::before, position at the start; for ::after, at the end
    // This is simplified - real implementation would integrate with layout
    if (style.pseudo_type == "before") {
        // Position at the start of parent content
        rect.x = parent_rect.x + style.margin_left.value;
        rect.y = parent_rect.y + style.margin_top.value;
    } else if (style.pseudo_type == "after") {
        // Position after parent content (simplified)
        rect.x = parent_rect.x + parent_rect.width - rect.width - style.margin_right.value;
        rect.y = parent_rect.y + style.margin_top.value;
    }
    
    // Apply padding
    float padding_left = style.padding_left.value;
    float padding_right = style.padding_right.value;
    float padding_top = style.padding_top.value;
    float padding_bottom = style.padding_bottom.value;
    
    // Draw background
    if (style.background_color != "transparent" && rect.width > 0.0f && rect.height > 0.0f) {
        Color c = makeColorFromCss(style.background_color);
        if (style.border_radius > 0.0f) {
            builder.addRoundedRect(rect, c, style.border_radius);
        } else {
            builder.addRect(rect, c);
        }
    }
    
    // Draw border
    if (style.border_width > 0.0f && style.border_style != "none") {
        Color border_color = makeColorFromCss(style.border_color);
        float bw = style.border_width;
        
        Rect top_border{rect.x, rect.y, rect.width, bw};
        Rect bottom_border{rect.x, rect.y + rect.height - bw, rect.width, bw};
        Rect left_border{rect.x, rect.y + bw, bw, rect.height - 2 * bw};
        Rect right_border{rect.x + rect.width - bw, rect.y + bw, bw, rect.height - 2 * bw};
        
        builder.addRect(top_border, border_color);
        builder.addRect(bottom_border, border_color);
        builder.addRect(left_border, border_color);
        builder.addRect(right_border, border_color);
    }
    
    // Draw content text
    if (!style.content.empty()) {
        Color text_color = makeColorFromCss(style.color);
        float font_size = style.font_size;
        
        // Position text inside the pseudo-element
        float text_x = rect.x + padding_left + style.border_width;
        float text_y = rect.y + padding_top + style.border_width;
        
        // Shape text using TextShapeRequest
        TextShapeRequest request;
        request.text = style.content;
        request.font_family = style.font_family;
        request.font_weight = style.font_weight;
        request.font_style = style.font_style;
        request.font_size = font_size;

        request.origin_x = text_x;
        request.origin_y = text_y;
        
        ShapedText shaped;
        if (text_shaper_.shape(request, shaped) && !shaped.glyphs.empty()) {
            float scale = shaped.scale_to_pixels;
            float ascent_px = shaped.ascent_units * scale;
            float baseline_y = text_y + ascent_px;
            float text_width_px = shaped.width_units * scale;
            float text_height_px = shaped.line_height_units * scale;
            
            DrawGlyphRunData glyph_run{};
            glyph_run.rect.x = text_x;
            glyph_run.rect.y = text_y;
            glyph_run.rect.width = text_width_px;
            glyph_run.rect.height = text_height_px;
            glyph_run.color = text_color;
            glyph_run.font_size = font_size;
            glyph_run.font_family = style.font_family;
            glyph_run.font_weight = style.font_weight;
            glyph_run.font_style = style.font_style;
            glyph_run.font_path = shaped.font_path;
            glyph_run.baseline_x = text_x;
            glyph_run.baseline_y = baseline_y;
            glyph_run.units_per_em = shaped.units_per_em;
            glyph_run.scale_to_pixels = shaped.scale_to_pixels;
            fillTextShadow(glyph_run, style);

            
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

} // namespace dong::render
