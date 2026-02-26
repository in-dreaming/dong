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
#include <unordered_set>

#include "../core/log.h"
#include "../core/profiler.h"
#include "../core/string_utils.h"
#include "../layout/display_contents.hpp"
#include "../layout/sticky_positioning.hpp"
namespace dong::render {


namespace {

using dong::collapseWhitespace;
using dong::toLower;

// Helper to lowercase + collapse whitespace
inline std::string toLowerCollapsed(const std::string& s) {
    return toLower(collapseWhitespace(s));
}

enum class BorderSide {
    Top,
    Right,
    Bottom,
    Left,
};

static void addDashedBorderHorizontal(DisplayListBuilder& builder,
                                     const Rect& r,
                                     const Color& c,
                                     float dash_len_px,
                                     float gap_len_px) {
    if (r.width <= 0.0f || r.height <= 0.0f) return;
    if (dash_len_px <= 0.0f) dash_len_px = 1.0f;
    if (gap_len_px < 0.0f) gap_len_px = 0.0f;

    float x = r.x;
    const float end = r.x + r.width;
    const float step = dash_len_px + gap_len_px;
    if (step <= 0.0f) return;

    while (x < end) {
        const float w = std::min(dash_len_px, end - x);
        if (w > 0.0f) {
            builder.addRect(Rect{x, r.y, w, r.height}, c);
        }
        x += step;
    }
}

static void addDashedBorderVertical(DisplayListBuilder& builder,
                                   const Rect& r,
                                   const Color& c,
                                   float dash_len_px,
                                   float gap_len_px) {
    if (r.width <= 0.0f || r.height <= 0.0f) return;
    if (dash_len_px <= 0.0f) dash_len_px = 1.0f;
    if (gap_len_px < 0.0f) gap_len_px = 0.0f;

    float y = r.y;
    const float end = r.y + r.height;
    const float step = dash_len_px + gap_len_px;
    if (step <= 0.0f) return;

    while (y < end) {
        const float h = std::min(dash_len_px, end - y);
        if (h > 0.0f) {
            builder.addRect(Rect{r.x, y, r.width, h}, c);
        }
        y += step;
    }
}

static void paintBorderSide(DisplayListBuilder& builder,
                            BorderSide side,
                            const Rect& side_rect,
                            const Color& c,
                            const std::string& border_style_lower) {
    if (side_rect.width <= 0.0f || side_rect.height <= 0.0f) return;

    // `solid`/`double`/etc 暂时走实线填充；`dashed`/`dotted` 用分段矩形近似。
    if (border_style_lower == "dashed" || border_style_lower == "dotted") {
        const float thickness = (side == BorderSide::Top || side == BorderSide::Bottom)
            ? side_rect.height
            : side_rect.width;
        const float base = std::max(1.0f, thickness);
        const float dash = (border_style_lower == "dotted") ? base : (base * 3.0f);
        const float gap = (border_style_lower == "dotted") ? base : (base * 2.0f);

        if (side == BorderSide::Top || side == BorderSide::Bottom) {
            addDashedBorderHorizontal(builder, side_rect, c, dash, gap);
        } else {
            addDashedBorderVertical(builder, side_rect, c, dash, gap);
        }
        return;
    }

    builder.addRect(side_rect, c);
}

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

// Convert CSSGradient to DrawLinearGradientData for the display list.
static DrawLinearGradientData buildLinearGradientData(
    const dong::dom::CSSGradient& grad,
    const Rect& target_rect,
    float border_radius)
{
    DrawLinearGradientData data{};
    data.rect = target_rect;
    data.angle_deg = grad.angle;
    data.radius = border_radius;

    int count = std::min(static_cast<int>(grad.stops.size()), kMaxGradientStops);
    data.stop_count = count;
    for (int i = 0; i < count; ++i) {
        data.stops[i].color = makeColorFromCss(grad.stops[i].color);
        data.stops[i].position = grad.stops[i].position;
    }
    return data;
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
    open_select_overlays_.clear();

    gen_.counters.clear();
    gen_.pushed_names_stack.clear();
    gen_.quote_depth = 0;

    if (layout_engine_) {
        current_dirty_rect_ = layout_engine_->getDirtyRect();
        const auto* layout_root = layout_engine_->getLayout(root);
        if (layout_root) {
            buildDisplayListNode(root, layout_root, display_list_builder_);
            paintSelectDropdownOverlays(display_list_builder_);
        }

    }

    layout_engine_ = nullptr;
    return display_list_builder_.get();
}

void Painter::buildDisplayListNode(const dom::DOMNodePtr& node,
                                   const layout::LayoutNode* layout_node,
                                   DisplayListBuilder& builder) {
    if (!node) return;

    struct ScopedCounterScope {
        Painter* painter = nullptr;
        bool enabled = false;
        ScopedCounterScope(Painter* p, const dom::ComputedStyle& s, bool en)
            : painter(p), enabled(en) {
            if (enabled && painter) painter->pushCounterScope(s);
        }
        ~ScopedCounterScope() {
            if (enabled && painter) painter->popCounterScope();
        }
    };

    const auto& style = node->getComputedStyle();
    if (style.display == "none") return;

    const bool has_counter_ops = !style.counter_resets.empty() || !style.counter_increments.empty();

    // display: contents - no box rendering, but still render children
    if (layout::shouldSkipLayoutNode(style)) {
        // Even without a layout box, `counter-reset`/`counter-increment` still participate.
        ScopedCounterScope counter_scope(this, style, has_counter_ops);

        // TODO: Render ::before pseudo-element if exists

        // Render children
        for (auto& child : node->getChildren()) {
            if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                auto child_layout = layout_engine_->getLayout(child);
                buildDisplayListNode(child, child_layout, builder);
            }
        }

        // TODO: Render ::after pseudo-element if exists

        return;
    }

    // visibility: hidden 元素不可见，但仍占位
    // 与 display: none 不同，visibility: hidden 仍然会进入布局
    const bool is_hidden = (style.visibility == "hidden" || style.visibility == "collapse");

    if (node->getType() == dom::DOMNode::NodeType::TEXT) return;

    // Apply counter ops before evaluating generated content and painting text.
    ScopedCounterScope counter_scope(this, style, has_counter_ops);


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

        // Expose layout metrics to DOM APIs (offsetTop/Left/Width/Height).
        // Note: we store absolute coords; JS can derive relative offsets by subtraction.
        node->setOffsetRect(node_rect.y, node_rect.x, node_rect.width, node_rect.height);

