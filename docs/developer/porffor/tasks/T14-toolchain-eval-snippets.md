# T14 — headless eval snippet 的预编译替代

- **Area**: toolchain
- **性质**: 纯实现型（含一次 snippet 盘点，产出并入 T13 分类表）— 对标物：QuickJS eval 路径的多帧渲染结果（迁移前后逐帧截图一致）
- **优先级**: P2
- **依赖**: T13（标记与 runner 基建）
- **预估**: 2–4 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §4

## 背景

Dong 的多帧回归测试（contentEditable bold 等）在指定帧后向 QuickJS eval 一段 snippet 驱动交互，实际入口是 `html_render_test --eval-after-frame0-file <path>`（读 JS 文件，事实 F14；配套脚本见 `scripts/tools/verify_*_headless.py`）。Porffor 无运行时 eval，`dong_engine_eval_script` 在 Porffor 路径**故意禁用**（不要绕）。替代方案二选一组合：

- **方案 A（本任务主体）**：build 期把每个测试的 snippet 文件预编译为一个 handler 模块（一个 export），测试 runner 在目标帧 callExport，代替 eval。
- **方案 B（部分测试更合适）**：交互改由 C++ 侧模拟输入事件驱动（不经 JS），逐测试判断；与 T20 的「CE/IME 下沉 C++」决策衔接。

> 注：Porffor 支持**编译期已知字符串**的 eval（setup §4），若 snippet 以字符串字面量内联进生成模块，理论上可直接 `eval(SNIPPET)`。实现时可实验此捷径，但不作为验收依赖——生成具名 export 更稳。

## 工作内容

1. **snippet 清单**：扫描测试与脚本（`--eval-after-frame0-file` 的所有引用、`scripts/tools/verify_*_headless.py`、snippet 文件目录），枚举全部 snippet 与其代码，产出分类表：**可预编译（方案 A）/ 需方案 B / dropped 候选**（无 QuickJS 类别）。分类结果并入 T13 盘点表。
2. **生成器**：为「可预编译」类，把 snippet 包装为 `export function __snippet_<testname>_<frame>() { ... }` 模块，走既有 porf 编译与 manifest 管线（与 T12 生成器共享代码；handler 形态按 T15 结论）。
3. **runner 扩展**：`html_render_test` 新增 `--call-export-after-frame0 <exportName>`，Porffor 模式下替代 `--eval-after-frame0-file`；由 T13 标记决定每个测试走 A、B 还是 dropped。
4. **试点**：至少 3 个多帧测试完成迁移并在 CI Porffor job 通过。

## 验收标准

1. snippet 分类清单合入（后续迁移的工作依据，并入 T13 盘点表）。
2. 生成器 + runner 扩展合入，3 个试点测试与 QuickJS baseline 渲染结果一致。
3. 「需方案 B」的测试有逐条去向（C++ 驱动改造任务或 dropped 标记），无悬空项。

## 完成记录

- `dong/scripts/porffor_test_tags.mjs`：标记解析（ready/pending/blocked/dropped）+ 默认规则；`porffor_test_tags.test.mjs` 单测。
- `dong/scripts/porffor_test_inventory.mjs` + `generate_t13_inventory.ps1` → `T13-test-inventory.md`（285 测，ready 138 / pending 147）。
- 构建组织选型：方案 a) 单 runner 全量 registry，写入盘点 doc。
- 试点迁移 11 个 ready：2 静态显式标记 + 6 script 模块 + 3 T14 多帧 pilot；`porffor_manifest.json` 注册 8 个新模块。
- `zig build run-porffor-tests` + `scripts/run-porffor-tests.mjs` CI runner（ready 集 + 覆盖率摘要）。
- `docs/developer/porffor/porffor-migration-checklist.md` 迁移 checklist。
- 验证：`porffor_test_tags.test.mjs`（需 node）；盘点由 PS1 生成。全量渲染需 `zig build` 后 `run-porffor-tests`。
