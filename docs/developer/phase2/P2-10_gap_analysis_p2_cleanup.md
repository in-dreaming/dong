# P2-10 — `gap_analysis.md` P2 清账

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 2 P2-10
> 来源清单：[`docs/developer/gap-analysis.md`](../specific/html_css_dom_gap_analysis.md)
> 历史清账：[P0-8](../phase0/P0-8_gap_analysis_p0_cleanup.md) [P1-9](../phase1/P1-9_gap_analysis_p1_cleanup.md)
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

继 P0-8（基础 / React 强依赖）与 P1-9（中等成本特性）之后，本任务把 web 标准对齐推向"够长尾、已稳定"的阶段：

- RTL / BiDi / `:dir()` 方向相关
- Pointer / Touch / IME 完整属性集
- Animation / Transition 高级特性
- CSS contain / content-visibility / 渲染隔离（性能向）
- 容器查询 `@container`
- `scroll-snap` 滚动吸附
- 无障碍（a11y）基础（aria-* 完整解析 + screen reader API hint）

继续遵循 [`positioning.md`](../positioning.md) Ultralight 路线：**只补游戏 editor / runtime 真正需要的**；不在范围的明确不做。

---

## 2. 筛选规则（与 P0-8 / P1-9 一致）

| 筛子 | 描述 |
|---|---|
| **F1** | 游戏 editor / runtime 真正需要 |
| **F2** | dong-ui (P2-5) 组件实现需要 |
| **F3** | 实施成本 1–5 人天（更大的自成 P3 任务） |
| **F4** | 不与其他 P2 任务冲突（P2-1/2/3 已覆盖 World UI；P2-4 覆盖矢量动画；P2-5 覆盖组件库；本任务关注剩余 web feature） |

---

## 3. 本批次清单

### 3.1 第一批：RTL / BiDi 基础

游戏 UI 全球化必备；阿拉伯语 / 希伯来语市场。

| 项 | 内容 |
|---|---|
| `direction: rtl` 完整支持 | 已有 CSS direction，但需补 inline element 顺序反转 |
| `unicode-bidi` 完整 | embed / isolate / bidi-override |
| `:dir(ltr/rtl)` 伪类 | 选择器 |
| HTML `dir` 属性映射到 CSS direction | [`gap_analysis.md`](../specific/html_css_dom_gap_analysis.md) § 2.1 |
| Logical properties RTL 反转（与 [P1-9 B8](../phase1/P1-9_gap_analysis_p1_cleanup.md) 协同）| margin-inline-start 在 RTL 下 = margin-right |
| `<bdi>` `<bdo>` 元素 | bidi 隔离 |
| BiDi reordering（HarfBuzz 驱动）| 字符顺序 vs 视觉顺序 |

### 3.2 第二批：Pointer / Touch 完整属性

| 项 | 内容 |
|---|---|
| `PointerEvent.pointerId/pointerType/pressure/tangentialPressure/tiltX/tiltY/twist/isPrimary` | 数位板 / pen 输入 |
| `setPointerCapture` / `releasePointerCapture` | drag 时 pointer 锁定 |
| `gotpointercapture` / `lostpointercapture` 事件 | 同上 |
| `TouchEvent.touches/changedTouches/targetTouches` | 移动多指 |
| `touch-action` CSS（已部分） | 完善 pan-x/y/none/manipulation |

### 3.3 第三批：动画 / 过渡高级

| 项 | 内容 |
|---|---|
| `transition` 多值（逗号分隔） | 已部分 |
| `animation` 多值 | 已部分 |
| `animation-composition` | 多动画叠加方式 |
| `Web Animations API`（基础）| `Element.animate(keyframes, options)` |
| `transitionrun/transitionstart/transitioncancel` 事件 | 完整 |
| `animationiteration` / `animationcancel` 事件 | 完整 |
| `getComputedStyle` 在动画期间返回当前插值 | 必须 |

### 3.4 第四批：渲染隔离（性能向）

| 项 | 内容 |
|---|---|
| `contain: layout/paint/size/style` | hint，dong 内部用作 invalidation 边界 |
| `content-visibility: auto/hidden/visible` | 离屏跳过渲染（与 P0-6 协同） |
| `will-change` | GPU 合成层提示 |
| `IntersectionObserver` 已有（P1-9 B1） | 协同 |

