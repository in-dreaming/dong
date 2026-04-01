#include "../painter.hpp"
#include "painter_style_utils.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include <vector>

#include "../list_marker.hpp"
#include "../../dom/details_element.hpp"

namespace dong::render {

namespace {

float computeScrollContentBottom(const dom::DOMNodePtr& node,
                                layout::Engine* layout_engine,
                                float current_max_bottom) {
    if (!node || !layout_engine) {
        return current_max_bottom;
    }

    for (const auto& child : node->getChildren()) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
            continue;
        }

        if (const layout::LayoutNode* child_layout = layout_engine->getLayout(child)) {
            const float child_bottom = child_layout->layout.position[1] + child_layout->layout.dimensions[1];

            // scrollHeight 的滚动范围应覆盖子元素的 margin 边缘（尤其是最后一个元素的 margin-bottom）。
            // 否则 scrollTop = scrollHeight 会被 clamp 到过小的 maxScroll，导致“滚不到底/中段高度不对”。
            float margin_bottom = 0.0f;
            const auto& cs = child->getComputedStyle();
            if (cs.margin_bottom.isPixel()) {
                margin_bottom = cs.margin_bottom.value;
            }
            if (margin_bottom < 0.0f) {
                margin_bottom = 0.0f; // 负外边距不扩展 scroll 范围
            }

            current_max_bottom = std::max(current_max_bottom, child_bottom + margin_bottom);
        }

        current_max_bottom = computeScrollContentBottom(child, layout_engine, current_max_bottom);
    }

    return current_max_bottom;
}

inline float effectiveBorderWidthPx(const dom::ComputedStyle& style,
                                   float side_width,
                                   dom::CSSBorderStyle side_style) {
    dom::CSSBorderStyle st = (side_style != dom::CSSBorderStyleUnset) ? side_style : style.border_style;
    if (st == dom::CSSBorderStyle::None || st == dom::CSSBorderStyle::Hidden) {
        return 0.0f;
    }
    return (side_width >= 0.0f) ? side_width : std::max(0.0f, style.border_width);
}

Rect computeClientRectFromBorderBox(const Rect& border_box, const dom::ComputedStyle& style) {
    const float bt = effectiveBorderWidthPx(style, style.border_top_width, style.border_top_style);
    const float br = effectiveBorderWidthPx(style, style.border_right_width, style.border_right_style);
    const float bb = effectiveBorderWidthPx(style, style.border_bottom_width, style.border_bottom_style);
    const float bl = effectiveBorderWidthPx(style, style.border_left_width, style.border_left_style);

    Rect client = border_box;
    client.x += bl;
    client.y += bt;
    client.width = std::max(0.0f, border_box.width - bl - br);
    client.height = std::max(0.0f, border_box.height - bt - bb);
    return client;
}

bool shouldDebugScrollMetricsForNode(const dom::DOMNodePtr& node) {
    if (!node) {
        return false;
    }

    // Debug scroll metrics for a specific element id.
    // - DONG_DEBUG_SCROLL_METRICS=1      -> legacy: only id=="sc"
    // - DONG_DEBUG_SCROLL_METRICS=<id>   -> debug that id
    const char* v = ::getenv("DONG_DEBUG_SCROLL_METRICS");
    if (!v || !v[0]) {
        return false;
    }

    const std::string id = node->getAttribute("id");
    if (v[0] == '*' && v[1] == '\0') {
        return true;
    }
    if (v[0] == '1' && v[1] == '\0') {
        return id == "sc";
    }
    return id == v;
}

// Calculate the 1-based counter for an <li> within its parent <ul>/<ol>.
// Respects <ol start="N"> attribute.
int calculateListItemCounter(const dom::DOMNodePtr& li_node) {
    auto parent = li_node->getParent();
    if (!parent) return 1;

    int start = 1;
    if (parent->getTagName() == "ol" && parent->hasAttribute("start")) {
        try {
            start = std::stoi(parent->getAttribute("start"));
        } catch (...) {}
    }

    int counter = start;
    for (const auto& sibling : parent->getChildren()) {
        if (!sibling || sibling->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
        if (sibling == li_node) return counter;
        if (sibling->getTagName() == "li") ++counter;
    }
    return counter;
}

} // namespace