        // Apply sticky offset if this is a sticky element
        if (style.position == "sticky" && layout_node->sticky_metadata) {
            // Get current scroll position from scroll container
            float scroll_x = 0.0f;
            float scroll_y = 0.0f;

            if (layout_node->sticky_metadata->scroll_container) {
                scroll_x = layout_node->sticky_metadata->scroll_container->getScrollLeft();
                scroll_y = layout_node->sticky_metadata->scroll_container->getScrollTop();
            }

            // Apply sticky offset
            auto* mutable_layout = const_cast<layout::LayoutNode*>(layout_node);
            layout::applyStickyOffset(node, mutable_layout, scroll_x, scroll_y);

            // Use visual position from sticky metadata
            if (layout_node->sticky_metadata->is_stuck) {
                node_rect.x = layout_node->sticky_metadata->visual_x;
                node_rect.y = layout_node->sticky_metadata->visual_y;
            }
        }
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

    const bool force_isolation = (node->getAttribute("__dong_isolate") == "1" || node->getAttribute("__dong_isolate") == "true");

    bool has_isolation = (style.isolation_isolate || is_scroll_container || has_transform || force_isolation) &&
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

        // 重要优化（可选）：当启用图层缓存时，隔离图层在“非脏”情况下会复用上一帧栅格结果。
        // 这时可以选择跳过子树 DisplayList 构建来降低 buildDisplayList 的 CPU。
        // 注意：该优化必须显式开启，避免在未启用缓存时导致隔离图层内容为空。
        static const bool kLayerCacheEnabled = ([]() {
            const char* v = std::getenv("DONG_LAYER_CACHE");
            return v && v[0] == '1';
        })();
        static const bool kSkipBuildForCachedLayers = ([]() {
            const char* v = std::getenv("DONG_LAYER_CACHE_SKIP_BUILD");
            return v && v[0] == '1';
        })();

        if (kLayerCacheEnabled && kSkipBuildForCachedLayers && has_isolation && !layer_dirty) {
            if (pushed_layer_node && !layer_stack_.empty()) {
                layer_stack_.pop_back();
            }
            return;
        }

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

