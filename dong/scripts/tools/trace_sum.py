#!/usr/bin/env python3
"""Summarize dong chrome-trace JSON (profiler_dump output).

This is a small utility to quickly identify time sinks by event name.
It expects B/E events in traceEvents.

Usage:
  python scripts/tools/trace_sum.py <trace.json> [--top N] [--filter REGEX]

Notes:
- Times are in microseconds in the trace, output is milliseconds.
"""

from __future__ import annotations

import argparse
import json
import re
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path


@dataclass
class Stat:
    count: int = 0
    sum_us: float = 0.0
    max_us: float = 0.0

    def add(self, dur_us: float) -> None:
        self.count += 1
        self.sum_us += dur_us
        if dur_us > self.max_us:
            self.max_us = dur_us


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("trace", type=Path)
    ap.add_argument("--top", type=int, default=30)
    ap.add_argument("--filter", dest="filter_re", type=str, default="")
    args = ap.parse_args()

    data = json.loads(args.trace.read_text(encoding="utf-8"))
    events = data.get("traceEvents") or []

    name_filter = re.compile(args.filter_re) if args.filter_re else None

    stacks: dict[tuple[int, int], list[dict]] = defaultdict(list)
    stats: dict[str, Stat] = defaultdict(Stat)

    for e in events:
        ph = e.get("ph")
        if ph == "B":
            key = (int(e.get("pid", 0)), int(e.get("tid", 0)))
            stacks[key].append(e)
        elif ph == "E":
            key = (int(e.get("pid", 0)), int(e.get("tid", 0)))
            if not stacks[key]:
                continue
            b = stacks[key].pop()
            if b.get("name") != e.get("name"):
                continue
            name = b.get("name") or ""
            if name_filter and not name_filter.search(name):
                continue
            dur_us = float(e.get("ts", 0.0)) - float(b.get("ts", 0.0))
            if dur_us < 0:
                continue
            stats[name].add(dur_us)

    rows = sorted(stats.items(), key=lambda kv: kv[1].sum_us, reverse=True)
    topn = max(1, args.top)

    print(f"file: {args.trace}")
    if args.filter_re:
        print(f"filter: {args.filter_re}")
    print(f"events: {len(events)}\n")

    print(f"{'name':48} {'count':>7} {'sum_ms':>12} {'max_ms':>10}")
    print("-" * 82)
    for name, st in rows[:topn]:
        print(f"{name[:48]:48} {st.count:7d} {st.sum_us/1000.0:12.3f} {st.max_us/1000.0:10.3f}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
