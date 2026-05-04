## 性能优化方法论总结：无人值守 Profile Loop 工作流

> 目标：把“猜原因/手工跑/一次性截图”升级成可重复、可对比、可回归的闭环：
> **自动 build → 自动运行（预热不计入）→ 固定时长采样 → 自动退出落盘 trace → 自动解析关键指标 → 输出最优配置/热点 → 再迭代改代码并复跑验证**。

---

## 1. 核心原则

- **可重复**：每次采样的窗口一致（固定 warmup + run），减少“第一次抖动/缓存冷启动”对结论的污染。
- **可对比**：每次跑完都会落盘一份结构化结果（`results_*.json`），可以和上一轮做差（关注关键指标趋势）。
- **可回归**：脚本输出的 trace/日志文件命名稳定，方便在 CI 或本地批量复跑。
- **先度量后优化**：先从 trace 里确认“大头到底在哪里”，再改代码；避免凭感觉优化。

---

## 2. 工具与产物

### 2.1 无人值守主脚本：`dong/scripts/tools/auto_profile_loop.py`

这个脚本负责把“跑一次 profile”变成“可循环的实验”。它会：

- **（可选）自动构建**：`cmake --build <build-dir> --config <config> --target <target>`
- **自动运行并采样**：通过环境变量驱动 demo 在稳态窗口内自动退出、并输出 Chrome trace JSON
  - `DONG_BENCH_AUTOSTOP=1`
  - `DONG_BENCH_WARMUP_MS=<warmup-ms>`（预热期，不计入采样）
  - `DONG_BENCH_RUN_MS=<run-ms>`（稳态采样期）
- **自动解析指标**（脚本内部自带解析器，不依赖外部工具）：
  - 估算 FPS（通过 `Frame` scope 的计数 / 采样秒数）
  - 统计关键 scope：
    - `SDL_WaitAndAcquireGPUSwapchainTexture`（阻塞获取 swapchain）
    - `SDL_AcquireGPUSwapchainTexture`（NOWAIT 获取 swapchain）
    - `SDL_SubmitGPUCommandBuffer`
    - `offscreen_rebuild`
- **落盘结构化结果**：输出 `results_<target>_<stamp>.json`，包含每轮跑的指标与自动挑选的 best。

### 2.2 辅助热点查看脚本：`tmp/trace_summary.py`

用于把某个 trace 的 BE scope 做 “Top N 自耗时” 汇总，辅助你从结果里快速锁定下一步改哪。

---

## 3. 标准闭环：一次完整迭代怎么跑

下面是推荐的“固定动作序列”。要点是：每次迭代都产出可对比的结果文件。

### 3.1 第 0 步：选定目标与采样窗口

- **target**：默认是 `3d_screen_script`（对应 `3d_screen_script.exe`）。
- **warmup**：建议 1–3 秒（让资源/缓存/管线创建稳定下来）。
- **run**：建议 3–10 秒（足够长，统计更稳定）。

### 3.2 第 1 步：跑无人值守 profile（推荐先 sweep）

`sweep` 用于快速比较“阻塞 acquire vs NOWAIT acquire”这种对 FPS/等待最敏感的开关：

```bash
python d:/mix/agents/game/indr/dong/dong/scripts/tools/auto_profile_loop.py \
  --sweep --iters 1 \
  --warmup-ms 2000 --run-ms 5000 \
  --out-dir d:/mix/agents/game/indr/dong/tmp/traces
```

你会得到：

- 多份 trace：`<target>_<stamp>_pm-..._fif-..._nw-..._i0.json`
- 对应日志：`..._i0.log`
- 一份汇总：`results_<target>_<stamp>.json`
- 控制台最后会打印 `BEST ...`（脚本按“更少 wait-acquire 自耗时 + 更高 fps”挑最优）。

> 提示：只想快迭代时加 `--no-build`，避免每次都编译。

### 3.3 第 2 步：从 best trace 里读“真正的大头”

有两条路：

- **快速路（脚本已给关键指标）**：直接看 `results_*.json` 里 `wait_acquire / submit / offscreen_rebuild / fps_est` 的趋势。
- **深挖路（Top N 热点）**：对 best 的 trace 再跑一次 `trace_summary.py`：

