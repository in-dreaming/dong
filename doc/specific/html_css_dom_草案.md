# 自研UI框架 Web标准支持清单

## 1. 概述

本文档基于WHATWG HTML Living Standard、MDN CSS Reference及DOM Standard，梳理自研UI框架对Web标准的支持范围[1][2][3]。框架采用lexbor进行HTML/CSS解析，Yoga处理Flexbox布局，SDL GPU负责渲染，FreeType/HarfBuzz/msdfgen处理文字，QuickJS提供脚本能力。

### 1.1 技术栈与支持原则

|模块|技术选型|影响范围|
|---|---|---|
|HTML/CSS解析|lexbor|元素解析、CSS选择器|
|布局引擎|Yoga+自研|Flexbox、Block/Inline|
|图形渲染|SDL GPU|视觉呈现、变换、滤镜|
|文字系统|FreeType+HarfBuzz+msdfgen|字体加载、文本塑形、字形渲染|
|脚本引擎|QuickJS|DOM操作、事件处理|

---

## 2. HTML元素支持清单

### 2.1 文档元数据元素

|元素|状态|说明|
|---|---|---|
|`<html>`|✅支持|lexbor已支持|
|`<head>`|✅支持|lexbor已支持|
|`<title>`|✅支持|lexbor已支持，用于窗口标题|
|`<base>`|✅支持|lexbor已支持|
|`<link>`|⚠️部分支持|仅支持stylesheet引用|
|`<meta>`|⚠️部分支持|仅解析charset、viewport|
|`<style>`|✅支持|lexbor已支持|

### 2.2 内容分区元素

|元素|状态|说明|
|---|---|---|
|`<body>`|✅支持|lexbor已支持|
|`<article>`|✅支持|作为通用容器|
|`<section>`|✅支持|作为通用容器|
|`<nav>`|✅支持|作为通用容器|
|`<aside>`|✅支持|作为通用容器|
|`<h1>`-`<h6>`|✅支持|lexbor已支持|
|`<header>`|✅支持|作为通用容器|
|`<footer>`|✅支持|作为通用容器|
|`<main>`|✅支持|作为通用容器|
|`<address>`|✅支持|作为通用容器|

### 2.3 文本内容元素

|元素|状态|说明|
|---|---|---|
|`<div>`|✅支持|核心容器元素|
|`<p>`|✅支持|lexbor已支持|
|`<hr>`|✅支持|渲染为水平线|
|`<pre>`|✅支持|保留空白符|
|`<blockquote>`|✅支持|作为块级容器|
|`<ol>`|✅支持|有序列表|
|`<ul>`|✅支持|无序列表|
|`<li>`|✅支持|列表项|
|`<dl>`|✅支持|定义列表|
|`<dt>`|✅支持|定义术语|
|`<dd>`|✅支持|定义描述|
|`<figure>`|✅支持|作为通用容器|
|`<figcaption>`|✅支持|作为通用容器|

### 2.4 内联文本语义元素

|元素|状态|说明|
|---|---|---|
|`<a>`|⚠️部分支持|支持href、target属性，但缺少默认样式(蓝色/下划线/pointer光标)|
|`<span>`|✅支持|核心内联容器|
|`<br>`|✅支持|换行|
|`<em>`|✅支持|斜体样式|
|`<strong>`|✅支持|粗体样式|
|`<small>`|✅支持|小号字体|
|`<s>`|✅支持|删除线|
|`<cite>`|✅支持|斜体样式|
|`<q>`|✅支持|引号包裹|
|`<code>`|✅支持|等宽字体|
|`<sub>`|✅支持|下标|
|`<sup>`|✅支持|上标|
|`<mark>`|✅支持|高亮背景|
|`<b>`|✅支持|粗体|
|`<i>`|✅支持|斜体|
|`<u>`|✅支持|下划线|
|`<abbr>`|✅支持|title属性tooltip|
|`<time>`|✅支持|作为内联容器|
|`<data>`|✅支持|作为内联容器|
|`<kbd>`|✅支持|等宽字体|
|`<samp>`|✅支持|等宽字体|
|`<var>`|✅支持|斜体|
|`<wbr>`|✅支持|可选换行点|
|`<ruby>`|❌不支持|复杂度高，优先级低|
|`<rt>`|❌不支持|依赖ruby|
|`<rp>`|❌不支持|依赖ruby|
|`<bdi>`|❌不支持|双向隔离复杂|
|`<bdo>`|⚠️部分支持|基础dir属性|

### 2.5 图片与多媒体元素

|元素|状态|说明|
|---|---|---|
|`<img>`|✅支持|SDL GPU纹理加载|
|`<picture>`|⚠️部分支持|仅解析，不支持响应式选择|
|`<source>`|⚠️部分支持|仅解析基础属性|
|`<video>`|⚠️可演进|预留接口，需外部解码器|
|`<audio>`|⚠️可演进|预留接口，需外部解码器|
|`<track>`|❌不支持|依赖完整video实现|
|`<map>`|❌不支持|图像映射复杂|
|`<area>`|❌不支持|依赖map|

### 2.6 嵌入内容元素

|元素|状态|说明|
|---|---|---|
|`<iframe>`|❌不支持|需完整浏览器环境|
|`<embed>`|❌不支持|插件机制不适用|
|`<object>`|❌不支持|插件机制不适用|
|`<param>`|❌不支持|依赖object|
|`<portal>`|❌不支持|实验性功能|

### 2.7 SVG与Canvas

|元素|状态|说明|
|---|---|---|
|`<svg>`|⚠️部分支持|基础形状，SDL GPU渲染|
|`<canvas>`|✅支持|2D Context映射到SDL GPU|
|`<math>`|❌不支持|MathML复杂度高|

### 2.8 表格元素

