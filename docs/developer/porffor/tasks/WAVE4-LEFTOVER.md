# Wave 4 遗留事项（2026-07-06）

Wave 4 任务 T18–T23 主体已合入 `feature/porffor`（**commit `394884a`**）。本地可运行验证：**`87b5789`**（Porffor 内存初始化 + Windows 构建修复）。**T21 QuickJS 退役** 未执行。

## 合入摘要

| 任务 | 状态 | 要点 |
|------|------|------|
| T18 | ✅ | `framework-spec.md` 评审通过 |
| T22 | ✅ | `porffor_framework_compile.mjs` + lint + 快照单测；`porf_counter` 编译链路 |
| T23 | ✅ | 三示例 + `components.md`；todo/game-ui 手写 Porffor 模块 |
| T21 | ⏸ | **blocked** — T13 仍有 ~147 pending |

## 遗留 — T21 QuickJS 退役

1. **T13 盘点未清零**：ready ≈138/285；需批量迁移或标 `dropped` 后方可删 QuickJS。
2. **`rg -i quickjs`** 仍大量命中 `src/script/`、`third_party/quickjs`。
3. **`dong_engine_eval_script`** 公开 API 语义变更需 API version 升级。

## 遗留 — 框架 / 工具链

1. **`{#each}` 编译器**：todo 当前手写 `porfRebuildList`；应迁入 `porffor_framework_compile.mjs`。
2. **porf-partial 组件内联**：规格 §5 未实现；组件仍为 HTML+JS 并列维护。
3. **三示例 baseline 截图**：需 `zig build` + Playwright/`html_baseline_render.py` 多帧对比 Preact 版。
4. **CI**：`porffor_framework_compile.test.mjs` 未接入 GitHub Actions（同 Wave3 Porffor job）。

## 遗留 — 示例 / 产品

1. **Preact 示例保留**：迁移期并存；文档指向 `porf-*` 为新默认。
2. **game-ui 分数随机增量**：Preact 用 `random*10`；Porffor 版固定 +5（可后续对齐）。
3. **todo 字符串 trim**：Porffor 无 `.trim()` — 依赖非空输入；可 host 化或 fork 补 string API。

## 遗留 — 测试

1. ~~新增示例 HTML 可标 `porffor: ready` 进 T13 runner~~ — `run-porffor-tests.mjs` 已扫描 `data/porf-*`（`96ac9e8`）。
2. CE/selection 类仍 blocked(T20)。

## Wave 5 入口

- T13 长尾迁移（147 pending → ready/dropped）
- T21 QuickJS 退役（盘点清零后）
- 框架 `{#each}` / partial 编译器增强

## 验证命令

```powershell
$node = "E:\ws\infra\dong\.tools\node-v22.16.0-win-x64\node.exe"
& $node dong\scripts\porffor_framework_compile.test.mjs
& $node dong\scripts\porffor_compile.mjs
cd dong; zig build run-porffor-framework-test
# 全量 native 构建需：CMake + Ninja + clang-cl + Windows SDK（见 build.env.windows.example）
```

## 构建环境备注（2026-07-06）

- **子模块**：`gpu` 指针已从失效 commit 更新至 `origin/main`（`8b0f2bc`）；`git submodule update --init --recursive` 初始化 ffmpeg/sdl/harfbuzz 等。
- **Node**：`build.zig` 自动探测 `../.tools/node-v22.16.0-win-x64/node.exe`；可设 `NODE_EXE` in `build.env`。
- **HarfBuzz**：无 amalgamation 时 Windows 走 CMake 路径（不再要求预编译 `.lib`）。
- **示例**：`zig build` 安装时复制 `porf-counter` / `porf-todo-classic` / `porf-game-ui` 至 `zig-out/bin/data/`。
