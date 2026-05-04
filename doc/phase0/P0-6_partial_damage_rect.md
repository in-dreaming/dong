# P0-6 — Display List 局部失效（Partial Damage Rect）

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 0 P0-6
> 性能门槛：[`doc/perf_budget.md`](../perf_budget.md) § 3.2 / § 3.6
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

把当前的"全或无"渲染策略升级为三档：

| 现状 | 改造后 |
|---|---|
| **干净帧** → present-only fast path（已有，~15× 加速）| 保留 |
| **任何变化** → 全树 layout + paint + 全命令重建 | 改为：**仅 dirty 子树重建命令**，其他子树复用上一帧 |

收益：editor / 复杂业务面板（场景 S2 / S6）下，单 panel 拖动 / 单 inspector field 输入时**避免整树重建**，达到 [`perf_budget.md`](../perf_budget.md) S2 "拖动 panel 时整树重建次数 = 0" Hard 门槛。

---

## 2. 现状

代码：`dong/src/core/engine_view.cpp`

```cpp
bool commands_dirty_ = true;
bool commands_regenerated_this_tick_ = false;

void markNeedsRepaint() { commands_dirty_ = true; }

void tickGenerateCommandsIfNeeded() {
    if (!commands_dirty_) return;
    // 完整：computeStyles + layout + buildDisplayList + GPUCompiler
    commands_regenerated_this_tick_ = true;
    commands_dirty_ = false;
}
```

`markNeedsRepaint` 在仓库被调 30+ 处（焦点变化、属性 setter、resize、scroll 等），每次都触发**整树**重建。

---

## 3. 设计

### 3.1 引入"Invalidation 类型 + 范围"

替换 `markNeedsRepaint()` 单一调用为：

```cpp
enum class InvalidationKind : uint8_t {
    Style,         // 只影响样式，可不 layout（如 color, opacity 切换）
    Layout,        // 影响布局（width / display / margin）
    Paint,         // 不影响 style/layout，仅重画（如 background-color）
    Geometry,      // hit-test rect 变化，影响 hit/scroll，不一定 paint
};

struct Invalidation {
    DOMNode* root;             // 失效子树根；nullptr = 整 view
    InvalidationKind kind;
    Rect screen_rect;          // 屏幕空间脏矩形（可选优化）
};

class EngineView {
    void invalidate(DOMNode* node, InvalidationKind k);
    // 旧 markNeedsRepaint() = invalidate(nullptr, Paint)
    
    std::vector<Invalidation> pending_invalidations_;
};
```

### 3.2 Invalidation 合并

每个 setter 调用 `invalidate(node, kind)`：

```cpp
void invalidate(DOMNode* n, InvalidationKind k) {
    // 升级规则：Layout > Style > Paint
    // 同子树内多次 invalidate → 合并到最近公共祖先 + 取最高 kind
    pending_invalidations_.push_back({n, k, n->getScreenRect()});
}
```

`tick` 开始时合并：

```cpp
void mergeInvalidations() {
    if (pending.empty()) return;
    // 1. 任一 nullptr → 升级为整树
    if (any_root_null) { full_repaint = true; return; }
    // 2. 合并为最近公共祖先 + 最高 kind（O(N log N) ）
    // 3. 累计 dirty rect union
}
```

### 3.3 Painter 的 sub-tree 重建

`Painter::buildDisplayList(root)` 改为支持子树：

```cpp
class Painter {
    DisplayList full_display_list_;          // 完整缓存
    std::unordered_map<NodeId, Range> node_to_range_;  // 每节点对应 DisplayList 中范围

    void rebuildSubtree(DOMNode* root, InvalidationKind kind) {
        // 1. 找到 root 在 full_display_list_ 中的 [start, end)
        auto [start, end] = node_to_range_[root->id];
        
        // 2. 仅对该子树执行 paint
        DisplayList partial;
        paintNode(root, partial);
        
        // 3. 替换 full_display_list_ 中对应段
        replaceRange(full_display_list_, start, end, partial);
        
        // 4. 更新后续节点的 range（offset shift）
        shiftRanges(start, partial.size() - (end - start));
    }
};
```

### 3.4 Style / Layout 的 sub-tree 重算

#### Layout sub-tree（与 Yoga 协同）

Yoga 已支持 dirty 标记 + `YGNodeMarkDirty`。当前 dong 的策略是"任何 invalidation → 整 root calculateLayout"。

