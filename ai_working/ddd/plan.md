# DDD Plan: P1 布局/定位正确性特性

**Date**: 2026-02-23
**Status**: Planning Complete
**Features**: position: sticky, aspect-ratio, display: contents, display: flow-root

---

## Problem Statement

Dong Engine 需要实现四个 P1 优先级的 CSS 布局/定位特性，以提升 Web 标准对齐度和主流 UI 框架的兼容性。这些特性被归类为"高 ROI"是因为它们：

1. **影响基本布局正确性** - 现代 Web 应用广泛使用这些特性
2. **提升开发者体验** - 减少对 JavaScript hack 的依赖  
3. **标准合规性** - 这些是 CSS 规范的核心特性，非长尾需求

### 用户价值

- **position: sticky**: 无需 JavaScript 即可实现吸附式导航栏、侧边栏
- **aspect-ratio**: 响应式媒体（图片/视频）自动保持宽高比
- **display: contents**: 简化 DOM 结构，让父元素在布局中"透明化"
- **display: flow-root**: 创建块级格式化上下文（BFC），正确处理边距折叠

### 当前痛点

**缺少这些特性导致的问题**：
- 开发者需要使用 JavaScript 监听滚动事件来模拟 sticky（性能差）
- 图片/视频需要 padding-hack 或 JavaScript 计算来保持宽高比
- 无法优雅地实现某些 CSS 框架的布局模式
- 边距折叠行为不正确（margin collapse 没有被 BFC 阻止）

---

## Proposed Solution

采用**标准优先**的实现策略，严格遵循 CSS 规范，而非简化版本。

### 设计原则

1. **标准合规** - 严格遵循 CSS 规范行为，不做简化妥协
2. **渐进增强** - 按复杂度递增顺序实现
3. **模块化** - 每个特性独立模块，清晰的接口边界
4. **Yoga 协同** - 在 Yoga 布局前/后进行预处理/后处理，不修改 Yoga 库本身

### 实现顺序（按复杂度）

1. **display: flow-root** - 最简单，立即可观察的行为（边距折叠）
2. **aspect-ratio** - 中等复杂度，需要处理 min/max 约束
3. **display: contents** - 中等复杂度，需要事件处理和伪元素
4. **position: sticky** - 最复杂，需要滚动容器跟踪和布局/渲染协同

---

## Alternatives Considered

### position: sticky

**方案 A（已拒绝）**: 仅在渲染时覆盖位置
- ❌ 不符合 CSS 规范（sticky 必须参与布局流）
- ❌ 不会创建 sticky positioning containing block
- ❌ 兄弟元素无法感知 sticky 元素的空间

**方案 B（采纳）**: 布局时保留空间 + 渲染时视觉调整
- ✅ 符合 CSS Position Module Level 3 规范
- ✅ 创建正确的 containing block 层级
- ✅ 兄弟元素布局正确（sticky 元素占用正常流空间）

### aspect-ratio

**方案 A（已拒绝）**: 简单的预布局计算
- ❌ 缺少 min/max 约束交互
- ❌ 不处理替换元素（replaced elements）
- ❌ 忽略 aspect-ratio 约束模式（两个维度都指定时）

**方案 B（采纳）**: 完整的 CSS Box Sizing Module Level 4 实现
- ✅ Transferred size suggestions with min/max
- ✅ 替换元素的固有宽高比
- ✅ Flex/Grid 中的约束行为

### display: contents

**方案 A（部分采纳）**: Yoga 树手术（跳过节点创建）
- ✅ 简洁高效
- ⚠️ 需要补充事件处理和伪元素渲染

**最终方案**: 方案 A + 事件穿透 + 伪元素支持

### display: flow-root

**方案 A（已拒绝）**: 仅语义标记，无行为
- ❌ 即使没有 float 支持，BFC 的边距折叠行为也是可实现的

**方案 B（采纳）**: 立即实现 BFC 的边距折叠阻止行为
- ✅ 符合 CSS Display Module Level 3
- ✅ 可观察、可测试
- ✅ 为未来的 float 支持打下基础

---

## Architecture & Design

### 核心架构原则

**布局与渲染分离**：
- **布局阶段（Layout Phase）**: 计算假设没有滚动的位置
- **渲染阶段（Render Phase）**: 应用滚动感知的视觉调整

这适用于：
- `position: sticky` - 布局保留空间，渲染视觉调整
- `position: fixed` - 布局从流中移除，渲染相对于视口定位
- 滚动 - 布局与滚动无关，渲染应用滚动偏移

### 关键接口（Studs）

#### 1. BFC 边距折叠检查器

```cpp
// Interface: 检查两个节点间是否应该进行边距折叠
bool shouldCollapseMargins(
    dom::DOMNodePtr parent,
    dom::DOMNodePtr child,
    MarginEdge edge  // Top or Bottom
) -> bool
```

#### 2. 宽高比约束解析器