|元素|状态|说明|
|---|---|---|
|`<table>`|✅支持|自研表格布局|
|`<caption>`|✅支持|表格标题|
|`<colgroup>`|⚠️部分支持|仅解析|
|`<col>`|⚠️部分支持|仅解析span属性|
|`<thead>`|✅支持|表头分组|
|`<tbody>`|✅支持|表体分组|
|`<tfoot>`|✅支持|表尾分组|
|`<tr>`|✅支持|表格行|
|`<th>`|✅支持|表头单元格|
|`<td>`|✅支持|数据单元格|

### 2.9 表单元素

|元素|状态|说明|
|---|---|---|
|`<form>`|⚠️部分支持|仅作为容器，无提交|
|`<label>`|⚠️部分支持|for属性解析，但点击label不聚焦关联input|
|`<input>`|⚠️部分支持|见下方input类型表|
|`<button>`|✅支持|可点击按钮|
|`<select>`|⚠️部分支持|解析支持，但无下拉弹出渲染、无键盘导航|
|`<option>`|⚠️部分支持|解析支持，依赖select完整实现|
|`<optgroup>`|⚠️部分支持|解析支持，无视觉分组|
|`<textarea>`|✅支持|多行文本输入|
|`<output>`|✅支持|作为内联容器|
|`<progress>`|✅支持|进度条渲染|
|`<meter>`|✅支持|度量条渲染|
|`<fieldset>`|✅支持|表单分组|
|`<legend>`|✅支持|分组标题|
|`<datalist>`|❌不支持|复杂交互|

### 2.10 Input类型支持

|type|状态|说明|
|---|---|---|
|`text`|✅支持|单行文本输入|
|`password`|✅支持|密码遮罩|
|`number`|✅支持|数字输入|
|`email`|⚠️部分支持|作为text处理|
|`url`|⚠️部分支持|作为text处理|
|`tel`|⚠️部分支持|作为text处理|
|`search`|⚠️部分支持|作为text处理|
|`checkbox`|⚠️部分支持|渲染支持，但点击不切换checked状态|
|`radio`|⚠️部分支持|渲染支持，但点击不切换，不互斥|
|`range`|✅支持|滑块|
|`color`|⚠️可演进|需颜色选择器|
|`date`|⚠️可演进|需日期选择器|
|`time`|⚠️可演进|需时间选择器|
|`datetime-local`|⚠️可演进|需日期时间选择器|
|`month`|⚠️可演进|需月份选择器|
|`week`|⚠️可演进|需周选择器|
|`file`|⚠️可演进|需系统文件对话框|
|`hidden`|✅支持|隐藏字段|
|`submit`|⚠️部分支持|仅触发事件|
|`reset`|⚠️部分支持|仅触发事件|
|`image`|❌不支持|图像提交按钮|

### 2.11 交互元素

|元素|状态|说明|
|---|---|---|
|`<details>`|⚠️部分支持|解析支持，但无点击展开/折叠行为、无open属性切换、无disclosure triangle|
|`<summary>`|⚠️部分支持|解析支持，但无折叠交互、无默认list-item样式|
|`<dialog>`|⚠️部分支持|解析支持，但无showModal()/show()/close()、无::backdrop、无top-layer|
|`<menu>`|❌不支持|上下文菜单|

### 2.12 脚本元素

|元素|状态|说明|
|---|---|---|
|`<script>`|✅支持|QuickJS执行|
|`<noscript>`|✅支持|lexbor已支持|
|`<template>`|✅支持|模板克隆|
|`<slot>`|⚠️可演进|Web Components|

### 2.13 全局属性支持

|属性|状态|说明|
|---|---|---|
|`id`|✅支持|lexbor已支持|
|`class`|✅支持|lexbor已支持|
|`style`|✅支持|内联样式|
|`hidden`|⚠️待修复|属性可解析但无display:none行为|
|`tabindex`|⚠️部分支持|属性可存储，焦点顺序行为待完善|
|`autofocus`|❌待实现|页面加载后自动聚焦|
|`data-*`|⚠️部分支持|属性可存储，JS dataset API缺失|
|`dir`|⚠️部分支持|CSS direction存在但HTML属性未映射|
|`lang`|⚠️待实现|未传递给HarfBuzz做语言感知塑形|
|`title`|⚠️部分支持|属性可存储，无tooltip渲染|
|`inert`|❌待实现|不可交互子树|
|`draggable`|❌待实现|拖拽系统|
|`contenteditable`|❌待实现|富文本编辑|
|`inputmode`|❌待实现|移动端虚拟键盘提示|
|`role`|⚠️部分支持|属性可存储，无无障碍树|
|`aria-*`|⚠️部分支持|属性可存储，无无障碍树|

---

## 3. CSS属性支持清单

### 3.0 已知系统性缺陷

**继承检测机制有误**：`StyleEngine::inheritFromParent()` 使用默认值比较（如 `color=="#000000"` 则继承），导致显式设置黑色被错误覆盖。需添加"属性已显式设置"标志位。

**缺失CSS全局关键字**：`inherit` / `initial` / `unset` / `revert` 均不支持。

**`!important` 未支持**：声明中的 `!important` 可能不影响级联优先级。

**缺失可继承属性传播**：`visibility`, `text-indent`, `text-transform`, `word-break`, `overflow-wrap`, `word-spacing` 未在继承函数中处理。

### 3.1 盒模型