改为：

```cpp
void layoutEngine.calculateLayout(node_root) {
    // root: invalidation 的最近公共祖先
    YGNodeCalculateLayout(yoga_node_for(node_root), ...);
    // Yoga 内部已经只重算 dirty 子树
}
```

约束：sub-tree layout 仅当父级布局未受影响时安全（`width: auto` 子级影响父级，必须升级到父级）。

升级规则（保守）：

```
若 invalidation kind == Layout 且 node 含以下属性：
   width/height = auto / fit-content / max-content / %
   flex-grow / flex-shrink / flex-basis = auto
   position = static / relative
→ 升级 root 到父级（递归），直到遇到 fixed-size 容器或 view root
```

实现：`Painter` / `LayoutEngine` 提供 `isLayoutSelfContained(node)` 查询，invalidate 时即升级。

#### Style sub-tree

`StyleEngine::computeStyles(node_root)` 已支持子树参数；当前调用方传的是整 root，改为传 invalidation root 即可。

继承属性变化（color / font-size / direction 等）→ 必须重算整子树（已是子树调用，OK）。

### 3.5 GPU 命令编译的增量

`GPUCompiler` 当前是一次性遍历 `DisplayList`。改造：

```cpp
class GPUCompiler {
    GPUCommandList full_;
    std::vector<Range> item_to_command_range_;  // displayitem -> commands

    void recompileRange(DisplayList& dl, Range item_range) {
        // 仅对该 item 范围重新生成 commands，替换 full_ 中对应段
        // 类似 Painter 的子树替换
    }
};
```

**注意**：与 P0-1 Uber Quad Pipeline 协同——uber batch 是按命令顺序连续累积的，子树替换会破坏 batch 边界，需要：

- 子树命令替换后，重新走一遍**该子树覆盖区域内**的 batch 划分。
- 实现简化：sub-tree paint 完后 emit 一次 "batch boundary" 标记，原 batch 不跨该子树。

### 3.6 Damage Rect 与 GPU 路径

GPU 端**不**做 scissor partial repaint（intermediate buffer 已经是整帧的，scissor 只是减少 fragment 工作量，对 PC GPU 收益有限，对 Tile-based mobile GPU 反而有害）。

damage rect 的真正用途：
1. **统计与监控**：每帧上报 damage 面积，超过 view 50% 触发 console warn。
2. **Phase 1+ DevTools**：可视化 damage 高亮。
3. **未来 mobile tile-based GPU 优化**：留接口，不强制使用。

**v1 不做 GPU partial scissor**，专注 CPU 侧 layout / paint / compile 增量。

### 3.7 接口与回归保障

```cpp
// 旧 API 仍可用（兼容期）
void markNeedsRepaint() { invalidate(nullptr, InvalidationKind::Paint); }

// 新 API
void invalidate(DOMNode* node, InvalidationKind kind);
```

env var `DONG_FORCE_FULL_REPAINT=1` → 关闭增量，走老路径（紧急回退用）。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — Invalidation 数据结构 + 收集** | 不改实际行为；所有 invalidation 仍升级到整树；新加 metric 上报"实际可省的子树面积" |
| **S2 — StyleEngine sub-tree 重算** | invalidation root 不为 nullptr 时仅重算该子树 styles；Layout 仍整树 |
| **S3 — Layout sub-tree（Yoga）** | 走 Yoga 子树 calculate；含 isLayoutSelfContained 升级 |
| **S4 — Painter sub-tree** | DisplayList 节点范围映射 + 替换 |
| **S5 — GPUCompiler 增量** | 与 P0-1 batch 协同 |
| **S6 — 真实接入：refactor markNeedsRepaint 调用点** | 30+ 调用处按场景选合适 InvalidationKind |
| **S7 — DevTools metric + 默认开启 + 删旧路径** | 删除 env var fallback |