        // Tables with <caption>: the layout wrapper includes caption, but border/background should
        // only cover the grid box (exclude the caption area), matching browser behavior.
        if (layout_engine_ && (style.display == "table" || style.display == "inline-table")) {
            dom::DOMNodePtr caption_node;
            for (const auto& ch : node->getChildren()) {
                if (!ch || ch->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
                if (ch->getComputedStyle().display == "table-caption") {
                    caption_node = ch;
                    break;
                }
            }
            if (caption_node) {
                const auto* cap_ln = layout_engine_->getLayout(caption_node);
                const float cap_h = (cap_ln && std::isfinite(cap_ln->layout.dimensions[1])) ? cap_ln->layout.dimensions[1] : 0.0f;
                if (cap_h > 0.0f) {
                    const std::string side = toLowerCollapsed(caption_node->getComputedStyle().caption_side);
                    if (side == "bottom") {
                        rect.height = std::max(0.0f, rect.height - cap_h);
                    } else {
                        rect.y += cap_h;
                        rect.height = std::max(0.0f, rect.height - cap_h);
                    }
                }
            }
        }

        // Theme hint variables for the current node (may be overridden for UA-like controls).
        std::string bg_color_css = style.background_color;
        std::string override_border_color;

        // 1.1 background box helpers (clip/origin/attachment)
        const float radius = style.border_radius;
        const float viewport_w = surface_ ? static_cast<float>(surface_->getWidth()) : 800.0f;
        const float viewport_h = surface_ ? static_cast<float>(surface_->getHeight()) : 600.0f;

        auto resolvePx = [&](const dong::dom::CSSValue& v, float parent_size) -> float {
            return v.resolvePixels(parent_size, 16.0f, viewport_w, viewport_h);
        };

        // Border helpers: support per-side overrides, matching layout_engine.cpp semantics.
        auto normalizeBorderStyle = [&](const std::string& s) -> std::string {
            return dong::toLower(collapseWhitespace(s));
        };
        auto effectiveBorderStyle = [&](const std::string& side_style) -> std::string {
            const std::string& st = !side_style.empty() ? side_style : style.border_style;
            std::string norm = normalizeBorderStyle(st);

            // Browsers tend to paint form controls with flat borders even if UA rules say inset/outset.
            // Normalize inset/outset to solid for UA-like controls so `color-scheme` comparisons are stable.
            const bool is_control = (tag == "button" || tag == "input" || tag == "select" || tag == "textarea");
            if (is_control && (norm == "inset" || norm == "outset")) {
                norm = "solid";
            }
            return norm;
        };
        auto effectiveBorderWidth = [&](float side_width, const std::string& side_style) -> float {
            float w = (side_width >= 0.0f) ? side_width : style.border_width;
            if (w < 0.0f) w = 0.0f;
            const std::string st = effectiveBorderStyle(side_style);
            if (st == "none" || st == "hidden") return 0.0f;
            return w;
        };
        auto effectiveBorderColor = [&](const std::string& side_color) -> std::string {
            if (!side_color.empty()) return side_color;

            // We don't currently track cascade origin (UA vs author). For form-control theming,
            // treat the known UA defaults as "not author specified" so `color-scheme: dark`
            // can override them.
            const bool is_known_ua_border =
                style.border_color.empty() ||
                style.border_color == "#000000" ||
                style.border_color == "#767676" ||  // legacy UA value
                style.border_color == "#dadce0" ||  // Chromium-like light border
                style.border_color == "#5f6368";    // Chromium-like checkbox/radio border

            if (!override_border_color.empty() && is_known_ua_border) {
                return override_border_color;
            }
            return style.border_color;
        };

        const float bt = effectiveBorderWidth(style.border_top_width, style.border_top_style);
        const float br = effectiveBorderWidth(style.border_right_width, style.border_right_style);
        const float bb = effectiveBorderWidth(style.border_bottom_width, style.border_bottom_style);
        const float bl = effectiveBorderWidth(style.border_left_width, style.border_left_style);
        const float bmax = std::max(std::max(bt, bb), std::max(bl, br));

        const float pad_l = resolvePx(style.padding_left, rect.width);
        const float pad_r = resolvePx(style.padding_right, rect.width);
        const float pad_t = resolvePx(style.padding_top, rect.height);
        const float pad_b = resolvePx(style.padding_bottom, rect.height);
        const float min_pad = std::min(std::min(pad_l, pad_r), std::min(pad_t, pad_b));


        Rect bg_border_box = rect;
        Rect bg_padding_box = rect;
        bg_padding_box.x += bl;
        bg_padding_box.y += bt;
        bg_padding_box.width = std::max(0.0f, bg_padding_box.width - (bl + br));
        bg_padding_box.height = std::max(0.0f, bg_padding_box.height - (bt + bb));

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

        const std::string bg_clip_kw = toLowerCollapsed(style.background_clip);
        const std::string bg_origin_kw = toLowerCollapsed(style.background_origin);
        const std::string bg_attach_kw = toLowerCollapsed(style.background_attachment);

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
        const bool has_border = (bt > 0.0f || br > 0.0f || bb > 0.0f || bl > 0.0f);

        // Form controls: derive a UA-like palette from `color-scheme`.
        // Important: our UA stylesheet is light-themed; when author only sets `color-scheme`,
        // controls should still adopt a dark palette similar to Chromium.
        if (tag == "button" || tag == "input" || tag == "select" || tag == "textarea") {
            const std::string scheme = toLowerCollapsed(style.color_scheme);
            const bool is_button = (tag == "button");
            const bool scheme_dark = (scheme == "dark");

            const std::string default_bg = scheme_dark
                ? (is_button ? "#3c4043" : "#303134")
                : (is_button ? "#f1f3f4" : "#ffffff");
            const std::string default_border = scheme_dark ? "#5f6368" : "#dadce0";

            // Treat known UA light defaults as "not author-specified" so `color-scheme: dark` can override them.
            const bool is_known_ua_bg =
                (bg_color_css == "#ffffff" ||
                 bg_color_css == "#f3f3f3" ||  // legacy UA value
                 bg_color_css == "#f1f3f4");  // Chromium-like light button

            if (bg_color_css.empty() || bg_color_css == "transparent" || (scheme_dark && is_known_ua_bg)) {
                bg_color_css = default_bg;
            }

            const bool no_side_border_colors =
                style.border_top_color.empty() && style.border_right_color.empty() &&
                style.border_bottom_color.empty() && style.border_left_color.empty();

            const bool is_known_ua_border =
                style.border_color.empty() ||
                style.border_color == "#000000" ||
                style.border_color == "#767676" ||  // legacy UA value
                style.border_color == "#dadce0" ||  // Chromium-like light border
                style.border_color == "#5f6368";    // checkbox/radio border

            if (scheme_dark && no_side_border_colors && is_known_ua_border) {
                override_border_color = default_border;
            }
        }



        const bool has_background = !bg_color_css.empty() && bg_color_css != "transparent";

        
        // DEBUG: 打印背景信息（默认关闭，避免 baseline compare 过程中刷屏）
        if (std::getenv("DONG_DEBUG_PAINT_BG") && (radius > 0.0f || has_background || has_border)) {
            DONG_LOG_DEBUG("[PAINT_BG] bg_color='%s' has_bg=%d radius=%.1f has_border=%d rect=(%.0f,%.0f,%.0f,%.0f)",
                bg_color_css.c_str(), has_background, radius, has_border,
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
                    Color bg_color = makeColorFromCss(bg_color_css);
                    DONG_LOG_DEBUG("[PAINT_BG] Drawing rounded rect: color=(%f,%f,%f,%f)", bg_color.r, bg_color.g, bg_color.b, bg_color.a);

                    Rect inner_rect = bg_clip_rect;
                    float inset_for_radius = 0.0f;
                    if (bg_clip_kw == "padding-box") {
                        inset_for_radius = bmax;
                    } else if (bg_clip_kw == "content-box") {
                        inset_for_radius = bmax + min_pad;
                    }
                    float inner_radius = std::max(0.0f, radius - inset_for_radius);

                    if (inner_rect.width > 0.0f && inner_rect.height > 0.0f) {
                        builder.addRoundedRect(inner_rect, bg_color, inner_radius);
                    }
                }

                // CSS gradients paint on top of background-color
                for (const auto& grad : style.background_gradients) {
                    if (grad.type == dong::dom::CSSGradient::Type::LINEAR && grad.stops.size() >= 2) {
                        Rect inner_rect = bg_clip_rect;
                        float inset_for_radius = 0.0f;
                        if (bg_clip_kw == "padding-box") inset_for_radius = bmax;
                        else if (bg_clip_kw == "content-box") inset_for_radius = bmax + min_pad;
                        float inner_radius = std::max(0.0f, radius - inset_for_radius);
                        auto gdata = buildLinearGradientData(grad, inner_rect, inner_radius);
                        builder.addLinearGradient(gdata);
                    }
                }

                if (has_border) {
                    const std::string st_t = effectiveBorderStyle(style.border_top_style);
                    const std::string st_r = effectiveBorderStyle(style.border_right_style);
                    const std::string st_b = effectiveBorderStyle(style.border_bottom_style);
                    const std::string st_l = effectiveBorderStyle(style.border_left_style);

                    const bool bevel = (st_t == "outset" || st_t == "inset" ||
                                       st_r == "outset" || st_r == "inset" ||
                                       st_b == "outset" || st_b == "inset" ||
                                       st_l == "outset" || st_l == "inset");

                    if (!bevel) {
                        auto nearlyEqual = [](float a, float b) {
                            return std::fabs(a - b) < 0.01f;
                        };

                        const std::string c_t = effectiveBorderColor(style.border_top_color);
                        const std::string c_r = effectiveBorderColor(style.border_right_color);
                        const std::string c_b = effectiveBorderColor(style.border_bottom_color);
                        const std::string c_l = effectiveBorderColor(style.border_left_color);

                        const bool uniform_width = nearlyEqual(bt, br) && nearlyEqual(bt, bb) && nearlyEqual(bt, bl);
                        const bool uniform_color = (c_t == c_r && c_t == c_b && c_t == c_l);
                        const bool uniform_style = (st_t == st_r && st_t == st_b && st_t == st_l);

                        if (uniform_width && uniform_color && uniform_style) {
                            // Fast path: uniform rounded border ring.
                            Color border_color = makeColorFromCss(c_t);
                            builder.addRoundedRect(rect, border_color, radius, bt);
                        } else {
                            // Fallback: clip + 4 rects (supports per-side overrides).
                            DisplayListBuilder::ScopedClip border_clip = builder.pushRoundedClip(rect, radius);

                            const float inner_h = std::max(0.0f, rect.height - bt - bb);

                            if (bt > 0.0f) builder.addRect(Rect{rect.x, rect.y, rect.width, bt}, makeColorFromCss(c_t));
                            if (bb > 0.0f) builder.addRect(Rect{rect.x, rect.y + rect.height - bb, rect.width, bb}, makeColorFromCss(c_b));
                            if (bl > 0.0f) builder.addRect(Rect{rect.x, rect.y + bt, bl, inner_h}, makeColorFromCss(c_l));
                            if (br > 0.0f) builder.addRect(Rect{rect.x + rect.width - br, rect.y + bt, br, inner_h}, makeColorFromCss(c_r));
                        }
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

                        // Use shorthand direction as a stable reference.
                        const std::string bstyle = normalizeBorderStyle(style.border_style);
                        const bool is_outset = (bstyle == "outset");

                        // Prefer using the element background as the base for bevel shading;
                        // this matches browsers better when author sets background-color but not border.
                        Color base = has_background ? makeColorFromCss(style.background_color)
                                                    : makeColorFromCss(style.border_color);

                        Color c_tl = is_outset ? lighten(base, 0.25f) : darken(base, 0.25f);
                        Color c_br = is_outset ? darken(base, 0.25f) : lighten(base, 0.25f);

                        DisplayListBuilder::ScopedClip border_clip = builder.pushRoundedClip(rect, radius);
                        const float inner_h = std::max(0.0f, rect.height - bt - bb);

                        if (bt > 0.0f) builder.addRect(Rect{rect.x, rect.y, rect.width, bt}, c_tl);
                        if (bl > 0.0f) builder.addRect(Rect{rect.x, rect.y + bt, bl, inner_h}, c_tl);
                        if (bb > 0.0f) builder.addRect(Rect{rect.x, rect.y + rect.height - bb, rect.width, bb}, c_br);
                        if (br > 0.0f) builder.addRect(Rect{rect.x + rect.width - br, rect.y + bt, br, inner_h}, c_br);
                    }
                }

            } else {

                // 非圆角情况：先背景，后边框
                if (has_background) {
                    Color bg_color = makeColorFromCss(bg_color_css);
                    builder.addRect(bg_clip_rect, bg_color);
                }

                // CSS gradients paint on top of background-color (non-rounded path)
                for (const auto& grad : style.background_gradients) {
                    if (grad.type == dong::dom::CSSGradient::Type::LINEAR && grad.stops.size() >= 2) {
                        auto gdata = buildLinearGradientData(grad, bg_clip_rect, 0.0f);
                        builder.addLinearGradient(gdata);
                    }
                }

                if (has_border) {
                    const std::string st_t = effectiveBorderStyle(style.border_top_style);
                    const std::string st_r = effectiveBorderStyle(style.border_right_style);
                    const std::string st_b = effectiveBorderStyle(style.border_bottom_style);
                    const std::string st_l = effectiveBorderStyle(style.border_left_style);

                    const bool bevel = (st_t == "outset" || st_t == "inset" ||
                                       st_r == "outset" || st_r == "inset" ||
                                       st_b == "outset" || st_b == "inset" ||
                                       st_l == "outset" || st_l == "inset");

                    const float inner_h = std::max(0.0f, rect.height - bt - bb);

                    Rect top_border{rect.x, rect.y, rect.width, bt};
                    Rect bottom_border{rect.x, rect.y + rect.height - bb, rect.width, bb};
                    Rect left_border{rect.x, rect.y + bt, bl, inner_h};
                    Rect right_border{rect.x + rect.width - br, rect.y + bt, br, inner_h};

                    if (!bevel) {
                        const Color c_top = makeColorFromCss(effectiveBorderColor(style.border_top_color));
                        const Color c_right = makeColorFromCss(effectiveBorderColor(style.border_right_color));
                        const Color c_bottom = makeColorFromCss(effectiveBorderColor(style.border_bottom_color));
                        const Color c_left = makeColorFromCss(effectiveBorderColor(style.border_left_color));

                        if (bt > 0.0f) paintBorderSide(builder, BorderSide::Top, top_border, c_top, st_t);
                        if (bb > 0.0f) paintBorderSide(builder, BorderSide::Bottom, bottom_border, c_bottom, st_b);
                        if (bl > 0.0f) paintBorderSide(builder, BorderSide::Left, left_border, c_left, st_l);
                        if (br > 0.0f) paintBorderSide(builder, BorderSide::Right, right_border, c_right, st_r);
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

                        const std::string bstyle = normalizeBorderStyle(style.border_style);
                        const bool is_outset = (bstyle == "outset");
                        Color base = makeColorFromCss(effectiveBorderColor(""));

                        Color c_tl = is_outset ? lighten(base, 0.25f) : darken(base, 0.25f);

                        Color c_br = is_outset ? darken(base, 0.25f) : lighten(base, 0.25f);

                        if (bt > 0.0f) builder.addRect(top_border, c_tl);
                        if (bl > 0.0f) builder.addRect(left_border, c_tl);
                        if (bb > 0.0f) builder.addRect(bottom_border, c_br);
                        if (br > 0.0f) builder.addRect(right_border, c_br);
                    }
                }

            }
        }

