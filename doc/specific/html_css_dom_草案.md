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
|`<a>`|✅支持|支持href、target属性|
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
|`<label>`|✅支持|for属性关联|
|`<input>`|⚠️部分支持|见下方input类型表|
|`<button>`|✅支持|可点击按钮|
|`<select>`|✅支持|下拉选择框|
|`<option>`|✅支持|选项|
|`<optgroup>`|✅支持|选项分组|
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
|`checkbox`|✅支持|复选框|
|`radio`|✅支持|单选按钮|
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
|`<details>`|✅支持|可折叠内容|
|`<summary>`|✅支持|折叠标题|
|`<dialog>`|✅支持|模态对话框|
|`<menu>`|❌不支持|上下文菜单|

### 2.12 脚本元素

|元素|状态|说明|
|---|---|---|
|`<script>`|✅支持|QuickJS执行|
|`<noscript>`|✅支持|lexbor已支持|
|`<template>`|✅支持|模板克隆|
|`<slot>`|⚠️可演进|Web Components|

---

## 3. CSS属性支持清单

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
|`margin-inline/block`|✅支持|映射到Yoga|
|`padding`|✅支持|Yoga已支持|
|`padding-top/right/bottom/left`|✅支持|Yoga已支持|
|`padding-inline/block`|✅支持|映射到Yoga|
|`border`|✅支持|Yoga已支持|
|`border-width`|✅支持|Yoga已支持|
|`border-style`|⚠️部分支持|solid/dashed/dotted/none|
|`border-color`|✅支持|SDL GPU渲染|
|`border-radius`|✅支持|SDL GPU圆角|
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
|`position`|✅支持|Yoga已支持relative/absolute|
|`top/right/bottom/left`|✅支持|Yoga已支持|
|`inset`|✅支持|映射到Yoga|
|`z-index`|✅支持|SDL GPU图层排序|
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

### 3.6 颜色与背景

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
|`::before`|✅支持|伪元素渲染|
|`::after`|✅支持|伪元素渲染|
|`::first-line`|⚠️部分支持|基础支持|
|`::first-letter`|⚠️部分支持|基础支持|
|`::placeholder`|✅支持|占位符样式|
|`::selection`|✅支持|选中文本样式|

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
|`color-mix()`|⚠️可演进|颜色混合|
|`attr()`|⚠️部分支持|基础属性引用|
|`env()`|⚠️部分支持|safe-area-inset-*|

---

## 4. DOM API支持清单

### 4.1 Node接口

|属性/方法|状态|说明|
|---|---|---|
|`nodeType`|✅支持|QuickJS绑定|
|`nodeName`|✅支持|QuickJS绑定|
|`nodeValue`|✅支持|QuickJS绑定|
|`textContent`|✅支持|QuickJS绑定|
|`parentNode`|✅支持|QuickJS绑定|
|`parentElement`|✅支持|QuickJS绑定|
|`childNodes`|✅支持|QuickJS绑定|
|`firstChild`|✅支持|QuickJS绑定|
|`lastChild`|✅支持|QuickJS绑定|
|`previousSibling`|✅支持|QuickJS绑定|
|`nextSibling`|✅支持|QuickJS绑定|
|`ownerDocument`|✅支持|QuickJS绑定|
|`appendChild()`|✅支持|QuickJS绑定|
|`insertBefore()`|✅支持|QuickJS绑定|
|`removeChild()`|✅支持|QuickJS绑定|
|`replaceChild()`|✅支持|QuickJS绑定|
|`cloneNode()`|✅支持|QuickJS绑定|
|`contains()`|✅支持|QuickJS绑定|
|`hasChildNodes()`|✅支持|QuickJS绑定|
|`normalize()`|✅支持|QuickJS绑定|
|`isEqualNode()`|✅支持|QuickJS绑定|
|`isSameNode()`|✅支持|QuickJS绑定|
|`compareDocumentPosition()`|✅支持|QuickJS绑定|

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
|`children`|✅支持|QuickJS绑定|
|`firstElementChild`|✅支持|QuickJS绑定|
|`lastElementChild`|✅支持|QuickJS绑定|
|`previousElementSibling`|✅支持|QuickJS绑定|
|`nextElementSibling`|✅支持|QuickJS绑定|
|`childElementCount`|✅支持|QuickJS绑定|
|`getAttribute()`|✅支持|QuickJS绑定|
|`setAttribute()`|✅支持|QuickJS绑定|
|`removeAttribute()`|✅支持|QuickJS绑定|
|`hasAttribute()`|✅支持|QuickJS绑定|
|`toggleAttribute()`|✅支持|QuickJS绑定|
|`getAttributeNames()`|✅支持|QuickJS绑定|
|`querySelector()`|✅支持|lexbor选择器|
|`querySelectorAll()`|✅支持|lexbor选择器|
|`matches()`|✅支持|lexbor选择器|
|`closest()`|✅支持|lexbor选择器|
|`getElementsByTagName()`|✅支持|QuickJS绑定|
|`getElementsByClassName()`|✅支持|QuickJS绑定|
|`getBoundingClientRect()`|✅支持|Yoga布局结果|
|`getClientRects()`|✅支持|Yoga布局结果|
|`scrollIntoView()`|✅支持|自研滚动|
|`scroll()/scrollTo()`|✅支持|自研滚动|
|`scrollBy()`|✅支持|自研滚动|
|`scrollTop/scrollLeft`|✅支持|自研滚动|
|`scrollWidth/scrollHeight`|✅支持|Yoga布局结果|
|`clientTop/clientLeft`|✅支持|Yoga布局结果|
|`clientWidth/clientHeight`|✅支持|Yoga布局结果|
|`offsetTop/offsetLeft`|✅支持|Yoga布局结果|
|`offsetWidth/offsetHeight`|✅支持|Yoga布局结果|
|`offsetParent`|✅支持|QuickJS绑定|
|`insertAdjacentHTML()`|✅支持|lexbor解析|
|`insertAdjacentElement()`|✅支持|QuickJS绑定|
|`insertAdjacentText()`|✅支持|QuickJS绑定|
|`remove()`|✅支持|QuickJS绑定|
|`before()`|✅支持|QuickJS绑定|
|`after()`|✅支持|QuickJS绑定|
|`replaceWith()`|✅支持|QuickJS绑定|
|`prepend()`|✅支持|QuickJS绑定|
|`append()`|✅支持|QuickJS绑定|
|`focus()`|✅支持|焦点管理|
|`blur()`|✅支持|焦点管理|
|`click()`|✅支持|事件触发|

