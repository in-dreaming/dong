#!/usr/bin/env python3
"""Analyze a Dong Chrome Trace JSON profile and print summary statistics."""

import json
import sys
from collections import defaultdict

def analyze(path):
    with open(path, 'r') as f:
        data = json.load(f)

    events = data.get('traceEvents', [])

    # Build begin/end pairs by matching B/E events per thread
    stacks = defaultdict(list)  # tid -> [(name, cat, ts)]
    durations = defaultdict(list)  # name -> [duration_us]

    for ev in events:
        ph = ev.get('ph')
        tid = ev.get('tid', 0)
        ts = ev.get('ts', 0)
        name = ev.get('name', '')
        cat = ev.get('cat', '')

        if ph == 'B':
            stacks[tid].append((name, cat, ts))
        elif ph == 'E':
            if stacks[tid]:
                bname, bcat, bts = stacks[tid].pop()
                dur = ts - bts
                durations[bname].append(dur)

    if not durations:
        print("No duration events found.")
        return

    # Print summary
    print(f"{'Name':<40} {'Count':>8} {'Total(ms)':>12} {'Avg(ms)':>10} {'Min(ms)':>10} {'Max(ms)':>10}")
    print("-" * 92)

    rows = []
    for name, durs in durations.items():
        total = sum(durs)
        avg = total / len(durs)
        mn = min(durs)
        mx = max(durs)
        rows.append((name, len(durs), total, avg, mn, mx))

    rows.sort(key=lambda r: r[2], reverse=True)

    for name, count, total, avg, mn, mx in rows:
        print(f"{name:<40} {count:>8} {total/1000:>12.2f} {avg/1000:>10.3f} {mn/1000:>10.3f} {mx/1000:>10.3f}")

    # Frame analysis: EngineView::tick
    tick_durs = durations.get('EngineView::tick', [])
    if tick_durs:
        print(f"\n--- Frame Analysis (EngineView::tick) ---")
        print(f"Total frames: {len(tick_durs)}")
        avg_frame = sum(tick_durs) / len(tick_durs)
        print(f"Avg frame time: {avg_frame/1000:.3f} ms ({1_000_000/avg_frame:.1f} FPS)")
        print(f"Min frame time: {min(tick_durs)/1000:.3f} ms ({1_000_000/max(tick_durs):.1f} FPS worst)")
        print(f"Max frame time: {max(tick_durs)/1000:.3f} ms ({1_000_000/min(tick_durs):.1f} FPS best)")

        # P50/P95/P99
        sorted_durs = sorted(tick_durs)
        p50 = sorted_durs[len(sorted_durs)//2]
        p95 = sorted_durs[int(len(sorted_durs)*0.95)]
        p99 = sorted_durs[int(len(sorted_durs)*0.99)]
        print(f"P50: {p50/1000:.3f} ms  P95: {p95/1000:.3f} ms  P99: {p99/1000:.3f} ms")

    # Init analysis
    init_names = ['EngineView::loadHTML', 'HTML::parse', 'Script::eval', 'Layout::primeInitial']
    init_durs = {n: durations.get(n, []) for n in init_names if n in durations}
    if init_durs:
        print(f"\n--- Init Phase ---")
        for name, durs in init_durs.items():
            total = sum(durs)
            print(f"  {name}: {total/1000:.2f} ms")

    # Per-tick breakdown: what fraction of tick time each sub-step takes
    sub_steps = ['SmoothScroll', 'SyncVideos', 'ScriptTasks', 'ComputeStyles',
                 'ComputeLayout', 'SyncDialogTopLayer', 'GenerateCommands',
                 'UploadVideoFrames', 'ExecuteGPU']
    if tick_durs:
        total_tick = sum(tick_durs)
        print(f"\n--- Per-Tick Breakdown (% of total tick time) ---")
        accounted = 0
        for s in sub_steps:
            if s in durations:
                t = sum(durations[s])
                accounted += t
                pct = 100 * t / total_tick if total_tick else 0
                avg_s = t / len(durations[s]) if durations[s] else 0
                print(f"  {s:<30} {t/1000:>10.2f} ms  ({pct:>5.1f}%)  avg={avg_s/1000:.3f} ms")
        unaccounted = total_tick - accounted
        if total_tick:
            print(f"  {'(overhead/other)':<30} {unaccounted/1000:>10.2f} ms  ({100*unaccounted/total_tick:>5.1f}%)")

if __name__ == '__main__':
    path = sys.argv[1] if len(sys.argv) > 1 else 'dong_profile.json'
    analyze(path)
