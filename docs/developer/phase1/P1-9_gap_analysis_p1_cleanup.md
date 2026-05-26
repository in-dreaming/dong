# P1-9 — `gap_analysis.md` P1 清账

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 1 P1-9
> 来源清单：[`docs/developer/gap-analysis.md`](../specific/html_css_dom_gap_analysis.md)
> 上轮清账：[P0-8](../phase0/P0-8_gap_analysis_p0_cleanup.md)
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

继 P0-8 清掉 web 标准"游戏 / React 强依赖小修小补"之后，本任务接着清 **Phase 1 必须的中等成本特性**：

- 完善表单 / 模态 / 折叠等元素的**完整行为**（不只是渲染）。
- 三大 Observer JS 绑定（`Mutation/Resize/Intersection`）。
- `position: fixed` 视口语义。
- `performance.now()` / `DOMRect` / `FormData`。

仍按 [`positioning.md`](../positioning.md) Ultralight 路线裁剪，**只补游戏 editor / runtime / React 生态需要的**。

---

## 2. 筛选规则（与 P0-8 同原则）

| 筛子 | 描述 |
|---|---|
| **F1** | 游戏 UI / editor 必需（不补会偶发劝退） |
| **F2** | React 生态强依赖 |
| **F3** | 实施成本 1–3 人天（更大特性自成 P1-* 任务） |
| **F4** | 不与 P1 的其他任务冲突（P1-5 grid / P1-6 hdr 颜色函数等已有归属） |

---

## 3. 本批次清单

### 3.1 第一批：DOM Observer JS 绑定

C++ 已有；仅 JS 绑定缺失（[`gap_analysis.md`](../specific/html_css_dom_gap_analysis.md) § 4.7）。

| API | 影响 |
|---|---|
| `MutationObserver` | React 第三方库（如 floating-ui）依赖 |
| `ResizeObserver` | 响应式组件 / chart 库 |
| `IntersectionObserver` | 延迟加载 / 无限滚动 |

实施：在 `js_observer_bindings.cpp` 添加 JS class + 方法（C++ 侧已有 `dom/observer.cpp`）。

### 3.2 第二批：position: fixed 视口语义

`gap_analysis.md` 标 P0；现状 fixed 退化为 absolute。

修复点：
- `position: fixed` 元素的包含块 = 视口（不是 transformed 祖先）。
- 滚动时元素位置不变。
- z-index 与正常流分离的堆叠上下文。
- transform / filter 祖先的"fixed 退化"特例（CSS spec：祖先有 transform 时，fixed 等同于 absolute）。

代码：`src/layout/layout_engine.cpp` + `src/render/painter.cpp`。

### 3.3 第三批：表单约束与验证

| API | 内容 |
|---|---|
| `input.required / pattern / min / max / step / minlength / maxlength` 验证 | 输入时不阻止，但 `:invalid` 伪类反映 |
| `input.checkValidity()` / `setCustomValidity()` | 编程触发验证 |
| `input.validity`（ValidityState）| 详细错误标志 |
| `<form>.checkValidity()` / `reportValidity()` | 整 form |
| `submit` 事件 + 默认验证拦截 | 标准行为 |
| `change` 事件标准化 | input/select 触发时机 |

### 3.4 第四批：`<details>` / `<dialog>` 完整行为

`gap_analysis.md` § 1 标"声称支持但实际未实现"。

| 元素 | 待补 |
|---|---|
| `<details>` | 点击 `<summary>` 切 `open` 属性 + 隐藏/显示子内容 + disclosure triangle marker + `:open/:closed` 伪类 + `toggle` 事件 |
| `<summary>` | 默认 `display: list-item` + 带 marker（disclosure triangle） |
| `<dialog>` | `showModal()` / `show()` / `close()` 方法 + `::backdrop` 渲染 + top-layer 提升 + Escape 关闭 + `close` / `cancel` 事件 |

实施成本相对中等（每个 ~2 天）。新增：`src/dom/details_element.cpp` `dialog_element.cpp` 已存在但需补完整。

### 3.5 第五批：常用 Web API 补齐

| API | 用途 |
|---|---|
| `performance.now()` | 高精度时间戳，动画 / perf 测量必备（`gap_analysis.md` § 4.8） |
| `performance.timeOrigin` | 配对 |
| `DOMRect` 类（getBoundingClientRect 返回类型） | 标准化 |
| `FormData` | 表单数据序列化 |
| `DOMParser`（最小集：`parseFromString` for HTML） | 第三方库常用 |
| `structuredClone()` | React / Redux 库依赖 |
| `crypto.randomUUID()` | id 生成（应用层常用） |

