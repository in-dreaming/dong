---
name: feature-verify
description: >-
  Parse the spec § 5 verification rules of a feature, run them inside its
  worktree, and emit a machine-readable report plus ledger event. Use after
  feature-cli-dispatch reports `cli_finished`. Hard-fails count as fail; only
  Hard pass means a feature can advance to review.
---

# Feature Verify

## When to Use

orchestrator 在 feature 状态 = `awaiting_verify` 时调用。也可手工触发"重跑验收"。

## Inputs

| 字段 | 必填 | 说明 |
|---|---|---|
| `feature_id` | ✓ | |
| `worktree_path` | ✓ | |
| `spec_path` | ✓ | |
| `mode` |   | `full` (默认) / `hard-only` / `single:<rule_id>` |

## Spec § 5 约定（feature 作者必须遵守）

每份 spec 的"§ 5 验证规则"必须包含一个 fenced YAML 块，便于本 skill 机器解析：

````markdown
## 5. 验证规则

```yaml
verify:
  hard:
    - id: render_all_tests_pixel_diff
      cmd: "zig build render-all-tests"
      cwd: "dong"
      then:
        - cmd: "python scripts/tools/compare_screenshots.py --baseline doc/baseline/uber_quad --current zig-out/test-renders --threshold 0.005"
      timeout_sec: 600
    - id: perf_baseline_game_hud
      cmd: "python scripts/tools/perf_baseline.py --scene game_hud --json out.json"
      cwd: "dong"
      assert:
        kind: json
        file: "dong/out.json"
        rules:
          - "fps.p50 >= 120"
          - "draw_calls.max <= 12"
  soft:
    - id: docs_updated
      cmd: "python scripts/tools/check_doc_updated.py P0-1"
      cwd: "."
```
````

如果 spec 里没有这个 yaml 块（早期 spec 都是自然语言），`orch.py verify` 会做尽力而为的解析（grep `\bcmd:`、提取相邻代码块），并在报告里加 `parse_quality: heuristic` 警告。**解析后若仍没有任何 `hard` 规则，验收一律失败**（合成规则 `_orch_no_hard_verify_rules`），避免 noop 流水线误标为已验收。

## Procedure

1. **找 spec yaml 块**：从 `spec_path` Read，提取 § 5 的 ` ```yaml verify: ... ``` ` 块。
2. **校验 worktree clean**：
   ```
   git -C <worktree> status --porcelain
   ```
   非空 → ledger `note: dirty_worktree_at_verify`，状态 fail。
3. **依次跑 hard 规则**（若 `mode=hard-only` 跳过 soft；若 `mode=single` 只跑指定）：
   每条规则：
   - 在 `cwd` 下运行 `cmd`，redirect 到 `reports/<id>/verify_<ts>/<rule_id>.log`。
   - 若有 `then[]`，前面成功才跑后续。
   - 若有 `assert.kind: json`，读取产物 json，按 jq-like 表达式校验：
     - `fps.p50 >= 120` → `data["fps"]["p50"] >= 120`
     - 简单运算符：`>= <= == != > <`，左侧支持 `.` 路径，右侧字面量数值。
   - 超时按 `timeout_sec` 杀进程，记 `timed_out: true`。
4. **跑 soft 规则**（同上，失败不阻塞）。
5. **写报告**（路径 `reports/<id>/verify_<ts>.json` 与 `.md`，结构见 [state-ledger-schema § 6](../../docs/developer/orchestration/state-ledger-schema.md#6-报告产物-reportsfeature)）。
6. **判定**：
   - `hard_pass = 全部 hard 规则 passed=true`（**且**至少存在一条 hard 规则；否则 `hard_pass=false`）。
   - `soft_pass = 全部 soft 规则 passed=true`。
   - `overall = "pass" if hard_pass else "fail"`。
7. **ledger**：
   ```jsonl
   {"event":"verify_finished","feature":"<id>","payload":{"pass":<bool>,"hard_failed":[...],"soft_failed":[...],"report_path":"..."}}
   ```

## Auto-Retry 触发（由 orchestrator 决策）

- `overall=fail` 且 retry 次数 < `RETRY_VERIFY` → orchestrator 调 [feature-cli-dispatch](../feature-cli-dispatch/SKILL.md) 把 `retry_feedback_path = reports/<id>/verify_<ts>.md` 塞回去。
- 重试时 cli 应在 prompt 里看到具体哪些规则挂了。

## Edge Cases

| 现象 | 处理 |
|---|---|
| spec 没有 yaml 块 | 启发式解析；报告中 `parse_quality=heuristic`；建议作者补 yaml |
| 验证命令需要 build 但 build 慢 | 在第一条 hard 规则前自动 `zig build` 预热（10 min 上限） |
| 命令依赖 GPU 而 CI 无 GPU | yaml 里加 `requires_gpu: true`；本机器无 GPU → 标 `skipped`，不计 fail |
| 命令打日志巨大 | 截断每条 log 到 200 KB，全部存 raw 到 `<rule_id>.log.raw` |

## Conventions

- 永不修改 worktree 文件（验收只读）。允许产生构建产物在 `dong/zig-out/` 等忽略目录。
- 永不动 `dev_next`。
- 验证全部用绝对路径 / `git -C` 显式指定 worktree，避免误进主仓库。
- 报告 md 顶部必须有"哪些规则挂了 / 看哪条 log"的人类摘要，方便 review 时快速定位。
