# P0-8 — `gap_analysis.md` 真·P0 清账

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 0 P0-8
> 来源清单：[`doc/specific/html_css_dom_gap_analysis.md`](../specific/html_css_dom_gap_analysis.md)
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

`gap_analysis.md` 列了大量 P0/P1 web 标准缺口。Ultralight 路线下，按 [`doc/positioning.md`](../positioning.md) § 2 的标准重新筛选，把**"游戏 UI / React 生态强依赖、不补会偶发劝退用户"**的子集一次性补齐。

非目标：补齐全部 P0。**Web 标准对齐是副产物，不是目标**（[`positioning.md`](../positioning.md) § 2）。

---

## 2. 筛选规则

把 `gap_analysis.md` 的 P0/P1 项过四个筛子，全过的进本批次：

| 筛子 | 描述 |
|---|---|
| **F1：游戏 UI 必需** | 例如手柄导航 → 已有 P0-4；focus-visible → 已有 |
| **F2：React / Preact 生态强依赖** | reconciler 内部用、hooks 库用、UI 库用 |
| **F3：实施成本 ≤ 1 人天** | 大特性自成 P0-* 任务；本任务专挑"小修小补但高价值" |
| **F4：不与其他 P0 任务冲突** | 例如 IME 三件套已有 P0-5，本任务不重复 |

筛后清单见 § 3。

---

## 3. 本批次清单（按优先级）

### 3.1 第一批：DOM API 元素级（React 生态强依赖）

| API | 当前 | 实施 |
|---|---|---|
| `Element.dataset` | C++ 已存属性，JS 无 DOMStringMap | 新增 JS proxy：`get` / `set` 走 `getAttribute("data-" + key)` |
| `Element.matches(selector)` | C++ `SelectorMatcher` 可用 | 直接绑定 |
| `Element.closest(selector)` | 同上 | 沿 parentNode 上溯 + matches |
| `Element.querySelector(s)` (元素级) | document 级已有 | 改为传 root node 即可 |
| `Element.querySelectorAll(s)` (元素级) | 同上 | 同上 |
| `Element.getBoundingClientRect()` | Yoga 布局可读 | 返回 DOMRect (新建)：x/y/w/h/top/right/bottom/left |
| `Element.remove()` | C++ 有 removeChild | 包装：`parent.removeChild(this)` |
| `Element.dispatchEvent(event)` | C++ 已有 | 绑定 |
| `Element.before/after/replaceWith(...)` | C++ 已有 | 批量绑定 |
| `Element.append/prepend(...)` | C++ 已有 | 同上 |

### 3.2 第二批：DOM 遍历属性（reconciler 强依赖）

| API | 当前 | 实施 |
|---|---|---|
| `Node.parentNode` / `parentElement` | 有非标方法 | 加 getter |
| `Node.childNodes` | 有非标 getChildren() | 包装为 NodeList getter |
| `Node.firstChild/lastChild/previousSibling/nextSibling` | C++ 树可遍历 | 加 getter |
| `Node.nodeType` / `nodeName` / `nodeValue` | C++ 知道类型 | 加 getter，常量 ELEMENT_NODE=1 / TEXT_NODE=3 / COMMENT_NODE=8 |
| `Node.textContent` setter | getter 已有 | 加 setter |
| `Node.isConnected` | 沿 parentNode 上溯到 document | 加 getter |
| `Node.contains(other)` | 类似 closest | 直接遍历 |

### 3.3 第三批：HTMLInputElement 核心属性

| API | 当前 | 实施 |
|---|---|---|
| `input.value` (get/set) | C++ InputElementState 有 | 必绑（reconciler / form 库） |
| `input.checked` (get/set) | 类似 | 必绑 |
| `input.disabled` | 类似 | 必绑 |
| `input.type` / `name` / `placeholder` | attribute 级已有 | 镜像 attribute 到 property |
| `select.value` / `selectedIndex` / `options` | 已有 SelectElementState | 必绑 |
| `textarea.value` | 类似 | 必绑 |

### 3.4 第四批：CSS 级联

