#!/usr/bin/env python3
"""Smoke test for native GPU backend (no dong_sdl_backend)."""
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


def main() -> int:
    root = Path(__file__).resolve().parents[2]
    bin_dir = root / "build-cmake-gpu"
    if not bin_dir.is_dir():
        bin_dir = root / "zig-out" / "bin"
    if not bin_dir.is_dir():
        print("ERROR: no build output directory", file=sys.stderr)
        return 1

    dll_paths = [
        bin_dir,
        bin_dir / "backends" / "gpu",
        bin_dir / "appcore",
        root / "zig-out" / "sdl3" / "lib",
        root / "zig-out" / "bin",
    ]
    env = os.environ.copy()
    env["PATH"] = ";".join(str(p) for p in dll_paths if p.exists()) + ";" + env.get("PATH", "")
    env["DONG_AUTO_QUIT"] = env.get("DONG_AUTO_QUIT", "120")

    if (bin_dir / "dong_sdl_backend.dll").exists():
        print(f"FAIL: dong_sdl_backend.dll in {bin_dir}", file=sys.stderr)
        return 1

    gpu_backend = bin_dir / "backends" / "gpu" / "dong_gpu_backend.dll"
    if not gpu_backend.exists():
        gpu_backend = bin_dir / "dong_gpu_backend.dll"
    if not gpu_backend.exists():
        print("WARN: dong_gpu_backend.dll not found (static link?)", file=sys.stderr)

    html = root / "examples" / "data" / "gamelikeui" / "game_ui_offline.html"
    if not html.exists():
        print(f"WARN: missing {html}", file=sys.stderr)

    tests: list[tuple[str, list[str]]] = []
    dong_app = bin_dir / ("dong_app.exe" if sys.platform == "win32" else "dong_app")
    if dong_app.exists() and html.exists():
        tests.append(("dong_app", [str(dong_app), "--html", str(html)]))

    screens = bin_dir / ("3d_screens_simple.exe" if sys.platform == "win32" else "3d_screens_simple")
    if screens.exists():
        tests.append(("3d_screens_simple", [str(screens)]))

    # html_render_test optional
    hrt = bin_dir / ("html_render_test.exe" if sys.platform == "win32" else "html_render_test")
    if hrt.exists() and html.exists():
        out_bmp = bin_dir / "_smoke_gpu.bmp"
        tests.append(
            (
                "html_render_test",
                [str(hrt), str(html), str(out_bmp), "400", "300"],
            )
        )

    if not tests:
        print("ERROR: no executables to run", file=sys.stderr)
        return 1

    failed = 0
    for name, cmd in tests:
        print(f"[smoke] {name}: {' '.join(cmd)}")
        r = subprocess.run(cmd, env=env, cwd=bin_dir)
        if r.returncode != 0:
            print(f"FAIL: {name} exit {r.returncode}", file=sys.stderr)
            failed += 1
        else:
            print(f"OK: {name}")

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