        // 1.2.3 Form controls (very minimal): checkbox/radio checked mark.
        // Our engine doesn't have native OS widgets; emulate a simple mark so baseline diffs shrink.
        // Skip if appearance: none (allows custom styling without native marks).
        if (tag == "input" && rect.width > 0.0f && rect.height > 0.0f) {
            BorderWidths bw{};
            bw.top = bt;
            bw.right = br;
            bw.bottom = bb;
            bw.left = bl;
            bw.max = bmax;
            paintCheckboxMark(node, rect, bw, builder);
        }

        if (tag == "textarea" && rect.width > 0.0f && rect.height > 0.0f) {
            BorderWidths bw{};
            bw.top = bt;
            bw.right = br;
            bw.bottom = bb;
            bw.left = bl;
            bw.max = bmax;
            paintTextareaResizeHandle(node, rect, bw, builder);
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
                const std::string bg_size = toLowerCollapsed(style.background_size);
                const std::string bg_repeat = toLowerCollapsed(style.background_repeat);
                const std::string bg_pos = toLowerCollapsed(style.background_position);

                const std::string ir = toLowerCollapsed(style.image_rendering);
                const ImageSampling img_sampling = (ir == "pixelated" || ir == "crisp-edges")
                    ? ImageSampling::Nearest
                    : ImageSampling::Linear;


                // Backgrounds are clipped to background-clip.
                float bg_clip_radius = radius;
                if (bg_clip_kw == "padding-box") {
                    bg_clip_radius = std::max(0.0f, radius - bmax);
                } else if (bg_clip_kw == "content-box") {
                    bg_clip_radius = std::max(0.0f, radius - bmax - min_pad);
                }


                DisplayListBuilder::ScopedClip bg_clip;
                if (bg_clip_radius > 0.0f) {
                    bg_clip = builder.pushRoundedClip(bg_clip_rect, bg_clip_radius);
                } else {
                    bg_clip = builder.pushClipRect(bg_clip_rect);
                }


                if (bg_size.find("cover") != std::string::npos) {
                    builder.addImage(bg_clip_rect, image_url, 1.0f, ImageFitMode::Cover, 0.5f, 0.5f, img_sampling);

                } else if (bg_size.find("contain") != std::string::npos) {
                    builder.addImage(bg_clip_rect, image_url, 1.0f, ImageFitMode::Contain, 0.5f, 0.5f, img_sampling);

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
                                builder.addImage(tile, image_url, 1.0f, ImageFitMode::Fill, 0.5f, 0.5f, img_sampling);

                            }
                        }
                    } else {
                        // Fallback: stretch to the clip rect.
                        builder.addImage(bg_clip_rect, image_url, 1.0f, ImageFitMode::Fill, 0.5f, 0.5f, img_sampling);

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
        auto effectiveBorderWidth = [&](float side_width, const std::string& side_style) -> float {
            const std::string st = !side_style.empty() ? toLowerCollapsed(side_style) : toLowerCollapsed(style.border_style);
            if (st == "none" || st == "hidden") {
                return 0.0f;
            }
            if (side_width >= 0.0f) {
                return side_width;
            }
            return std::max(0.0f, style.border_width);
        };

        const float bt = effectiveBorderWidth(style.border_top_width, style.border_top_style);
        const float br = effectiveBorderWidth(style.border_right_width, style.border_right_style);
        const float bb = effectiveBorderWidth(style.border_bottom_width, style.border_bottom_style);
        const float bl = effectiveBorderWidth(style.border_left_width, style.border_left_style);
        const float bmax = std::max(std::max(bt, bb), std::max(bl, br));

        // CSS 语义：overflow 的裁剪区域是 padding box（不包含 border），否则子内容会覆盖边框。
        Rect clip_rect = node_rect;
        clip_rect.x += bl;
        clip_rect.y += bt;
        clip_rect.width = std::max(0.0f, node_rect.width - bl - br);
        clip_rect.height = std::max(0.0f, node_rect.height - bt - bb);

        const float inner_radius = std::max(0.0f, style.border_radius - bmax);
        if (inner_radius > 0.0f) {
            clip_scope = builder.pushRoundedClip(clip_rect, inner_radius);
        } else {
            clip_scope = builder.pushClipRect(clip_rect);
        }
    }

