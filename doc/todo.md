# Dong Engine - Web标准对齐 TODO

基于 gap analysis 整理的待办事项，按优先级分组。

注意：完成了的任务需要进行完成标注

---

## P0 - 基础功能缺失（影响基本可用性）

### CSS 级联/继承

- [x] 修复 CSS 继承检测机制：添加"属性已显式设置"标志位，替代 `inheritFromParent()` 中的默认值比较（当前 `color=="#000000"` 则继承，导致显式设黑色被覆盖）
- [x] 支持 CSS 全局关键字 `inherit` / `initial` / `unset`
- [x] 补全缺失的可继承属性：`visibility`, `text-indent`, `text-transform`, `word-break`, `overflow-wrap`, `word-spacing`

### HTML 属性行为

- [x] `[hidden]` 属性：UA 样式表加 `[hidden] { display: none !important; }`
- [x] checkbox/radio 点击切换 `checked` 属性（当前 painter 读取属性但无点击事件处理）
- [x] `<a>` 默认样式：UA 样式表加 `a { color: #0000EE; text-decoration: underline; cursor: pointer; }`
- [x] `maxlength` / `minlength`：`InputElementState::insertText` 需检查长度限制
- [x] `readonly`：阻止 readonly input 的编辑操作
- [ ] `<select>` 下拉弹出渲染（当前无下拉弹出层、无选项列表渲染）

### JS DOM 绑定 - Node 接口

- [x] 绑定 `parentNode` / `parentElement` 属性
- [x] 绑定 `childNodes` 属性（当前仅有非标准 `getChildren()`）
- [x] 绑定 `firstChild` / `lastChild` 属性
- [x] 绑定 `nodeType` / `nodeName` 属性
- [x] 绑定 `insertBefore()`（草案声称支持但实际未绑定）
- [x] 暴露 Node 类型常量（`ELEMENT_NODE=1`, `TEXT_NODE=3` 等）

### JS DOM 绑定 - Element 接口

- [x] 绑定 `element.dataset`（DOMStringMap，`data-*` 属性访问）
- [x] 绑定 `element.querySelector()` / `querySelectorAll()`（当前仅 document 上可用）
- [x] 绑定 `element.matches()` / `element.closest()`
- [x] 绑定 `element.getBoundingClientRect()`（需 DOMRect 返回类型）
- [x] 绑定 `element.remove()`（草案声称支持但实际未绑定）
- [x] 绑定 `element.dispatchEvent()`（草案声称支持但实际未绑定）
- [x] 绑定 `element.hidden` 属性（boolean，反映 hidden attribute）

### JS DOM 绑定 - HTMLElement 特定接口

- [x] `HTMLInputElement.value` / `.checked` / `.disabled` / `.type` / `.name`
- [x] `HTMLSelectElement.value` / `.selectedIndex` / `.options`
- [x] `HTMLTextAreaElement.value`

### 事件系统

- [x] 实现 `scroll` 事件（JS 无法监听滚动）
- [x] 实现 `resize` 事件
- [x] 实现 `DOMContentLoaded` 事件
- [x] 实现 `change` 事件（input blur 时、checkbox/select 立即触发）
- [x] MouseEvent dispatch 时设置 `offsetX`/`offsetY`
- [x] MouseEvent dispatch 时设置 `altKey`/`ctrlKey`/`shiftKey`/`metaKey`
- [x] KeyboardEvent dispatch 时设置 `key` 和 `code`（当前仅构造器中初始化为 ""）
- [x] KeyboardEvent dispatch 时设置修饰键
- [x] WheelEvent 绑定 `deltaX`/`deltaY`/`deltaZ`
- [x] InputEvent 绑定 `data`（插入的字符）

---

## P1 - 重要功能（影响常见 UI 模式）

### CSS 属性