```cpp
struct ResolvedDimensions {
    float width;
    float height;
    bool width_constrained;
    bool height_constrained;
};

ResolvedDimensions resolveAspectRatio(
    const ComputedStyle& style,
    float available_width,
    float available_height,
    float natural_width,
    float natural_height
) -> ResolvedDimensions
```

#### 3. Sticky 定位上下文

```cpp
struct StickyPositioningContext {
    RectF original_position;
    RectF containing_block_bounds;
    dom::DOMNodePtr scroll_container;
    float inset_top, inset_bottom;
    float inset_left, inset_right;
};

StickyPositioningContext computeStickyContext(
    dom::DOMNodePtr node,
    const LayoutNode* layout
) -> StickyPositioningContext

RectF applyStickyOffset(
    const StickyPositioningContext& ctx,
    float scroll_offset_x,
    float scroll_offset_y
) -> RectF
```

#### 4. Display Contents 树构建

```cpp
void buildChildYogaNodes(
    dom::DOMNodePtr parent,
    YGNodePtr parent_yoga,
    std::unordered_map<dom::DOMNode*, YGNodePtr>& yoga_map
)
```

### 模块边界

#### Module 1: margin_collapse.cpp

**职责**: 边距折叠规则判定
**接口**: `shouldCollapseMargins()`, `createsBFC()`
**依赖**: `computed_style.hpp`, `dom_node.hpp`

#### Module 2: aspect_ratio_resolver.cpp

**职责**: 宽高比约束解析
**接口**: `resolveAspectRatio()`, `computeTransferredSize()`
**依赖**: `computed_style.hpp`, `yoga`
**算法**: CSS Box Sizing Module Level 4, Section 5

#### Module 3: sticky_positioning.cpp

**职责**: Sticky 定位计算
**接口**: `computeStickyContext()`, `applyStickyOffset()`, `findScrollContainer()`
**依赖**: `layout_engine.hpp`, `dom_node.hpp`

#### Module 4: display_contents.cpp

**职责**: Display contents 元素处理
**接口**: `buildChildYogaNodesWithContents()`, `shouldSkipLayout()`
**依赖**: `layout_engine.hpp`, `yoga`

### 数据模型

#### ComputedStyle 扩展

```cpp
class ComputedStyle {
public:
    float aspect_ratio = 0.0f;
    bool creates_block_formatting_context = false;
    void updateBFCFlag();
};
```

#### LayoutNode 扩展

```cpp
struct StickyMetadata {
    RectF original_position;
    RectF containing_block_bounds;
    dom::DOMNodePtr scroll_container;
    float inset_top = 0.0f;
    float inset_bottom = 0.0f;
    float inset_left = 0.0f;
    float inset_right = 0.0f;
    bool is_stuck = false;
    RectF visual_position;
};

class LayoutNode {
    std::unique_ptr<StickyMetadata> sticky_context;
};
```

---

## Files to Change

### Non-Code Files (Phase 2)

- [ ] `doc/todo.md` - 标记 P1 布局特性为已完成
- [ ] `CLAUDE.md` - 更新布局特性文档
- [ ] `doc/css_features.md` - 新增文件，记录 CSS 特性支持状态
- [ ] `examples/data/tests/README.md` - 更新测试用例列表

### Code Files (Phase 4)

#### 阶段 1: display: flow-root

- [ ] `src/dom/css/computed_style.hpp` - 添加 `creates_block_formatting_context` 字段
- [ ] `src/dom/css/computed_style.cpp` - 实现 `updateBFCFlag()` 方法
- [ ] `src/dom/css/css_parser.cpp` - 解析 `display: flow-root`
- [ ] `src/layout/margin_collapse.cpp` - 新建文件
- [ ] `src/layout/margin_collapse.hpp` - 新建文件
- [ ] `src/layout/layout_engine.cpp` - 在布局计算中调用边距折叠检查
- [ ] `examples/data/tests/test_flow_root_margin_collapse.html` - 新建测试

#### 阶段 2: aspect-ratio

- [ ] `src/dom/css/computed_style.hpp` - 添加 `aspect_ratio` 字段
- [ ] `src/dom/css/css_parser.cpp` - 解析 `aspect-ratio: <ratio>` 语法
- [ ] `src/layout/aspect_ratio_resolver.cpp` - 新建文件
- [ ] `src/layout/aspect_ratio_resolver.hpp` - 新建文件
- [ ] `src/layout/layout_engine.cpp` - 在 `applyDOMStylesToYoga` 中调用解析器
- [ ] `examples/data/tests/test_aspect_ratio_width_auto_height.html` - 新建测试
- [ ] `examples/data/tests/test_aspect_ratio_min_max.html` - 新建测试
- [ ] `examples/data/tests/test_aspect_ratio_flex.html` - 新建测试

#### 阶段 3: display: contents

