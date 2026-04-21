#!/usr/bin/env python3
"""
Dong Multi-Feature Orchestrator CLI (`orch`)

Concrete implementation of the skills in .claude/skills/orchestration-*/, 
feature-worktree-setup/, feature-cli-dispatch/, feature-verify/, feature-merge-back/,
and parallel-wave-orchestrator/. See doc/orchestration/README.md.

Usage:
    python dong/scripts/orch.py <subcommand> [args]

Common subcommands:
    init [--create-dev-next]   First-time setup; register 27 features
    status                     Short summary
    list [--phase p0|p1|p2]    Full feature table
    eligible                   What could dispatch right now
    tick [--dry-run] [--max N] Advance one orchestration tick
    dispatch <id>              Manually dispatch one feature
    verify   <id>              Run the spec section-5 verify for a feature
    approve  <id>              Mark review_approved
    reject   <id> <reason>     Mark review_rejected
    merge    <id>              Merge feature branch into dev_next
    mark-merged <id>           Ledger-only: already merged outside orch (see README)
    ledger-reset [--remove-worktrees]  Backup ledger; all features back to registered
    rollback <id> --from-seq N [--reason]  Undo ledger range (failed merge recovery)
    abort    <id> [reason]     Abort / abandon feature
    ledger-dump [N]            Print last N ledger lines (default 40)
    snapshot                   Re-reduce ledger and rewrite snapshots/state.json
    config-show                Print effective config

Layout:
    <repo-root>/
      dong/.orchestration/   (gitignored)
        state-ledger.jsonl
        snapshots/state.json
        workers/<id>.lock
        reports/<id>/...
        config.json
      dong/.worktrees/<id>/  (gitignored)
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import shutil
import signal
import subprocess
import sys
import textwrap
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, NoReturn, Optional, Tuple

# ----------------------------------------------------------------------------
# Constants / registry of features
# ----------------------------------------------------------------------------

SCHEMA_VERSION = 1

# Each feature: (id, slug, spec_path, groups, upstream, wave, est_weeks)
FEATURES: List[Dict[str, Any]] = [
    # Phase 0
    dict(id="P0-1", slug="uber-quad",       spec="doc/phase0/P0-1_uber_quad_pipeline.md",       groups=["render-core"],                upstream=[],                          wave=0, weeks=3),
    dict(id="P0-2", slug="nineslice-panel", spec="doc/phase0/P0-2_nineslice_panel.md",           groups=["render-core"],                upstream=["P0-1"],                    wave=1, weeks=1),
    dict(id="P0-3", slug="mask-conic",      spec="doc/phase0/P0-3_mask_and_conic_gradient.md",   groups=["render-core"],                upstream=["P0-1"],                    wave=1, weeks=2),
    dict(id="P0-4", slug="gamepad-nav",     spec="doc/phase0/P0-4_gamepad_spatial_navigation.md",groups=["input"],                      upstream=[],                          wave=0, weeks=2),
    dict(id="P0-5", slug="ime",             spec="doc/phase0/P0-5_ime_composition.md",           groups=["input"],                      upstream=[],                          wave=0, weeks=1.5),
    dict(id="P0-6", slug="damage-rect",     spec="doc/phase0/P0-6_partial_damage_rect.md",       groups=["render-core", "layout-core"], upstream=["P0-1"],                    wave=2, weeks=3),
    dict(id="P0-7", slug="perf-baseline",   spec="doc/phase0/P0-7_perf_baseline.md",             groups=["tools"],                      upstream=[],                          wave=0, weeks=1.5),
    dict(id="P0-8", slug="gap-p0",          spec="doc/phase0/P0-8_gap_analysis_p0_cleanup.md",   groups=["dom-scatter"],                upstream=[],                          wave=0, weeks=2),
    # Phase 1
    dict(id="P1-1", slug="drawlist-abi",    spec="doc/phase1/P1-1_drawlist_c_abi.md",            groups=["render-core", "abi"],         upstream=["P0-1","P0-2","P0-3","P0-6"],wave=2, weeks=4),
    dict(id="P1-2", slug="host-view",       spec="doc/phase1/P1-2_host_view_embed.md",           groups=["abi", "appcore"],             upstream=["P1-1"],                    wave=3, weeks=3),
    dict(id="P1-3", slug="devtools",        spec="doc/phase1/P1-3_devtools_v1.md",               groups=["appcore"],                    upstream=["P0-6","P0-7"],             wave=3, weeks=4),
    dict(id="P1-4", slug="live-reload",     spec="doc/phase1/P1-4_live_reload.md",               groups=["appcore"],                    upstream=[],                          wave=0, weeks=2),
    dict(id="P1-5", slug="css-grid",        spec="doc/phase1/P1-5_css_grid_subset.md",           groups=["layout"],                     upstream=[],                          wave=1, weeks=4),
    dict(id="P1-6", slug="hdr",             spec="doc/phase1/P1-6_hdr_output.md",                groups=["render-core"],                upstream=[],                          wave=1, weeks=2),
    dict(id="P1-7", slug="js-bench",        spec="doc/phase1/P1-7_js_engine_benchmark.md",       groups=["tools-readonly"],             upstream=[],                          wave=0, weeks=2),
    dict(id="P1-8", slug="async-layout",    spec="doc/phase1/P1-8_async_layout_shaping.md",      groups=["layout", "script"],           upstream=["P0-6"],                    wave=2, weeks=4),
    dict(id="P1-9", slug="gap-p1",          spec="doc/phase1/P1-9_gap_analysis_p1_cleanup.md",   groups=["dom-scatter"],                upstream=[],                          wave=1, weeks=4),
    # Phase 2
    dict(id="P2-1", slug="world-text",      spec="doc/phase2/P2-1_world_text.md",                groups=["render-3d"],                  upstream=["P0-1","P1-1"],             wave=4, weeks=3),
    dict(id="P2-2", slug="decal",           spec="doc/phase2/P2-2_decal.md",                     groups=["render-3d"],                  upstream=[],                          wave=4, weeks=3),
    dict(id="P2-3", slug="world-overlay",   spec="doc/phase2/P2-3_world_overlay.md",             groups=["render-3d", "appcore"],       upstream=["P1-2"],                    wave=5, weeks=3),
    dict(id="P2-4", slug="lottie-rive",     spec="doc/phase2/P2-4_lottie_rive.md",               groups=["render", "vector"],           upstream=[],                          wave=4, weeks=4),
    dict(id="P2-5", slug="dong-ui",         spec="doc/phase2/P2-5_dong_ui_library.md",           groups=["js-tooling"],                 upstream=["P0-3","P0-4","P2-1"],      wave=5, weeks=8),
    dict(id="P2-6", slug="ts-npm",          spec="doc/phase2/P2-6_typescript_npm.md",            groups=["js-tooling"],                 upstream=["P1-1"],                    wave=3, weeks=3),
    dict(id="P2-7", slug="visual-editor",   spec="doc/phase2/P2-7_visual_editor.md",             groups=["appcore", "js-tooling"],      upstream=["P1-3","P1-4","P2-5"],      wave=6, weeks=6),
    dict(id="P2-8", slug="dpkg",            spec="doc/phase2/P2-8_dpkg_resource_pack.md",        groups=["tooling"],                    upstream=[],                          wave=4, weeks=4),
    dict(id="P2-9", slug="async-default",   spec="doc/phase2/P2-9_async_default_on.md",          groups=["layout", "script"],           upstream=["P1-8"],                    wave=5, weeks=1),  # has observe window
    dict(id="P2-10",slug="gap-p2",          spec="doc/phase2/P2-10_gap_analysis_p2_cleanup.md",  groups=["dom-scatter"],                upstream=[],                          wave=4, weeks=6),
]

# R (hard-conflict) vs Y (rebase-conflict) vs G (free) — source of truth: doc/orchestration/dependency-matrix.md
GROUP_CONFLICT: Dict[str, str] = {
    "render-core":  "R",
    "layout-core":  "R",
    "layout":       "Y",
    "script":       "Y",
    "appcore":      "Y",
    "dom-scatter":  "Y",
    "abi":          "Y",
    "render-3d":    "Y",
    "input":        "G",
    "render":       "Y",
    "vector":       "G",
    "tools":        "G",
    "tools-readonly":"G",
    "tooling":      "G",
    "js-tooling":   "G",
}

# Observation windows (days) before eligible — keyed by feature id
OBSERVE_WINDOW_DAYS = {
    "P2-9": 30,
}

DEFAULT_CONFIG = {
    "cli_tool": "noop-dryrun",
    "max_parallel_workers": 4,
    "max_per_group_r": 1,
    "max_per_group_y": 2,
    "merge_serial": True,
    "retry_verify": 2,
    "auto_push": False,
    "auto_approve_soft_pass": False,
    # Per-feature observe window (days after upstream merged_at). 0 or null = off.
    "observe_window_days": {},
    "tools": {
        "noop-dryrun": {
            "command": "python",
            "args": ["-c", "import sys,time; print('[NOOP] dispatched', sys.argv[1:]); time.sleep(1); print('STATUS: ready-for-orchestrator-verify')"],
            "passthrough_placeholders": True,
        },
        "cursor-cli": {
            "command": "cursor-agent",
            "args": ["--workspace", "{worktree}", "--prompt-file", "{prompt_file}", "--non-interactive"],
        },
        "codex": {
            "command": "codex",
            "args": ["exec", "--cd", "{worktree}", "--prompt-file", "{prompt_file}"],
        },
        "claude-internal": {
            "command": "claude-internal",
            "args": [
                "--bare",
                "-p",
                "Your assigned task is in `.task/prompt.md` in the current working directory. "
                "Use Read to open it, then implement it fully. When done: append a line "
                "`STATUS: ready-for-orchestrator-verify` to `.task/notes.md` and ensure "
                "`git status` is clean (all changes committed).",
                "--permission-mode",
                "acceptEdits",
                "--max-turns",
                "50",
            ],
        },
    },
}

# ----------------------------------------------------------------------------
# Paths
# ----------------------------------------------------------------------------

def repo_root() -> Path:
    p = Path(__file__).resolve().parent.parent.parent  # dong/scripts/orch.py -> repo root
    return p

def dong_dir() -> Path:
    return repo_root() / "dong"

def orch_dir() -> Path:
    return dong_dir() / ".orchestration"

def ledger_path() -> Path:
    return orch_dir() / "state-ledger.jsonl"

def snapshot_path() -> Path:
    return orch_dir() / "snapshots" / "state.json"

def workers_dir() -> Path:
    return orch_dir() / "workers"

def reports_dir() -> Path:
    return orch_dir() / "reports"

def worktrees_dir() -> Path:
    return dong_dir() / ".worktrees"

def config_path() -> Path:
    return orch_dir() / "config.json"

# ----------------------------------------------------------------------------
# IO helpers
# ----------------------------------------------------------------------------

def utcnow_iso() -> str:
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

def ensure_dirs() -> None:
    for p in (orch_dir(), snapshot_path().parent, workers_dir(), reports_dir(), worktrees_dir()):
        p.mkdir(parents=True, exist_ok=True)

def load_config() -> Dict[str, Any]:
    if not config_path().exists():
        return DEFAULT_CONFIG.copy()
    try:
        with config_path().open("r", encoding="utf-8") as f:
            cfg = json.load(f)
    except Exception as e:
        die(f"Failed to read config.json: {e}")
    merged: Dict[str, Any] = {**DEFAULT_CONFIG, **cfg}
    default_tools: Dict[str, Any] = DEFAULT_CONFIG["tools"]  # type: ignore[assignment]
    merged["tools"] = {**default_tools, **cfg.get("tools", {})}
    def_obs: Dict[str, Any] = DEFAULT_CONFIG.get("observe_window_days") or {}
    merged["observe_window_days"] = {**def_obs, **(cfg.get("observe_window_days") or {})}
    return merged

def write_default_config_if_missing() -> None:
    if not config_path().exists():
        with config_path().open("w", encoding="utf-8") as f:
            json.dump(DEFAULT_CONFIG, f, ensure_ascii=False, indent=2)

def die(msg: str, code: int = 2) -> "NoReturn":
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(code)

def info(msg: str) -> None:
    print(msg)

# ----------------------------------------------------------------------------
# Ledger (append-only JSONL)
# ----------------------------------------------------------------------------

def ledger_read_all() -> List[Dict[str, Any]]:
    if not ledger_path().exists():
        return []
    events = []
    with ledger_path().open("r", encoding="utf-8") as f:
        for ln in f:
            ln = ln.strip()
            if not ln:
                continue
            try:
                events.append(json.loads(ln))
            except json.JSONDecodeError:
                # truncated last line possibly; skip with warning
                print(f"WARN: skipping unparseable ledger line: {ln[:80]}...", file=sys.stderr)
    return events

def next_seq() -> int:
    events = ledger_read_all()
    if not events:
        return 1
    seqs = [e.get("seq", 0) for e in events]
    return max(seqs) + 1

def ledger_append(event: str, feature: str, payload: Optional[Dict[str, Any]] = None, actor: str = "orchestrator") -> Dict[str, Any]:
    ensure_dirs()
    line = {
        "ts": utcnow_iso(),
        "seq": next_seq(),
        "feature": feature,
        "actor": actor,
        "event": event,
        "payload": payload or {},
    }
    with ledger_path().open("a", encoding="utf-8", newline="\n") as f:
        f.write(json.dumps(line, ensure_ascii=False, separators=(",", ":")) + "\n")
        f.flush()
        try:
            os.fsync(f.fileno())
        except OSError:
            pass
    return line

def ledger_append_batch(events: List[Tuple[str, str, Dict[str, Any]]]) -> List[Dict[str, Any]]:
    out = []
    for ev, feat, payload in events:
        out.append(ledger_append(ev, feat, payload))
    return out

# ----------------------------------------------------------------------------
# Reducer
# ----------------------------------------------------------------------------

TERMINAL_STATES = {"merged", "abandoned", "done"}

def reduce_state(events: List[Dict[str, Any]]) -> Dict[str, Any]:
    """Replay the ledger to compute per-feature state. Rollback-aware: if a
    rollback event references `from_event_id`, we drop events in [from_event_id, rollback.seq]."""
    # Handle rollback ranges first
    skip = set()
    for ev in events:
        if ev.get("event") == "rollback":
            frm = ev.get("payload", {}).get("from_event_id")
            seq = ev.get("seq")
            if frm is not None and seq is not None:
                for s in range(int(frm), int(seq) + 1):
                    skip.add(s)

    features: Dict[str, Dict[str, Any]] = {}
    schema_version = SCHEMA_VERSION

    for ev in events:
        if ev.get("seq") in skip:
            continue
        kind = ev.get("event")
        feat = ev.get("feature")
        payload = ev.get("payload") or {}
        if not feat:
            continue
        if feat == "_meta":
            if kind == "schema_init":
                schema_version = payload.get("version", SCHEMA_VERSION)
            continue

        f = features.setdefault(feat, {
            "id": feat,
            "status": "unknown",
            "branch": None,
            "worktree": None,
            "spec_path": None,
            "groups": [],
            "upstream": [],
            "retry_count": 0,
            "verify_history": [],
            "started_at": None,
            "merged_at": None,
            "last_event_idx": None,
            "merge_sha": None,
            "observed_since": None,
        })
        f["last_event_idx"] = ev.get("seq")

        if kind == "feature_registered":
            f["spec_path"] = payload.get("spec_path")
            f["groups"] = payload.get("groups", [])
            f["upstream"] = payload.get("upstream", [])
            f["status"] = "registered"
        elif kind == "prepare_started":
            f["branch"] = payload.get("branch")
            f["worktree"] = payload.get("worktree_path")
            f["status"] = "preparing"
        elif kind == "prepare_finished":
            f["status"] = "prepared"
            f["base_sha"] = payload.get("commit_sha")
        elif kind == "cli_dispatched":
            f["status"] = "cli_running"
            f["cli_tool"] = payload.get("cli_tool")
            f["task_id"] = payload.get("task_id")
            f["session_log"] = payload.get("session_log")
            if not f["started_at"]:
                f["started_at"] = ev.get("ts")
        elif kind == "cli_finished":
            f["status"] = "awaiting_verify"
            f["cli_exit_code"] = payload.get("exit_code")
        elif kind == "verify_started":
            f["status"] = "verifying"
        elif kind == "verify_finished":
            f["verify_history"].append({
                "ts": ev.get("ts"),
                "pass": payload.get("pass"),
                "hard_failed": payload.get("hard_failed", []),
                "soft_failed": payload.get("soft_failed", []),
                "report_path": payload.get("report_path"),
            })
            if payload.get("pass"):
                f["status"] = "awaiting_review"
            else:
                f["retry_count"] = f.get("retry_count", 0) + 1
                f["status"] = "blocked" if f["retry_count"] >= 99 else "awaiting_retry"
        elif kind == "review_requested":
            f["status"] = "awaiting_review"
        elif kind == "review_approved":
            f["status"] = "mergeable"
        elif kind == "review_rejected":
            f["status"] = "awaiting_retry"
            f["retry_count"] = f.get("retry_count", 0) + 1
        elif kind == "merge_started":
            f["status"] = "merging"
        elif kind == "merge_finished":
            f["status"] = "merged"
            f["merged_at"] = ev.get("ts")
            f["merge_sha"] = payload.get("merge_sha")
        elif kind == "feature_done":
            f["status"] = payload.get("final_status", "done")
            if f["status"] == "merged" and not f["merged_at"]:
                f["merged_at"] = ev.get("ts")
        elif kind == "worker_released":
            pass
        elif kind == "note":
            pass
        elif kind == "rollback":
            pass
        else:
            # unknown event kind; warn but continue
            print(f"WARN: unknown event kind {kind} in ledger seq={ev.get('seq')}", file=sys.stderr)

    # Derived: active_workers
    active = sum(1 for v in features.values() if v["status"] in {"preparing", "prepared", "cli_running", "awaiting_verify", "verifying", "awaiting_review", "awaiting_retry", "mergeable", "merging"})
    return {
        "schema_version": schema_version,
        "features": features,
        "active_workers": active,
        "ledger_lines": len(events),
        "generated_at": utcnow_iso(),
    }

def write_snapshot(state: Dict[str, Any]) -> None:
    snapshot_path().parent.mkdir(parents=True, exist_ok=True)
    with snapshot_path().open("w", encoding="utf-8") as f:
        json.dump(state, f, ensure_ascii=False, indent=2)

# ----------------------------------------------------------------------------
# Git helpers
# ----------------------------------------------------------------------------

def run(cmd: List[str], cwd: Optional[Path] = None, check: bool = True, capture: bool = True, env: Optional[Dict[str, str]] = None) -> subprocess.CompletedProcess:
    res = subprocess.run(
        cmd,
        cwd=str(cwd) if cwd else None,
        capture_output=capture,
        text=True,
        env=env,
    )
    if check and res.returncode != 0:
        raise RuntimeError(f"command failed: {' '.join(shlex.quote(c) for c in cmd)}\nstderr: {res.stderr}")
    return res

def git(*args: str, cwd: Optional[Path] = None, check: bool = True, capture: bool = True) -> subprocess.CompletedProcess:
    return run(["git", *args], cwd=cwd or repo_root(), check=check, capture=capture)

def git_branch_exists(branch: str) -> bool:
    res = git("rev-parse", "--verify", f"refs/heads/{branch}", check=False)
    return res.returncode == 0

def git_worktree_exists(path: Path) -> bool:
    res = git("worktree", "list", "--porcelain", check=False)
    return res.returncode == 0 and str(path.resolve()) in res.stdout

def git_current_branch() -> str:
    return git("rev-parse", "--abbrev-ref", "HEAD").stdout.strip()

def git_workdir_clean() -> bool:
    res = git("status", "--porcelain")
    return res.stdout.strip() == ""

def git_head_sha() -> str:
    return git("rev-parse", "HEAD").stdout.strip()


def resolve_tool_executable(cmd: str) -> str:
    """Resolve a CLI name or path for subprocess (Windows: PATHEXT / .cmd shims)."""
    cmd = os.path.expandvars(os.path.expanduser((cmd or "").strip()))
    if not cmd:
        raise RuntimeError("empty cli command")
    p = Path(cmd)
    if p.is_file():
        return str(p.resolve())
    found = shutil.which(cmd)
    if found:
        return found
    if os.name == "nt" and not p.suffix:
        for ext in (".cmd", ".exe", ".bat"):
            w = shutil.which(cmd + ext)
            if w:
                return w
    return cmd


def build_cli_argv(exe: str, args: List[str]) -> List[str]:
    """
    argv for subprocess.Popen. On Windows, .cmd/.bat are not PE executables for
    CreateProcess — must run via ``cmd.exe /c ...``.
    """
    resolved = resolve_tool_executable(exe)
    if os.name == "nt" and resolved.lower().endswith((".cmd", ".bat")):
        inner = subprocess.list2cmdline([resolved, *args])
        return ["cmd.exe", "/c", inner]
    return [resolved, *args]


# ----------------------------------------------------------------------------
# Skill: feature-worktree-setup
# ----------------------------------------------------------------------------

def skill_worktree_setup(feature_id: str, dry_run: bool = False) -> Dict[str, Any]:
    meta = find_feature_meta(feature_id)
    slug = meta["slug"]
    spec_rel = meta["spec"]
    base_ref = "dev_next"
    branch = f"feat/{feature_id}-{slug}"
    worktree = worktrees_dir() / feature_id

    lock_file = workers_dir() / f"{feature_id}.lock"
    if worktree.exists():
        if lock_file.exists():
            raise RuntimeError(f"worktree {worktree} exists and lock {lock_file} is held; refuse to reuse")
        if not dry_run:
            info(f"worktree {worktree} exists but no lock; removing")
            git("worktree", "remove", "--force", str(worktree), check=False)

    if dry_run:
        return {
            "feature_id": feature_id,
            "branch": branch,
            "worktree_path": str(worktree),
            "base_ref": base_ref,
            "base_exists": git_branch_exists(base_ref),
            "dry_run": True,
        }

    if not git_branch_exists(base_ref):
        raise RuntimeError(f"base branch `{base_ref}` does not exist; run `orch init --create-dev-next` first")

    ledger_append("prepare_started", feature_id, {
        "branch": branch, "worktree_path": str(worktree.relative_to(repo_root())),
        "base_ref": base_ref,
    })

    if git_branch_exists(branch):
        info(f"branch `{branch}` already exists; attaching worktree (no -b)")
        res = git("worktree", "add", str(worktree), branch, check=False)
        if res.returncode != 0:
            raise RuntimeError(
                "git worktree add failed (branch may be checked out in another worktree):\n"
                f"{res.stderr.strip()}\n"
                "Hint: `git worktree list` — remove the other checkout or `git branch -D` after backup if abandoned."
            )
    else:
        git("worktree", "add", "-b", branch, str(worktree), base_ref)

    # Seed .task
    task_dir = worktree / ".task"
    task_dir.mkdir(parents=True, exist_ok=True)
    spec_abs = repo_root() / spec_rel
    if spec_abs.exists():
        shutil.copy(spec_abs, task_dir / "spec.md")
    (task_dir / "plan.md").write_text(f"# {feature_id} Plan\n\n(cli to fill)\n", encoding="utf-8")
    (task_dir / "notes.md").write_text(f"# {feature_id} Notes\n\n", encoding="utf-8")

    # Exclude .task from commits in this worktree
    exclude_file = worktree / ".git"
    if exclude_file.is_file():
        with exclude_file.open("r", encoding="utf-8") as f:
            gitdir_line = f.read().strip()
        gitdir = Path(gitdir_line.removeprefix("gitdir: ").strip())
        if not gitdir.is_absolute():
            gitdir = (worktree / gitdir).resolve()
        info_exclude = gitdir / "info" / "exclude"
        info_exclude.parent.mkdir(parents=True, exist_ok=True)
        existing = ""
        if info_exclude.exists():
            existing = info_exclude.read_text(encoding="utf-8")
        if ".task/" not in existing:
            info_exclude.write_text(existing + "\n.task/\n", encoding="utf-8")

    base_sha = git("rev-parse", "HEAD", cwd=worktree).stdout.strip()

    # empty lock placeholder
    lock_file.write_text("", encoding="utf-8")

    ledger_append("prepare_finished", feature_id, {"commit_sha": base_sha})

    return {
        "feature_id": feature_id,
        "branch": branch,
        "worktree_path": str(worktree),
        "base_sha": base_sha,
        "task_dir": str(task_dir),
    }

# ----------------------------------------------------------------------------
# Skill: feature-cli-dispatch
# ----------------------------------------------------------------------------

def render_prompt_md(feature_id: str, meta: Dict[str, Any], retry_feedback: Optional[str]) -> str:
    extra = ""
    retry_block = ""
    if retry_feedback:
        retry_block = f"\n以下是上次 verify 报告，请针对失败项修正：\n\n```\n{retry_feedback}\n```\n"
    return textwrap.dedent(f"""\
        # Task: {feature_id} {meta['slug']}

        你被分配实现一个 dong 项目的 feature。所有上下文已在你工作目录内。

        ## 强制约束

        - 你只能改本 worktree 内的文件；不要 cd 到 worktree 外。
        - 不要 push 任何分支。不要改写 git 历史。
        - 你的最终交付 = 一组本地 commit（逻辑切分），分支已就位。
        - 必须满足 spec 的 § 5 验收（Hard 全过；Soft 尽量过）。
        - 若与 spec 矛盾，先在 .task/notes.md 记原因，再选最接近 spec 意图的实现。
        - 不要修改 doc/orchestration/、doc/phase*/ 下任何文件。

        ## 必读文档

        1. .task/spec.md
        2. CLAUDE.md / AGENTS.md
        3. doc/positioning.md
        4. doc/perf_budget.md
        {extra}
        ## 工作流建议

        1. 读 spec 全文 → 在 .task/plan.md 写实现计划。
        2. 按 spec § 4 实现步骤逐步推进，每个步骤 1 commit。
        3. 跑 spec § 5 列出的验证命令；失败就改到通过。
        4. 在 .task/notes.md 记下偏差、未实现项、已知风险。

        ## Retry 反馈
        {retry_block}

        ## 完成判定

        - .task/plan.md 存在
        - spec § 5 Hard 规则本地跑通
        - `git status` 干净
        - .task/notes.md 末尾加一行 `STATUS: ready-for-orchestrator-verify`

        完成后退出 cli 进程。
        """)

def skill_cli_dispatch(feature_id: str, retry_feedback_path: Optional[Path] = None, dry_run: bool = False) -> Dict[str, Any]:
    meta = find_feature_meta(feature_id)
    cfg = load_config()
    tool_name = cfg.get("cli_tool", "noop-dryrun")
    tool_cfg = cfg["tools"].get(tool_name)
    if not tool_cfg:
        raise RuntimeError(f"cli tool `{tool_name}` not found in config.tools")

    worktree = worktrees_dir() / feature_id
    if not worktree.exists() and not dry_run:
        raise RuntimeError(f"worktree {worktree} not prepared; run dispatch only after worktree-setup")

    task_dir = worktree / ".task"
    prompt_file = task_dir / "prompt.md"
    retry_feedback = None
    if retry_feedback_path and retry_feedback_path.exists():
        retry_feedback = retry_feedback_path.read_text(encoding="utf-8")
    if not dry_run:
        task_dir.mkdir(exist_ok=True)
        prompt_file.write_text(render_prompt_md(feature_id, meta, retry_feedback), encoding="utf-8")

    report_root = reports_dir() / feature_id
    report_root.mkdir(parents=True, exist_ok=True)
    session_log = report_root / "cli_session.log"
    exit_code_file = report_root / "cli_exit.txt"

    placeholders = {
        "{worktree}": str(worktree),
        "{prompt_file}": str(prompt_file),
        "{feature_id}": feature_id,
        "{spec_path}": str(repo_root() / meta["spec"]),
    }
    rendered_args = [apply_placeholders(a, placeholders) for a in tool_cfg.get("args", [])]
    raw_cmd = str(tool_cfg["command"])
    command = build_cli_argv(raw_cmd, rendered_args)
    if command[0].lower().endswith("cmd.exe") or command[0].lower() == "cmd.exe":
        if shutil.which("cmd.exe") is None:
            raise RuntimeError("cmd.exe not on PATH; cannot launch .cmd/.bat cli wrapper")
    elif not Path(command[0]).is_file():
        raise RuntimeError(
            f"cli command not found for this process: {raw_cmd!r} (argv0 {command[0]!r}). "
            "Set `tools.<name>.command` to a full path (e.g. ...\\\\claude-internal.cmd), "
            "or add the install directory to PATH for the environment that runs `python dong/scripts/orch.py`."
        )

    if dry_run:
        return {
            "feature_id": feature_id,
            "cli_tool": tool_name,
            "command": command,
            "session_log": str(session_log),
            "dry_run": True,
        }

    env = os.environ.copy()
    for k, v in (tool_cfg.get("env") or {}).items():
        env[k] = os.path.expandvars(v)

    # Launch detached background process; redirect stdout+stderr to session log.
    log_fh = session_log.open("ab")
    popen_kwargs: Dict[str, Any] = dict(cwd=str(worktree), stdout=log_fh, stderr=subprocess.STDOUT, env=env)
    if os.name == "nt":
        popen_kwargs["creationflags"] = 0x00000200  # CREATE_NEW_PROCESS_GROUP
    else:
        popen_kwargs["start_new_session"] = True

    try:
        proc = subprocess.Popen(command, **popen_kwargs)
    except FileNotFoundError as e:
        log_fh.close()
        raise RuntimeError(f"cli command not found: {command[0]} ({e})")

    # Spawn a tiny shim that waits and writes exit code — python one-liner.
    # To keep this simple we rely on a separate wait step at status time instead of a shim.

    lock = {
        "feature": feature_id,
        "pid": proc.pid,
        "host": "orch",
        "started_at": utcnow_iso(),
        "cli_tool": tool_name,
        "cli_session": str(session_log.relative_to(repo_root())).replace(os.sep, "/"),
        "exit_file": str(exit_code_file.relative_to(repo_root())).replace(os.sep, "/"),
        "command": command,
    }
    (workers_dir() / f"{feature_id}.lock").write_text(json.dumps(lock, ensure_ascii=False, indent=2), encoding="utf-8")

    ledger_append("cli_dispatched", feature_id, {
        "cli_tool": tool_name,
        "cli_args": command,
        "task_id": f"pid-{proc.pid}",
        "session_log": lock["cli_session"],
    })

    return {"feature_id": feature_id, "pid": proc.pid, "command": command, "session_log": str(session_log)}

def apply_placeholders(s: str, ph: Dict[str, str]) -> str:
    out = s
    for k, v in ph.items():
        out = out.replace(k, v)
    return out

# ----------------------------------------------------------------------------
# CLI monitor (health check)
# ----------------------------------------------------------------------------

def pid_alive(pid: int) -> bool:
    if os.name == "nt":
        try:
            res = subprocess.run(["tasklist", "/FI", f"PID eq {pid}", "/NH"], capture_output=True, text=True)
            return str(pid) in (res.stdout or "")
        except Exception:
            return False
    try:
        os.kill(pid, 0)
        return True
    except (OSError, ProcessLookupError):
        return False

def monitor_active_clis() -> None:
    """For each in-progress feature, check cli process; if dead, append cli_finished."""
    events = ledger_read_all()
    state = reduce_state(events)
    for fid, f in state["features"].items():
        if f["status"] != "cli_running":
            continue
        lock_path = workers_dir() / f"{fid}.lock"
        if not lock_path.exists():
            continue
        try:
            lock = json.loads(lock_path.read_text(encoding="utf-8"))
        except Exception:
            continue
        pid = lock.get("pid")
        if not pid or not pid_alive(int(pid)):
            exit_code = 0
            # try to read session log tail
            session_log = repo_root() / lock.get("cli_session", "")
            tail = ""
            if session_log.exists():
                tail = session_log.read_text(encoding="utf-8", errors="replace").splitlines()[-5:]
                tail = "\n".join(tail)
            # best-effort file count
            files_changed = 0
            try:
                wt = worktrees_dir() / fid
                if wt.exists():
                    diff = git("diff", "--name-only", "dev_next...HEAD", cwd=wt, check=False).stdout.splitlines()
                    files_changed = len(diff)
            except Exception:
                pass
            ledger_append("cli_finished", fid, {
                "exit_code": exit_code,
                "files_changed": files_changed,
                "tail": tail,
            })

# ----------------------------------------------------------------------------
# Skill: feature-verify
# ----------------------------------------------------------------------------

YAML_VERIFY_BLOCK_RE = re.compile(r"```yaml\s*\n(verify:[\s\S]*?)\n```", re.MULTILINE)

def parse_verify_yaml(spec_text: str) -> Optional[Dict[str, Any]]:
    m = YAML_VERIFY_BLOCK_RE.search(spec_text)
    if not m:
        return None
    try:
        import yaml  # type: ignore
    except ImportError:
        print("WARN: pyyaml not installed; cannot parse verify yaml. Install with `pip install pyyaml`.", file=sys.stderr)
        return None
    try:
        data = yaml.safe_load(m.group(1))
        if isinstance(data, dict) and "verify" in data:
            return data["verify"]
    except Exception as e:
        print(f"WARN: failed to parse verify yaml: {e}", file=sys.stderr)
    return None

def skill_verify(feature_id: str, mode: str = "full") -> Dict[str, Any]:
    meta = find_feature_meta(feature_id)
    spec_abs = repo_root() / meta["spec"]
    worktree = worktrees_dir() / feature_id
    if not worktree.exists():
        raise RuntimeError(f"worktree {worktree} missing")

    if not spec_abs.exists():
        raise RuntimeError(f"spec not found: {spec_abs}")

    ledger_append("verify_started", feature_id, {
        "rules_file": str(meta["spec"]),
        "runner": "scripts/orch.py",
        "mode": mode,
    })

    spec_text = spec_abs.read_text(encoding="utf-8")
    verify_cfg = parse_verify_yaml(spec_text)
    parse_quality = "yaml"
    if verify_cfg is None:
        verify_cfg = {"hard": [], "soft": []}
        parse_quality = "heuristic_empty"

    hard_rules = verify_cfg.get("hard", []) or []
    soft_rules = verify_cfg.get("soft", []) or []
    if mode == "hard-only":
        soft_rules = []

    ts = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H-%M-%SZ")
    report_dir = reports_dir() / feature_id / f"verify_{ts}"
    report_dir.mkdir(parents=True, exist_ok=True)

    def run_rule(r: Dict[str, Any], kind: str) -> Dict[str, Any]:
        rid = r.get("id", f"anon_{kind}")
        cmd = r.get("cmd")
        cwd = (repo_root() / r.get("cwd", ".")).resolve()
        timeout = r.get("timeout_sec", 600)
        logf = report_dir / f"{rid}.log"
        entry = {"id": rid, "kind": kind, "command": cmd, "cwd": str(cwd), "passed": False, "exit_code": None, "duration_sec": 0}
        if not cmd:
            entry["error"] = "missing cmd"
            return entry
        t0 = time.time()
        try:
            with logf.open("wb") as lf:
                proc = subprocess.run(cmd, shell=True, cwd=str(cwd), timeout=timeout,
                                      stdout=lf, stderr=subprocess.STDOUT)
            entry["exit_code"] = proc.returncode
            entry["passed"] = proc.returncode == 0
        except subprocess.TimeoutExpired:
            entry["error"] = f"timeout after {timeout}s"
        except Exception as e:
            entry["error"] = str(e)
        entry["duration_sec"] = round(time.time() - t0, 2)
        # optional assert block
        if entry["passed"] and "assert" in r:
            a = r["assert"]
            if a.get("kind") == "json":
                afile = repo_root() / a.get("file", "")
                try:
                    data = json.loads(afile.read_text(encoding="utf-8"))
                    failed = [rule for rule in a.get("rules", []) if not eval_assert_rule(rule, data)]
                    if failed:
                        entry["passed"] = False
                        entry["assert_failed"] = failed
                except Exception as e:
                    entry["passed"] = False
                    entry["assert_error"] = str(e)
        return entry

    rule_results = []
    for r in hard_rules:
        rule_results.append(run_rule(r, "hard"))
    for r in soft_rules:
        rule_results.append(run_rule(r, "soft"))

    if len(hard_rules) == 0:
        rule_results.append({
            "id": "_orch_no_hard_verify_rules",
            "kind": "hard",
            "command": "",
            "cwd": str(repo_root()),
            "passed": False,
            "exit_code": None,
            "duration_sec": 0,
            "error": (
                "Spec §5 must include a fenced ```yaml verify:``` with at least one `hard:` entry and `cmd`. "
                "See doc/orchestration/README.md § merged vs delivery."
            ),
        })

    hard_failed = [r["id"] for r in rule_results if r["kind"] == "hard" and not r["passed"]]
    soft_failed = [r["id"] for r in rule_results if r["kind"] == "soft" and not r["passed"]]
    hard_pass = len(hard_failed) == 0
    soft_pass = len(soft_failed) == 0
    overall = "pass" if hard_pass else "fail"

    report = {
        "feature": feature_id,
        "ran_at": utcnow_iso(),
        "spec_path": meta["spec"],
        "parse_quality": parse_quality,
        "rules": rule_results,
        "hard_pass": hard_pass,
        "soft_pass": soft_pass,
        "overall": overall,
    }
    json_report = report_dir / "report.json"
    md_report = report_dir / "report.md"
    json_report.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    md_report.write_text(render_verify_md(report), encoding="utf-8")

    ledger_append("verify_finished", feature_id, {
        "pass": hard_pass,
        "hard_failed": hard_failed,
        "soft_failed": soft_failed,
        "report_path": str(md_report.relative_to(repo_root())).replace(os.sep, "/"),
    })

    return report

def eval_assert_rule(rule: str, data: Dict[str, Any]) -> bool:
    # rule like "fps.p50 >= 120"
    m = re.match(r"^\s*([\w\.]+)\s*(>=|<=|==|!=|>|<)\s*([\-0-9\.eE]+)\s*$", rule)
    if not m:
        return False
    path, op, rhs = m.group(1), m.group(2), float(m.group(3))
    cur: Any = data
    for seg in path.split("."):
        if isinstance(cur, dict) and seg in cur:
            cur = cur[seg]
        else:
            return False
    try:
        lhs = float(cur)
    except Exception:
        return False
    return {
        ">=": lhs >= rhs, "<=": lhs <= rhs, "==": lhs == rhs,
        "!=": lhs != rhs, ">": lhs > rhs, "<": lhs < rhs,
    }[op]

def render_verify_md(report: Dict[str, Any]) -> str:
    lines = [f"# Verify Report — {report['feature']}", "",
             f"- spec: `{report['spec_path']}`",
             f"- ran_at: {report['ran_at']}",
             f"- overall: **{report['overall']}**",
             f"- parse_quality: {report['parse_quality']}",
             ""]
    for kind in ("hard", "soft"):
        rules = [r for r in report["rules"] if r["kind"] == kind]
        if not rules:
            continue
        lines.append(f"## {kind.capitalize()} ({sum(1 for r in rules if r['passed'])}/{len(rules)})")
        for r in rules:
            status = "PASS" if r["passed"] else "FAIL"
            lines.append(f"- [{status}] `{r['id']}` ({r.get('duration_sec',0)}s) exit={r.get('exit_code')}")
            if not r["passed"] and r.get("assert_failed"):
                for af in r["assert_failed"]:
                    lines.append(f"    - assert failed: `{af}`")
            if r.get("error"):
                lines.append(f"    - error: {r['error']}")
        lines.append("")
    return "\n".join(lines)

# ----------------------------------------------------------------------------
# Skill: feature-merge-back
# ----------------------------------------------------------------------------

def skill_merge(feature_id: str, strategy: str = "squash", dry_run: bool = False) -> Dict[str, Any]:
    meta = find_feature_meta(feature_id)
    slug = meta["slug"]
    branch = f"feat/{feature_id}-{slug}"
    worktree = worktrees_dir() / feature_id

    if not git_branch_exists("dev_next"):
        raise RuntimeError("dev_next branch missing")
    if not git_branch_exists(branch):
        raise RuntimeError(f"feature branch {branch} missing")

    # abort if any other merge in flight
    state = reduce_state(ledger_read_all())
    in_merge = [fid for fid, v in state["features"].items() if v.get("status") == "merging" and fid != feature_id]
    if in_merge:
        raise RuntimeError(f"another merge in flight: {in_merge}; serial policy refuses")

    if dry_run:
        return {"feature_id": feature_id, "branch": branch, "strategy": strategy, "dry_run": True}

    # Must pass before merge_started — otherwise ledger sticks at "merging".
    if not git_workdir_clean():
        raise RuntimeError("main working tree is dirty; commit or stash first")

    ledger_append("merge_started", feature_id, {"strategy": strategy, "target_ref": "dev_next"})

    # checkout dev_next in main workdir
    git("switch", "dev_next")

    # rebase feature onto latest dev_next inside worktree
    res = git("rebase", "dev_next", cwd=worktree, check=False)
    if res.returncode != 0:
        git("rebase", "--abort", cwd=worktree, check=False)
        ledger_append("note", feature_id, {"text": "rebase_conflict; merge aborted"})
        raise RuntimeError(f"rebase conflict in {worktree}; manual resolution required")

    # merge to dev_next
    commit_msg_path = reports_dir() / feature_id / "commit_msg.txt"
    commit_msg_path.parent.mkdir(parents=True, exist_ok=True)
    commit_msg_path.write_text(_gen_commit_message(feature_id, meta, state), encoding="utf-8")

    squash_no_commit = False
    if strategy == "squash":
        git("merge", "--squash", branch)
        staged = git("diff", "--cached", "--stat", check=False).stdout.strip()
        if staged:
            git("commit", "-F", str(commit_msg_path))
        else:
            squash_no_commit = True
            ledger_append(
                "note",
                feature_id,
                {"text": "squash merge produced no staged changes (noop / identical branch); skip git commit"},
            )
    elif strategy == "merge":
        git("merge", "--no-ff", "-F", str(commit_msg_path), branch)
    elif strategy == "ff":
        git("merge", "--ff-only", branch)
    else:
        raise RuntimeError(f"unknown strategy: {strategy}")

    merge_sha = git_head_sha()
    # save diff (empty squash: no new commit, do not use merge_sha~..merge_sha)
    if squash_no_commit:
        diff_txt = "# no file changes merged (feature tree matched dev_next)\n"
    else:
        diff_txt = git("diff", f"{merge_sha}~..{merge_sha}", check=False).stdout
    (reports_dir() / feature_id / "merge_diff.txt").write_text(diff_txt, encoding="utf-8")

    # remove worktree, keep branch
    git("worktree", "remove", str(worktree), check=False)
    lock_file = workers_dir() / f"{feature_id}.lock"
    if lock_file.exists():
        lock_file.unlink()

    ledger_append_batch([
        ("merge_finished", feature_id, {"merge_sha": merge_sha, "target_sha_after": merge_sha}),
        ("feature_done",   feature_id, {"final_status": "merged"}),
        ("worker_released",feature_id, {}),
    ])
    return {"feature_id": feature_id, "merge_sha": merge_sha}

def _gen_commit_message(feature_id: str, meta: Dict[str, Any], state: Dict[str, Any]) -> str:
    f = state["features"].get(feature_id, {})
    last_verify = (f.get("verify_history") or [{}])[-1]
    return textwrap.dedent(f"""\
        [{feature_id}] {meta['slug']}

        Spec: {meta['spec']}
        Worktree: dong/.worktrees/{feature_id} (squashed)
        Verify: hard PASS / soft {'PASS' if not last_verify.get('soft_failed') else 'some-fail'}
        Reports: dong/.orchestration/reports/{feature_id}/

        Auto-generated by dong/scripts/orch.py
        """)

# ----------------------------------------------------------------------------
# Scheduler / eligibility
# ----------------------------------------------------------------------------

def find_feature_meta(feature_id: str) -> Dict[str, Any]:
    for f in FEATURES:
        if f["id"] == feature_id:
            return f
    die(f"unknown feature id: {feature_id}")

def group_class(groups: List[str]) -> str:
    classes = {GROUP_CONFLICT.get(g, "Y") for g in groups}
    for level in ("R", "Y", "G"):
        if level in classes:
            return level
    return "G"

def _downstream_count(fid: str) -> int:
    return sum(1 for m in FEATURES if fid in m["upstream"])

def observe_days_for(fid: str, cfg: Dict[str, Any]) -> Optional[int]:
    """Days to wait after upstream merged_at. None = no window / disabled."""
    ovr = cfg.get("observe_window_days") or {}
    if fid in ovr:
        v = ovr[fid]
        if v is None or v == 0:
            return None
        return int(v)
    return OBSERVE_WINDOW_DAYS.get(fid)

def compute_eligible(state: Dict[str, Any], cfg: Dict[str, Any]) -> List[str]:
    features = state["features"]
    in_progress_status = {"preparing","prepared","cli_running","awaiting_verify","verifying","awaiting_review","awaiting_retry","mergeable","merging"}
    active_by_group_R: Dict[str, int] = {}
    active_by_group_Y: Dict[str, int] = {}
    active_total = 0
    for fid, v in features.items():
        if v["status"] in in_progress_status:
            active_total += 1
            for g in v.get("groups", []):
                cls = GROUP_CONFLICT.get(g, "Y")
                if cls == "R":
                    active_by_group_R[g] = active_by_group_R.get(g, 0) + 1
                elif cls == "Y":
                    active_by_group_Y[g] = active_by_group_Y.get(g, 0) + 1

    eligible = []
    # Priority: earlier wave > more downstream (critical path) > longer weeks > id
    for meta in sorted(FEATURES, key=lambda m: (m["wave"], -_downstream_count(m["id"]), -m["weeks"], m["id"])):
        fid = meta["id"]
        st = features.get(fid, {}).get("status", "registered") if fid in features else "unregistered"
        # `prepared` = worktree ready but `cli_dispatched` never succeeded (e.g. CLI binary missing); must be
        # eligible so `tick` / dispatch can retry `skill_cli_dispatch` only.
        if st not in ("registered", "unregistered", "prepared"):
            continue
        # upstream merged
        if any(features.get(u, {}).get("status") != "merged" for u in meta["upstream"]):
            continue
        # R group not occupied
        blocked_by_R = False
        for g in meta["groups"]:
            if GROUP_CONFLICT.get(g, "Y") == "R" and active_by_group_R.get(g, 0) >= cfg.get("max_per_group_r", 1):
                blocked_by_R = True
                break
        if blocked_by_R:
            continue
        # Y group cap
        blocked_by_Y = False
        for g in meta["groups"]:
            if GROUP_CONFLICT.get(g, "Y") == "Y" and active_by_group_Y.get(g, 0) >= cfg.get("max_per_group_y", 2):
                blocked_by_Y = True
                break
        if blocked_by_Y:
            continue
        # observe window (config observe_window_days overrides built-in table)
        w = observe_days_for(fid, cfg)
        if w:
            # require upstream's merged_at + w days
            latest_upstream_merged = None
            for u in meta["upstream"]:
                ts = features.get(u, {}).get("merged_at")
                if ts:
                    dt = datetime.strptime(ts, "%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=timezone.utc)
                    latest_upstream_merged = max(latest_upstream_merged, dt) if latest_upstream_merged else dt
            if not latest_upstream_merged:
                continue
            eligible_at = latest_upstream_merged.timestamp() + w * 86400
            if time.time() < eligible_at:
                continue
        eligible.append(fid)
        # cap total
        if active_total + len(eligible) >= cfg.get("max_parallel_workers", 4):
            break
    return eligible

# ----------------------------------------------------------------------------
# Commands
# ----------------------------------------------------------------------------

def cmd_init(args: argparse.Namespace) -> None:
    ensure_dirs()
    write_default_config_if_missing()

    # add .gitignore entries at repo root if missing
    gi = repo_root() / ".gitignore"
    if gi.exists():
        text = gi.read_text(encoding="utf-8")
        added = []
        for line in ("dong/.orchestration/", "dong/.worktrees/"):
            if line not in text:
                added.append(line)
        if added:
            with gi.open("a", encoding="utf-8") as f:
                f.write("\n# orchestration\n" + "\n".join(added) + "\n")
            info(f"added to .gitignore: {added}")

    # Determine if ledger already initialized
    events = ledger_read_all()
    already_init = any(e["event"] == "schema_init" for e in events)
    if not already_init:
        ledger_append("schema_init", "_meta", {"version": SCHEMA_VERSION})

    # Optionally create dev_next
    if args.create_dev_next:
        if git_branch_exists("dev_next"):
            info("dev_next already exists; skipping")
        else:
            if not git_workdir_clean():
                die("working tree is dirty; commit/stash before creating dev_next")
            git("switch", "-c", "dev_next")
            info("created branch dev_next")

    # Register features (idempotent: only if not yet)
    state = reduce_state(ledger_read_all())
    to_register = [m for m in FEATURES if m["id"] not in state["features"]]
    for m in to_register:
        ledger_append("feature_registered", m["id"], {
            "spec_path": m["spec"], "groups": m["groups"], "upstream": m["upstream"],
            "wave": m["wave"], "weeks": m["weeks"], "slug": m["slug"],
        })
    info(f"registered {len(to_register)} new features (total {len(FEATURES)})")

    write_snapshot(reduce_state(ledger_read_all()))
    info(f"orchestration initialized at {orch_dir()}")


def cmd_ledger_reset(args: argparse.Namespace) -> None:
    """Backup state-ledger.jsonl and re-init feature rows (all `registered`). Git branches untouched."""
    ensure_dirs()
    led = ledger_path()
    if led.exists() and led.read_text(encoding="utf-8").strip():
        ts = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        bak = led.parent / f"state-ledger.jsonl.bak.{ts}"
        shutil.copy2(led, bak)
        info(f"Backed up ledger to {bak}")

    # Remove worktrees *before* deleting ledger so a hung `git worktree remove` never leaves no ledger.
    if args.remove_worktrees:
        wt_root = worktrees_dir()
        if wt_root.exists():
            for child in sorted(wt_root.iterdir()):
                if child.is_dir():
                    git("worktree", "remove", "--force", str(child), check=False)
            info(f"Removed worktrees under {wt_root}")

    if led.exists():
        led.unlink()

    if workers_dir().exists():
        for lf in workers_dir().glob("*.lock"):
            lf.unlink(missing_ok=True)
        info("Removed worker locks")

    ledger_append("schema_init", "_meta", {"version": SCHEMA_VERSION, "reason": "ledger-reset"})
    for m in FEATURES:
        ledger_append("feature_registered", m["id"], {
            "spec_path": m["spec"],
            "groups": m["groups"],
            "upstream": m["upstream"],
            "wave": m["wave"],
            "weeks": m["weeks"],
            "slug": m["slug"],
        })
    write_snapshot(reduce_state(ledger_read_all()))
    info(
        "Ledger reset: 27 features are `registered`.\n"
        "Use `mark-merged` only for work already on dev_next.\n"
        "Verify now fails if spec §5 has no `hard` rules — add YAML verify blocks before merge."
    )


def cmd_status(args: argparse.Namespace) -> None:
    state = reduce_state(ledger_read_all())
    cfg = load_config()
    counts: Dict[str, int] = {}
    for v in state["features"].values():
        counts[v["status"]] = counts.get(v["status"], 0) + 1
    eligible = compute_eligible(state, cfg)
    info(f"dev_next exists: {git_branch_exists('dev_next')}")
    info(f"active_workers:  {state['active_workers']} / cap {cfg['max_parallel_workers']}")
    info("features by status:")
    for k, v in sorted(counts.items()):
        info(f"  {k:>18} : {v}")
    info(f"eligible now ({len(eligible)}): {', '.join(eligible) if eligible else '-'}")

def cmd_list(args: argparse.Namespace) -> None:
    state = reduce_state(ledger_read_all())
    phase = getattr(args, "phase", None)
    print(f"{'ID':<6} {'phase':<5} {'wave':<4} {'status':<18} {'groups':<30} upstream")
    print("-" * 100)
    for m in FEATURES:
        if phase and not m["id"].lower().startswith(phase.lower()):
            continue
        f = state["features"].get(m["id"], {})
        print(f"{m['id']:<6} {m['id'][:2]:<5} W{m['wave']:<3} {f.get('status','-'):<18} {','.join(m['groups']):<30} {','.join(m['upstream'])}")

def cmd_eligible(args: argparse.Namespace) -> None:
    state = reduce_state(ledger_read_all())
    cfg = load_config()
    el = compute_eligible(state, cfg)
    if not el:
        info("(none)")
        return
    for fid in el:
        meta = find_feature_meta(fid)
        info(f"{fid:<6} wave={meta['wave']} weeks={meta['weeks']} groups={','.join(meta['groups'])}")

def cmd_dispatch(args: argparse.Namespace) -> None:
    fid = args.feature
    find_feature_meta(fid)
    monitor_active_clis()
    state = reduce_state(ledger_read_all())
    f = state["features"].get(fid, {})
    if f.get("status") not in ("registered", "prepared"):
        die(f"{fid} status is `{f.get('status')}`, expected registered/prepared")
    if f.get("status") == "registered":
        skill_worktree_setup(fid, dry_run=args.dry_run)
    out = skill_cli_dispatch(fid, dry_run=args.dry_run)
    info(json.dumps(out, ensure_ascii=False, indent=2))

def cmd_verify(args: argparse.Namespace) -> None:
    report = skill_verify(args.feature, mode=args.mode)
    info(json.dumps({k: report[k] for k in ("feature","overall","hard_pass","soft_pass","parse_quality")},
                    ensure_ascii=False, indent=2))

def cmd_approve(args: argparse.Namespace) -> None:
    ledger_append("review_approved", args.feature, {"approver": args.approver, "notes": args.notes or ""})
    info(f"{args.feature}: approved")

def cmd_reject(args: argparse.Namespace) -> None:
    ledger_append("review_rejected", args.feature, {"approver": args.approver, "reasons": [args.reason]})
    info(f"{args.feature}: rejected")

def cmd_merge(args: argparse.Namespace) -> None:
    out = skill_merge(args.feature, strategy=args.strategy, dry_run=args.dry_run)
    info(json.dumps(out, ensure_ascii=False, indent=2))

def cmd_mark_merged(args: argparse.Namespace) -> None:
    """Record that a feature was merged outside the normal orch pipeline (e.g. dev
    already landed uber-quad on dev_next before orch was used)."""
    fid = args.feature
    find_feature_meta(fid)  # validate id
    state = reduce_state(ledger_read_all())
    f = state["features"].get(fid, {})
    st = f.get("status", "registered")
    if st == "merged":
        info(f"{fid}: already merged in ledger; nothing to do")
        return
    allowed = {"registered", "prepared"}
    if st not in allowed and not args.force:
        die(
            f"{fid}: status is `{st}`; expected registered/prepared. "
            f"Use `orch abort {fid}` first, or pass --force (dangerous)."
        )
    merge_sha = (args.merge_sha or "").strip()
    if not merge_sha:
        if git_branch_exists("dev_next"):
            merge_sha = git("rev-parse", "dev_next").stdout.strip()
        else:
            merge_sha = git("rev-parse", "HEAD").stdout.strip()
    target_sha = (args.target_sha_after or merge_sha).strip()
    note_text = args.note or f"manual mark-merged: code landed outside orch (merge_sha={merge_sha})"
    ledger_append("note", fid, {"text": note_text})
    ledger_append("merge_finished", fid, {"merge_sha": merge_sha, "target_sha_after": target_sha})
    ledger_append("feature_done", fid, {"final_status": "merged"})
    lock = workers_dir() / f"{fid}.lock"
    if lock.exists():
        lock.unlink()
    ledger_append("worker_released", fid, {})
    write_snapshot(reduce_state(ledger_read_all()))
    info(f"{fid}: marked merged (merge_sha={merge_sha}). Run `orch status` or `orch eligible`.")

def cmd_rollback(args: argparse.Namespace) -> None:
    """Append a rollback event: reducer drops ledger lines [from_seq, new_seq] inclusive."""
    fid = args.feature
    find_feature_meta(fid)
    from_seq = int(args.from_seq)
    reason = args.reason or "rollback"
    ledger_append("rollback", fid, {"from_event_id": from_seq, "reason": reason})
    write_snapshot(reduce_state(ledger_read_all()))
    info(f"{fid}: rollback from seq {from_seq}; see `orch status` / `orch ledger-dump`")


def cmd_abort(args: argparse.Namespace) -> None:
    fid = args.feature
    lock = workers_dir() / f"{fid}.lock"
    if lock.exists():
        try:
            data = json.loads(lock.read_text(encoding="utf-8"))
            pid = data.get("pid")
            if pid and pid_alive(int(pid)):
                try:
                    if os.name == "nt":
                        subprocess.run(["taskkill", "/F", "/PID", str(pid)], check=False)
                    else:
                        os.kill(int(pid), signal.SIGTERM)
                except Exception:
                    pass
        except Exception:
            pass
        lock.unlink()
    ledger_append("feature_done", fid, {"final_status": "abandoned", "reason": args.reason or ""})
    info(f"{fid}: aborted")

def cmd_ledger_dump(args: argparse.Namespace) -> None:
    events = ledger_read_all()
    n = args.count or 40
    for e in events[-n:]:
        print(json.dumps(e, ensure_ascii=False))

def cmd_snapshot(args: argparse.Namespace) -> None:
    state = reduce_state(ledger_read_all())
    write_snapshot(state)
    info(f"snapshot rebuilt: {snapshot_path()}")

def cmd_config_show(args: argparse.Namespace) -> None:
    info(json.dumps(load_config(), ensure_ascii=False, indent=2))

def cmd_tick(args: argparse.Namespace) -> None:
    """One full orchestration tick."""
    cfg = load_config()
    dry = args.dry_run
    actions: List[str] = []

    # Step 1-2: reduce + monitor clis
    if not dry:
        monitor_active_clis()
    state = reduce_state(ledger_read_all())

    # Step 3: verify awaiting_verify
    for fid, f in list(state["features"].items()):
        if f["status"] == "awaiting_verify":
            actions.append(f"verify {fid}")
            if not dry:
                skill_verify(fid, mode="full")
    state = reduce_state(ledger_read_all())

    # Step 4: announce awaiting_review (no auto-approve unless cfg says so)
    for fid, f in state["features"].items():
        if f["status"] == "awaiting_review":
            actions.append(f"awaiting_review {fid} (use `orch approve {fid}` or `orch reject`)")

    # Step 5: serial merge (one per tick)
    mergeable = [fid for fid, v in state["features"].items() if v["status"] == "mergeable"]
    if mergeable:
        fid = mergeable[0]
        actions.append(f"merge {fid}")
        if not dry:
            try:
                skill_merge(fid)
            except Exception as e:
                actions.append(f"merge {fid} FAILED: {e}")
    state = reduce_state(ledger_read_all())

    # Step 6-7: dispatch new workers
    eligible = compute_eligible(state, cfg)
    free_slots = cfg["max_parallel_workers"] - state["active_workers"]
    max_n = args.max if args.max else free_slots
    dispatched = 0
    for fid in eligible:
        if dispatched >= max_n or dispatched >= free_slots:
            break
        actions.append(f"dispatch {fid}")
        if not dry:
            try:
                fst = state["features"].get(fid, {}).get("status")
                if fst == "registered":
                    skill_worktree_setup(fid)
                skill_cli_dispatch(fid)
            except Exception as e:
                actions.append(f"dispatch {fid} FAILED: {e}")
                continue
        dispatched += 1
        # recompute to respect caps
        state = reduce_state(ledger_read_all())

    write_snapshot(reduce_state(ledger_read_all()))
    head = "DRY RUN" if dry else "EXECUTED"
    info(f"[{head}] tick @ {utcnow_iso()}  actions={len(actions)}")
    for a in actions:
        info(f"  - {a}")

# ----------------------------------------------------------------------------
# Argparse
# ----------------------------------------------------------------------------

def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(prog="orch", description="Dong multi-feature orchestrator")
    sub = p.add_subparsers(dest="cmd", required=True)

    s = sub.add_parser("init", help="first-time initialization")
    s.add_argument("--create-dev-next", action="store_true", help="create dev_next branch if missing")
    s.set_defaults(func=cmd_init)

    s = sub.add_parser(
        "ledger-reset",
        help="backup state-ledger.jsonl and re-register all features (noop / mistaken merged recovery)",
    )
    s.add_argument(
        "--remove-worktrees",
        action="store_true",
        help="git worktree remove --force each dong/.worktrees/<id>/ (recommended before real CLI)",
    )
    s.set_defaults(func=cmd_ledger_reset)

    s = sub.add_parser("status", help="short summary")
    s.set_defaults(func=cmd_status)

    s = sub.add_parser("list", help="list all features")
    s.add_argument("--phase", choices=["p0", "p1", "p2"], help="filter by phase")
    s.set_defaults(func=cmd_list)

    s = sub.add_parser("eligible", help="list eligible features for dispatch")
    s.set_defaults(func=cmd_eligible)

    s = sub.add_parser("dispatch", help="setup worktree + dispatch cli for a feature")
    s.add_argument("feature")
    s.add_argument("--dry-run", action="store_true")
    s.set_defaults(func=cmd_dispatch)

    s = sub.add_parser("verify", help="run verify for a feature")
    s.add_argument("feature")
    s.add_argument("--mode", choices=["full", "hard-only"], default="full")
    s.set_defaults(func=cmd_verify)

    s = sub.add_parser("approve", help="mark feature review_approved")
    s.add_argument("feature")
    s.add_argument("--approver", default="human")
    s.add_argument("--notes", default="")
    s.set_defaults(func=cmd_approve)

    s = sub.add_parser("reject", help="mark feature review_rejected")
    s.add_argument("feature")
    s.add_argument("reason")
    s.add_argument("--approver", default="human")
    s.set_defaults(func=cmd_reject)

    s = sub.add_parser("merge", help="merge approved feature into dev_next")
    s.add_argument("feature")
    s.add_argument("--strategy", choices=["squash", "merge", "ff"], default="squash")
    s.add_argument("--dry-run", action="store_true")
    s.set_defaults(func=cmd_merge)

    s = sub.add_parser(
        "mark-merged",
        help="ledger-only: feature already merged outside orch (e.g. P0-1 uber-quad committed by hand)",
    )
    s.add_argument("feature")
    s.add_argument("--merge-sha", default="", help="commit on dev_next that contains the work (default: git rev-parse dev_next or HEAD)")
    s.add_argument("--target-sha-after", default="", help="dev_next tip after merge (default: same as merge-sha)")
    s.add_argument("--note", default="", help="reason for audit trail")
    s.add_argument("--force", action="store_true", help="allow even if feature was in_progress (use after abort/cleanup)")
    s.set_defaults(func=cmd_mark_merged)

    s = sub.add_parser(
        "rollback",
        help="undo ledger range [from_seq, this event] (e.g. after failed merge); see state-ledger-schema",
    )
    s.add_argument("feature")
    s.add_argument("--from-seq", type=int, required=True, dest="from_seq", help="first ledger seq to drop (inclusive)")
    s.add_argument("--reason", default="")
    s.set_defaults(func=cmd_rollback)

    s = sub.add_parser("abort", help="abort a feature")
    s.add_argument("feature")
    s.add_argument("--reason", default="")
    s.set_defaults(func=cmd_abort)

    s = sub.add_parser("tick", help="advance one tick")
    s.add_argument("--dry-run", action="store_true")
    s.add_argument("--max", type=int, help="max new dispatches this tick")
    s.set_defaults(func=cmd_tick)

    s = sub.add_parser("ledger-dump", help="print last N ledger events")
    s.add_argument("count", type=int, nargs="?", default=40)
    s.set_defaults(func=cmd_ledger_dump)

    s = sub.add_parser("snapshot", help="rebuild snapshot")
    s.set_defaults(func=cmd_snapshot)

    s = sub.add_parser("config-show", help="print effective config")
    s.set_defaults(func=cmd_config_show)

    return p

def main(argv: Optional[List[str]] = None) -> None:
    parser = build_parser()
    args = parser.parse_args(argv)
    args.func(args)

if __name__ == "__main__":
    main()
