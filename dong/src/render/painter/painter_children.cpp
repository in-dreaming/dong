#include "../painter.hpp"

#include <algorithm>
#include <vector>

namespace dong::render {

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
        item.z_index = child_style.z_index;
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

    for (const auto& item : sorted_children) {
        // 跳过已经在容器层绘制过的 inline 元素
        if (item.child->getAttribute("__inline_rendered__") == "1") {
            item.child->setAttribute("__inline_rendered__", "");
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
            constexpr float kScrollbarWidth = 8.0f;
            constexpr float kScrollbarMinThumbHeight = 20.0f;
            constexpr float kScrollbarPadding = 2.0f;

            // 滚动条轨道位置（在容器右侧）
            Rect track_rect{};
            track_rect.x = node_rect.x + node_rect.width - kScrollbarWidth - kScrollbarPadding;
            track_rect.y = node_rect.y + kScrollbarPadding;
            track_rect.width = kScrollbarWidth;
            track_rect.height = node_rect.height - kScrollbarPadding * 2.0f;

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
