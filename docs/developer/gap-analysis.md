# Dong Engine - Web标准差距分析报告

基于 WHATWG HTML Living Standard、MDN CSS Reference、DOM Standard 对 Dong 引擎现有实现进行源码级对照分析。本报告不包含 CSS Grid、多列布局等已明确排除的特性。

---

> **注意**：部分条目可能已过时。请以 [css-subset.md](../reference/css-subset.md) 与 [features-index.md](../reference/features-index.md) 为准。

## 一、历史草案与实现差异

以下条目来自旧版支持清单，与当前实现可能不一致（需逐项核对）：

| 特性 | 草案状态 | 实际状态 |
|------|---------|---------|
| `<details>` / `<summary>` 交互行为 | 支持 | 无点击展开/折叠、无 `open` 属性切换、无 disclosure triangle |
| `<dialog>` 行为 | 支持 | 无 `showModal()`/`show()`/`close()`、无 `::backdrop`、无 top-layer |
| `<select>` 下拉渲染 | 支持 | 无下拉弹出层、无选项列表渲染、无键盘导航 |
| checkbox/radio 点击切换 | 支持 | `painter.cpp` 读取 `checked` 属性但无点击事件切换该属性 |
| `Node.insertBefore()` | 支持 | JS 绑定中未暴露，仅 `appendChild`/`removeChild` 绑定 |
| `Node.cloneNode()` | 支持 | JS 绑定中未暴露 |
| `Node.contains()` | 支持 | JS 绑定中未暴露 |
| `Element.querySelector()` (元素级) | 支持 | 仅 `document` 上可用，元素级未绑定 |
| `Element.matches()` / `closest()` | 支持 | JS 绑定中未暴露 |
| `Element.getBoundingClientRect()` | 支持 | JS 绑定中未暴露 |
| `Element.remove()` | 支持 | JS 绑定中未暴露 |
| `Element.dispatchEvent()` | 支持 | JS 绑定中未暴露 |
| `ResizeObserver` | 支持 | JS 绑定中未暴露 |
| `MutationObserver` | 支持 | JS 绑定中未暴露 |
| DOM 遍历属性 (`parentNode`, `childNodes`, `firstChild` 等) | 支持 | 大部分 JS 绑定中未暴露 |

---

## 二、HTML 差距

### 2.1 全局属性缺失 (按优先级排列)

| 属性 | 优先级 | 现状 | 标准行为 |
|------|--------|------|---------|
| `hidden` | **P0** | 属性可解析但无行为效果 | 应映射到 `display: none !important`。修复：UA 样式表加 `[hidden] { display: none !important; }` |
| `autofocus` | **P1** | 无实现 | 页面加载后自动聚焦到带此属性的元素 |
| `tabindex` | **P1** | 属性可存储 | 控制焦点顺序，`-1` 表示可编程聚焦但不可 Tab 到达，`0` 表示按 DOM 顺序，正值指定顺序 |
| `data-*` / `dataset` | **P1** | 属性可存储但无 JS `dataset` API | 应暴露 `HTMLElement.dataset` DOMStringMap |
| `inert` | **P2** | 无实现 | 标记元素及其子树不可交互、不可聚焦 |
| `lang` | **P2** | 无实现 | 应传递给 HarfBuzz 做语言感知的文本塑形 |
| `dir` | **P2** | CSS `direction` 存在但 HTML `dir` 属性未映射 | 应映射到 `direction` CSS 属性，支持 `:dir()` 伪类 |
| `title` | **P2** | 属性可存储但不渲染 tooltip | 常用于游戏 UI 的提示信息 |
| `inputmode` | **P3** | 无实现 | 移动端虚拟键盘类型提示 |
| `contenteditable` | **P3** | 无实现 | 富文本编辑 |

### 2.2 表单元素属性缺失

#### Input 元素

