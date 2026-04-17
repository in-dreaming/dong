---
name: feature-merge-back
description: >-
  Merge an approved feature branch into `dev_next` (squash by default), update
  the ledger, remove the worktree, and keep the branch around for 30 days.
  Use only after a `review_approved` event. Always serial — orchestrator must
  not call this in parallel.
---

# Feature Merge-Back

## When to Use

仅当 feature 状态 = `mergeable`（即 `verify_finished{pass:true}` + `review_approved`）时调用。同一时刻只能跑一个本 skill 实例。

## Inputs

| 字段 | 必填 | 说明 |
|---|---|---|
| `feature_id` | ✓ | |
| `branch` | ✓ | `feat/<id>-<slug>` |
| `worktree_path` | ✓ | |
| `strategy` |   | `squash` (默认) / `merge` (no-ff) / `ff` |
| `commit_message` |   | 缺省按下方模板生成 |

## Procedure

1. **前置闸门**：
   - 同一 ledger 检查没有别的 `merge_started` 在跑（找不到对应 `merge_finished` / `rollback`）。有则**直接拒绝**返回 `wait_in_queue`。
   - 主仓库 `git status` 必须 clean。
   - 锁文件 `workers/<feature_id>.lock` 必须存在，确认 review 流程有效。

2. **同步 dev_next**：
   ```
   cd dong
   git switch dev_next
   git pull --ff-only origin dev_next
   ```
   失败（非 ff） → ledger `note: dev_next_diverged`，状态 → `blocked`，提示用户。

3. **rebase feature 分支到最新 dev_next**（在 worktree 内做，避免污染主仓库）：
   ```
   git -C <worktree_path> fetch ../..  dev_next:dev_next
   git -C <worktree_path> rebase dev_next
   ```
   - 有冲突 → 中止 rebase（`git rebase --abort`），ledger `note: rebase_conflict`，状态 → `blocked`，提示用户。
   - rebase 后跑一次 [feature-verify](../feature-verify/SKILL.md) 的 `mode=hard-only`（防止 rebase 引入回归）。
     - 失败 → ledger 写新一条 `verify_finished`，状态 → `cli_running` 重试。
     - 通过 → 继续。

4. **生成 commit message**（squash 模式默认模板）：

   ```
   [<feature_id>] <名称>

   Spec: <spec_path>
   Worktree: dong/.worktrees/<feature_id> (squashed)
   Verify: hard PASS / soft <ok>/<total>
   Reports: dong/.orchestration/reports/<feature_id>/

   <feature_id>: <一句话描述，从 spec § 1 摘录>
   ```

5. **执行 merge**：

   - `squash`（默认）：
     ```
     cd dong
     git switch dev_next
     git merge --squash feat/<id>-<slug>
     git commit -F .orchestration/reports/<id>/commit_msg.txt
     ```
   - `merge`（no-ff，保留 feature 历史）：
     ```
     git merge --no-ff feat/<id>-<slug> -F .../commit_msg.txt
     ```
   - `ff`：仅当 dev_next 是 feature 直系祖先时；不推荐。

6. **保存 merge diff**（备查）：
   ```
   git diff <merge_sha>~ <merge_sha> > .orchestration/reports/<id>/merge_diff.txt
   ```

7. **push（可选）**：默认**不 push**；让 orchestrator 主循环结束 / 一个 wave 完成后统一 push（减少远端事件）。
   若用户启动时给 `auto_push=true` → `git push origin dev_next`。

8. **清理 worktree**：
   ```
   git worktree remove dong/.worktrees/<feature_id>
   rm dong/.orchestration/workers/<feature_id>.lock
   ```
   feature 分支 **保留**（30 天后由独立 cleanup 脚本处理）。

9. **ledger**：
   ```jsonl
   {"event":"merge_started","feature":"<id>","payload":{"strategy":"squash","target_ref":"dev_next"}}
   {"event":"merge_finished","feature":"<id>","payload":{"merge_sha":"<sha>","target_sha_after":"<sha>"}}
   {"event":"feature_done","feature":"<id>","payload":{"final_status":"merged"}}
   {"event":"worker_released","feature":"<id>","payload":{}}
   ```

10. **触发后续**：本 skill 不调度新 worker；返回控制权给 orchestrator 主循环，由它在下一个 tick 重新算 eligible。

## Failure Modes

| 现象 | 动作 |
|---|---|
| dev_next 与 origin 分叉 | blocked，让用户裁决 |
| rebase 冲突 | abort + blocked，给用户冲突文件清单 |
| rebase 后 verify 失败 | 自动回 cli 重做（计入 retry）；超限 → blocked |
| commit hook 失败 | 不 amend；ledger note，blocked |
| worktree remove 失败（文件占用） | 重试一次；仍失败 → 仅 ledger note，**不影响 merge 完成判定** |

## Safety Rails

- 永不 `git push --force`。
- 永不动 `main`。
- 永不批量 merge：循环里 `for feat in mergeable: merge(feat)` 必须串行 await。
- merge commit message 由本 skill 写出文件，再 `git commit -F`，避免编码问题。
- merge 前必跑 hard verify（即使刚跑过 verify，rebase 后再跑一次）。

## Out of Scope

- 不创建 GitHub PR（如需 PR 流程，由独立 skill 包一层；本 skill 只做本地 merge）。
- 不删除远端分支。
- 不发通知（由 orchestrator 主 prompt 报告给用户）。
