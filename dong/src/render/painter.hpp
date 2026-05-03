#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "../dom/dom/dom_node.hpp"

namespace dong::dom {
class Selection;
}



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

    // Set the resource root directory used to resolve relative image paths.
    void setResourceRoot(const std::string& root) { resource_root_ = root; }
    const std::string& getResourceRoot() const { return resource_root_; }

    // Set editing state for contenteditable caret/selection rendering
    void setEditingState(const dom::DOMNodePtr& focused, const dom::Selection* sel, bool caret_visible = true);

    // Set editing state for input/textarea caret/selection rendering
    void setInputEditingState(const dom::DOMNodePtr& focused_input, bool caret_visible);

    // Text renderer mode preference (propagated to glyph runs)
    void setTextRendererMode(TextRendererMode mode) { text_renderer_mode_ = mode; }
    TextRendererMode getTextRendererMode() const { return text_renderer_mode_; }

    // P0-6 S4: Partial repaint support.
    // Set the dirty node pointers for this frame. If non-empty, buildDisplayList
    // will reuse cached display items for nodes NOT in this set.
    // Pass an empty set to force full repaint (default behavior).
    // Also pass the set of all ancestor nodes of dirty nodes, so that
    // container nodes with dirty descendants are NOT cached.
    void setDirtyNodes(const std::unordered_set<const void*>& dirty,
                       const std::unordered_set<const void*>& ancestors) {
        dirty_nodes_ = &dirty;
        dirty_ancestors_ = &ancestors;
        partial_repaint_active_ = !dirty.empty();
    }
    void clearDirtyNodes() {
        dirty_nodes_ = nullptr;
        dirty_ancestors_ = nullptr;
        partial_repaint_active_ = false;
    }

    // Stats: how many nodes were skipped in the last partial repaint
    uint32_t getLastPartialSkipCount() const { return partial_skip_count_; }
    uint32_t getLastPartialRepaintCount() const { return partial_repaint_count_; }

    // 访问最近一帧构建好的 DisplayList / LayerTree
    const DisplayList& getDisplayList() const { return display_list_builder_.get(); }
    const LayerTree& getLayerTree() const { return layer_tree_; }

    // Append pre-built items (e.g. from overlay direct-draw, bypassing DOM/layout).
    void appendOverlayItems(const std::vector<DisplayItem>& items) {
        display_list_builder_.appendItems(items);
    }

    // Restore a previously cached DOM display list (skip-if-clean optimization).
    void restoreDisplayList(const std::vector<DisplayItem>& items) {
        display_list_builder_.clear();
        display_list_builder_.appendItems(items);
    }

    void clearDisplayList() {
        display_list_builder_.clear();
    }

    // Evaluate generated content text for a pseudo-element style.
    // Called from inline rendering helpers (e.g. for open-quote/close-quote in <q>).
    std::string evaluateContentText(const dom::ComputedStyle& style, const dom::DOMNodePtr& node = nullptr);

private:
    RenderSurface* surface_;
    layout::Engine* layout_engine_;
    layout::DirtyRect current_dirty_rect_;
    bool use_dirty_rect_ = false; // 先关闭基于 DirtyRect 的裁剪，避免与后续图层缓存行为冲突
    std::string resource_root_;  // Base directory for resolving relative image paths

    // Editing state for contenteditable caret/selection rendering
    dom::DOMNodePtr focused_editable_;
    const dom::Selection* editing_selection_ = nullptr;
    bool caret_visible_ = true;

    // Editing state for input/textarea caret/selection rendering
    dom::DOMNodePtr focused_input_;
    bool input_caret_visible_ = false;

    // DisplayList / LayerTree 构建器
    DisplayListBuilder display_list_builder_;
    LayerTree layer_tree_;
    std::vector<int> layer_stack_;
    // Top-layer modal dialogs deferred from normal tree traversal
    std::vector<dom::DOMNodePtr> top_layer_modals_;
    TextShaper text_shaper_;
    TextRendererMode text_renderer_mode_ = TextRendererMode::Auto;

    // P0-6 S4: Partial repaint cached state
    const std::unordered_set<const void*>* dirty_nodes_ = nullptr;
    const std::unordered_set<const void*>* dirty_ancestors_ = nullptr;
    bool partial_repaint_active_ = false;
    // Cache of previous frame's node ranges (maps node_ptr -> item range in previous display list)
    std::unordered_map<const void*, std::pair<uint32_t, uint32_t>> prev_node_ranges_;
    // Cache of previous frame's display items (for copying clean node items)
    std::vector<DisplayItem> prev_display_items_;
    // Stats
    uint32_t partial_skip_count_ = 0;
    uint32_t partial_repaint_count_ = 0;

    // Generated content (counter()/counters()/open-quote/close-quote)
    struct GeneratedContentState {
        std::unordered_map<std::string, std::vector<int>> counters;
        std::vector<std::vector<std::string>> pushed_names_stack;
        int quote_depth = 0;
    } gen_;

    void pushCounterScope(const dom::ComputedStyle& style);
    void popCounterScope();
    // Push only the counter-reset portion (no increments). Used by the sibling
    // iteration loop so that a counter-reset on element E is visible to E's
    // following siblings (CSS spec: counter-reset scope covers following siblings).
    void pushCounterResetsOnly(const dom::ComputedStyle& style);
    void applyCounterIncrementsOnly(const dom::ComputedStyle& style);
    // Pop the last pushed instance of a specific counter (used when a later sibling
    // resets the same counter, replacing the previous sibling-level instance).
    void popCounterResetByName(const std::string& name);
    std::string evaluateCounterText(const std::string& name, const std::string& style = "decimal");
    std::string evaluateCountersText(const std::string& name, const std::string& sep, const std::string& style = "decimal");
    std::string evaluateQuoteToken(const dom::ComputedStyle& style,
                                  const dom::ComputedStyle::ContentToken& tok,
                                  const dom::DOMNodePtr& node);


    struct OpenSelectOverlay {
        dom::DOMNodePtr node;
        const layout::LayoutNode* layout = nullptr;
        float tx = 0.0f;
        float ty = 0.0f;
        float bl = 0.0f;
        float bt = 0.0f;
        float br = 0.0f;
        float bb = 0.0f;
    };
    std::vector<OpenSelectOverlay> open_select_overlays_;

    void paintSelectDropdownOverlays(DisplayListBuilder& builder);


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

    void paintVideoElement(const dom::DOMNodePtr& node,
                           const layout::LayoutNode* layout_node,
                           const dom::ComputedStyle& style,
                           DisplayListBuilder& builder);

    void paintVideoPlaceholder(const Rect& rect, DisplayListBuilder& builder);

    void renderAltText(const Rect& rect, const std::string& alt_text, const dom::ComputedStyle& style, DisplayListBuilder& builder);

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

    // 渲染 disclosure triangle for details element
    void renderDisclosureTriangle(const dom::DOMNodePtr& node,
                                  const Rect& node_rect,
                                  bool is_open,
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

    // 阶段 7: textarea resize handle（仅绘制可视把手，不做交互拖拽）
    void paintTextareaResizeHandle(const dom::DOMNodePtr& node,
                                   const Rect& rect,
                                   const BorderWidths& bw,
                                   DisplayListBuilder& builder) const;
};


using PainterPtr = std::unique_ptr<Painter>;

} // namespace dong::render