### 3.5 第五批：容器查询

| 项 | 内容 |
|---|---|
| `@container (min-width: ...)` | 组件级响应式 |
| `container-type: inline-size / size / normal` | 容器类型声明 |
| `container-name: foo` | 命名容器 |
| `cqw / cqh / cqi / cqb` 单位 | 容器查询单位 |

dong-ui (P2-5) 强依赖；同一 Button 组件在不同父容器尺寸下变态。

### 3.6 第六批：滚动吸附

| 项 | 内容 |
|---|---|
| `scroll-snap-type: x/y mandatory/proximity` | 吸附轴与强度 |
| `scroll-snap-align: start/center/end` | 子项对齐点 |
| `scroll-snap-stop: normal/always` | 翻页强制 |
| `scroll-padding` / `scroll-margin` | 吸附偏移 |

游戏分页 UI / 卡片轮播必备。

### 3.7 第七批：无障碍（a11y）基础

dong 不强求 screen reader 兼容（嵌入游戏 UI 通常不接），但要给业务方留 hook：

| 项 | 内容 |
|---|---|
| `role` `aria-*` 完整解析（已部分）| 完整 |
| `Element.ariaXxx` 反射属性 | 标准化 |
| 无障碍树构建（内部 Accessibility Tree） | 不暴露给系统 AT，但 DevTools / 业务方可查询 |
| `inert` 属性完整行为 | （[P0-8](../phase0/P0-8_gap_analysis_p0_cleanup.md) 已有 hidden）|
| `focus-visible` 改进（更精确判定键盘 vs 鼠标）| 视觉细节 |

### 3.8 第八批：剩余 CSS / Web 长尾

| 项 | 内容 |
|---|---|
| `quotes` + `content: open-quote / close-quote` | `<q>` 元素 |
| `counter-reset / counter-increment / counter()` | CSS 计数器（自动编号）|
| `image-set()` | 分辨率切换 |
| `text-emphasis` | 着重符（中日文）|
| `font-feature-settings` / `font-variant-*` | OpenType features |
| `hyphens: auto` | 自动断字（需 hyphenation 库）|
| Constructable StyleSheets (`new CSSStyleSheet()`) | 动态样式注入 |
| `dispatch CustomEvent` JS 完整流程 | 已有，确认完整 |
| `crypto.subtle.digest` | 仅业务侧调（`randomUUID` P1-9 已有）|
| `Selection / Range` 完整 | 已部分 |
| `requestIdleCallback` | 非紧急任务 |

### 3.9 第九批：dong 私有的"反向标准化"

历经三轮清账后，dong 已存在一些私有特性（`<host-view>` `<dong-lottie>` `<dong-rive>` `--hdr-boost` `nav-*` 等）。本批次**整理这些私有特性的命名 / 文档**：

- 统一前缀：HTML 标签 `dong-*`；CSS 属性 `--dong-*`；data-* 属性 `data-dong-*`。
- 在 `docs/reference/extensions.md` 集中列出。
- DevTools 显示时标识"dong extension"。

---

## 4. 实施步骤

9 批互相独立，**可并行**：

| Batch | 描述 | 工时估 |
|---|---|---|
| B1 | RTL / BiDi | 5 d |
| B2 | Pointer / Touch 属性 | 3 d |
| B3 | Animation 高级 + WAAPI 基础 | 4 d |
| B4 | 渲染隔离（contain / content-visibility / will-change） | 3 d |
| B5 | 容器查询 `@container` | 4 d |
| B6 | scroll-snap | 2 d |
| B7 | a11y 基础 | 3 d |
| B8 | 长尾 CSS / Web | 4 d |
| B9 | dong 扩展整理 + 文档 | 1 d |

