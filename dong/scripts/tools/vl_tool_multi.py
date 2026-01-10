import argparse
import base64
import json
import os
import re
import sys
import glob
from pathlib import Path


def _get_image_mime_type(image_path: str) -> str:
    ext = image_path.lower().split(".")[-1]
    return {
        "png": "image/png",
        "jpg": "image/jpeg",
        "jpeg": "image/jpeg",
        "gif": "image/gif",
        "webp": "image/webp",
        "bmp": "image/bmp",
    }.get(ext, "image/png")


def _encode_image_to_base64(image_path: str):
    with open(image_path, "rb") as f:
        data = base64.b64encode(f.read()).decode("utf-8")
    return data, _get_image_mime_type(image_path)


def _read_text(path: str) -> str:
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def _extract_json_from_text(s: str):
    # try fenced json first
    m = re.search(r"```(?:json)?\s*(\{.*?\})\s*```", s, re.DOTALL)
    if m:
        s = m.group(1)
    else:
        m = re.search(r"\{.*\}", s, re.DOTALL)
        if m:
            s = m.group(0)
    return json.loads(s)


def _llm_analyze(image_path: str, html_string: str, prompt_extra: str, model: str, endpoint: str, api_key: str, debug: bool):
    import requests

    b64, mime = _encode_image_to_base64(image_path)

    system_prompt = (
        "你是一位专业的 web UI 渲染对比分析助手。\n"
        "你会收到：1) 一张合并后的对比大图（包含 baseline 和多帧渲染结果）；2) 原始 HTML。\n"
        "请逐帧比较每一帧与 baseline 的差异，并推测差异可能来源（例如帧同步/缓存/混合/文本栅格化/资源未绑定等）。\n"
        "输出必须是 JSON 字符串。"
    )

    user_text = (
        "图片布局说明：每一行对应一个 frame。三列分别是：BASE、FRAME_i、DIFF(BASE vs FRAME_i)。\n"
        "DIFF 列越亮代表差异越大。\n\n"
        "任务：\n"
        "1) 逐行分析每一帧相对 baseline 的差异（位置、尺寸、颜色、边框/圆角、文字、阴影等）。\n"
        "2) 判断差异是‘稳定差异’还是‘闪烁/低频不一致’（如果有的帧对、有的帧错）。\n"
        "3) 给出最可能的 1-3 个根因方向，并指出需要在引擎里重点排查的模块。\n\n"
        "要求输出 JSON：\n"
        "{\n"
        '  "overall_severity": "CRITICAL"|"WARNING"|"INFO"|"NONE",\n'
        '  "summary": "...",\n'
        '  "frames": [\n'
        "    {\n"
        '      "frame_index": 0,\n'
        '      "diff_present": true|false,\n'
        '      "diff_description": "...",\n'
        '      "likely_category": ["sync", "pipeline_state", "resource_lifetime", "text", "raster", "layout", "other"]\n'
        "    }\n"
        "  ],\n"
        '  "likely_root_causes": ["..."],\n'
        '  "engine_checklist": ["..." ]\n'
        "}\n\n"
        + (prompt_extra or "")
    )

    payload = {
        "model": model,
        "messages": [
            {"role": "system", "content": system_prompt},
            {
                "role": "user",
                "content": [
                    {"type": "text", "text": user_text},
                    {"type": "image_url", "image_url": {"url": f"data:{mime};base64,{b64}"}},
                    {"type": "text", "text": f"HTML String:\n```html\n{html_string}\n```"},
                ],
            },
        ],
        "max_tokens": 1200,
    }

    headers = {"Content-Type": "application/json", "Authorization": f"Bearer {api_key}"}

    if debug:
        payload_size = len(json.dumps(payload).encode("utf-8"))
        print(f"[DEBUG] payload={payload_size/1024:.1f}KB image_b64={len(b64)/1024:.1f}KB model={model}", file=sys.stderr)

    r = requests.post(endpoint, headers=headers, json=payload, timeout=60)
    r.raise_for_status()
    j = r.json()
    content = j["choices"][0]["message"]["content"]
    if debug:
        print(f"[DEBUG] raw_response=\n{content}\n", file=sys.stderr)
    return _extract_json_from_text(content)


