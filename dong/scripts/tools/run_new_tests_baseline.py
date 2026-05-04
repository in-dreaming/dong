"""
Render browser baselines for new tests and visually compare with dong output.
Generates a summary HTML report.
"""
import subprocess, sys, os
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
TESTS_SRC = REPO / "examples" / "data" / "tests"
DONG_OUT  = REPO / "zig-out" / "tmp" / "new_tests"
BASE_OUT  = REPO / "zig-out" / "tmp" / "new_tests_baseline"
REPORT    = REPO / "zig-out" / "tmp" / "new_tests_report.html"

BASE_OUT.mkdir(parents=True, exist_ok=True)

baseline_script = REPO / "scripts" / "tools" / "html_baseline_render.py"

test_files = sorted(TESTS_SRC.glob("test_*.html"))
print(f"Found {len(test_files)} tests")

results = []

for html in test_files:
    stem = html.stem
    baseline_png = BASE_OUT / f"{stem}_baseline.png"
    dong_bmp     = DONG_OUT / f"{stem}.bmp"

    # 1. Render baseline with Playwright
    print(f"[{stem}] baseline...", end="", flush=True)
    r = subprocess.run(
        [sys.executable, str(baseline_script), str(html),
         "--out", str(baseline_png), "--width", "800", "--height", "600"],
        capture_output=True, text=True, timeout=30
    )
    if r.returncode != 0:
        print(f" FAIL (baseline): {r.stderr[:200]}")
        results.append((stem, "BASELINE_FAIL", None, None))
        continue
    print(" ok", end="", flush=True)

    # 2. Check dong render exists
    if not dong_bmp.exists():
        print(f" FAIL (no dong render)")
        results.append((stem, "DONG_MISSING", str(baseline_png), None))
        continue

    print(f" ok")
    results.append((stem, "OK", str(baseline_png), str(dong_bmp)))

# Generate HTML report
print(f"\nGenerating report: {REPORT}")
with open(REPORT, "w", encoding="utf-8") as f:
    f.write("<!DOCTYPE html><html><head><meta charset='utf-8'><title>Dong Test Report</title>\n")
    f.write("<style>body{font-family:monospace;margin:20px;} table{border-collapse:collapse;width:100%;}")
    f.write("td,th{border:1px solid #ccc;padding:8px;text-align:center;vertical-align:top;}")
    f.write("img{max-width:400px;border:1px solid #ddd;} .ok{color:green;} .fail{color:red;}")
    f.write("</style></head><body>\n")
    f.write(f"<h1>Dong Test Report ({len(results)} tests)</h1>\n")

    ok_count = sum(1 for _, s, _, _ in results if s == "OK")
    f.write(f"<p>Pass: {ok_count}/{len(results)}</p>\n")

    f.write("<table><tr><th>Test</th><th>Status</th><th>Browser Baseline</th><th>Dong Render</th></tr>\n")
    for stem, status, base_path, dong_path in results:
        cls = "ok" if status == "OK" else "fail"
        f.write(f"<tr><td>{stem}</td><td class='{cls}'>{status}</td>")
        if base_path:
            bp = base_path.replace("\\", "/")
            f.write(f"<td><img src='file:///{bp}' alt='baseline'></td>")
        else:
            f.write("<td>-</td>")
        if dong_path:
            dp = dong_path.replace("\\", "/")
            f.write(f"<td><img src='file:///{dp}' alt='dong'></td>")
        else:
            f.write("<td>-</td>")
        f.write("</tr>\n")
    f.write("</table></body></html>\n")

print(f"\n=== Results ===")
for stem, status, _, _ in results:
    mark = "PASS" if status == "OK" else "FAIL"
    print(f"  [{mark}] {stem}")

ok = sum(1 for _, s, _, _ in results if s == "OK")
fail = len(results) - ok
print(f"\nTotal: {ok} pass, {fail} fail out of {len(results)}")
print(f"Report: {REPORT}")
