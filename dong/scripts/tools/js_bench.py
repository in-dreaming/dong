#!/usr/bin/env python3
"""JS engine benchmark runner for Dong.
Runs JS microbenchmarks inside dong_app and collects results.

Usage:
  python scripts/tools/js_bench.py                        # Run all benchmarks
  python scripts/tools/js_bench.py --report md            # Generate markdown report
  python scripts/tools/js_bench.py --output results.json  # Save JSON results
  python scripts/tools/js_bench.py --exe-dir path/to/bin  # Override exe directory
  python scripts/tools/js_bench.py --runs 3               # Average over N runs
"""
import argparse
import json
import os
import re
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path


def find_exe(exe_dir: Path) -> Path:
    """Find dong_app executable in the given directory."""
    name = "dong_app.exe" if sys.platform == "win32" else "dong_app"
    exe = exe_dir / name
    if exe.exists():
        return exe
    raise FileNotFoundError(f"dong_app not found at {exe}")


def run_benchmark(exe: Path, html: Path, timeout_s: float = 60.0, porffor_module: str | None = None) -> str:
    """Run dong_app with the benchmark HTML and capture output."""
    env = os.environ.copy()
    env["DONG_BENCH_AUTOSTOP"] = "1"
    env["DONG_BENCH_WARMUP_MS"] = "500"
    env["DONG_BENCH_RUN_MS"] = "30000"
    env["DONG_LOG_TO_STDOUT"] = "1"
    env["DONG_SCRIPT_TIMEOUT_MS"] = "60000"
    if porffor_module:
        env["DONG_PORFFOR_MODULE"] = porffor_module

    cmd = [str(exe), "--html", str(html)]
    if porffor_module:
        cmd.extend(["--porffor-module", porffor_module])
    proc = subprocess.run(
        cmd,
        env=env,
        cwd=str(exe.parent),
        timeout=timeout_s,
        capture_output=True,
        check=False,
    )
    output = proc.stdout.decode("utf-8", errors="replace")
    output += proc.stderr.decode("utf-8", errors="replace")
    return output


def parse_results(output: str) -> dict:
    """Parse [BENCH] lines from dong_app output."""
    results = {}
    pattern = r"\[BENCH\]\s+([\w]+):\s+([\d.]+)ms\s+\((\d+)\s+iterations,\s+([\d.]+)\s+ns/iter\)"
    for m in re.finditer(pattern, output):
        name = m.group(1)
        total_ms = float(m.group(2))
        iterations = int(m.group(3))
        ns_per_iter = float(m.group(4))
        results[name] = {
            "total_ms": total_ms,
            "iterations": iterations,
            "ns_per_iter": ns_per_iter,
        }
    return results


def merge_results(all_runs: list[dict]) -> dict:
    """Average results across multiple runs."""
    if len(all_runs) == 1:
        return all_runs[0]

    merged = {}
    all_keys = set()
    for run in all_runs:
        all_keys.update(run.keys())

    for key in all_keys:
        values = [r[key] for r in all_runs if key in r]
        if not values:
            continue
        merged[key] = {
            "total_ms": round(sum(v["total_ms"] for v in values) / len(values), 1),
            "iterations": values[0]["iterations"],
            "ns_per_iter": round(
                sum(v["ns_per_iter"] for v in values) / len(values), 1
            ),
            "runs": len(values),
            "min_ns": min(v["ns_per_iter"] for v in values),
            "max_ns": max(v["ns_per_iter"] for v in values),
        }
    return merged


