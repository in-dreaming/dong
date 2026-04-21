---
name: feature-cli-dispatch
description: >-
  Hand a prepared feature worktree to a configurable CLI tool (default
  `cursor-cli`, user can swap) so the CLI implements the spec autonomously.
  Runs the CLI in the background, captures session log, writes a lock file,
  and emits ledger events. Use after feature-worktree-setup, before
  feature-verify.
---

# Feature CLI Dispatch

## Why this is a separate skill

cli 的实际命令、参数、prompt 模板在不同环境会变（cursor-cli / codex / 自研）。把 dispatch 单独抽出来，方便用户**只改本 skill 与 `dong/.orchestration/config.json` 而不动主 prompt**。

## When to Use

仅在 [feature-worktree-setup](../feature-worktree-setup/SKILL.md) 已经成功且对应 feature 当前状态 = `prepared` 时调用。

## Inputs

| 字段 | 必填 | 说明 |
|---|---|---|
| `feature_id` | ✓ | |
| `worktree_path` | ✓ | 来自 setup skill 输出 |
| `spec_path` | ✓ | 同上 |
| `cli_tool` |   | 默认从 `dong/.orchestration/config.json` 读取，缺省 `cursor-cli` |
| `extra_context_paths[]` |   | 额外塞给 cli 的参考文档路径（如依赖矩阵） |
| `retry_feedback_path` |   | 若是 verify 失败重试，把 verify 报告 md 路径塞进来 |

## CLI Configuration

`dong/.orchestration/config.json`（**用户编辑**）：

**Windows**：若 `command` 为 PATH 上的 `*.cmd` / `*.bat`，`orch.py` 会用 `cmd.exe /c` 包装（`CreateProcess` 不能直接跑 cmd 脚本）。若子进程仍找不到命令，把 `command` 改成 `where` 输出的**绝对路径**。

```json
{
  "cli_tool": "cursor-cli",
  "tools": {
    "cursor-cli": {
      "command": "cursor-agent",
      "args": ["--workspace", "{worktree}", "--prompt-file", "{prompt_file}", "--non-interactive"],
      "env": { "CURSOR_API_KEY": "$CURSOR_API_KEY" }
    },
    "codex": {
      "command": "codex",
      "args": ["exec", "--cd", "{worktree}", "--prompt-file", "{prompt_file}"]
    },
    "claude-internal": {
      "command": "claude-internal",
      "args": [
        "--bare",
        "-p",
        "Your assigned task is in `.task/prompt.md` ...",
        "--permission-mode",
        "acceptEdits",
        "--max-turns",
        "50"
      ]
    },
    "noop-dryrun": {
      "command": "echo",
      "args": ["DRYRUN", "{feature_id}"]
    }
  }
}
```

placeholder：

| 名 | 值 |
|---|---|
| `{worktree}` | 绝对路径 |
| `{prompt_file}` | dispatch 写出的 prompt md 路径 |
| `{feature_id}` | feature id |
| `{spec_path}` | spec 绝对路径 |

`claude-internal`：**`command` 填你 PATH 上的真实可执行名**（常见为自建 wrapper `claude-internal`，或官方 `claude`；二者择一即可）。`orch` 将进程 `cwd` 设为 `{worktree}`，`-p` 里让模型用 **Read** 打开 `.task/prompt.md`（与 `{prompt_file}` 同路径）；args 中可不写占位符。

## Procedure

1. **加载配置**：
   - 读 `dong/.orchestration/config.json`。无则报错，提示用户先创建。
   - 选 `cli_tool` 对应条目。