|属性|状态|说明|
|---|---|---|
|`width`|✅支持|Yoga已支持|
|`height`|✅支持|Yoga已支持|
|`min-width`|✅支持|Yoga已支持|
|`max-width`|✅支持|Yoga已支持|
|`min-height`|✅支持|Yoga已支持|
|`max-height`|✅支持|Yoga已支持|
|`margin`|✅支持|Yoga已支持|
|`margin-top/right/bottom/left`|✅支持|Yoga已支持|
|`margin-inline/block`|⚠️部分支持|逻辑属性简写缺失，需映射到物理属性|
|`padding`|✅支持|Yoga已支持|
|`padding-top/right/bottom/left`|✅支持|Yoga已支持|
|`padding-inline/block`|⚠️部分支持|逻辑属性简写缺失，需映射到物理属性|
|`border`|✅支持|Yoga已支持|
|`border-width`|✅支持|Yoga已支持|
|`border-style`|⚠️部分支持|solid/dashed/dotted/none|
|`border-color`|✅支持|SDL GPU渲染|
|`border-radius`|⚠️部分支持|px值支持，百分比值(50%)解析为px数值|
|`box-sizing`|✅支持|Yoga已支持|
|`box-shadow`|✅支持|SDL GPU渲染|
|`outline`|✅支持|SDL GPU渲染|
|`outline-width/style/color/offset`|✅支持|SDL GPU渲染|

### 3.2 布局-Flexbox

|属性|状态|说明|
|---|---|---|
|`display: flex`|✅支持|Yoga核心功能|
|`display: inline-flex`|✅支持|Yoga已支持|
|`flex-direction`|✅支持|Yoga已支持|
|`flex-wrap`|✅支持|Yoga已支持|
|`flex-flow`|✅支持|Yoga已支持|
|`justify-content`|✅支持|Yoga已支持|
|`align-items`|✅支持|Yoga已支持|
|`align-content`|✅支持|Yoga已支持|
|`flex-grow`|✅支持|Yoga已支持|
|`flex-shrink`|✅支持|Yoga已支持|
|`flex-basis`|✅支持|Yoga已支持|
|`flex`|✅支持|Yoga已支持|
|`align-self`|✅支持|Yoga已支持|
|`order`|✅支持|Yoga已支持|
|`gap`|✅支持|Yoga已支持|
|`row-gap`|✅支持|Yoga已支持|
|`column-gap`|✅支持|Yoga已支持|

### 3.3 布局-Grid

|属性|状态|说明|
|---|---|---|
|`display: grid`|❌不支持|Yoga不支持Grid|
|`grid-template-*`|❌不支持|Yoga不支持Grid|
|`grid-auto-*`|❌不支持|Yoga不支持Grid|
|`grid-area`|❌不支持|Yoga不支持Grid|
|`grid-column/row`|❌不支持|Yoga不支持Grid|

### 3.4 布局-定位

|属性|状态|说明|
|---|---|---|
|`display: block`|✅支持|自研Block布局|
|`display: inline`|✅支持|自研Inline布局|
|`display: inline-block`|✅支持|自研布局|
|`display: none`|✅支持|Yoga已支持|
|`display: contents`|❌待实现|布局透传，组件包装器模式|
|`display: flow-root`|❌待实现|新建BFC|
|`display: list-item`|⚠️部分支持|UA样式存在但无marker生成|
|`position`|⚠️部分支持|relative/absolute支持，fixed退化为absolute，sticky未实现|
|`top/right/bottom/left`|✅支持|Yoga已支持|
|`inset`|✅支持|映射到Yoga|
|`z-index`|⚠️部分支持|不区分auto与0，auto应不创建堆叠上下文|
|`float`|⚠️部分支持|基础左右浮动|
|`clear`|⚠️部分支持|基础清除|
|`overflow`|✅支持|裁剪+滚动|
|`overflow-x/y`|✅支持|分轴控制|
|`visibility`|✅支持|hidden/visible|
|`clip-path`|⚠️部分支持|基础形状|

### 3.5 布局-多列

|属性|状态|说明|
|---|---|---|
|`columns`|❌不支持|多列布局复杂|
|`column-count`|❌不支持|多列布局复杂|
|`column-width`|❌不支持|多列布局复杂|
|`column-gap`|✅支持|Flex上下文中|
|`column-rule`|❌不支持|多列布局复杂|

### 3.5a 待新增CSS属性

|属性|状态|说明|
|---|---|---|
|`aspect-ratio`|❌待实现|响应式宽高比，Baseline 2021|
|`object-position`|❌待实现|替换内容定位，配合object-fit|
|`list-style-type`|❌待实现|列表标记类型|
|`list-style-position`|❌待实现|列表标记位置|
|`list-style`|❌待实现|列表样式简写|
|`will-change`|❌待实现|GPU合成层提示|
|`scroll-behavior`|❌待实现|smooth/auto滚动过渡|
|`overscroll-behavior`|❌待实现|防止滚动链|
|`accent-color`|❌待实现|表单控件主题色|
|`appearance`|❌待实现|去除默认控件样式|
|`table-layout`|❌待实现|fixed/auto表格布局|
|`border-collapse`|❌待实现|表格边框合并|
|`border-spacing`|❌待实现|表格边框间距|
|`caption-side`|❌待实现|表格标题定位|
|`tab-size`|❌待实现|制表符宽度|
|`contain`|❌待实现|渲染隔离|
|`content-visibility`|❌待实现|跳过离屏渲染|
|`color-scheme`|❌待实现|light/dark主题|
|`counter-reset`|❌待实现|CSS计数器|
|`counter-increment`|❌待实现|CSS计数器|
|`quotes`|❌待实现|引号定义|
|`scroll-snap-type`|❌待实现|滚动吸附|
|`scroll-snap-align`|❌待实现|滚动吸附对齐|
|`image-rendering`|❌待实现|像素风纹理采样|

### 3.5b 待新增CSS简写/逻辑属性

|属性|状态|说明|
|---|---|---|
|`place-items`|❌待实现|align-items + justify-items|
|`place-content`|❌待实现|align-content + justify-content|
|`place-self`|❌待实现|align-self + justify-self|
|`margin-inline-start/end`|❌待实现|CSS逻辑属性|
|`padding-block-start/end`|❌待实现|CSS逻辑属性|
|`border-inline/block`|❌待实现|CSS逻辑属性|
|`inset-block/inline`|❌待实现|CSS逻辑属性|