- [x] `position: fixed` 正确实现（已支持，视口相对包含块）
- [ ] `position: sticky` 实现（需 scroll-offset 感知定位）
- [ ] `aspect-ratio` 属性
- [ ] `object-position` 属性（已有 `object-fit` 但缺此项）
- [ ] `list-style-type` / `list-style-position` / `list-style` 属性（ComputedStyle 和 CSS parser 均缺失）
- [ ] `display: contents` 支持
- [ ] `display: flow-root` 支持
- [ ] `will-change` 属性（GPU 合成层提示）
- [ ] `scroll-behavior` 属性（smooth 滚动过渡）
- [ ] `overscroll-behavior` 属性（防止滚动链）
- [ ] `accent-color` 属性（表单控件主题色）
- [ ] `appearance` 属性（`none` 去除默认控件样式）
- [ ] `table-layout` 属性（`fixed` / `auto`）
- [ ] `border-collapse` / `border-spacing` 属性
- [ ] `tab-size` 属性（制表符宽度）
- [x] `!important` 声明支持

### CSS 值/解析修复

- [x] `z-index: auto` vs `0` 区分（`auto` 不创建堆叠上下文）
- [x] `font-size` 关键字值（`xx-small`~`xx-large`, `smaller`/`larger`）
- [x] `border-width` 关键字值（`thin`/`medium`/`thick`）
- [x] `border-radius` 百分比值修复（当前 `50%` 解析为 `50px`）
- [x] `opacity` 百分比值支持（`opacity: 50%`）
- [x] `gap` 双值语法 + 百分比值 + `calc()` 支持（双值语法已实现）
- [x] `overflow` 双值语法（`overflow: hidden auto`）- 有单独的 overflow-x/overflow-y
- [x] `overflow: clip` 值（不同于 `hidden`，无滚动条无编程滚动）
- [x] `text-decoration` 完整简写解析（已支持 line/color/style/thickness）
- [x] `transition` / `animation` 逗号分隔多值解析（已实现）

### CSS 伪类/伪元素

- [ ] `:open` / `:closed` 伪类（`<details>` 和 `<dialog>` 必需）
- [ ] `::marker` 伪元素（列表标记样式）
- [ ] `::backdrop` 伪元素（模态 `<dialog>` 背景遮罩）
- [ ] `::placeholder` 渲染实现（选择器匹配已有但无渲染）

### CSS 函数

- [ ] `color-mix()` 函数（颜色混合，Baseline 2023）

### HTML 元素行为

- [ ] `<details>`/`<summary>` 完整交互：点击 toggle、`open` 属性切换、disclosure triangle、`:open` 伪类
- [ ] `<dialog>` 完整行为：`showModal()`/`show()`/`close()`、`::backdrop`、top-layer、Escape 关闭
- [x] `<label>` for 属性点击聚焦关联 input
- [x] `autofocus` 属性行为（页面加载后自动聚焦）
- [x] `disabled` 完整行为（阻止点击事件 + 视觉灰化）
- [ ] `<select>` 键盘导航（Arrow Up/Down, Enter）

### JS DOM 绑定补充

- [x] Node: `previousSibling` / `nextSibling`, `textContent` setter, `replaceChild()`, `cloneNode()`, `contains()`, `isConnected`
- [x] Element: `children` (as property), `firstElementChild`/`lastElementChild`, `previousElementSibling`/`nextElementSibling`
- [x] Element: `hasAttribute()`/`removeAttribute()`（草案声称支持但未绑定）
- [x] Element: `scrollTop`/`scrollLeft`, `clientWidth`/`clientHeight`, `offsetWidth`/`offsetHeight`/`offsetTop`/`offsetLeft`
- [x] Element: `before()`/`after()`/`replaceWith()`/`prepend()`/`append()`
- [x] Element: `tabIndex` 属性
- [ ] HTMLDialogElement: `open`, `showModal()`, `show()`, `close()`, `returnValue`
- [ ] HTMLDetailsElement: `open` 属性
- [x] HTMLAnchorElement: `href`, `target`
- [x] HTMLImageElement: `src`, `alt`, `naturalWidth`/`naturalHeight`, `complete`
- [x] Document: `head`, `activeElement`, `createDocumentFragment()`, `readyState`

