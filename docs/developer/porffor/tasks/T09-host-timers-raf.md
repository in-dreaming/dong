# T09 — `setInterval` 与 `requestAnimationFrame`

- **Area**: dong-host
- **性质**: 纯实现型 — 对标物：浏览器 `setInterval` / `requestAnimationFrame` 标准语义（时序细节以 QuickJS 版 `tickTimers` / `tickAnimationFrames` 现有行为为准）
- **优先级**: P1
- **依赖**: 无（沿用 `setTimeout` 的现有基建）；rAF 带参回调可选依赖 T15 方案 A
- **预估**: 1–2 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §9

## 背景

`setTimeout`（export 名回调 + C++ `PorfforHost::processTimers` 每帧 tick）已实现。本任务在同一基建上补 `setInterval` 与 `requestAnimationFrame`。评估结论：**高可行，纯 Dong 侧**。

## API

| import | 说明 |
|--------|------|
| `dong_set_interval(exportName, ms) -> timerId` | 重复触发；C++ 侧 timer 加 repeat 标志 |
| `dong_clear_interval(timerId)` / `dong_clear_timeout(timerId)` | 统一取消（如现状没有 clearTimeout 一并补上） |
| `dong_request_animation_frame(exportName) -> rafId` | 下一渲染帧回调一次，回调时参数（时间戳）写入全局槽或直接作为 export 参数传入 |
| `dong_cancel_animation_frame(rafId)` | |
| prelude 包装 | `setInterval(name, ms)`、`requestAnimationFrame(name)` 等 |

## 设计点

1. rAF 回调需要时间戳。**已确认现状不支持带参**（setup §3 事实 F2：`dong_porf_main_fn` 是 `int(*)(void)`）。两条路：
   - a) 依赖 T15 方案 A 的带参 export（干净，但有前置）；
   - b) 全局槽 import：回调前 host 写入时间戳，handler 内 `dong_raf_timestamp() -> f64` 读取（无前置，与 T08 事件槽同套路）。
   建议先 b 落地，T15 就绪后再提供 a。
2. 回调内 `clearInterval` 自己：C++ 侧遍历 timer 列表时要容忍回调中修改列表（拷贝或延迟删除）。
3. rAF 与 `processTimers` 的调用顺序对齐浏览器习惯：timers 先、rAF 后、然后渲染。引擎钩子已存在且顺序正确（事实 F11：`tickProcessScriptTasks` 内 `processPendingTasks` → `tickTimers` → `tickAnimationFrames`，后者在 Porffor 下是空实现）——rAF 队列接入 `tickAnimationFrames` 即可。
4. **timer id 现状不可靠（F9）**：id = `timers_.size()+1`，删除后会重复。做 clearTimeout/clearInterval 前先改为单调递增 id + map。

## 验收标准

1. 用例：`setInterval` 每 100ms 更新计数文本，5 次后回调内自我 `clearInterval`，最终文本 = "5"。
2. 用例：rAF 链式自续（回调里再次 `requestAnimationFrame` 自己）驱动一个属性动画 60 帧，帧计数正确，`cancelAnimationFrame` 能停。
3. 回调 export 不存在时（拼写错误）：debug 日志 + no-op，不 crash。

## 完成记录

- **日期**: 2026-07-04
- **commit**: `e273b9d`（host 实现）、`5edaaef`（repro + 完成记录）
- **摘要**: 实现 `setInterval`/`clearInterval`/`clearTimeout`、`requestAnimationFrame`/`cancelAnimationFrame`；timer id 改为单调递增 map（修复 F9）；rAF 支持 `dong_raf_timestamp()` 全局槽与 T15 f64 带参 export；`tickAnimationFrames` 接入 PorfforHost；回调中修改 timer 列表安全。
- **变更文件**:
  - `dong/src/script/porffor/dong_porf_host.hpp/cpp`
  - `dong/src/script/porffor/js_bindings_porffor.cpp`
  - `dong/src/script/porffor/dong_porffor_prelude.js`
  - `dong/scripts/porffor_compile.mjs`
  - `docs/developer/porffor/tasks/repro/t09/`
- **验收命令**: `E:\ws\infra\dong\.tools\node-v22.16.0-win-x64\node.exe docs/developer/porffor/tasks/repro/t09/t09_verify.mjs`
- **遗留问题**: clang native 集成测试需本机 clang；engine 内端到端 timer/rAF 待 T13。
