import argparse
import subprocess
import sys
from pathlib import Path


def run(cmd, cwd=None):
    p = subprocess.run(cmd, cwd=cwd, shell=False, capture_output=True, text=True)
    if p.returncode != 0:
        sys.stderr.write(p.stdout)
        sys.stderr.write(p.stderr)
        raise RuntimeError(f"Command failed ({p.returncode}): {' '.join(cmd)}")
    return p.stdout


def main() -> int:
    ap = argparse.ArgumentParser(description="Render browser baseline + dong multi-frame outputs, then build a merged diff image + JSON report.")
    ap.add_argument("--bin-dir", default="zig-out/bin", help="Directory containing html_render_test.exe")
    ap.add_argument("--tests-dir", default="zig-out/bin/data/tests", help="Directory containing *.html")
    ap.add_argument("--out-dir", default="zig-out/tmp/baseline_compare", help="Output directory")
    ap.add_argument("--width", type=int, default=800)
    ap.add_argument("--height", type=int, default=600)
    ap.add_argument("--frames", type=int, default=60)
    ap.add_argument("--frame-ms", type=int, default=16)
    ap.add_argument("--update", action="store_true", help="Call dong_view_update() between frames")
    ap.add_argument("--llm", action="store_true", help="Enable LLM analysis in vl_tool_multi.py")
    ap.add_argument("--case", default="", help="Only run one case by stem (e.g. cursor_test)")
    args = ap.parse_args()

    repo_root = Path(__file__).resolve().parents[2]
    bin_dir = (repo_root / args.bin_dir).resolve()
    tests_dir = (repo_root / args.tests_dir).resolve()
    out_dir = (repo_root / args.out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    exe = bin_dir / "html_render_test.exe"
    if not exe.exists():
        print(f"ERROR: not found: {exe}", file=sys.stderr)
        return 2

    baseline = (repo_root / "scripts/tools/html_baseline_render.py").resolve()
    vl_multi = (repo_root / "scripts/tools/vl_tool_multi.py").resolve()

    html_files = sorted(tests_dir.glob("*.html"))
    if args.case:
        html_files = [p for p in html_files if p.stem == args.case]

    if not html_files:
        print("ERROR: no html tests selected", file=sys.stderr)
        return 3

    for html in html_files:
        stem = html.stem
        case_dir = out_dir / stem
        case_dir.mkdir(parents=True, exist_ok=True)

        base_png = case_dir / f"{stem}_base.png"
        print(f"[BASE] {stem} -> {base_png}")
        run([
            sys.executable,
            str(baseline),
            str(html),
            "--out",
            str(base_png),
            "--width",
            str(args.width),
            "--height",
            str(args.height),
            "--wait-ms",
            "50",
        ])

        # Dong frames
        out_base = str(case_dir / f"{stem}.bmp")
        print(f"[DONG] {stem} frames={args.frames} -> {case_dir}")
        cmd = [
            str(exe),
            str(html),
            out_base,
            str(args.width),
            str(args.height),
            str(args.frames),
            "--frame-ms",
            str(args.frame_ms),
        ]
        if not args.update:
            cmd.append("--no-update")
        run(cmd, cwd=str(bin_dir))

        # Merge + report
        pad = len(str(args.frames - 1))
        base_frame = case_dir / f"{stem}_f{str(0).zfill(pad)}.bmp"
        merged = case_dir / f"{stem}_merged.png"
        report = case_dir / f"{stem}_report.json"
        print(f"[MERGE] {stem} -> {merged}")

        cmd2 = [
            sys.executable,
            str(vl_multi),
            str(html),
            "--base",
            str(base_png),
            "--glob",
            str(case_dir / f"{stem}_f*.bmp"),
            "--out-image",
            str(merged),
            "--out-json",
            str(report),
        ]
        if not args.llm:
            cmd2.append("--no-llm")
        run(cmd2)

        print(f"[DONE] {stem}: {report}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
