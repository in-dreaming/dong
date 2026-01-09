## 游戏引擎内嵌 UI 系统规范草案
---

### 0. 设计原则

* **声明式优先**：UI 结构与样式应主要通过声明式描述完成
* **强裁剪意识**：不追求浏览器级兼容性
* **GPU 驱动**：渲染应直接映射到 GPU 管线（自研）
* **可演进**：任何模块都应允许后续“升格”为完整实现（如 video）

---

### 1. 支持的 HTML 标签
#### `<div>` 通用块级容器
- **支持属性**: `id`、`class`、`style`、`title` 等常用属性。
- **说明**: 用于布局分组和容器，可包含其它元素。
- **实现建议**: 强烈建议支持，性能开销小，是最基础的容器；无需替代方案。

#### `<span>` 通用行内容器
- **支持属性**: 与 `<div>` 相同。
- **说明**: 仅影响包裹内容，不自动换行，常用于给文本或内联元素添加样式。
- **实现建议**: 推荐支持，性能影响微小，可用于内联样式或包装。

#### `<p>` 段落
- **支持属性**: 与 `<div>` 类似，可包含 `align` 等语义属性。
- **说明**: 表示独立文本段，会在前后自动换行。
- **实现建议**: 推荐支持，可作为文本块容器；若需精简，可用加样式的 `<div>` 模拟。

#### `<h1>`–`<h6>` 标题
- **支持属性**: 与 `<div>` 类似。
- **说明**: 六级语义标题，默认字体大小依次递减。
- **实现建议**: 推荐支持，提供语义化结构；也可用带不同字号的 `<div>` 替代以减少标签种类。

#### `<ul>`/`<ol>` 列表
- **支持属性**: 与 `<div>` 类似，可附加 `type`、`start` 等传统列表属性。
- **说明**: `<ul>` 表示无序列表，`<ol>` 表示有序列表，列表项由 `<li>` 包裹。
- **实现建议**: 建议支持语义化列表；若需替代，可使用多个带样式的 `<div>` 或 Flexbox 布局。

#### `<li>` 列表项
- **支持属性**: 与 `<div>` 相同。
- **说明**: `<ul>` 或 `<ol>` 中的条目元素。
- **实现建议**: 列表实现所必需；若不支持，可用其它容器配合样式模拟。

#### `<table>`/`<tr>`/`<td>` 表格
- **支持属性**: `border`、`cellspacing`、`cellpadding`、`colspan` 等。
- **说明**: `<table>` 包裹表格，`<tr>` 表示行，`<td>` 表示单元格。
- **实现建议**: 仅在需要二维数据展示时支持；常规 UI 布局建议用 `<div>`+CSS（例如 Flexbox）。

#### `<a>` 超链接
- **支持属性**: `href`、`target`、`id`、`class`、`style` 等。
- **说明**: 用于导航或触发交互，可加载页面、切换场景或调用游戏内逻辑。
- **实现建议**: 建议支持，可在捕获点击后执行自定义逻辑；不必实现所有浏览器特性。

#### `<img>` 图像
- **支持属性**: `src`、`alt`、`width`、`height`、`title`、`loading` 等。
- **说明**: 显示静态图片，需处理资源加载，`alt` 提供占位文本。
- **实现建议**: 推荐支持；注意大图的内存与性能，可考虑合图或延迟加载。

#### `<button>` 按钮
- **支持属性**: `type`、`id`、`class`、`style`、`disabled` 等。
- **说明**: 交互式元素，触发 `click` 等事件，可单独使用或放在表单内。
- **实现建议**: 推荐支持；即便不做表单提交，也可用 `<div>`/`<a>` 模拟。

#### `<br>` 换行
- **支持属性**: 无。
- **说明**: 在文本流中插入换行。
- **实现建议**: 可简易支持，但鼓励用段落或 CSS `margin` 控制间距。

#### `<hr>` 水平线
- **支持属性**: `width`、`size`、`color` 等。
- **说明**: 分隔内容，渲染为水平分割线。
- **实现建议**: 可支持或用带背景色/边框的 `<div>` 替代以获得更多样式控制。

