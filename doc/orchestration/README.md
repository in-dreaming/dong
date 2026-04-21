# Dong Multi-Feature Orchestration

> 把 27 个 phase0/1/2 feature 用 **1 个主 orchestrator + N 个 cli worker** 的形式并行推进到 `dev_next` 分支的全套规则与提示词。
>
> 设计目标：**冲突可控 / 状态可追 / 验收可重复 / 合并可回滚**。
>
> 状态：v1 / 2026-04-17。

---

## 1. 入口

| 角色 | 起点 |
|---|---|
| 想看依赖图、wave 调度 | [`dependency-matrix.md`](./dependency-matrix.md) |
| 想跑起来：把哪段 prompt 灌给主 agent | [`orchestrator-prompt.md`](./orchestrator-prompt.md) — **整段复制为 system prompt** |
| 想知道 ledger 长什么样 | [`state-ledger-schema.md`](./state-ledger-schema.md) |
| 想替换 cli 工具 | 改 `dong/.orchestration/config.json` + 看 [skill: feature-cli-dispatch](../../.claude/skills/feature-cli-dispatch/SKILL.md) |

---

## 2. 角色拓扑

```
        ┌──────────────────────────────────────────────┐
        │ User (你)                                    │
        │ - 启动 / 暂停 / approve / reject             │
        └──────────────┬───────────────────────────────┘
                       │ chat
                       ▼
        ┌──────────────────────────────────────────────┐
        │ Orchestrator Agent                           │
        │ system prompt = orchestrator-prompt.md       │
        │ skills:                                      │
        │   - parallel-wave-orchestrator (主循环)      │
        │   - orchestration-state-ledger (状态)        │
        │   - feature-worktree-setup                   │
        │   - feature-cli-dispatch                     │
        │   - feature-verify                           │
        │   - feature-merge-back                       │
        └──┬────────────┬────────────┬─────────────────┘
           │            │            │
           ▼            ▼            ▼
    cli worker A   cli worker B   cli worker C  (≤ MAX_PARALLEL_WORKERS)
    在 dong/.worktrees/<id>/ 内自治实现 spec
```

---

## 3. 物理布局

```
dong/
├── .orchestration/             # gitignore
│   ├── state-ledger.jsonl
│   ├── snapshots/state.json
│   ├── workers/<id>.lock
│   ├── reports/<id>/
│   │   ├── cli_session.log
│   │   ├── verify_<ts>.json / .md
│   │   ├── commit_msg.txt
│   │   └── merge_diff.txt
│   └── config.json             # cli 工具 / 调度参数
├── .worktrees/                 # gitignore
│   ├── P0-1/                   # 一个 feature = 一个 worktree
│   │   └── .task/
│   │       ├── spec.md         # 拷贝自 doc/phase0/...
│   │       ├── plan.md         # cli 写
│   │       ├── notes.md        # cli 写
│   │       └── prompt.md       # dispatch skill 写
│   └── ...
└── (主仓库工作目录始终在 dev_next)
```

---

## 4. 文档清单

| 文件 | 作用 |
|---|---|
| [`dependency-matrix.md`](./dependency-matrix.md) | 27 任务依赖、群组、wave、并发上限 |
| [`orchestrator-prompt.md`](./orchestrator-prompt.md) | 主 agent 的 system prompt 全文 |
| [`state-ledger-schema.md`](./state-ledger-schema.md) | 状态账本 / 报告 / 锁文件格式 |
| `dong/scripts/orch.py` | Python CLI 实现（见 § 6.0） |

## 5. Skill 清单（`.claude/skills/`）

| skill | 触发时机 | 关键产物 |
|---|---|---|
| [parallel-wave-orchestrator](../../.claude/skills/parallel-wave-orchestrator/SKILL.md) | 每次 tick | 调度决定 + 报告 |
| [orchestration-state-ledger](../../.claude/skills/orchestration-state-ledger/SKILL.md) | 任何状态读写 | ledger 行 / snapshot |
| [feature-worktree-setup](../../.claude/skills/feature-worktree-setup/SKILL.md) | 首次派发一个 feature | `.worktrees/<id>` + `feat/<id>-<slug>` 分支 |
| [feature-cli-dispatch](../../.claude/skills/feature-cli-dispatch/SKILL.md) | worktree 已就绪 | cli 后台进程 + lock 文件 |
| [feature-verify](../../.claude/skills/feature-verify/SKILL.md) | cli 自报完成 | `verify_<ts>.json` + 判定 |
| [feature-merge-back](../../.claude/skills/feature-merge-back/SKILL.md) | review_approved | dev_next 上一个新 commit |

