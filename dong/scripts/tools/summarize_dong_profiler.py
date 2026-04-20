#!/usr/bin/env python3
"""
Summarize Dong Chrome Trace JSON (dong_profile.json from DONG_PROFILER=1 + engine destroy).

- Duration events: pairs ph \"B\" / \"E\" per tid (stack), duration = ts(E) - ts(B) in microseconds.
- Instant ph \"i\" with args.value: reported as scalar samples (mean/count) per name.

Usage:
  python scripts/tools/summarize_dong_profiler.py zig-out/tmp/profile.json
  python scripts/tools/summarize_dong_profiler.py zig-out/tmp/profile.json --top 40
"""
from __future__ import annotations

import argparse
import json
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, DefaultDict, Dict, List, Optional, Tuple


@dataclass
class DurAgg:
    count: int = 0
    total_us: float = 0.0
    max_us: float = 0.0

    def add(self, dt_us: float) -> None:
        self.count += 1
        self.total_us += dt_us
        self.max_us = max(self.max_us, dt_us)


@dataclass
class ScalarAgg:
    count: int = 0
    total: float = 0.0
    last: Optional[int] = None

    def add(self, v: int) -> None:
        self.count += 1
        self.total += float(v)
        self.last = v


def summarize(path: Path, top: int) -> int:
    data = json.loads(path.read_text(encoding="utf-8"))
    events: List[Dict[str, Any]] = data.get("traceEvents") or []
    # Stable order by timestamp
    events.sort(key=lambda e: (e.get("ts", 0), str(e.get("ph", ""))))

    stacks: Dict[int, List[Tuple[str, float]]] = defaultdict(list)
    dur_by_name: DefaultDict[str, DurAgg] = defaultdict(DurAgg)
    instant_by_name: DefaultDict[str, ScalarAgg] = defaultdict(ScalarAgg)

    for ev in events:
        ph = ev.get("ph")
        tid = int(ev.get("tid", 0))
        name = str(ev.get("name", ""))
        ts = float(ev.get("ts", 0))

        if ph == "B":
            stacks[tid].append((name, ts))
        elif ph == "E":
            st = stacks[tid]
            if not st:
                continue
            b_name, b_ts = st.pop()
            dt = ts - b_ts
            if dt >= 0:
                dur_by_name[b_name].add(dt)
        elif ph == "i":
            args = ev.get("args") or {}
            if "value" in args:
                try:
                    instant_by_name[name].add(int(args["value"]))
                except (TypeError, ValueError):
                    pass

    print(f"file: {path}")
    print(f"duration events (B/E), by total time (top {top}):")
    rows = sorted(dur_by_name.items(), key=lambda kv: kv[1].total_us, reverse=True)[:top]
    print(f"{'name':<48} {'count':>8} {'total_ms':>12} {'mean_us':>10} {'max_ms':>10}")
    for n, a in rows:
        mean = a.total_us / a.count if a.count else 0.0
        print(f"{n:<48} {a.count:8d} {a.total_us / 1000.0:12.3f} {mean:10.1f} {a.max_us / 1000.0:10.3f}")

    if instant_by_name:
        print()
        print("instant samples (args.value), by name:")
        print(f"{'name':<48} {'count':>8} {'mean':>14} {'last':>14}")
        for n in sorted(instant_by_name.keys()):
            a = instant_by_name[n]
            mean = a.total / a.count if a.count else 0.0
            print(f"{n:<48} {a.count:8d} {mean:14.3f} {a.last if a.last is not None else 0:14d}")

    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description="Summarize Dong Chrome Trace profiler JSON.")
    ap.add_argument("trace_json", type=Path, help="Path to dong_profile.json (or DONG_PROFILE_OUTPUT)")
    ap.add_argument("--top", type=int, default=35, help="Max rows for duration table")
    args = ap.parse_args()
    if not args.trace_json.is_file():
        print(f"Not a file: {args.trace_json}", file=sys.stderr)
        return 2
    return summarize(args.trace_json, max(1, args.top))


if __name__ == "__main__":
    raise SystemExit(main())