#### 自定义标签
- **说明**: 通过注册 `<ui-button>` 等自定义元素封装复杂逻辑。
- **实现建议**: 视引擎能力可选支持；最好映射到已有渲染元素，或使用标准元素+脚本实现。

### 2. 支持的 CSS 属性
#### 布局（Layout）
- **display**: 控制显示类型，至少支持 `block`、`inline`、`none`；有条件支持 `flex`、`grid`。
- **position**: 建议实现 `static`、`relative`、`absolute`；`fixed`、`sticky` 视需求提供。
- **top/right/bottom/left**: 与定位配合以精确放置元素。
- **width/height/min/max 系列**: 支持像素与百分比，处理自适应与限制。
- **margin/padding**: 建议全面支持，常用且开销低。
- **overflow**: 支持 `hidden`、`auto`/`scroll` 以裁剪或滚动溢出内容。
- **Flex 属性**: `flex-direction`、`flex-wrap`、`justify-content`、`align-items` 等，在实现弹性布局时提供。
- **box-sizing**: 至少支持 `content-box` 与 `border-box`。
- **z-index**: 必须支持整数叠放顺序，管理多层 UI。

#### 外观（Appearance）
- **background-color**: 支持常见颜色表示，配合背景图像使用。
- **background-image/background-size/background-repeat**: 提供填充、拉伸（`cover`/`contain`）、重复等模式。
- **color**: 文本颜色，支持标准颜色定义。
- **opacity**: 0–1 透明度，影响元素及子元素。
- **visibility**: `visible`/`hidden`，隐藏但保留占位。
- **border/border-width/border-style/border-color**: 支持实线、虚线等基本样式。
- **border-radius**: 圆角，开销小，推荐支持。
- **box-shadow**: 可选，为元素添加阴影并注意性能。
- **cursor**: 常用光标类型，增强交互反馈。
- **其他视觉属性**: `outline`、`clip-path` 等按需扩展。

#### 字体（Font）
- **font-family**: 字体系列。
- **font-size**: 字体大小。
- **font-weight**: 字重（`normal`、`bold` 或数值）。
- **font-style**: 斜体/常规。
- **line-height**: 行高。
- **text-align**: 左/右/居中/两端对齐。
- **text-decoration**: 下划线、删除线等。
- **letter-spacing/word-spacing**: 字符与单词间距。
- **text-shadow**: 可选，增强可视效果。

#### 动画（Animation）
- **transition**: `transition-property`、`transition-duration`、`transition-timing-function` 等，属性变更时平滑过渡。
- **animation**: `animation-name`、`animation-duration`、`@keyframes` 等；可先支持基本参数或用脚本替代复杂关键帧。

#### 变换（Transform）
- **transform**: 支持常见 2D 变换（`translate`、`rotate`、`scale`、`skew`）；3D 变换可选。
- **transform-origin**: 调整旋转/缩放的基点，可按需提供。

#### 滤镜（Filter）
- **filter**: `blur()`、`contrast()`、`brightness()` 等效果，可选支持且注意 GPU/性能开销。

> 其它属性（如 `animation-iteration-count`、`background-clip` 等）可按需求扩展，但应优先保证上述核心能力。

### 3. 支持的事件类型
#### Pointer 事件
- **事件**: `pointerdown`、`pointermove`、`pointerup`、`pointerenter`、`pointerleave`、`pointerover`、`pointerout`、`pointercancel`。
- **触发条件**: 指针设备（鼠标/触摸/笔）与元素交互。
- **事件对象**: `PointerEvent`，含 `pointerId`、`pointerType`、`clientX/Y`、`button/buttons`、`pressure`、`isPrimary` 等。
- **冒泡**: 支持冒泡，可使用 `event.stopPropagation()`。
- **实现建议**: 强烈建议支持，统一覆盖点击、拖拽、悬停等交互。

#### Keyboard 事件
- **事件**: `keydown`、`keyup`（`keypress` 已废弃）。
- **触发条件**: 焦点元素检测到按键按下或抬起。
- **事件对象**: `KeyboardEvent`，含 `key`、`code`、修饰键状态与 `repeat`。
- **冒泡**: 可冒泡，捕获或冒泡阶段均可处理。
- **实现建议**: 支持快捷键、文本输入；结合输入框需处理 IME。