---

## 6. 启动指南

### 6.0a 已在 Git 里合过 uber-quad（P0-1），`init` 之后如何标记完成

编排器只看 **ledger**，不会读 Git 历史。若 P0-1 已在 `dev_next`（或你当前分支）上提交、且**没有**走 `dispatch → verify → merge`，在 `init` 之后执行：

```bash
# 可选：显式写上含 uber-quad 的那次提交（推荐，便于审计）
python dong/scripts/orch.py mark-merged P0-1 --merge-sha <commit_sha>

# 或省略：有 dev_next 则用 dev_next 的 HEAD，否则用当前分支 HEAD
python dong/scripts/orch.py mark-merged P0-1

# 带说明
python dong/scripts/orch.py mark-merged P0-1 --merge-sha <sha> --note "manual merge before orch"
```

然后：

```bash
python dong/scripts/orch.py snapshot
python dong/scripts/orch.py status
```

此时 P0-1 在账本里为 `merged`，下游 P0-2 / P0-3 / P0-6 等会按依赖变为可派发。若某 feature 曾被误 `dispatch` 卡在中间态，先 `orch abort <id>` 再 `mark-merged`，或 `mark-merged --force`（慎用）。

### 6.0 两种使用方式

| 方式 | 谁驱动 | 适合 |
|---|---|---|
| **A. Python CLI** `python dong/scripts/orch.py ...` | 你 / CI / cron | **推荐**：确定性高、易脚本化、无需开 agent |
| **B. Agent + skills** | 长期运行的 orchestrator agent | 需要"自己决策 + 给你发通知"的场景 |

两种方式共用同一个 ledger / config / 目录布局，可以互相切换。先用 A 把流程跑通，再决定是否引入 B。

orch.py 子命令速查：

```bash
python dong/scripts/orch.py init                  # 首次初始化（不建 dev_next）
python dong/scripts/orch.py init --create-dev-next  # 同时建 dev_next
python dong/scripts/orch.py status                # 当前状态
python dong/scripts/orch.py list [--phase p0|p1|p2]
python dong/scripts/orch.py eligible              # 现在能派发哪些
python dong/scripts/orch.py tick [--dry-run]      # 推进一个 tick
python dong/scripts/orch.py dispatch P0-1         # 手动派单 feature
python dong/scripts/orch.py verify   P0-1         # 跑 spec § 5 验收
python dong/scripts/orch.py approve  P0-1
python dong/scripts/orch.py merge    P0-1         # 合并回 dev_next
python dong/scripts/orch.py mark-merged P0-1      # 已在 dev_next 上合过（orch 外），只写账本
python dong/scripts/orch.py rollback P0-4 --from-seq 58 --reason "merge failed"  # 撤销账本区间
python dong/scripts/orch.py abort    P0-1
python dong/scripts/orch.py ledger-dump 40
python dong/scripts/orch.py snapshot
python dong/scripts/orch.py config-show
```

### 6.1 准备工作

1. 确认在 `main` 上、`git status` clean。
2. 跑 `python dong/scripts/orch.py init`，会自动：
   - 在根 `.gitignore` 追加 `dong/.orchestration/` + `dong/.worktrees/`
   - 创建 `dong/.orchestration/` 目录骨架与 `config.json` 占位
   - 写 ledger 的 `schema_init` 并登记 27 个 feature
