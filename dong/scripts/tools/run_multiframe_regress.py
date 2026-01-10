import argparse
import json
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
    ap = argparse.ArgumentParser(description="Batch-run dong html_render_test in multi-frame mode and check intra-run determinism.")
    ap.add_argument("--bin-dir", default="zig-out/bin", help="Directory containing html_render_test.exe (default: zig-out/bin)")
    ap.add_argument("--tests-dir", default="zig-out/bin/data/tests", help="Directory containing test HTML files")
    ap.add_argument("--out-dir", default="zig-out/tmp/multiframe", help="Output directory")
    ap.add_argument("--width", type=int, default=800)
    ap.add_argument("--height", type=int, default=600)
    ap.add_argument("--frames", type=int, default=60)
    ap.add_argument("--frame-ms", type=int, default=0)
    ap.add_argument("--update", action="store_true", help="Call dong_view_update() between frames")
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

    vl_tool = (repo_root / "scripts/tools/vl_tool_multi.py").resolve()

    html_files = sorted(tests_dir.glob("*.html"))
    if not html_files:
        print(f"ERROR: no html tests found in {tests_dir}", file=sys.stderr)
        return 3

    summary = []

    for html in html_files:
        stem = html.stem
        case_dir = out_dir / stem
        case_dir.mkdir(parents=True, exist_ok=True)

        # Place frames under case_dir
        out_base = str(case_dir / f"{stem}.bmp")

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

        print(f"[RUN] {stem} frames={args.frames} update={args.update}")
        run(cmd, cwd=str(bin_dir))

        base = case_dir / f"{stem}_f{str(0).zfill(len(str(args.frames - 1)))}.bmp"
        glob_pat = str(case_dir / f"{stem}_f*.bmp")

        report_json = case_dir / f"{stem}_nondet.json"
        merged_png = case_dir / f"{stem}_nondet.png"

        cmd2 = [
            sys.executable,
            str(vl_tool),
            str(html),
            "--base",
            str(base),
            "--glob",
            glob_pat,
            "--out-image",
            str(merged_png),
            "--out-json",
            str(report_json),
            "--no-llm",
        ]
        run(cmd2)

        data = json.loads(report_json.read_text(encoding="utf-8"))
        diff_frames = [f for f in data.get("frames", []) if f.get("diff_present")]
        summary.append({"case": stem, "diff_frames": len(diff_frames), "report": str(report_json)})

        if diff_frames:
            print(f"  -> NONDETERMINISTIC: {len(diff_frames)} frame(s) differ. See {report_json}")
        else:
            print("  -> OK: all frames identical")

    summary_path = out_dir / "summary.json"
    summary_path.write_text(json.dumps(summary, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"\n[SUMMARY] {summary_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
