# 渲染管线优化：Uber Quad Pipeline

## 1. 动机

当前渲染管线采用"每类图元一个 pipeline"的设计：

```
DisplayList
  │
  ▼
GPUCompiler  ──▶  GPUCommandList (逐命令线性序列)
                       │
                       ▼
              SDLGPUDriver::execute()
              ┌──────────────────────────┐
              │ for cmd in commands:     │
              │   bindPipeline(cmd.type) │  ← 每条命令切换 pipeline
              │   pushUniforms(cmd)      │
              │   drawPrimitives(1)      │  ← 每条命令 1 次 draw call
              └──────────────────────────┘
```

**基准数据** (800×600, 2026-03-31):

| 页面 | 总 draws | rect | round | shadow | img | text | layers |
|------|----------|------|-------|--------|-----|------|--------|
| game_ui2 | 68 | 22 | 18 | 0 | 0 | 28 | 0 |
| test_select_keyboard | 91 | 34 | 16 | 0 | 0 | 41 | 1 |
| transform_test | 22 | 2 | 10 | 0 | 0 | 10 | 7 |
| text_shadow_test | 20 | 2 | 9 | 4 | 0 | 5 | 0 |

**问题**：
- 每帧 68–91 次 pipeline bind + draw call
- rect、round、shadow、gradient 本质上都是"一个四边形 + 不同参数"
- 当前排序/批次仅供 debug 日志使用，不影响实际渲染

## 2. 目标架构

### 2.1 核心思路

将 rect、round-rect、shadow、gradient 统一到一个 **uber quad pipeline**，通过 material type 字段在 shader 内分派。Text 和特例路径保持独立。

```
DisplayList
  │
  ▼
GPUCompiler  ──▶  GPUCommandList (分类)
                  ┌──────────────────────────────────────┐
                  │  UberQuadBatch[]     (rect/round/    │
                  │                       shadow/gradient)│
                  │  TextCommand[]       (Slug / MSDF)   │
                  │  ImageCommand[]      (texture quads)  │
                  │  SpecialCommand[]    (layer/clip/YUV) │
                  └──────────────────────────────────────┘
                       │
                       ▼
              SDLGPUDriver::execute()
              ┌──────────────────────────────────────────┐
              │ 1. bindPipeline(uber_quad)               │
              │    uploadInstances(uber_batch)            │
              │    drawInstanced(N)                       │  ← 1 draw call
              │                                          │
              │ 2. for text in text_commands:            │  ← 保持独立
              │      draw(text)                          │
              │                                          │
              │ 3. 特例命令穿插执行（按 z-order）          │
              │    (clip push/pop → flush uber batch)    │
              │    (layer begin/end → render pass break) │
              └──────────────────────────────────────────┘
```

### 2.2 Uber Quad Instance 数据布局

```hlsl
struct UberQuadInstance {
    float4 rect;            // x, y, w, h (像素坐标)
    float4 color;           // RGBA
    float4 params;          // [0]=material_type, [1]=radius, [2]=stroke_width, [3]=blur
    float4 extra;           // 渐变角度/stops 或预留
    float4x4 transform;    // 可选：仅 layer transform 场景需要
};
```

**Material Types** (params.x):
- `0` = solid rect (fill color)
- `1` = rounded rect (SDF with radius)
- `2` = shadow (SDF + blur)
- `3` = gradient (linear, angle + stops)

### 2.3 Shader 分派

```hlsl
float4 uber_quad_fs(VS_Output input) : SV_Target {
    int material = (int)input.params.x;

    if (material == 0) return solidRect(input);
    if (material == 1) return roundedRect(input);
    if (material == 2) return shadow(input);
    if (material == 3) return gradient(input);

    return float4(1, 0, 1, 1); // debug: magenta
}
```

### 2.4 CSS 绘制顺序保证

CSS 要求严格的绘制顺序（背景 → 边框 → 内容 → 前景）。批次合并**不能跨越**以下边界：

1. **Clip 变更**：`PushClipRect` / `PopClip` 必须 flush 当前 uber batch，设置 scissor
2. **Layer 边界**：`BeginIsolatedLayer` / `EndIsolatedLayer` 必须 flush 并切换 render target
3. **Text 命令**：text 与 uber quad 交替出现时，需要按原始顺序 flush