### 4.3 Document接口

|属性/方法|状态|说明|
|---|---|---|
|`documentElement`|✅支持|QuickJS绑定|
|`head`|✅支持|QuickJS绑定|
|`body`|✅支持|QuickJS绑定|
|`title`|✅支持|QuickJS绑定|
|`activeElement`|✅支持|焦点管理|
|`getElementById()`|✅支持|QuickJS绑定|
|`getElementsByTagName()`|✅支持|QuickJS绑定|
|`getElementsByClassName()`|✅支持|QuickJS绑定|
|`getElementsByName()`|✅支持|QuickJS绑定|
|`querySelector()`|✅支持|lexbor选择器|
|`querySelectorAll()`|✅支持|lexbor选择器|
|`createElement()`|✅支持|QuickJS绑定|
|`createTextNode()`|✅支持|QuickJS绑定|
|`createComment()`|✅支持|QuickJS绑定|
|`createDocumentFragment()`|✅支持|QuickJS绑定|
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
|`dispatchEvent()`|✅支持|QuickJS绑定|
|`Event`|✅支持|QuickJS绑定|
|`CustomEvent`|✅支持|QuickJS绑定|
|`MouseEvent`|✅支持|SDL事件映射|
|`KeyboardEvent`|✅支持|SDL事件映射|
|`WheelEvent`|✅支持|SDL事件映射|
|`PointerEvent`|✅支持|SDL事件映射|
|`TouchEvent`|✅支持|SDL事件映射|
|`FocusEvent`|✅支持|焦点管理|
|`InputEvent`|✅支持|表单输入|
|`CompositionEvent`|⚠️可演进|IME输入|
|`DragEvent`|⚠️可演进|拖拽系统|
|`AnimationEvent`|✅支持|动画系统|
|`TransitionEvent`|✅支持|过渡系统|
|`UIEvent`|✅支持|QuickJS绑定|
|`event.preventDefault()`|✅支持|QuickJS绑定|
|`event.stopPropagation()`|✅支持|QuickJS绑定|
|`event.stopImmediatePropagation()`|✅支持|QuickJS绑定|
|`event.target`|✅支持|QuickJS绑定|
|`event.currentTarget`|✅支持|QuickJS绑定|
|`event.bubbles`|✅支持|事件冒泡|
|`event.cancelable`|✅支持|QuickJS绑定|
|`event.composed`|⚠️可演进|Shadow DOM|
|`event.eventPhase`|✅支持|捕获/冒泡阶段|

### 4.5 CSSOM

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
|`CSS.supports()`|✅支持|特性检测|
|`matchMedia()`|✅支持|媒体查询|
|`MediaQueryList`|✅支持|QuickJS绑定|

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
|`ResizeObserver`|✅支持|布局监听|
|`MutationObserver`|✅支持|DOM变更监听|
|`PerformanceObserver`|⚠️可演进|性能监测|
|`Clipboard`|⚠️可演进|系统剪贴板|
|`Notification`|❌不支持|系统通知|
|`Geolocation`|❌不支持|系统定位|
|`History`|⚠️部分支持|简化路由|
|`Location`|⚠️部分支持|简化路由|
|`Navigator`|⚠️部分支持|基础属性|
|`Screen`|✅支持|SDL窗口信息|
|`Window`|⚠️部分支持|简化实现|

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

## 6. 参考文献

[1] WHATWG. HTML Living Standard. https://html.spec.whatwg.org/multipage/

[2] MDN Web Docs. CSS Reference. https://developer.mozilla.org/en-US/docs/Web/CSS/Reference

[3] WHATWG. DOM Living Standard. https://dom.spec.whatwg.org/