**总工时：~30 人天**（最大的清账批次；多人并行可压缩）。

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 既有 baseline 全集 | ≥ 99% 通过 |
| RTL 主流 case 像素与 Chrome diff < 2% | 必须 |
| 数位板 PointerEvent.pressure 正确传递（手动）| 必须 |
| Web Animations API: `Element.animate()` 基础调用工作 | 必须 |
| `content-visibility: auto` 离屏元素跳过 paint（counter 验证） | 必须 |
| `@container (min-width: 400px)` 容器查询响应正确 | 像素 baseline |
| `scroll-snap-type: x mandatory` 吸附正确 | 手动 + baseline |
| ARIA 属性 JS 反射 `el.ariaLabel` 工作 | 单元 |
| dong 私有特性统一前缀 | 文档 + DevTools |
| `gap_analysis.md` 本批条目状态全部 ✅（除明确不做的）| 必须 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 第三方 a11y test（axe-core）至少 critical 0 | 期望 |
| 阿拉伯语 demo 完全可读可交互 | 期望 |
| 移动多指手势 demo（pinch-zoom） | 期望（v1 仅事件，gesture 库业务侧自带） |
| `font-feature-settings` 高级特性（OpenType）| 期望 |

### 5.3 必须新增的测试

| 文件 | 验证 |
|---|---|
| `test_rtl_basic.html` | 阿拉伯语段落 |
| `test_rtl_logical_properties.html` | margin-inline-start 反转 |
| `test_dir_pseudo.html` | `:dir()` 选择器 |
| `test_pointer_event_full.html` | pointerType/pressure 等 |
| `test_pointer_capture.html` | drag pointer 锁定 |
| `test_animation_multi.html` | 多 animation 逗号分隔 |
| `test_web_animations_api.html` | element.animate |
| `test_content_visibility_auto.html` | 离屏跳渲染 perf |
| `test_container_query_basic.html` | `@container` 基础 |
| `test_container_query_units.html` | cqw/cqh |
| `test_scroll_snap_x_mandatory.html` | 吸附 |
| `test_aria_reflection.html` | el.aria* |
| `test_inert_full.html` | inert 子树不可交互 |
| `test_quotes_content.html` | counter 计数 |
| `test_dong_extensions_listing.html` | DevTools 中标识私有扩展 |

每个用例必须有 Chromium baseline 入库（除 dong 私有的）。

### 5.4 验证脚本

```bash
# 像素回归
python dong/scripts/tools/run_baseline_compare.py --filter "test_(rtl|dir_|pointer_|animation_multi|web_animations|content_visibility|container_query|scroll_snap|aria_|inert_full|quotes_)"

# JS 单元
zig build run-feature-tests --filter "Bidi|Pointer|Animation|ContentVisibility|ContainerQuery|ScrollSnap|Aria"

# a11y
node dong/scripts/test_axe_core.mjs
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| RTL 重排涉及 inline 布局深度修改 | 与 HarfBuzz BiDi 实现对齐；测试用例覆盖 |
| 容器查询 invalidation 与 P0-6 冲突 | container-type 视为隔离边界；invalidation 不跨 |
| `content-visibility: auto` 隐藏时 hit-test / focus 行为 | spec 明示离屏不可交互；与 P0-6 协同 |
| Animation API 与 CSS animation 双系统冲突 | API 仅作为 CSS animation 的 JS 控制层 |
| 私有扩展前缀历史包袱（已有 demo 用旧名）| 留 1 个版本兼容期 + console.warn 旧名 |
| ScrollSnap 与游戏 controller spatial nav 冲突 | dpad 触发 snap 时显式 prevent |

---

## 7. 不在本任务范围（明确不做或留 P3+）

- ❌ 完整 ATK / NSAccessibility / UIA 系统集成（接 system AT；游戏 UI 通常不接）
- ❌ Speech synthesis / recognition
- ❌ Vibration / Geolocation / Notification
- ❌ Web Components 完整（Shadow DOM / Slot / Custom Elements）—— 除非业务强需求
- ❌ Full WAAPI（仅基础 `animate()`，复杂 KeyframeEffect 留 P3）
- ❌ Service Worker / Web Worker
- ❌ MathML
- ❌ SVG 高级（filters / animation / interactive）
- ❌ Print stylesheet / `@page`
- ❌ Constructable Stylesheets 完整（仅基础）

---

## 8. 完成后更新

- [ ] `docs/developer/gap-analysis.md` 大量条目状态从 ⚠️/❌ 改 ✅；明确不做的标"❌ Won't Do（理由）"
- [ ] `docs/reference/css-subset.md` 同步
- [ ] `docs/reference/features-index.md` 多个章节扩充：RTL / Pointer / Animation / 容器查询 / ScrollSnap / a11y
- [ ] `docs/reference/extensions.md`（新建）集中列私有扩展
- [ ] `docs/overview/positioning.md` § 6 能力边界表更新
