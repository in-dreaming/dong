# 自研UI框架 测试清单

## 1. 概述

本文档将需要支持的特性按功能模块组合为独立测试用例，分为 HTML 元素、CSS 属性、DOM API 三类。能力基准见 [css-subset.md](../reference/css-subset.md) 与 [features-index.md](../reference/features-index.md)。

### 1.1 测试用例统计

|类别|模块数|用例数|覆盖特性数|
|---|---|---|---|
|HTML元素测试|10|45|85+|
|CSS属性测试|12|62|150+|
|DOM API测试|8|48|120+|
|**合计**|**30**|**155**|**355+**|

---

## 2. HTML元素测试

### 2.1 文档结构与元数据

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-001|基础文档结构|`<html>`,`<head>`,`<body>`,lexbor解析|
|HTML-002|文档标题设置|`<title>`,窗口标题同步|
|HTML-003|样式表内联|`<style>`,lexbor CSS解析|
|HTML-004|外部样式表链接|`<link rel="stylesheet">`,资源加载|
|HTML-005|基础URL设置|`<base href>`,相对路径解析|
|HTML-006|视口元信息|`<meta charset>`,`<meta viewport>`|

### 2.2 内容分区元素

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-007|语义化页面布局|`<header>`,`<nav>`,`<main>`,`<aside>`,`<footer>`|
|HTML-008|文章与章节|`<article>`,`<section>`,容器嵌套|
|HTML-009|标题层级|`<h1>`-`<h6>`,默认样式|
|HTML-010|地址信息|`<address>`,作为容器|

### 2.3 文本内容元素

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-011|基础块级容器|`<div>`,`<p>`,块级布局|
|HTML-012|预格式化文本|`<pre>`,空白符保留|
|HTML-013|引用块|`<blockquote>`,块级容器|
|HTML-014|有序列表|`<ol>`,`<li>`,列表编号|
|HTML-015|无序列表|`<ul>`,`<li>`,列表符号|
|HTML-016|定义列表|`<dl>`,`<dt>`,`<dd>`,术语描述|
|HTML-017|图文组合|`<figure>`,`<figcaption>`|
|HTML-018|水平分割线|`<hr>`,渲染样式|

### 2.4 内联文本语义元素

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-019|超链接|`<a href>`,`<a target>`,点击跳转|
|HTML-020|内联容器|`<span>`,样式应用|
|HTML-021|换行控制|`<br>`,`<wbr>`,文本断行|
|HTML-022|强调文本|`<em>`,`<strong>`,`<mark>`|
|HTML-023|字体样式|`<b>`,`<i>`,`<u>`,`<s>`|
|HTML-024|上下标|`<sub>`,`<sup>`,位置偏移|
|HTML-025|等宽文本|`<code>`,`<kbd>`,`<samp>`,`<var>`|
|HTML-026|引用与缩写|`<q>`,`<cite>`,`<abbr title>`|
|HTML-027|时间与数据|`<time datetime>`,`<data value>`|
|HTML-028|小号文本|`<small>`,字号缩小|
|HTML-029|双向文本|`<bdo dir>`,方向覆盖|

### 2.5 图片与多媒体

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-030|图片加载与显示|`<img src>`,`<img alt>`,SDL GPU纹理|
|HTML-031|图片尺寸属性|`<img width>`,`<img height>`|
|HTML-032|图片容器|`<picture>`,`<source>`,基础解析|
|HTML-033|视频占位(可演进)|`<video>`,预留接口|
|HTML-034|音频占位(可演进)|`<audio>`,预留接口|

### 2.6 SVG与Canvas

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-035|Canvas 2D绑定|`<canvas>`,2D Context,SDL GPU映射|
|HTML-036|SVG基础形状|`<svg>`,rect/circle/path,SDL GPU渲染|

### 2.7 表格元素

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-037|基础表格结构|`<table>`,`<tr>`,`<td>`,自研布局|
|HTML-038|表格分组|`<thead>`,`<tbody>`,`<tfoot>`|
|HTML-039|表头单元格|`<th>`,默认样式|
|HTML-040|表格标题|`<caption>`,位置渲染|
|HTML-041|列定义|`<colgroup>`,`<col span>`|