| 属性/行为 | 优先级 | 现状 |
|----------|--------|------|
| `maxlength` / `minlength` | **P0** | `InputElementState::insertText` 不检查长度限制 |
| `readonly` | **P0** | `:read-only` 伪类匹配但不阻止编辑 |
| `disabled` 行为完善 | **P1** | FocusManager 阻止聚焦，但点击事件未被阻止，无视觉禁用效果 |
| `required` 验证行为 | **P1** | `:required` 伪类可匹配但无实际验证 |
| `pattern` | **P2** | 无正则验证 |
| `min` / `max` / `step` | **P2** | 数字/日期类型无范围验证 |
| `checked` 动态切换 | **P0** | checkbox/radio 点击不切换 `checked` 状态 |
| `name` + 表单数据构建 | **P2** | 仅存储，无 FormData 序列化 |
| `list` (datalist 关联) | **P3** | `<datalist>` 不支持 |

#### Select / Textarea

| 缺失 | 优先级 |
|------|--------|
| `<select>` 下拉弹出渲染 | **P0** |
| `<select>` 键盘导航 (Arrow Up/Down, Enter) | **P1** |
| `<optgroup>` 视觉分组 | **P2** |
| `<textarea>` `rows`/`cols` 默认尺寸映射 | **P2** |

### 2.3 元素行为缺失

| 行为 | 优先级 | 说明 |
|------|--------|------|
| `<details>`/`<summary>` 折叠切换 | **P0** | 点击 summary 切换 open 属性、隐藏/显示内容、渲染 disclosure triangle |
| `<dialog>` 模态 | **P0** | `showModal()`/`close()`、`::backdrop`、顶层渲染、Escape 关闭 |
| `<a>` 默认样式 | **P0** | 缺少 `color: blue; text-decoration: underline; cursor: pointer` |
| `<label>` for 点击聚焦 | **P1** | 点击 label 应聚焦关联 input |
| `<a href="#id">` 锚点导航 | **P2** | 页内跳转 |
| `<img loading="lazy">` | **P2** | 延迟加载 |
| `<img>` alt 文本渲染 | **P2** | 加载失败时显示 alt 文字 |

### 2.4 UA 样式表缺失条目

| 元素 | 缺失的默认样式 |
|------|--------------|
| `<details>` | `display: block`，非 open 时隐藏 summary 以外的子元素 |
| `<summary>` | `display: list-item`，disclosure triangle marker |
| `<dialog>` | `display: none`（未打开时）、打开时居中定位 |
| `<progress>` | 进度条可视化渲染 |
| `<meter>` | 度量条可视化渲染 |
| `<legend>` | `<fieldset>` 内的边框嵌入定位 |
| `[hidden]` | `display: none !important` |

---

## 三、CSS 差距

### 3.1 缺失的 CSS 属性

#### P0 - 必须支持

| 属性 | 用途 | 说明 |
|------|------|------|
| `position: fixed` | 视口固定定位（头部导航、模态、toast） | 代码引用了 `"fixed"` 但可能退化为 `absolute`，缺少视口相对包含块逻辑 |
| `position: sticky` | 滚动吸顶（列表分组头、导航栏） | 完全缺失，需 scroll-offset 感知定位 |
| `aspect-ratio` | 响应式图片/视频容器宽高比 | 完全缺失，Baseline 2021 |
| `list-style-type` / `list-style-position` / `list-style` | 列表项标记（圆点、数字） | ComputedStyle 和 CSS parser 均缺失 |
| `object-position` | 替换内容（img）在容器内的对齐 | 有 `object-fit` 但缺 `object-position` |

#### P1 - 高优先级

| 属性 | 用途 |
|------|------|
| `display: contents` | 布局透传（组件包装器模式） |
| `display: flow-root` | 新建 BFC（替代 clearfix） |
| `will-change` | GPU 合成层提示 |
| `resize` | textarea 可拖拽调整大小 |
| `scroll-behavior` | `smooth` 滚动过渡 |
| `overscroll-behavior` | 防止滚动链（模态对话框内滚动不冒泡） |
| `accent-color` | 表单控件主题色 |
| `appearance` | `none` 去除默认控件样式 |
| `table-layout` | `fixed` / `auto` 表格布局算法 |
| `border-collapse` / `border-spacing` | 表格边框渲染 |
| `caption-side` | 表格标题定位 |
| `tab-size` | 制表符宽度（`<pre>` 和代码块） |

#### P2 - 中优先级