    paintMediaElements(node, layout_node, tag, style, is_hidden, builder);

    paintTextAndInput(node, layout_node, tag, style, is_hidden, builder);

    paintChildrenAndOverlays(node, layout_node, node_rect, has_layout_rect, is_scroll_container, builder);





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

void Painter::pushCounterScope(const dom::ComputedStyle& style) {
    // CSS counters:
    // - counter-reset creates a new scope (push)
    // - counter-increment mutates the current scope (no push)
    std::vector<std::string> pushed_resets;

    auto ensureBaseCounter = [&](const std::string& name) {
        auto& st = gen_.counters[name];
        if (st.empty()) {
            st.push_back(0);  // implicit root scope
        }
    };

    auto pushReset = [&](const std::string& name, int value) {
        gen_.counters[name].push_back(value);
        pushed_resets.push_back(name);
    };

    // Apply resets first (last one wins per name), keeping stable order.
    {
        std::unordered_set<std::string> seen;
        std::vector<dom::ComputedStyle::CounterDirective> dedup;
        dedup.reserve(style.counter_resets.size());
        for (auto it = style.counter_resets.rbegin(); it != style.counter_resets.rend(); ++it) {
            if (it->name.empty()) continue;
            if (seen.insert(it->name).second) {
                dedup.push_back(*it);
            }
        }
        std::reverse(dedup.begin(), dedup.end());
        for (const auto& r : dedup) {
            const int v = r.has_value ? r.value : 0;
            pushReset(r.name, v);
        }
    }

    // Then increments (persist across siblings).
    for (const auto& inc : style.counter_increments) {
        if (inc.name.empty()) continue;
        ensureBaseCounter(inc.name);
        auto it = gen_.counters.find(inc.name);
        if (it == gen_.counters.end() || it->second.empty()) continue;
        it->second.back() += inc.value;
    }

    gen_.pushed_names_stack.push_back(std::move(pushed_resets));
}

void Painter::popCounterScope() {
    if (gen_.pushed_names_stack.empty()) return;
    auto pushed = std::move(gen_.pushed_names_stack.back());
    gen_.pushed_names_stack.pop_back();

    for (const auto& name : pushed) {
        auto it = gen_.counters.find(name);
        if (it == gen_.counters.end()) continue;
        if (!it->second.empty()) {
            it->second.pop_back();
        }
        if (it->second.empty()) {
            gen_.counters.erase(it);
        }
    }
}


std::string Painter::evaluateCounterText(const std::string& name) {
    auto it = gen_.counters.find(name);
    if (it == gen_.counters.end() || it->second.empty()) {
        return "0";
    }
    return std::to_string(it->second.back());
}

std::string Painter::evaluateCountersText(const std::string& name, const std::string& sep) {
    auto it = gen_.counters.find(name);
    if (it == gen_.counters.end() || it->second.empty()) {
        return "0";
    }

    std::string out;
    for (size_t i = 0; i < it->second.size(); ++i) {
        if (i > 0) out += sep;
        out += std::to_string(it->second[i]);
    }
    return out;
}

std::string Painter::evaluateQuoteToken(const dom::ComputedStyle& style,
                                       const dom::ComputedStyle::ContentToken& tok) {
    const std::vector<std::string>& q = style.quotes;
    const size_t pair_count = q.size() / 2;

    auto quoteAt = [&](int depth, bool open) -> std::string {
        if (pair_count == 0) return {};
        int d = depth;
        if (d < 0) d = 0;
        size_t idx = static_cast<size_t>(d);
        if (idx >= pair_count) idx = pair_count - 1;
        size_t base = idx * 2;
        return open ? q[base] : q[base + 1];
    };

    switch (tok.type) {
    case dom::ComputedStyle::ContentToken::Type::OpenQuote: {
        std::string s = quoteAt(gen_.quote_depth, true);
        gen_.quote_depth = std::max(0, gen_.quote_depth + 1);
        return s;
    }
    case dom::ComputedStyle::ContentToken::Type::CloseQuote: {
        // CSS spec: if quote depth is 0, close-quote generates no content and depth stays 0.
        if (gen_.quote_depth <= 0) {
            gen_.quote_depth = 0;
            return {};
        }
        gen_.quote_depth -= 1;
        return quoteAt(gen_.quote_depth, false);
    }
    case dom::ComputedStyle::ContentToken::Type::NoOpenQuote:
        gen_.quote_depth = std::max(0, gen_.quote_depth + 1);
        return {};
    case dom::ComputedStyle::ContentToken::Type::NoCloseQuote:
        // CSS spec: if quote depth is 0, no-close-quote does nothing.
        if (gen_.quote_depth <= 0) {
            gen_.quote_depth = 0;
            return {};
        }
        gen_.quote_depth -= 1;
        return {};


    default:
        return {};
    }
}

std::string Painter::evaluateContentText(const dom::ComputedStyle& style) {
    if (!style.content_tokens.empty()) {
        std::string out;
        for (const auto& tok : style.content_tokens) {
            switch (tok.type) {
            case dom::ComputedStyle::ContentToken::Type::String:
                out += tok.text;
                break;
            case dom::ComputedStyle::ContentToken::Type::Counter:
                out += evaluateCounterText(tok.text);
                break;
            case dom::ComputedStyle::ContentToken::Type::Counters:
                out += evaluateCountersText(tok.text, tok.separator.empty() ? "." : tok.separator);
                break;
            case dom::ComputedStyle::ContentToken::Type::OpenQuote:
            case dom::ComputedStyle::ContentToken::Type::CloseQuote:
            case dom::ComputedStyle::ContentToken::Type::NoOpenQuote:
            case dom::ComputedStyle::ContentToken::Type::NoCloseQuote:
                out += evaluateQuoteToken(style, tok);
                break;
            }
        }
        return out;
    }

    return style.content;
}

void Painter::renderPseudoElement(const dom::DOMNodePtr& pseudo,
                                   const Rect& parent_rect,
                                   DisplayListBuilder& builder) {
    if (!pseudo) return;
    
    const auto& style = pseudo->getComputedStyle();
    if (style.display == "none") return;

    struct ScopedGeneratedCounters {
        Painter* painter;
        ScopedGeneratedCounters(Painter* p, const dom::ComputedStyle& s) : painter(p) { painter->pushCounterScope(s); }
        ~ScopedGeneratedCounters() { painter->popCounterScope(); }
    } scope(this, style);

    const std::string content_text = evaluateContentText(style);

    const bool has_background = (style.background_color != "transparent");
    const bool has_border = (style.border_width > 0.0f && style.border_style != "none");
    if (content_text.empty() && !has_background && !has_border) {
        return;
    }

    
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
    if (!content_text.empty()) {
        Color text_color = makeColorFromCss(style.color);
        float font_size = style.font_size;
        
        // Position text inside the pseudo-element
        float text_x = rect.x + padding_left + style.border_width;
        float text_y = rect.y + padding_top + style.border_width;
        
        // Shape text using TextShapeRequest
        TextShapeRequest request;
        request.text = content_text;

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
            glyph_run.font_paths = shaped.font_paths;
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
                inst.font_path_index = sg.font_path_index;
                inst.units_per_em = sg.units_per_em;
                glyph_run.glyphs.push_back(inst);
            }
            
            builder.addGlyphRun(std::move(glyph_run));
        }
    }
}