| 项 | 当前 | 实施 |
|---|---|---|
| **`inherit` 关键字** | ❌ | StyleEngine 显式判断；命中→拷贝 parent computed style |
| **`initial` 关键字** | ❌ | 重置为 spec 初始值（建表） |
| **`unset` 关键字** | ❌ | 继承属性 = inherit；非继承 = initial |
| **`!important`** | 可能未参与级联优先级 | css_parser 标记 → cascade 比较时优先 |
| **继承检测 bug** | 用 `color=="#000000"` 比较 → 显式黑色被覆盖 | 引入 `bool style.<prop>_explicitly_set` 标记，替换默认值比较（`gap_analysis.md` § 3.6） |

> 这条 bug 在 `gap_analysis.md` 是 **P0**；不修，业务方"显式黑色 / Arial"会被父级偷换，行为不可预期。

### 3.5 第五批：常用事件类型与属性

| 项 | 当前 | 实施 |
|---|---|---|
| `scroll` 事件 | 滚动状态有，事件未派发 | scroll 状态变化时 dispatch 到滚动元素 |
| `resize` 事件 | view resize 路径已有 | dispatch on window |
| `DOMContentLoaded` | 加载完成时机已有 | dispatch on document |
| `change` 事件 | input/select 部分已有 | 标准化触发时机：blur 时 / checkbox/select 立即 |
| `MouseEvent.altKey/ctrlKey/shiftKey/metaKey` | C++ 收到，JS 未暴露 | 加属性 |
| `MouseEvent.offsetX/offsetY` | 计算可得 | 加属性（相对 target） |
| `KeyboardEvent.key/code` | dispatch 时未设置 | 在事件构造时填 |
| `WheelEvent.deltaX/deltaY/deltaZ` | C++ 有 | 加属性 |
| `Event.currentTarget` | dispatch 时未设置 | listener 调用前设值 |
| `Event.stopImmediatePropagation()` | 内部支持 | 绑方法 |

> `compositionstart/update/end` **不在本任务**（属于 P0-5）。

### 3.6 第六批：CSS 解析常见值类型坑

| 项 | 描述 |
|---|---|
| `border-radius: 50%` 被解析为 `50px` | parser fix：百分比与父框宽度关联；`gap_analysis.md` § 3.2 |
| `opacity: 50%` 解析失败 | parser 接受百分比 |
| `font-size` 关键字 (`small`/`large`) | 加 keyword 表 |
| `border-width` 关键字 (`thin`/`medium`/`thick`) | 加 keyword 表 |

### 3.7 第七批：UA 样式表小修

| 项 | 描述 |
|---|---|
| `[hidden] { display: none !important }` | 一行 UA 样式 |
| `<a>` 默认样式：`color: blue; text-decoration: underline; cursor: pointer` | 一段 UA 样式 |

---

## 4. 实施步骤

按 § 3 分七批，每批一个 PR，**互不依赖、可并行**：

| Batch | 描述 | 工时估 |
|---|---|---|
| B1 | DOM API 元素级 | 1.5 d |
| B2 | DOM 遍历属性 | 1 d |
| B3 | HTMLInputElement / Select / Textarea | 1.5 d |
| B4 | CSS 级联（inherit/initial/unset/!important + 继承 bug） | 2 d |
| B5 | 事件类型与属性 | 1.5 d |
| B6 | CSS 值解析坑 | 1 d |
| B7 | UA 样式 | 0.2 d |

**总工时：≤ 8 人天**（留缓冲一周）。

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 现有 baseline 全集 | ≥ 99% 通过（不引入像素回归） |
| React Counter / Game UI / Todo demo | 全部跑通；新加事件 / API 不破坏 |
| 显式黑色 `color: #000000` 不再被父级 color 覆盖 | 新加 `test_explicit_color.html` baseline 比对 |
| `[hidden]` 元素不可见 | 新加 `test_hidden_attribute.html` |
| `<a>` 默认蓝色下划线 | `test_anchor_default.html` |
| `inherit/initial/unset` 三关键字按 spec 行为 | 各 1 个测试用例 |
| `!important` 在级联中优先 | `test_important_cascade.html` |
| `event.currentTarget` 在 listener 内非 null | JS 单元 |
| `dataset.foo` ↔ `data-foo` 双向同步 | JS 单元 |
| 元素级 `querySelector` 仅在子树内匹配 | JS 单元 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| `gap_analysis.md` 中本批次相关条目 | 全部移到 ✅ 状态 |
| 第三方 React 组件库（如 Radix / shadcn）至少 3 个组件能跑（hover/focus/click 路径） | 推荐验收 |
| QuickJS 调用开销不显著上升 | 新 binding 的 P/Invoke 抖动 < 5% |

