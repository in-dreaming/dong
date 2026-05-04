#!/usr/bin/env python3
"""
P0-1 / Slug 回归矩阵：打印建议的环境变量组合与可选的 html_render_test 命令。

用法（仓库根或 dong/ 下）:
  python dong/scripts/tools/uber_quad_slug_matrix.py
  python dong/scripts/tools/uber_quad_slug_matrix.py --html examples/data/tests/test_text_shadow_test.html
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument(
        "--html",
        help="若指定，则在 dong/ 下尝试执行: zig build run-html-test -- <path>",
    )
    args = p.parse_args()

    combos = [
        ("1", "auto"),
        ("1", "slug"),
        ("1", "msdf"),
        ("0", "slug"),
    ]

    print("=== Uber Quad (DONG_DONT_USE_UBER_QUAD) x Text renderer (DONG_TEXT_RENDERER) ===")
    print("执行前在 shell 中设置变量（Windows cmd 示例）:")
    for u, t in combos:
        print(f"  set DONG_DONT_USE_UBER_QUAD={u}")
        print(f"  set DONG_TEXT_RENDERER={t}")
        print("  rem 然后运行 dong_app / feature tests / baseline 对比")
        print()

    if args.html:
        dong_dir = os.path.join(os.path.dirname(__file__), "..", "..")
        dong_dir = os.path.normpath(dong_dir)
        cmd = ["zig", "build", "run-html-test", "--", args.html]
        print("Running:", " ".join(cmd), f"(cwd={dong_dir})")
        r = subprocess.call(cmd, cwd=dong_dir)
        return int(r)

    return 0


if __name__ == "__main__":
    sys.exit(main())