def generate_markdown_report(results: dict, machine: str, num_runs: int, engine: str) -> str:
    """Generate a markdown formatted report."""
    lines = [
        "# Dong JS Engine Benchmark Report",
        "",
        f"**Engine**: {engine}",
        f"**Machine**: {machine}",
        f"**Timestamp**: {datetime.now().isoformat()}",
        f"**Runs**: {num_runs}",
        "",
        "| Benchmark | Total (ms) | Iterations | ns/iter |",
        "|-----------|----------:|----------:|--------:|",
    ]
    for name, data in sorted(results.items()):
        lines.append(
            f"| {name} | {data['total_ms']:.1f} | {data['iterations']} | {data['ns_per_iter']:.1f} |"
        )
    lines.append("")

    if num_runs > 1 and "min_ns" in next(iter(results.values()), {}):
        lines.append("### Variance (min/max ns/iter)")
        lines.append("")
        lines.append("| Benchmark | Min | Max | Spread |")
        lines.append("|-----------|----:|----:|-------:|")
        for name, data in sorted(results.items()):
            if "min_ns" in data:
                spread = data["max_ns"] - data["min_ns"]
                lines.append(
                    f"| {name} | {data['min_ns']:.1f} | {data['max_ns']:.1f} | {spread:.1f} |"
                )
        lines.append("")

    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description="Dong JS engine benchmark runner")
    ap.add_argument("--exe-dir", default=None, help="Override exe directory")
    ap.add_argument("--output", default=None, help="Save JSON results to file")
    ap.add_argument(
        "--report", choices=["md"], default=None, help="Generate report format"
    )
    ap.add_argument(
        "--runs", type=int, default=1, help="Number of runs to average (default: 1)"
    )
    ap.add_argument(
        "--engine",
        choices=["quickjs", "porffor"],
        default="porffor",
        help="Script engine to benchmark (default: porffor)",
    )
    ap.add_argument(
        "--html",
        default=None,
        help="Override benchmark HTML path",
    )
    ap.add_argument(
        "--timeout", type=float, default=60.0, help="Timeout per run in seconds"
    )
    args = ap.parse_args()

    script_dir = Path(__file__).resolve().parent
    dong_dir = script_dir.parent.parent
    exe_dir = Path(args.exe_dir) if args.exe_dir else dong_dir / "zig-out" / "bin"
    html_path = (
        Path(args.html)
        if args.html
        else (
            dong_dir / "examples" / "data" / "benchmarks" / "js_microbench_porffor.html"
            if args.engine == "porffor"
            else dong_dir / "examples" / "data" / "benchmarks" / "js_microbench.html"
        )
    )
    porffor_module = "js_microbench" if args.engine == "porffor" else None

    try:
        exe = find_exe(exe_dir)
    except FileNotFoundError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

    if not html_path.exists():
        print(f"ERROR: Benchmark HTML not found: {html_path}", file=sys.stderr)
        return 1

    # Compute relative path from exe dir for dong_app --html argument
    try:
        rel_html = os.path.relpath(html_path, exe_dir)
    except ValueError:
        # On Windows, relpath fails across drives
        rel_html = str(html_path)

    print(f"Running JS benchmarks...")
    print(f"  exe:   {exe}")
    print(f"  engine: {args.engine}")
    print(f"  html:  {html_path}")
    print(f"  runs:  {args.runs}")
    print()

    all_runs = []
    for run_idx in range(args.runs):
        if args.runs > 1:
            print(f"  Run {run_idx + 1}/{args.runs}...", end=" ", flush=True)

        try:
            output = run_benchmark(exe, Path(rel_html), timeout_s=args.timeout, porffor_module=porffor_module)
        except subprocess.TimeoutExpired:
            print(f"ERROR: Benchmark timed out after {args.timeout}s")
            return 1

        results = parse_results(output)

        if not results:
            print("WARNING: No benchmark results found in output")
            if args.runs == 1:
                print("--- stdout+stderr (first 2000 chars) ---")
                print(output[:2000])
            return 1

        if args.runs > 1:
            print(f"{len(results)} benchmarks collected")

        all_runs.append(results)

    merged = merge_results(all_runs)
    machine = os.environ.get("COMPUTERNAME", os.environ.get("HOSTNAME", "unknown"))

    # Print report
    report = generate_markdown_report(merged, machine, args.runs, args.engine)
    print(report)

    # Save JSON
    if args.output:
        out_data = {
            "engine": args.engine,
            "machine": machine,
            "timestamp": datetime.now().isoformat(),
            "runs": args.runs,
            "results": merged,
        }
        Path(args.output).write_text(json.dumps(out_data, indent=2), encoding="utf-8")
        print(f"Results saved to {args.output}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