// Render the ::marker pseudo-element for a <li> node.
// Uses the parent <ul>/<ol>'s list-style-type (inherited to the li).
void Painter::renderMarkerForListItem(const dom::DOMNodePtr& node,
                                      const Rect& node_rect,
                                      DisplayListBuilder& builder) {
    auto marker_pseudo = node->getPseudoMarker();
    if (!marker_pseudo) return;

    const auto& style = node->getComputedStyle();
    const auto& marker_style = marker_pseudo->getComputedStyle();
    const auto marker_type = style.list_style_type;
    if (marker_type == dom::CSSListStyleType::None) return;

    int counter = calculateListItemCounter(node);
    std::string marker_text = generateMarkerText(counter, toString(marker_type));
    if (marker_text.empty()) return;

    // Determine marker color (::marker color overrides, else inherit from li)
    Color text_color = painter_detail::makeColorFromCss(
        marker_style.isExplicitlySet("color") ? marker_style.color : style.color);
    float font_size = marker_style.isExplicitlySet("font-size")
        ? marker_style.font_size : style.font_size;
    const std::string& font_family = marker_style.isExplicitlySet("font-family")
        ? marker_style.font_family : style.font_family;
    const std::string font_weight = toString(marker_style.isExplicitlySet("font-weight")
        ? marker_style.font_weight : style.font_weight);
    const std::string font_style_val = toString(marker_style.isExplicitlySet("font-style")
        ? marker_style.font_style : style.font_style);

    // Shape the marker text
    TextShapeRequest request;
    request.text = marker_text;
    request.font_family = font_family;
    request.font_weight = font_weight;
    request.font_style = font_style_val;
    request.font_size = font_size;
    request.origin_x = 0;
    request.origin_y = 0;

    ShapedText shaped;
    if (!text_shaper_.shape(request, shaped) || shaped.glyphs.empty()) return;

    float scale = shaped.scale_to_pixels;
    float ascent_px = shaped.ascent_units * scale;
    float marker_width = shaped.width_units * scale;
    float marker_height = shaped.line_height_units * scale;

    // Position: "outside" places marker left of the li content, "inside" prepends.
    const bool is_outside = (style.list_style_position != dom::CSSListStylePosition::Inside);
    const float kMarkerGap = 8.0f; // gap between marker and content
    float marker_x, marker_y;

    if (is_outside) {
        marker_x = node_rect.x - marker_width - kMarkerGap;
    } else {
        float pl = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
        float bl = std::max(0.0f, style.border_left_width >= 0.0f ? style.border_left_width : style.border_width);
        marker_x = node_rect.x + bl + pl;
        // Store the inside marker width so the li text renderer can offset its start position.
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f", marker_width + kMarkerGap);
        node->setAttribute("__inside_marker_width__", buf);
    }
    float pt = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
    float bt = std::max(0.0f, style.border_top_width >= 0.0f ? style.border_top_width : style.border_width);
    marker_y = node_rect.y + bt + pt;

    float baseline_y = marker_y + ascent_px;

    // Build glyph run
    DrawGlyphRunData glyph_run{};
    glyph_run.rect.x = marker_x;
    glyph_run.rect.y = marker_y;
    glyph_run.rect.width = marker_width;
    glyph_run.rect.height = marker_height;
    glyph_run.color = text_color;
    glyph_run.font_size = font_size;
    glyph_run.font_family = font_family;
    glyph_run.font_weight = font_weight;
    glyph_run.font_style = font_style_val;
    glyph_run.font_paths = shaped.font_paths;
    glyph_run.font_path = shaped.font_path;
    glyph_run.baseline_x = marker_x;
    glyph_run.baseline_y = baseline_y;
    glyph_run.units_per_em = shaped.units_per_em;
    glyph_run.scale_to_pixels = shaped.scale_to_pixels;

    for (const auto& sg : shaped.glyphs) {
        GlyphInstance inst{};
        inst.glyph_id = sg.glyph_id;
        inst.pen_x_units = sg.pen_x_units;
        inst.pen_y_units = sg.pen_y_units;
        inst.font_path_index = sg.font_path_index;
        inst.units_per_em = sg.units_per_em;
        glyph_run.glyphs.push_back(inst);
    }

    builder.addGlyphRun(glyph_run);
}

