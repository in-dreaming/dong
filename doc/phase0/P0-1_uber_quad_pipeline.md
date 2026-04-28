# P0-1 — Uber Quad Pipeline 落地

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 0 P0-1
> 设计原案：[`doc/arch/render_pipeline.md`](../arch/render_pipeline.md)
> 性能门槛：[`doc/perf_budget.md`](../perf_budget.md) § 3.1
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

把当前"每类图元一个 pipeline + 每条命令 1 draw call"的设计，重构为：

- **rect / rounded-rect / shadow / gradient** 合并到单一 `uber_quad_pipeline`，shader 内按 material type 分派。
- **同 batch 内合并 instances**，一条 GPU draw call 画 N 个 quad。
- **保留 text / image / video YUV / isolated layer** 的独立路径（短期不动）。

最终效果：典型 UI 场景 GPU draw call 减少 ~50%，达到 [`perf_budget.md`](../perf_budget.md) S1 的 Soft 目标（`game_ui2` ≤ 30 draw, `test_select_keyboard` ≤ 50 draw）。

---

## 2. 现状摘要

| 项 | 当前 |
|---|---|
| pipeline 数 | rect / round_rect / shadow / image / text / gradient / video YUV ≈ 7 个 |
| `game_ui2` draw call | 68（22 rect + 18 round + 28 text） |
| `test_select_keyboard` draw call | 91 |
| GPUCommand 字段 | `Rect / Color / radius / stroke_width / blur / image_* / layer_* / text_*`（fat struct）|
| 排序 / 批次 | `sort_key` 已计算，但执行时仍按原顺序、不合批 |

代码索引：
- 命令定义：`dong/src/render/gpu_ir.hpp`
- 编译器：`dong/src/render/gpu_compiler.cpp`（DisplayList → GPUCommandList）
- 后端 init / pipeline：`dong/backends/sdl/sdl_gpu_driver_init.cpp`
- 后端 execute：`dong/backends/sdl/sdl_gpu_driver_execute.cpp`
- 旧 shader：`dong/backends/sdl/shaders/{rect,round_rect,shadow,gradient}_*.hlsl`

---

## 3. 设计

### 3.1 Uber Quad Instance 数据布局

```hlsl
struct UberQuadInstance {
    float4 rect;            // x, y, w, h（像素坐标，pre-transform）
    float4 color;           // RGBA（pre-multiplied 见 § 3.5）
    float4 radii;           // 4 角圆角：tl, tr, br, bl（不等圆角支持）
    float4 params;          // [0]=material_type, [1]=stroke_width, [2]=blur, [3]=reserved
    float4 grad0;           // gradient 起点 (xy) + stop0 颜色 packed (zw)
    float4 grad1;           // gradient 终点 (xy) + stop1 颜色 packed (zw)
    // 不放 transform：transform 在 batch 边界 push/pop，不混入 instance
};  // 总计：6 × float4 = 96 bytes / instance
```

**Material Types**（`params.x`）：

| 值 | 类型 | 使用字段 |
|---|---|---|
| 0 | solid rect | `color` |
| 1 | rounded rect (fill) | `color` `radii` |
| 2 | rounded rect (stroke) | `color` `radii` `params.y` (stroke_width) |
| 3 | box-shadow | `color` `radii` `params.z` (blur) |
| 4 | linear gradient | `radii` `grad0` `grad1`（双 stop，扩展见 § 7） |

> 暂不收 conic-gradient（在 P0-3 单独 shader 处理）。

### 3.2 Pipeline 与 Shader

新增：

- `dong/backends/sdl/shaders/uber_quad_vs.hlsl`
- `dong/backends/sdl/shaders/uber_quad_fs.hlsl`

VS：从 `instance_buffer` 取 `UberQuadInstance`，输出 `pos / local_uv / params / radii / color / grad0 / grad1`。

FS：

```hlsl
float4 uber_fs(VS_Out i) : SV_Target {
    int material = (int)i.params.x;
    [branch] switch (material) {
        case 0: return solidRect(i);
        case 1: return roundedRectFill(i);
        case 2: return roundedRectStroke(i);
        case 3: return boxShadow(i);
        case 4: return linearGradient(i);
        default: return float4(1, 0, 1, 1); // debug: magenta
    }
}
```

> material 通常在 batch 内连续相同 → branch 预测友好，不会显著 GPU 占用上升。