|属性|状态|说明|
|---|---|---|
|`color`|✅支持|SDL GPU渲染|
|`background`|✅支持|SDL GPU渲染|
|`background-color`|✅支持|SDL GPU渲染|
|`background-image`|✅支持|SDL GPU纹理|
|`background-position`|✅支持|SDL GPU渲染|
|`background-size`|✅支持|cover/contain/值|
|`background-repeat`|✅支持|SDL GPU渲染|
|`background-attachment`|⚠️部分支持|scroll/local|
|`background-clip`|✅支持|SDL GPU裁剪|
|`background-origin`|✅支持|SDL GPU渲染|
|`background-blend-mode`|⚠️部分支持|基础混合模式|
|`opacity`|✅支持|SDL GPU渲染|
|`mix-blend-mode`|⚠️部分支持|基础混合模式|

### 3.7 渐变

|属性|状态|说明|
|---|---|---|
|`linear-gradient()`|✅支持|SDL GPU着色器|
|`radial-gradient()`|✅支持|SDL GPU着色器|
|`conic-gradient()`|⚠️可演进|需自研着色器|
|`repeating-linear-gradient()`|✅支持|SDL GPU着色器|
|`repeating-radial-gradient()`|✅支持|SDL GPU着色器|

### 3.8 字体与文本

|属性|状态|说明|
|---|---|---|
|`font-family`|✅支持|FreeType加载|
|`font-size`|✅支持|FreeType+msdfgen|
|`font-weight`|✅支持|FreeType已支持|
|`font-style`|✅支持|FreeType已支持|
|`font-variant`|⚠️部分支持|small-caps|
|`font-stretch`|⚠️部分支持|FreeType部分支持|
|`font`|✅支持|简写属性|
|`line-height`|✅支持|Yoga已支持|
|`letter-spacing`|✅支持|HarfBuzz调整|
|`word-spacing`|✅支持|HarfBuzz调整|
|`text-align`|✅支持|自研文本布局|
|`text-align-last`|⚠️部分支持|基础支持|
|`text-indent`|✅支持|自研文本布局|
|`text-decoration`|✅支持|SDL GPU渲染|
|`text-decoration-line`|✅支持|underline/overline/line-through|
|`text-decoration-color`|✅支持|SDL GPU渲染|
|`text-decoration-style`|⚠️部分支持|solid/dashed/dotted|
|`text-decoration-thickness`|✅支持|SDL GPU渲染|
|`text-transform`|✅支持|uppercase/lowercase/capitalize|
|`text-overflow`|✅支持|ellipsis/clip|
|`text-shadow`|✅支持|SDL GPU渲染|
|`white-space`|✅支持|normal/nowrap/pre/pre-wrap|
|`word-break`|✅支持|HarfBuzz处理|
|`word-wrap/overflow-wrap`|✅支持|HarfBuzz处理|
|`vertical-align`|⚠️部分支持|baseline/top/middle/bottom|
|`direction`|✅支持|HarfBuzz RTL支持|
|`unicode-bidi`|⚠️部分支持|基础双向文本|
|`writing-mode`|⚠️可演进|垂直排版复杂|
|`-webkit-line-clamp`|✅支持|多行截断|

### 3.9 变换

|属性|状态|说明|
|---|---|---|
|`transform`|✅支持|SDL GPU矩阵|
|`transform-origin`|✅支持|SDL GPU渲染|
|`transform-style`|⚠️部分支持|flat，3D受限|
|`perspective`|⚠️部分支持|基础透视|
|`perspective-origin`|⚠️部分支持|基础支持|
|`backface-visibility`|✅支持|SDL GPU渲染|
|`translate`|✅支持|SDL GPU渲染|
|`rotate`|✅支持|SDL GPU渲染|
|`scale`|✅支持|SDL GPU渲染|

### 3.10 过渡与动画

|属性|状态|说明|
|---|---|---|
|`transition`|✅支持|自研动画系统|
|`transition-property`|✅支持|自研动画系统|
|`transition-duration`|✅支持|自研动画系统|
|`transition-timing-function`|✅支持|ease/linear/cubic-bezier|
|`transition-delay`|✅支持|自研动画系统|
|`animation`|✅支持|自研动画系统|
|`animation-name`|✅支持|@keyframes引用|
|`animation-duration`|✅支持|自研动画系统|
|`animation-timing-function`|✅支持|自研动画系统|
|`animation-delay`|✅支持|自研动画系统|
|`animation-iteration-count`|✅支持|自研动画系统|
|`animation-direction`|✅支持|自研动画系统|
|`animation-fill-mode`|✅支持|自研动画系统|
|`animation-play-state`|✅支持|自研动画系统|
|`@keyframes`|✅支持|lexbor解析+自研|

### 3.11 滤镜与特效

|属性|状态|说明|
|---|---|---|
|`filter`|⚠️部分支持|基础滤镜，SDL GPU着色器|
|`blur()`|✅支持|SDL GPU着色器|
|`brightness()`|✅支持|SDL GPU着色器|
|`contrast()`|✅支持|SDL GPU着色器|
|`grayscale()`|✅支持|SDL GPU着色器|
|`saturate()`|✅支持|SDL GPU着色器|
|`sepia()`|✅支持|SDL GPU着色器|
|`hue-rotate()`|✅支持|SDL GPU着色器|
|`invert()`|✅支持|SDL GPU着色器|
|`opacity()`|✅支持|SDL GPU着色器|
|`drop-shadow()`|✅支持|SDL GPU着色器|
|`backdrop-filter`|⚠️部分支持|需额外渲染pass|

### 3.12 光标与交互

