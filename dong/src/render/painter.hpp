#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../dom/dom_node.hpp"
#include "../layout/layout_engine.hpp"
#include "render_surface.hpp"
#include "display_list.hpp"
#include "text_shaper.hpp"

namespace dong::render {

// 纯 DisplayList 构建器（不依赖 Skia）
class Painter {
public:
    Painter(RenderSurface* surface);
    ~Painter();

    // 构建 DisplayList（从 DOM/Layout 遍历生成）
    const DisplayList& buildDisplayList(const dom::DOMNodePtr& root, layout::Engine* layout_engine);

    // 访问最近一帧构建好的 DisplayList
    const DisplayList& getDisplayList() const { return display_list_builder_.get(); }

private:
    RenderSurface* surface_;
    layout::Engine* layout_engine_;
    layout::DirtyRect current_dirty_rect_;
    bool use_dirty_rect_ = true;

    // DisplayList 构建器
    DisplayListBuilder display_list_builder_;
    TextShaper text_shaper_;

    // 从 DOM/Layout 构建 DisplayList（递归遍历）
    void buildDisplayListNode(const dom::DOMNodePtr& node,
                              const layout::LayoutNode* layout_node,
                              DisplayListBuilder& builder);

    // 脏矩形优化
    bool isNodeInDirtyRect(const layout::LayoutNode* layout_node) const;
};

using PainterPtr = std::unique_ptr<Painter>;

} // namespace dong::render