// ============ buildDisplayListNode 重构辅助方法实现 ============

// 注意：这些方法使用 painter_detail 命名空间中的工具函数
// 它们定义在 painter_style_utils.hpp 中

bool Painter::shouldSkipNode(const dom::DOMNodePtr& node, const dom::ComputedStyle& style) const {
    if (!node) return true;
    if (style.display == "none") return true;
    if (node->getType() == dom::DOMNode::NodeType::TEXT) return true;
    return false;
}

Painter::LayerDecision Painter::decideLayerNeeds(const dom::DOMNodePtr& node,
                                                  const layout::LayoutNode* layout_node,
                                                  const dom::ComputedStyle& style,
                                                  const Rect& node_rect,
                                                  bool has_layout_rect,
                                                  DisplayListBuilder& builder) {
    LayerDecision decision;
    decision.clamped_opacity = std::clamp(style.opacity, 0.0f, 1.0f);

    bool should_apply_clip = has_layout_rect &&
        (shouldClipOverflow(style.overflow) || shouldClipOverflow(style.overflow_x) ||
         shouldClipOverflow(style.overflow_y));

    decision.is_scroll_container = should_apply_clip &&
        (isScrollOverflow(style.overflow) || isScrollOverflow(style.overflow_x) ||
         isScrollOverflow(style.overflow_y));

    bool has_transform = (style.transform_translate_x != 0.0f || style.transform_translate_y != 0.0f ||
                          style.transform_scale_x != 1.0f || style.transform_scale_y != 1.0f ||
                          style.transform_rotate != 0.0f || style.transform_skew_x != 0.0f ||
                          style.transform_skew_y != 0.0f);

    bool force_isolation = (node->getAttribute("__dong_isolate") == "1" ||
                            node->getAttribute("__dong_isolate") == "true");

    float builder_tx = builder.getTranslateX();
    float builder_ty = builder.getTranslateY();

    Rect layer_bounds = computeLayerBounds(node_rect, has_layout_rect, builder_tx, builder_ty);

    decision.has_isolation = (style.isolation_isolate || decision.is_scroll_container ||
                             has_transform || force_isolation) &&
                             layer_bounds.width > 0.0f && layer_bounds.height > 0.0f;

    decision.needs_layer = decision.has_isolation || decision.clamped_opacity < 0.999f || has_transform;

    if (decision.needs_layer) {
        decision.content_dirty = node->isLayoutDirty() || decision.is_scroll_container;
        if (!decision.content_dirty && (builder_tx != 0.0f || builder_ty != 0.0f)) {
            decision.content_dirty = true;
        }
        if (!decision.content_dirty && use_dirty_rect_ && !current_dirty_rect_.isEmpty()) {
            decision.content_dirty = isRectInDirtyRect(layer_bounds);
        }
    }

    return decision;
}

Rect Painter::computeLayerBounds(const Rect& node_rect, bool has_layout_rect,
                                 float builder_tx, float builder_ty) const {
    Rect bounds = node_rect;
    if (!has_layout_rect && surface_) {
        bounds.x = 0.0f;
        bounds.y = 0.0f;
        bounds.width = static_cast<float>(surface_->getWidth());
        bounds.height = static_cast<float>(surface_->getHeight());
    }
    bounds.x += builder_tx;
    bounds.y += builder_ty;
    return bounds;
}