### 2.8 表单元素

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-042|表单容器|`<form>`,`<fieldset>`,`<legend>`|
|HTML-043|标签关联|`<label for>`,点击聚焦|
|HTML-044|文本输入|`<input type="text">`,`<input type="password">`|
|HTML-045|数字输入|`<input type="number">`,数值验证|
|HTML-046|复选与单选|`<input type="checkbox">`,`<input type="radio">`|
|HTML-047|滑块控件|`<input type="range">`,拖拽交互|
|HTML-048|隐藏字段|`<input type="hidden">`|
|HTML-049|按钮元素|`<button>`,点击事件|
|HTML-050|下拉选择|`<select>`,`<option>`,`<optgroup>`|
|HTML-051|多行文本|`<textarea>`,输入处理|
|HTML-052|进度与度量|`<progress>`,`<meter>`,渲染样式|
|HTML-053|输出显示|`<output>`,内联容器|
|HTML-054|日期时间输入(可演进)|`<input type="date/time">`,预留接口|
|HTML-055|颜色选择(可演进)|`<input type="color">`,预留接口|
|HTML-056|文件选择(可演进)|`<input type="file">`,预留接口|

### 2.9 交互元素

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-057|折叠内容|`<details>`,`<summary>`,展开收起|
|HTML-058|模态对话框|`<dialog>`,显示隐藏|

### 2.10 脚本与模板

|编号|用例名称|覆盖特性|
|---|---|---|
|HTML-059|脚本执行|`<script>`,QuickJS执行|
|HTML-060|无脚本回退|`<noscript>`,条件渲染|
|HTML-061|模板克隆|`<template>`,内容克隆|
|HTML-062|插槽(可演进)|`<slot>`,Web Components预留|

---

## 3. CSS属性测试

### 3.1 盒模型

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-001|宽高设置|`width`,`height`,Yoga布局|
|CSS-002|最小最大尺寸|`min-width`,`max-width`,`min-height`,`max-height`|
|CSS-003|外边距|`margin`,`margin-top/right/bottom/left`|
|CSS-004|逻辑外边距|`margin-inline`,`margin-block`|
|CSS-005|内边距|`padding`,`padding-top/right/bottom/left`|
|CSS-006|逻辑内边距|`padding-inline`,`padding-block`|
|CSS-007|边框宽度|`border`,`border-width`|
|CSS-008|边框颜色|`border-color`,四边独立设置|
|CSS-009|边框样式|`border-style`:solid/dashed/dotted/none|
|CSS-010|圆角边框|`border-radius`,四角独立设置|
|CSS-011|盒模型模式|`box-sizing`:content-box/border-box|
|CSS-012|盒阴影|`box-shadow`,多阴影叠加|
|CSS-013|轮廓|`outline`,`outline-width/style/color/offset`|

### 3.2 Flexbox布局

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-014|Flex容器|`display:flex`,`display:inline-flex`|
|CSS-015|主轴方向|`flex-direction`:row/column/row-reverse/column-reverse|
|CSS-016|换行控制|`flex-wrap`:nowrap/wrap/wrap-reverse|
|CSS-017|复合流向|`flex-flow`,简写属性|
|CSS-018|主轴对齐|`justify-content`:flex-start/center/flex-end/space-between/space-around/space-evenly|
|CSS-019|交叉轴对齐|`align-items`:flex-start/center/flex-end/stretch/baseline|
|CSS-020|多行对齐|`align-content`,多行容器|
|CSS-021|弹性增长|`flex-grow`,空间分配|
|CSS-022|弹性收缩|`flex-shrink`,空间收缩|
|CSS-023|弹性基准|`flex-basis`,初始尺寸|
|CSS-024|弹性简写|`flex`,三合一简写|
|CSS-025|自身对齐|`align-self`,单项覆盖|
|CSS-026|排列顺序|`order`,项目排序|
|CSS-027|间隙设置|`gap`,`row-gap`,`column-gap`|

