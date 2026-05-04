#!/usr/bin/env python3
"""
Parse Dong stderr/log lines containing [GPUStats] and compare two runs (A vs B).

Example:
  set DONG_GPU_STATS=1
  dong_app ... 2> run_a.log
  dong_app ... 2> run_b.log
  python scripts/tools/compare_gpu_stats.py run_a.log run_b.log

Single-file summary (mean / last frame metrics):

  python scripts/tools/compare_gpu_stats.py run_a.log
"""
from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List


GPU_STATS_RE = re.compile(
    r"\[GPUStats\]\s+"
    r"frame=(?P<frame>\d+)\s+"
    r"cpu_prepare_us=(?P<cpu_prepare>-?\d+)\s+"
    r"cpu_execute_us=(?P<cpu_execute>-?\d+)\s+"
    r"uber_inst_batches=(?P<uber_inst_batches>\d+)\s+"
    r"uber_inst_instances=(?P<uber_inst_instances>\d+)\s+"
    r"uber_uniform_draws=(?P<uber_uniform_draws>\d+)\s+"
    r"uber_gpu_draws_total=(?P<uber_gpu_draws_total>\d+)\s+"
    r"uber_compiler_pool=(?P<uber_compiler_pool>\d+)\s+"
    r"legacy_uber_batch_fallback=(?P<legacy_fallback>\d+)"
)


@dataclass
class GpuStatsRow:
    frame: int
    cpu_prepare_us: int
    cpu_execute_us: int
    uber_inst_batches: int
    uber_inst_instances: int
    uber_uniform_draws: int
    uber_gpu_draws_total: int
    uber_compiler_pool: int
    legacy_uber_batch_fallback: int

    @classmethod
    def from_match(cls, m: re.Match[str]) -> GpuStatsRow:
        d = m.groupdict()
        return cls(
            frame=int(d["frame"]),
            cpu_prepare_us=int(d["cpu_prepare"]),
            cpu_execute_us=int(d["cpu_execute"]),
            uber_inst_batches=int(d["uber_inst_batches"]),
            uber_inst_instances=int(d["uber_inst_instances"]),
            uber_uniform_draws=int(d["uber_uniform_draws"]),
            uber_gpu_draws_total=int(d["uber_gpu_draws_total"]),
            uber_compiler_pool=int(d["uber_compiler_pool"]),
            legacy_uber_batch_fallback=int(d["legacy_fallback"]),
        )


def iter_rows_from_text(text: str) -> Iterable[GpuStatsRow]:
    for line in text.splitlines():
        m = GPU_STATS_RE.search(line)
        if m:
            yield GpuStatsRow.from_match(m)


def load_rows(path: Path) -> List[GpuStatsRow]:
    return list(iter_rows_from_text(path.read_text(encoding="utf-8", errors="replace")))


def mean_int(xs: List[int]) -> float:
    if not xs:
        return 0.0
    return float(sum(xs)) / float(len(xs))


def summarize(rows: List[GpuStatsRow]) -> Dict[str, float]:
    if not rows:
        return {}
    keys = [
        "cpu_prepare_us",
        "cpu_execute_us",
        "uber_inst_batches",
        "uber_inst_instances",
        "uber_uniform_draws",
        "uber_gpu_draws_total",
        "uber_compiler_pool",
        "legacy_uber_batch_fallback",
    ]
    out: Dict[str, float] = {"frames": float(len(rows))}
    for k in keys:
        vals = [getattr(r, k) for r in rows]
        out[f"{k}_mean"] = mean_int(vals)
        out[f"{k}_last"] = float(vals[-1])
    return out


def fmt_delta(a: float, b: float) -> str:
    if a == 0 and b == 0:
        return "0"
    if a == 0:
        return f"n/a (A=0, B={b:.1f})"
    pct = (b - a) / a * 100.0
    return f"{b - a:+.1f} ({pct:+.1f}%)"


def print_compare(sa: Dict[str, float], sb: Dict[str, float], label_a: str, label_b: str) -> None:
    print(f"frames: {label_a}={int(sa.get('frames', 0))}  {label_b}={int(sb.get('frames', 0))}")
    keys = [
        ("cpu_prepare_us_mean", "cpu_prepare_us (mean µs)"),
        ("cpu_execute_us_mean", "cpu_execute_us (mean µs)"),
        ("uber_gpu_draws_total_mean", "uber_gpu_draws_total (mean, proxy drawcalls)"),
        ("uber_uniform_draws_mean", "uber_uniform_draws (mean)"),
        ("uber_inst_batches_mean", "uber_inst_batches (mean)"),
        ("legacy_uber_batch_fallback_mean", "legacy_uber_batch_fallback (mean)"),
    ]
    for k, title in keys:
        va = sa.get(k, 0.0)
        vb = sb.get(k, 0.0)
        print(f"{title}:  {label_a}={va:.1f}  {label_b}={vb:.1f}  delta_B_vs_A={fmt_delta(va, vb)}")


def main() -> int:
    p = argparse.ArgumentParser(description="Parse [GPUStats] from Dong logs: one file = summary, two = A/B compare.")
    p.add_argument("paths", nargs="*", type=Path, help="One log (summary) or two logs (A vs B)")
    args = p.parse_args()

    if len(args.paths) == 0:
        p.print_help()
        return 2

    if len(args.paths) == 1:
        rows = load_rows(args.paths[0], False)
        s = summarize(rows)
        if not s:
            print(f"No [GPUStats] lines in {args.paths[0]}", file=sys.stderr)
            return 1
        print(f"file: {args.paths[0]}")
        for k, v in sorted(s.items()):
            print(f"  {k}: {v:.3f}")
        return 0

    rows_a = load_rows(args.paths[0], False)
    rows_b = load_rows(args.paths[1], False)
    sa, sb = summarize(rows_a), summarize(rows_b)
    if not sa or not sb:
        print("One or both logs had no [GPUStats] lines.", file=sys.stderr)
        return 1
    print_compare(sa, sb, str(args.paths[0]), str(args.paths[1]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
