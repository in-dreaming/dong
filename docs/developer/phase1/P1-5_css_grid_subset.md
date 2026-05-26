# P1-5 — CSS Grid 子集

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 1 P1-5
> 既有讨论：[`docs/developer/wip/backlog.md`](../specific/wip项.md) § grid
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

让 dong 支持**够游戏 editor / inventory / 工具栏使用**的 CSS Grid 子集，不追求完整 Grid spec：

- `display: grid` / `inline-grid`
- `grid-template-columns` / `grid-template-rows`：`px / % / fr / minmax / auto / repeat()`
- `grid-row` / `grid-column` (start/end / span)
- `grid-row-gap` / `grid-column-gap` / `gap`
- `justify-items` / `align-items` / `justify-self` / `align-self`
- `place-items` / `place-self` 缩写

不在 v1 范围（详见 § 7）：`grid-template-areas`、命名 line、auto-flow `dense`、subgrid、隐式 grid 大量复杂语义。

---

## 2. 现状

| 项 | 状态 |
|---|---|
| `display: grid` | ❌（[`html_css_dom_草案.md`](../specific/html_css_dom_草案.md) § 3.3） |
| Yoga 上游 PR #1865 | 进行中，未合 |
| Flex / Block / Inline 已有 | ✅ |

代码索引：
- 布局：`dong/src/layout/layout_engine.{hpp,cpp}`（Yoga wrapper）
- CSS 解析：`dong/src/dom/css/css_parser.cpp` `css_value_parser.cpp`
- ComputedStyle：`dong/src/dom/css/computed_style.{hpp,cpp}`

---

## 3. 设计

### 3.1 双轨策略

```
display: grid 元素
   │
   ▼
  if (Yoga 已合 PR #1865 && 子 Grid 特性都覆盖)
       → 走 Yoga grid （原生路径）
  else
       → 走 dong 自研 GridEngine （fallback）
```

dong 不绑死 Yoga 进度——自研 GridEngine 作为生产路径，Yoga grid 作为后续替代候选。

实施上**先做自研**：可控、易裁剪、不阻塞业务。

### 3.2 自研 `GridEngine`

新增：`dong/src/layout/grid_engine.{hpp,cpp}`

输入：

- 容器尺寸 (`available_width`, `available_height`)
- `grid-template-columns/rows`：解析后的 track list（`vector<TrackSize>`）
- `gap`
- 子项：每个子有 `column-start/end`、`row-start/end`（含 `auto` / `span N`）

输出：

- 每个子项的 `(x, y, width, height)`，喂回 `LayoutEngine`。

#### 3.2.1 Track 数据模型

```cpp
struct TrackSize {
    enum class Kind : uint8_t {
        Fixed,     // px
        Percent,   // % of container
        Fr,        // 1fr / 2fr ...
        Auto,      // 内容驱动
        MinMax,    // minmax(min, max)
    };
    Kind kind;
    float value;             // px / pct / fr count
    float min_value, max_value; // for MinMax
};

struct GridTemplate {
    std::vector<TrackSize> columns;
    std::vector<TrackSize> rows;
    float gap_x = 0, gap_y = 0;
};
```

#### 3.2.2 子项放置算法（简化版）