|属性|状态|说明|
|---|---|---|
|`cursor`|✅支持|SDL系统光标|
|`pointer-events`|✅支持|事件穿透控制|
|`user-select`|✅支持|文本选择控制|
|`touch-action`|⚠️部分支持|基础手势|
|`resize`|⚠️可演进|需实现拖拽手柄|
|`caret-color`|✅支持|SDL GPU渲染|

### 3.13 选择器支持

|选择器|状态|说明|
|---|---|---|
|`*`|✅支持|lexbor已支持|
|`element`|✅支持|lexbor已支持|
|`.class`|✅支持|lexbor已支持|
|`#id`|✅支持|lexbor已支持|
|`[attr]`|✅支持|lexbor已支持|
|`[attr=value]`|✅支持|lexbor已支持|
|`[attr~=value]`|✅支持|lexbor已支持|
|`[attr\|=value]`|✅支持|lexbor已支持|
|`[attr^=value]`|✅支持|lexbor已支持|
|`[attr$=value]`|✅支持|lexbor已支持|
|`[attr*=value]`|✅支持|lexbor已支持|
|`A B`|✅支持|后代选择器|
|`A > B`|✅支持|子选择器|
|`A + B`|✅支持|相邻兄弟|
|`A ~ B`|✅支持|通用兄弟|
|`:hover`|✅支持|鼠标悬停|
|`:active`|✅支持|激活状态|
|`:focus`|✅支持|焦点状态|
|`:focus-visible`|✅支持|键盘焦点|
|`:focus-within`|✅支持|子元素焦点|
|`:visited`|❌不支持|无浏览历史|
|`:link`|⚠️部分支持|等同于a[href]|
|`:checked`|✅支持|表单选中|
|`:disabled`|✅支持|表单禁用|
|`:enabled`|✅支持|表单启用|
|`:required`|✅支持|表单必填|
|`:optional`|✅支持|表单可选|
|`:valid`|⚠️部分支持|基础验证|
|`:invalid`|⚠️部分支持|基础验证|
|`:placeholder-shown`|✅支持|占位符显示|
|`:first-child`|✅支持|lexbor已支持|
|`:last-child`|✅支持|lexbor已支持|
|`:nth-child()`|✅支持|lexbor已支持|
|`:nth-last-child()`|✅支持|lexbor已支持|
|`:first-of-type`|✅支持|lexbor已支持|
|`:last-of-type`|✅支持|lexbor已支持|
|`:nth-of-type()`|✅支持|lexbor已支持|
|`:only-child`|✅支持|lexbor已支持|
|`:only-of-type`|✅支持|lexbor已支持|
|`:empty`|✅支持|lexbor已支持|
|`:not()`|✅支持|lexbor已支持|
|`:is()`|✅支持|lexbor已支持|
|`:where()`|✅支持|lexbor已支持|
|`:has()`|⚠️部分支持|性能敏感|
|`:open`/`:closed`|❌待实现|details/dialog状态|
|`:target`|❌待实现|URL hash匹配|
|`:any-link`|❌待实现|带href的a元素|
|`:indeterminate`|❌待实现|checkbox不确定状态|
|`:nth-last-of-type()`|❌待实现|仅有nth-of-type|
|`::before`|✅支持|伪元素渲染|
|`::after`|✅支持|伪元素渲染|
|`::first-line`|⚠️部分支持|选择器匹配但无渲染实现|
|`::first-letter`|⚠️部分支持|选择器匹配但无渲染实现|
|`::placeholder`|⚠️部分支持|选择器匹配但无渲染实现|
|`::selection`|⚠️部分支持|选择器匹配但无渲染实现|
|`::marker`|❌待实现|列表标记样式|
|`::backdrop`|❌待实现|modal dialog背景遮罩|

### 3.14 At规则

|规则|状态|说明|
|---|---|---|
|`@import`|✅支持|lexbor已支持|
|`@media`|✅支持|lexbor已支持，自研查询|
|`@font-face`|✅支持|FreeType加载|
|`@keyframes`|✅支持|lexbor已支持|
|`@supports`|✅支持|lexbor已支持|
|`@page`|❌不支持|无打印需求|
|`@layer`|⚠️可演进|级联层|
|`@container`|⚠️可演进|容器查询|
|`@scope`|❌不支持|实验性功能|

### 3.15 CSS单位

|单位|状态|说明|
|---|---|---|
|`px`|✅支持|基础单位|
|`%`|✅支持|Yoga已支持|
|`em`|✅支持|相对字体大小|
|`rem`|✅支持|相对根字体|
|`vw/vh`|✅支持|视口单位|
|`vmin/vmax`|✅支持|视口单位|
|`ch`|✅支持|字符宽度|
|`ex`|⚠️部分支持|近似x高度|
|`cm/mm/in/pt/pc`|✅支持|绝对单位换算|
|`fr`|❌不支持|Grid专用|
|`deg/rad/turn`|✅支持|角度单位|
|`s/ms`|✅支持|时间单位|

### 3.16 CSS函数

|函数|状态|说明|
|---|---|---|
|`calc()`|✅支持|lexbor解析+自研计算|
|`min()`|✅支持|自研计算|
|`max()`|✅支持|自研计算|
|`clamp()`|✅支持|自研计算|
|`var()`|✅支持|CSS变量|
|`url()`|✅支持|资源引用|
|`rgb()/rgba()`|✅支持|颜色值|
|`hsl()/hsla()`|✅支持|颜色值|
|`hwb()`|⚠️可演进|颜色值|
|`lab()/lch()`|⚠️可演进|颜色值|
|`oklch()/oklab()`|⚠️可演进|颜色值|
|`color()`|⚠️可演进|颜色函数|
|`color-mix()`|❌待实现|颜色混合，Baseline 2023|
|`light-dark()`|❌待实现|随color-scheme切换|
|`counter()`|❌待实现|CSS计数器|
|`counters()`|❌待实现|CSS计数器|
|`env()`|❌待实现|safe-area-inset-*|
|`attr()`|⚠️部分支持|基础属性引用|

