# P0-2 — 9-slice / nine-patch Panel

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 0 P0-2
> 性能门槛：[`doc/perf_budget.md`](../perf_budget.md) § 3.1
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

支持游戏 UI 普遍使用的 9-slice panel 渲染：

- 一张 panel 贴图 → 4 角不变形 + 4 边按规则 stretch / tile + 中心 stretch / tile / cover。
- 通过 CSS `border-image` 标准属性触发，**不引入私有属性**。
- 复用 P0-1 的 Uber Quad 路径（增加 material type 5），**单 panel 一条 GPU draw call**。

---

## 2. 现状

- `doc/specific/html_css_dom_草案.md` 标 `border-image` 未列入支持。
- 当前若需要 panel 视觉，业务方常用的方式是：
  - 拆 9 张 `<img>` 手摆（笨）
  - 大图整张 stretch（变形）
  - CSS `border` + `border-radius` + 渐变 凑（无法表达美术效果）
- 代码侧无 nineslice 路径；`background-image` 走的是普通 image quad（无 9 区分）。

---

## 3. 设计

### 3.1 CSS 触发：`border-image`（W3C 标准子集）

支持子集（v1）：

```css
.panel {
  border-image-source: url("panel.png");
  border-image-slice: 32 32 32 32;        /* 上 右 下 左，单位 px（也支持百分比） */
  border-image-width: 32px;               /* outer border-width；可缩写 */
  border-image-repeat: stretch | repeat | round;  /* 一/两值 */
  /* simplified shorthand: */
  border-image: url("panel.png") 32 / 32px stretch;
}
```

**v1 不支持**：`border-image-outset`（panel 外延绘制）、`fill` 关键字下的"中心也绘"语义之外的复杂组合。

> "中心是否绘制"：CSS 默认中心**不绘**；加 `fill` 关键字才绘。dong 的游戏 panel 几乎都需要中心，**默认 `fill`**（与浏览器不同，文档中明确），可显式 `border-image-slice: ... no-fill` 关闭。

### 3.2 内部数据结构

```cpp
// dong/src/render/display_list.hpp（新增）
struct NineSliceItem {
    Rect dest_rect;            // panel 外框（内容区 + border-image-width）
    std::string image_src;     // 贴图路径
    float slice_top, slice_right, slice_bottom, slice_left;     // 源图 9 区切分
    float width_top, width_right, width_bottom, width_left;     // 目标 border 宽
    enum class Repeat : uint8_t { Stretch, Repeat, Round };
    Repeat repeat_h = Repeat::Stretch;
    Repeat repeat_v = Repeat::Stretch;
    bool fill_center = true;
    float opacity = 1.0f;
};
```

### 3.3 GPU 路径：Uber Quad material 5

复用 P0-1 的 instance buffer：

| 字段 | 含义 |
|---|---|
| `rect` | dest_rect（panel 外框） |
| `radii` | `{slice_top, slice_right, slice_bottom, slice_left}` |
| `params.x` | 5（material = nineslice） |
| `params.y` | repeat_h（packed: 0=stretch / 1=repeat / 2=round） |
| `params.z` | repeat_v |
| `params.w` | fill_center ? 1 : 0 |
| `grad0` | `{width_top, width_right, width_bottom, width_left}` |
| `grad1` | `{src_w, src_h, atlas_uv_x, atlas_uv_y}` 贴图 atlas 信息 |

`color` 字段复用为 tint（默认 1,1,1,opacity）。

### 3.4 Shader 草案

```hlsl
// material == 5：在 fragment 内部分九区采样
float4 nineslice_fs(VS_Out i) {
    float2 dst_pos = i.local_uv * i.rect.zw;  // 像素位置 in dst rect
    float4 widths = i.grad0;                   // top, right, bottom, left
    float4 slices = i.radii;                   // top, right, bottom, left
    float2 src_size = i.grad1.xy;
    float2 atlas_uv = i.grad1.zw;

    // 1. 判定 9 区：(col, row) ∈ {(0,0)..(2,2)}
    int col = (dst_pos.x < widths.w) ? 0 : (dst_pos.x > i.rect.z - widths.y) ? 2 : 1;
    int row = (dst_pos.y < widths.x) ? 0 : (dst_pos.y > i.rect.w - widths.z) ? 2 : 1;

    if (col == 1 && row == 1 && i.params.w < 0.5) discard;  // !fill_center

    // 2. 计算源图 UV：4 角直接映射；4 边按 repeat 模式映射；中心同 4 边
    float2 uv = compute_nineslice_uv(col, row, dst_pos, widths, slices, src_size, i.rect, repeat_h, repeat_v);

    return SAMPLE_ATLAS(atlas_uv + uv) * i.color;
}
```

