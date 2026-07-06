# Wave 3 遗留事项（2026-07-05）

Wave 3 任务 T10–T14、T19、T20 已合入 `feature/porffor`（**commit `8d34b91`**）。以下为**未在本波完成**或需后续 wave 承接的项。

## 合入摘要

| 任务 | 状态 | 要点 |
|------|------|------|
| T10 | ✅ | callback fetch + 结果槽 + `t10_verify.mjs` |
| T11 | ✅ | clipboard / matchMedia / cssSupports / dialog / scene / textLayout host + prelude |
| T12 | ✅ | `porffor_inline_handlers.mjs` build 期提取；运行时 stub 接管 |
| T13 | ✅ | 标记约定、`run-porffor-tests.mjs`、盘点表、8 个试点 ready |
| T14 | ✅ | `--call-export-after-frame0`、`porffor_snippet_compile.mjs`、snippet 盘点 |
| T19 | ✅ | 异步约定、Promise 禁用 + `porffor_lint.mjs` |
| T20 | ✅ | 决策矩阵 + parseHtml / formSerialize / selectionText |

## 遗留 — 构建 / CI

1. **GitHub Actions Porffor job**：`run-porffor-tests.mjs` 脚本已就绪，需在 CI workflow 中增加 job（依赖 `zig build` 产物 `html_render_test`）。
2. **全量 `ready` 集渲染**：当前约 138/285 ready；147 pending 待批量迁移（长尾）。
3. **`porffor_compile` 全量模块链接耗时**：>10 模块已测 <3s；150 模块需 CI 实测后决定是否分 registry。
4. **QuickJS baseline job（非门槛）**：T13 规格要求保留对比 job，T21 前移除。

## 遗留 — 运行时 / 原生验证

1. **T10 fetch**：HTTP 并发 / abort / 404 需 `html_render_test` 或集成测试（clang 步骤 SKIP）。
2. **T11 scene**：`test_scene_graph.html` Porffor smoke 未在 CI 跑通。
3. **T11 textLayout / overlay**：`drawRect` / `renderText` prelude 已注册 import 与否需与示例对齐。
4. **T17 交叉**：`3d_screens` 多屏 Porffor smoke（Wave 2 遗留）。

## 遗留 — 工具链

1. **T12 与 manifest 全自动**：内联 handler 提取需接入 `zig build` 默认路径（当前脚本可手动跑）。
2. **T14 试点多帧**：3 个 snippet 试点需 `html_render_test` 构建后逐帧对比 baseline（本机未跑 dong 二进制）。
3. **内联 handler 编译失败 UX**：部分复杂 handler 仍需更清晰 HTML 行号（acorn parse error 映射）。

## 遗留 — API / 产品

1. **Promise 子集**：按 T19 **不做**；若战略变更需新 fork 任务。
2. **T20 (b) 类**：~23 个 CE/selection 测试 → C++ 输入驱动迁移（Wave 4 / T22 前后）。
3. **T20 IME**：composition 事件字段扩展（`dong_event_data`）未做。
4. **fetch `dong_fetch_start_ex`**：POST / headers 进阶版（T10 规格可后补）。
5. **二进制 response body**：fetch 仅文本/JSON（规格已写明）。

## 遗留 — 文档

1. **setup.md F 表**：可增 F16 fetch 槽、F17 测试标记约定（可选）。
2. **T13 盘点表编码**：重新生成时确保 UTF-8（`node scripts/porffor_test_inventory.mjs`）。

## Wave 4 状态（2026-07-06）

T18–T23 已合入；详见 [`WAVE4-LEFTOVER.md`](./WAVE4-LEFTOVER.md)。T21 QuickJS 退役 **未执行**（T13 pending 未清零）。

## Wave 4 入口（归档）

- T18 规格评审 → T22/T23 框架
- T21 QuickJS 退役（以 T13 ready 清零 + 框架示例替换为前提）

## 验证命令（本机）

```powershell
$node = "E:\ws\infra\dong\.tools\node-v22.16.0-win-x64\node.exe"
cd E:\ws\infra\dong\dong\third_party\porffor
& $node ../../../docs/developer/porffor/tasks/repro/t10/t10_verify.mjs
& $node ../../scripts/porffor_test_tags.test.mjs
cd E:\ws\infra\dong\dong
& $node scripts/porffor_test_inventory.mjs
```
