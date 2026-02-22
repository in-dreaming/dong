注意
- 完成的任务需要标注完成。
- 需要在examples/data/tests下添加测试用例。使用dong/scripts/tools/run_baseline_compare.py进行基准对比验证feature实现是否正确。


# Dong Engine - Web标准对齐 TODO

基于 gap analysis 整理的待办事项，按优先级分组。

定位校准：本引擎更偏**游戏/UI 导向的 Web-like 子集**，优先级以 ROI 为导向——先做会影响主流 UI 框架的 **layout / paint 正确性、表单可用性、滚动/定位、伪元素管线、IME**；高成本低收益/浏览器长尾特性集中放到“暂缓/冻结”。

---

## P0 - 基础功能缺失（影响基本可用性）

### 表单基础可用性

- [x] `<select>` 下拉弹出渲染 - 已实现下拉框展开/收起、选项点击、键盘导航（方向键、Enter、Escape），支持 `selectedIndex`/`value`/`options` 属性，触发 `change` 事件。测试用例：`test_select_keyboard.html`、`test_select_basic.html`

---

## P1 - 立刻推进（黄金路线 / 高 ROI）

### 布局/定位正确性（高 ROI）

- [ ] `position: sticky` 实现（需 scroll-offset 感知定位）
- [ ] `aspect-ratio` 属性
- [ ] `display: contents` 支持
- [ ] `display: flow-root` 支持（BFC 行为、常见布局模式）

### 滚动体系（高 ROI）

- [ ] `overscroll-behavior` 属性（防止滚动链）
- [ ] `scroll-behavior` 属性（可先做简化版 smooth 滚动）

### 列表体系闭环（高 ROI，建议成组推进）

- [ ] `list-style-type` / `list-style-position` / `list-style` 属性
- [ ] `::marker` 伪元素（列表标记样式）

### 伪元素管线（高 ROI）

- [ ] `::placeholder` 渲染实现（选择器匹配已有但无渲染）

### 输入法（IME/CJK）事件（高 ROI）

- [ ] `compositionstart` / `compositionupdate` / `compositionend` 事件

### 常见 UI 兼容性（仍偏高 ROI）

- [ ] `object-position` 属性（已有 `object-fit` 但缺此项）
- [ ] `appearance` 属性（`none` 去除默认控件样式）
- [ ] `border-collapse` / `border-spacing` 属性
- [ ] `table-layout` 属性（`fixed` / `auto`，如果 table 体系需要完整支持）

---

## P2 - 健康推进（中 ROI / 影响高级场景）

### CSS 属性

- [ ] CSS 逻辑属性（`margin-inline-start`, `padding-block-end`, `border-inline` 等）
- [ ] `color-scheme` 属性（`light`/`dark`）
- [ ] `hyphens` 属性
- [ ] `counter-reset` / `counter-increment` + `counter()`/`counters()` 函数
- [ ] `quotes` 属性
- [ ] `image-rendering` 属性（`pixelated`/`crisp-edges`）
- [ ] `resize` 属性（textarea 拖拽调整大小）
- [ ] `caption-side` 属性
- [ ] `will-change` 属性（建议引擎自动 layer promotion；可 P2 后段再评估）
- [ ] `accent-color` 属性（表单控件主题色；可 P2 后段）

### CSS 值/解析

- [ ] `visibility: collapse` 值
- [ ] `flex-basis: content` 关键字
- [ ] `white-space: pre-line` / `break-spaces` 文本布局器正确处理
- [ ] `text-align: justify` 实际两端对齐实现
- [ ] `background-position` 四值语法
- [ ] `font-weight` 数值 100-900 到 FreeType 选字映射验证

### CSS 简写属性

- [ ] `place-items` / `place-content` / `place-self` 简写
- [ ] `margin-inline` / `margin-block` / `padding-inline` / `padding-block` 简写
- [ ] `border-inline` / `border-block` 简写
- [ ] `inset-block` / `inset-inline` 简写
- [ ] `list-style` 简写

### CSS 伪类/伪元素

- [ ] `:open` / `:closed` 伪类（`<details>` 需要；`<dialog>` 相关暂缓见 P4）
- [ ] `:target` 伪类
- [ ] `:any-link` 伪类
- [ ] `:indeterminate` 伪类
- [ ] `:nth-last-of-type()` 伪类
- [ ] `::selection` 渲染实现

### CSS 函数

- [ ] `color-mix()` 函数（颜色混合，Baseline 2023）
- [ ] `oklch()` / `oklab()` 颜色函数
- [ ] `light-dark()` 函数
- [ ] `env()` 函数（`safe-area-inset-*`）
- [ ] `counter()` / `counters()` 函数