### 3.3 定位与显示

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-028|显示类型|`display`:block/inline/inline-block/none|
|CSS-029|相对定位|`position:relative`,`top/right/bottom/left`|
|CSS-030|绝对定位|`position:absolute`,脱离文档流|
|CSS-031|固定定位|`position:fixed`,视口定位|
|CSS-032|粘性定位|`position:sticky`,滚动吸附|
|CSS-033|定位简写|`inset`,四方向简写|
|CSS-034|层叠顺序|`z-index`,图层排序|
|CSS-035|浮动布局|`float`:left/right,`clear`|
|CSS-036|溢出处理|`overflow`,`overflow-x`,`overflow-y`|
|CSS-037|可见性|`visibility`:visible/hidden|
|CSS-038|裁剪路径|`clip-path`,基础形状|

### 3.4 颜色与背景

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-039|文本颜色|`color`,rgb/hsl/hex格式|
|CSS-040|背景颜色|`background-color`,透明度|
|CSS-041|背景图片|`background-image`,纹理加载|
|CSS-042|背景位置|`background-position`,关键字与数值|
|CSS-043|背景尺寸|`background-size`:cover/contain/值|
|CSS-044|背景重复|`background-repeat`:repeat/no-repeat/repeat-x/repeat-y|
|CSS-045|背景附着|`background-attachment`:scroll/local|
|CSS-046|背景裁剪|`background-clip`,`background-origin`|
|CSS-047|背景简写|`background`,复合属性|
|CSS-048|透明度|`opacity`,元素整体透明|
|CSS-049|混合模式|`background-blend-mode`,`mix-blend-mode`|

### 3.5 渐变

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-050|线性渐变|`linear-gradient()`,角度与色标|
|CSS-051|径向渐变|`radial-gradient()`,形状与位置|
|CSS-052|重复渐变|`repeating-linear-gradient()`,`repeating-radial-gradient()`|
|CSS-053|锥形渐变(可演进)|`conic-gradient()`,预留接口|

### 3.6 字体与文本

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-054|字体族|`font-family`,FreeType加载|
|CSS-055|字体大小|`font-size`,单位换算|
|CSS-056|字体粗细|`font-weight`,100-900/bold/normal|
|CSS-057|字体样式|`font-style`:normal/italic/oblique|
|CSS-058|字体变体|`font-variant`:small-caps|
|CSS-059|字体简写|`font`,复合属性|
|CSS-060|行高|`line-height`,数值与单位|
|CSS-061|字间距|`letter-spacing`,HarfBuzz调整|
|CSS-062|词间距|`word-spacing`,HarfBuzz调整|
|CSS-063|文本对齐|`text-align`:left/center/right/justify|
|CSS-064|末行对齐|`text-align-last`|
|CSS-065|首行缩进|`text-indent`,数值|
|CSS-066|文本装饰线|`text-decoration-line`:underline/overline/line-through|
|CSS-067|装饰颜色|`text-decoration-color`|
|CSS-068|装饰样式|`text-decoration-style`:solid/dashed/dotted|
|CSS-069|装饰粗细|`text-decoration-thickness`|
|CSS-070|文本装饰简写|`text-decoration`|
|CSS-071|文本变换|`text-transform`:uppercase/lowercase/capitalize|
|CSS-072|文本溢出|`text-overflow`:ellipsis/clip|
|CSS-073|文本阴影|`text-shadow`,多阴影|
|CSS-074|空白处理|`white-space`:normal/nowrap/pre/pre-wrap/pre-line|
|CSS-075|单词断行|`word-break`,`overflow-wrap`|
|CSS-076|垂直对齐|`vertical-align`:baseline/top/middle/bottom|
|CSS-077|文本方向|`direction`:ltr/rtl,HarfBuzz支持|
|CSS-078|双向隔离|`unicode-bidi`,基础支持|
|CSS-079|多行截断|`-webkit-line-clamp`,配合overflow|

