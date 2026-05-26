# Dong Profiler 使用指南

## 概述

Dong 内置了一个多线程性能分析器，输出 **Chrome Trace JSON** 格式，可在 `chrome://tracing` 或 [Perfetto UI](https://ui.perfetto.dev/) 中可视化。

---

## 1. 基本流程

### 1.1 启用 Profiler 并采集数据

在运行可执行文件时添加 `--profile <trace.json>` 参数：

```bash
# 交互式 demo
3d_screen_script.exe --profile trace_demo.json

# dong_app 指定 HTML
dong_app.exe --html data/video/video_play_test.html --profile trace_video.json

# html_render_test 批量渲染
html_render_test.exe examples/data/tests/flexbox_basic.html out.bmp 800 600 --frames 60 --profile trace_flex.json
```

当程序正常退出（或达到 `DONG_BENCH_*` 自动停止条件）时，profiler 自动把事件 dump 到指定文件。

### 1.2 可视化

1. 打开 Chrome/Edge，访问 `chrome://tracing`
2. 点击 **Load** 按钮，选择生成的 `.json` 文件
3. 使用 WASD / 鼠标滚轮 平移/缩放时间轴

或使用 Perfetto UI：
1. 访问 https://ui.perfetto.dev/
2. 点击 **Open trace file** 导入 JSON

---

## 2. 代码打点 API

### 2.1 C API

```c
// 初始化（可选，首次调用会自动初始化）
void dong_profiler_init(void);

// 开始/结束事件
void dong_profiler_begin(const char* name, const char* category);
void dong_profiler_end(void);

// 帧边界
void dong_profiler_frame_begin(void);
void dong_profiler_frame_end(void);

// 瞬时事件（无持续时间）
void dong_profiler_instant(const char* name, const char* category);

// 导出到文件
int dong_profiler_dump(const char* filepath);

// 运行时开关
void dong_profiler_set_enabled(int enabled);
```

### 2.2 C++ RAII 宏（推荐）

```cpp
#include "core/profiler.h"

void foo() {
    DONG_PROFILE_FUNCTION();  // 自动以函数名打点

    {
        DONG_PROFILE_SCOPE("SubTask");  // 自定义名称
        // ...
    }

    {
        DONG_PROFILE_SCOPE_CAT("GPU::submit", "gpu");  // 带分类
        // ...
    }
}
```

### 2.3 已有打点位置（参考）

引擎内部已在关键路径埋点，分类/名称示例：

| 分类       | 典型事件名                        | 说明                     |
|------------|-----------------------------------|--------------------------|
| `frame`    | `Frame`, `View::update`, `Engine::tick` | 帧循环主干               |
| `style`    | `Style::compute`                  | CSS 样式计算             |
| `layout`   | `Layout::calculate`               | Yoga 布局                |
| `render`   | `Painter::buildDisplayList`, `GPUCompiler::compile` | 显示列表构建与编译 |
| `gpu`      | `GPU::frame`, `GPU::prepareResources`, `SDL_SubmitGPUCommandBuffer*` | GPU 提交/同步 |
| `video`    | `Video::readFrame`, `Video::seek` | 视频解码                 |
| `script`   | `Script::processTasks`            | QuickJS 任务             |

---

## 3. 热点分析方法

### 3.1 在 chrome://tracing 中

1. **按帧分析**：找到 `Frame` 条带，展开查看其内部耗时分布。
2. **排序**：点击 **m** 键打开统计面板，可按 self-time / total-time 排序。
3. **关注长尾**：放大时间轴，观察单次耗时异常的事件（spike）。

### 3.2 使用 `trace_sum.py` 脚本

位于 `scripts/tools/trace_sum.py`，可快速汇总事件耗时：

```bash
python scripts/tools/trace_sum.py trace_demo.json --top 20
```

输出示例：

```
file: trace_demo.json
events: 12345

name                                             count       sum_ms     max_ms
----------------------------------------------------------------------------------
Frame                                              300      4500.000     18.000
View::update                                       300      3800.000     15.000
Layout::calculate                                  300      1200.000      8.000
SDL_WaitAndAcquireGPUSwapchainTexture              300       800.000     12.000
...
```

**关键指标**：
- **sum_ms**：总耗时（定位"累计开销大"的函数）
- **max_ms**：单次最大耗时（定位"偶发卡顿/长尾"）
- **count**：调用次数

### 3.3 典型热点与优化方向

| 热点事件                                   | 可能原因                                        | 优化方向                                   |
|--------------------------------------------|------------------------------------------------|-------------------------------------------|
| `SDL_WaitAndAcquireGPUSwapchainTexture`    | 等待 GPU 完成上一帧                             | 尝试 `DONG_GPU_SWAPCHAIN_NOWAIT=1`        |
| `Layout::calculate`                        | DOM 结构复杂或频繁 dirty                        | 减少 DOM 节点 / 避免不必要的样式修改       |
| `Painter::buildDisplayList`                | 绘制命令过多                                    | 简化 UI / 启用 layer-cache                |
| `Video::readFrame`                         | 视频解码慢                                      | 降低分辨率 / 使用硬解                      |
| `SDL_SubmitGPUCommandBufferAndAcquireFence`| 离屏读回等待 GPU                                | 减少读回次数 / 异步化                      |

---

## 4. 自动化性能采样 (Auto Profiler Loop)

脚本位于 `scripts/tools/auto_profile_loop.py`，用于**批量跑不同配置、自动采集 trace、汇总对比**。

### 4.1 基本用法

```bash
cd dong

# 单次采样（默认 target=3d_screen_script, build-dir=d:/mix/.../build-cmake）
python scripts/tools/auto_profile_loop.py

# 指定参数
python scripts/tools/auto_profile_loop.py \
    --build-dir d:/mix/agents/game/indr/dong/build-cmake \
    --target 3d_screen_script \
    --warmup-ms 2000 \
    --run-ms 5000 \
    --out-dir d:/mix/agents/game/indr/dong/tmp/traces
```

### 4.2 关键参数

| 参数                  | 默认值              | 说明                                                    |
|-----------------------|---------------------|--------------------------------------------------------|
| `--build-dir`         | `d:/.../build-cmake`| CMake 构建目录                                          |
| `--config`            | `Release`           | CMake 构建配置                                          |
| `--target`            | `3d_screen_script`  | 要构建并运行的目标                                       |
| `--warmup-ms`         | `2000`              | 预热时间（ms），预热期间的事件会被清空                    |
| `--run-ms`            | `5000`              | 采样时间（ms），预热后开始计时                            |
| `--iters`             | `1`                 | 每个配置跑几次                                           |
| `--out-dir`           | `tmp/traces`        | trace / log / 汇总 JSON 输出目录                         |
| `--sweep`             | `false`             | 自动对比 `nowait=0/1` 等组合                             |
| `--nowait`            | `false`             | 设置 `DONG_GPU_SWAPCHAIN_NOWAIT=1`                      |
| `--layer-cache`       | `false`             | 设置 `DONG_LAYER_CACHE=1`                               |
| `--no-split-cmd-buf`  | `false`             | 设置 `DONG_GPU_SPLIT_CMD_BUF=0`                         |
| `--no-build`          | `false`             | 跳过构建步骤                                             |

### 4.3 输出文件

运行完成后，`--out-dir` 下会生成：

```
traces/
├── 3d_screen_script_20260119_143000_pm-default_fif-default_nw-0_i0.json   # trace
├── 3d_screen_script_20260119_143000_pm-default_fif-default_nw-0_i0.log    # 运行日志
├── results_3d_screen_script_20260119_143000.json                          # 汇总
```

**汇总 JSON 结构**（`results_*.json`）：

```json
{
  "stamp": "20260119_143000",
  "target": "3d_screen_script",
  "args": { ... },
  "results": [
    {
      "cfg": { "present_mode": null, "frames_in_flight": null, "nowait": false },
      "status": "ok",
      "fps_est": 60.2,
      "wait_acquire": { "self_us": 123456, "cnt": 301, "avg_us": 410.15, "max_us": 12000 },
      "submit": { ... },
      "frame": { ... },
      "trace": ".../*.json"
    },
    ...
  ],
  "best": { ... }
}
```

### 4.4 工作原理

1. **构建**：调用 `cmake --build` 编译目标。
2. **运行**：启动 `<target>.exe --profile <trace.json>`，并设置环境变量：
   - `DONG_BENCH_AUTOSTOP=1`：到达采样时间后自动退出
   - `DONG_BENCH_WARMUP_MS`：预热时长
   - `DONG_BENCH_RUN_MS`：采样时长
3. **解析**：读取生成的 trace JSON，计算关键事件（`Frame`, `SDL_*Acquire*`, `SDL_SubmitGPU*`）的统计指标。
4. **汇总**：选出 `wait_acquire.self_us` 最小 / `fps_est` 最高的配置作为 `best`。

### 4.5 对比 `--sweep`

```bash
python scripts/tools/auto_profile_loop.py --sweep --iters 2
```

会自动跑两组配置（`nowait=0` 与 `nowait=1`），每组跑 2 次，最后在终端输出类似：

```
=== DONE ===
wrote: tmp/traces/results_3d_screen_script_20260119_143000.json
BEST cfg={'nowait': True, ...} fps~120.5 wait_self_us=5000 trace=...
```

---

## 5. 环境变量参考

| 变量名                                | 说明                                             |
|---------------------------------------|--------------------------------------------------|
| `DONG_BENCH_AUTOSTOP`                 | `1`=启用自动停止（传 `--profile` 时默认启用）     |
| `DONG_BENCH_WARMUP_MS`                | 预热毫秒数（默认 2000）                           |
| `DONG_BENCH_RUN_MS`                   | 采样毫秒数（默认 5000）                           |
| `DONG_GPU_SWAPCHAIN_NOWAIT`           | `1`=不阻塞等待 swapchain                          |
| `DONG_GPU_PRESENT_MODE`               | `mailbox` / `vsync` / `immediate`                |
| `DONG_GPU_FRAMES_IN_FLIGHT`           | 帧缓冲数（1~3）                                   |
| `DONG_LAYER_CACHE`                    | `1`=启用隔离层缓存                                |
| `DONG_GPU_SPLIT_CMD_BUF`              | `0`=禁用 command buffer 拆分                      |

---

## 6. 常见问题

### Q: trace 文件为空或很小？
- 确保程序正常退出（或达到 `DONG_BENCH_RUN_MS`）。
- 检查 `DONG_PROFILER_ENABLED` 编译宏是否为 `1`（默认开启）。

### Q: 如何只采集特定阶段？
- 调用 `dong_profiler_clear()` 清空已有事件后再开始关心的阶段。
- 使用 `dong_profiler_set_enabled(0/1)` 动态开关。

### Q: 如何添加自定义打点？
- 在代码中使用 `DONG_PROFILE_SCOPE("MyName")` 或 `DONG_PROFILE_SCOPE_CAT("MyName", "mycategory")`。
- 重新编译后即可在 trace 中看到。
