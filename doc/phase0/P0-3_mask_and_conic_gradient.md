# P0-3 — CSS mask + conic-gradient

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 0 P0-3
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

让 dong 能渲染游戏 UI 高频形态：**圆形血条、技能冷却扇形、雷达扫描、环形进度**。这些靠两个 CSS 特性：

1. **`mask-image` / `mask`**（标准 CSS Masking Module Level 1）
2. **`conic-gradient()`**（CSS Images Module Level 4）

通常组合使用：

```css
.cooldown {
  background: rgba(0, 0, 0, 0.7);
  mask-image: conic-gradient(black 0% 60%, transparent 60% 100%);
}
```

---

## 2. 现状

- `conic-gradient`：[`doc/specific/html_css_dom_草案.md`](../specific/html_css_dom_草案.md) § 3.7 标"⚠️可演进"，**未实现**。
- `mask` / `mask-image`：未列入支持。
- `clip-path`（基础形状）已部分支持，但圆形进度需要的"按角度遮罩"靠 clip-path 做不出。

---

## 3. 设计

### 3.1 conic-gradient

#### 3.1.1 CSS 解析

支持子集（v1）：

```css
conic-gradient(<color-stop>, <color-stop>, ...)
conic-gradient(from <angle>, ...)
conic-gradient(from <angle> at <position>, ...)
repeating-conic-gradient(...)
```

复用 `linear-gradient` 已有的 stop 解析；新增 `from` / `at` 解析。

#### 3.1.2 GPU 路径：独立 material（不进 Uber）

理由：conic-gradient 在 fragment 内做 `atan2`，与 uber 的 solid / rounded 分支差异大；且 stop 数量动态，会撑爆 instance 字段。

新增 pipeline `conic_gradient_pipeline`。Instance 数据：

```hlsl
struct ConicGradientInstance {
    float4 rect;
    float4 center_angle;     // [xy]=center px, [z]=from_angle (rad), [w]=stop_count
    float4 stops[8];         // packed: [x]=position(0..1), [yzw]=rgb（alpha 共用）
    // 简化：v1 限制 stop 数 ≤ 8；超出截断
};
```

Shader：

```hlsl
float angle_at(float2 px, float2 center, float from) {
    float a = atan2(px.y - center.y, px.x - center.x); // -pi..pi, 0 = +x
    a = (a + 1.5708 - from) / 6.2832;                  // 0..1，0 = 顶部
    return frac(a + 1.0);
}
float4 conic_fs(VS_Out i) {
    float t = angle_at(i.pos, i.center_angle.xy, i.center_angle.z);
    return interpolate_stops(t, i.stops, (int)i.center_angle.w);
}
```

#### 3.1.3 用作 background-image

走 `Painter` 已有的 background path；emit 一条 `ConicGradientCommand`。

### 3.2 mask-image / mask

#### 3.2.1 CSS 子集（v1）

```css
mask-image: <image>;       /* url() | linear-gradient() | radial-gradient() | conic-gradient() */
mask-mode: alpha | luminance;        /* default: alpha */
mask-repeat: no-repeat | repeat | space | round;
mask-position: <position>;
mask-size: <bg-size>;
mask-clip: border-box | padding-box | content-box;

/* 缩写 */
mask: <image> [<position>/<size>] <repeat>;
```

不支持（v1）：`mask-composite`、SVG `<mask>` 元素引用、多 mask 叠加。

#### 3.2.2 GPU 路径：通用离屏 mask pass

mask 与现有 `BeginIsolatedLayer` 相似，但增加"采样另一个图源作为 alpha"的步骤。

```
PaintWithMask(element):
  1. BeginIsolatedLayer(layer_id, rect, opacity=1.0)    ← 元素本体绘到离屏 RT
  2. ... 元素正常绘制 ...
  3. EndIsolatedLayer
  4. ApplyMask(layer_id, mask_source)                   ← 新命令
        - 二次 pass，bind layer texture as base，bind mask as alpha source
        - 输出到上层（或下一个层）
```

**新增 GPUCommand**：

```cpp
struct ApplyMaskCommand {
    uint64_t layer_id;
    enum class Source : uint8_t { Image, LinearGradient, RadialGradient, ConicGradient };
    Source source_type;
    // image：image_src + uv_rect
    // gradient：复用 linear/radial/conic 的 instance 字段
    enum class Mode : uint8_t { Alpha, Luminance };
    Mode mode;
};
```

**Shader（mask_apply_fs.hlsl）**：

```hlsl
float4 mask_apply(VS_Out i) {
    float4 base = SAMPLE_LAYER(i.uv);
    float a = (mode == ALPHA) ? SAMPLE_MASK(i.mask_uv).a
                              : luminance(SAMPLE_MASK(i.mask_uv).rgb);
    return float4(base.rgb, base.a * a);
}
```

