# CSS 子集参考

> Dong 走 [Ultralight 路线](../overview/positioning.md)：按**游戏 UI 需求**裁剪 CSS，不追求浏览器完整兼容。  
> 本文档列出**当前已实现或部分实现**的 CSS 能力；完整 Case 索引见 [features-index.md](./features-index.md)。

**图例**：✅ 已支持 · ⚠️ 部分支持 · ❌ 不支持

**测试路径**：下文 Case 均相对于 `dong/zig-out/bin/data/`（或 `examples/data/` 源码目录）。

---

## 1. 渲染模式与 CSS 关系

| 模式 | CSS 支持范围 |
|------|-------------|
| **DOM**（默认） | 本文档全部适用 |
| **Scene Graph** | 仅 `position: absolute` + 显式像素/`transform`；不支持 `%` 宽度、`margin`、`inline-block` 等 |
| **Direct Draw** | 无 CSS，走 `dong.drawRect` 等 JS API |

---

## 2. 布局

### 2.1 Flexbox（Yoga）

| 属性 | 状态 | 说明 |
|------|------|------|
| `display: flex` / `inline-flex` | ✅ | Yoga 核心 |
| `flex-direction` / `flex-wrap` / `flex-flow` | ✅ | |
| `justify-content` / `align-items` / `align-content` | ✅ | |
| `align-self` / `flex` / `flex-grow` / `flex-shrink` / `flex-basis` | ✅ | |
| `order` | ✅ | |
| `gap` / `row-gap` / `column-gap` | ✅ | Flex 上下文 |

### 2.2 Grid

| 属性 | 状态 | 说明 |
|------|------|------|
| `display: grid` | ✅ | 自研 Grid 引擎 |
| `grid-template-columns` / `rows` | ✅ | `fr` / `px` / `%` / `auto` |
| `grid-column` / `grid-row` | ✅ | |
| `repeat()` | ✅ | |
| `gap` | ✅ | Grid 上下文 |

Case：`tests/test_grid_basic.html`

### 2.3 定位

| 属性 | 状态 | 说明 |
|------|------|------|
| `position: static` / `relative` / `absolute` | ✅ | |
| `position: fixed` | ✅ | 视口语义，滚动时固定 |
| `position: sticky` | ✅ | 布局占位 + 渲染时视觉调整；支持嵌套与 containing block 夹紧 |
| `top` / `right` / `bottom` / `left` / `inset` | ✅ | |
| `z-index` | ⚠️ | 基础堆叠；`auto` 与 `0` 语义可能不完全一致 |

Case：`test_sticky_scroll_top.html`、`test_sticky_scroll_bottom.html`、`test_sticky_parent_clamp.html`、`test_sticky_nested.html`、`test_position_fixed.html`

### 2.4 Display 与 BFC

| 属性 | 状态 | 说明 |
|------|------|------|
| `display: block` / `inline` / `inline-block` / `none` | ✅ | |
| `display: flex` / `grid` | ✅ | 见上 |
| `display: contents` | ✅ | Yoga 树透传，子节点挂到祖父 |
| `display: flow-root` | ✅ | 阻断 margin collapse |
| `display: list-item` | ✅ | 配合 `::marker` |
| `float` / `clear` | ⚠️ | 基础左右浮动，非完整 BFC |
| `overflow` / `overflow-x` / `overflow-y` | ✅ | 裁剪 + 滚动 |
| `visibility` | ✅ | `visible` / `hidden` |

Case：`test_display_contents_layout.html`、`test_flow_root_margin_collapse.html`

### 2.5 尺寸与比例

| 属性 | 状态 | 说明 |
|------|------|------|
| `width` / `height` | ✅ | |
| `min-width` / `max-width` / `min-height` / `max-height` | ✅ | |
| `aspect-ratio` | ✅ | Yoga 前预解析；支持 min/max、replaced elements、flex 约束 |
| `box-sizing` | ✅ | |

