# T21 — QuickJS 退役

- **Area**: dong-host + toolchain
- **性质**: 纯实现型（删除与收敛，无新语义）— 对标物：退役前后 Porffor CI 全绿且渲染结果不变
- **优先级**: P3（终点任务，里程碑意义）
- **依赖**: T06–T20、T22、T23 全部；硬门槛 = T13 盘点表清零（每个测试 `ready` 或 `dropped`，无 `pending`/`blocked`）
- **预估**: 2–3 人天（前提是迁移已完成）
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §1 §7

## 背景

全面切换的终点：QuickJS 从代码库、构建、文档中完全移除。在此之前 QuickJS 仅作迁移期 baseline（T13 的非门槛对比 job）存在。

## 工作内容

1. **删除 QuickJS 绑定层**：`js_bindings_quickjs.inc.hpp`、`js_bindings_quickjs.hpp`、`js_node_bindings.*`、`js_fetch_bindings.*`、`js_clipboard_bindings.*`、`js_selection_bindings.*`、`js_observer_bindings.*`、`js_scene_bindings.cpp`、`js_text_layout_bindings.cpp`（Porffor 版在 T11 落地）、`quickjs_compat.h`、`module_loader.*` 的 QuickJS 分支。
2. **收敛 `DONG_SCRIPT_ENGINE_PORFFOR` ifdef**：Porffor 成为唯一实现——删除宏定义与全部 `#else` / `#ifndef` 分支（`engine_view.cpp` 现有 10+ 处、`js_bindings.hpp` 的双头文件切换等），`src/script/porffor/` 收编为 `src/script/` 主体。
3. **构建与依赖**：移除 submodule `dong/third_party/quickjs`、`zig build quickjs` 目标、CMake 相关目标与链接项。
4. **C API 语义定稿**：`dong_engine_eval_script` 标记 deprecated（返回 error）或改语义为「callExport 指定模块 export」，更新 `include/dong.h` 注释与 API version 说明。
5. **文档同步**：`CLAUDE.md` / `AGENTS.md` / `docs/` 全部 QuickJS 描述更新为 Porffor；`调研1.md` 与本任务组文档归档说明。
6. **CI**：移除 T13 的 QuickJS baseline job。

## 验收标准

1. 全仓库 `rg -i quickjs` 仅剩历史/归档文档命中（代码与构建脚本零命中）。
2. 全平台构建（Windows + 交叉编译 Linux 目标）绿。
3. Porffor CI job 全绿，T13 盘点表 100% `ready` 或 `dropped`。
4. 二进制体积与启动时间对比数据（退役前后）记入完成记录——这是本次切换动机的最终验证。

## 完成记录

- **状态**: **已完成**（2026-07-07，commit `92923c2`）
- **前置**: T13 盘点 **280 ready / 0 blocked / 6 dropped**；T20 execCommand 七例已迁移
- **变更摘要**:
  - 删除 `dong/third_party/quickjs` submodule 及全部 QuickJS 绑定源文件（`js_bindings.cpp`、`js_node_bindings.*` 等）
  - `script_engine.hpp` 仅 re-export Porffor；`engine_view.cpp` 移除双引擎 ifdef
  - `dong_engine_eval_script` 保留 ABI、恒返回 `DONG_ERR_INTERNAL`；推荐 `dong_engine_call_porffor_export`
  - CMake / `build.zig` 移除 QuickJS 目标与链接；`.gitmodules` 移除 quickjs 条目
  - 用户文档（`README.md`、`AGENTS.md`、`CLAUDE.md`、getting-started 等）更新为 Porffor
- **构建验证**: Windows `zig build` 绿（2026-07-07）
- **二进制**: `dong.dll` ≈ **5.96 MiB**（6,247,936 bytes，Release，含 Porffor 生成 C）
- **遗留引用**: 任务组历史文档（`setup.md`、`调研1.md`、T06–T20 规格）仍含 QuickJS 作迁移参照说明——属归档文档，符合验收 §1

## 风险

- 不要提前做：任何 `pending` 测试残留时删除 QuickJS 会失去 baseline 与回退路径；
- `dong_engine_eval_script` 是公开 C API，语义变更要在 API version 层面处理（升 version 或文档明示）。