### 3.6 第六批：CSS 中频特性

| 特性 | 用途 |
|---|---|
| `display: contents` | 组件包装器透传布局 |
| `display: flow-root` | BFC（margin collapse 阻断） |
| `accent-color` | 表单控件主题色 |
| `appearance: none` | 去除默认控件样式 |
| `scroll-behavior: smooth` | 平滑滚动 |
| `overscroll-behavior` | 防止滚动链（modal 内必须） |
| `tab-size` | `<pre>` 制表符宽度 |
| `text-decoration` 完整简写 | line / style / color / thickness |
| `transition` / `animation` 多值（逗号分隔）| 已有部分支持，需 robust |

### 3.7 第七批：CSS 颜色函数（与 P1-6 HDR 协同）

| 函数 | 用途 |
|---|---|
| `color-mix()` | 颜色混合（Baseline 2023） |
| `light-dark()` | 跟 `color-scheme` 切换 |
| `color-scheme: light dark` | 主题适配 |

注：`oklch / oklab / display-p3` 在 P1-6 HDR 任务中已实现解析，本任务只补 `color-mix / light-dark / color-scheme`。

### 3.8 第八批：Logical Properties（中等优先级）

| 特性 | 用途 |
|---|---|
| `margin-inline-start/end` `padding-inline-start/end` | RTL 适配；现代 CSS 框架常用 |
| `inset-block` `inset-inline` | 同上 |
| `border-inline` `border-block` | 同上 |
| `place-items` `place-content` `place-self` | flex/grid 缩写（[P1-5](./P1-5_css_grid_subset.md) 已部分） |

物理属性映射：根据 `direction` + `writing-mode` 解析；v1 仅 LTR + horizontal-tb（默认），其他 fallback。

---

## 4. 实施步骤

8 批互相独立，**可并行**：

| Batch | 描述 | 工时估 |
|---|---|---|
| B1 | Observer JS bindings | 1.5 d |
| B2 | position: fixed 视口语义 | 2 d |
| B3 | 表单验证 (`required/pattern/checkValidity` + ValidityState) | 3 d |
| B4 | `<details>` `<dialog>` 完整行为 + `::backdrop` + top-layer | 4 d |
| B5 | `performance.now/DOMRect/FormData/DOMParser/structuredClone/randomUUID` | 2 d |
| B6 | CSS 中频（contents/flow-root/accent-color/appearance/scroll-behavior/overscroll/tab-size/text-decoration） | 3 d |
| B7 | `color-mix/light-dark/color-scheme` | 2 d |
| B8 | Logical properties（LTR + horizontal-tb 子集） | 2 d |

**总工时：~20 人天**（较 P0-8 长，是 Phase 1 的"持续清账"工作）。

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 既有 baseline 全集 | ≥ 99% 通过（不引入回归） |
| Observer 三件套 JS 单元用例全过 | 必须 |
| `position: fixed` 在普通祖先 / transformed 祖先两种场景行为正确 | 像素 baseline + 单元 |
| `<details>` 点击切换 + 像素 baseline 一致 | 必须 |
| `<dialog>.showModal()` 正确创建 modal + ::backdrop 显示 + Escape 关闭 + close 事件 | 必须 |
| 表单 `required/pattern` `:invalid` 伪类正确反映；`checkValidity()` 返回正确值 | 单元 |
| `performance.now()` 单调递增 + 与 `Date.now()` 偏移 < 100ms | 单元 |
| `display: contents` 子节点正确"消失"成为父元素的子节点参与布局 | 像素 baseline |
| `display: flow-root` 阻断 margin collapse | 像素 baseline |
| `color-mix(in srgb, red, blue)` 像素与 Chrome 一致 | baseline |
| `margin-inline-start: 10px` 在 LTR 下等同 `margin-left: 10px` | 像素 baseline |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 第三方组件库 smoke test 通过率 | Radix UI / shadcn / Floating UI / chart.js 各跑一个 |
| `gap_analysis.md` 本批次条目状态全部移到 ✅ | 必须 |
| `<dialog>` modal stack（嵌套 dialog）正确 | 期望 |
| `light-dark()` 跟随系统主题切换 | 期望 |

### 5.3 必须新增的测试用例

