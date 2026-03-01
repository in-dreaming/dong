注意
- 完成的任务需要标注完成。
- 需要在examples/data/tests下添加测试用例。使用dong/scripts/tools/run_baseline_compare.py进行基准对比验证feature实现是否正确。


# Dong Engine - Web标准对齐 TODO

基于 gap analysis 整理的待办事项，按优先级分组。

定位校准：本引擎更偏**游戏/UI 导向的 Web-like 子集**，优先级以 ROI 为导向——先做会影响主流 UI 框架的 **layout / paint 正确性、表单可用性、滚动/定位、伪元素管线、IME**；高成本低收益/浏览器长尾特性集中放到“暂缓/冻结”。


## P2 - 健康推进（中 ROI / 影响高级场景）

### CSS 属性

- [x] CSS 逻辑属性（`margin-inline-start`, `padding-block-end`, `border-inline` 等）
- [x] `color-scheme` 属性（`light`/`dark`）
- [x] `hyphens` 属性
- [x] `counter-reset` / `counter-increment` + `counter()`/`counters()` 函数
- [x] `quotes` 属性
- [x] `image-rendering` 属性（`pixelated`/`crisp-edges`）
- [x] `resize` 属性（textarea 拖拽调整大小）
- [x] `caption-side` 属性
- [x] `will-change` 属性（建议引擎自动 layer promotion；可 P2 后段再评估）
- [x] `accent-color` 属性（表单控件主题色；可 P2 后段）


### CSS 值/解析

- [x] `visibility: collapse` 值 - 已实现 CSS 值解析和渲染管线处理：
  - **表格元素**（tr, thead, tbody, tfoot, col, colgroup）：`visibility: collapse` 行为等同于 `display: none`（隐藏但不占用空间）
  - **非表格元素**：`visibility: collapse` 行为等同于 `visibility: hidden`（不可见但仍占位）
  - **实现位置**：`src/render/painter.cpp:413-421` - 根据元素类型判断 collapse 行为
  - **测试用例**：`test_visibility_collapse_non_table.html`, `test_visibility_collapse_table_row.html`, `test_visibility_collapse_table_tbody.html`
- [x] `flex-basis: content` 关键字 - 已实现完整的 flex-basis: content 关键字支持：
  - **CSS解析**：在 `src/dom/css/css_parser.cpp` 中的 `parseValue()` 函数添加了对 'content' 关键字的识别，将其解析为 `CSSValue::Unit::CONTENT`
  - **CSSValue扩展**：在 `src/dom/css/css_value.hpp` 中添加了 `CONTENT` 单元类型和 `isContent()` 辅助方法
  - **Yoga映射**：在 `src/dom/css/style_engine.cpp` 中将 `flex-basis: content` 映射为 `auto` 传递给 Yoga，让 Yoga 使用内容尺寸计算
  - **语义**：`flex-basis: content` 表示 flex item 的基础尺寸由其内容决定（等价于 max-content 或 fit-content）
  - **测试用例**：`test_flex_basis_content.html`（包含4个测试场景：基础用法、与auto对比、小内容、带max-width约束）
- [x] `white-space: pre-line` / `break-spaces` 文本布局器正确处理
- [x] `text-align: justify` 实际两端对齐实现 - 已实现完整的单词间距均匀分配算法，支持多行文本的最后一行左对齐规则。实现位置：`src/render/painter/painter_text.cpp` 中的 `emitFullTextLine` 函数。测试用例：`test_text_align_justify.html`
- [x] `background-position` 四值语法 - 已实现完整的 1值、2值、3值、4值语法支持：
  - **1值语法**：`center` → `50% 50%`, `left` → `0% 50%`, `right` → `100% 50%`, `top` → `50% 0%`, `bottom` → `50% 100%`
  - **2值语法**：`left top` → `0% 0%`, `right bottom` → `100% 100%`, `center center` → `50% 50%`
  - **3值语法**：`right 10px top` → `calc(100% - 10px) 0%`, `left 20px bottom` → `20px 100%`
  - **4值语法**：`right 10px bottom 20px` → `calc(100% - 10px) calc(100% - 20px)`
  - **实现位置**：`src/dom/css/css_parser.cpp` 中的 `parseBackgroundPosition()` 函数，`src/render/painter.cpp` 中的渲染支持
  - **测试用例**：`test_background_position_four_value.html`
