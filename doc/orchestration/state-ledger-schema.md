# 状态账本 (state-ledger.jsonl) Schema

> orchestrator 与所有 skill 的**唯一真实来源**。文件路径默认：
>
> ```
> dong/.orchestration/state-ledger.jsonl
> ```
>
> 用 append-only JSONL，每行一个事件。当前状态由 reducer 重放得到。
>
> 设计原则：**append-only / 可重放 / 人类可读 / 易 grep**。所有 skill 只能 append，永远不许就地改写历史行。

---

## 1. 文件布局

```
dong/.orchestration/
├── state-ledger.jsonl          # 主账本（事件流）
├── workers/
│   ├── P0-1.lock               # 任一 in_progress feature 持锁文件（pid + start ts）
│   └── ...
├── reports/
│   ├── P0-1/
│   │   ├── verify_2026-04-18T03-12-00Z.json
│   │   └── cli_session.log
│   └── ...
└── snapshots/
    └── state.json              # reducer 输出的当前状态快照（可随时重建）
```

---

## 2. 事件类型

每行 JSON 必须含的公共字段：

```json
{
  "ts": "2026-04-17T11:42:01Z",
  "feature": "P0-1",
  "actor": "orchestrator | cli | human",
  "event": "<see below>",
  "payload": { ... }
}
```

| event | 含义 | payload 必需字段 |
|---|---|---|
| `feature_registered` | 首次入队 | `spec_path`, `groups[]`, `upstream[]` |
| `prepare_started` | 开始准备 worktree / 分支 | `branch`, `worktree_path`, `base_ref` |
| `prepare_finished` | worktree 创建好 | `commit_sha` |
| `cli_dispatched` | 已把任务交给 cli | `cli_tool`, `cli_args`, `task_id`, `session_log` |
| `cli_progress` | cli 心跳（可选） | `note`, `last_log_line` |
| `cli_finished` | cli 自报完成 | `exit_code`, `duration_sec`, `files_changed` |
| `verify_started` | 开始跑 § 5 验收 | `rules_file`, `runner` |
| `verify_finished` | 验收结果 | `pass: bool`, `hard_failed[]`, `soft_failed[]`, `report_path` |
| `review_requested` | 等人审 | `pr_url?`, `notes` |
| `review_approved` | 人/orchestrator 通过 | `approver`, `notes` |
| `review_rejected` | 驳回 | `approver`, `reasons[]` |
| `merge_started` | 开始合并到 dev_next | `strategy: ff|squash`, `target_ref: dev_next` |
| `merge_finished` | 合并成功 | `merge_sha`, `target_sha_after` |
| `rollback` | 撤销 | `from_event_id`, `reason`, `actions[]` |
| `feature_done` | 终态 | `final_status: merged|abandoned` |
| `worker_released` | 释放 worker 槽 | `slot_id?` |
| `note` | 自由备注 | `text` |

---

## 3. 状态机

```
registered ──► preparing ──► cli_running ──► verifying
                                               │
                                               ├─ pass ─► review ─┐
                                               │                   │
                                               └─ fail ─► cli_running (retry)
                                                                   │
                                                              approved
                                                                   │
                                                                   ▼
                                                                merging ──► merged ──► done
                                                                   │
                                                              rejected ──► cli_running
                                                                   │
                                                              abandoned ──► done
```

reducer 规则：

1. `feature_registered` → `registered`
2. `prepare_*` → `preparing` → `prepared`
3. `cli_dispatched` → `cli_running`
4. `cli_finished` → `awaiting_verify`
5. `verify_finished{pass:true}` → `awaiting_review`
6. `verify_finished{pass:false}` → `cli_running`（自动 dispatch 重试，最多 `RETRY_VERIFY` 次；超限 → `blocked`）
7. `review_approved` → `mergeable`
8. `review_rejected` → `cli_running`（带反馈再做）
9. `merge_finished` → `merged`
10. `feature_done{final_status:merged}` → `done`
11. `rollback` 任何阶段 → 回退到指定事件之前的状态

