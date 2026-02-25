import argparse
import sys
from pathlib import Path


def _resolve_html_target(html: str) -> str:
    p = Path(html)
    if p.exists():
        return p.resolve().as_uri()
    # Fallback: treat input as raw HTML string
    return "data:text/html," + html


def main() -> int:
    ap = argparse.ArgumentParser(
        description=(
            "Render an HTML file with a standard browser engine (Chromium via Playwright) and save a screenshot as baseline.\n"
            "Note: this tool does NOT use dong."
        )
    )
    ap.add_argument("html", help="Path to HTML file (preferred). If path doesn't exist, treated as HTML string.")
    ap.add_argument("--out", default="baseline.png", help="Output image path (default: baseline.png)")
    ap.add_argument("--width", type=int, default=800, help="Viewport width (default: 800)")
    ap.add_argument("--height", type=int, default=600, help="Viewport height (default: 600)")
    ap.add_argument("--device-scale-factor", type=float, default=1.0, help="Device scale factor (default: 1.0)")
    ap.add_argument("--wait-ms", type=int, default=50, help="Extra wait time before screenshot (default: 50ms)")
    ap.add_argument("--timeout-ms", type=int, default=15000, help="Navigation timeout (default: 15000ms)")

    ap.add_argument(
        "--click",
        default="",
        help="Optional click before screenshot, format: x,y (e.g. 240,190)",
    )
    ap.add_argument(
        "--click-seq",
        default="",
        help="Optional click sequence before screenshot, format: x1,y1;x2,y2 (separator ';')",
    )
    ap.add_argument(
        "--post-click-wait-ms",
        type=int,
        default=50,
        help="Extra wait time after click before screenshot (default: 50ms)",
    )


    ap.add_argument("--full-page", action="store_true", help="Capture full page (default: viewport only)")
    ap.add_argument(
        "--omit-background",
        action="store_true",
        help="Make background transparent in screenshot (if supported)",
    )
    args = ap.parse_args()

    if args.width <= 0 or args.height <= 0:
        print("ERROR: invalid viewport size", file=sys.stderr)
        return 2

    try:
        from playwright.sync_api import sync_playwright
    except Exception as e:
        print("ERROR: Playwright is not available.", file=sys.stderr)
        print("Install:", file=sys.stderr)
        print("  pip install playwright", file=sys.stderr)
        print("  playwright install chromium", file=sys.stderr)
        print(f"Details: {e}", file=sys.stderr)
        return 3

    target = _resolve_html_target(args.html)
    out_path = Path(args.out)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        context = browser.new_context(
            viewport={"width": args.width, "height": args.height},
            device_scale_factor=args.device_scale_factor,
            reduced_motion="reduce",
        )
        page = context.new_page()

        # Best-effort determinism: disable animations/transitions.
        page.add_style_tag(
            content=(
                "*{animation:none !important; transition:none !important;}"
                "html,body{scrollbar-width:none;}"
            )
        )

        page.goto(target, wait_until="load", timeout=args.timeout_ms)
        if args.wait_ms > 0:
            page.wait_for_timeout(args.wait_ms)

        click_tokens = []
        if args.click_seq:
            click_tokens = [t.strip() for t in args.click_seq.split(";") if t.strip()]
        elif args.click:
            click_tokens = [args.click.strip()]

        for token in click_tokens:
            try:
                parts = [p.strip() for p in token.split(",")]
                x = int(parts[0])
                y = int(parts[1])
            except Exception:
                print("ERROR: invalid click token, expected x,y", file=sys.stderr)
                return 4

            try:
                hit = page.evaluate(
                    "(p)=>{const el=document.elementFromPoint(p.x,p.y); return el? (el.tagName+" + "'" + "#" + "'" + "+(el.id||'')+" + "'" + "." + "'" + "+(el.className||'')) : null; }",
                    {"x": x, "y": y},
                )
                print(f"[baseline_click] at=({x},{y}) hit={hit}", file=sys.stderr)
            except Exception as e:
                print(f"[baseline_click] elementFromPoint failed: {e}", file=sys.stderr)

            page.mouse.click(x, y)
            if args.post_click_wait_ms > 0:
                page.wait_for_timeout(args.post_click_wait_ms)

        if click_tokens:
            try:
                metrics = page.evaluate(
                    "() => {\n"
                    "  const ids = ['autoContainer','smoothContainer'];\n"
                    "  const out = {};\n"
                    "  for (const id of ids) {\n"
                    "    const c = document.getElementById(id);\n"
                    "    if (!c) { out[id] = null; continue; }\n"
                    "    out[id] = { scrollTop: c.scrollTop, scrollHeight: c.scrollHeight, clientHeight: c.clientHeight };\n"
                    "  }\n"
                    "  return out;\n"
                    "}"
                )
                print(f"[baseline_click] metrics={metrics}", file=sys.stderr)
            except Exception as e:
                print(f"[baseline_click] metrics eval failed: {e}", file=sys.stderr)


        screenshot_kwargs = {
            "path": str(out_path),
            "full_page": args.full_page,
        }
        # omit_background is not available in very old Playwright; guard it.
        if args.omit_background:
            screenshot_kwargs["omit_background"] = True

        page.screenshot(**screenshot_kwargs)

        context.close()
        browser.close()

    print(str(out_path))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