#### 3.2.3 mask + conic-gradient 组合（高频用例）

特殊优化：`mask: conic-gradient(...)` + 简单背景（solid color / linear gradient）的组合，可以**跳过离屏层**，直接在 conic shader 内 fold 进 base 颜色：

```
if (background is solid color && mask is conic-gradient):
    emit single ConicMaskedRectCommand    ← material 6（uber 的扩展）
    1 draw call 完成
```

CD 圆形进度、技能冷却条等几乎都命中此 fast path。**这是性能关键点**。

### 3.3 与 P0-1 Uber Quad 的协调

| 组合 | 路径 |
|---|---|
| `background: conic-gradient(...)` | 独立 conic pipeline |
| `mask: conic-gradient(...)` + solid background | uber material 6（fast path） |
| `mask: conic-gradient(...)` + 复杂内容 | 通用 mask pass（离屏 + apply） |
| `mask-image: url(...)` | 通用 mask pass |

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — conic-gradient 解析 + pipeline** | 仅 `background-image: conic-gradient(...)` 走通；无 mask；`test_conic_basic.html` 出图 |
| **S2 — mask 解析 + 通用 mask pass** | 离屏 + apply 两 pass；`test_mask_image.html` 出图 |
| **S3 — fast path: conic mask + solid bg** | uber material 6；`test_cooldown_fast.html` 单 draw call 验证 |
| **S4 — repeating-conic + radial mask + 完整 baseline** | 全集像素回归 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `test_conic_basic.html` | 与 Chromium baseline diff < 1% |
| `test_mask_image.html` | 同上 |
| `test_cooldown_circular.html`（mask + conic）| 同上 |
| Fast path（conic mask + solid bg）draw call | = 1 |
| 通用 mask pass（含离屏） | ≤ 3 GPU pass per element（含 BeginLayer / Element / Apply） |
| baseline 全集像素回归 | ≥ 99% 通过 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 同帧 50 个圆形 CD（fast path）| ≤ 5 draw call（合批后） |
| 同帧 50 个 mask + 复杂内容 | ≤ 60 ms / 帧（Reference） |
| conic stop 上限 | ≥ 8（v1 上限），超出在 console warn |

### 5.3 必须新增的测试用例

| 文件 | 验证 |
|---|---|
| `test_conic_basic.html` | conic-gradient 4 stop |
| `test_conic_from_at.html` | from + at 偏移 |
| `test_conic_repeating.html` | repeating-conic |
| `test_mask_image_url.html` | url image mask |
| `test_mask_linear_gradient.html` | linear gradient mask |
| `test_mask_alpha_vs_luminance.html` | mode 切换 |
| `test_cooldown_circular.html` | conic mask + solid bg fast path |
| `test_cooldown_50.html` | 50 个 CD perf 用例 |
| `test_radar_scan.html` | conic + 旋转动画 |

### 5.4 验证命令

```bash
python dong/scripts/tools/run_baseline_compare.py --filter "test_conic_|test_mask_|test_cooldown_|test_radar_"

# Fast path 验证
DONG_BENCH_AUTOSTOP=1 dong_app.exe --html data/tests/test_cooldown_fast.html
python dong/tmp/trace_summary.py <trace>.json --top 20 | grep -E "Conic|Mask|UberQuad"
# 期望：仅 UberQuad（material 6），无 BeginIsolatedLayer
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| `atan2` 在某些移动 GPU 精度不够 | shader 中限制不靠近中心 1px 内取值；中心半径 `< 0.5px` 时直接采 stops[0] |
| stop ≥ 8 截断导致视觉不一致 | console warn；v1 提示业务方先用 8 个；v2 改为纹理 stops |
| 通用 mask pass 离屏 RT 增多 → 内存升高 | RT 共享池（已有 LayerTree 机制）；mask 临时层用最低分辨率 RT |
| mask 与 `transform` 同时使用时 UV 错位 | shader 内 mask UV 不应用 transform，统一在像素空间计算 |
| Slug 字体 + mask 组合 | 短期不支持（mask 内 text 走 MSDF）；文档明示 |

---

## 7. 不在本方案范围

- ❌ `mask-composite`（多 mask 复合）
- ❌ SVG `<mask>` 元素引用
- ❌ `backdrop-filter` 与 mask 组合
- ❌ `mask-border` (mask 的 9-slice)

---

## 8. 完成后更新

- [ ] `doc/specific/html_css_dom_草案.md` § 3.7 / § 3.16 更新 `conic-gradient` `mask` 状态
- [ ] `doc/重要特性.md` 新增 § "Mask + Conic Gradient（圆形 CD/血条）"
- [ ] `doc/perf_budget.md` § 3.1 / § 3.2 更新（mask 离屏 RT 内存）
