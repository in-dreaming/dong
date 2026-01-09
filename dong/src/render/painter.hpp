#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../dom/dom_node.hpp"
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

    // 脏矩形优化
    bool isNodeInDirtyRect(const layout::LayoutNode* layout_node) const;
    bool isRectInDirtyRect(const Rect& rect) const;
};

using PainterPtr = std::unique_ptr<Painter>;

} // namespace dong::render
