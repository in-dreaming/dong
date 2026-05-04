# Phase 1 方案集

> Phase 1 主线：让 dong "可被宿主嵌入、可被团队上量"。聚焦点：**可嵌入性 + 工程化 / 开发体验**。
>
> 上游决策：[`doc/positioning.md`](../positioning.md) · 三阶段路线：[`doc/roadmap.md`](../roadmap.md) · 性能门槛：[`doc/perf_budget.md`](../perf_budget.md)
>
> 时间窗口：3–9 个月（Phase 0 完成之后）。
>
> 状态：方案 v1 / 2026-04-17。每个工作项独立 PR / 独立验收。

---

## 1. 工作项总览

| ID | 名称 | 类型 | 估时 | 关键依赖 | 方案 |
|---|---|---|---|---|---|
| **P1-1** | DongDrawList C ABI v1 | 架构 | 3–4 周 | Phase 0 完成（含 P0-1 / P0-2 / P0-3 的命令稳定）| [P1-1](./P1-1_drawlist_c_abi.md) |
| **P1-2** | `<host-view>` 嵌入元素 + DrawHostView 命令 | 架构 | 2 周 | P1-1 | [P1-2](./P1-2_host_view_embed.md) |
| **P1-3** | DevTools v1（嵌入式 inspector） | 工程化 | 4 周 | 与 P0-6 / P0-7 联动 | [P1-3](./P1-3_devtools_v1.md) |
| **P1-4** | Live reload | 工程化 | 2 周 | — | [P1-4](./P1-4_live_reload.md) |
| **P1-5** | CSS Grid 子集 | 渲染特性 | 3 周 | — | [P1-5](./P1-5_css_grid_subset.md) |
| **P1-6** | HDR 输出（基础） | 渲染特性 | 3 周 | — | [P1-6](./P1-6_hdr_output.md) |
| **P1-7** | JS 引擎 benchmark 报告 | 决策依据 | 2 周 | — | [P1-7](./P1-7_js_engine_benchmark.md) |
| **P1-8** | Async layout / shaping（实验） | 性能 | 3 周 | 与 P0-6 协同 | [P1-8](./P1-8_async_layout_shaping.md) |
| **P1-9** | `gap_analysis.md` P1 清账 | 标准对齐 | 4 周（多人并行 8 批） | — | [P1-9](./P1-9_gap_analysis_p1_cleanup.md) |

---

## 2. 推荐执行顺序与并行度

```
Month 1
  ├── P1-1 (DrawList ABI 头文件冻结、Emitter 基础几何)
  ├── P1-7 (JS bench：vtable 抽象 + 三引擎接入)        ← 独立可并行
  ├── P1-9 (B1 Observer / B5 Web API / B7 color-mix) ← 独立小批
  └── P1-4 (Live reload watcher 集成)                  ← 独立可并行

Month 2
  ├── P1-1 续 (图片/文本/渐变/9-slice)
  ├── P1-2 (host-view 元素 + DrawList 联动)             ← 等 P1-1 S2 后启动
  ├── P1-5 (CSS Grid 自研 GridEngine)                   ← 独立
  ├── P1-7 续 (benchmark 跑数 + 报告)
  ├── P1-9 续 (B2 fixed / B3 form validity / B6 CSS 中频)
  └── P1-8 (worker 骨架 + Layout snapshot)              ← 等 P0-6 完成

Month 3
  ├── P1-1 完工 (S6/S7 iter + ABI 文档)
  ├── P1-2 完工 (host-view scene graph 模式 + demo)
  ├── P1-3 (DevTools v1：DOM 树 + style + perf panel)   ← 等 P0-6/P0-7 完成
  ├── P1-5 完工 (place-* + Chrome diff baseline)
  ├── P1-6 (HDR：swapchain + shader 变体 + --hdr-boost)
  ├── P1-8 完工 (force_sync + 实验报告)
  └── P1-9 续 (B4 details/dialog 完整 + B8 logical props)

Buffer
  └── 整合 + Phase 1 完成判定全部跑通
  └── 所有方案文档"完成后更新清单"勾掉
```

并行节点：

- **P1-1 / P1-2** 是 ABI 串联，建议同人或同组顺次推进。
- **P1-3** 在 P0-6 / P0-7 完成后启动；不需要等 P1-1。
- **P1-4 / P1-5 / P1-6 / P1-7 / P1-8 / P1-9** 互相独立，可六人 / 六组并行。
- **P1-8** 必须等 P0-6 完成（依赖 invalidation 机制）。

总周期：**~3 个月**（一个 6 人小队全速推进）；缓冲到 9 个月。

---

## 3. Phase 1 完成判定（与 [`roadmap.md`](../roadmap.md) § 2.2 对齐）

