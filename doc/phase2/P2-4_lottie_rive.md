# P2-4 — Lottie / Rive 兼容路径

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 2 P2-4
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

让美术输出的 **Lottie**（Bodymovin JSON 序列）与 **Rive**（`.riv` 二进制）可以在 dong 中直接播放，**复用 Slug 已有的贝塞尔曲线 GPU 管线**：

- `<dong-lottie src="...">` 作为 HTML 自定义 replaced 元素
- `<dong-rive src="...">` 同上
- JS API：play / pause / seek / state machine（Rive）
- 与 dong DOM/CSS 协同：宽高 / opacity / transform / clip
- 与 P0-1 Uber Quad 协同：常态贝塞尔曲线 batch 与 Slug 字体共 atlas

不目标：完整 After Effects expression（Lottie 高级特性）、Rive complete state-machine 编辑器。

---

## 2. 现状

| 项 | 现状 |
|---|---|
| Slug 贝塞尔曲线 GPU 管线 | ✅，[`doc/arch/arch_font.md`](../arch/arch_font.md) § 2.3 |
| 矢量图标 / SVG | ❌（[`doc/specific/html_css_dom_草案.md`](../specific/html_css_dom_草案.md) § 2.7 仅基础形状） |
| Lottie 支持 | ❌ |
| Rive 支持 | ❌ |
| 第三方 lottie / rive runtime（C++）| 都有 MIT/Apache 开源 |

---

## 3. 设计

### 3.1 路线选择

两个走向：

| 方案 | 优点 | 缺点 |
|---|---|---|
| **重新实现** Lottie/Rive 解析 + 渲染 | 完全自控 | 工时巨大；维护成本高 |
| **集成开源 runtime + 复用 Slug pipeline** | 工时可控；与上游兼容 | 需要做"runtime → DongDrawList"翻译层 |

**v1 走方案二**：