---

## 4. 当前状态快照（snapshots/state.json）

由 orchestrator 每次 tick 后写入。结构：

```json
{
  "generated_at": "2026-04-17T11:42:30Z",
  "ledger_lines": 1284,
  "dev_next_sha": "ab12cd34",
  "active_workers": 3,
  "max_workers": 4,
  "features": {
    "P0-1": {
      "status": "merging",
      "branch": "feat/P0-1-uber-quad",
      "worktree": "../dong.worktrees/P0-1",
      "groups": ["render-core"],
      "upstream": [],
      "downstream_blocked": ["P0-2", "P0-3", "P0-6"],
      "verify_pass": true,
      "last_event_idx": 1281,
      "started_at": "2026-04-15T08:00:00Z",
      "duration_min": 4422
    },
    "P0-7": { "status": "merged", ... },
    "P0-4": { "status": "cli_running", ... }
  }
}
```

---

## 5. 锁文件 (workers/<feature>.lock)

```json
{
  "feature": "P0-1",
  "pid": 18244,
  "host": "orchestrator-A",
  "started_at": "2026-04-15T08:00:00Z",
  "cli_tool": "cursor-cli",
  "cli_session": ".orchestration/reports/P0-1/cli_session.log"
}
```

约定：

- 持锁期间禁止任何其他 worker 修改本 feature 的 worktree。
- 锁文件存在但 `pid` 已死 → orchestrator 视为 `stale`，可强制释放并 `rollback` 到 `prepare_finished`。

---

## 6. 报告产物 (reports/<feature>/)

| 文件 | 写入者 | 内容 |
|---|---|---|
| `cli_session.log` | cli skill | cli 工具的完整 stdout/stderr |
| `verify_<ts>.json` | verify skill | 见下文 |
| `verify_<ts>.md` | verify skill | 给人看的摘要 |
| `merge_diff.txt` | merge skill | 合并前 diff（备查） |

`verify_*.json` 结构：

```json
{
  "feature": "P0-1",
  "ran_at": "2026-04-17T03:12:00Z",
  "spec_path": "doc/phase0/P0-1_uber_quad_pipeline.md",
  "rules": [
    {
      "id": "perf_baseline_game_hud",
      "kind": "hard",
      "command": "python scripts/tools/perf_baseline.py --scene game_hud",
      "exit_code": 0,
      "duration_sec": 38.4,
      "metrics": { "fps_p50": 142.0, "draw_calls": 9 },
      "thresholds": { "fps_p50_min": 120, "draw_calls_max": 12 },
      "passed": true
    },
    {
      "id": "render_all_tests_pixel_diff",
      "kind": "hard",
      "command": "zig build render-all-tests && python scripts/tools/compare_screenshots.py ...",
      "exit_code": 1,
      "passed": false,
      "diff_summary": "12 cases > 0.5% pixel diff"
    }
  ],
  "hard_pass": false,
  "soft_pass": true,
  "overall": "fail"
}
```

---

## 7. grep 速查范例

```bash
# 当前所有 in_progress feature
jq -c 'select(.event=="cli_dispatched" or .event=="cli_finished")' state-ledger.jsonl

# P0-1 的全生命周期
grep '"feature":"P0-1"' state-ledger.jsonl

# 所有 verify 失败
jq -c 'select(.event=="verify_finished" and .payload.pass==false)' state-ledger.jsonl
```

---

## 8. 兼容/演进

- 新增 event type 必须**追加**，不删旧字段，旧 reducer 跳过未知 event。
- schema 版本号写在第一行（启动时）：

```json
{"ts":"2026-04-17T00:00:00Z","feature":"_meta","actor":"orchestrator","event":"schema_init","payload":{"version":1}}
```

- 重大改动 → bump version + 写迁移说明。
