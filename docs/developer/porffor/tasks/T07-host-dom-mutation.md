# T07 — DOM 结构修改 import 集

- **Area**: dong-host
- **性质**: 纯实现型 — 对标物：QuickJS 版 DOM 结构操作行为（`js_node_bindings.cpp` 的 appendChild/insertBefore/remove 等）；生效时机等 Dong 特有语义在「关键设计点」内定稿后按其执行
- **优先级**: P1
- **依赖**: T06（复用其字符串/JSON 约定与测试脚手架）
- **预估**: 2–4 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §8

## 背景

覆盖 `test_dom_manipulation.html` 一类用例：创建/插入/删除节点。评估结论：**中~高可行，纯 Dong 侧**。注意游戏 UI 场景下 DOM 修改需与引擎的布局/渲染帧同步——沿用现有 stage/commit 或帧末批处理策略，由 C++ 侧决定生效时机。

## API 清单

| import | 说明 |
|--------|------|
| `dong_create_element(tag) -> nodeId` | 新节点先挂在"游离池"，未插入树 |
| `dong_create_text_node(text) -> nodeId` | |
| `dong_append_child(parentId, childId)` | |
| `dong_insert_before(parentId, newId, refId)` | `refId=0` 等价 append |
| `dong_remove(nodeId)` | `element.remove()`；从树摘除并回收判定 |
| `dong_replace_child(parentId, newId, oldId)` | |
| `dong_parent(nodeId) -> nodeId` / `dong_first_child` / `dong_next_sibling` | 遍历（返回 0 = null） |
| `dong_clone_node(nodeId, deep_i32) -> nodeId` | 可选，视测试需要 |

## 关键设计点（实现前先在任务 PR 描述里定稿）

1. **nodeId 生命周期**：removed 节点的 id 是否失效？建议：id 永久有效直到 C++ 侧引用计数归零；对已销毁 id 的操作静默 no-op + debug 日志，绝不 crash。
2. **批处理时机**：结构修改立即生效 vs 帧末 commit。与现有 `setTextContent` 的 stage/commit 语义保持一致。
3. **表单集合类**（`form.elements`、`checkValidity`、`submit`）不在本任务，如测试急需另立小任务，返回 JSON 走 T06 的约定。
4. **nodeId 反查性能**：现状 `getNodeIdFor` 是线性反查 O(n)（setup §3 事实 F8），本任务的 500 节点压力用例会退化成 O(n²)——顺带补 node→id 反向索引（`unordered_map<void*, uint64_t>`）。

## 验收标准

1. `test_dom_manipulation.html` 的 Porffor manifest 版通过。
2. 压力用例：循环 `createElement` + `appendChild` 500 节点再逐个 `remove`，无泄漏（C++ 侧节点计数回落）、无 crash。
3. 对无效 nodeId 的操作不 crash（含 0 与已删除 id）。

## 完成记录

- **日期**: 2026-07-04
- **commit**: （见下方 git commit）
- **摘要**: 实现 T07 DOM 结构修改 import 全集；`getNodeIdFor` 补 F8 反向索引 O(1)；无效/已释放 nodeId 静默 no-op + debug 日志；remove/replace 后释放 id。
- **变更文件**:
  - `dong/src/script/porffor/js_bindings_porffor.hpp/cpp`
  - `dong/src/script/porffor/dong_porf_host.hpp/cpp`
  - `dong/src/script/porffor/dong_porffor_prelude.js`
  - `dong/scripts/porffor_compile.mjs`
  - `docs/developer/porffor/tasks/repro/t07/`
- **验收命令**: `E:\ws\infra\dong\.tools\node-v22.16.0-win-x64\node.exe docs/developer/porffor/tasks/repro/t07/t07_verify.mjs`
- **遗留问题**: clang native 链路依赖本机 clang；engine 内 `test_dom_manipulation.html` 集成待 T13 迁移。
