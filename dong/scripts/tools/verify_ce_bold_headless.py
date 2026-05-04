#!/usr/bin/env python3
"""
Headless regression for contenteditable bold (no extra empty line inside #editor).

Prerequisite: run from repo (see --run), or pass --bmp to an existing frame-1 capture.

  cd dong
  zig build run-html-test -- examples/data/tests/test_contenteditable_basic.html \\
    zig-out/tmp/ce_regress.bmp 800 600 2 \\
    --eval-after-frame0-file snippets/ce_bold_after_frame0.js --stitch-horizontal

  python scripts/tools/verify_ce_bold_headless.py --bmp zig-out/tmp/ce_regress_f1.bmp
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys


def row_ink_counts(path: str, x0: int, x1: int, y0: int, y1: int, rgb_sum_thr: int = 720) -> list[int]:
    from PIL import Image

    img = Image.open(path).convert("RGB")
    w, h = img.size
    out: list[int] = []
    for y in range(y0, min(y1, h)):
        cnt = 0
        for x in range(x0, min(x1, w)):
            r, g, b = img.getpixel((x, y))
            if r + g + b < rgb_sum_thr:
                cnt += 1
        out.append(cnt)
    return out


def empty_line_sandwich(inks: list[int], *, min_ink: int, hole_rows: int) -> bool:
    """True if there is a vertical run of weak rows with strong ink above and below."""
    n = len(inks)
    if n <= hole_rows:
        return False
    for i in range(n - hole_rows):
        window = inks[i : i + hole_rows]
        if max(window) >= min_ink:
            continue
        above = max(inks[:i]) if i else 0
        below = max(inks[i + hole_rows :]) if i + hole_rows < n else 0
        if above >= min_ink and below >= min_ink:
            return True
    return False


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--bmp",
        default="zig-out/tmp/ce_regress_f1.bmp",
        help="Post-bold frame (e.g. *_f1.bmp from html_render_test)",
    )
    ap.add_argument(
        "--run",
        action="store_true",
        help="Run zig build run-html-test first (cwd must be dong/)",
    )
    ap.add_argument(
        "--snippet",
        default="snippets/ce_bold_after_frame0.js",
        help="Eval snippet path (relative to tests/ HTML dir)",
    )
    args = ap.parse_args()

    dong_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    if args.run:
        html = os.path.join(dong_root, "examples", "data", "tests", "test_contenteditable_basic.html")
        out = os.path.join(dong_root, "zig-out", "tmp", "ce_regress.bmp")
        cmd = [
            "zig",
            "build",
            "run-html-test",
            "--",
            html,
            out,
            "800",
            "600",
            "2",
            "--eval-after-frame0-file",
            args.snippet,
            "--stitch-horizontal",
        ]
        print("+", " ".join(cmd))
        r = subprocess.run(cmd, cwd=dong_root)
        if r.returncode != 0:
            return r.returncode

    bmp = args.bmp if os.path.isabs(args.bmp) else os.path.join(dong_root, args.bmp)
    if not os.path.isfile(bmp):
        print("ERROR: BMP not found:", bmp, file=sys.stderr)
        return 2

    # Content column inside #editor for test_contenteditable_basic @ 800x600 (away from side borders).
    x0, x1, y0, y1 = 100, 600, 175, 228
    inks = row_ink_counts(bmp, x0, x1, y0, y1)
    bad = empty_line_sandwich(inks, min_ink=40, hole_rows=10)
    if bad:
        print("FAIL: suspected empty line inside editor region:", bmp)
        return 1
    print("OK: no empty-line sandwich in editor region:", bmp)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ImportError as e:
        if "PIL" in str(e):
            print("ERROR: pip install pillow", file=sys.stderr)
            raise SystemExit(3) from e
        raise
