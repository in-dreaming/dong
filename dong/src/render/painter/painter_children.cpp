#include "../painter.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>

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
            current_max_bottom = std::max(current_max_bottom, child_bottom);
        }

        current_max_bottom = computeScrollContentBottom(child, layout_engine, current_max_bottom);
    }

    return current_max_bottom;
}

inline float effectiveBorderWidthPx(const dom::ComputedStyle& style,
                                   float side_width,
                                   const std::string& side_style) {
    const std::string& st = !side_style.empty() ? side_style : style.border_style;
    if (st == "none" || st == "hidden") {
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
    static const bool kEnabled = []() {
        const char* v = ::getenv("DONG_DEBUG_SCROLL_METRICS");
        return v && v[0] == '1';
    }();

    if (!kEnabled || !node) {
        return false;
    }

    const std::string id = node->getAttribute("id");
    return id == "sc";
}

} // namespace

void Painter::paintChildrenAndOverlays(const dom::DOMNodePtr& node,
                                      const layout::LayoutNode* layout_node,
                                      const Rect& node_rect,
                                      bool has_layout_rect,
                                      bool is_scroll_container,
                                      DisplayListBuilder& builder) {
    if (!node) return;

    // 4. 渲染 ::before 伪元素
    if (node->hasPseudoElements()) {
        auto pseudo_before = node->getPseudoBefore();
        if (pseudo_before) {
            renderPseudoElement(pseudo_before, node_rect, builder);
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

    // 5. 递归子节点（按 z-index 排序）
    const auto& children = node->getChildren();

    // 滚动容器内容高度：同一帧内会被多处使用（content_size + scrollbar），这里计算一次复用。
    float scroll_content_height = 0.0f;
    bool have_scroll_content_height = false;

    // 维护滚动容器的 client/content 尺寸（用于 scrollTo/scrollBy clamp 与滚动条绘制）
    Rect client_rect = node_rect;
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
               n->getComputedStyle().position == "sticky";
    };

    if (!need_z_sort) {
        // Sticky elements are positioned; when stuck they can overlap later siblings.
        // Paint stickies last so they appear above normal-flow content.
        std::vector<dom::DOMNodePtr> deferred_sticky;
        deferred_sticky.reserve(children.size());

        for (const auto& child : children) {
            if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                continue;
            }
            // 跳过已经在容器层绘制过的 inline 元素
            if (child->getAttribute("__inline_rendered__") == "1") {
                child->setAttribute("__inline_rendered__", "");  // 清除标记
                continue;
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
        auto pseudo_after = node->getPseudoAfter();
        if (pseudo_after) {
            renderPseudoElement(pseudo_after, node_rect, builder);
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
            constexpr float kScrollbarWidth = 8.0f;
            constexpr float kScrollbarMinThumbHeight = 20.0f;
            constexpr float kScrollbarPadding = 2.0f;

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
            scroll_ratio = std::clamp(scroll_ratio, 0.0f, 1.0f);

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