- [ ] 三平台用纯 dong（无 SDL window）能演示宿主消费 `DongDrawList` 翻译为自己 mesh / draw 命令，UI 像素与 SDL backend 一致（容差 < 1%）。
- [ ] `<host-view>` 嵌入 demo：HTML 内插入"被宿主控制的 minimap quad"，HTML 滚动 / clip / opacity 都正确传递。
- [ ] DevTools 在 dong_app 中可一键打开，可改任意元素 inline style 立即看到效果。
- [ ] React 应用 live reload 改一个组件 prop ≤ 500 ms 看到画面更新。
- [ ] HDR 显示器上 dong 输出能进入 HDR 模式，亮度峰值 > 400 nits（视显示器能力）。
- [ ] JS bench 报告产出，决策"切 / 不切"已写入 `positioning.md` § 8 决策记录。
- [ ] CSS Grid 子集跑通典型 inventory / editor 用例。
- [ ] 所有 9 个工作项的 § 5 Hard 验证规则全部通过。
- [ ] 所有方案文档底部的"完成后更新清单"全部执行。

---

## 4. 文档约定

继承 [Phase 0 README](../phase0/README.md) § 4 的八节结构与 § 5 共用工程约定，不重复。

新增约定：

| 项 | 约定 |
|---|---|
| **PR 标题前缀** | `[P1-X]`（同 P0 风格） |
| **新增公共头文件** | `dong/include/dong_*.h` 必须经 review，纳入稳定 ABI |
| **vtable / plugin 接口** | 每次扩展必须更新版本号 + 兼容文档（参考 P1-1 ABI 版本机制） |
| **DevTools dogfood** | DevTools 自身用 dong 渲染，遇到 DevTools 渲染问题 → P0/P1 各任务必须修 |

---

## 5. 与 Phase 0 / Phase 2 的关系

| 关系 | 说明 |
|---|---|
| **Phase 0 → Phase 1** | Phase 0 性能 + 交互稳定下来后，Phase 1 才能谈"嵌入宿主"和"上量给团队"。P0-6 是 P1-3 / P1-8 的前置。 |
| **Phase 1 → Phase 2** | P1-1（DrawList ABI）+ P1-2（host-view）是 Phase 2 World Space UI primitives 的底层基础；P1-7 决策若切 JS 引擎则在 Phase 2 落地；P1-8 实验若达标则 P2-9 默认开启。 |
| **`integration_ue.md` / `integration_unity.md`** | 长期低优先级；Phase 1 仅交付 ABI，**adapter 工程实现不在 Phase 1 范围**（[`positioning.md`](../positioning.md) § 8、[`roadmap.md`](../roadmap.md) § 7）。 |

---

## 6. 与既有文档的索引

| 文档 | 与本目录关系 |
|---|---|
| [`doc/positioning.md`](../positioning.md) | 上位决策；本目录方案不得违背 |
| [`doc/roadmap.md`](../roadmap.md) | Phase 1 概览来自此 |
| [`doc/perf_budget.md`](../perf_budget.md) | 验证规则的指标依据 |
| [`doc/phase0/README.md`](../phase0/README.md) | 多数 Phase 1 任务是 Phase 0 的延续 / 进阶 |
| [`doc/ideal/引擎适配.md`](../ideal/引擎适配.md) | P1-1 / P1-2 的设计原案 |
| [`doc/ideal/hdr.md`](../ideal/hdr.md) | P1-6 占位文档；本任务完成后替换 |
| [`doc/arch/react.md`](../arch/react.md) | P1-7 / P1-9 React 生态依据 |
| [`doc/arch/arch_font.md`](../arch/arch_font.md) | P1-8 shaping 异步的依据 |
| [`doc/specific/html_css_dom_gap_analysis.md`](../specific/html_css_dom_gap_analysis.md) | P1-9 来源清单 |
| [`doc/integration_ue.md`](../integration_ue.md) [`integration_unity.md`](../integration_unity.md) | P1-1 / P1-2 完成后状态可对接（adapter 仍低优先级） |

---

## 7. 决策记录

| 日期 | 决策 |
|---|---|
| 2026-04-17 | Phase 1 方案集 v1 创建 |
| 2026-04-17 | UE/Unity adapter 工程实现明确**不**在 Phase 1（只交付 ABI） |
| 2026-04-17 | World Space 3D primitives 不在 Phase 1（归 Phase 2） |
| 2026-04-17 | 官方组件库 `dong-ui` 不在 Phase 1（归 Phase 2） |
| 2026-04-17 | 切换 JS 引擎不在 Phase 1（仅产 benchmark 报告，决策后续 Phase 落地） |

后续 Phase 1 内重大调整在此处追加。