**Round 模式实现**：`tile_count = round(dst_size / src_tile)`；至少 1；scale = dst_size / (tile_count * src_tile)。

---

## 4. 实施步骤

| Step | 范围 | 说明 |
|---|---|---|
| **S1 — CSS 解析** | `border-image-*` 属性 + shorthand 解析；填入 `ComputedStyle` | 单元测试：parser 各种语法都能正确分解 |
| **S2 — Painter emit** | DOM 元素若 `border-image-source` 非空，`Painter` emit `NineSliceItem`（暂时仍用旧 path 拆 9 quad） | `test_nineslice_basic.html` baseline 出来 |
| **S3 — Uber material 5** | shader + GPUCompiler 编码 instance；旧 path 切走 | 单 panel 1 draw call 验证；像素与 S2 一致 |
| **S4 — repeat / round / fill 完整支持** | shader 内 repeat 数学；测试用例补全 | 像素与 Chromium baseline diff < 1% |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `test_nineslice_basic.html` | 与 Chromium baseline 像素 diff < 1%（容差含子像素） |
| `test_nineslice_repeat.html` `test_nineslice_round.html` | 同上 |
| 单 panel draw call | = 1（含 fill 中心） |
| 旧 image quad path 不被破坏 | `examples/data/tests/` 全集 baseline ≥ 99% 通过 |
| GPU validation | 0 error |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 100 个 panel 同帧 | ≤ 3 draw call（合批后） |
| 切到 nineslice 渲染 vs 拆 9 个 image quad | GPU 时间下降 ≥ 30% |

### 5.3 必须新增的测试用例

放在 `dong/examples/data/tests/`：

| 文件 | 验证点 |
|---|---|
| `test_nineslice_basic.html` | stretch 四边 + fill 中心 |
| `test_nineslice_repeat.html` | repeat 四边 |
| `test_nineslice_round.html` | round 模式整数倍 tile |
| `test_nineslice_no_fill.html` | 中心透明 |
| `test_nineslice_asymmetric.html` | 不等切分（如 `slice: 16 32 48 8`） |
| `test_nineslice_panel_grid.html` | 100 个相同 panel 验证合批 |

每个用例必须有对应 Chromium baseline（`scripts/tools/html_baseline_render.py` 跑一次入库）。

### 5.4 验证脚本

```bash
# 像素 diff
python dong/scripts/tools/run_baseline_compare.py --filter test_nineslice_

# Draw call 验证（draw 数应 = panel 数）
DONG_BENCH_AUTOSTOP=1 dong_app.exe --html data/tests/test_nineslice_panel_grid.html
python dong/tmp/trace_summary.py <trace>.json --top 30 | grep UberQuad
# 期望：1 行，instance_count = 100
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| Repeat / round 在 fragment 内做 floor / fmod，浮点精度边界 1px 缝隙 | shader 内统一加 `0.5px` 偏移；测试覆盖 panel 边长不能整除 tile 的 case |
| 大 panel + repeat 模式 fragment 算力上升 | 通过 mip-mapping + 预生成 tiled 材质 fallback；用 `auto_profile_loop` 度量 |
| atlas 内 panel 贴图与其他贴图共享时 UV 越界 | 使用 atlas allocator 时为 nineslice 贴图保留 1px padding，shader 内 clamp UV |
| 旧 path 不删除导致心智负担 | S3 完成后立即删旧 path |

---

## 7. 不在本方案范围

- ❌ `border-image-outset`（外延绘制；游戏 panel 极少用）
- ❌ SVG `border-image`（与 P0-3 mask / SVG 路径合并讨论）
- ❌ "12-slice" / 自由切分（暂无强需求）
- ❌ 与 `border-radius` 同时启用（本来就是互斥语义）

---

## 8. 完成后更新

- [ ] `doc/specific/html_css_dom_草案.md` § 3.1 把 `border-image` 标 ✅
- [ ] `doc/重要特性.md` 新增 § "9-slice Panel" 段
- [ ] `doc/perf_budget.md` § 4 atlas 估算更新