void Painter::renderDisclosureTriangle(const dom::DOMNodePtr& node,
                                      const Rect& node_rect,
                                      bool is_open,
                                      DisplayListBuilder& builder) {
    if (!node) return;

    const auto& style = node->getComputedStyle();

    // 计算三角指示器的位置和大小
    const float triangle_size = 8.0f; // 三角指示器的大小
    const float triangle_margin = 4.0f; // 与边框的距离

    // 三角指示器位于details元素的左上角
    float triangle_x = node_rect.x + triangle_margin;
    float triangle_y = node_rect.y + triangle_margin;

    // 获取颜色
    Color triangle_color = painter_detail::makeColorFromCss(style.color);

    // 根据open状态确定三角的方向
    if (is_open) {
        // 打开状态：向下三角（使用多个小矩形近似三角形）
        for (int i = 0; i < triangle_size; i++) {
            float width = triangle_size - i * 2;
            if (width <= 0) break;
            float y = triangle_y + i;
            Rect rect;
            rect.x = triangle_x + i;
            rect.y = y;
            rect.width = width;
            rect.height = 1.0f;
            builder.addRect(rect, triangle_color);
        }
    } else {
        // 关闭状态：向右三角（使用多个小矩形近似三角形）
        for (int i = 0; i < triangle_size; i++) {
            float height = triangle_size - i * 2;
            if (height <= 0) break;
            float x = triangle_x + i;
            Rect rect;
            rect.x = x;
            rect.y = triangle_y + i;
            rect.width = 1.0f;
            rect.height = height;
            builder.addRect(rect, triangle_color);
        }
    }
}