| 属性 | 用途 |
|------|------|
| `contain` | `layout`/`paint`/`size`/`content` 渲染隔离 |
| `content-visibility` | `auto`/`hidden` 跳过离屏子树渲染 |
| `color-scheme` | `light`/`dark` 系统主题适配 |
| `writing-mode` | `vertical-rl`/`vertical-lr` 竖排文字 |
| `text-orientation` | 竖排中的字形方向控制 |
| `hyphens` | 自动断字 |
| `counter-reset` / `counter-increment` / `counter()` | CSS 计数器自动编号 |
| `quotes` | `<q>` 引号定义，`content: open-quote` |
| `scroll-snap-type` / `scroll-snap-align` | 滚动吸附（轮播、分页） |
| `image-rendering` | `pixelated`/`crisp-edges`（像素风游戏） |
| `@container` / `container-type` | 容器查询（组件级响应式） |
| `@layer` | 级联层控制 |

### 3.2 已支持属性的缺失值/关键字

| 属性 | 缺失值 | 影响 |
|------|--------|------|
| `z-index` | `auto` vs `0` 区分 | `auto` 不创建堆叠上下文，`0` 创建。当前默认 `int 0` |
| `overflow` | `clip`（不可滚动的 hidden）、双值语法 `overflow: hidden auto` | `auto` vs `scroll` 滚动条策略 |
| `visibility` | `collapse`（表格行/列） | 当前仅 `visible`/`hidden` |
| `font-size` | 关键字值 `xx-small`~`xx-large`、`smaller`/`larger` | `parseFloatHelper` 解析失败 |
| `font-weight` | 数值 `100`-`900` 映射 | 存为字符串但需验证 FreeType 选字是否正确 |
| `border-width` | 关键字 `thin`/`medium`/`thick` | `parseFloatHelper` 解析失败 |
| `border-radius` | 百分比值 `50%` | 当前 `50%` 被解析为 `50px` |
| `opacity` | 百分比值 `opacity: 50%` | `parseFloatHelper` 不处理 `%` |
| `gap` | 百分比值、`calc()` 表达式、双值语法 | `parseFloatHelper` 仅返回 px |
| `white-space` | `pre-line`、`break-spaces` | 存储了但文本布局器是否正确处理？ |
| `text-align` | `justify`（两端对齐） | 内联布局是否实现了单词间距分布？ |
| `flex-basis` | `content` 关键字 | `parseValue` 不处理 |
| `background-position` | 四值语法 `right 10px bottom 20px` | 需验证解析器 |

### 3.3 缺失的 CSS 简写属性

| 简写 | 展开为 | 状态 |
|------|--------|------|
| `place-items` | `align-items` + `justify-items` | 缺失 |
| `place-content` | `align-content` + `justify-content` | 缺失 |
| `place-self` | `align-self` + `justify-self` | 缺失 |
| `gap` 双值 | `row-gap column-gap` | 解析器仅读取一个值 |
| `overflow` 双值 | `overflow-x overflow-y` | 解析器设为同一个值 |
| `margin-inline` / `margin-block` | 逻辑方向 margin | 缺失 |
| `padding-inline` / `padding-block` | 逻辑方向 padding | 缺失 |
| `border-inline` / `border-block` | 逻辑方向 border | 缺失 |
| `inset-block` / `inset-inline` | 逻辑方向 offset | 缺失 |
| `list-style` | `list-style-type position image` | 缺失 |
| `text-decoration` 完整简写 | `line style color thickness` | 解析器将整个值存为一个字符串未分解 |
| `transition` 多值 | 逗号分隔的多个过渡 | 解析不正确 |
| `animation` 多值 | 逗号分隔的多个动画 | 同上 |

### 3.4 缺失的伪类/伪元素

| 伪类 | 优先级 | 说明 |
|------|--------|------|
| `:open` / `:closed` | **P0** | `<details>` 和 `<dialog>` 必需 |
| `:target` | **P2** | 匹配 URL hash 指向的元素 |
| `:any-link` | **P2** | 匹配带 href 的 `<a>` |
| `:indeterminate` | **P2** | checkbox 不确定状态、无 value 的 progress |
| `:default` | **P3** | 默认提交按钮/选中选项 |
| `:in-range` / `:out-of-range` | **P3** | 数值输入范围验证 |
| `:lang()` | **P3** | 语言匹配 |
| `:dir()` | **P3** | 文字方向匹配 |
| `:nth-last-of-type()` | **P2** | 仅有 `:nth-of-type()` |

