#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Run baseline comparisons for all Preact discovery components.
This script tests the fixed HTML baseline renderer with actual Preact components.
"""

import subprocess
import sys
from pathlib import Path
import json


def run_baseline_compare(component_name: str, width: int = 1024, height: int = 768):
    """Run baseline comparison for a single Preact component."""

    html_path = Path(f"zig-out/bin/data/{component_name}/index.html")

    if not html_path.exists():
        print(f"ERROR: {html_path} not found", file=sys.stderr)
        return False

    out_dir = Path(f"zig-out/tmp/baseline_compare/{component_name}")
    out_dir.mkdir(parents=True, exist_ok=True)

    # Generate baseline
    base_png = out_dir / "index_base.png"
    print(f"[BASE] {component_name} -> {base_png}")

    result = subprocess.run([
        sys.executable,
        "scripts/tools/html_baseline_render.py",
        str(html_path),
        "--out", str(base_png),
        "--width", str(width),
        "--height", str(height),
        "--wait-ms", "500",
    ], capture_output=True, text=True)

    if result.returncode != 0:
        print(f"[ERROR] Baseline capture failed for {component_name}:", file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        return False

    print(f"     OK: Baseline captured: {base_png}")

    # Generate Dong renders
    dong_exe = Path("zig-out/bin/html_render_test.exe")
    if not dong_exe.exists():
        dong_exe = Path("zig-out/bin/html_render_test")

    if not dong_exe.exists():
        print(f"ERROR: html_render_test binary not found", file=sys.stderr)
        return False

    print(f"[DONG] {component_name} frames=5")
    for frame_idx in range(5):
        output_bmp = out_dir / f"index_{frame_idx}.bmp"

        result = subprocess.run([
            str(dong_exe),
            str(html_path),
            str(output_bmp),
            str(width),
            str(height),
        ], capture_output=True, text=True)

        if result.returncode != 0:
            print(f"[ERROR] Frame {frame_idx} render failed:", file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return False

        print(f"     OK: Frame {frame_idx}: {output_bmp}")

    print(f"[DONE] {component_name}")
    return True


def main():
    components = [
        "preact-discovery-buttons",
        "preact-discovery-inbox",
        "preact-ui-components",
        "preact-discovery-cards",
    ]

    print("=" * 80)
    print("PREACT BASELINE COMPARISON TEST SUITE")
    print("=" * 80)
    print()

    results = {}
    for component in components:
        print(f"\nProcessing: {component}")
        print("-" * 80)
        success = run_baseline_compare(component)
        results[component] = "PASS" if success else "FAIL"

    print()
    print("=" * 80)
    print("SUMMARY")
    print("=" * 80)
    for component, status in results.items():
        status_symbol = "[OK]" if status == "PASS" else "[FAIL]"
        print(f"{status_symbol} {component}: {status}")

    all_pass = all(s == "PASS" for s in results.values())
    return 0 if all_pass else 1


if __name__ == "__main__":
    raise SystemExit(main())
