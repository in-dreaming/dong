#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../dom/dom/dom_node.hpp"


#include "../layout/layout_engine.hpp"
#include "render_surface.hpp"
#include "display_list.hpp"
#include "layer_tree.hpp"
#include "text_shaper.hpp"

namespace dong::render {

// 纯 DisplayList 构建器（不依赖 Skia）
class Painter {
public:
    Painter(RenderSurface* surface);
    ~Painter();

    // 构建 DisplayList（从 DOM/Layout 遍历生成）
    const DisplayList& buildDisplayList(const dom::DOMNodePtr& root, layout::Engine* layout_engine);

    // 访问最近一帧构建好的 DisplayList / LayerTree
    const DisplayList& getDisplayList() const { return display_list_builder_.get(); }
    const LayerTree& getLayerTree() const { return layer_tree_; }

private:
    RenderSurface* surface_;
    layout::Engine* layout_engine_;
    layout::DirtyRect current_dirty_rect_;
    bool use_dirty_rect_ = false; // 先关闭基于 DirtyRect 的裁剪，避免与后续图层缓存行为冲突

    // DisplayList / LayerTree 构建器
    DisplayListBuilder display_list_builder_;
    LayerTree layer_tree_;
    std::vector<int> layer_stack_;
    TextShaper text_shaper_;

    // 从 DOM/Layout 构建 DisplayList（递归遍历）
    void buildDisplayListNode(const dom::DOMNodePtr& node,
                              const layout::LayoutNode* layout_node,
                              DisplayListBuilder& builder);

    // --- buildDisplayListNode helpers (split to keep this function manageable) ---
    void paintMediaElements(const dom::DOMNodePtr& node,
                            const layout::LayoutNode* layout_node,
                            const std::string& tag,
                            const dom::ComputedStyle& style,
                            bool is_hidden,
                            DisplayListBuilder& builder);

    void paintTextAndInput(const dom::DOMNodePtr& node,
                           const layout::LayoutNode* layout_node,
                           const std::string& tag,
                           const dom::ComputedStyle& style,
                           bool is_hidden,
                           DisplayListBuilder& builder);

    void paintChildrenAndOverlays(const dom::DOMNodePtr& node,
                                 const layout::LayoutNode* layout_node,
                                 const Rect& node_rect,
                                 bool has_layout_rect,
                                 bool is_scroll_container,
                                 DisplayListBuilder& builder);

    // 渲染伪元素 (::before/::after)
    void renderPseudoElement(const dom::DOMNodePtr& pseudo,
                             const Rect& parent_rect,
                             DisplayListBuilder& builder);

    // 渲染 ::marker 伪元素（列表项标记）
    void renderMarkerForListItem(const dom::DOMNodePtr& node,
                                 const Rect& node_rect,
                                 DisplayListBuilder& builder);


    // 脏矩形优化
    bool isNodeInDirtyRect(const layout::LayoutNode* layout_node) const;
    bool isRectInDirtyRect(const Rect& rect) const;

    // --- buildDisplayListNode 重构辅助方法 ---
    struct LayerDecision {
        bool needs_layer = false;
        bool has_isolation = false;
        bool is_scroll_container = false;
        bool pushed_layer = false;
        bool content_dirty = false;
        float clamped_opacity = 1.0f;
    };

    // 阶段 1: 检查是否需要跳过此节点
    bool shouldSkipNode(const dom::DOMNodePtr& node, const dom::ComputedStyle& style) const;

    // 阶段 2: Layer 相关决策和设置
    LayerDecision decideLayerNeeds(const dom::DOMNodePtr& node,
                                   const layout::LayoutNode* layout_node,
                                   const dom::ComputedStyle& style,
                                   const Rect& node_rect,
                                   bool has_layout_rect,
                                   DisplayListBuilder& builder);

    Rect computeLayerBounds(const Rect& node_rect, bool has_layout_rect,
                            float builder_tx, float builder_ty) const;

    LayerTransform computeLayerTransform(const dom::ComputedStyle& style,
                                         const Rect& layer_bounds) const;

    bool shouldSkipCachedLayer(const LayerDecision& decision) const;

    // 阶段 3: 边框计算
    struct BorderWidths {
        float top = 0, right = 0, bottom = 0, left = 0, max = 0;
    };
    BorderWidths computeBorderWidths(const dom::ComputedStyle& style) const;

    // 阶段 4: 阴影绘制
    void paintBoxShadow(const Rect& rect, const dom::ComputedStyle& style,
                        DisplayListBuilder& builder) const;

    // 阶段 5: 背景和边框绘制
    void paintBackgroundAndBorder(const Rect& rect,
                                  const BorderWidths& bw,
                                  const dom::ComputedStyle& style,
                                  DisplayListBuilder& builder);

    // 阶段 6: Checkbox/Radio 标记
    void paintCheckboxMark(const dom::DOMNodePtr& node,
                           const Rect& rect,
                           const BorderWidths& bw,
                           DisplayListBuilder& builder) const;
};

using PainterPtr = std::unique_ptr<Painter>;

} // namespace dong::render