### 3.7 变换

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-080|2D平移|`transform:translate()`,`translateX()`,`translateY()`|
|CSS-081|2D旋转|`transform:rotate()`,角度单位|
|CSS-082|2D缩放|`transform:scale()`,`scaleX()`,`scaleY()`|
|CSS-083|2D倾斜|`transform:skew()`,`skewX()`,`skewY()`|
|CSS-084|矩阵变换|`transform:matrix()`|
|CSS-085|3D变换|`transform:translate3d()`,`rotate3d()`,`scale3d()`|
|CSS-086|变换原点|`transform-origin`,九宫格定位|
|CSS-087|透视|`perspective`,`perspective-origin`|
|CSS-088|背面可见性|`backface-visibility`:visible/hidden|
|CSS-089|独立变换属性|`translate`,`rotate`,`scale`|

### 3.8 过渡与动画

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-090|过渡属性|`transition-property`,指定属性|
|CSS-091|过渡时长|`transition-duration`,时间单位|
|CSS-092|过渡函数|`transition-timing-function`:ease/linear/cubic-bezier|
|CSS-093|过渡延迟|`transition-delay`|
|CSS-094|过渡简写|`transition`,复合属性|
|CSS-095|关键帧定义|`@keyframes`,百分比关键帧|
|CSS-096|动画名称|`animation-name`,关键帧引用|
|CSS-097|动画时长|`animation-duration`|
|CSS-098|动画函数|`animation-timing-function`|
|CSS-099|动画延迟|`animation-delay`|
|CSS-100|动画次数|`animation-iteration-count`:数值/infinite|
|CSS-101|动画方向|`animation-direction`:normal/reverse/alternate|
|CSS-102|动画填充|`animation-fill-mode`:forwards/backwards/both|
|CSS-103|动画状态|`animation-play-state`:running/paused|
|CSS-104|动画简写|`animation`,复合属性|

### 3.9 滤镜与特效

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-105|模糊滤镜|`filter:blur()`|
|CSS-106|亮度滤镜|`filter:brightness()`|
|CSS-107|对比度滤镜|`filter:contrast()`|
|CSS-108|灰度滤镜|`filter:grayscale()`|
|CSS-109|饱和度滤镜|`filter:saturate()`|
|CSS-110|褐色滤镜|`filter:sepia()`|
|CSS-111|色相旋转|`filter:hue-rotate()`|
|CSS-112|反色滤镜|`filter:invert()`|
|CSS-113|透明度滤镜|`filter:opacity()`|
|CSS-114|投影滤镜|`filter:drop-shadow()`|
|CSS-115|多滤镜组合|`filter`,多值叠加|
|CSS-116|背景滤镜|`backdrop-filter`,毛玻璃效果|

### 3.10 光标与交互

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-117|光标样式|`cursor`:pointer/default/text/move等|
|CSS-118|事件穿透|`pointer-events`:auto/none|
|CSS-119|文本选择|`user-select`:none/text/all|
|CSS-120|触摸行为|`touch-action`,基础手势|
|CSS-121|光标颜色|`caret-color`,输入框光标|

### 3.11 选择器

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-122|基础选择器|`*`,`element`,`.class`,`#id`|
|CSS-123|属性选择器|`[attr]`,`[attr=value]`,`[attr^=]`,`[attr$=]`,`[attr*=]`|
|CSS-124|组合选择器|后代`A B`,子`A>B`,相邻`A+B`,通用兄弟`A~B`|
|CSS-125|交互伪类|`:hover`,`:active`,`:focus`,`:focus-visible`,`:focus-within`|
|CSS-126|表单伪类|`:checked`,`:disabled`,`:enabled`,`:required`,`:optional`|
|CSS-127|验证伪类|`:valid`,`:invalid`,`:placeholder-shown`|
|CSS-128|结构伪类-子元素|`:first-child`,`:last-child`,`:only-child`|
|CSS-129|结构伪类-nth|`:nth-child()`,`:nth-last-child()`|
|CSS-130|结构伪类-类型|`:first-of-type`,`:last-of-type`,`:nth-of-type()`,`:only-of-type`|
|CSS-131|逻辑伪类|`:not()`,`:is()`,`:where()`,`:empty`|
|CSS-132|关系伪类|`:has()`,性能敏感场景|
|CSS-133|伪元素|`::before`,`::after`,content属性|
|CSS-134|文本伪元素|`::first-line`,`::first-letter`|
|CSS-135|表单伪元素|`::placeholder`,`::selection`|