> S1–S6 可与 P0-1 并行；S7 必须在 P0-1 之后。

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `dock_demo` 拖动一个 panel header 时 | `commands_regenerated_this_tick_ = false`（即整树未重建）；damage rect 局限于该 panel + 镜像 panel |
| 单个 input 输入字符 | layout 重算节点数 ≤ 该 input 子树；不触发 root layout |
| baseline 像素回归 | `examples/data/tests/` 全集 ≥ 99% 通过（与 full repaint 路径像素一致） |
| 切到 `DONG_FORCE_FULL_REPAINT=1` 仍可工作 | 必须 |
| 与 P0-1 Uber Quad 共存无 batch 错乱 | `transform_test` 等含层叠场景像素一致 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| `dock_demo` 拖 panel header 帧 layout+paint 时间 | ≤ 当前完整重建的 30% |
| editor 100 个 inspector field 中改 1 个 | 重算节点数 < 5 |
| 同帧内多次 invalidate 合并比 | 合并到 LCA + Kind 最高，**不重复**计算 |
| Damage rect 平均面积 | 拖 panel 场景下 < view 30% |

### 5.3 必须新增的测试

#### 单元 / 集成

```cpp
// dong/tests/test_invalidation.cpp（新增）
TEST(Invalidation, SinglePropertyChange) {
    auto eng = setup_with(R"(
        <div id="root">
          <div id="panel-a"><span>A</span></div>
          <div id="panel-b"><span>B</span></div>
        </div>)");
    
    InvalidationStats stats; eng->setStatsSink(&stats);
    
    eng->getElementById("panel-a")->setStyle("background", "red");
    eng->tick();
    
    EXPECT_EQ(stats.style_recomputed_nodes, count_in_subtree("#panel-a"));
    EXPECT_EQ(stats.layout_recomputed_nodes, 0);  // background 不影响 layout
    EXPECT_FALSE(stats.full_repaint);
}

TEST(Invalidation, LayoutPropagatesUpForAutoSize) {
    // panel-a 是 width:auto；改 panel-a 子元素 → 升级到 panel-a 父级
}

TEST(Invalidation, MultipleMergedToLCA) {
    // 同帧改 panel-a 和 panel-b → 合并到 root
}
```

#### 性能回归

加入 `perf_baseline.py`（P0-7）：

| 场景 | 度量 |
|---|---|
| `bench_dock_drag` (脚本驱动 60 帧拖动) | 平均 layout / paint / compile 时间 |
| `bench_inspector_input` (脚本输入字符) | 同上 + 重算节点数 |
| `bench_full_repaint_baseline` (env var force full) | 作对照 |

CI 比较：增量路径 ≤ full 路径 30%。

### 5.4 验证命令

```bash
# 单元
cd dong && zig build run-feature-tests --filter Invalidation

# 像素回归
python dong/scripts/tools/run_baseline_compare.py

# 性能
python dong/scripts/tools/perf_baseline.py --scene S6_dock_drag
python dong/scripts/tools/perf_baseline.py --scene S6_inspector_input

# 与上一次 main 对比
python dong/scripts/tools/perf_baseline.py --diff main
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| sub-tree 边界判定错（漏升级）→ 视觉不一致 | `DONG_FORCE_FULL_REPAINT=1` 紧急回退；像素 baseline 全集是看门狗；S6 完成前都开 fallback 验证 |
| 节点 → DisplayList range 映射在动态插删时维护成本高 | 用 stable id + 范围链表；O(log N) 维护 |
| 与 P0-1 uber batch 协同时跨子树边界破坏 batch | 子树替换强制 batch 边界；统计平均 batch 大小，下降 ≤ 30% 是可接受 |
| Yoga sub-tree calculate 在嵌入 sticky / aspect-ratio 等已有特性时表现异常 | 这些特性的测试用例（`test_sticky_*` / `test_aspect_ratio_*`）必须全过 |
| 内存占用：要保留 full DisplayList + 范围映射 | DisplayList 已经在 cache，新增映射 O(N)，总开销可控；监控 metric 上报 |

---

## 7. 不在本方案范围

- ❌ GPU 端 partial scissor（移动 GPU 优化，留 Phase 2+）
- ❌ Animation tick 的局部更新（动画已有独立路径，本任务不动）
- ❌ Layer compositing 重排（只在层内增量，跨层不优化）
- ❌ Direct Draw 路径（Direct Draw 本身是单条命令，不需要增量）
- ❌ Scene Graph 路径（O(1) 更新已有）

---

## 8. 完成后更新

- [ ] `doc/perf_budget.md` § 3.2 / § 3.6 当前实测列
- [ ] `doc/重要特性.md` 新增 § "增量重建（Damage Rect）"
- [ ] `doc/summary/architecture.md` 一帧数据流更新
