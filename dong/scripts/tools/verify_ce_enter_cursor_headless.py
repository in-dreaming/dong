#!/usr/bin/env python3
"""
Headless regression for contenteditable:
1) Clicking inside CE after underline should place caret and Enter should insert a paragraph (<br>).
2) Clicking outside CE should not place a caret (so Enter should not change editor line count).

This script is intended to be run from the repo (see --run):
  cd dong
  python scripts/tools/verify_ce_enter_cursor_headless.py --run
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from typing import List, Tuple, Optional


def row_ink_counts(path: str, x0: int, x1: int, y0: int, y1: int, rgb_sum_thr: int = 720) -> List[int]:
    from PIL import Image

    img = Image.open(path).convert("RGB")
    w, h = img.size
    out: List[int] = []
    for y in range(y0, min(y1, h)):
        cnt = 0
        for x in range(x0, min(x1, w)):
            r, g, b = img.getpixel((x, y))
            if r + g + b < rgb_sum_thr:
                cnt += 1
        out.append(cnt)
    return out


def count_strong_line_groups(inks: List[int], *, min_ink: int, min_gap_rows: int = 5) -> int:
    """
    Approximate number of text lines by grouping consecutive "strong" rows.
    """
    strong = [v >= min_ink for v in inks]
    groups: List[Tuple[int, int]] = []

    i = 0
    while i < len(strong):
        if not strong[i]:
            i += 1
            continue
        start = i
        while i < len(strong) and strong[i]:
            i += 1
        end = i  # exclusive
        groups.append((start, end))

    # Merge groups that are too close (noise).
    merged: List[Tuple[int, int]] = []
    for (s, e) in groups:
        if not merged:
            merged.append((s, e))
            continue
        ps, pe = merged[-1]
        if s - pe <= min_gap_rows:
            merged[-1] = (ps, e)
        else:
            merged.append((s, e))
    return len(merged)

def sum_ink_rows(inks: List[int], *, ink_row_thr: int) -> int:
    return sum(v for v in inks if v >= ink_row_thr)


def run_case(*, name: str, click: str, key: str, key_frame: int) -> Tuple[str, str, str]:
    dong_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    html = os.path.join(dong_root, "examples", "data", "tests", "test_contenteditable_basic.html")
    out = os.path.join(dong_root, "zig-out", "tmp", f"ce_enter_cursor_{name}.bmp")

    env = os.environ.copy()
    # frame indices: after frame0 snippet runs, we inject clicks on frame 1 and keys on frame key_frame
    env["DONG_TEST_CLICK_FRAME"] = "1"
    env["DONG_TEST_CLICKS"] = click
    env["DONG_TEST_KEY_FRAME"] = str(key_frame)
    env["DONG_TEST_KEYS"] = key

    cmd = [
        "zig",
        "build",
        "run-html-test",
        "--",
        html,
        out,
        "800",
        "600",
        "3",
        "--eval-after-frame0-file",
        "snippets/ce_underline_try_typing_after_frame0.js",
    ]

    print("+", " ".join(cmd))
    # Capture logs: we rely on log-based assertions for CE correctness.
    r = subprocess.run(cmd, cwd=dong_root, env=env, capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError(f"run-html-test failed for case {name} rc={r.returncode}")

    # run-html-test writes <stem>_f0.bmp, <stem>_f1.bmp, <stem>_f2.bmp for frames=3.
    stem, _ = os.path.splitext(out)
    f2 = f"{stem}_f2.bmp"
    f1 = f"{stem}_f1.bmp"
    if not os.path.isfile(f1):
        raise FileNotFoundError(f"expected BMP not found: {f1}")
    if not os.path.isfile(f2):
        raise FileNotFoundError(f"expected BMP not found: {f2}")
    log_text = (r.stdout or "") + "\n" + (r.stderr or "")
    return f1, f2, log_text


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--run", action="store_true", help="Run headless render tests first")
    args = ap.parse_args()

    dong_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

    # Content column inside #editor for test_contenteditable_basic @ 800x600.
    # Use a wider y-range because the inserted <br> line can shift depending on caret offset.
    x0, x1 = 80, 650
    y0, y1 = 160, 320

    mid_f1, mid_f2, mid_log = run_case(name="mid", click="320,180,1", key="13", key_frame=2)
    blank_f1, blank_f2, blank_log = run_case(name="blank", click="120,230,1", key="13", key_frame=2)
    outside_f1, outside_f2, outside_log = run_case(name="outside", click="20,40,1", key="13", key_frame=2)

    def count_insert_paragraph(log_text: str) -> int:
        return log_text.count("[insertParagraph]")

    ins_mid = count_insert_paragraph(mid_log)
    ins_blank = count_insert_paragraph(blank_log)
    ins_outside = count_insert_paragraph(outside_log)

    # print a small evidence snippet (helps debugging without spamming full logs).
    def evidence_snippet(log_text: str) -> str:
        idx = log_text.find("[insertParagraph]")
        if idx == -1:
            return "(no insertParagraph)"
        start = max(0, idx - 120)
        end = min(len(log_text), idx + 240)
        return log_text[start:end].replace("\n", " ")

    print(f"[Case mid] f2={mid_f2} insertParagraph_count={ins_mid} evidence={evidence_snippet(mid_log)}")
    print(f"[Case blank] f2={blank_f2} insertParagraph_count={ins_blank} evidence={evidence_snippet(blank_log)}")
    print(f"[Case outside] f2={outside_f2} insertParagraph_count={ins_outside} evidence={evidence_snippet(outside_log)}")

    ok = True
    if ins_mid < 1:
        print("FAIL: expected Enter to insert a paragraph when caret was inside underline", file=sys.stderr)
        ok = False
    if ins_blank < 1:
        print("FAIL: expected Enter to insert a paragraph when caret was inside editor blank area", file=sys.stderr)
        ok = False
    if ins_outside != 0:
        print("FAIL: expected no insertParagraph after Enter outside editor", file=sys.stderr)
        ok = False

    if ok:
        print("OK: CE Enter + caret behavior matches headless expectations.")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())