### 5.3 必须新增的测试用例

| 文件 | 验证 |
|---|---|
| `test_dataset_basic.html` | dataset.* ↔ data-* |
| `test_element_matches_closest.html` | matches / closest 行为 |
| `test_element_query_subtree.html` | 元素级 qS / qSA |
| `test_get_bounding_client_rect.html` | 含 transform / scroll 的 rect 计算 |
| `test_explicit_color.html` | 显式黑色不被覆盖（继承 bug fix） |
| `test_inherit_initial_unset.html` | 三关键字 |
| `test_important_cascade.html` | !important 优先级 |
| `test_hidden_attribute.html` | [hidden] |
| `test_anchor_default.html` | `<a>` 默认样式 |
| `test_event_currentTarget.html` | currentTarget 非 null |
| `test_event_modifier_keys.html` | altKey/ctrlKey/shiftKey/metaKey |
| `test_event_key_code.html` | KeyboardEvent.key/code 填写 |
| `test_input_value_property.html` | input.value get/set |
| `test_select_value_property.html` | select.value/selectedIndex |
| `test_scroll_event.html` | scroll 事件触发 |
| `test_resize_event.html` | resize 事件触发 |
| `test_dom_traversal.html` | parentNode/firstChild/etc. |
| `test_node_isConnected.html` | isConnected |
| `test_border_radius_percent.html` | 50% 实际是百分比（已有同名，扩验证） |
| `test_opacity_percent.html` | 50% 解析正确（已有，扩验证） |

### 5.4 验证脚本

```bash
# 像素回归
python dong/scripts/tools/run_baseline_compare.py --filter "test_dataset|test_element_|test_explicit_|..."

# JS 单元
cd dong && zig build run-feature-tests --filter "B1|B2|B3|B4|B5|B6|B7"

# 第三方 React 组件库 smoke
node dong/react/test/run_radix_smoke.mjs    # 新增
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| CSS 继承 bug 修复后大量已有 baseline 像素改变（因为以前继承错） | 修复前先跑 baseline，记录哪些用例像素会改；正确性达成后用新像素更新 baseline |
| `!important` 级联引入新优先级，部分用户样式行为变 | 修复前列出受影响 case；与业务方沟通 |
| `Node.dispatchEvent` 暴露后 JS 可触发任意事件，可能触发未防的代码路径 | 全集回归 |
| 第三方组件库依赖 `getComputedStyle` 的更多属性 | 本批不一并补；issue 立项延 Phase 1 |

---

## 7. 不在本任务范围

- ❌ Observer JS bindings (`MutationObserver` / `ResizeObserver` / `IntersectionObserver`)（Phase 1 P1-9）
- ❌ `position: fixed` 视口语义（Phase 1 P1-9）
- ❌ `<details>` / `<dialog>` 完整行为（Phase 1 P1-9）
- ❌ IME 三件套（P0-5）
- ❌ Form validation `:required/:pattern/:checkValidity`（Phase 1 P1-9）
- ❌ CSS Grid（Phase 1 P1-5）
- ❌ Logical properties（margin-inline 等）（Phase 1+）
- ❌ `color-mix()` / `oklch` / `light-dark()`（Phase 1+）

---

## 8. 完成后更新

- [ ] `doc/specific/html_css_dom_gap_analysis.md` 把本批次条目状态从 ⚠️/❌ 改为 ✅
- [ ] `doc/specific/html_css_dom_草案.md` 同步状态
- [ ] `doc/重要特性.md` 章节"DOM API"扩充
- [ ] `doc/arch/react.md` § 4.4 "DOM binding 差距"清掉已补条目
