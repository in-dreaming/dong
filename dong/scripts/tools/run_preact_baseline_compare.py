#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
React/Preact baseline comparison tool.

This tool compares Preact/React component rendering between Chrome baseline and Dong engine.
Unlike the native HTML rendering baseline, this tool understands:
- ES module bundling and execution
- React/Preact component lifecycle
- Dynamic DOM creation and updates
- Bundle.js execution timing

Usage:
    python run_preact_baseline_compare.py \
        --preact-dir zig-out/bin/data \
        --out-dir zig-out/tmp/preact_baseline \
        --width 1024 --height 768 \
        --case discovery-buttons

Or run all preact tests:
    python run_preact_baseline_compare.py --preact-dir zig-out/bin/data
"""

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path
from typing import Optional

# Force UTF-8 output on Windows
if sys.platform == "win32":
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')


def run(cmd, cwd=None, env=None, check=True):
    """Execute command with environment override."""
    merged_env = os.environ.copy()
    if env:
        merged_env.update(env)
    p = subprocess.run(
        cmd, cwd=cwd, env=merged_env, shell=False, capture_output=True, text=True
    )
    if check and p.returncode != 0:
        sys.stderr.write(p.stdout)
        sys.stderr.write(p.stderr)
        raise RuntimeError(f"Command failed ({p.returncode}): {' '.join(cmd)}")
    return p.stdout, p.returncode


def find_binary(bin_dir: Path, name: str) -> Optional[Path]:
    """Find executable in bin directory."""
    for candidate in (name, name + ".exe"):
        p = bin_dir / candidate
        if p.exists():
            return p
    return None


def _run_single_preact_case(
    stem: str,
    html_path: Path,
    case_dir: Path,
    args,
    html_render_test: Path,
    baseline_script: Path,
    vl_multi_script: Path,
    bin_dir: Path,
):
    """Run baseline + dong render + analysis for a single preact test."""
    print(f"\n{'=' * 60}")
    print(f"Testing: {stem}")
    print(f"{'=' * 60}")

    # Clean old outputs
    for pattern in [f"{stem}_f*.bmp", f"{stem}_merged.png", f"{stem}_report.json", f"{stem}_base.png"]:
        for p in case_dir.glob(pattern):
            try:
                p.unlink()
            except OSError:
                pass

    # Step 1: Generate Chrome baseline
    base_png = case_dir / f"{stem}_base.png"
    print(f"\n[1/4] Generating Chrome baseline...")
    cmd_base = [
        sys.executable,
        str(baseline_script),
        str(html_path),
        "--out",
        str(base_png),
        "--width",
        str(args.width),
        "--height",
        str(args.height),
        "--wait-ms",
        str(args.base_wait_ms + args.react_wait_ms),  # Combine waits into single --wait-ms
    ]

    try:
        stdout, _ = run(cmd_base)
        print(f"      ✓ Baseline saved: {base_png.name}")
    except RuntimeError as e:
        print(f"      ✗ Baseline generation failed: {e}")
        return False

    # Step 2: Render with Dong engine
    out_base = str(case_dir / f"{stem}.bmp")
    print(f"\n[2/4] Rendering with Dong engine ({args.frames} frames @ {args.frame_ms}ms)...")

    dong_env = {
        "DONG_DISABLE_SCROLLBARS": "1",
        "DONG_SCRIPT_TIMEOUT_MS": str(args.script_timeout_ms),
    }

    cmd_dong = [
        str(html_render_test),
        str(html_path),
        out_base,
        str(args.width),
        str(args.height),
        str(args.frames),
        "--frame-ms",
        str(args.frame_ms),
    ]

    if not args.update:
        cmd_dong.append("--no-update")

    try:
        stdout, _ = run(cmd_dong, cwd=str(bin_dir), env=dong_env)
        print(f"      ✓ Dong frames saved: {case_dir.name}/{stem}_f*.bmp")
    except RuntimeError as e:
        print(f"      ✗ Dong rendering failed: {e}")
        return False

    # Step 3: Generate diff analysis
    merged = case_dir / f"{stem}_merged.png"
    report = case_dir / f"{stem}_report.json"
    print(f"\n[3/4] Analyzing visual differences...")

    cmd_vl = [sys.executable, str(vl_multi_script), str(html_path), "--base", str(base_png)]

    if args.frames <= 1:
        cmd_vl += ["--frames", out_base]
    else:
        cmd_vl += ["--glob", str(case_dir / f"{stem}_f*.bmp")]

    cmd_vl += ["--out-image", str(merged), "--out-json", str(report)]

    if args.llm:
        # LLM analysis enabled
        pass
    else:
        cmd_vl.append("--no-llm")

    try:
        stdout, _ = run(cmd_vl)
        print(f"      ✓ Analysis saved: {report.name}")
    except RuntimeError as e:
        print(f"      ✗ Analysis failed: {e}")
        return False

    # Step 4: Print summary
    print(f"\n[4/4] Summary")
    try:
        with open(report, encoding='utf-8', errors='replace') as f:
            analysis = json.load(f)

        severity = analysis.get("llm", {}).get("overall_severity", "UNKNOWN")
        summary = analysis.get("llm", {}).get("summary", "No summary")

        severity_icon = {
            "NONE": "✓",
            "WARNING": "⚠",
            "CRITICAL": "✗",
        }.get(severity, "?")

        print(f"      {severity_icon} Severity: {severity}")
        print(f"      Summary: {summary[:100]}...")

    except Exception as e:
        print(f"      Could not read analysis: {e}")

    return True


def find_preact_tests(preact_dir: Path) -> dict[str, Path]:
    """Find all preact test HTML files."""
    tests = {}

    # Pattern: preact-<name>/index.html
    for item in preact_dir.iterdir():
        if item.is_dir() and item.name.startswith("preact-"):
            html = item / "index.html"
            if html.exists():
                # Extract test name from directory
                test_name = item.name[len("preact-"):]  # Remove "preact-" prefix
                tests[test_name] = html

    return tests


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Compare Preact/React component rendering between Chrome and Dong.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Test single case
  python run_preact_baseline_compare.py --case discovery-buttons

  # Test all preact components with LLM analysis
  python run_preact_baseline_compare.py --llm --width 1024 --height 768

  # Custom output directory
  python run_preact_baseline_compare.py --out-dir /tmp/preact_compare
        """,
    )

    ap.add_argument(
        "--preact-dir",
        default="zig-out/bin/data",
        help="Directory containing preact-* subdirectories (default: zig-out/bin/data)",
    )
    ap.add_argument(
        "--out-dir",
        default="zig-out/tmp/preact_baseline",
        help="Output directory for results (default: zig-out/tmp/preact_baseline)",
    )
    ap.add_argument(
        "--bin-dir",
        default="zig-out/bin",
        help="Directory containing html_render_test (default: zig-out/bin)",
    )

    ap.add_argument("--width", type=int, default=1024, help="Viewport width (default: 1024)")
    ap.add_argument("--height", type=int, default=768, help="Viewport height (default: 768)")
    ap.add_argument("--frames", type=int, default=5, help="Number of frames to capture (default: 5)")
    ap.add_argument(
        "--frame-ms", type=int, default=100, help="Milliseconds between frames (default: 100)"
    )
    ap.add_argument("--update", action="store_true", help="Call dong_view_update() between frames")
    ap.add_argument("--llm", action="store_true", help="Enable LLM analysis")

    ap.add_argument(
        "--base-wait-ms",
        type=int,
        default=50,
        help="Extra wait before Chrome screenshot (default: 50ms)",
    )
    ap.add_argument(
        "--react-wait-ms",
        type=int,
        default=500,
        help="Extra wait for React/Preact hydration (default: 500ms)",
    )
    ap.add_argument(
        "--script-timeout-ms",
        type=int,
        default=10000,
        help="Dong script execution timeout (default: 10000ms)",
    )

    ap.add_argument(
        "--case", default="", help="Run single test by name (e.g. discovery-buttons)"
    )
    ap.add_argument(
        "--skip", nargs="*", default=[], help="Skip tests by name (e.g. --skip counter game-ui)"
    )

    args = ap.parse_args()

    # Resolve paths
    repo_root = Path(__file__).resolve().parents[2]
    preact_dir = (repo_root / args.preact_dir).resolve()
    out_dir = (repo_root / args.out_dir).resolve()
    bin_dir = (repo_root / args.bin_dir).resolve()

    out_dir.mkdir(parents=True, exist_ok=True)

    # Find required binaries and scripts
    html_render_test = find_binary(bin_dir, "html_render_test")
    if not html_render_test:
        print(
            f"ERROR: html_render_test not found in {bin_dir}\n"
            f"Make sure to run 'zig build' first",
            file=sys.stderr,
        )
        return 2

    baseline_script = repo_root / "scripts/tools/html_baseline_render.py"
    vl_multi_script = repo_root / "scripts/tools/vl_tool_multi.py"

    if not baseline_script.exists() or not vl_multi_script.exists():
        print(
            f"ERROR: Required scripts not found\n"
            f"  baseline: {baseline_script}\n"
            f"  vl_multi: {vl_multi_script}",
            file=sys.stderr,
        )
        return 2

    # Find preact tests
    all_tests = find_preact_tests(preact_dir)

    if not all_tests:
        print(
            f"ERROR: No preact tests found in {preact_dir}\n"
            f"Expected directories: preact-*/index.html",
            file=sys.stderr,
        )
        return 3

    # Filter tests
    tests_to_run = all_tests
    if args.case:
        if args.case not in all_tests:
            print(f"ERROR: Test '{args.case}' not found", file=sys.stderr)
            return 3
        tests_to_run = {args.case: all_tests[args.case]}

    if args.skip:
        skip_set = set(args.skip)
        tests_to_run = {k: v for k, v in tests_to_run.items() if k not in skip_set}

    print(f"\nFound {len(tests_to_run)} preact test(s) to run")
    print(f"Output directory: {out_dir}")
    print()

    failed = []
    passed = []

    for test_name in sorted(tests_to_run.keys()):
        html_path = tests_to_run[test_name]
        case_dir = out_dir / test_name
        case_dir.mkdir(parents=True, exist_ok=True)

        try:
            success = _run_single_preact_case(
                test_name,
                html_path,
                case_dir,
                args,
                html_render_test,
                baseline_script,
                vl_multi_script,
                bin_dir,
            )
            if success:
                passed.append(test_name)
            else:
                failed.append(test_name)
        except Exception as e:
            print(f"\n      ✗ Exception: {e}")
            failed.append(test_name)

    # Final summary
    print(f"\n{'=' * 60}")
    print(f"SUMMARY")
    print(f"{'=' * 60}")
    print(f"Passed: {len(passed)}/{len(tests_to_run)}")
    if passed:
        print(f"  ✓ {', '.join(passed)}")
    if failed:
        print(f"Failed: {len(failed)}/{len(tests_to_run)}")
        print(f"  ✗ {', '.join(failed)}")

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