### 3.3 Batch 切分规则

遍历 `GPUCommandList` 时构造 `UberQuadBatch`：

```
flush 当前 batch 并切换路径，触发条件（任一）：
  - 遇到 text / image / video / isolated layer / clip push/pop 命令
  - 遇到 transform push/pop（layer_transform 变化）
  - instance 数达到 batch 上限 (4096，避免 buffer 越界)
  - 遇到不同 isolation layer / opacity 边界
```

规则保证 CSS 绘制顺序（背景→边框→内容→前景）不被破坏。

### 3.4 Instance Buffer 管理

- 每帧 reset 的 `ring buffer`（容量初始 256 KB ≈ 2700 instances）。
- 不够则 grow ×2，记录 high watermark；下一帧分配该 size。
- 用 SDL_GPU 的 `SDL_UploadToGPUBuffer`，per-frame 一次上传或多次小段上传（按 batch flush 决定）。
- buffer 不重复释放，View 销毁时统一 release。

### 3.5 颜色与混合

- Instance buffer 里的 `color` 与既有 `Color` 一致（**未预乘**）。
- shader 末端 premultiply：`return float4(rgb * a, a);`
- pipeline blend state：`SrcAlpha = ONE`（因为已 premultiplied），`DstAlpha = ONE_MINUS_SRC_ALPHA`。
- 与现有 image/text 路径一致，便于跨路径 blend 顺序正确。

### 3.6 GPUCompiler 改动

```cpp
// 新增：UberQuadBatch 累积器
struct UberQuadBatch {
    std::vector<UberQuadInstance> instances;
    bool empty() const { return instances.empty(); }
    void clear() { instances.clear(); }
};

// GPUCompiler 维护 current_uber_batch
// 遇到可合批命令 → push 到 instances
// 遇到非可合批命令 → emit Flush + 该命令
```

`GPUCommandList` 新增命令类型：

```cpp
enum class GPUCommandType : uint8_t {
    // ... 既有 ...
    UberQuadBatch,   // 一组 instance，由 driver 一次 draw
};
```

`GPUCommand` 字段扩展（不破坏既有）：

```cpp
struct GPUCommand {
    // ... 既有 ...
    uint32_t uber_batch_offset = 0;  // 在 instance_buffer 中的起始 instance 下标
    uint32_t uber_batch_count = 0;   // instance 数量
};
```

### 3.7 SDLGPUDriver 改动

`execute()`：

```cpp
for (const auto& cmd : commands) {
    switch (cmd.type) {
        case GPUCommandType::UberQuadBatch:
            // 1. SDL_BindGPUVertexBuffers(uber_vs_quad)        ← 静态 unit quad
            // 2. SDL_BindGPUVertexBuffers(uber_instance_buffer + offset)
            // 3. SDL_BindGPUGraphicsPipeline(uber_pipeline)
            // 4. SDL_DrawGPUPrimitives(6 verts, cmd.uber_batch_count instances)
            break;
        case GPUCommandType::DrawText:
            executeDrawText(cmd); break;
        // ... 其他保留 ...
    }
}
```

`prepare_resources()` 阶段：

```cpp
// 把本帧所有 UberQuadBatch 命令的 instances 串成一段 buffer 一次上传
// 减少 SDL_UploadToGPUBuffer 调用次数
```

---

## 4. 实施步骤（建议拆 PR）

| Step | PR 范围 | 说明 |
|---|---|---|
| **S1 — Pipeline scaffold** | shader + pipeline 创建 + 单 instance solid rect demo | 不接入 GPUCompiler；env var 控制开关；验证能画出红色矩形 |
| **S2 — solid + rounded fill 接入** | GPUCompiler 输出 UberQuadBatch（仅 material 0、1）；旧路径并存 | 跑 `game_ui2`，draw call 减少；像素与 baseline 一致 |
| **S3 — stroke + shadow + gradient** | 加入 material 2、3、4 | 跑全部 baseline test，像素无回归 |
| **S4 — Batch 边界与 clip / layer** | 完整 flush 规则 | 跑 `transform_test` `text_shadow_test` 等含 clip/layer 用例 |
| **S5 — 切默认路径并清理** | 把 env var 默认改为 uber；移除旧 pipeline 与旧 shader | 一个 PR 内完成切换 + 删除 |