**批次分割策略**：

```
遍历 GPUCommandList:
  if (cmd is uber-quad-eligible):
      append to current_uber_batch
  else:
      flush(current_uber_batch)  // 提交已积累的 uber quads
      execute(cmd)               // 执行 text / clip / layer 等
```

这保证了 CSS 绘制顺序，同时最大化了连续 uber quad 的合批。

## 3. 不纳入 Uber Quad 的特例路径

### 3.1 Slug Text（默认文字方案）

- 独立的 vertex/index buffer（动态生成几何）
- 独特的 vertex format（5 × float4）
- 依赖 curve/band 纹理对
- 保持 `slug_text_pipeline_` 不变

### 3.2 MSDF Text

- instanced quads + atlas 纹理采样
- 保持 `text_pipeline_` 不变

### 3.3 Image Quad

- 需要绑定各自的纹理 + sampler
- 不同的 blend state（premultiplied alpha）
- 保持 `image_pipeline_` 不变
- **后续优化**：如果 atlas 化成功，可以合并到 uber

### 3.4 YUV Video

- 三平面纹理 + 特殊 shader
- 保持 `video_yuv_pipeline_` 不变

### 3.5 Isolated Layers

- 涉及 render target 切换 / offscreen rendering
- 不可能批次合并
- 保持现有逻辑不变

## 4. 实施路径

### Phase 1：Uber Quad Shader + Pipeline 创建

1. 新增 `shaders/uber_quad_vs.hlsl` + `uber_quad_fs.hlsl`
2. 在 `sdl_gpu_driver_init.cpp` 创建 `uber_quad_pipeline_`
3. Instance buffer 管理（ring buffer or per-frame upload）
4. 验证：能用 uber pipeline 绘制 solid rect

### Phase 2：GPUCompiler 产出 UberQuadBatch

1. 在 `GPUCommandList` 中新增 `UberQuadBatch` 结构
2. `GPUCompiler::compile()` 将 rect/round/shadow/gradient 编译为 `UberQuadInstance`
3. 遇到 text/clip/layer 时产出 flush 标记
4. 保持旧路径可通过环境变量切换

### Phase 3：Execute 端批次提交

1. `SDLGPUDriver::execute()` 识别 uber batch 命令
2. 上传 instance buffer → `SDL_DrawGPUPrimitivesIndirect` 或 instanced draw
3. flush 时提交已积累的 instances
4. 保留 text/clip/layer 的现有执行路径

### Phase 4：移除旧路径

1. 移除 `rect_pipeline_`、`round_rect_pipeline_`、`shadow_pipeline_`、`gradient_pipeline_`
2. 移除对应的旧 shader 文件
3. 清理 `GPUCommand` fat struct 中不再需要的字段
4. 更新基准数据，验证性能提升

## 5. 性能预期

以 `game_ui2` 为例：
- **现状**：68 draw calls (22 rect + 18 round + 0 shadow + 0 grad + 28 text)
- **Phase 3 后**：~3–5 uber batches (按 clip/text 边界分割) + 28 text draws ≈ **31–33 draw calls**
- **降幅**：~52–54%

以 `test_select_keyboard` 为例：
- **现状**：91 draw calls
- **Phase 3 后**：~5–8 uber batches + 41 text draws ≈ **46–49 draw calls**
- **降幅**：~46–49%

text draw calls 后续可通过 text batching 进一步优化，但不在本方案范围内。

## 6. 风险与约束

| 风险 | 缓解 |
|------|------|
| Uber shader 分支导致 GPU 占用上升 | material type 通常连续相同，分支预测友好 |
| Instance buffer 大小管理 | 动态 ring buffer，每帧 reset |
| 旧新路径并存的复杂度 | 环境变量切换，phase 4 彻底移除 |
| CSS 绘制顺序破坏 | flush 边界严格保证顺序 |

## 7. 验证标准

- [ ] 所有 `examples/data/tests/` 的渲染输出与 baseline BMP 像素一致
- [ ] `game_ui2` draw call 数 < 40
- [ ] `test_select_keyboard` draw call 数 < 55
- [ ] 无 GPU validation layer 错误
- [ ] Slug/MSDF text 渲染不受影响