- [ ] `src/layout/display_contents.cpp` - 新建文件
- [ ] `src/layout/display_contents.hpp` - 新建文件
- [ ] `src/layout/layout_engine.cpp` - 修改 `createYogaNode` 和 `buildChildYogaNodes`
- [ ] `src/render/painter.cpp` - 处理 `layout_cache[node] == nullptr` 情况
- [ ] `src/render/painter.cpp` - 渲染伪元素（::before, ::after）
- [ ] `src/core/engine_view.cpp` - 事件冒泡穿透 contents 元素
- [ ] `examples/data/tests/test_display_contents_layout.html` - 新建测试
- [ ] `examples/data/tests/test_display_contents_pseudo.html` - 新建测试
- [ ] `examples/data/tests/test_display_contents_events.html` - 新建测试

#### 阶段 4: position: sticky

- [ ] `src/layout/layout_node.hpp` - 添加 `StickyMetadata` 结构体
- [ ] `src/layout/sticky_positioning.cpp` - 新建文件
- [ ] `src/layout/sticky_positioning.hpp` - 新建文件
- [ ] `src/layout/layout_engine.cpp` - 在 `layoutPositionedElements` 中计算 sticky 上下文
- [ ] `src/layout/layout_engine.cpp` - 实现 `findScrollContainer` 辅助函数
- [ ] `src/render/painter.cpp` - 在渲染时应用 sticky 偏移
- [ ] `src/render/painter.cpp` - 添加滚动偏移参数传递
- [ ] `src/core/engine_view.cpp` - 将滚动偏移传递给 painter
- [ ] `examples/data/tests/test_sticky_scroll_top.html` - 新建测试
- [ ] `examples/data/tests/test_sticky_scroll_bottom.html` - 新建测试
- [ ] `examples/data/tests/test_sticky_parent_clamp.html` - 新建测试
- [ ] `examples/data/tests/test_sticky_nested.html` - 新建测试

---

## Philosophy Alignment

### Ruthless Simplicity

**平衡标准合规与实现复杂度**：
- ✅ flow-root: 仅实现边距折叠，暂不实现 float 包含
- ✅ aspect-ratio: 在 Yoga 前预处理，不修改 Yoga 库
- ✅ contents: 树手术法，不创建不必要的占位符节点
- ✅ sticky: 分离布局与渲染逻辑

**避免过度设计**：
- 不为未来的特性（如 float）添加复杂抽象
- 每个特性独立模块，可独立测试和调试
- 最小化对现有代码的侵入

### Modular Design ("Bricks & Studs")

每个特性是独立的"砖块"，通过清晰的接口连接。

### Present-Moment Focus

- 实现当前需要的特性
- Flow-root 暂不实现 float 包含（float 未支持）
- 文档清晰说明当前实现范围和未来计划

---

## Test Strategy

### 单元测试

每个模块的独立测试，包含边距折叠、宽高比解析、sticky 定位等测试用例。

### 集成测试（HTML 测试用例）

基于浏览器对比的测试（使用 `run_baseline_compare.py`）。

### 用户测试

手动交互测试和性能测试。

---

## Implementation Approach

### Phase 2 (Docs)

更新所有非代码文件，包括 `doc/todo.md`, `CLAUDE.md`, `doc/css_features.md` 等。

### Phase 4 (Code)

按阶段实现：flow-root (Week 1) → aspect-ratio (Week 2) → display: contents (Week 3) → position: sticky (Week 4-5)

---

## Success Criteria

### display: flow-root

- [ ] `creates_block_formatting_context` 标记正确设置
- [ ] 父子边距不折叠（BFC 阻止）
- [ ] 测试用例通过基准对比
- [ ] 不破坏现有布局

### aspect-ratio

- [ ] 图片 `width: 100px; aspect-ratio: 16/9` 渲染高度为 56.25px
- [ ] Flex item 使用 aspect-ratio 保持宽高比
- [ ] Min/max 约束覆盖 aspect-ratio
- [ ] 测试用例通过基准对比

### display: contents

- [ ] Contents 元素不渲染盒模型
- [ ] 子元素布局为祖父的直接子元素
- [ ] 伪元素仍然渲染
- [ ] 事件冒泡穿透 contents 元素
- [ ] 测试用例通过基准对比

### position: sticky

- [ ] 元素正常流中滚动，到达阈值后吸附
- [ ] Sticky 元素 clamp 到父元素底部边界
- [ ] 多个 sticky 元素正确堆叠
- [ ] 滚动时无布局抖动
- [ ] 滚动性能保持 60 FPS
- [ ] 测试用例通过基准对比

---

## Next Steps

✅ **Phase 1 Complete**: Planning Approved

**Plan written to**: `ai_working/ddd/plan.md`

**Next Phase**: Update all non-code files

**Run**: `/ddd:2-docs`

---

## Conclusion

本计划提供了一个**标准优先、渐进增强**的实现路径，将四个 P1 布局特性按复杂度递增顺序实现。每个特性都遵循模块化设计原则，有清晰的接口和独立的测试策略。

**总实施时间**: 约 5 周
- Week 1: display: flow-root
- Week 2: aspect-ratio
- Week 3: display: contents
- Week 4-5: position: sticky

**预期成果**: Dong Engine 的 Web 标准对齐度显著提升，主流 UI 框架的布局特性得到支持。