每步 PR 必须带 § 5 中至少一条验证规则的实测对比。

---

## 5. 通过验证规则

```yaml
verify:
  hard:
    - id: p0_1_status_command_exists
      cmd: "python -c \"print('P0-1 verify hard check')\""
      cwd: "."
      timeout_sec: 60
    - id: p0_1_orch_syntax
      cmd: "python -m py_compile dong/scripts/orch.py"
      cwd: "."
      timeout_sec: 120
  soft:
    - id: p0_1_notes_marker
      cmd: "python -c \"from pathlib import Path; p=Path('dong/.worktrees/P0-1/.task/notes.md'); print('ok' if p.exists() else 'missing')\""
      cwd: "."
      timeout_sec: 60
```

### 5.1 Hard（必须满足，不达标 PR 不准合）

| 项 | 阈值 |
|---|---|
| 像素回归 | `scripts/tools/run_baseline_compare.py` 在 `examples/data/tests/` 全集 ≥ 99% 通过；diff 像素 < 256 的用例不计入失败 |
| GPU validation | Vulkan / D3D12 validation layer 0 error / 0 warning |
| `game_ui2` draw call | ≤ 50（Hard） |
| `test_select_keyboard` draw call | ≤ 80（Hard） |
| FPS 不回归 | `game_ui2` 在 Reference 机型上不低于优化前 95% |

### 5.2 Soft（期望，未达需立 issue 排期）

| 项 | 阈值 |
|---|---|
| `game_ui2` draw call | ≤ 30 |
| `test_select_keyboard` draw call | ≤ 50 |
| `transform_test` draw call | ≤ 12（当前 22） |
| 单条 uber batch 平均 instance 数 | ≥ 4 |
| GPU 时间 / 帧（Reference, present-only off）| ≤ 4 ms（当前 6.5 ms） |

### 5.3 验证脚本（必须新增 / 复用）

```bash
# 像素回归
python dong/scripts/tools/run_baseline_compare.py --suite examples/data/tests

# Draw call 统计（基于 trace）
python dong/scripts/tools/auto_profile_loop.py --no-build \
  --target dong_app -- --html data/gamelikeui/game_ui2.html \
  --warmup-ms 1000 --run-ms 3000

python dong/tmp/trace_summary.py <trace>.json --top 20 \
  | grep -E "DrawInstancedQuads|UberQuad|DrawRoundedRect|DrawShadow"

# 期望输出：UberQuadBatch ≤ 30 行，旧 *_pipeline 出现 0 次（S5 之后）
```

S5 之后，`perf_baseline.py`（P0-7）应能稳定回归本指标。

---

## 6. 风险与回退

| 风险 | 缓解 / 回退 |
|---|---|
| Shader 内 switch 在某些 GPU 上分支预测不好，反而变慢 | 留 env var `DONG_RENDER_PATH=legacy` 紧急回退；S5 之前不删旧路径 |
| Instance buffer per-frame 上传开销 > 节省的 draw call | 改为 ring buffer + persistent mapping；用 `auto_profile_loop` 的 `submit.self_us` 度量 |
| Material 5+ 扩展时打破现有 layout | 保留 `params.w` 为 reserved；新增 material 时在文档中追加 |
| Mobile GPU 不支持 6 × float4 instance attribute | iOS / Android 验证清单加入此项；不通过则拆为多个 attribute slot |
| Slug text / MSDF text 与 uber batch 交错时 z-order 错乱 | 严格按命令顺序 flush；`test_text_shadow_test` 是回归看门狗 |

---

## 7. 不在本方案范围

- ❌ Conic gradient（走 P0-3 单独 shader）
- ❌ Text 合批（已有 GlyphRun 合并，不再扩）
- ❌ Image 合并到 uber（需要 atlas 化，留 Phase 1+）
- ❌ 跨 view batch（GlobalShared 内的多 view 合批，留 Phase 1+）
- ❌ Compute shader / mesh shader 路径（保留传统 vs+fs）

---

## 8. 完成后更新清单

PR 合入 S5 后，必须更新：

- [ ] `doc/perf_budget.md` § 3.1 当前实测列
- [ ] `doc/重要特性.md` § GPU 性能优化
- [ ] `doc/arch/render_pipeline.md` 末尾追加"已实施 / 实测数据"
- [ ] `doc/positioning.md` § 8 决策记录追加"Uber Quad 已默认开启"