---

## 4. DOM API支持清单

### 4.1 Node接口

|属性/方法|状态|说明|
|---|---|---|
|`nodeType`|⚠️待绑定|C++层支持，JS绑定缺失|
|`nodeName`|⚠️待绑定|C++层支持，JS绑定缺失|
|`nodeValue`|⚠️待绑定|C++层支持，JS绑定缺失|
|`textContent`|⚠️部分支持|getter可用，setter待绑定|
|`parentNode`|⚠️待绑定|C++层支持，JS绑定缺失|
|`parentElement`|⚠️待绑定|C++层支持，JS绑定缺失|
|`childNodes`|⚠️待绑定|仅有非标准getChildren()|
|`firstChild`|⚠️待绑定|C++层支持，JS绑定缺失|
|`lastChild`|⚠️待绑定|C++层支持，JS绑定缺失|
|`previousSibling`|⚠️待绑定|C++层支持，JS绑定缺失|
|`nextSibling`|⚠️待绑定|C++层支持，JS绑定缺失|
|`ownerDocument`|⚠️待绑定|C++层支持，JS绑定缺失|
|`appendChild()`|✅支持|QuickJS绑定|
|`insertBefore()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`removeChild()`|✅支持|QuickJS绑定|
|`replaceChild()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`cloneNode()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`contains()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`hasChildNodes()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`normalize()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`isEqualNode()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`isSameNode()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`compareDocumentPosition()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`isConnected`|❌待实现|框架常用，需新增|

### 4.2 Element接口

|属性/方法|状态|说明|
|---|---|---|
|`tagName`|✅支持|QuickJS绑定|
|`id`|✅支持|QuickJS绑定|
|`className`|✅支持|QuickJS绑定|
|`classList`|✅支持|QuickJS绑定|
|`attributes`|✅支持|QuickJS绑定|
|`innerHTML`|✅支持|lexbor解析|
|`outerHTML`|✅支持|lexbor序列化|
|`dataset`|⚠️待绑定|data-*属性可存储但无JS DOMStringMap|
|`hidden`|⚠️待绑定|属性可解析但无行为(需UA样式+JS属性)|
|`children`|⚠️部分支持|非标准getChildren()方法，应为property|
|`firstElementChild`|⚠️待绑定|C++层支持，JS绑定缺失|
|`lastElementChild`|⚠️待绑定|C++层支持，JS绑定缺失|
|`previousElementSibling`|⚠️待绑定|C++层支持，JS绑定缺失|
|`nextElementSibling`|⚠️待绑定|C++层支持，JS绑定缺失|
|`childElementCount`|⚠️待绑定|C++层支持，JS绑定缺失|
|`getAttribute()`|✅支持|QuickJS绑定|
|`setAttribute()`|✅支持|QuickJS绑定|
|`removeAttribute()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`hasAttribute()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`toggleAttribute()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`getAttributeNames()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`querySelector()`|⚠️部分支持|仅document上可用，元素级未绑定|
|`querySelectorAll()`|⚠️部分支持|仅document上可用，元素级未绑定|
|`matches()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`closest()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`getElementsByTagName()`|⚠️待绑定|C++层支持，元素级JS绑定缺失|
|`getElementsByClassName()`|⚠️待绑定|C++层支持，元素级JS绑定缺失|
|`getBoundingClientRect()`|⚠️待绑定|Yoga布局结果可用，JS绑定缺失|
|`getClientRects()`|⚠️待绑定|Yoga布局结果可用，JS绑定缺失|
|`scrollIntoView()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`scroll()/scrollTo()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`scrollBy()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`scrollTop/scrollLeft`|⚠️待绑定|C++层支持，JS绑定缺失|
|`scrollWidth/scrollHeight`|⚠️待绑定|Yoga布局结果可用，JS绑定缺失|
|`clientTop/clientLeft`|⚠️待绑定|Yoga布局结果可用，JS绑定缺失|
|`clientWidth/clientHeight`|⚠️待绑定|Yoga布局结果可用，JS绑定缺失|
|`offsetTop/offsetLeft`|⚠️待绑定|Yoga布局结果可用，JS绑定缺失|
|`offsetWidth/offsetHeight`|⚠️待绑定|Yoga布局结果可用，JS绑定缺失|
|`offsetParent`|⚠️待绑定|C++层支持，JS绑定缺失|
|`insertAdjacentHTML()`|✅支持|lexbor解析|
|`insertAdjacentElement()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`insertAdjacentText()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`remove()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`before()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`after()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`replaceWith()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`prepend()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`append()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`replaceChildren()`|❌待实现|需新增|
|`focus()`|✅支持|焦点管理|
|`blur()`|✅支持|焦点管理|
|`click()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`dispatchEvent()`|⚠️待绑定|C++层支持，JS绑定缺失|

### 4.3 Document接口

|属性/方法|状态|说明|
|---|---|---|
|`documentElement`|✅支持|QuickJS绑定|
|`head`|⚠️待绑定|C++层支持，JS绑定缺失|
|`body`|✅支持|QuickJS绑定|
|`title`|✅支持|QuickJS绑定|
|`activeElement`|⚠️待绑定|焦点管理C++层支持，JS绑定缺失|
|`getElementById()`|✅支持|QuickJS绑定|
|`getElementsByTagName()`|✅支持|QuickJS绑定|
|`getElementsByClassName()`|✅支持|QuickJS绑定|
|`getElementsByName()`|✅支持|QuickJS绑定|
|`querySelector()`|✅支持|lexbor选择器|
|`querySelectorAll()`|✅支持|lexbor选择器|
|`createElement()`|✅支持|QuickJS绑定|
|`createTextNode()`|✅支持|QuickJS绑定|
|`createComment()`|✅支持|QuickJS绑定|
|`createDocumentFragment()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`createAttribute()`|✅支持|QuickJS绑定|
|`importNode()`|✅支持|QuickJS绑定|
|`adoptNode()`|✅支持|QuickJS绑定|
|`createEvent()`|⚠️部分支持|基础事件类型|
|`createRange()`|⚠️可演进|文本选区|
|`createTreeWalker()`|✅支持|QuickJS绑定|
|`createNodeIterator()`|✅支持|QuickJS绑定|
|`elementFromPoint()`|✅支持|命中测试|
|`elementsFromPoint()`|✅支持|命中测试|
|`getSelection()`|⚠️可演进|文本选区|
|`hasFocus()`|✅支持|焦点管理|