```bash
python d:/mix/agents/game/indr/dong/tmp/trace_summary.py \
  d:/mix/agents/game/indr/dong/tmp/traces/<best_trace>.json --top 30
```

输出会告诉你：哪些 scope 的 **self time（自耗时）** 最多。

### 3.4 第 3 步：提出假设并做最小改动

规则：一次只改一类原因，改动要“能解释指标变化”。典型例子：

- `SDL_WaitAndAcquire...` 很大：这是**被 swapchain 阻塞**（常见于 vsync / frame pacing / queue 满）。
  - 对应策略：提供 NOWAIT 路径、允许跳帧/轻 sleep，或调整 present mode / frames-in-flight。
- `SDL_SubmitGPUCommandBuffer` 很大：CPU 侧提交/录制/同步开销偏高。
  - 对应策略：减少每帧重复 submit（例如：把静态资源上传从“每帧”挪到“一次”）。
- `offscreen_rebuild` 很大：离屏命令/资源重建频繁。
  - 对应策略：引入缓存、减少 dirty 触发、拆分更新粒度。

### 3.5 第 4 步：复跑同样窗口并做对比

复跑同样参数（同 warmup/run），拿新的 `results_*.json` 与上一轮对比：

- **看趋势**：`wait_acquire.self_us`、`submit.self_us`、`fps_est`。
- **看稳定性**：同一配置多跑几次（`--iters 3`）避免偶然值。

---

## 4. 关键指标怎么判读（经验速记）

- **`wait_acquire`（阻塞/非阻塞获取 swapchain）**
  - 大：大概率不是“CPU 算不过来”，而是**帧被显示/队列/同步边界卡住**。
  - 小但 FPS 仍低：说明瓶颈可能转移到了 draw/submit/资源准备。

- **`submit`（`SDL_SubmitGPUCommandBuffer`）**
  - 大：通常意味着每帧 work 太多、或重复做了不该每帧做的工作（创建/上传/状态切换/过多 pass）。

- **`offscreen_rebuild`**
  - 大：通常意味着“离屏路径一直在重建命令/资源”，而不是只在 dirty 时做增量更新。

- **`fps_est`**
  - 这是脚本基于 `Frame` scope 的计数估算的“稳态 FPS”。对比不同配置/改动时很好用。

---

## 5. 无人值守跑批的工程化细节（避免卡死/污染数据）

- **非 0 退出码**：只要 trace 文件成功写出，就继续跑下一轮；不要因为进程异常退出而中断实验。
- **超时保护**：脚本使用 
  - `timeout_s = warmup + run + timeout-grace` 
  防止 demo 卡死导致 loop 停住。
- **固定输入条件**：采样期间尽量减少随机输入/窗口焦点变化等外因。
- **输出组织**：统一放在一个 `out-dir`，保持时间戳命名；便于做历史对比与回归。

---

## 6. 推荐的日常用法（最常用的 3 条命令）

1) **快速比较阻塞 vs NOWAIT（推荐入口）**：

```bash
python d:/mix/agents/game/indr/dong/dong/scripts/tools/auto_profile_loop.py --no-build \
  --sweep --iters 1 --warmup-ms 2000 --run-ms 5000 \
  --out-dir d:/mix/agents/game/indr/dong/tmp/traces
```

2) **固定配置多跑几次看稳定性**（例如只测 NOWAIT）：

```bash
python d:/mix/agents/game/indr/dong/dong/scripts/tools/auto_profile_loop.py --no-build \
  --nowait --iters 3 --warmup-ms 2000 --run-ms 5000 \
  --out-dir d:/mix/agents/game/indr/dong/tmp/traces
```

3) **对 best trace 做 Top N 热点汇总**：

```bash
python d:/mix/agents/game/indr/dong/tmp/trace_summary.py d:/mix/agents/game/indr/dong/tmp/traces/<best_trace>.json --top 30
```

---

## 7. 这套工作流的“完成标志”

- 你能在无人介入的情况下连续跑多轮：每轮都有 trace + `results_*.json`。
- 你能从结果里稳定回答：
  - “当前瓶颈是 swapchain 等待 / submit / rebuild / 还是别的？”
  - “某个改动让关键指标改善了多少？是否稳定？”
- 你能把一次优化固化成可复跑的实验（参数、脚本、输出目录规范），后续任何回归都能被迅速捕获。