| 伪元素 | 优先级 | 说明 |
|--------|--------|------|
| `::marker` | **P1** | 列表标记样式 |
| `::backdrop` | **P0** | 模态 `<dialog>` 背景遮罩 |
| `::placeholder` 渲染 | **P1** | 选择器匹配但无渲染实现 |
| `::selection` 渲染 | **P2** | 选择器匹配但无渲染实现 |
| `::first-line` / `::first-letter` | **P3** | 选择器匹配但无渲染实现 |

### 3.5 缺失的 CSS 函数

| 函数 | 优先级 | 说明 |
|------|--------|------|
| `color-mix()` | **P1** | 颜色混合，Baseline 2023 |
| `oklch()` / `oklab()` | **P2** | 感知均匀色彩空间 |
| `hwb()` | **P3** | 色相-白度-黑度颜色 |
| `light-dark()` | **P2** | 随 `color-scheme` 切换颜色 |
| `env()` | **P2** | `safe-area-inset-*` 刘海屏适配 |
| `attr()` | **P3** | 属性值引用（用于 `content`） |
| `counter()` / `counters()` | **P2** | CSS 计数器（自动编号） |
| `image-set()` | **P3** | 分辨率切换背景图 |

### 3.6 级联/继承系统缺陷

#### 继承检测机制有误 (P0)

`StyleEngine::inheritFromParent()` 使用默认值比较来检测"未设置"：

```cpp
if (computed.color == "#000000") computed.color = parent_style.color;
if (computed.font_family == "Arial") computed.font_family = parent_style.font_family;
```

**问题**：如果作者显式写 `color: #000000`，引擎会错误地用父级颜色覆盖它。根因是缺少"属性是否被显式设置"的跟踪标志。

#### 缺失全局 CSS 关键字 (P0)

| 关键字 | 行为 |
|--------|------|
| `inherit` | 强制从父级继承 |
| `initial` | 重置为规范定义的初始值 |
| `unset` | 继承属性作为 `inherit`，非继承属性作为 `initial` |
| `revert` | 回退到 UA 样式表值 |

#### `!important` 支持 (P1)

解析器可能不处理 `!important` 声明，影响级联优先级。

#### 缺失的可继承属性

以下属性应默认继承但未在 `inheritFromParent()` 中处理：
- `visibility`
- `text-indent`
- `text-transform`
- `word-break`
- `overflow-wrap`
- `word-spacing`
- `list-style-type`（实现后）
- `tab-size`（实现后）

#### CSS 逻辑属性 (P2)

完全缺失 CSS 逻辑属性支持（`margin-inline-start`, `padding-block-end` 等）。现代 CSS 框架广泛使用，RTL 语言支持必需。

---

## 四、DOM API 差距

### 4.1 Node 接口 - 缺失的 JS 绑定

| 属性/方法 | 优先级 | 说明 |
|----------|--------|------|
| `parentNode` / `parentElement` | **P0** | DOM 遍历基础，未绑定为 JS 属性 |
| `childNodes` | **P0** | 未绑定，仅有非标准 `getChildren()` |
| `firstChild` / `lastChild` | **P0** | 未绑定 |
| `previousSibling` / `nextSibling` | **P1** | 未绑定 |
| `nodeType` / `nodeName` | **P0** | 区分元素/文本节点的基础 |
| `textContent` (setter) | **P1** | 设置文本内容 |
| `insertBefore()` | **P0** | 声称支持但未绑定 |
| `replaceChild()` | **P1** | 声称支持但未绑定 |
| `cloneNode()` | **P1** | 声称支持但未绑定 |
| `contains()` | **P1** | 声称支持但未绑定 |
| `isConnected` | **P1** | 框架常用 |
| Node 类型常量 (`ELEMENT_NODE=1`, `TEXT_NODE=3`) | **P1** | `nodeType` 的配套 |

### 4.2 Element 接口 - 缺失的 JS 绑定

