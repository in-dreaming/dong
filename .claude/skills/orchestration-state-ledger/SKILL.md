---
name: orchestration-state-ledger
description: >-
  Append events to dong/.orchestration/state-ledger.jsonl, replay it into a
  features-state map, write snapshots, and grep helpers. The single source of
  truth shared by every other orchestration skill. Use whenever a skill needs
  to read or write feature lifecycle state.
---

# Orchestration State Ledger

## When to Use

任何 orchestration 流程**只通过本 skill 读写状态**。直接编辑 `state-ledger.jsonl` 或 `snapshots/state.json` 是禁止行为。

## Storage

- 主账本：`dong/.orchestration/state-ledger.jsonl`（append-only JSONL）
- 快照：`dong/.orchestration/snapshots/state.json`（reducer 输出）
- Schema 详见：[`doc/orchestration/state-ledger-schema.md`](../../doc/orchestration/state-ledger-schema.md)

## API（本 skill 提供给其他 skill 的概念性接口）

实现可以是一个小 Python 脚本或主 agent 直接读写文件。下面给约定。

### `append(event)`

`event` 必含字段：

```json
{
  "feature": "<id|_meta>",
  "actor": "orchestrator|cli|human",
  "event": "<see schema>",
  "payload": { ... }
}
```

skill 自动注入：

| 字段 | 来源 |
|---|---|
| `ts` | 当前 ISO-8601 UTC（精度秒） |
| `seq` | 单调递增整数（读末行 +1） |

实现要点：

1. 打开 ledger 用 `O_APPEND`；写入是单行 JSON + `\n`。
2. 写入后 `flush + fsync`。
3. 失败 → 抛错给调用方；不允许"静默丢失事件"。
4. 写入前 sanity 校验：`event` 字段在 schema § 2 枚举内；payload 必含字段齐。

### `appendBatch(events[])`

同 `append`，但同一 tick 内多个事件原子写（给同一 fd 连续 write）。

### `loadAll() → ledgerLines[]`

一次性读全文件，按行 parse。**用于初始化**；正常 tick 用 `loadIncremental(lastSeq)`。

### `loadIncremental(lastSeq)`

只读 `seq > lastSeq` 的行（先 stat 文件大小变化判断是否有新事件，没新事件就跳过）。

### `reduce(events) → state`

reducer 规则严格按 [schema § 3](../../doc/orchestration/state-ledger-schema.md#3-状态机)。输出：

```json
{
  "features": {
    "P0-1": {
      "status": "cli_running",
      "branch": "feat/P0-1-uber-quad",
      "worktree": "dong/.worktrees/P0-1",
      "groups": ["render-core"],
      "upstream": [],
      "spec_path": "doc/phase0/P0-1_uber_quad_pipeline.md",
      "retry_count": 0,
      "last_event_idx": 1281,
      "started_at": "2026-04-15T08:00:00Z",
      "verify_history": [{"ts":"...", "pass":false, "report":"..."}]
    },
    ...
  },
  "active_workers": 3,
  "ledger_lines": 1284
}
```

特别注意：

- `rollback` 事件回退到 `payload.from_event_id` 之前的状态时，reducer 重新跑前缀 + 跳过 rollback 区段（区段 = `from_event_id` 到 rollback 自身的所有 event）。**不要"在状态里减"**，要重放。
- 未知 event type → 跳过 + 写 warning（不抛）。

### `writeSnapshot(state)`

写 `snapshots/state.json`，pretty-printed。每次 tick 末尾调一次。

### `grep(query) → matches[]`

便利函数：底层 `rg` 调用，匹配 ledger 行。常用：

| 用途 | 查询 |
|---|---|
| 某 feature 全生命周期 | `feature":"P0-1"` |
| 当前 in_progress | `event":"cli_dispatched"` 后无对应 `cli_finished` |
| 失败的 verify | `event":"verify_finished","payload":{"pass":false` |

## Schema Migration

- 启动时第一次写文件 → append `schema_init` 事件，含 `payload.version=1`。
- 升级 schema → 写 `schema_migrated`，标 `from_version` / `to_version`。
- reducer 必须能跑老版本 ledger（向后兼容）；新字段 default 处理。

## 写入示例（伪代码）

```python
def append(event_kind, feature, payload, actor="orchestrator"):
    line = {
        "ts": iso_now_utc(),
        "seq": next_seq(),
        "feature": feature,
        "actor": actor,
        "event": event_kind,
        "payload": payload,
    }
    validate(line)  # 抛 ValueError on bad schema
    with open(LEDGER, "a", encoding="utf-8") as f:
        f.write(json.dumps(line, ensure_ascii=False, separators=(",", ":")) + "\n")
        f.flush()
        os.fsync(f.fileno())
    return line["seq"]
```

## Recovery / Repair

| 现象 | 处理 |
|---|---|
| 文件最后一行不完整（断电） | 备份 → 切除最后一行 → append `note: ledger_truncated_recovered`（payload 含被切除的 raw bytes hex） |
| `seq` 不单调（外部误编辑） | 抛错；不允许 reducer 继续 |
| 整个文件丢失 | 从 `snapshots/state.json` 重建为"伪事件流"（最低限度，仅恢复 status），并写 `note: ledger_rebuilt_from_snapshot` |

## Conventions

- 编码 UTF-8 / LF，不要 BOM。
- 每行 ≤ 64 KB（payload 大就放外部文件，ledger 只存路径）。
- 时间戳一律 UTC，避免 Windows / Linux 时区差。
- ledger 与 snapshot 都进 `.gitignore`，**不进 git**。
- 路径全部相对仓库根，避免 worktree / 主仓库混淆。

## Anti-patterns（禁止）

- ❌ 在 reducer 里"修补"状态（应通过 append 新事件实现）。
- ❌ 用 ledger 当数据库做 query（仅顺序读 / grep）。
- ❌ 在 cli prompt 里让 cli 直接写 ledger（cli 只写自己 worktree；状态由 orchestrator 写）。