2. **生成 cli prompt**（写到 `worktree_path/.task/prompt.md`）：

   模板（必须包含这些 section，措辞可调）：

   ```markdown
   # Task: <feature_id> <名称>

   你被分配实现一个 dong 项目的 feature。所有上下文已在你工作目录内。

   ## 强制约束

   - 你只能改本 worktree 内的文件；不要 cd 到 worktree 外。
   - 不要 push 任何分支。不要改写 git 历史。
   - 你的最终交付 = 一组本地 commit（逻辑切分），分支已就位。
   - 必须满足 spec 的 § 5 验收（Hard 全过；Soft 尽量过）。
   - 若与 spec 矛盾，先在 .task/notes.md 记原因，再选最接近 spec 意图的实现。
   - 不要修改 doc/orchestration/、doc/phase*/ 下任何文件。

   ## 必读文档

   1. .task/spec.md            ← 你的 spec
   2. CLAUDE.md / AGENTS.md    ← 项目约定
   3. doc/positioning.md
   4. doc/perf_budget.md       ← 性能门槛
   {extra_context}

   ## 工作流建议

   1. 读 spec 全文 → 在 .task/plan.md 写实现计划。
   2. 按 spec § 4 实现步骤逐步推进，每个步骤 1 commit。
   3. 跑 spec § 5 列出的验证命令；失败就改到通过。
   4. 在 .task/notes.md 记下偏差、未实现项、已知风险。

   ## Retry 反馈（仅重试时存在）

   {retry_feedback_block}

   ## 完成的判定

   - .task/plan.md 存在
   - 所有 spec § 5 Hard 命令本地跑通（你可自己跑一遍验收 dry run）
   - `git status` 干净（所有改动已 commit）
   - 在 .task/notes.md 末尾写一行 `STATUS: ready-for-orchestrator-verify`

   完成后退出 cli 进程。
   ```

   - `{extra_context}` 把 `extra_context_paths` 渲染为列表。
   - `{retry_feedback_block}` 仅当重试存在；正常派发为空。

3. **建 session log 目录**：
   ```
   dong/.orchestration/reports/<feature_id>/
   ```

4. **启动 cli（后台）**：
   - 用 Shell 工具 `block_until_ms: 0`。
   - 命令：根据 config 渲染。
   - 输出 redirect 到 `reports/<id>/cli_session.log`。

   推荐 wrapper（PowerShell / bash 通用思路）：让 cli 进程的退出码也写到 `reports/<id>/cli_exit.txt`，方便后续读。

5. **写 lock 文件**：
   ```json
   {
     "feature": "<id>",
     "pid": <pid>,
     "host": "orchestrator",
     "started_at": "<iso>",
     "cli_tool": "<tool>",
     "cli_session": ".orchestration/reports/<id>/cli_session.log"
   }
   ```

6. **追加 ledger**：
   ```jsonl
   {"event":"cli_dispatched","feature":"<id>","payload":{"cli_tool":"<tool>","cli_args":[...],"task_id":"<shell-task-id>","session_log":".orchestration/reports/<id>/cli_session.log"}}
   ```

7. **不阻塞**：返回 `task_id` 给 orchestrator；监控/验收由后续 tick 进行。

## Monitoring (called from orchestrator main loop)

每个 tick：

- `Await` 该 task_id（block 短，例如 5–10 s）。
- 若任务结束：
  - 读 `cli_exit.txt`，写 ledger `cli_finished` 含 `exit_code`、`duration_sec`、`files_changed = git diff --stat dev_next..HEAD | wc -l`。
  - 状态 → `awaiting_verify`。
- 若仍跑：
  - 读 session log 末尾 30 行，若有"STATUS: ready-for-orchestrator-verify" → 视为完成（cli 自报）。
  - 若 > 2 h 无新输出 → 写 `note: stale`，提示用户。

## Failure Modes

| 现象 | 动作 |
|---|---|
| cli 命令找不到 | ledger `note`，状态 → `blocked` |
| cli 异常退出 (exit ≠ 0) | 自动重试 1 次（在 dispatch 入口由 orchestrator 决定 retry 次数）；超限 → blocked |
| cli 修改了 worktree 外文件 | verify skill 会发现；视为 fail，retry |
| cli 推了远端分支 | 立刻 rollback：`git push --delete origin feat/<id>-<slug>`，状态 → blocked，提示用户 |

## Conventions

- cli 完全在 worktree 内自治，**不允许**调 `dev_next` / 主仓库工作目录。
- prompt 不嵌入 cli 工具的 vendor-specific 指令（保持工具无关）。
- 所有 prompt 调整都在 `prompt.md` 模板里改；本 skill 不要硬编码。