| 属性/方法 | 优先级 | 说明 |
|----------|--------|------|
| `dataset` | **P0** | `data-*` 属性访问，极其常用 |
| `querySelector()` (元素级) | **P0** | 仅 document 上可用 |
| `querySelectorAll()` (元素级) | **P0** | 同上 |
| `matches()` | **P0** | 事件委托核心方法 |
| `closest()` | **P0** | 事件委托核心方法 (`event.target.closest('.item')`) |
| `getBoundingClientRect()` | **P0** | 定位计算基础 |
| `remove()` | **P0** | 现代 DOM 移除方法 |
| `hidden` (property) | **P0** | 简易显示/隐藏切换 |
| `children` (as property) | **P1** | 当前为非标准 `getChildren()` 方法 |
| `firstElementChild` / `lastElementChild` | **P1** | 未绑定 |
| `previousElementSibling` / `nextElementSibling` | **P1** | 未绑定 |
| `childElementCount` | **P2** | 未绑定 |
| `hasAttribute()` / `removeAttribute()` | **P1** | 声称支持但未绑定 |
| `scrollTop` / `scrollLeft` | **P1** | 滚动位置管理 |
| `clientWidth` / `clientHeight` | **P1** | 布局计算必需 |
| `offsetWidth` / `offsetHeight` / `offsetTop` / `offsetLeft` | **P1** | 定位计算 |
| `before()` / `after()` / `replaceWith()` | **P1** | 现代 DOM 操作 |
| `prepend()` / `append()` | **P1** | 现代 DOM 操作 |
| `replaceChildren()` | **P2** | 高效替换所有子节点 |
| `insertAdjacentElement()` / `insertAdjacentText()` | **P2** | 未绑定 |
| `dispatchEvent()` | **P0** | 自定义事件通信基础 |
| `tabIndex` | **P1** | 焦点顺序控制 |
| `setPointerCapture()` / `releasePointerCapture()` | **P2** | 拖拽交互 |
| `click()` | **P2** | 编程式点击触发 |

### 4.3 HTMLElement 特定接口 - 缺失

#### HTMLInputElement (P0)

| 属性/方法 | 说明 |
|----------|------|
| `value` | 当前值（最关键的表单属性） |
| `checked` | checkbox/radio 状态 |
| `disabled` | 禁用状态 |
| `type` | 输入类型 |
| `name` | 表单提交名 |
| `placeholder` | 提示文本 |
| `required` / `readOnly` | 验证标志 |
| `select()` | 选中全部文本 |
| `setSelectionRange()` | 设置选区范围 |
| `selectionStart` / `selectionEnd` | 选区位置 |
| `checkValidity()` / `setCustomValidity()` | 验证方法 |

#### HTMLSelectElement (P0)

| 属性/方法 | 说明 |
|----------|------|
| `value` | 当前选中值 |
| `selectedIndex` | 选中项索引 |
| `options` | HTMLOptionsCollection |
| `disabled` | 禁用状态 |

#### HTMLTextAreaElement (P1)

| 属性/方法 | 说明 |
|----------|------|
| `value` | 当前文本内容 |
| `disabled` / `readOnly` | 状态标志 |
| `selectionStart` / `selectionEnd` | 选区 |

#### HTMLDialogElement (P0)

| 属性/方法 | 说明 |
|----------|------|
| `open` | 是否打开 |
| `showModal()` | 模态打开（带 backdrop） |
| `show()` | 非模态打开 |
| `close()` | 关闭 |
| `returnValue` | 关闭时的返回值 |

#### HTMLDetailsElement (P1)

| 属性 | 说明 |
|------|------|
| `open` | 是否展开 |

#### HTMLAnchorElement (P1)

| 属性 | 说明 |
|------|------|
| `href` | 链接 URL |
| `target` | 目标上下文 |

#### HTMLImageElement (P1)

| 属性 | 说明 |
|------|------|
| `src` | 图片源 |
| `alt` | 替代文本 |
| `naturalWidth` / `naturalHeight` | 原始尺寸 |
| `complete` | 是否加载完成 |

#### HTMLFormElement (P2)

| 属性/方法 | 说明 |
|----------|------|
| `elements` | 表单控件集合 |
| `submit()` / `reset()` | 提交/重置 |
| `checkValidity()` | 验证 |

### 4.4 Document 接口 - 缺失的 JS 绑定