#### Focus 事件
- **事件**: `focus`、`blur` 及其冒泡版本 `focusin`、`focusout`。
- **触发条件**: 元素获得/失去焦点。
- **事件对象**: `FocusEvent`，含 `relatedTarget`。
- **冒泡**: `focus`、`blur` 不冒泡，`focusin`、`focusout` 可冒泡。
- **实现建议**: 聚焦元素（输入框、按钮等）需支持，以便样式或校验逻辑。

#### 其它 UI 事件
- **事件**: `input`、`change`、`click` 等。
- **实现建议**: 视场景需要启用；若已支持 Pointer 事件，可用脚本综合 `click`。

### 4. 支持的 JavaScript 全局 API 与宿主绑定
#### `engine` 全局对象
- **`engine.log(message: string): void`**: 输出日志，便于调试。
- **`engine.quit(): void`**: 退出游戏或关闭应用，可加确认或权限检查。
- **`engine.triggerEvent(name: string, data: any): void`**: 向游戏逻辑发布自定义事件，推荐事件总线/发布订阅机制。

#### `ui` 全局对象（可选）
- **`ui.createElement(tag: string): HTMLElement`**: 动态创建 DOM 元素。
- **`ui.findElementById(id: string): HTMLElement | null`**: 根据 ID 查找元素。
- **`ui.showModal(id: string): void`**: 显示模态窗口或界面元素，可扩展标题与按钮回调。

#### `timer` 全局对象（类似 `setTimeout`/`setInterval`）
- **`timer.setTimeout(callback, delay): number`**: 延迟执行一次回调，返回定时器 ID；可基于 UI 线程或游戏主循环。
- **`timer.clearTimeout(id): void`**: 取消对应延时调用。
- **`timer.setInterval(callback, interval): number`**: 周期性执行回调，返回 ID。
- **`timer.clearInterval(id): void`**: 取消周期性调用。

#### 其它全局对象
- **`document` / DOM API**: 若采用原生 HTML 环境可直接支持。
- **`console`**: 可映射到引擎日志系统。
- **扩展性**: 建议文档化可用接口并允许自定义扩展。

### 5. 可选模块
#### `<video>` 视频模块
- **用途**: 播放过场、教程等视频。
- **建议**: 实现成本高（解码与渲染管线），若需求有限可改用帧动画或引擎现有视频能力。

#### `<canvas>` 画布模块
- **用途**: 提供可编程 2D 绘图上下文。
- **建议**: 需提供 `CanvasRenderingContext2D` 等 API 并处理 GPU 上传，复杂度高；可用预渲染纹理或精灵动画替代。

#### 表单模块（`<input>`、`<textarea>`、`<select>` 等）
- **用途**: 登录、聊天、设置等输入场景。
- **建议**: 中等复杂度，需要处理文本输入、光标、选区和 IME；也可开发专用输入组件。

#### `<svg>` 矢量模块
- **用途**: 渲染可缩放矢量图形。
- **建议**: 解析与渲染复杂且性能成本高，通常不推荐；可将矢量图转为纹理或使用引擎自身矢量库。

#### `<audio>` 音频模块
- **用途**: 播放音效或背景音。
- **建议**: 可复用引擎音频系统；若已有 HTML 环境，可支持 `<audio>` 并处理格式兼容。

> 本草案旨在为内嵌 UI 系统的设计与实现提供基础指引，具体取舍应结合项目需求与性能目标。

### 参考资料
1. [display - CSS | MDN](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/display)
2. [CSS The position Property](https://www.w3schools.com/css/css_positioning.asp)
3. [background-color - CSS | MDN](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/background-color)
4. [filter - CSS | MDN](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/filter)
5. [Element: pointerdown event - Web APIs | MDN](https://developer.mozilla.org/en-US/docs/Web/API/Element/pointerdown_event)
6. [Element: focus event - Web APIs | MDN](https://developer.mozilla.org/en-US/docs/Web/API/Element/focus_event)