### 4.4 事件系统

|接口/方法|状态|说明|
|---|---|---|
|`EventTarget`|✅支持|QuickJS绑定|
|`addEventListener()`|✅支持|QuickJS绑定|
|`removeEventListener()`|✅支持|QuickJS绑定|
|`dispatchEvent()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`Event`|✅支持|QuickJS绑定|
|`CustomEvent`|✅支持|QuickJS绑定|
|`MouseEvent`|✅支持|SDL事件映射|
|`KeyboardEvent`|✅支持|SDL事件映射|
|`WheelEvent`|⚠️部分支持|C++滚动处理存在，但JS事件无deltaX/Y/Z属性|
|`PointerEvent`|⚠️部分支持|C++ PointerType存在，但JS无pointerId/pointerType等属性|
|`TouchEvent`|⚠️部分支持|C++ touch处理存在，但JS事件未完整暴露|
|`FocusEvent`|✅支持|焦点管理|
|`InputEvent`|⚠️部分支持|事件触发但缺少data/inputType属性|
|`CompositionEvent`|⚠️可演进|IME输入|
|`DragEvent`|⚠️可演进|拖拽系统|
|`AnimationEvent`|✅支持|动画系统|
|`TransitionEvent`|✅支持|过渡系统|
|`UIEvent`|✅支持|QuickJS绑定|
|`event.preventDefault()`|✅支持|QuickJS绑定|
|`event.stopPropagation()`|✅支持|QuickJS绑定|
|`event.stopImmediatePropagation()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`event.target`|✅支持|dispatch时设置|
|`event.currentTarget`|⚠️待绑定|需在事件处理期间正确设置|
|`event.bubbles`|✅支持|事件冒泡|
|`event.cancelable`|✅支持|QuickJS绑定|
|`event.composed`|⚠️可演进|Shadow DOM|
|`event.eventPhase`|✅支持|捕获/冒泡阶段|

### 4.4a 缺失的事件类型

|事件|状态|说明|
|---|---|---|
|`scroll`|❌待实现|滚动事件，JS无法监听|
|`resize`|❌待实现|窗口/元素尺寸变化|
|`DOMContentLoaded`|❌待实现|DOM解析完成|
|`change`|❌待实现|表单值提交|
|`submit`|❌待实现|表单提交|
|`contextmenu`|❌待实现|右键菜单|
|`load`/`error` (img)|❌待实现|资源加载|
|`toggle`|❌待实现|details展开/折叠|
|`close`/`cancel`|❌待实现|dialog关闭|
|`compositionstart/update/end`|❌待实现|IME输入(CJK)|
|`copy`/`cut`/`paste`|❌待实现|剪贴板|
|`beforeinput`|❌待实现|可取消的输入前事件|

### 4.4b 事件属性缺失

|事件类型|缺失属性|说明|
|---|---|---|
|MouseEvent|`offsetX`/`offsetY`|相对目标元素位置|
|MouseEvent|`pageX`/`pageY`|相对文档位置|
|MouseEvent|`altKey`/`ctrlKey`/`shiftKey`/`metaKey`|修饰键状态|
|KeyboardEvent|`key`/`code`|dispatch时未设置|
|KeyboardEvent|修饰键|dispatch时未设置|
|WheelEvent|`deltaX`/`deltaY`/`deltaZ`|滚动量|
|InputEvent|`data`/`inputType`|插入字符/变更类型|
|FocusEvent|`relatedTarget`|失去/获得焦点的对端元素|
|PointerEvent|`pointerId`/`pointerType`/`pressure`|指针属性|

### 4.4c HTMLElement特定接口(待实现)

|接口|属性/方法|说明|
|---|---|---|
|HTMLInputElement|`value`, `checked`, `disabled`, `type`, `name`|表单核心属性|
|HTMLInputElement|`placeholder`, `required`, `readOnly`|验证属性|
|HTMLInputElement|`select()`, `setSelectionRange()`|选区方法|
|HTMLInputElement|`checkValidity()`, `setCustomValidity()`|验证方法|
|HTMLSelectElement|`value`, `selectedIndex`, `options`|选择框核心|
|HTMLTextAreaElement|`value`, `selectionStart`, `selectionEnd`|文本域核心|
|HTMLDialogElement|`open`, `showModal()`, `show()`, `close()`|对话框核心|
|HTMLDetailsElement|`open`|折叠面板|
|HTMLAnchorElement|`href`, `target`|链接属性|
|HTMLImageElement|`src`, `alt`, `naturalWidth/Height`, `complete`|图片属性|
|HTMLFormElement|`elements`, `submit()`, `reset()`|表单方法|