| 属性/方法 | 优先级 | 说明 |
|----------|--------|------|
| `document.head` | **P1** | 声称支持但未绑定 |
| `document.activeElement` | **P1** | 焦点管理必需 |
| `document.createDocumentFragment()` | **P1** | 批量 DOM 操作 |
| `document.readyState` | **P1** | 加载状态（loading/interactive/complete） |
| `document.elementFromPoint()` | **P2** | 命中测试 |
| `document.hasFocus()` | **P2** | 焦点查询 |
| `document.scrollingElement` | **P2** | 文档滚动元素 |
| `document.visibilityState` | **P3** | 标签页可见性 |

### 4.5 事件系统差距

#### 缺失的事件类型

| 事件 | 优先级 | 说明 |
|------|--------|------|
| `scroll` | **P0** | 滚动事件，JS 无法监听。对 infinite scroll、sticky header、parallax 关键 |
| `resize` | **P0** | 窗口/元素尺寸变化 |
| `DOMContentLoaded` | **P0** | DOM 解析完成生命周期事件 |
| `change` | **P0** | 表单值提交（input blur 时、checkbox/select 立即触发） |
| `submit` | **P1** | 表单提交 |
| `contextmenu` | **P1** | 右键菜单（拦截后显示自定义菜单） |
| `load` / `error` (img) | **P1** | 资源加载成功/失败 |
| `toggle` | **P1** | `<details>` 展开/折叠 |
| `close` / `cancel` | **P1** | `<dialog>` 关闭/取消 |
| `compositionstart` / `compositionupdate` / `compositionend` | **P1** | IME 输入（CJK 必需） |
| `copy` / `cut` / `paste` | **P2** | 剪贴板事件 |
| `beforeinput` | **P2** | 可取消的输入前事件 |
| `dragstart` / `drag` / `dragend` / `drop` | **P3** | 拖拽 |
| `scrollend` | **P3** | 滚动完成 |

#### 事件属性缺失

| 事件类型 | 缺失属性 | 优先级 |
|---------|---------|--------|
| MouseEvent | `offsetX`/`offsetY`（相对目标元素） | **P0** |
| MouseEvent | `pageX`/`pageY`（相对文档） | **P1** |
| MouseEvent | `altKey`/`ctrlKey`/`shiftKey`/`metaKey` | **P0** |
| MouseEvent | `movementX`/`movementY` | **P2** |
| MouseEvent | `relatedTarget` | **P2** |
| KeyboardEvent | `key`（dispatch 时未设置，仅构造器中初始化为 ""） | **P0** |
| KeyboardEvent | `code`（dispatch 时未设置） | **P0** |
| KeyboardEvent | 修饰键（dispatch 时未设置） | **P0** |
| KeyboardEvent | `repeat` | **P2** |
| InputEvent | `data`（插入的字符） | **P0** |
| InputEvent | `inputType`（变更类型） | **P1** |
| FocusEvent | `relatedTarget` | **P1** |
| WheelEvent | `deltaX`/`deltaY`/`deltaZ` | **P0** |
| PointerEvent | `pointerId`/`pointerType`/`pressure`/`isPrimary` | **P1** |
| Event 基类 | `currentTarget` | **P1** |
| Event 基类 | `stopImmediatePropagation()` | **P1** |
| Event 基类 | `isTrusted` | **P2** |

### 4.6 CSSOM 差距

| 接口/方法 | 优先级 | 说明 |
|----------|--------|------|
| `CSSStyleDeclaration.getPropertyValue()` | **P1** | 标准获取样式值方法 |
| `CSSStyleDeclaration.setProperty()` | **P1** | 标准设置样式方法（支持 priority） |
| `CSSStyleDeclaration.removeProperty()` | **P1** | 移除单个内联样式 |
| `CSSStyleDeclaration.cssText` | **P2** | 获取/设置全部内联样式字符串 |
| `CSS.supports()` | **P2** | 声称支持但未绑定 |
| `window.matchMedia()` | **P2** | 声称支持但未绑定 |
| Constructable StyleSheets (`new CSSStyleSheet()`) | **P3** | 动态样式注入 |

### 4.7 Observer API 差距