### 事件系统补充

- [x] `submit` 事件
- [ ] `contextmenu` 事件
- [ ] `load` / `error` 事件（img 资源）
- [ ] `toggle` 事件（`<details>`）
- [ ] `close` / `cancel` 事件（`<dialog>`）
- [ ] `compositionstart` / `compositionupdate` / `compositionend` 事件（IME/CJK）
- [ ] InputEvent: `inputType` 属性
- [ ] FocusEvent: `relatedTarget` 属性
- [ ] PointerEvent: `pointerId`/`pointerType`/`pressure`/`isPrimary`
- [ ] Event 基类: `currentTarget`, `stopImmediatePropagation()`

### CSSOM

- [ ] `CSSStyleDeclaration.getPropertyValue()` / `.setProperty()` / `.removeProperty()`

### Observer API

- [ ] `IntersectionObserver` JS 绑定
- [ ] `ResizeObserver` JS 绑定（草案声称支持但未绑定）
- [ ] `MutationObserver` JS 绑定（草案声称支持但未绑定）

### Web API

- [x] `performance.now()`

---

## P2 - 完善功能（影响高级场景）

### CSS 属性

- [ ] CSS 逻辑属性（`margin-inline-start`, `padding-block-end`, `border-inline` 等）
- [ ] `contain` 属性（`layout`/`paint`/`size`/`content`）
- [ ] `content-visibility` 属性
- [ ] `color-scheme` 属性（`light`/`dark`）
- [ ] `writing-mode` 属性（`vertical-rl`/`vertical-lr`）
- [ ] `text-orientation` 属性
- [ ] `hyphens` 属性
- [ ] `counter-reset` / `counter-increment` + `counter()`/`counters()` 函数
- [ ] `quotes` 属性
- [ ] `scroll-snap-type` / `scroll-snap-align` 属性
- [ ] `image-rendering` 属性（`pixelated`/`crisp-edges`）
- [ ] `resize` 属性（textarea 拖拽调整大小）
- [ ] `caption-side` 属性

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

- [ ] `:target` 伪类
- [ ] `:any-link` 伪类
- [ ] `:indeterminate` 伪类
- [ ] `:nth-last-of-type()` 伪类
- [ ] `::selection` 渲染实现

### CSS 函数

- [ ] `oklch()` / `oklab()` 颜色函数
- [ ] `light-dark()` 函数
- [ ] `env()` 函数（`safe-area-inset-*`）
- [ ] `counter()` / `counters()` 函数

### CSS At规则

- [ ] `@container` / `container-type` 容器查询
- [ ] `@layer` 级联层

### HTML

- [ ] `tabindex` 行为完善（焦点顺序、`-1` 可编程聚焦）
- [ ] `inert` 属性（不可交互子树）
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

---

## P3 - 低优先级

- [ ] `inputmode` 属性
- [ ] `contenteditable` 属性
- [ ] `:default` / `:in-range` / `:out-of-range` / `:lang()` / `:dir()` 伪类
- [ ] `::first-line` / `::first-letter` 渲染实现
- [ ] `hwb()` / `attr()` / `image-set()` CSS 函数
- [ ] `crypto.randomUUID()`
- [ ] `navigator.clipboard`
- [ ] `Selection` / `Range` API
- [ ] `requestIdleCallback()`
- [ ] `dragstart` / `drag` / `dragend` / `drop` 事件
- [ ] `scrollend` 事件
- [ ] `document.visibilityState`
- [ ] `revert` CSS 关键字
- [ ] Constructable StyleSheets (`new CSSStyleSheet()`)
- [ ] `<datalist>` 元素
- [ ] CSS masking (`mask`/`mask-image` 等)
- [ ] `shape-outside` 属性