### 3.12 At规则与函数

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-136|样式导入|`@import`,外部样式表|
|CSS-137|媒体查询|`@media`,屏幕尺寸/特性查询|
|CSS-138|字体定义|`@font-face`,FreeType加载|
|CSS-139|特性检测|`@supports`,条件规则|
|CSS-140|CSS变量|`var()`,自定义属性|
|CSS-141|计算函数|`calc()`,四则运算|
|CSS-142|比较函数|`min()`,`max()`,`clamp()`|
|CSS-143|颜色函数|`rgb()`,`rgba()`,`hsl()`,`hsla()`|
|CSS-144|资源函数|`url()`,资源引用|
|CSS-145|属性函数|`attr()`,基础属性|
|CSS-146|环境函数|`env()`,safe-area-inset|

### 3.13 CSS单位

|编号|用例名称|覆盖特性|
|---|---|---|
|CSS-147|绝对单位|`px`,`cm`,`mm`,`in`,`pt`,`pc`|
|CSS-148|相对单位-字体|`em`,`rem`,`ch`|
|CSS-149|相对单位-视口|`vw`,`vh`,`vmin`,`vmax`|
|CSS-150|百分比单位|`%`,相对父元素|
|CSS-151|角度单位|`deg`,`rad`,`turn`|
|CSS-152|时间单位|`s`,`ms`|

---

## 4. DOM API测试

### 4.1 Node接口

|编号|用例名称|覆盖特性|
|---|---|---|
|DOM-001|节点类型属性|`nodeType`,`nodeName`,`nodeValue`|
|DOM-002|文本内容|`textContent`,获取与设置|
|DOM-003|父节点访问|`parentNode`,`parentElement`|
|DOM-004|子节点访问|`childNodes`,`firstChild`,`lastChild`|
|DOM-005|兄弟节点访问|`previousSibling`,`nextSibling`|
|DOM-006|所属文档|`ownerDocument`|
|DOM-007|节点添加|`appendChild()`,`insertBefore()`|
|DOM-008|节点移除|`removeChild()`|
|DOM-009|节点替换|`replaceChild()`|
|DOM-010|节点克隆|`cloneNode()`,深浅克隆|
|DOM-011|节点关系|`contains()`,`hasChildNodes()`|
|DOM-012|节点比较|`isEqualNode()`,`isSameNode()`,`compareDocumentPosition()`|
|DOM-013|节点规范化|`normalize()`,合并文本节点|

### 4.2 Element接口

|编号|用例名称|覆盖特性|
|---|---|---|
|DOM-014|元素标识|`tagName`,`id`,`className`|
|DOM-015|类列表操作|`classList`:add/remove/toggle/contains|
|DOM-016|属性集合|`attributes`,NamedNodeMap|
|DOM-017|属性操作|`getAttribute()`,`setAttribute()`,`removeAttribute()`|
|DOM-018|属性检测|`hasAttribute()`,`toggleAttribute()`,`getAttributeNames()`|
|DOM-019|HTML内容|`innerHTML`,`outerHTML`,lexbor解析|
|DOM-020|子元素访问|`children`,`firstElementChild`,`lastElementChild`|
|DOM-021|兄弟元素访问|`previousElementSibling`,`nextElementSibling`|
|DOM-022|子元素计数|`childElementCount`|
|DOM-023|选择器查询-单个|`querySelector()`,lexbor选择器|
|DOM-024|选择器查询-多个|`querySelectorAll()`,NodeList|
|DOM-025|选择器匹配|`matches()`,`closest()`|
|DOM-026|标签名查询|`getElementsByTagName()`|
|DOM-027|类名查询|`getElementsByClassName()`|
|DOM-028|布局信息-边界|`getBoundingClientRect()`,`getClientRects()`|
|DOM-029|布局信息-滚动|`scrollTop`,`scrollLeft`,`scrollWidth`,`scrollHeight`|
|DOM-030|布局信息-客户端|`clientTop`,`clientLeft`,`clientWidth`,`clientHeight`|
|DOM-031|布局信息-偏移|`offsetTop`,`offsetLeft`,`offsetWidth`,`offsetHeight`,`offsetParent`|
|DOM-032|滚动操作|`scroll()`,`scrollTo()`,`scrollBy()`,`scrollIntoView()`|
|DOM-033|插入操作|`insertAdjacentHTML()`,`insertAdjacentElement()`,`insertAdjacentText()`|
|DOM-034|DOM操作方法|`remove()`,`before()`,`after()`,`replaceWith()`|
|DOM-035|内容操作|`prepend()`,`append()`|
|DOM-036|焦点操作|`focus()`,`blur()`|
|DOM-037|点击触发|`click()`,事件模拟|