|接口/方法|状态|说明|
|---|---|---|
|`element.style`|✅支持|QuickJS绑定|
|`CSSStyleDeclaration`|✅支持|QuickJS绑定|
|`getComputedStyle()`|✅支持|样式计算|
|`document.styleSheets`|✅支持|QuickJS绑定|
|`CSSStyleSheet`|✅支持|QuickJS绑定|
|`CSSRule`|✅支持|QuickJS绑定|
|`CSSStyleRule`|✅支持|QuickJS绑定|
|`CSSMediaRule`|✅支持|QuickJS绑定|
|`CSSKeyframesRule`|✅支持|QuickJS绑定|
|`insertRule()`|✅支持|QuickJS绑定|
|`deleteRule()`|✅支持|QuickJS绑定|
|`CSS.supports()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`matchMedia()`|⚠️待绑定|C++层支持，JS绑定缺失|
|`MediaQueryList`|⚠️待绑定|C++层支持，JS绑定缺失|

### 4.6 定时器与动画帧

|方法|状态|说明|
|---|---|---|
|`setTimeout()`|✅支持|QuickJS绑定|
|`clearTimeout()`|✅支持|QuickJS绑定|
|`setInterval()`|✅支持|QuickJS绑定|
|`clearInterval()`|✅支持|QuickJS绑定|
|`requestAnimationFrame()`|✅支持|SDL帧同步|
|`cancelAnimationFrame()`|✅支持|SDL帧同步|
|`requestIdleCallback()`|⚠️可演进|空闲回调|
|`cancelIdleCallback()`|⚠️可演进|空闲回调|
|`queueMicrotask()`|✅支持|QuickJS绑定|

### 4.7 存储API

|接口|状态|说明|
|---|---|---|
|`localStorage`|✅支持|自研文件存储|
|`sessionStorage`|✅支持|自研内存存储|
|`Storage.getItem()`|✅支持|QuickJS绑定|
|`Storage.setItem()`|✅支持|QuickJS绑定|
|`Storage.removeItem()`|✅支持|QuickJS绑定|
|`Storage.clear()`|✅支持|QuickJS绑定|
|`Storage.key()`|✅支持|QuickJS绑定|
|`Storage.length`|✅支持|QuickJS绑定|
|`IndexedDB`|❌不支持|复杂度高|
|`Cookie`|❌不支持|无网络场景|

### 4.8 网络API

|接口|状态|说明|
|---|---|---|
|`fetch()`|⚠️可演进|需外部HTTP库|
|`XMLHttpRequest`|⚠️可演进|需外部HTTP库|
|`WebSocket`|⚠️可演进|需外部库|
|`EventSource`|❌不支持|SSE复杂|
|`Beacon`|❌不支持|无网络场景|

### 4.9 其他Web API

|接口|状态|说明|
|---|---|---|
|`console`|✅支持|QuickJS绑定|
|`JSON`|✅支持|QuickJS内置|
|`Promise`|✅支持|QuickJS内置|
|`URL`|✅支持|QuickJS绑定|
|`URLSearchParams`|✅支持|QuickJS绑定|
|`TextEncoder`|✅支持|QuickJS绑定|
|`TextDecoder`|✅支持|QuickJS绑定|
|`atob()/btoa()`|✅支持|QuickJS绑定|
|`Blob`|⚠️可演进|二进制数据|
|`File`|⚠️可演进|文件处理|
|`FileReader`|⚠️可演进|文件读取|
|`FormData`|⚠️可演进|表单数据|
|`AbortController`|✅支持|QuickJS绑定|
|`AbortSignal`|✅支持|QuickJS绑定|
|`IntersectionObserver`|⚠️可演进|可见性监测|
|`ResizeObserver`|⚠️待绑定|C++层支持，JS绑定缺失|
|`MutationObserver`|⚠️待绑定|C++层支持，JS绑定缺失|
|`PerformanceObserver`|⚠️可演进|性能监测|
|`Clipboard`|⚠️可演进|系统剪贴板|
|`Notification`|❌不支持|系统通知|
|`Geolocation`|❌不支持|系统定位|
|`History`|⚠️部分支持|简化路由|
|`Location`|⚠️部分支持|简化路由|
|`Navigator`|⚠️部分支持|基础属性|
|`Screen`|✅支持|SDL窗口信息|
|`Window`|⚠️部分支持|简化实现|
|`performance.now()`|❌待实现|高精度时间戳|
|`structuredClone()`|❌待实现|深拷贝JS对象|
|`DOMParser`|❌待实现|HTML字符串解析为DOM|
|`DOMRect`|❌待实现|getBoundingClientRect返回类型|
|`FormData`|❌待实现|表单数据收集|

---

## 5. 不支持项汇总

以下功能由于设计原则（强裁剪意识、无WebView、GPU驱动）明确不支持：

|类别|项目|原因|
|---|---|---|
|HTML|`<iframe>`,`<embed>`,`<object>`|需完整浏览器环境|
|HTML|`<ruby>`,`<rt>`,`<rp>`|Ruby注音复杂度高|
|HTML|`<map>`,`<area>`|图像映射交互复杂|
|HTML|`<track>`|依赖完整媒体实现|
|CSS|Grid布局|Yoga不支持，需自研|
|CSS|多列布局|复杂度高，优先级低|
|CSS|`:visited`|无浏览历史概念|
|CSS|`@page`|无打印需求|
|DOM|IndexedDB|复杂度高|
|DOM|Service Worker|需浏览器环境|
|DOM|Web Worker|需多线程实现|
|DOM|WebGL|直接使用SDL GPU|
|DOM|WebRTC|音视频通信复杂|
|DOM|WebXR|XR设备支持复杂|

---

## 6. 差距分析备注

> 本文档经过源码级对照分析（2024年），部分此前标记为"✅支持"的特性已更正为实际状态。
> 标记为"⚠️待绑定"表示 C++ 层已有实现但 QuickJS JS 绑定缺失。
> 标记为"❌待实现"表示需要新增功能。
> 完整差距分析详见 `html_css_dom_gap_analysis.md`，待办事项详见 `/doc/todo.md`。

---

## 6. 参考文献

[1] WHATWG. HTML Living Standard. https://html.spec.whatwg.org/multipage/

[2] MDN Web Docs. CSS Reference. https://developer.mozilla.org/en-US/docs/Web/CSS/Reference

[3] WHATWG. DOM Living Standard. https://dom.spec.whatwg.org/