| 文件 | 验证 |
|---|---|
| `test_mutation_observer.html` (已有，扩 JS 绑定) | 增删属性触发 |
| `test_resize_observer.html` | 尺寸变化触发 |
| `test_intersection_observer.html` | 进入 / 离开视口触发 |
| `test_position_fixed_viewport.html` | 滚动时位置不变 |
| `test_position_fixed_in_transform.html` | transform 祖先内退化为 absolute |
| `test_details_toggle.html` | 点击切 open + toggle 事件 |
| `test_dialog_modal.html` | showModal + ::backdrop + Escape |
| `test_dialog_form_method.html` | `<form method="dialog">` 提交关 dialog |
| `test_form_validity.html` | required/pattern + checkValidity |
| `test_form_submit_event.html` | submit 事件 + 验证拦截 |
| `test_performance_now.html` | now 单调；timeOrigin |
| `test_dom_rect.html` | DOMRect 字段正确 |
| `test_form_data.html` | FormData append/get/delete |
| `test_dom_parser.html` | parseFromString HTML |
| `test_structured_clone.html` | 深克隆对象 / 数组 / Map |
| `test_crypto_random_uuid.html` | 格式正确 |
| `test_display_contents.html`（已有，扩） | layout 透传 + 事件冒泡 |
| `test_display_flow_root.html` | margin collapse 阻断 |
| `test_accent_color.html` | input checkbox 主题色 |
| `test_appearance_none.html` | 去默认样式 |
| `test_scroll_behavior_smooth.html` | smooth scroll 动画 |
| `test_overscroll_behavior_none.html` | 模态内滚动不冒泡 |
| `test_tab_size.html` | `<pre>` 制表符 |
| `test_color_mix.html` | color-mix 像素一致 |
| `test_light_dark.html` | light-dark + color-scheme 切换 |
| `test_logical_margin_inline.html` | LTR 下逻辑属性 |
| `test_logical_inset.html` | inset-block / inset-inline |

每个用例必须有 Chromium baseline 入库。

### 5.4 验证脚本

```bash
# 像素回归
python dong/scripts/tools/run_baseline_compare.py --filter "test_(mutation|resize|intersection|position_fixed|details|dialog|form_|performance|dom_rect|form_data|dom_parser|display_contents|display_flow_root|accent|appearance|scroll_behavior|overscroll|tab_size|color_mix|light_dark|logical_)"

# JS 单元
zig build run-feature-tests --filter "Observer|FormValidity|Dialog|Details|Performance"

# 第三方组件库 smoke
node dong/react/test/run_radix_smoke.mjs
node dong/react/test/run_floating_ui_smoke.mjs
node dong/react/test/run_chartjs_smoke.mjs
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| `<dialog>` 的 top-layer 与现有 z-index / overflow 系统冲突 | 单独 layer 渲染（与 P0-3 isolated layer 同机制）；像素 baseline 看门狗 |
| Observer 性能影响（每帧调度）| 内部 throttle；只在 invalidation 涉及 observed node 时触发 |
| 表单验证改变现有交互（业务方未预期） | 业务侧用 `novalidate` 关掉默认行为 |
| `display: contents` 会破坏 hit-test / focus / 事件冒泡 | 测试用例 `test_display_contents_events.html` 是看门狗 |
| Logical property 在 RTL 下未实现 | 文档明示 v1 仅 LTR；RTL 留 Phase 2 |
| Async layout（P1-8）开启时 `getBoundingClientRect` 同步性 | P1-8 已设计 force_sync；本任务不处理 |

---

## 7. 不在本任务范围

- ❌ 完整 RTL 文本 / `:dir()` 伪类 / BiDi（Phase 2）
- ❌ Web Animations API（CSS animation 已支持；JS API 留 Phase 2）
- ❌ Pointer Events 完整属性集（PointerEvent.pointerId / pressure，留 Phase 2）
- ❌ Touch Events 完整暴露（移动端，留 Phase 2）
- ❌ Vibrate / Geolocation / Notification API（不在游戏 UI 工具范畴）
- ❌ Web Components（Shadow DOM / slot / Custom Elements；除非业务强需求）
- ❌ Constructable StyleSheets（动态样式注入；优先级低）

---

## 8. 完成后更新

- [ ] `docs/developer/gap-analysis.md` 本批次条目状态从 ⚠️/❌ 改为 ✅
- [ ] `docs/reference/css-subset.md` 同步更新
- [ ] `docs/reference/features-index.md` 表单 / dialog / observer / fixed / 逻辑属性 / color-mix / light-dark 等章节扩充
- [ ] `docs/developer/arch/react-reconciler-spec.md` § 4.4 / § 4.5 同步可用 API