### 4.3 Document接口

|编号|用例名称|覆盖特性|
|---|---|---|
|DOM-038|文档元素|`documentElement`,`head`,`body`|
|DOM-039|文档标题|`title`,读写|
|DOM-040|活动元素|`activeElement`,焦点管理|
|DOM-041|ID查询|`getElementById()`|
|DOM-042|名称查询|`getElementsByName()`|
|DOM-043|元素创建|`createElement()`,`createTextNode()`|
|DOM-044|注释与片段|`createComment()`,`createDocumentFragment()`|
|DOM-045|属性创建|`createAttribute()`|
|DOM-046|节点导入|`importNode()`,`adoptNode()`|
|DOM-047|树遍历器|`createTreeWalker()`,`createNodeIterator()`|
|DOM-048|命中测试|`elementFromPoint()`,`elementsFromPoint()`|
|DOM-049|焦点检测|`hasFocus()`|
|DOM-050|事件创建|`createEvent()`,基础事件类型|

### 4.4 事件系统

|编号|用例名称|覆盖特性|
|---|---|---|
|DOM-051|事件监听|`addEventListener()`,`removeEventListener()`|
|DOM-052|事件派发|`dispatchEvent()`|
|DOM-053|事件对象|`Event`,`type`,`target`,`currentTarget`|
|DOM-054|自定义事件|`CustomEvent`,detail数据|
|DOM-055|鼠标事件|`MouseEvent`,clientX/Y,button,buttons|
|DOM-056|键盘事件|`KeyboardEvent`,key,code,keyCode|
|DOM-057|滚轮事件|`WheelEvent`,deltaX/Y/Z|
|DOM-058|指针事件|`PointerEvent`,pointerId,pointerType|
|DOM-059|触摸事件|`TouchEvent`,touches,targetTouches|
|DOM-060|焦点事件|`FocusEvent`,relatedTarget|
|DOM-061|输入事件|`InputEvent`,data,inputType|
|DOM-062|动画事件|`AnimationEvent`,animationName,elapsedTime|
|DOM-063|过渡事件|`TransitionEvent`,propertyName,elapsedTime|
|DOM-064|事件传播控制|`preventDefault()`,`stopPropagation()`,`stopImmediatePropagation()`|
|DOM-065|事件阶段|`eventPhase`,`bubbles`,`cancelable`|
|DOM-066|捕获与冒泡|capture选项,事件流|

### 4.5 CSSOM

|编号|用例名称|覆盖特性|
|---|---|---|
|DOM-067|内联样式|`element.style`,CSSStyleDeclaration|
|DOM-068|计算样式|`getComputedStyle()`,样式计算|
|DOM-069|样式表集合|`document.styleSheets`,StyleSheetList|
|DOM-070|样式表操作|`CSSStyleSheet`,`cssRules`|
|DOM-071|规则类型|`CSSStyleRule`,`CSSMediaRule`,`CSSKeyframesRule`|
|DOM-072|规则操作|`insertRule()`,`deleteRule()`|
|DOM-073|特性检测|`CSS.supports()`|
|DOM-074|媒体查询|`matchMedia()`,`MediaQueryList`|

