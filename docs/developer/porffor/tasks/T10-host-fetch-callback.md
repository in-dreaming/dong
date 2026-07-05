# T10 — callback 版 `fetch`

- **Area**: dong-host
- **性质**: 纯实现型 — 对标物：本任务 API 表（规格即任务文件）+ QuickJS 版 `js_fetch_bindings.cpp` 的网络行为（协议支持、错误分类）
- **优先级**: P2
- **依赖**: T08（复用"完成时 callExport + 结果槽"模式）、T16（body 字符串通道）
- **预估**: 2–3 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §4 §8

## 背景

Porffor 的 `Promise`/`async` 有 known bugs（setup §4），评估结论：**不做原生 `fetch().then()`，做 callback 版**。callback 是唯一官方异步模型（全面切换，无 QuickJS 兜底）；Promise 子集是否开放由 T19 评估定夺，在此之前 `fetch().then()` 形式的测试一律改写为 callback 形式（改写指引进 T13 迁移清单）。

## 模型

```js
// Porffor 侧脚本
export function onData() {
  const status = fetchStatus();      // import
  const body = fetchBody();          // import，本次完成请求的 body
  setTextContent(el, body);
}
dongFetch('ui://data.json', 'onData');   // 发起，立即返回 requestId
```

C++ 侧：请求完成后，在**下一帧主线程**把结果写入"当前 fetch 结果槽"，然后 callExport `onData`，调用结束后清槽。与事件槽（T08）同一套路。

## API

| import | 说明 |
|--------|------|
| `dong_fetch_start(url, exportName) -> requestId` | GET；url 支持引擎资源协议与 http(s)（按引擎现有网络栈能力） |
| `dong_fetch_start_ex(url, method, bodyStr, headersJson, exportName) -> requestId` | 进阶版，可后补 |
| `dong_fetch_abort(requestId)` | |
| 结果槽读取 | `dong_fetch_request_id() -> i32`、`dong_fetch_status() -> i32`、`dong_fetch_ok() -> i32`、`dong_fetch_body() -> str`、`dong_fetch_header(name) -> str`、`dong_fetch_error() -> str`（网络失败时非空） |

## 设计点

1. 多个并发请求共用一个回调 export 时靠 `dong_fetch_request_id()` 区分。
2. 结果槽生命周期 = 该次回调执行期间；回调外读取返回空 + debug 日志。
3. 二进制 body 暂不支持（游戏 UI 场景 JSON/文本够用），文档写明。
4. 失败路径也走同一回调（`ok=0` + `error` 非空），不设单独 error 回调，减少 export 样板。

## 验收标准

1. 用例：加载本地 JSON 资源 → 回调内 `JSON.parse` → 更新 DOM 文本，通过。
2. 用例：404 与非法 url，回调收到 `ok=0`，页面显示错误文本，不 crash。
3. 并发 2 个请求各自回调正确配对（requestId 断言）。
4. abort 后回调不触发。

## 完成记录

- **日期**: 2026-07-05
- **Commit**: (见 feature/porffor 分支)
- **实现要点**:
  - callback 版 fetch：`dong_fetch_start(url, exportName) -> requestId`，下一帧 `processFetches` 写入 fetch 结果槽并 `callExport`
  - 结果槽：`fetchRequestId` / `fetchStatus` / `fetchOk` + T16 `pullHostString` 读 `fetchBody` / `fetchError`；回调外读空 + debug 日志
  - 网络：`ResourceLoader::loadTextResource`（本地/`ui://`/http）；HTTP 工作线程，主线程 dispatch
  - `fetchAbort` 取消 pending，完成时不触发回调
- **Prelude**: `dongFetch`, `fetchStatus`, `fetchBody`, `fetchError`, `fetchAbort`, `fetchRequestId`, `fetchHeader`
- **验证**: `node docs/developer/porffor/tasks/repro/t10/t10_verify.mjs`（2c codegen + host wiring + T08/T16 回归）
- **遗留**: 二进制 body 未支持；`fetch_header` 暂无响应头（ResourceLoader 未返回 headers）；`dong_fetch_start_ex` 未实现