void Painter::paintChildrenAndOverlays(const dom::DOMNodePtr& node,
                                      const layout::LayoutNode* layout_node,
                                      const Rect& node_rect,
                                      bool has_layout_rect,
                                      bool is_scroll_container,
                                      DisplayListBuilder& builder) {
    if (!node) return;




    // 3.5 渲染 ::marker 伪元素（<li> 的列表标记）
    if (node->getTagName() == "li" && node->getPseudoMarker()) {
        renderMarkerForListItem(node, node_rect, builder);
    }

    // 4. 渲染 ::before 伪元素
    if (node->hasPseudoElements()) {
        const std::string inflow = node->getAttribute("__dong_pseudo_before_inflow__");
        if (inflow != "1") {
            auto pseudo_before = node->getPseudoBefore();
            if (pseudo_before) {
                renderPseudoElement(pseudo_before, node_rect, builder);
            }
        } else {
            node->setAttribute("__dong_pseudo_before_inflow__", "");
        }
    }


    // <select> 是 replaced control：其子树（<option>/<optgroup>）不走常规递归绘制。
    // 关闭态/打开态由 painter_select 负责渲染。
    if (node->getTagName() == "select") {
        // 6. 渲染 ::after 伪元素
        if (node->hasPseudoElements()) {
            auto pseudo_after = node->getPseudoAfter();
            if (pseudo_after) {
                renderPseudoElement(pseudo_after, node_rect, builder);
            }
        }
        return;
    }

    // <details> 元素：根据 open 属性控制子内容显示/隐藏
    if (node->getTagName() == "details") {
        // 获取 details 状态
        auto* details_state = dom::getDetailsState(node);
        if (!details_state) {
            // 如果没有状态对象，则创建并同步
            details_state = dom::getDetailsState(node);
        }

        // 渲染三角指示器
        renderDisclosureTriangle(node, node_rect, details_state->isOpen(), builder);

        // 如果 details 是关闭状态，则不渲染除 summary 外的子内容
        if (!details_state->isOpen()) {
            // 只渲染 summary 元素
            for (const auto& child : node->getChildren()) {
                if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT &&
                    child->getTagName() == "summary") {
                    const layout::LayoutNode* child_layout = layout_engine_ ? layout_engine_->getLayout(child) : nullptr;
                    buildDisplayListNode(child, child_layout, builder);
                }
            }

            // 渲染 ::after 伪元素
            if (node->hasPseudoElements()) {
                auto pseudo_after = node->getPseudoAfter();
                if (pseudo_after) {
                    renderPseudoElement(pseudo_after, node_rect, builder);
                }
            }
            return;
        }
        // 如果 details 是打开状态，则继续正常渲染所有子内容
    }

    // 5. 递归子节点（按 z-index 排序）
    const auto& children = node->getChildren();

    // 滚动容器内容高度：同一帧内会被多处使用（content_size + scrollbar），这里计算一次复用。
    float scroll_content_height = 0.0f;
    bool have_scroll_content_height = false;

    // 维护滚动容器的 client/content 尺寸（用于 scrollTo/scrollBy clamp 与滚动条绘制）
    Rect client_rect = node_rect;

    // Debug helper: print scroll-related styles and rect even if not treated as a scroll container.
    if (shouldDebugScrollMetricsForNode(node) && has_layout_rect) {
        const dom::ComputedStyle& dbg_style = node->getComputedStyle();
        std::fprintf(
            stderr,
            "[ScrollDbg] id=%s overflow=%s overflow_y=%s scroll_behavior=%s is_scroll_container=%d rect=(x=%.1f,y=%.1f,w=%.1f,h=%.1f)\n",
            node->getAttribute("id").c_str(),
            toString(dbg_style.overflow),
            toString(dbg_style.overflow_y),
            toString(dbg_style.scroll_behavior),
            is_scroll_container ? 1 : 0,
            node_rect.x, node_rect.y, node_rect.width, node_rect.height);
    }

    if (is_scroll_container && has_layout_rect) {
        const dom::ComputedStyle& style = node->getComputedStyle();
        client_rect = computeClientRectFromBorderBox(node_rect, style);

        float content_bottom = client_rect.y;
        if (layout_engine_) {
            // 用子树的最大 bottom 作为内容底部（比只看直接子节点更鲁棒）。
            content_bottom = computeScrollContentBottom(node, layout_engine_, content_bottom);
        }

        float content_height = content_bottom - client_rect.y;
        if (content_height < 0.0f) content_height = 0.0f;

        scroll_content_height = content_height;
        have_scroll_content_height = true;

        node->setClientRect(client_rect.y, client_rect.x, client_rect.width, client_rect.height);
        node->setContentSize(client_rect.width, content_height);

        if (shouldDebugScrollMetricsForNode(node)) {
            std::fprintf(
                stderr,
                "[ScrollMetrics] id=%s client=(x=%.1f,y=%.1f,w=%.1f,h=%.1f) content_h=%.1f scroll_y=%.1f max_scroll_y=%.1f\n",
                node->getAttribute("id").c_str(),
                client_rect.x, client_rect.y, client_rect.width, client_rect.height,
                node->getScrollHeight(),
                node->getScrollY(),
                std::max(0.0f, node->getScrollHeight() - node->getClientHeight()));
        }
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

    // 大多数子节点的 z-index 都是 0：避免每节点都分配 + sort（O(n log n)），直接按 DOM 顺序遍历。
    bool need_z_sort = false;
    for (const auto& child : children) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
            continue;
        }
        const auto& child_style = child->getComputedStyle();
        // Check if any child has a non-auto z-index (which creates a stacking context)
        if (child_style.z_index.has_value()) {
            need_z_sort = true;
            break;
        }
    }

    auto should_defer_sticky = [](const dom::DOMNodePtr& n) {
        return n && n->getType() == dom::DOMNode::NodeType::ELEMENT &&
               n->getComputedStyle().position == dom::CSSPosition::Sticky;
    };

    if (!need_z_sort) {
        // Sticky elements are positioned; when stuck they can overlap later siblings.
        // Paint stickies last so they appear above normal-flow content.
        std::vector<dom::DOMNodePtr> deferred_sticky;
        deferred_sticky.reserve(children.size());

        // CSS counter-reset sibling scoping (CSS Lists Level 3):
        // A counter-reset on element E creates an instance covering E and all
        // following siblings. When a later sibling F resets the SAME counter,
        // F's instance REPLACES E's (scope of E ends at F). We implement this by
        // popping the previous sibling-level instance before pushing the new one.
        // All sibling-level pushes are popped after the sibling list is done.
        std::unordered_map<std::string, int> sibling_pushed;  // name → count pushed at this level

        for (const auto& child : children) {
            if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                continue;
            }
            // 跳过已经在容器层绘制过的 inline 元素
            if (child->getAttribute("__inline_rendered__") == "1") {
                child->setAttribute("__inline_rendered__", "");  // 清除标记
                continue;
            }

            // Debug: detect if inline child is NOT being skipped in a CE container
            if (node->isContentEditable() && child->getTagName() == "span" &&
                child->getComputedStyle().display == dom::CSSDisplay::Inline) {
                DONG_LOG_WARN("[CHILDREN-CE] *** INLINE SPAN NOT SKIPPED! tag=%s display=%s inline_rendered=%s ***",
                              child->getTagName().c_str(),
                              toString(child->getComputedStyle().display),
                              child->getAttribute("__inline_rendered__").c_str());
            }

            // For each counter-reset in this child, if a previous sibling already
            // pushed that counter at this level, pop it first (replace semantics).
            if (!child->getComputedStyle().counter_resets.empty()) {
                for (const auto& r : child->getComputedStyle().counter_resets) {
                    if (r.name.empty()) continue;
                    auto it = sibling_pushed.find(r.name);
                    if (it != sibling_pushed.end() && it->second > 0) {
                        popCounterResetByName(r.name);
                        it->second -= 1;
                    }
                }
                pushCounterResetsOnly(child->getComputedStyle());
                for (const auto& r : child->getComputedStyle().counter_resets) {
                    if (!r.name.empty()) sibling_pushed[r.name]++;
                }
            }

            if (should_defer_sticky(child)) {
                deferred_sticky.push_back(child);
                continue;
            }

            const layout::LayoutNode* child_layout = layout_engine_ ? layout_engine_->getLayout(child) : nullptr;
            buildDisplayListNode(child, child_layout, builder);
        }

        for (const auto& child : deferred_sticky) {
            const layout::LayoutNode* child_layout = layout_engine_ ? layout_engine_->getLayout(child) : nullptr;
            buildDisplayListNode(child, child_layout, builder);
        }

        // Pop all remaining sibling-level counter-reset scopes.
        for (auto& kv : sibling_pushed) {
            for (int i = 0; i < kv.second; ++i) {
                popCounterResetByName(kv.first);
            }
        }
    } else {
        // 收集需要绘制的子元素及其 z-index
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
            item.z_index = child_style.z_index.value_or(0);  // auto is treated as 0 for sorting
            item.original_order = order++;
            sorted_children.push_back(item);
        }

        // 按 z-index 升序排序，相同 z-index 保持 DOM 顺序
        std::sort(sorted_children.begin(), sorted_children.end(),
                  [](const ChildWithZIndex& a, const ChildWithZIndex& b) {
                      if (a.z_index != b.z_index) {
                          return a.z_index < b.z_index;
                      }
                      return a.original_order < b.original_order;
                  });

        std::vector<dom::DOMNodePtr> deferred_sticky;
        deferred_sticky.reserve(sorted_children.size());

        for (const auto& item : sorted_children) {
            // 跳过已经在容器层绘制过的 inline 元素
            if (item.child->getAttribute("__inline_rendered__") == "1") {
                item.child->setAttribute("__inline_rendered__", "");  // 清除标记
                continue;
            }

            if (should_defer_sticky(item.child)) {
                deferred_sticky.push_back(item.child);
                continue;
            }

            const layout::LayoutNode* child_layout = layout_engine_ ? layout_engine_->getLayout(item.child) : nullptr;
            buildDisplayListNode(item.child, child_layout, builder);
        }

        for (const auto& child : deferred_sticky) {
            const layout::LayoutNode* child_layout = layout_engine_ ? layout_engine_->getLayout(child) : nullptr;
            buildDisplayListNode(child, child_layout, builder);
        }
    }

    // 恢复滚动偏移
    if (applied_scroll_translate) {
        builder.popTranslate();
    }

    // 6. 渲染 ::after 伪元素
    if (node->hasPseudoElements()) {
        const std::string inflow = node->getAttribute("__dong_pseudo_after_inflow__");
        if (inflow != "1") {
            auto pseudo_after = node->getPseudoAfter();
            if (pseudo_after) {
                renderPseudoElement(pseudo_after, node_rect, builder);
            }
        } else {
            node->setAttribute("__dong_pseudo_after_inflow__", "");
        }
    }


    // 7. 滚动条渲染（在子节点之后绘制，确保滚动条在内容之上）
    static const bool kDisableScrollbars = []() {
        const char* v = ::getenv("DONG_DISABLE_SCROLLBARS");
        return v && v[0] == '1';
    }();

    if (!kDisableScrollbars && is_scroll_container && layout_node && node_rect.width > 0.0f && node_rect.height > 0.0f) {
        // 内容高度：优先复用上面计算过的值，避免重复遍历 children。
        float content_height = scroll_content_height;
        if (!have_scroll_content_height) {
            float content_bottom = client_rect.y;
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

            content_height = content_bottom - client_rect.y;
            if (content_height < 0.0f) content_height = 0.0f;
        }

        float visible_height = client_rect.height;

        // 只有当内容高度大于可视高度时才显示滚动条
        if (content_height > visible_height + 1.0f) {
            const float kScrollbarWidth = 8.0f;
            const float kScrollbarMinThumbHeight = 20.0f;
            const float kScrollbarPadding = 2.0f;

            // 滚动条轨道位置（在 client 区域右侧）
            Rect track_rect{};
            track_rect.x = client_rect.x + client_rect.width - kScrollbarWidth - kScrollbarPadding;
            track_rect.y = client_rect.y + kScrollbarPadding;
            track_rect.width = kScrollbarWidth;
            track_rect.height = client_rect.height - kScrollbarPadding * 2.0f;

            Color track_color{};
            track_color.r = 0.0f;
            track_color.g = 0.0f;
            track_color.b = 0.0f;
            track_color.a = 0.1f;
            builder.addRoundedRect(track_rect, track_color, kScrollbarWidth * 0.5f);

            // 计算滑块高度和位置
            float thumb_height_ratio = visible_height / content_height;
            float thumb_height = std::max(kScrollbarMinThumbHeight, track_rect.height * thumb_height_ratio);

            float scroll_position = node->getScrollY();
            float max_scroll = content_height - visible_height;
            float scroll_ratio = (max_scroll > 0.0f) ? (scroll_position / max_scroll) : 0.0f;
            scroll_ratio = std::max(0.0f, std::min(scroll_ratio, 1.0f));

            float thumb_y = track_rect.y + (track_rect.height - thumb_height) * scroll_ratio;

            Rect thumb_rect{};
            thumb_rect.x = track_rect.x;
            thumb_rect.y = thumb_y;
            thumb_rect.width = kScrollbarWidth;
            thumb_rect.height = thumb_height;

            Color thumb_color{};
            thumb_color.r = 0.4f;
            thumb_color.g = 0.4f;
            thumb_color.b = 0.4f;
            thumb_color.a = 0.5f;
            builder.addRoundedRect(thumb_rect, thumb_color, kScrollbarWidth * 0.5f);
        }
    }
}

} // namespace dong::render