def main() -> int:
    ap = argparse.ArgumentParser(description="Merge baseline + multi-frame renders into one image and analyze per-frame diffs.")
    ap.add_argument("html", help="Path to HTML file")
    ap.add_argument("--base", required=True, help="Baseline image path (from browser renderer)")

    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--frames", nargs="*", help="Frame image paths (space-separated)")
    g.add_argument("--glob", help="Glob pattern for frame images, e.g. 'zig-out/tmp/tests/cursor_test_multi_f*.bmp'")

    ap.add_argument("--out-image", default="vl_multi.png", help="Output merged image (default: vl_multi.png)")
    ap.add_argument("--out-json", default="vl_multi.json", help="Output report JSON (default: vl_multi.json)")
    ap.add_argument("--max-side", type=int, default=4096, help="Downscale merged image so max(width,height)<=max-side (default: 4096)")

    ap.add_argument("--no-llm", action="store_true", help="Only compute bbox diffs and write merged image")
    ap.add_argument("--model", default=os.environ.get("OPENROUTER_MODEL", "x-ai/grok-4.1-fast"))
    ap.add_argument("--endpoint", default=os.environ.get("OPENROUTER_ENDPOINT", "https://openrouter.ai/api/v1/chat/completions"))
    ap.add_argument("--api-key", default=os.environ.get("OPENROUTER_API_KEY") or os.environ.get("OPENAI_API_KEY"))
    ap.add_argument("--prompt-extra", default="", help="Extra text appended to the LLM prompt")
    ap.add_argument("--debug", action="store_true")
    args = ap.parse_args()

    try:
        from PIL import Image, ImageChops, ImageDraw
    except Exception as e:
        print("ERROR: Pillow is required (pip install pillow).", file=sys.stderr)
        print(f"Details: {e}", file=sys.stderr)
        return 3

    html_string = _read_text(args.html)

    frame_paths = []
    if args.glob:
        frame_paths = sorted(glob.glob(args.glob))
    else:
        frame_paths = args.frames or []

    if not frame_paths:
        print("ERROR: no frame images found", file=sys.stderr)
        return 2

    base_img = Image.open(args.base).convert("RGBA")
    base_rgb = base_img.convert("RGB")
    w, h = base_img.size

    frames = []
    for p in frame_paths:
        img = Image.open(p).convert("RGBA")
        if img.size != (w, h):
            img = img.resize((w, h))
        frames.append((p, img))

    # Build per-frame diff info + diff images.
    # NOTE: For RGBA images, Pillow's ImageChops.difference often produces a diff image with alpha=0,
    # and Image.getbbox() will then incorrectly return None even if RGB differs.
    # So we compute bbox on RGB-only diff.
    diffs = []
    for p, img in frames:
        img_rgb = img.convert("RGB")
        d_rgb = ImageChops.difference(base_rgb, img_rgb)
        bbox = d_rgb.getbbox()  # None if identical
        # enhance diff visibility
        d_vis = ImageChops.multiply(d_rgb.convert("RGBA"), Image.new("RGBA", (w, h), (4, 4, 4, 255)))
        diffs.append((p, bbox, d_vis))


    # Compose: each row = BASE | FRAME_i | DIFF
    pad = 8
    cols = 3
    out_w = cols * w + (cols + 1) * pad
    out_h = len(frames) * h + (len(frames) + 1) * pad

    canvas = Image.new("RGBA", (out_w, out_h), (20, 20, 20, 255))
    draw = ImageDraw.Draw(canvas)

    for i, ((frame_path, frame_img), (_, bbox, diff_img)) in enumerate(zip(frames, diffs)):
        y0 = pad + i * (h + pad)
        x_base = pad
        x_frame = pad * 2 + w
        x_diff = pad * 3 + w * 2

        canvas.alpha_composite(base_img, (x_base, y0))
        canvas.alpha_composite(frame_img, (x_frame, y0))
        canvas.alpha_composite(diff_img, (x_diff, y0))

        label = f"frame={i}"
        if bbox is None:
            label += " diff=NONE"
        else:
            label += f" diff=bbox{bbox}"
        # draw label once per row
        draw.text((x_base, y0 - pad + 1), label, fill=(230, 230, 230, 255))

    # Downscale if needed
    max_side = max(canvas.size)
    if args.max_side and max_side > args.max_side:
        scale = args.max_side / max_side
        new_size = (max(1, int(canvas.size[0] * scale)), max(1, int(canvas.size[1] * scale)))
        canvas = canvas.resize(new_size)

    out_image = Path(args.out_image)
    out_image.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(out_image)

    report = {
        "html": str(args.html),
        "base": str(args.base),
        "frames": [
            {
                "frame_index": i,
                "path": p,
                "diff_bbox": bbox,
                "diff_present": bbox is not None,
            }
            for i, (p, bbox, _) in enumerate(diffs)
        ],
        "merged_image": str(out_image),
    }

    if not args.no_llm:
        if not args.api_key:
            print("ERROR: missing API key (set OPENROUTER_API_KEY or pass --api-key)", file=sys.stderr)
            return 4
        try:
            llm = _llm_analyze(
                image_path=str(out_image),
                html_string=html_string,
                prompt_extra=args.prompt_extra,
                model=args.model,
                endpoint=args.endpoint,
                api_key=args.api_key,
                debug=args.debug,
            )
            report["llm"] = llm
        except Exception as e:
            report["llm_error"] = str(e)

    out_json = Path(args.out_json)
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")

    print(str(out_json))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