- Lottie：[`lottie-cpp`](https://github.com/airbnb/lottie-android)（Android C++ 部分）或 [`rlottie`](https://github.com/Samsung/rlottie)（Samsung，MIT）—— **优先 rlottie**：纯 C++ + 无依赖 + 性能好。
- Rive：[`rive-cpp`](https://github.com/rive-app/rive-cpp)（MIT，官方）。

每个 runtime 输出 vector path / paint command；dong 把这些 path 喂给 Slug pipeline（贝塞尔曲线 → curve/band 纹理 → 解析覆盖率渲染）。

### 3.2 架构

```
.json (Lottie) / .riv (Rive)
        │
        ▼
   rlottie / rive-cpp                ← 第三方 runtime（无修改）
        │
        ▼  (PathBuilder 接口：moveTo/lineTo/quadTo/cubicTo/close + paint)
   DongVectorBridge                   ← 新增模块
        │
        ▼
   Slug pipeline                     ← 复用现有 curve/band 上传 + shader
        │
        ▼
   GPU draw call
```

`DongVectorBridge` 把 cubic Bézier → 拆 quadratic（Slug 仅支持二阶）→ 上传 curve atlas。

### 3.3 C ABI

```c
// dong/include/dong_vector.h（新增）
typedef struct DongVectorAnimation DongVectorAnimation;

typedef enum {
    DONG_VECTOR_LOTTIE = 0,
    DONG_VECTOR_RIVE = 1,
} DongVectorFormat;

DongVectorAnimation* dong_vector_load(dong_engine_t* eng,
                                       DongVectorFormat fmt,
                                       const void* data, size_t len);

void dong_vector_destroy(DongVectorAnimation* va);

// 时间控制
double dong_vector_duration(DongVectorAnimation* va);
void   dong_vector_play(DongVectorAnimation* va);
void   dong_vector_pause(DongVectorAnimation* va);
void   dong_vector_seek(DongVectorAnimation* va, double time_seconds);
void   dong_vector_set_loop(DongVectorAnimation* va, int loop);
void   dong_vector_set_speed(DongVectorAnimation* va, float speed);

// 渲染
void dong_vector_tick(DongVectorAnimation* va, float dt);
void dong_vector_draw(DongVectorAnimation* va, float x, float y, float w, float h);
//  draw 把命令写入当前 view 的 DisplayList，作为 Direct Draw 路径

// Rive state machine（v1 仅基础 trigger / bool / number 输入）
typedef struct {
    const char* state_machine_name;
} DongRiveStateMachineConfig;

int dong_vector_rive_play_state_machine(DongVectorAnimation* va,
                                        const DongRiveStateMachineConfig* cfg);
int dong_vector_rive_set_input_bool(DongVectorAnimation* va, const char* name, int value);
int dong_vector_rive_set_input_number(DongVectorAnimation* va, const char* name, double value);
int dong_vector_rive_fire_trigger(DongVectorAnimation* va, const char* name);
```

### 3.4 HTML 元素

`<dong-lottie>` / `<dong-rive>` 作为 replaced element（与 `<host-view>`、`<img>` 同枝）：

```html
<dong-lottie
  src="loading.json"
  width="200" height="200"
  loop="1"
  speed="1.5"
  autoplay="1"></dong-lottie>

<dong-rive
  src="character.riv"
  state-machine="Walk"
  width="400" height="400"></dong-rive>
```

JS API：

```js
const lt = document.querySelector('dong-lottie');
lt.play();
lt.pause();
lt.seek(1.5);
lt.duration;       // getter

const rv = document.querySelector('dong-rive');
rv.setInput('isWalking', true);
rv.fireTrigger('Jump');
```

### 3.5 渲染管线协同

**Slug 复用**：rlottie / rive-cpp 输出的 path 都是 cubic Bézier。

```
cubic curve → 自适应细分为 N 段 quadratic → 上传 curve atlas (Slug 已有格式)
            → build band texture
            → instanced quad draw with Slug shader (复用 slug_text_*.hlsl)
```

差异：
- Slug 文字是 EM 空间归一化；vector animation 是任意 px 空间。在 vertex shader 里 scale/transform。
- 颜色不再来自 GlyphRun；改为 path-level paint（solid color / linear gradient / radial gradient）。

**结果**：vector animation 与文字共享 GPU pipeline + atlas，资源开销低。

### 3.6 缓存策略

| 静态部分 | 缓存 |
|---|---|
| Path geometry (在动画时间轴上不变的 path) | 上传一次到 curve atlas，永久驻留 |
| Per-frame path 变形（mask / morph）| 每帧重新上传该 path 段（atlas slot 复用） |
| 线性 gradient 等 paint | per-frame uniform 数据；不入 atlas |

`dong_vector_tick` 内部判定 dirty path，最小化上传。

### 3.7 与 P0-1 Uber Quad 的关系

Vector animation **不**走 Uber Quad（material 类型不一样），独立 batch；但与 Slug text 共享：

- 同 view 内的"vector quad" + "slug text glyph quad" 可合 batch（共享 shader pipeline）。
- 仅切 instance buffer，无 pipeline 切换。

### 3.8 性能预期

| 场景 | 目标 |
|---|---|
| 5 个 1080p Lottie 动画同帧 | ≥ 60 FPS |
| 1 个 4K Rive state machine | ≥ 60 FPS |
| 100 个小 icon Lottie（载入提示）| ≥ 60 FPS |
| 单 Lottie 解析 + 首次 path build | < 50 ms |

主要 CPU 开销来自 rlottie/rive-cpp 内部 (animation evaluation)；GPU 开销由 Slug pipeline 摊薄。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — rlottie 集成 + 第三方构建** | zig build 接入；最小 demo 跑出 path |
| **S2 — DongVectorBridge：cubic → quadratic + curve atlas 上传** | 单 path 渲染正确 |
| **S3 — Lottie 完整：多 layer + paint + tick** | 跑通 1 个真实 Lottie 设计稿 |
| **S4 — `<dong-lottie>` HTML 元素 + JS API** | 与 DOM 协同 |
| **S5 — rive-cpp 集成 + state machine 基础** | 跑 rive 官方 demo |
| **S6 — `<dong-rive>` HTML 元素 + JS API** | 同上 |
| **S7 — Perf 调优 + 缓存** | 达到 § 3.8 目标 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 5 个真实 Lottie 设计稿渲染像素与 web Lottie player（lottie-web）≤ 5% diff | baseline |
| 1 个真实 Rive 文件 state machine 输入正确切换状态 | demo |
| 与 dong CSS `transform`、`opacity`、`clip-path: inset` 协同正常 | 像素 |
| 多 view 共享 Slug atlas 时无冲突 | counter |
| 5×1080p Lottie ≥ 60 FPS | perf |
| 销毁不泄漏 path / atlas slot | ASAN |
| Lottie/Rive 文件解析失败时优雅降级（显示 fallback rect + console.error） | 必须 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 100 icon Lottie ≥ 60 FPS | 期望 |
| Rive state machine 切换延迟 | < 16 ms |
| 单 Lottie 加载时间 | < 50 ms |
| Atlas usage（5 大 Lottie） | < 32 MB |

### 5.3 必须新增的测试

| 文件 | 验证 |
|---|---|
| `examples/data/lottie/loading.json` | 经典 loading 动画 |
| `examples/data/lottie/check_animation.json` | 复杂多 layer |
| `examples/data/rive/character.riv` | state machine 案例 |
| `test_lottie_basic.html` | 基础 Lottie 渲染 |
| `test_lottie_5x.html` | 5 个并发 |
| `test_rive_state_machine.html` | state machine 触发 |
| `test_lottie_with_css_transform.html` | dong CSS 协同 |
| `tests/vector/test_path_subdivide.cpp` | cubic → quadratic 数学单元 |
| `dong/scripts/test_lottie_baseline.py` | 与 lottie-web 像素对比 |

### 5.4 验证命令

```bash
# 像素对比 lottie-web (Playwright)
python dong/scripts/test_lottie_baseline.py --lottie-files data/lottie/*.json

# Perf
DONG_BENCH_AUTOSTOP=1 dong_app.exe --html data/tests/test_lottie_5x.html
python dong/tmp/trace_summary.py <trace>.json --top 20 | grep -i vector
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| rlottie 不支持高级 expression / 部分 effect | 文档明示 v1 子集；与 rlottie 上游同步特性矩阵 |
| rive-cpp 体积较大（~500 KB） | 作为可选构建（`-Drive=enabled`），默认禁用；业务侧按需打开 |
| 渲染像素与 web 端有差（亚像素 / 圆角策略不同）| 接受 < 5% diff；文档说明 |
| Slug atlas 容量不够（vector path 多）| 增 atlas 多页；超出 console.warn + fallback |
| 不同字号 Lottie 共享 atlas 时碎片化 | atlas 内按 path size bin 分桶 |
| State machine 输入名拼错没报错 | C ABI 返回错误码 + console.warn |

---

## 7. 不在本方案范围

- ❌ Lottie 完整 expression / 文字图层（v1 仅 path + paint）
- ❌ Lottie 内嵌 image (image asset 走 dong 自己 atlas)
- ❌ Rive 完整 mesh deformation（runtime 已支持，dong v1 仅 path）
- ❌ SVG 渲染（与 vector animation 同管线，但独立任务；Phase 3+ 评估）
- ❌ 自研 Lottie / Rive 编辑器
- ❌ 转换工具（Lottie ↔ Rive）

---

## 8. 完成后更新

- [ ] `doc/specific/html_css_dom_草案.md` § 2.7 / § 2.5 表格新增 `<dong-lottie>` `<dong-rive>` 行
- [ ] `doc/重要特性.md` 新增 § "Lottie / Rive 矢量动画"
- [ ] `dong/include/dong_vector.h` 文档化
- [ ] `doc/integration_ue.md` / `integration_unity.md` 加 "Lottie / Rive" 集成段
- [ ] `doc/perf_budget.md` 新增 vector animation 场景
