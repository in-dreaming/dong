---
name: feature-worktree-setup
description: >-
  Prepare an isolated git worktree + feature branch from `dev_next` (or a
  declared upstream feature), seed the task folder with the spec, and emit the
  ledger events. Use when the orchestrator wants to start a new feature in
  the multi-cli parallel pipeline (see docs/developer/orchestration/orchestrator-prompt.md).
---

# Feature Worktree Setup

## When to Use

仅在 **orchestrator** 决定派发一个新 feature 时调用。每个 feature 一辈子只调一次（rollback 后重启除外）。

## Inputs

| 字段 | 必填 | 说明 |
|---|---|---|
| `feature_id` | ✓ | 例 `P0-1` |
| `slug` | ✓ | kebab-case，例 `uber-quad` |
| `spec_path` | ✓ | 例 `docs/developer/phase0/P0-1_uber_quad_pipeline.md` |
| `groups[]` | ✓ | 来自 dependency-matrix § 2.3，例 `["render-core"]` |
| `upstream[]` |   | 已 merge 的 feature id 列表（不影响 base，仅记录） |
| `base_ref` |   | 默认 `dev_next` |

## Procedure

1. **前置检查**：
   - `cd dong` 后 `git fetch origin`。
   - `git rev-parse --verify dev_next` 必须成功；否则报错"orchestrator 启动步骤未跑"。
   - `git status` 在主仓库必须 clean。
   - `dong/.worktrees/<feature_id>` 不存在。否则：检查 `dong/.orchestration/workers/<feature_id>.lock`：
     - 锁存在 → **拒绝执行**，报告 conflicting worker。
     - 锁不存在 → `git worktree remove --force dong/.worktrees/<feature_id>` 后继续。

2. **同步基线**：
   ```
   cd dong
   git switch dev_next
   git pull --ff-only origin dev_next
   ```

3. **创建 worktree + 分支**：
   ```
   git worktree add -b feat/<feature_id>-<slug> .worktrees/<feature_id> dev_next
   ```
   分支名固定 `feat/<id>-<slug>`，方便 grep。

4. **建立任务目录**：
   ```
   .worktrees/<feature_id>/.task/
     spec.md                  # 拷贝自 spec_path（不要软链，跨平台）
     plan.md                  # 占位，cli 写
     notes.md                 # 占位
     verify-config.json       # 解析 spec § 5 后由 verify skill 写
   ```
   `.task/` 加入 worktree 内 `.git/info/exclude`（**不进 commit**）。

5. **快照 base sha**：
   ```
   git -C .worktrees/<feature_id> rev-parse HEAD
   ```

6. **追加 ledger**（顺序、同一 tick 内连写）：
   ```jsonl
   {"event":"prepare_started","feature":"<id>","payload":{"branch":"feat/<id>-<slug>","worktree_path":".worktrees/<id>","base_ref":"dev_next"}}
   {"event":"prepare_finished","feature":"<id>","payload":{"commit_sha":"<sha>"}}
   ```
   公共字段（ts/actor）由 [orchestration-state-ledger](../orchestration-state-ledger/SKILL.md) 注入。

7. **创建 lock 占位**（cli 启动前先空着，dispatch skill 会填 pid）：
   ```
   dong/.orchestration/workers/<feature_id>.lock   (空文件)
   ```

## Outputs

```json
{
  "feature_id": "P0-1",
  "branch": "feat/P0-1-uber-quad",
  "worktree_path": "dong/.worktrees/P0-1",
  "base_sha": "ab12cd34",
  "task_dir": "dong/.worktrees/P0-1/.task"
}
```

## Failure Modes

| 现象 | 动作 |
|---|---|
| `dev_next` 不存在 | 拒绝；返回 `error: dev_next missing`；让 orchestrator 走启动步骤 |
| `git pull` 不是 ff | 报告：`dev_next 上游被改写，需用户裁决` |
| worktree 已存在 + 锁存在 | 拒绝并返回锁文件内容 |
| 磁盘空间 / 权限错误 | 原样返回 stderr |

## Cleanup（被 merge skill 调用，不是本 skill）

merge 完成后由 [feature-merge-back](../feature-merge-back/SKILL.md) 负责：

```
git worktree remove dong/.worktrees/<feature_id>
rm dong/.orchestration/workers/<feature_id>.lock
# 分支 feat/<id>-<slug> 保留 30 天
```

## Conventions

- 永不 push feature 分支到 origin（除非 cli 显式需要 PR；此时 dispatch skill 单独决策）。
- 永不直接在主仓库 checkout feature 分支；只通过 worktree 隔离。
- worktree 路径必须在 `.gitignore` 排除（项目根 `.gitignore` 加 `dong/.worktrees/`、`dong/.orchestration/`）。