3. （可选）编辑 `dong/.orchestration/config.json`。`"cli_tool": "claude-internal"` 时，`tools.claude-internal.command` 应为你本机 **实际可执行文件名**（例如自建入口 `claude-internal`，或官方 `claude`），并保证在 `PATH` 中（或写绝对路径）。
4. 若用 Claude Code 系 CLI，需已登录或按 [headless 文档](https://code.claude.com/docs/en/headless) 配置 `ANTHROPIC_API_KEY` / `--bare` 等（以你的 wrapper 为准）。

以下为最小示例（可把 `cli_tool` 改为 `claude-internal`）：
   ```json
   {
     "cli_tool": "noop-dryrun",
     "max_parallel_workers": 4,
     "max_per_group_r": 1,
     "max_per_group_y": 2,
     "merge_serial": true,
     "retry_verify": 2,
     "auto_push": false,
     "tools": {
       "noop-dryrun": {
         "command": "echo",
         "args": ["[NOOP]", "{feature_id}", "spec={spec_path}"]
       }
     }
   }
   ```
   先用 `noop-dryrun` 走通整套流程，再换真 cli。

### 6.2 启动 orchestrator agent

把 [`orchestrator-prompt.md`](./orchestrator-prompt.md) 全文设为新会话的 system prompt（cursor / claude-code / codex 均可）。

第一句对它说：

```
启动。
```

它会按 [orchestrator-prompt § 1](./orchestrator-prompt.md#1-启动一次性动作首次运行-orchestrator-时) 跑首次初始化：建 `dev_next`、创建目录、登记 27 个 feature、给你 Wave 0 计划。

### 6.3 把 cli 替换成真实工具

确认流程通后，编辑 `config.json`：

```json
"cli_tool": "cursor-cli",
"tools": {
  "cursor-cli": {
    "command": "cursor-agent",
    "args": ["--workspace", "{worktree}", "--prompt-file", "{prompt_file}", "--non-interactive"]
  }
}
```

orchestrator 不重启即可生效（每次 dispatch 都重新读 config）。

### 6.4 日常交互

| 你说 | orchestrator 做 |
|---|---|
| `go` | 推进一个 tick |
| `list` | 输出当前 features 状态表 |
| `approve P0-1` | 把 P0-1 标 approved，下一 tick 触发 merge |
| `reject P0-1 verify 漏跑性能` | 状态回 cli_running，把理由附给 cli |
| `hold P0-1` | 暂停审；状态停在 awaiting_review |
| `pause` | 不再 dispatch 新 worker |
| `resume` | 解除 pause |
| `abort P0-X` | 杀 cli + ledger feature_done(abandoned) |
| `wave` | 输出 wave 进度卡片 |

---

## 7. 安全护栏（一页速查）

- ✅ 所有写状态都通过 ledger skill。
- ✅ 一个 feature 一个 worktree + 一个分支。
- ✅ R 群组同时 ≤ 1，merge 串行。
- ✅ verify hard fail 必 retry / blocked，不许越过。
- ✅ 验收报告永远存盘可追溯。
- ❌ 永不 push --force / 永不动 main。
- ❌ 永不在 main 工作目录直接 checkout feature 分支。
- ❌ 永不并行 merge。

---

## 8. FAQ

**Q: cli 工具崩了 / 我手动改了 worktree 里的代码？**
A: 都没问题。cli 自动 retry；手动改也会被下一次 verify 检测；只要不动 ledger / 主仓库 / dev_next 就安全。

**Q: 我想跳过某个 feature？**
A: `abort P0-X`。下游会根据依赖矩阵自动判定 blocked 或绕开。

**Q: 我想本地试一个新 cli 工具？**
A: 给 `config.json.tools` 加一项，命名 `experiment`；启动时 `--cli_tool experiment` 传给 dispatch override。

**Q: 出了 bug 想倒推哪个 feature 引入？**
A: ledger `merge_finished` 事件是顺序的；对应 commit 在 dev_next 上是 squash，message 含 feature id；`git log dev_next --grep="\\[P0-1\\]"` 可定位。

**Q: dev_next 想发版了？**
A: 走另外的 release 流程（不在本套 orchestration 范围）。本套只负责 dev_next 内的 feature 流转。

---

## 9. 演进 / 待补

| 项 | 优先级 | 备注 |
|---|---|---|
| ledger / config 写一个 Python CLI（`dong-orch`）替代 agent 直接 shell | 中 | 性能更好，但当前 skill 已够用 |
| GitHub PR 集成（替代/并行本地 merge） | 低 | 需要团队 review 时再做 |
| 远端 wave 仪表板（HTML）| 低 | 可由 dong 自己实现（dogfood） |
| 把 spec § 5 的自然语言全部改写成 yaml 块 | 高 | verify skill 现在 fall back 启发式；改完后报告更可靠 |

---

## 10. 决策记录

| 日期 | 决策 |
|---|---|
| 2026-04-17 | orchestration v1 文档与 6 skill 创建 |
| 2026-04-17 | 单一集成分支 `dev_next`；merge 串行；R 群组 cap=1 |
| 2026-04-17 | cli 工具通过 `config.json` 抽象，本套规则与具体 cli 解耦 |
| 2026-04-17 | 状态账本 append-only JSONL，不进 git |