- [x] `font-weight` 数值 100-900 到 FreeType 选字映射验证 - 已实现完整的 font-weight 数值 100-900 到 FreeType 字体选择映射：
  - **CSS解析**：`font-weight` 属性支持数值 100-900（Thin=100, ExtraLight=200, Light=300, Regular=400, Medium=500, SemiBold=600, Bold=700, ExtraBold=800, Black=900）和关键词 normal=400, bold=700, bolder=700, lighter=300
  - **值规范化**：非100倍数的值自动规范化到最近的100倍数（e.g., 450→500, 349→300），超出100-900范围的值被钳制
  - **FreeType选字**：通过 `normalizeFontWeight()` 函数解析数值，`findClosestWeightFont()` 函数基于 FreeType OS/2 表的 usWeightClass 读取实际字体权重，选择最接近的变体
  - **实现位置**：`src/render/font_resolver.cpp:236-270` (normalizeFontWeight), `src/render/font_resolver.cpp:729-856` (getFontFileWeight, findClosestWeightFont), `src/render/font_resolver.cpp:280-387` (appendWeightedCandidatesForFamily)
  - **测试用例**：`test_font_weight_100_900.html`, `test_font_weight_closest_match.html`, `test_font_weight_mapping.html`

### CSS 简写属性

- [x] `place-items` / `place-content` / `place-self` 简写 - 已实现完整的简写属性解析，支持单值（同时设置align和justify）和双值（第一个值align，第二个值justify）格式。测试用例：`test_place_shorthand.html`
- [x] `margin-inline` / `margin-block` / `padding-inline` / `padding-block` 简写 - 已实现逻辑盒模型简写属性解析和映射，基于 horizontal-tb 和 ltr 固定映射。实现见 `css_parser.cpp` 中的 helper 函数。
- [x] `border-inline` / `border-block` 简写 - 已实现逻辑边框简写属性解析。
- [x] `inset-block` / `inset-inline` 简写 - 已实现逻辑定位边距简写属性解析。
- [x] `list-style` 简写 - 已实现，支持 type/position/image 组合解析，测试用例：`test_list_style_shorthand.html`

### CSS 伪类/伪元素

- [x] `:open` / `:closed` 伪类（`<details>` 需要；已实现）
- [x] `:target` 伪类 - 已实现URL片段标识符匹配，检查元素ID是否匹配location.hash。实现位置：`src/dom/css/selector_matcher.cpp` 中的 `matchesPseudoClass` 函数。测试用例：`test_pseudo_classes_new.html`
- [x] `:any-link` 伪类 - 已实现匹配任何有href属性的<a>, <area>, <link>元素。实现位置：`src/dom/css/selector_matcher.cpp` 中的 `matchesPseudoClass` 函数。测试用例：`test_pseudo_classes_new.html`
- [x] `:indeterminate` 伪类 - 已实现匹配不确定状态的表单元素（checkbox的indeterminate属性为true，radio group中没有选中项）。实现位置：`src/dom/css/selector_matcher.cpp` 中的 `matchesPseudoClass` 函数。测试用例：`test_pseudo_classes_new.html`
- [x] `:nth-last-of-type()` 伪类 - 已实现从后往前匹配同类型兄弟元素的第An+B个。实现位置：`src/dom/css/selector_matcher.cpp` 中的 `matchesNthLastOfType` 函数。测试用例：`test_pseudo_classes_new.html`
- [x] `::selection` 渲染实现

### CSS 函数