LayerTransform Painter::computeLayerTransform(const dom::ComputedStyle& style,
                                              const Rect& layer_bounds) const {
    LayerTransform transform = LayerTransform::identity();

    float sx = style.transform_scale_x;
    float sy = style.transform_scale_y;
    float tx = style.transform_translate_x;
    float ty = style.transform_translate_y;
    float angle_rad = style.transform_rotate * 3.14159265358979f / 180.0f;
    float skew_x_rad = style.transform_skew_x * 3.14159265358979f / 180.0f;
    float skew_y_rad = style.transform_skew_y * 3.14159265358979f / 180.0f;

    float origin_x = layer_bounds.x + layer_bounds.width * style.transform_origin_x / 100.0f;
    float origin_y = layer_bounds.y + layer_bounds.height * style.transform_origin_y / 100.0f;

    float cos_r = cosf(angle_rad);
    float sin_r = sinf(angle_rad);
    float tan_kx = tanf(skew_x_rad);
    float tan_ky = tanf(skew_y_rad);

    float a00 = sx * (cos_r - sin_r * tan_ky);
    float a01 = sy * (cos_r * tan_kx - sin_r);
    float a10 = sx * (sin_r + cos_r * tan_ky);
    float a11 = sy * (sin_r * tan_kx + cos_r);

    transform.m[0] = a00;
    transform.m[1] = a01;
    transform.m[2] = origin_x + tx - (a00 * origin_x + a01 * origin_y);
    transform.m[3] = a10;
    transform.m[4] = a11;
    transform.m[5] = origin_y + ty - (a10 * origin_x + a11 * origin_y);

    return transform;
}

bool Painter::shouldSkipCachedLayer(const LayerDecision& decision) const {
    static const bool kLayerCacheEnabled = ([]() {
        const char* v = std::getenv("DONG_LAYER_CACHE");
        return v && v[0] == '1';
    })();
    static const bool kSkipBuildForCachedLayers = ([]() {
        const char* v = std::getenv("DONG_LAYER_CACHE_SKIP_BUILD");
        return v && v[0] == '1';
    })();
    return kLayerCacheEnabled && kSkipBuildForCachedLayers &&
           decision.has_isolation && !decision.content_dirty;
}

Painter::BorderWidths Painter::computeBorderWidths(const dom::ComputedStyle& style) const {
    BorderWidths bw;

    auto normalizeBorderStyle = [](std::string s) -> std::string {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return s;
    };
    auto effectiveBorderStyle = [&](const std::string& side_style) -> std::string {
        const std::string& st = !side_style.empty() ? side_style : style.border_style;
        return normalizeBorderStyle(st);
    };
    auto effectiveBorderWidth = [&](float side_width, const std::string& side_style) -> float {
        float w = (side_width >= 0.0f) ? side_width : style.border_width;
        if (w < 0.0f) w = 0.0f;
        const std::string st = effectiveBorderStyle(side_style);
        return (st == "none" || st == "hidden") ? 0.0f : w;
    };

    bw.top = effectiveBorderWidth(style.border_top_width, style.border_top_style);
    bw.right = effectiveBorderWidth(style.border_right_width, style.border_right_style);
    bw.bottom = effectiveBorderWidth(style.border_bottom_width, style.border_bottom_style);
    bw.left = effectiveBorderWidth(style.border_left_width, style.border_left_style);
    bw.max = std::max(std::max(bw.top, bw.bottom), std::max(bw.left, bw.right));

    return bw;
}

void Painter::paintBoxShadow(const Rect& rect, const dom::ComputedStyle& style,
                             DisplayListBuilder& builder) const {
    if (style.box_shadows.empty() || rect.width <= 0.0f || rect.height <= 0.0f) return;

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

        if (shadow_rect.width <= 0.0f || shadow_rect.height <= 0.0f) continue;

        float radius = std::max(0.0f, style.border_radius);
        if (shadow.blur_radius > 0.0f) {
            builder.addShadow(shadow_rect, sc, radius, shadow.blur_radius);
        } else {
            builder.addRoundedRect(shadow_rect, sc, radius);
        }
    }
}

