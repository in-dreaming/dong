# Porffor 异步约定（T19 定稿）

> 统一 T08 事件槽、T09 定时器/rAF、T10 fetch 结果槽的 callback 模型。Porffor 路径**禁止** `Promise` / `async` / `await`（见 `dong/scripts/porffor_lint.mjs`）。

## 1. 回调形态

- 回调一律 **export 名字符串**；C++ 在适当时机 `callExport(module, exportName)`。
- 模块内通过 prelude 注册 listener：`addEventListener(nodeId, type, 'myHandler')`。
- 定时器 / rAF / fetch：`setTimeout('onTick', 100)`、`dongFetch(url, 'onData')`。

## 2. 结果槽（host → wasm 字符串 / 标量）

| 场景 | 读取 API | 生命周期 |
|------|----------|----------|
| 通用字符串 | `pullHostString()` after `dong_*` prepare import | 同 T16 |
| Fetch | `fetchRequestId()` / `fetchStatus()` / `fetchOk()` / `fetchBody()` / `fetchError()` | **仅回调执行期间** |
| Event | `eventType()` / `eventTarget()` / … | **仅 handler 执行期间** |

槽外读取：返回空 / 0，host 打 debug 日志（不 crash）。

## 3. 失败路径

- Fetch：失败与成功共用同一 export；`fetchOk() === 0` 且 `fetchError()` 非空。
- 不设单独 error 回调 export，减少样板。

## 4. 可重入（T08）

- `callExport` save/restore active module 与 `result_slot` 栈。
- 事件槽 `pushEventSlot` / `popEventSlot`；fetch 槽 `pushFetchSlot` / `popFetchSlot`。
- 嵌套 timer / 事件 / fetch 回调按栈顺序恢复。

## 5. 帧驱动

引擎每帧（`tickProcessScriptTasks`）顺序：

1. `processFetches()` — 完成队列 → callExport
2. `processTimers` / `processAnimationFrames`
3. 布局 flush

## 6. 与 QuickJS 的差异

| QuickJS | Porffor |
|---------|---------|
| `fetch().then()` | `dongFetch(url, 'onData')` + 结果槽 |
| `setTimeout(fn)` | `setTimeout('exportName', ms)` |
| `addEventListener(..., fn)` | `addEventListener(..., 'exportName')` |
| Promise 微任务 | **不支持** |

## 7. 引用

- T08：`dong_event_*` imports
- T09：`dong_set_interval` / `dong_request_animation_frame`
- T10：`dong_fetch_*` imports
- 禁用 lint：`porffor_lint.mjs`（编译期 / 内联 handler 提取）
