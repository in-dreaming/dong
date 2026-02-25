import argparse
import os
import subprocess
import sys
from pathlib import Path


def run(cmd, cwd=None, env=None):
    merged_env = os.environ.copy()
    if env:
        merged_env.update(env)
    p = subprocess.run(cmd, cwd=cwd, env=merged_env, shell=False, capture_output=True, text=True)
    if p.returncode != 0:
        sys.stderr.write(p.stdout)
        sys.stderr.write(p.stderr)
        raise RuntimeError(f"Command failed ({p.returncode}): {' '.join(cmd)}")
    return p.stdout


def find_html_render_test(bin_dir: Path) -> Path | None:
    for name in ("html_render_test", "html_render_test.exe"):
        p = bin_dir / name
        if p.exists():
            return p
    return None


def _run_single_case(stem, html, case_dir, args, exe, baseline, vl_multi, bin_dir):
    """Run baseline + dong render + merge for a single test case."""
    # Clean old outputs
    for p in case_dir.glob(f"{stem}_f*.bmp"):
        try:
            p.unlink()
        except OSError:
            pass
    for p in [case_dir / f"{stem}_merged.png", case_dir / f"{stem}_report.json", case_dir / f"{stem}_base.png"]:
        try:
            if p.exists():
                p.unlink()
        except OSError:
            pass

    # Browser baseline
    base_png = case_dir / f"{stem}_base.png"
    print(f"[BASE] {stem} -> {base_png}")
    cmd_base = [
        sys.executable, str(baseline), str(html),
        "--out", str(base_png),
        "--width", str(args.width),
        "--height", str(args.height),
        "--wait-ms", str(args.base_wait_ms),
    ]
    if args.click:
        cmd_base += ["--click", args.click,
                     "--post-click-wait-ms", str(args.base_post_click_wait_ms)]
    run(cmd_base)

    # Dong frames
    out_base = str(case_dir / f"{stem}.bmp")
    print(f"[DONG] {stem} frames={args.frames} -> {case_dir}")
    cmd = ["/usr/bin/env", "DONG_DISABLE_SCROLLBARS=1"]
    click_xy = args.dong_click or args.click
    if click_xy:
        cmd.append(f"DONG_TEST_CLICK={click_xy},1")
    cmd += [str(exe), str(html), out_base,
            str(args.width), str(args.height), str(args.frames),
            "--frame-ms", str(args.frame_ms)]
    if not args.update:
        cmd.append("--no-update")
    run(cmd, cwd=str(bin_dir))

    # Merge + report
    merged = case_dir / f"{stem}_merged.png"
    report = case_dir / f"{stem}_report.json"
    print(f"[MERGE] {stem} -> {merged}")
    cmd2 = [sys.executable, str(vl_multi), str(html),
            "--base", str(base_png)]
    if args.frames <= 1:
        cmd2 += ["--frames", out_base]
    else:
        cmd2 += ["--glob", str(case_dir / f"{stem}_f*.bmp")]
    cmd2 += ["--out-image", str(merged), "--out-json", str(report)]
    if not args.llm:
        cmd2.append("--no-llm")
    run(cmd2)


def main() -> int:
    ap = argparse.ArgumentParser(description="Render browser baseline + dong multi-frame outputs, then build a merged diff image + JSON report.")
    ap.add_argument("--bin-dir", default="zig-out/bin", help="Directory containing html_render_test (default: zig-out/bin)")
    ap.add_argument("--tests-dir", default="zig-out/bin/data/tests", help="Directory containing *.html")
    ap.add_argument("--out-dir", default="zig-out/tmp/baseline_compare", help="Output directory")
    ap.add_argument("--width", type=int, default=800)
    ap.add_argument("--height", type=int, default=600)
    ap.add_argument("--frames", type=int, default=60)
    ap.add_argument("--frame-ms", type=int, default=16)
    ap.add_argument("--update", action="store_true", help="Call dong_view_update() between frames")
    ap.add_argument("--llm", action="store_true", help="Enable LLM analysis in vl_tool_multi.py")
    ap.add_argument("--case", default="", help="Only run one case by stem (e.g. cursor_test)")
    ap.add_argument("--skip", nargs="*", default=[], help="Skip cases by stem (e.g. --skip complex foo)")

    ap.add_argument("--click", default="", help="Optional baseline click before capture, format: x,y (e.g. 240,250)")
    ap.add_argument("--dong-click", default="", help="Optional dong click, format: x,y (button defaults to 1)")
    ap.add_argument("--base-wait-ms", type=int, default=50, help="Baseline extra wait before click/screenshot (default: 50ms)")
    ap.add_argument("--base-post-click-wait-ms", type=int, default=50, help="Baseline extra wait after click (default: 50ms)")

    args = ap.parse_args()

    repo_root = Path(__file__).resolve().parents[2]
    bin_dir = (repo_root / args.bin_dir).resolve()
    tests_dir = (repo_root / args.tests_dir).resolve()
    out_dir = (repo_root / args.out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    exe = find_html_render_test(bin_dir)
    if not exe:
        cand = [str((bin_dir / n).resolve()) for n in ("html_render_test", "html_render_test.exe")]
        print("ERROR: html_render_test binary not found. Tried:\n  " + "\n  ".join(cand), file=sys.stderr)
        return 2

    baseline = (repo_root / "scripts/tools/html_baseline_render.py").resolve()
    vl_multi = (repo_root / "scripts/tools/vl_tool_multi.py").resolve()

    html_files = sorted(tests_dir.glob("*.html"))
    if args.case:
        html_files = [p for p in html_files if p.stem == args.case]
    if args.skip:
        skip_set = set(args.skip)
        html_files = [p for p in html_files if p.stem not in skip_set]

    if not html_files:
        print("ERROR: no html tests selected", file=sys.stderr)
        return 3

    failed = []
    for html in html_files:
        stem = html.stem
        case_dir = out_dir / stem
        case_dir.mkdir(parents=True, exist_ok=True)

        try:
            _run_single_case(stem, html, case_dir, args, exe, baseline, vl_multi, bin_dir)
            print(f"[DONE] {stem}")
        except RuntimeError as e:
            print(f"[CRASH] {stem}: {e}", file=sys.stderr)
            failed.append(stem)

    if failed:
        print(f"\n[SUMMARY] {len(failed)} case(s) crashed: {', '.join(failed)}", file=sys.stderr)
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