Case：`test_aspect_ratio_width_auto_height.html`、`test_aspect_ratio_min_max.html`、`test_aspect_ratio_flex.html`

### 2.6 盒模型

| 属性 | 状态 | 说明 |
|------|------|------|
| `margin` / `margin-*` | ✅ | |
| `padding` / `padding-*` | ✅ | |
| `border` / `border-width` / `border-color` | ✅ | |
| `border-style` | ⚠️ | `solid` / `dashed` / `dotted` / `none` |
| `border-radius` | ✅ | px 与百分比 |
| `border-image` | ✅ | 9-slice GPU 渲染，四角不变形 |
| `box-shadow` | ✅ | |
| `outline` / `outline-*` | ✅ | |

Case：`test_border_radius_percent.html`、`test_nineslice_basic.html`

### 2.7 逻辑属性

| 属性 | 状态 | 说明 |
|------|------|------|
| `margin-inline` / `margin-block` 等 | ⚠️ | 部分逻辑属性已支持 |
| `padding-inline` / `padding-block` 等 | ⚠️ | |
| `inset-inline` / `inset-block` 等 | ⚠️ | |

Case：`test_logical_properties.html`、`test_logical_properties_basic.html`

---

## 3. 排版与文本

| 属性 / 特性 | 状态 | 说明 |
|-------------|------|------|
| `color` | ✅ | |
| `font-family` / `font-size` / `font-weight` / `font-style` | ✅ | 字重 100–900 |
| `line-height` / `letter-spacing` | ✅ | |
| `text-align` / `vertical-align` | ✅ | |
| `text-decoration` | ⚠️ | 基础 underline 等 |
| `text-shadow` | ✅ | |
| `text-overflow: ellipsis` | ✅ | |
| `white-space` | ⚠️ | 常用值；见各 `test_white_space_*` |
| `word-break` / `overflow-wrap` | ⚠️ | |
| `hyphens` / 软连字符 | ✅ | |
| `-webkit-line-clamp` 等 | ⚠️ | 部分多行截断场景 |

Case：`text_shadow_test.html`、`test_text_overflow_ellipsis.html`、`test_css_hyphens_soft_hyphen.html`、`test_font_weight_100_900.html`

### 3.1 列表

| 属性 | 状态 | 说明 |
|------|------|------|
| `list-style-type` | ✅ | disc / circle / square / decimal / alpha / roman 等 |
| `list-style-position` | ✅ | `outside` / `inside` |
| `list-style` | ✅ | 简写 |
| `::marker` | ✅ | 独立样式 |

Case：`test_list_markers_basic.html`、`test_list_style_types.html`、`test_marker_pseudo.html`、`test_list_nested.html`

---

## 4. 颜色

| 语法 / 属性 | 状态 | 说明 |
|-------------|------|------|
| `#rgb` / `#rrggbb` / `rgb()` / `rgba()` / `hsl()` | ✅ | |
| `oklab()` / `oklch()` | ✅ | |
| `color-mix()` | ✅ | |
| `light-dark()` | ⚠️ | 部分场景 |
| `currentColor` | ✅ | |

Case：`test_oklab_color.html`、`test_oklch_color.html`、`test_color_mix.html`、`test_light_dark_function.html`

---

## 5. 背景与渐变

| 属性 / 函数 | 状态 | 说明 |
|-------------|------|------|
| `background-color` / `background-image` | ✅ | |
| `background-position` / `background-size` / `background-repeat` | ✅ | `cover` / `contain` |
| `background-clip` / `background-origin` | ✅ | |
| `linear-gradient()` | ✅ | |
| `radial-gradient()` | ✅ | |
| `conic-gradient()` | ✅ | 亦可用作 mask 快速路径 |
| `repeating-*-gradient()` | ✅ | |
| `opacity` | ✅ | |
| `mix-blend-mode` | ⚠️ | 基础模式；mask 合成等 |

Case：`gradient_test.html`、`test_conic_from_at.html`、`test_conic_basic.html`