### 4.6 定时器与帧同步

|编号|用例名称|覆盖特性|
|---|---|---|
|DOM-075|延时执行|`setTimeout()`,`clearTimeout()`|
|DOM-076|循环执行|`setInterval()`,`clearInterval()`|
|DOM-077|帧同步|`requestAnimationFrame()`,`cancelAnimationFrame()`|
|DOM-078|微任务|`queueMicrotask()`|
|DOM-079|空闲回调(可演进)|`requestIdleCallback()`,`cancelIdleCallback()`|

### 4.7 存储与数据

|编号|用例名称|覆盖特性|
|---|---|---|
|DOM-080|本地存储|`localStorage`,文件持久化|
|DOM-081|会话存储|`sessionStorage`,内存存储|
|DOM-082|存储操作|`getItem()`,`setItem()`,`removeItem()`,`clear()`|
|DOM-083|存储遍历|`key()`,`length`|

### 4.8 工具API

|编号|用例名称|覆盖特性|
|---|---|---|
|DOM-084|控制台|`console`:log/warn/error/info/debug|
|DOM-085|JSON处理|`JSON`:parse/stringify,QuickJS内置|
|DOM-086|Promise|`Promise`,async/await,QuickJS内置|
|DOM-087|URL解析|`URL`,`URLSearchParams`|
|DOM-088|文本编码|`TextEncoder`,`TextDecoder`|
|DOM-089|Base64|`atob()`,`btoa()`|
|DOM-090|中断控制|`AbortController`,`AbortSignal`|
|DOM-091|尺寸观察|`ResizeObserver`,布局变化|
|DOM-092|DOM观察|`MutationObserver`,DOM变更|
|DOM-093|屏幕信息|`Screen`,SDL窗口信息|
|DOM-094|导航信息|`Navigator`,基础属性|
|DOM-095|历史记录|`History`,简化路由|
|DOM-096|位置信息|`Location`,简化路由|

### 4.9 可演进API

|编号|用例名称|覆盖特性|
|---|---|---|
|DOM-097|Fetch接口预留|`fetch()`,需外部HTTP库|
|DOM-098|XHR接口预留|`XMLHttpRequest`,需外部HTTP库|
|DOM-099|WebSocket预留|`WebSocket`,需外部库|
|DOM-100|文件API预留|`Blob`,`File`,`FileReader`|
|DOM-101|表单数据预留|`FormData`|
|DOM-102|可见性观察预留|`IntersectionObserver`|
|DOM-103|剪贴板预留|`Clipboard`,系统API|
|DOM-104|拖拽事件预留|`DragEvent`,拖拽系统|
|DOM-105|输入法事件预留|`CompositionEvent`,IME输入|

---

## 5. 综合场景测试

### 5.1 跨模块集成测试

|编号|用例名称|覆盖特性|
|---|---|---|
|INT-001|响应式布局|`@media`,Flexbox,viewport单位|
|INT-002|表单交互完整流程|表单元素+`:focus`+`InputEvent`+样式变化|
|INT-003|动画与变换组合|`@keyframes`+`transform`+`transition`|
|INT-004|滚动容器|`overflow:auto`+`scroll()`+滚动事件|
|INT-005|弹窗交互|`<dialog>`+`z-index`+焦点管理|
|INT-006|列表渲染|`<ul>`+`<li>`+DOM创建+样式应用|
|INT-007|图文混排|`<img>`+文本元素+Flexbox布局|
|INT-008|主题切换|CSS变量+DOM样式修改+过渡效果|
|INT-009|拖拽滑块|`<input type="range">`+指针事件+样式反馈|
|INT-010|折叠面板|`<details>`+`<summary>`+动画过渡|

---

## 6. 参考文献

[1] 见 [features-index.md](../reference/features-index.md) 与 [css-subset.md](../reference/css-subset.md)