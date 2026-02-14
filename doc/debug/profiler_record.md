# 性能优化记录

## 2024-02 3D Screens Simple 优化

### 问题
23屏幕3D场景帧率仅 ~35 FPS，预期 2000+ FPS。

### 根因
1. **每帧无条件渲染** - hover/focus屏幕即使内容无变化也每帧执行完整渲染流水线
2. **日志输出开销** - `DONG_LOG_INFO` 每帧输出大量日志，`printf` 是严重瓶颈

### 优化方案

#### 1. Dirty 标志检查
```c
// scene3d.c - hover屏幕只在dirty时更新
if (scr && scr->dirty) {  // 之前: if (scr)
    update_screen_texture(...);
    scr->dirty = 0;
}
```

输入事件已设置 dirty（mouse move/click/wheel/key），无需改动。

#### 2. Overlay 延迟渲染
```c
// overlay.c - 首帧后只在dirty时渲染
if (overlay->initial_render_done && !overlay->dirty) return;
// ... render ...
overlay->initial_render_done = 1;
overlay->dirty = 0;
```

`dong_overlay_eval_script()` 执行时设置 `dirty=1`。

#### 3. 日志级别降级
每帧执行的日志从 `DONG_LOG_INFO` 改为 `DONG_LOG_DEBUG`：
- `[tick] DisplayList/GPU commands` (engine_view.cpp)
- `[GPU Execute] DrawText` (sdl_gpu_driver_execute.cpp)
- `[DisplayListBuilder] addGlyphRun` (display_list.hpp)
- `[GPUCompiler] compile` (gpu_ir.hpp)

### 结果
| 指标 | 优化前 | 优化后 |
|------|--------|--------|
| FPS | ~35 | ~2000 |
| 提升 | - | **57x** |

### 经验
1. **日志是隐形杀手** - 生产代码避免每帧 INFO 日志
2. **Dirty 模式** - 静态内容用标志位跳过渲染
3. **测试时设置 `DONG_LOG_LEVEL=WARN`** - 排除日志干扰