### CSS At规则

- [ ] `@layer` 级联层

### HTML

- [ ] `tabindex` 行为完善（焦点顺序、`-1` 可编程聚焦）
- [ ] `<details>`/`<summary>` 完整交互：点击 toggle、`open` 属性切换、disclosure triangle（可后置实现）
- [ ] HTMLDetailsElement: `open` 属性
- [ ] `lang` 属性传递给 HarfBuzz
- [ ] `dir` 属性映射到 CSS `direction` + `:dir()` 伪类
- [ ] `title` 属性 tooltip 渲染
- [ ] `pattern` 正则验证
- [ ] `min` / `max` / `step` 范围验证
- [ ] `<optgroup>` 视觉分组
- [ ] `<textarea>` `rows`/`cols` 默认尺寸映射
- [ ] `<a href="#id">` 锚点导航
- [ ] `<img loading="lazy">` 延迟加载
- [ ] `<img>` alt 文本渲染
- [ ] `name` + FormData 序列化

### JS DOM 绑定

- [ ] Element: `childElementCount`, `replaceChildren()`, `insertAdjacentElement()`/`insertAdjacentText()`
- [ ] Element: `setPointerCapture()`/`releasePointerCapture()`, `click()`
- [ ] HTMLFormElement: `elements`, `submit()`/`reset()`, `checkValidity()`
- [ ] HTMLInputElement: `select()`, `setSelectionRange()`, `selectionStart`/`selectionEnd`, `checkValidity()`/`setCustomValidity()`
- [ ] HTMLTextAreaElement: `disabled`/`readOnly`, `selectionStart`/`selectionEnd`
- [ ] Document: `elementFromPoint()`, `hasFocus()`, `scrollingElement`

### CSSOM

- [ ] `CSSStyleDeclaration.cssText`
- [ ] `CSS.supports()` JS 绑定（草案声称支持但未绑定）
- [ ] `window.matchMedia()` JS 绑定（草案声称支持但未绑定）

### 事件

- [ ] `toggle` 事件（`<details>`）
- [ ] `copy` / `cut` / `paste` 事件
- [ ] `beforeinput` 事件
- [ ] MouseEvent: `pageX`/`pageY`, `movementX`/`movementY`, `relatedTarget`
- [ ] KeyboardEvent: `repeat`
- [ ] Event: `isTrusted`

### Web API

- [ ] `structuredClone()`
- [ ] `FormData`
- [ ] `DOMParser`
- [ ] `DOMRect` 类型


### D&D

- [ ] `dragstart` / `drag` / `dragend` / `drop` 事件。 不实现完整规范。具体见文档 doc/todo_drag.md

---

## P3 - 低优先级（可长期排队）

- [ ] `inputmode` 属性
- [ ] `:default` / `:in-range` / `:out-of-range` / `:lang()` / `:dir()` 伪类
- [ ] `::first-line` / `::first-letter` 渲染实现
- [ ] `hwb()` / `attr()` / `image-set()` CSS 函数
- [ ] `crypto.randomUUID()`
- [ ] `requestIdleCallback()`
- [ ] `scrollend` 事件
- [ ] `document.visibilityState`
- [ ] `revert` CSS 关键字
- [ ] Constructable StyleSheets (`new CSSStyleSheet()`)
- [ ] `<datalist>` 元素
- [ ] `scroll-snap-type` / `scroll-snap-align` 属性

---

## P4 - 暂缓/冻结（高成本低收益，避免架构泥潭）

### 暂停

- [ ] `<dialog>` 完整行为：`showModal()`/`show()`/`close()`、top-layer、Escape 关闭
- [ ] `::backdrop` 伪元素（模态 `<dialog>` 背景遮罩）
- [ ] HTMLDialogElement: `open`, `showModal()`, `show()`, `close()`, `returnValue`
- [ ] `close` / `cancel` 事件（`<dialog>`）
- [ ] `inert` 属性（不可交互子树）
- [ ] `contain` 属性（`layout`/`paint`/`size`/`content`）
- [ ] `content-visibility` 属性

### 长期冻结

- [ ] `@container` / `container-type` 容器查询
- [ ] `writing-mode` 属性（`vertical-rl`/`vertical-lr`）
- [ ] `text-orientation` 属性
- [ ] `contenteditable` 属性
- [ ] `Selection` / `Range` API
- [ ] `navigator.clipboard`
- [ ] CSS masking (`mask`/`mask-image` 等)
- [ ] `shape-outside` 属性