参考 [W3C CSS Grid Layout Module Level 1](https://www.w3.org/TR/css-grid-1/) §  Placement Algorithm。

**v1 简化**：

1. **显式放置优先**：所有有 `grid-row/column` 显式的子项先按指定位置放置。
2. **隐式行为**：剩余子项按 DOM 顺序填入空闲格子（auto-flow `row`，不支持 `dense`）。
3. **冲突**：两个显式占同格 → 后者覆盖（log warn）。

#### 3.2.3 Track 大小算法（简化版）

```
1. 分配 fixed (px) 与 percent (%) → 直接定值
2. 分配 auto track：
   - 每行/列扫子项 max-content size，取最大
3. 分配 fr track：
   - free_space = available - sum(fixed + percent + auto + gaps)
   - fr_unit = free_space / sum(fr counts)
   - fr_unit ≥ 0
4. 应用 minmax(): clamp 到 min/max
5. 累计前缀和得到每行/列的起始坐标
```

复杂度：O(rows × cols + N)，N = 子项数。100 单元格 < 0.5 ms。

#### 3.2.4 子项对齐

`justify-self/align-self` / `justify-items/align-items` 在子项级别 `start | end | center | stretch`：

```
cell_rect = (x_track, y_track, w_track, h_track)
child_size = compute_child_intrinsic_size_or_set_size()
apply justify/align → final (x, y, w, h)
```

### 3.3 与 Yoga 的协作

`LayoutEngine::calculateLayout` 走 Yoga 时遇 `display: grid`：

```cpp
if (style.display == DisplayKind::Grid) {
    // 1. 把当前 Yoga 节点先按 block 提供 (available_w, available_h)
    // 2. GridEngine 接管：拿到所有子节点 → 计算每个子的最终 rect
    // 3. 把每个子的 rect 写回 Yoga 节点（YGNodeStyleSetWidth / Height +
    //    YGNodeStyleSetPositionType(Absolute) + YGNodeStyleSetPosition(...)）
    // 4. Yoga 走 absolute layout 路径，不再干预
    grid_engine.layout(node, available);
}
```

**Yoga 内嵌**：把 grid 子项视为 absolute-positioned，子项内部还能继续走 flex / block / grid（递归 grid 时再走 GridEngine）。

### 3.4 CSS 解析

#### 3.4.1 `grid-template-columns`

```
grid-template-columns: 100px 1fr 2fr;
grid-template-columns: repeat(3, 1fr);
grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));   ← v1 支持 auto-fill / auto-fit
grid-template-columns: 100px [name1] 1fr [name2 name3];          ← 命名 line v1 解析但忽略
```

`repeat()` 解析时直接展开为多个 TrackSize（`auto-fill / auto-fit` 在 layout 阶段计算实际重复数）。

#### 3.4.2 `grid-row / grid-column`

```
grid-column: 1 / 3;            // start=1, end=3
grid-column: span 2;           // 任意起点 + span 2
grid-column: 1 / span 2;       // start=1, span=2
grid-column-start: auto;       // 由放置算法决定
```

#### 3.4.3 `gap`

```
gap: 10px;                     // 双轴
gap: 10px 20px;                // row-gap column-gap
row-gap: 10px;
column-gap: 20px;
```

#### 3.4.4 `place-items` / `place-self`

```
place-items: center;           // align-items + justify-items
place-items: start end;        // align-items=start, justify-items=end
```

### 3.5 与 Style 继承 / inherit / important 协调

复用 P0-8 的 cascade 修复（`inherit/initial/unset` + `_explicitly_set` 标记）；GridEngine 不引入新 cascade 逻辑。

### 3.6 与 P0-6 增量布局

`grid-template-*` 改 → invalidate kind = Layout，整 grid 容器 + 所有子项重算。

子项 `grid-row/column` 改但不影响其他子项位置 → 升级到 grid 容器（Layout）；不能局部，因为 placement 算法整体性强。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — CSS 解析（不实现 layout）** | `grid-template-columns/rows` `grid-row/column` `gap` `place-*` 全集解析进 ComputedStyle；DevTools 可见 |
| **S2 — GridEngine v0：固定 + fr** | `display: grid` + `grid-template-columns: 100px 1fr 1fr` 跑通；3-列布局正确 |
| **S3 — `repeat()` + `auto` track** | 含 `repeat(3, 1fr)` `auto` track |
| **S4 — `minmax()` + `auto-fill`/`auto-fit`** | 响应式 grid（典型 inventory） |
| **S5 — `grid-row`/`column` 显式 + span + auto-flow** | 全 placement 算法 |
| **S6 — `justify/align-items/self` + `place-*` 缩写** | 子项对齐 |
| **S7 — Baseline 用例 + Chrome diff** | 像素验证 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `display: grid` 基础三列布局像素与 Chromium baseline diff | < 1% |
| `repeat(3, 1fr) + gap: 16px` 等分像素一致 | 必须 |
| `repeat(auto-fill, minmax(120px, 1fr))` 在不同容器宽度下行为符合 spec | 必须 |
| `grid-row/column: span N` 显式占位正确 | 单元 |
| `gap` `place-items` 缩写 | 必须 |
| Layout 时间（100 cells） | < 1 ms |
| 嵌套 grid（grid 内子也是 grid） | 工作 |
| 与 flex 兄弟容器混用 | 工作 |
| baseline 像素回归（既有 flex / block 用例）| ≥ 99% 通过（不破坏其他布局） |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 像素 diff < 0.5%（与 Chrome） | 期望 |
| 100 cell layout 时间 | < 0.5 ms |
| 1000 cell layout 时间 | < 5 ms |
| `auto` track 含 max-content 计算 | 与 Chrome 一致 |

### 5.3 必须新增的测试用例

| 文件 | 验证 |
|---|---|
| `test_grid_basic_3col.html` | 100px 1fr 1fr |
| `test_grid_repeat.html` | repeat(3, 1fr) |
| `test_grid_repeat_auto_fill.html` | auto-fill + minmax |
| `test_grid_explicit_placement.html` | grid-row/column 显式 + span |
| `test_grid_auto_flow.html` | 隐式放置 |
| `test_grid_auto_track.html` | `auto` track + 内容驱动 |
| `test_grid_gap.html` | gap / row-gap / column-gap |
| `test_grid_align_self.html` | justify/align-items/self |
| `test_grid_inventory.html` | 6×6 inventory 实战 |
| `test_grid_responsive.html` | 容器宽度变化（resize） |
| `test_grid_nested.html` | grid 嵌套 grid |
| `test_grid_in_flex.html` | flex 父级里有 grid |
| `test_grid_perf_1000.html` | perf 用例 |

每个用例必须有 Chromium baseline 入库。

### 5.4 验证命令

```bash
# 像素回归
python dong/scripts/tools/run_baseline_compare.py --filter test_grid_

# Layout perf
DONG_BENCH_AUTOSTOP=1 dong_app.exe --html data/tests/test_grid_perf_1000.html
python dong/tmp/trace_summary.py <trace>.json --top 10 | grep -i grid

# 与 Yoga PR #1865 行为对比（v2，留 issue）
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| Grid spec 复杂，子集裁剪后业务用了不支持的特性 | 文档明示已支持 / 不支持清单；解析阶段未支持的语法在 console.warn |
| 与 Yoga 的 absolute 子节点路径不兼容 | S2 完成后立即跑 mixed flex+grid 用例；不通过则 LayoutEngine 内 grid 节点单独走 |
| Chrome / Firefox 行为差异（spec 含歧义场景）| baseline 以 Chromium 为准；明确 dong 与 Chrome 对齐 |
| 自研后又上 Yoga grid，双实现维护负担 | 自研放在 `grid_engine.cpp` 单文件 < 1500 行；以后 Yoga 稳定可一键切换 |
| `grid-template-areas` 业务方期望支持 | 文档明示 v1 不支持；v2 评估 |
| Performance：repeat 展开后 track 数量爆炸 | repeat count > 1000 时 console warn + clamp |

---

## 7. 不在本方案范围（v1 不支持）

- ❌ `grid-template-areas`（命名区域；v2 评估）
- ❌ Subgrid (`subgrid` 关键字)
- ❌ Auto-flow `dense`（向前回填；很少业务真的需要）
- ❌ 命名 line（`[name1] 1fr [name2]`）
- ❌ `grid-area` 简写
- ❌ Baseline alignment (`align-items: baseline` 在 grid 上；flex 已支持，grid 不做)
- ❌ Masonry layout (`grid-template-rows: masonry`；CSS WG 未定稿)

---

## 8. 完成后更新

- [ ] `docs/reference/css-subset.md` § 3.3 grid 行从 ❌ 改 ✅（注明子集）
- [ ] `docs/developer/wip/backlog.md` § grid 表格更新
- [ ] `docs/reference/features-index.md` 新增 § "CSS Grid 子集"
- [ ] `docs/roadmap.md` Phase 1 P1-5 状态 ✅
- [ ] `docs/developer/perf-budget.md` 新增 grid layout 时间指标