- [x] `color-mix()` 函数（颜色混合，Baseline 2023）- 已实现完整的 color-mix(in srgb, <color> <percentage>?, <color> <percentage>?) 语法：
  - **支持的颜色格式**：hex (#rgb/#rrggbb/#rrggbbaa), rgb()/rgba(), 命名颜色
  - **混合算法**：sRGB 空间线性插值，支持 alpha 通道预乘/逆预乘混合
  - **百分比逻辑**：默认各 50%，支持单百分比（另一个为 100%-x）或双百分比（自动归一化）
  - **实现位置**：`painter_style_utils.hpp` 中的 `parseCssColor()` 函数添加 color-mix 分支
  - **测试用例**：`test_color_mix.html`
- [x] `oklch()` / `oklab()` 颜色函数 - 已实现完整的 OKLab 和 OKLCH 颜色函数支持：
  - **oklab(L a b / alpha)**: L 范围 0-1, a/b 范围约 -0.4 到 0.4, alpha 可选（支持小数和百分比）
  - **oklch(L C H / alpha)**: L 范围 0-1, C 范围 0-0.4, H 范围 0-360度（支持负值和>360的自动归一化）, alpha 可选
  - **颜色空间转换**: OKLab → XYZ D65 → Linear sRGB → sRGB (Gamma校正) 的完整转换链
  - **CSS解析支持**: 在 `painter_style_utils.hpp` 中添加颜色解析，在 `css_parser.cpp` 中识别为有效颜色格式
  - **实现位置**: `src/render/painter/painter_style_utils.hpp` (颜色转换函数 + 解析逻辑), `src/dom/css/css_parser.cpp:2005` (parseColor识别)
  - **测试用例**: `test_oklab_color.html`, `test_oklch_color.html`
- [x] `light-dark()` 函数
- [x] `env()` 函数（`safe-area-inset-*`）
- [x] `counter()` / `counters()` 函数 - 已实现完整的CSS计数器函数支持：
  - **counter(name, style?)**：输出当前作用域计数器值，支持decimal, lower-alpha, upper-alpha, lower-roman, upper-roman样式
  - **counters(name, separator, style?)**：输出嵌套计数器值链，用指定分隔符连接
  - **作用域管理**：counter-reset创建新作用域，counter-increment修改当前作用域
  - **实现位置**：`src/dom/css/css_parser.cpp`中的parseCounterDirectiveList和parseContentTokens函数，`src/render/painter.cpp`中的pushCounterScope/popCounterScope/evaluateCounterText/evaluateCountersText函数
  - **测试用例**：`test_counter_basic.html`, `test_counter_reset_increment.html`, `test_counter_styles.html`, `test_counters_nested.html`

### CSS At规则

- [x] `@layer` 级联层 - 已实现完整的@layer语法解析和级联优先级系统：
  - **语法支持**：`@layer name { ... }`（命名层）、`@layer { ... }`（匿名层）、`@layer name;`（预声明层）、`@layer name1, name2;`（多层级联声明）
  - **优先级规则**：未分层样式 > 后声明的层 > 先声明的层
  - **实现位置**：`src/dom/css/css_parser.cpp` 中的 `parseLayerRules()` 函数，`src/dom/css/style_engine.cpp` 中的 `sortRulesWithLayerPriority()` 函数
  - **测试用例**：`test_css_layer_basic.html`、`test_css_layer_priority.html`、`test_css_layer_anonymous.html`、`test_css_layer_nested.html`、`test_css_layer_specificity.html`

### HTML

- [x] `tabindex` 行为完善（焦点顺序、`-1` 可编程聚焦）- 已实现完整的tabindex属性支持：
  - **tabindex="0"**：元素可通过Tab键聚焦，按DOM顺序参与Tab序列
  - **tabindex="-1"**：元素不参与Tab序列，但可通过JavaScript focus()编程聚焦
  - **tabindex正整数**：按数值升序优先于tabindex="0"的元素
  - **实现位置**：`src/dom/focus_manager.cpp`中的`moveFocus()`函数实现Tab键导航逻辑，`src/script/js_node_bindings.cpp`中的`elem_getTabIndex`/`elem_setTabIndex`实现JavaScript API
  - **测试用例**：`test_tabindex.html`
- [x] `<details>`/`<summary>` 完整交互：点击 toggle、`open` 属性切换、disclosure triangle（已实现）
- [x] HTMLDetailsElement: `open` 属性（已实现）
- [x] `lang` 属性传递给 HarfBuzz
  - **实现位置**：`src/dom/dom/dom_node.cpp:597-615` - `getEffectiveLang()` 方法实现语言属性继承
  - **实现位置**：`src/render/text_shaper.cpp:29-40` - `setHbLanguage()` 函数设置 HarfBuzz 语言
  - 实现位置：`src/render/text_shaper.cpp:365-398` - `shapeSpan()` 函数接收并传递语言参数
  - 实现位置：`src/render/painter/painter_text.cpp` - 多个 TextShapeRequest 创建点设置 lang 字段
  - 测试用例：`test_lang_attribute.html`, `test_lang_turkish_i_dot.html`
  - 功能说明：HTML lang 属性现在正确传递给 HarfBuzz 文本塑形器，影响语言特定的字形选择（如土耳其语的 i/İ 区分）
- [x] `dir` 属性映射到 CSS `direction` + `:dir()` 伪类
  - **实现位置**：`src/dom/dom/dom_node.cpp:745-778` - `getEffectiveDirection()` 方法实现 dir 属性解析和继承
  - **实现位置**：`src/dom/css/selector_matcher.cpp:526-532` - `:dir()` 伪类选择器实现
  - **实现位置**：`src/dom/dom/dom_node.cpp:617-743` - `detectTextDirection()` 辅助函数检测文本方向（Unicode bidi 字符检测）
  - **实现位置**：`src/dom/css/style_engine.cpp` - dir 属性到 CSS direction 属性的映射
  - **功能说明**：支持 `dir="ltr"`, `dir="rtl"`, `dir="auto"` 属性，正确继承父元素方向，`dir="auto"` 根据文本内容的第一个强方向字符决定方向
  - **测试用例**：`test_dir_attribute.html`, `test_dir_pseudo_class.html`
- [x] `title` 属性 tooltip 渲染
- [x] `pattern` 正则验证
  - **实现位置**：`src/dom/input_element.hpp/cpp` - 添加`setPattern()`, `getPattern()`, `hasPattern()`, `matchesPattern()`方法
  - **实现位置**：`src/script/js_node_bindings.cpp` - 更新`input_checkValidity()`和`checkControlValidity()`函数
  - **功能说明**：支持正则表达式验证，匹配HTML5 pattern属性语义
  - **测试用例**：`test_form_validation_pattern.html`

- [x] `min` / `max` / `step` 范围验证
  - **实现位置**：`src/dom/input_element.hpp/cpp` - 添加`setMin()`, `setMax()`, `setStep()`, `isInRange()`方法
  - **实现位置**：`src/script/js_node_bindings.cpp` - 更新验证逻辑支持数值范围检查
  - **功能说明**：支持number、range、date、time等input类型的范围验证
  - **测试用例**：`test_form_validation_range.html`

- [x] `:invalid` / `:valid` 伪类支持
  - **实现位置**：`src/dom/css/selector_matcher.cpp` - 更新`isElementValid()`函数
  - **功能说明**：根据表单验证状态自动应用`:invalid`和`:valid`伪类样式
  - **测试用例**：`test_form_validation_comprehensive.html`
- [x] `<optgroup>` 视觉分组
- [x] `<textarea>` `rows`/`cols` 默认尺寸映射
  - **实现位置**：`src/layout/layout_engine.cpp` - 添加`computeAverageCharWidth()`函数和textarea处理逻辑
  - **功能说明**：
    - `rows`属性决定默认高度（行数 × 行高），默认值为2
    - `cols`属性决定默认宽度（列数 × 0.5em），默认值为20
    - 只在没有显式CSS width/height时使用rows/cols计算尺寸
  - **测试用例**：`test_textarea_rows_cols.html`, `test_textarea_rows_cols_override.html`
- [x] `<a href="#id">` 锚点导航
  - **实现位置**：`src/core/engine_view.cpp:1810-1823` - handle hash link clicks in sendMouseButton()
  - **功能说明**：
    - 点击 href 以 # 开头的链接时，滚动到对应 id 的元素位置
    - 使用 scrollIntoView() 方法将目标元素滚动到视口
    - 更新内部的 fragment/hash 状态（用于 :target 伪类匹配）
  - **测试用例**：`test_anchor_navigation.html`
- [x] `<img loading="lazy">` 延迟加载
- [x] `<img>` alt 文本渲染
- [x] `name` + FormData 序列化
  - **实现说明**：FormData序列化功能已完整实现，支持所有表单控件（input, select, textarea）的name属性序列化
  - **功能特性**：
    - 支持disabled控件过滤（disabled控件不参与序列化）
    - 支持checkbox/radio特殊序列化规则（unchecked不序列化，checkbox无value时默认"on"）
    - 支持select元素选项值序列化
    - 支持textarea文本内容序列化
  - **测试用例**：`test_formdata_name_serialization.html`（包含12个全面测试场景）
  - **实现位置**：`src/script/js_bindings.cpp`中的`serializeFormControl`函数

### JS DOM 绑定

- [x] Element: `childElementCount`, `replaceChildren()`, `insertAdjacentElement()`/`insertAdjacentText()`
  - **实现位置**: `src/script/js_node_bindings.cpp:227-232` (childElementCount), `:313-330` (replaceChildren), `:332-367` (insertAdjacentElement/insertAdjacentText)
  - **测试用例**: `test_js_dom_bindings_p2.html`, `test_dom_node_interface.html`, `test_element_click_and_capture.html`
- [x] Element: `setPointerCapture()`/`releasePointerCapture()`, `click()`
  - **实现位置**: `src/script/js_node_bindings.cpp:369-391` (setPointerCapture/releasePointerCapture/click), `src/dom/dom/dom_node.cpp` (DOMNode::click方法)
  - **测试用例**: `test_element_click_and_capture.html`
- [x] HTMLFormElement: `elements`, `submit()`/`reset()`, `checkValidity()`
- [x] HTMLInputElement: `select()`, `setSelectionRange()`, `selectionStart`/`selectionEnd`, `checkValidity()`/`setCustomValidity()`
- [x] HTMLTextAreaElement: `disabled`/`readOnly`, `selectionStart`/`selectionEnd`
- [x] Document: `elementFromPoint()`, `hasFocus()`, `scrollingElement`

### CSSOM


- [x] `CSSStyleDeclaration.cssText` - 已实现完整的cssText属性支持：
  - **getter**: 返回元素内联样式的序列化字符串（property:value;格式）
  - **setter**: 解析并设置内联样式，支持清除样式（空字符串）
  - **实现位置**: `src/script/js_bindings.cpp:1261-1396` - cssText getter/setter函数
  - **测试用例**: `test_csstext_cssom.html`
- [x] `CSS.supports()` JS 绑定 - 已实现完整的CSS.supports()方法：
  - **两种重载**: `CSS.supports(property, value)` 和 `CSS.supports(conditionText)`
  - **属性支持检查**: 基于引擎已知支持的CSS属性列表返回true/false
  - **条件语法解析**: 支持`(property: value)`格式的条件文本解析
  - **实现位置**: `src/script/js_bindings.cpp:2871-2965` - css_supports函数
  - **测试用例**: `test_cssom_supports.html`
- [x] `window.matchMedia()` JS 绑定 - 已实现完整的matchMedia()方法：
  - **MediaQueryList对象**: 返回包含`media`、`matches`、`onchange`属性的对象
  - **媒体查询支持**: 支持`prefers-color-scheme`、`prefers-reduced-motion`、`orientation`、`min-width/max-width`、`min-height/max-height`
  - **事件监听器**: 提供`addEventListener`/`removeEventListener`方法占位符
  - **实现位置**: `src/script/js_bindings.cpp:2967-3105` - window_matchMedia函数
  - **测试用例**: `test_cssom_matchmedia.html`

### 事件

- [x] `toggle` 事件（`<details>`）（已实现）
- [x] `copy` / `cut` / `paste` 事件
- [x] `beforeinput` 事件
- [x] MouseEvent: `pageX`/`pageY`, `movementX`/`movementY`, `relatedTarget`
- [x] KeyboardEvent: `repeat`
- [x] Event: `isTrusted`

### Web API


- [x] `structuredClone()` - 已实现深度克隆函数，支持原始值、数组、嵌套对象：
  - **实现位置**：`src/script/js_bindings.cpp:137-210` - structuredClone_impl函数
  - **全局注册**：`src/script/js_bindings.cpp:2629-2631` - 注册为全局函数
  - **测试用例**：`test_structuredclone.html`
  - **功能说明**：支持null、boolean、number、string、array、object的深度克隆，不支持函数、Map/Set/ArrayBuffer
- [x] `FormData` - 已实现表单数据接口，支持完整的键值对操作：
  - **实现位置**：`src/script/js_bindings.cpp:244-468` - FormDataStorage结构+构造器+方法
  - **全局注册**：`src/script/js_bindings.cpp:2637-2639` - 注册为构造函数
  - **测试用例**：`test_formdata_api.html`
  - **功能说明**：支持append/get/getAll/has/delete/set/entries/keys/values方法，支持同名多值存储
- [x] `DOMParser` - 已实现HTML字符串解析接口：
  - **实现位置**：`src/script/js_bindings.cpp:470-557` - DOMParser构造器+parseFromString方法
  - **全局注册**：`src/script/js_bindings.cpp:2641-2643` - 注册为构造函数
  - **测试用例**：`test_domparser_api.html`
  - **功能说明**：支持text/html MIME类型，返回包含body属性的Document对象
- [x] `DOMRect` 类型 - 已实现矩形几何接口：
  - **实现位置**：`src/script/js_bindings.cpp:212-242` - DOMRect构造器+computed properties
  - **全局注册**：`src/script/js_bindings.cpp:2633-2635` - 注册为构造函数
  - **测试用例**：`test_domrect_api.html`
  - **功能说明**：构造函数接受(x,y,width,height)，提供只读的top/right/bottom/left计算属性
  - **getBoundingClientRect**：已在Element绑定中实现，返回DOMRect实例


### D&D

- [x] `dragstart` / `drag` / `dragend` / `drop` 事件。 不实现完整规范。具体见文档 doc/todo_drag.md

---

## P3 - 低优先级（可长期排队）

- [ ] `inputmode` 属性
- [ ] `:default` / `:in-range` / `:out-of-range` / `:lang()` 伪类
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