| API | 优先级 | 说明 |
|----|--------|------|
| `IntersectionObserver` | **P1** | 元素可见性检测（延迟加载、无限滚动） |
| `ResizeObserver` | **P1** | 声称支持但 JS 绑定缺失 |
| `MutationObserver` | **P1** | 声称支持但 JS 绑定缺失 |

### 4.8 其他 Web API 差距

| API | 优先级 | 说明 |
|----|--------|------|
| `performance.now()` | **P1** | 高精度时间戳，动画计时和性能测量 |
| `structuredClone()` | **P2** | 深拷贝 JS 对象 |
| `FormData` | **P2** | 表单数据收集和操作 |
| `DOMParser` | **P2** | HTML/XML 字符串解析为 DOM |
| `DOMRect` | **P2** | `getBoundingClientRect()` 的返回类型 |
| `crypto.randomUUID()` | **P3** | 生成唯一 ID |
| `navigator.clipboard` | **P3** | 剪贴板 API |
| `Selection` / `Range` | **P3** | 文本选区 |
| `requestIdleCallback()` | **P3** | 空闲回调 |

---

## 五、综合优先级排序 (Top 30 Actions)

### P0 - 基础功能缺失（影响基本可用性）

1. **修复 CSS 继承检测**：添加"属性已显式设置"跟踪，替代默认值比较
2. **`hidden` 属性行为**：UA 样式表加 `[hidden] { display: none !important; }`
3. **checkbox/radio 点击切换 `checked`**
4. **`<a>` 默认样式**（blue, underline, cursor:pointer）
5. **JS 绑定: DOM 遍历** (`parentNode`, `childNodes`, `firstChild`, `nodeType` 等)
6. **JS 绑定: `querySelector()`/`querySelectorAll()` 元素级**
7. **JS 绑定: `getBoundingClientRect()`**
8. **JS 绑定: `element.dataset`**
9. **JS 绑定: `matches()`/`closest()`**
10. **JS 绑定: `remove()`/`dispatchEvent()`**
11. **JS 绑定: `HTMLInputElement.value`/`.checked`**
12. **事件属性: MouseEvent modifier keys, KeyboardEvent `key`/`code`**
13. **事件类型: `scroll`, `resize`, `DOMContentLoaded`**
14. **WheelEvent JS 属性: `deltaX`/`deltaY`**

### P1 - 重要功能（影响常见 UI 模式）

15. **`position: fixed`** 正确实现
16. **`position: sticky`** 实现
17. **`aspect-ratio`** CSS 属性
18. **`<details>`/`<summary>` 完整行为**（toggle, `:open`, disclosure triangle）
19. **`<dialog>` 完整行为**（`showModal()`, `::backdrop`, Escape 关闭）
20. **`<select>` 下拉渲染**
21. **CSS 全局关键字**：`inherit` / `initial` / `unset`
22. **`!important` 支持**
23. **`list-style` 相关属性 + `::marker`**
24. **IME 事件**：`compositionstart`/`update`/`end`
25. **JS 绑定: `insertBefore()`/`cloneNode()`/`before()`/`after()`/`append()`/`prepend()`**
26. **JS 绑定: `HTMLDialogElement.showModal()`/`close()`**
27. **JS 绑定: `IntersectionObserver`, `ResizeObserver`, `MutationObserver`**
28. **`performance.now()`**

### P2 - 完善功能（影响高级场景）

29. **CSS 逻辑属性**（`margin-inline`, `padding-block` 等）
30. **`color-mix()` 函数**
31. **`display: contents` / `flow-root`**
32. **`scroll-behavior: smooth` / `overscroll-behavior`**
33. **`accent-color` / `appearance: none`**
34. **`table-layout` / `border-collapse`**
35. **`label for` 点击聚焦**
36. **`autofocus` 属性行为**
37. **`disabled` 完整行为**（阻止点击事件 + 视觉灰化）
38. **`maxlength`/`readonly` 输入约束**
39. **`font-size` 关键字、`border-width` 关键字**
40. **PointerEvent 属性绑定**

---

## 六、参考规范

- [WHATWG HTML Living Standard](https://html.spec.whatwg.org/multipage/)
- [MDN CSS Reference](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference)
- [WHATWG DOM Standard](https://dom.spec.whatwg.org/)
- [MDN Web API Reference](https://developer.mozilla.org/en-US/docs/Web/API)
