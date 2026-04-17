---
name: parallel-wave-orchestrator
description: >-
  Run one tick of the multi-feature scheduling loop: read state, advance
  finished/verified/approved/mergeable features, and dispatch new CLI workers
  bounded by Wave / group / concurrency rules from
  doc/orchestration/dependency-matrix.md. Use this skill on every orchestrator
  tick instead of inlining the logic.
---

# Parallel Wave Orchestrator

## When to Use

主 orchestrator agent 每次"继续推进"或定时事件触发，**只调本 skill**。本 skill 把所有 feature 流转串成一次幂等的 tick。

## Inputs

| 字段 | 必填 | 说明 |
|---|---|---|
| `mode` |   | `tick` (默认) / `dry-run` (只算不执行) / `force-recompute` (重建快照) |
| `dispatch_overrides` |   | 强制只 dispatch 指定 feature ids（人工干预） |

## Reference Inputs (always read)

每次 tick 必读：

1. `dong/.orchestration/state-ledger.jsonl` 末尾增量。
2. `dong/.orchestration/snapshots/state.json`。
3. `doc/orchestration/dependency-matrix.md` § 2 / § 3 / § 5。
4. `dong/.orchestration/config.json`（取 `MAX_PARALLEL_WORKERS` 等）。

## Tick Procedure

按以下顺序，**每步完成才进下一步**。任何步骤抛错：写 ledger `note` 后中断 tick 并报告用户。

### Step 1 · Reduce state

- 用 [orchestration-state-ledger](../orchestration-state-ledger/SKILL.md) 把 ledger 重放成 `features` map。
- 写新的 `snapshots/state.json`。

### Step 2 · Health check 已 in_progress workers

- 对每个状态 ∈ {`cli_running`, `verifying`, `merging`} 的 feature：
  - 检查 lock 文件 PID 是否还活着（OS 调用）。
  - cli_running 还跑：`Await` 短窗，读 session log 末尾，若 cli 自报 `STATUS: ready-for-orchestrator-verify` 或退出码已写入 → ledger `cli_finished`。
  - 若 > 2h 无新输出 → ledger `note: stale`，标 `stale=true`（不回收，不 dispatch 新）。

### Step 3 · 推进 awaiting_verify

- 对每个 `awaiting_verify` feature → 调 [feature-verify](../feature-verify/SKILL.md)（在主 agent 内同步跑；verify 本身不并行 — 因为 build 资源串行更快）。
- 写 ledger `verify_finished`。
- 若失败且 retry 未超 → 调 [feature-cli-dispatch](../feature-cli-dispatch/SKILL.md) 重派（带 retry_feedback_path）。
- 若失败且 retry 超限 → 状态 `blocked`，停止处理该 feature。

### Step 4 · 报告 awaiting_review

- 对每个 `awaiting_review` feature → 给用户输出标准摘要（见 [orchestrator-prompt § 3.5](../../doc/orchestration/orchestrator-prompt.md#35-review)）。
- **不自动批准**（除非用户启动时 `auto_approve_soft_pass=true` 且 hard+soft 全过）。
- 等用户 `approve` / `reject <reason>` / `hold` 之一。

### Step 5 · 串行 merge

- 在 `mergeable` 队列里取一个（FIFO，按 ledger `review_approved` 时间）→ 调 [feature-merge-back](../feature-merge-back/SKILL.md)。
- 必须等本次 merge 完全结束（ledger 有 `merge_finished`）才进 Step 6。
- 同 tick 内只 merge 一个，避免 dev_next 历史交错；多余 mergeable 留下一个 tick。

### Step 6 · 计算 eligible 集合

候选 feature：状态 ∈ {`registered`, `prepared`}（不在跑也不在 blocked）。

逐个判定 eligible（全部满足才算）：

1. 所有 `upstream` ∈ `merged`。
2. 不与某个 in_progress feature **同 R 群组**。
3. 同一 Y 群组中 in_progress 数 < `MAX_PER_GROUP_Y`。
4. 当前总 in_progress 数 < `MAX_PARALLEL_WORKERS`。
5. 若有"观察期"约束（`observe_until` 字段），现在已过窗口。
6. 若 `dispatch_overrides` 非空，候选必须在其中。

按 wave 顺序排序候选：先 Wave 0 / 1 / 2 ...，同 wave 内按 spec 估时升序（短任务先发，让长任务后排，便于 wave 整体早完）。

### Step 7 · Dispatch 新 worker

- 在剩余空 slot 内逐个：
  - 调 [feature-worktree-setup](../feature-worktree-setup/SKILL.md)。
  - 调 [feature-cli-dispatch](../feature-cli-dispatch/SKILL.md)（`block_until_ms: 0`，后台跑）。
  - 写 lock 文件、ledger 事件。
- 每 dispatch 一个就重算 in_progress 数 / 群组 cap，避免 race。

### Step 8 · 写快照 + 报告

- 更新 `snapshots/state.json`（含 `dev_next_sha`、`active_workers`、每 feature 状态）。
- 给用户**单条**摘要（不要 spam）：

  ```
  [TICK <iso>] dev_next @ <sha>
  Wave 0: merged 4/7 · in_progress 2 · blocked 1 · queued 0
  Wave 1: queued 5
  This tick: verify P0-1 PASS · merge P0-7 done · dispatch P0-4 + P1-7
  Awaiting your action: review P0-1 (see READY message above)
  ```

## DRY RUN mode

`mode=dry-run`：跑 Step 1 / 2 / 6（计算 eligible），但不调用任何会写状态的 skill；只输出"如果执行会做什么"。用于用户预览。

## Concurrency Invariants（必须保持）

1. 同一 feature 同时只有一个 cli session（lock 文件保证）。
2. 同一 R 群组同时 in_progress ≤ 1。
3. 同一 Y 群组同时 in_progress ≤ `MAX_PER_GROUP_Y`（默认 2）。
4. 同时 merge 数 = 1。
5. ledger 写入串行（同一 tick 内追加多事件 OK，不要在多线程并发写）。

## Failure / Recovery

| 现象 | 处理 |
|---|---|
| 某 cli 工具崩 | health check 发现退出 → verify → fail → retry；超限 blocked |
| ledger 末行截断 | 备份后切除 → ledger `note: ledger_truncated_recovered` → 继续 |
| dev_next 远端被改写（fetch 发现） | 立即停 dispatch，进 panic 模式：所有新 dispatch 暂停，仅允许人工修复 |
| 磁盘满 | 在 dispatch 前先 `df` 检查；< 5 GB 拒绝创建新 worktree |

## Anti-patterns（禁止做）

- ❌ 在本 skill 里自己写 cli 调用命令（应走 dispatch skill）。
- ❌ 在本 skill 里直接 `git merge`（应走 merge skill）。
- ❌ 跳过依赖矩阵硬限制"因为只是个小改动"。
- ❌ 把 verify 的"soft fail"等同 hard fail（导致大量 blocked）。
- ❌ 把多个 merge 塞同一 tick（违反串行不变量）。

## Exit Criteria of a Tick

一个 tick "结束"的判定：

- 没有更多步骤可推进；
- 报告已发送给用户；
- ledger 当前事件已 fsync。

下一次 tick 由用户回复 / 定时器再次触发。
