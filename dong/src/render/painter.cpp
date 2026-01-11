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

        // 1.1 box-shadow（先画在背景之下�?
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
        float bw = style.border_width;
        float radius = style.border_radius;
        bool has_border = bw > 0.0f && style.border_style != "none";
        bool has_background = !style.background_color.empty() && style.background_color != "transparent";
        
        // DEBUG: 打印背景信息
        if (radius > 0.0f || has_background || has_border) {
            DONG_LOG_INFO("[PAINT_BG] bg_color='%s' has_bg=%d radius=%.1f has_border=%d rect=(%.0f,%.0f,%.0f,%.0f)",
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
                // 圆角情况：先绘制边框圆角矩形，再绘制背景圆角矩形
                if (has_border) {
                    Color border_color = makeColorFromCss(style.border_color);
                    builder.addRoundedRect(rect, border_color, radius);
                }
                if (has_background) {
                    Color bg_color = makeColorFromCss(style.background_color);
                    DONG_LOG_INFO("[PAINT_BG] Drawing rounded rect: color=(%f,%f,%f,%f)", bg_color.r, bg_color.g, bg_color.b, bg_color.a);
                    // 内部背景矩形需要缩小 border_width
                    Rect inner_rect = rect;
                    if (has_border) {
                        inner_rect.x += bw;
                        inner_rect.y += bw;
                        inner_rect.width -= 2 * bw;
                        inner_rect.height -= 2 * bw;
                    }
                    float inner_radius = std::max(0.0f, radius - bw);
                    if (inner_rect.width > 0.0f && inner_rect.height > 0.0f) {
                        builder.addRoundedRect(inner_rect, bg_color, inner_radius);
                    }
                }
            } else {
                // 非圆角情况：先背景，后边框
                if (has_background) {
                    Color bg_color = makeColorFromCss(style.background_color);
                    builder.addRect(rect, bg_color);
                }
                if (has_border) {
                    Color border_color = makeColorFromCss(style.border_color);
                    // 绘制四条边框
                    Rect top_border{rect.x, rect.y, rect.width, bw};
                    Rect bottom_border{rect.x, rect.y + rect.height - bw, rect.width, bw};
                    Rect left_border{rect.x, rect.y + bw, bw, rect.height - 2 * bw};
                    Rect right_border{rect.x + rect.width - bw, rect.y + bw, bw, rect.height - 2 * bw};
                    builder.addRect(top_border, border_color);
                    builder.addRect(bottom_border, border_color);
                    builder.addRect(left_border, border_color);
                    builder.addRect(right_border, border_color);
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
                // TODO: 根据 background-size/repeat/position 计算实际绘制参数
                // 目前简化为 cover 模式
                builder.addImage(rect, image_url, 1.0f);
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

    // 2. ͼƬ (visibility: hidden ʱ����)
    if (layout_node && tag == "img" && !is_hidden) {
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

    // 3. 文本内容 (visibility: hidden 时跳过)
    if (layout_node && tag != "script" && tag != "style" && tag != "head" && tag != "img" && !is_hidden) {
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
                // 检查是否为 inline/inline-block 元素
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

        // 当有 inline 子元素时，需要按顺序处理每个子节点，避免文本重叠
        // 使用容器层完全接管混合内容的布局和绘�?
        // 注意：如果容器是 flex 布局，不应该使用混合内容路径，因�?flex 会正确计算子元素位置
        const bool is_flex_container = (style.display == "flex" || style.display == "inline-flex");
        if (has_text_child && has_inline_element_child && tag_prefers_text && !is_flex_container) {
            if (debug_class.find("overlay-row") != std::string::npos) {
                DONG_LOG_INFO("[Painter] overlay-row MIXED PATH raw_text='%s'", raw_text.c_str());
            }
            // 混合内容：有 TEXT 节点�?inline 元素
            // 按子节点顺序分别绘制每个内容，计算正确的起始 X 位置
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

            // 计算容器�?baseline 度量
            float container_baseline_offset = 0.0f;
            float container_ascent_px = 0.0f;
            float container_line_height_px = 0.0f;
            {
                TextShapeRequest req{};
                req.text = "X";  // 使用 X 作为基准字符
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

            // 按子节点顺序处理所有内容（TEXT �?inline ELEMENT�?
            for (const auto& child : node->getChildren()) {
                if (!child) continue;

                if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                    const auto& child_style = child->getComputedStyle();
                    
                    // inline-block 元素应该通过正常递归渲染，使用布局引擎计算的位置和尺寸
                    // 只有纯 inline 元素才在这里处理文本
                    if (child_style.display == "inline") {
                        // 获取 inline 元素的文本内容并直接在此绘制
                        std::string child_text;
                        for (const auto& grandchild : child->getChildren()) {
                            if (grandchild && grandchild->getType() == dom::DOMNode::NodeType::TEXT) {
                                child_text += grandchild->getTextContent();
                            }
                        }
                        child_text = collapseWhitespace(child_text);
                        
                        if (!child_text.empty()) {
                            // 使用 inline 元素的样式
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
                                
                                // 计算 inline 元素的 padding
                                float child_pad_left = child_style.padding_left.isPixel() ? child_style.padding_left.value : 0.0f;
                                float child_pad_right = child_style.padding_right.isPixel() ? child_style.padding_right.value : 0.0f;
                                
                                float text_x = x + pad_left + cumulative_x_offset + child_pad_left;
                                
                                // 绘制 inline 元素的背景（如果有）
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
                            
                            // 标记该 inline 元素已经在容器层绘制
                            child->setAttribute("__inline_rendered__", "1");
                        }
                    }
                    // inline-block 元素不在这里处理，让它们通过正常递归渲染
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

            // 应用 text-transform
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
                // capitalize 暂不支持（需要更复杂�?Unicode 处理�?
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

                // 使用 HarfBuzz 做一次完�?shaping，并基于 glyph 宽度进行换行
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

                // word-spacing: 额外的单词间距（应用于空格字符）
                float word_spacing_px = style.word_spacing_px;
                float word_spacing_units = 0.0f;
                if (word_spacing_px != 0.0f && scale > 0.0f) {
                    word_spacing_units = word_spacing_px / scale;
                }

                // 行高/基线度量（设计单位）
                // 优先使用 CSS line-height，如果未设置则使用字体度�?
                float line_height_units = shaped_full.line_height_units;
                float ascent_units = shaped_full.ascent_units;
                float descent_units = shaped_full.descent_units;

                // 应用 CSS line-height 属�?
                if (style.line_height > 0.0f) {
                    if (style.line_height_is_unitless) {
                        // 倍数：line-height * font-size
                        float css_line_height_px = style.line_height * font_size;
                        line_height_units = css_line_height_px / std::max(scale, 1e-3f);
                    } else {
                        // 像素�?
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

                // 基于 UTF-8 字节偏移构建潜在换行点（空格 + CJK 字符后）
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
                            push_break(i_byte); // 在空格之后允许换�?
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
                    // 对于�?ASCII（如 CJK），在字符后允许换行
                    push_break(i_byte);
                }

                // 保证文本末尾是一个换行候�?
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
                        // 统计空格数量用于 word-spacing
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
                    // 应用 word-spacing
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
                    // 跳过行首空格
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
                            // 后续 break point 只会更宽，直接停�?
                            break;
                        }
                    }

                    if (!found_any) {
                        // 无法在当前行宽内找到 break point，强制在第一个可�?break 或文本末尾换�?
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
                        // 没有 glyph（可能是全空格），直接结�?
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
                    int accumulated_spaces = 0;  // 累计空格数用�?word-spacing
                    for (int gi = first_glyph; gi <= last_glyph; ++gi) {
                        const ShapedGlyph& sg = glyphs[static_cast<size_t>(gi)];
                        GlyphInstance inst{};
                        inst.glyph_id = sg.glyph_id;
                        float base_x_units = sg.pen_x_units - first_pen_x_units;
                        if (letter_spacing_units != 0.0f && glyph_index_in_run > 0) {
                            base_x_units += letter_spacing_units * static_cast<float>(glyph_index_in_run);
                        }
                        // 应用 word-spacing：在空格之后的字符添加额外间�?
                        if (word_spacing_units != 0.0f && accumulated_spaces > 0) {
                            base_x_units += word_spacing_units * static_cast<float>(accumulated_spaces);
                        }
                        inst.pen_x_units = base_x_units;
                        inst.pen_y_units = sg.pen_y_units;
                        inst.font_path = sg.font_path;
                        inst.units_per_em = sg.units_per_em;
                        glyph_run.glyphs.push_back(inst);
                        ++glyph_index_in_run;
                        // 检查当�?glyph 是否对应空格字符
                        if (sg.cluster < text.size() && text[sg.cluster] == ' ') {
                            ++accumulated_spaces;
                        }
                    }

                    builder.addGlyphRun(std::move(glyph_run));

                    // 绘制 text-decoration（下划线/删除线/上划线）
                    if (!style.text_decoration.empty() && style.text_decoration != "none") {
                        Color deco_color = style.text_decoration_color.empty() 
                            ? text_color 
                            : makeColorFromCss(style.text_decoration_color);
                        float deco_thickness = std::max(1.0f, font_size * 0.05f);
                        
                        if (style.text_decoration == "underline") {
                            // 下划线：在基线下方
                            Rect underline{text_x, baseline_y + font_size * 0.1f, line_width_px, deco_thickness};
                            builder.addRect(underline, deco_color);
                        } else if (style.text_decoration == "line-through") {
                            // 删除线：在文本中间
                            Rect strikethrough{text_x, baseline_y - ascent_px * 0.35f, line_width_px, deco_thickness};
                            builder.addRect(strikethrough, deco_color);
                        } else if (style.text_decoration == "overline") {
                            // 上划线：在文本上方
                            Rect overline{text_x, baseline_y - ascent_px, line_width_px, deco_thickness};
                            builder.addRect(overline, deco_color);
                        }
                    }

                    line_start = best_break;
                    ++line_index;
                }
            }
        }
    }

    // 3.5 Input 元素特殊渲染 (visibility: hidden 时跳过)
    if (layout_node && tag == "input" && !is_hidden) {
        float x = layout_node->layout.position[0];
        float y = layout_node->layout.position[1];
        float width = layout_node->layout.dimensions[0];
        float height = layout_node->layout.dimensions[1];
        
        float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
        float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
        
        float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
        
        // 获取 value �?placeholder
        std::string display_text = node->getAttribute("value");
        Color text_color = makeColorFromCss(style.color);
        
        if (display_text.empty()) {
            display_text = node->getAttribute("placeholder");
            // placeholder 使用半透明颜色
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
                
                // 垂直居中
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

    // 4. 渲染 ::before 伪元素
    if (node->hasPseudoElements()) {
        auto pseudo_before = node->getPseudoBefore();
        if (pseudo_before) {
            renderPseudoElement(pseudo_before, node_rect, builder);
        }
    }

    // 5. 递归子节点（按 z-index 排序）
    const auto& children = node->getChildren();
    
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
            glyph_run.font_path = shaped.font_path;
            glyph_run.baseline_x = text_x;
            glyph_run.baseline_y = baseline_y;
            
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
