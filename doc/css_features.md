# CSS Features Support Status

本文档记录 Dong Engine 对 CSS 特性的支持状态。

## Layout & Positioning（布局与定位）

### position: sticky ✅

**状态**: 已实现

**规范**: CSS Position Module Level 3

**功能描述**:
- 滚动感知定位：元素在正常流中滚动，到达阈值后吸附到视口边缘
- Sticky positioning containing block：创建正确的 containing block 层级
- 布局时保留空间：sticky 元素占用正常流空间（兄弟元素布局正确）
- 渲染时视觉调整：根据滚动位置调整视觉位置
- 父元素边界 clamp：sticky 元素 clamp 到包含块的 padding box（非 content box）
- 支持嵌套 sticky 元素

**实现细节**:
- 布局阶段：计算 sticky 上下文（原始位置、包含块边界、滚动容器、inset 值）
- 渲染阶段：应用滚动感知偏移，clamp 到包含块边界
- 模块：`src/layout/sticky_positioning.cpp`

**测试用例**:
- `test_sticky_scroll_top.html` - 顶部吸附
- `test_sticky_scroll_bottom.html` - 底部吸附
- `test_sticky_parent_clamp.html` - 父元素边界 clamp
- `test_sticky_nested.html` - 嵌套 sticky

**已知限制**:
- 暂不支持 transform 祖先的影响（未来扩展）

---

### aspect-ratio ✅

**状态**: 已实现

**规范**: CSS Box Sizing Module Level 4

**功能描述**:
- 宽高比约束：自动保持元素宽高比
- Transferred size suggestions：width 固定时计算 height，反之亦然
- Min/max 约束交互：min-width/max-width/min-height/max-height 优先级高于 aspect-ratio
- 替换元素支持：使用图片/视频的固有宽高比
- Flex/Grid 约束模式：两个维度都指定时，aspect-ratio 作为约束

**实现细节**:
- 预布局计算：在 Yoga 布局前解析 aspect-ratio 并计算尺寸
- 不修改 Yoga 库：通过预处理实现，保持依赖库纯净
- 模块：`src/layout/aspect_ratio_resolver.cpp`

**CSS 语法**:
```css
img {
  width: 300px;
  aspect-ratio: 16 / 9;  /* height 自动计算为 168.75px */
}

div {
  aspect-ratio: 1;  /* 正方形 */
  width: 200px;
  max-height: 100px;  /* min/max 约束生效，最终 height = 100px */
}
```

**测试用例**:
- `test_aspect_ratio_width_auto_height.html` - Width 固定，height auto
- `test_aspect_ratio_min_max.html` - Min/max 约束交互
- `test_aspect_ratio_flex.html` - Flex 容器中的 aspect-ratio

---

### display: contents ✅

**状态**: 已实现

**规范**: CSS Display Module Level 3, Section 2.6

**功能描述**:
- 布局透明化：元素不生成盒模型，子元素成为祖父的直接子元素
- 伪元素渲染：`::before` 和 `::after` 仍然渲染
- 事件冒泡穿透：事件正确穿透 contents 元素
- 无盒模型渲染：不渲染背景、边框、padding

**实现细节**:
- Yoga 树手术：跳过 Yoga 节点创建，子节点附加到祖父节点
- 布局缓存标记：`layout_cache[node] = nullptr` 标记为跳过
- 模块：`src/layout/display_contents.cpp`

**CSS 示例**:
```html
<div style="display: flex;">
  <div style="display: contents;">
    <div>Item 1</div>
    <div>Item 2</div>
  </div>
</div>
<!-- Item 1 和 Item 2 成为 flex 容器的直接子元素 -->
```

**测试用例**:
- `test_display_contents_layout.html` - 布局透明化
- `test_display_contents_pseudo.html` - 伪元素渲染
- `test_display_contents_events.html` - 事件处理

**已知限制**:
- 可访问性树：暂不暴露到 a11y 树（渲染器不处理 a11y）

---

### display: flow-root ✅

**状态**: 已实现（部分）

**规范**: CSS Display Module Level 3, Section 3

**功能描述**:
- 创建块级格式化上下文（BFC）
- 阻止边距折叠：父子边距不折叠
- Float 包含：暂未实现（float 未支持）

**实现细节**:
- BFC 标记：`creates_block_formatting_context` 标志
- 边距折叠检查：`src/layout/margin_collapse.cpp`
- 未来扩展：float 支持时添加 float 包含逻辑

**CSS 示例**:
```html
<div style="margin: 20px;">
  <div style="display: flow-root;">
    <div style="margin: 40px;">Child</div>
  </div>
</div>
<!-- 父子边距不折叠，总间距 = 20px + 40px = 60px -->
```

**测试用例**:
- `test_flow_root_margin_collapse.html` - 边距折叠阻止

**已知限制**:
- 仅实现边距折叠行为
- Float 包含需要 float 支持（未来实现）

---

## 实现架构

### 核心原则

**布局与渲染分离**:
- 布局阶段：计算假设没有滚动的位置
- 渲染阶段：应用滚动感知的视觉调整

**模块化设计**:
- 每个特性独立模块（margin_collapse, aspect_ratio_resolver, sticky_positioning, display_contents）
- 清晰的接口边界（"Bricks & Studs"）
- 可独立测试和重新生成

**Yoga 协同策略**:
- 在 Yoga 前/后预处理/后处理
- 不修改 Yoga 库本身
- 保持依赖库的纯净性

### 数据模型

**ComputedStyle 扩展**:
```cpp
class ComputedStyle {
    float aspect_ratio = 0.0f;  // 0 = auto, >0 = width/height ratio
    bool creates_block_formatting_context = false;
    void updateBFCFlag();
};
```

**LayoutNode 扩展**:
```cpp
struct StickyMetadata {
    RectF original_position;        // Normal flow position
    RectF containing_block_bounds;  // Parent padding box
    dom::DOMNodePtr scroll_container;
    float inset_top, inset_bottom, inset_left, inset_right;
    bool is_stuck;
    RectF visual_position;
};
```

---

## 测试策略

所有特性使用 `scripts/tools/run_baseline_compare.py` 进行浏览器对比测试，确保渲染结果与主流浏览器一致。

---

## 参考规范

- [CSS Position Module Level 3](https://drafts.csswg.org/css-position-3/)
- [CSS Box Sizing Module Level 4](https://drafts.csswg.org/css-sizing-4/)
- [CSS Display Module Level 3](https://drafts.csswg.org/css-display-3/)