---

## 6. 变换、滤镜与遮罩

| 属性 | 状态 | 说明 |
|------|------|------|
| `transform` | ✅ | 2D 变换 |
| `transform-origin` | ✅ | |
| `filter` | ⚠️ | 部分滤镜 |
| `backdrop-filter` | ⚠️ | 部分场景 |
| `mask` / `mask-image` | ✅ | conic 快速路径 + 通用 multiply 合成 |
| `clip-path` | ⚠️ | 基础形状 |
| `object-fit` | ✅ | 替换元素 |
| `object-position` | ⚠️ | |

Case：`transform_test.html`、`test_mask_conic_cooldown.html`、`test_mask_general.html`

---

## 7. 层叠、选择与伪类

### 7.1 At-rules

| 规则 | 状态 | 说明 |
|------|------|------|
| `@media` | ✅ | 基础媒体查询 |
| `@layer` | ✅ | 层叠层 |
| `@supports` | ✅ | 特性查询 |
| `@keyframes` | ⚠️ | 部分动画路径；亦可用 Web Animations API |
| `@page` | ❌ | 无打印需求 |
| `@import` | ⚠️ | 外部 stylesheet 部分支持 |

Case：`test_css_layer_basic.html`、`test_css_supports.html`、`test_external_stylesheet_basic.html`

### 7.2 伪类 / 伪元素（常用）

| 选择器 | 状态 | 说明 |
|--------|------|------|
| `:hover` / `:active` / `:focus` / `:focus-visible` | ✅ | |
| `:disabled` / `:enabled` / `:checked` | ✅ | |
| `:first-child` / `:last-child` / `:nth-child()` | ✅ | |
| `::before` / `::after` | ✅ | |
| `::marker` | ✅ | 见列表 |
| `::selection` | ⚠️ | |
| `:visited` | ❌ | 故意不支持 |
| `:dir()` | ⚠️ | |

Case：`test_pseudo_classes_new.html`、`test_selection_pseudo.html`

---

## 8. 动画

| 机制 | 状态 | 说明 |
|------|------|------|
| `Element.animate()`（Web Animations API） | ✅ | JS 驱动 |
| CSS `@keyframes` + `animation` | ⚠️ | 部分属性动画 |
| `transition` | ⚠️ | 常用属性 |

---

## 9. Dong 私有 CSS 扩展

详见 [extensions.md](./extensions.md)。

| 属性 | 用途 |
|------|------|
| `nav-up` / `nav-down` / `nav-left` / `nav-right` | 手柄空间导航覆盖 |
| `--dong-hdr-boost` | HDR 亮度增益 |

Case：`test_spatial_nav_explicit.html`、`test_gamepad_full_flow.html`

---

## 10. 明确不支持（CSS 相关）

| 类别 | 不支持项 |
|------|----------|
| 打印 | `@page`、打印专用布局 |
| 多列 | `column-count` / `columns` / `column-rule` 等 |
| 历史/安全 | `:visited` |
| 表单外观 | 完整 `appearance` 重置（部分控件仍用 UA 样式） |
| 级联关键字 | `inherit` / `initial` / `unset` / `revert` — ⚠️ 部分或缺失 |
| 优先级 | `!important` — ⚠️ 可能未完全生效 |

HTML 侧不支持项（`<iframe>`、Shadow DOM 等）见 [定位文档](../overview/positioning.md) § 能力边界。

---

## 11. 验证方式

```bash
cd dong
zig build examples
zig build run-feature-tests
zig build render-all-tests

# 单页截图对比
zig build run-html-test -- data/tests/test_grid_basic.html out.bmp 800 600
```

---

## 相关文档

- [渲染模式](../guide/render-modes.md)
- [Dong 扩展](./extensions.md)
- [JavaScript API](./js-api.md)
- 内部 gap 分析（维护者）：[html_css_dom_gap_analysis.md](../../docs/developer/specific/html_css_dom_gap_analysis.md)