void Painter::paintBackgroundAndBorder(const Rect& rect,
                                       const BorderWidths& bw,
                                       const dom::ComputedStyle& style,
                                       DisplayListBuilder& builder) {
    bool has_background = !style.background_color.empty() && style.background_color != "transparent";
    bool has_border = (bw.top > 0.0f || bw.right > 0.0f || bw.bottom > 0.0f || bw.left > 0.0f);

    // Resolve border-radius values (support percentages relative to element size)
    // For border-radius, percentages are relative to the corresponding dimension:
    // horizontal radii use width, vertical radii use height
    // For simplicity, we'll use a single radius value (the average of width/height for percentage)
    float radius = style.border_radius;

    // Check if any corner has a set value (which might be percentage)
    if (style.border_top_left_radius.isSet() || style.border_top_right_radius.isSet() ||
        style.border_bottom_left_radius.isSet() || style.border_bottom_right_radius.isSet()) {

        // Helper to resolve a corner radius value
        auto resolveRadius = [&](const dom::CSSValue& val) -> float {
            if (!val.isSet()) return radius; // Fall back to legacy value
            if (val.isPercent()) {
                // For percentage border-radius, use the average of width and height
                // (In full CSS spec, horizontal and vertical radii can differ, but we use circular arcs)
                float ref_size = (rect.width + rect.height) * 0.5f;
                return val.value * ref_size / 100.0f;
            } else if (val.isPixel()) {
                return val.value;
            }
            return 0.0f;
        };

        // For now, use the maximum of all four corners as the single radius
        // (Painter currently doesn't support per-corner radii)
        float tl = resolveRadius(style.border_top_left_radius);
        float tr = resolveRadius(style.border_top_right_radius);
        float bl = resolveRadius(style.border_bottom_left_radius);
        float br = resolveRadius(style.border_bottom_right_radius);
        radius = std::max(std::max(tl, tr), std::max(bl, br));
    }

    if (rect.width <= 0.0f || rect.height <= 0.0f) return;

    // 计算背景盒
    Rect padding_box = rect;
    padding_box.x += bw.left;
    padding_box.y += bw.top;
    padding_box.width = std::max(0.0f, padding_box.width - (bw.left + bw.right));
    padding_box.height = std::max(0.0f, padding_box.height - (bw.top + bw.bottom));

    if (radius > 0.0f) {
        // 圆角路径
        if (has_background) {
            Color bg_color = makeColorFromCss(style.background_color);
            std::string bg_clip = style.background_clip;
            std::transform(bg_clip.begin(), bg_clip.end(), bg_clip.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            float inset = 0.0f;
            if (bg_clip == "padding-box") inset = bw.max;
            float inner_radius = std::max(0.0f, radius - inset);
            if (padding_box.width > 0.0f && padding_box.height > 0.0f) {
                builder.addRoundedRect(padding_box, bg_color, inner_radius);
            }
        }
        if (has_border) {
            // 简化的圆角边框实现：使用填充方式
            Color border_color = makeColorFromCss(style.border_color);
            // 绘制外圆角矩形
            builder.addRoundedRect(rect, border_color, radius);
            // 如果边框较窄，可以绘制内圆角矩形减去中间部分
            // 这里简化处理，实际应该使用专门的边框绘制方法
        }
    } else {
        // 矩形路径
        if (has_background) {
            builder.addRect(padding_box, makeColorFromCss(style.background_color));
        }
        if (has_border) {
            auto effectiveColor = [&](const std::string& side_color) -> Color {
                return makeColorFromCss(!side_color.empty() ? side_color : style.border_color);
            };
            auto effectiveStyle = [&](const std::string& side_style) -> std::string {
                const std::string& st = !side_style.empty() ? side_style : style.border_style;
                return toLowerCollapsed(st);
            };

            if (bw.top > 0.0f) {
                paintBorderSide(builder,
                                BorderSide::Top,
                                Rect{rect.x, rect.y, rect.width, bw.top},
                                effectiveColor(style.border_top_color),
                                effectiveStyle(style.border_top_style));
            }
            if (bw.right > 0.0f) {
                paintBorderSide(builder,
                                BorderSide::Right,
                                Rect{rect.x + rect.width - bw.right, rect.y + bw.top,
                                     bw.right, rect.height - bw.top - bw.bottom},
                                effectiveColor(style.border_right_color),
                                effectiveStyle(style.border_right_style));
            }
            if (bw.bottom > 0.0f) {
                paintBorderSide(builder,
                                BorderSide::Bottom,
                                Rect{rect.x, rect.y + rect.height - bw.bottom, rect.width, bw.bottom},
                                effectiveColor(style.border_bottom_color),
                                effectiveStyle(style.border_bottom_style));
            }
            if (bw.left > 0.0f) {
                paintBorderSide(builder,
                                BorderSide::Left,
                                Rect{rect.x, rect.y + bw.top, bw.left,
                                     rect.height - bw.top - bw.bottom},
                                effectiveColor(style.border_left_color),
                                effectiveStyle(style.border_left_style));
            }
        }
    }
}

void Painter::paintCheckboxMark(const dom::DOMNodePtr& node,
                                const Rect& rect,
                                const BorderWidths& bw,
                                DisplayListBuilder& builder) const {
    if (!node || !node->hasAttribute("type")) return;

    const auto& style = node->getComputedStyle();
    if (style.appearance == "none") return;

    std::string t = node->getAttribute("type");
    std::transform(t.begin(), t.end(), t.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if ((t != "checkbox" && t != "radio") || !node->hasAttribute("checked")) return;

    Rect inner = rect;
    inner.x += bw.left;
    inner.y += bw.top;
    inner.width = std::max(0.0f, inner.width - (bw.left + bw.right));
    inner.height = std::max(0.0f, inner.height - (bw.top + bw.bottom));

    float side = std::min(inner.width, inner.height);
    if (side <= 0.0f) return;

    // Clamp border radius for mark painting.
    const float max_r = std::min(rect.width, rect.height) * 0.5f;
    const float rr = std::clamp(std::max(0.0f, style.border_radius), 0.0f, max_r);

    // CSS `accent-color`: `auto` uses a UA default accent; explicit colors override it.
    // Approximate Chromium baseline default accent (environment-dependent; tuned to our Playwright baseline).
    Color mark_color = makeColorFromCss("#258292");
    if (!style.accent_color.empty() && style.accent_color != "auto") {
        mark_color = makeColorFromCss(style.accent_color);
    }

    if (t == "checkbox") {
        // Fill the control with the accent color. Paint over the border as Chromium does.
        builder.addRoundedRect(rect, mark_color, rr);


        // White checkmark (staircase approximation) to resemble browser checkbox glyph.
        Color white{1.0f, 1.0f, 1.0f, 1.0f};
        const float px = std::max(1.0f, std::floor(side * 0.10f));
        const float step = px;

        const float x0 = inner.x;
        const float y0 = inner.y;

        // Two legs of the check: down-left -> up-right.
        const float pts[][2] = {
            {0.22f, 0.55f}, {0.28f, 0.62f}, {0.34f, 0.69f},
            {0.40f, 0.62f}, {0.48f, 0.54f}, {0.58f, 0.44f}, {0.68f, 0.34f}
        };
        for (const auto& p : pts) {
            Rect r{x0 + side * p[0], y0 + side * p[1], px, px};
            if (r.width > 0.0f && r.height > 0.0f) builder.addRect(r, white);
        }
        (void)step;
        return;

    }

    // radio
    const float radius = side * 0.5f;
    const float stroke = std::max(1.0f, side * 0.18f);
    builder.addRoundedRect(inner, mark_color, radius, stroke);

    Rect dot{inner.x + side * 0.35f, inner.y + side * 0.35f,
             std::max(0.0f, side * 0.30f), std::max(0.0f, side * 0.30f)};
    if (dot.width > 0.0f && dot.height > 0.0f) {
        builder.addRoundedRect(dot, mark_color, std::min(dot.width, dot.height) * 0.5f);
    }

}

void Painter::paintTextareaResizeHandle(const dom::DOMNodePtr& node,
                                       const Rect& rect,
                                       const BorderWidths& bw,
                                       DisplayListBuilder& builder) const {
    if (!node) return;

    const auto& style = node->getComputedStyle();
    if (style.appearance == "none") return;

    const std::string r = toLowerCollapsed(style.resize);
    if (r == "none") return;

    // Only show the gripper when the box is scrollable (matches textarea common behavior).
    const std::string ov = toLowerCollapsed(style.overflow);
    if (ov == "visible") return;

    Rect inner = rect;
    inner.x += bw.left;
    inner.y += bw.top;
    inner.width = std::max(0.0f, inner.width - (bw.left + bw.right));
    inner.height = std::max(0.0f, inner.height - (bw.top + bw.bottom));
    if (inner.width <= 0.0f || inner.height <= 0.0f) return;

    const float max_size = 16.0f;
    const float size = std::min(max_size, std::min(inner.width, inner.height));
    if (size < 8.0f) return;

    const bool scheme_dark = (toLowerCollapsed(style.color_scheme) == "dark");
    Color c = makeColorFromCss(scheme_dark ? "#c9c9c9" : "#8a8a8a");
    c.a = 1.0f;

    const float pad = 2.0f;
    const float step = 2.0f;
    const float px = 2.0f;

    // Draw a simple diagonal gripper using 2px squares (3 diagonals), tuned for visibility.
    for (int k = 0; k < 3; ++k) {
        const float offset = static_cast<float>(k) * 4.0f;
        for (float t = 0.0f; t < size - offset; t += step) {
            const float x = inner.x + inner.width - pad - offset - t;
            const float y = inner.y + inner.height - pad - t;
            builder.addRect(Rect{x, y, px, px}, c);
        }
    }

}

} // namespace dong::render

