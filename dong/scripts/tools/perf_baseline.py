#!/usr/bin/env python3
"""
perf_baseline.py - Performance baseline runner for Dong rendering engine.

Runs configured scenes, collects profiler traces, extracts metrics,
judges against soft/hard thresholds, and outputs structured results.

Usage:
  python scripts/tools/perf_baseline.py                     # Run all scenes
  python scripts/tools/perf_baseline.py --scene S1          # Run specific scene
  python scripts/tools/perf_baseline.py --ci                # CI mode (exit 1 on hard fail)
  python scripts/tools/perf_baseline.py --report md         # Generate markdown report
  python scripts/tools/perf_baseline.py --diff <prev.json>  # Compare with previous run

Environment:
  DONG_PERF_EXE_DIR   Override exe directory (default: zig-out/bin)
  DONG_PERF_SCENES    Override scenes YAML path
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
import time
from dataclasses import asdict, dataclass, field
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional


# ---------------------------------------------------------------------------
# YAML loading (with fallback for missing PyYAML)
# ---------------------------------------------------------------------------

def _load_yaml(path: Path) -> dict:
    """Load YAML file. Tries PyYAML first, falls back to simple hand-rolled parser."""
    text = path.read_text(encoding="utf-8")
    try:
        import yaml
        return yaml.safe_load(text)
    except ImportError:
        pass
    return _parse_simple_yaml(text)


def _parse_simple_yaml(text: str) -> dict:
    """
    Minimal YAML parser sufficient for perf_scenes.yaml structure.
    Handles: top-level keys, nested dicts, lists in [...] form, quoted strings.
    """
    result = {}
    lines = text.split("\n")
    current_top = None
    current_section = None
    current_subsection = None

    i = 0
    while i < len(lines):
        line = lines[i]
        i += 1

        # Skip comments and blank lines
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue

        # Measure indent
        indent = len(line) - len(line.lstrip())

        # Top-level key (no indent, ends with ':')
        if indent == 0 and stripped.endswith(":"):
            current_top = stripped[:-1]
            result[current_top] = {}
            current_section = None
            current_subsection = None
            continue

        if current_top is None:
            continue

        # Parse key: value
        if ":" not in stripped:
            continue

        key, _, raw_val = stripped.partition(":")
        key = key.strip()
        raw_val = raw_val.strip()

        # Determine nesting level
        if indent == 2:
            # Direct child of top-level
            current_section = key
            current_subsection = None
            if raw_val:
                result[current_top][key] = _parse_yaml_value(raw_val)
            else:
                result[current_top][key] = {}
        elif indent == 4 and current_section:
            # Child of section
            current_subsection = key
            if raw_val:
                if not isinstance(result[current_top].get(current_section), dict):
                    result[current_top][current_section] = {}
                result[current_top][current_section][key] = _parse_yaml_value(raw_val)
            else:
                if not isinstance(result[current_top].get(current_section), dict):
                    result[current_top][current_section] = {}
                result[current_top][current_section][key] = {}
        elif indent == 6 and current_section and current_subsection:
            # Child of subsection
            parent = result[current_top].get(current_section, {})
            if isinstance(parent, dict):
                sub = parent.get(current_subsection, {})
                if isinstance(sub, dict):
                    sub[key] = _parse_yaml_value(raw_val)
                    parent[current_subsection] = sub
                    result[current_top][current_section] = parent

    return result


def _parse_yaml_value(raw: str):
    """Parse a YAML scalar value."""
    # Remove trailing comment
    if " #" in raw:
        raw = raw[:raw.index(" #")].strip()

    # Quoted string
    if (raw.startswith('"') and raw.endswith('"')) or (raw.startswith("'") and raw.endswith("'")):
        return raw[1:-1]

    # List: ["a", "b", "c"]
    if raw.startswith("[") and raw.endswith("]"):
        inner = raw[1:-1].strip()
        if not inner:
            return []
        items = []
        for item in _split_list_items(inner):
            items.append(_parse_yaml_value(item.strip()))
        return items

    # Integer
    try:
        return int(raw)
    except ValueError:
        pass

    # Float
    try:
        return float(raw)
    except ValueError:
        pass

    # Boolean
    if raw.lower() in ("true", "yes"):
        return True
    if raw.lower() in ("false", "no"):
        return False

    # Null
    if raw.lower() in ("null", "~", ""):
        return None

    return raw


def _split_list_items(s: str) -> list:
    """Split comma-separated items respecting quotes."""
    items = []
    current = ""
    in_quote = None
    for ch in s:
        if ch in ('"', "'") and in_quote is None:
            in_quote = ch
            current += ch
        elif ch == in_quote:
            in_quote = None
            current += ch
        elif ch == "," and in_quote is None:
            items.append(current)
            current = ""
        else:
            current += ch
    if current.strip():
        items.append(current)
    return items


# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

@dataclass
class MetricResult:
    name: str
    value: float
    unit: str
    soft: float
    hard: float
    verdict: str  # "pass", "warn", "FAIL"
    description: str = ""


@dataclass
class SceneResult:
    scene_id: str
    scene_name: str
    track: str
    metrics: List[MetricResult] = field(default_factory=list)
    error: Optional[str] = None
    exe_path: str = ""
    duration_s: float = 0.0
    timestamp: str = ""


@dataclass
class RunResult:
    machine: str
    timestamp: str
    scenes: List[SceneResult] = field(default_factory=list)
    any_hard_fail: bool = False


# ---------------------------------------------------------------------------
# Metric extraction from traces
# ---------------------------------------------------------------------------

GPU_STATS_RE = re.compile(
    r"\[GPUStats\]\s+"
    r"frame=(?P<frame>\d+)\s+"
    r"cpu_prepare_us=(?P<cpu_prepare>-?\d+)\s+"
    r"cpu_execute_us=(?P<cpu_execute>-?\d+)\s+"
    r"uber_inst_batches=(?P<uber_inst_batches>\d+)\s+"
    r"uber_inst_instances=(?P<uber_inst_instances>\d+)\s+"
    r"uber_uniform_draws=(?P<uber_uniform_draws>\d+)\s+"
    r"uber_gpu_draws_total=(?P<uber_gpu_draws_total>\d+)"
)


def extract_fps_from_trace(trace_path: Path, run_ms: int) -> Optional[float]:
    """Extract FPS by counting frame scopes in Chrome trace."""
    if not trace_path.exists():
        return None

    try:
        data = json.loads(trace_path.read_text(encoding="utf-8"))
        events = data.get("traceEvents") or []
    except (json.JSONDecodeError, KeyError):
        return None

    # Count "Frame" B/E pairs or "Frame" X events
    frame_count = 0
    frame_starts = []
    frame_ends = []

    for ev in events:
        name = ev.get("name", "")
        ph = ev.get("ph", "")

        # Match various frame scope names
        if name in ("Frame", "frame", "Engine::tick", "tick"):
            if ph == "B":
                frame_starts.append(ev.get("ts", 0))
            elif ph == "E":
                frame_ends.append(ev.get("ts", 0))
            elif ph == "X":
                frame_count += 1

    # Use B/E pairs if we have them
    if frame_starts and frame_ends:
        frame_count = min(len(frame_starts), len(frame_ends))

    if frame_count == 0:
        # Fallback: count any top-level B events as approximate frame count
        return None

    # Calculate FPS: frames / run_duration
    # Try to get actual duration from first to last frame timestamp
    if frame_starts and frame_ends and len(frame_ends) >= 2:
        duration_us = frame_ends[-1] - frame_starts[0]
        if duration_us > 0:
            return frame_count / (duration_us / 1_000_000.0)

    # Fallback: use configured run_ms
    if run_ms > 0:
        return frame_count / (run_ms / 1000.0)

    return None


def extract_cpu_frame_us_from_trace(trace_path: Path) -> Optional[float]:
    """Extract average CPU frame time from Chrome trace."""
    if not trace_path.exists():
        return None

    try:
        data = json.loads(trace_path.read_text(encoding="utf-8"))
        events = data.get("traceEvents") or []
    except (json.JSONDecodeError, KeyError):
        return None

    # Collect frame durations
    stacks: Dict[int, list] = {}
    durations = []

    for ev in events:
        name = ev.get("name", "")
        ph = ev.get("ph", "")
        tid = ev.get("tid", 0)

        if name in ("Frame", "frame", "Engine::tick", "tick"):
            if ph == "B":
                stacks.setdefault(tid, []).append(ev.get("ts", 0))
            elif ph == "E":
                st = stacks.get(tid)
                if st:
                    start = st.pop()
                    dur = ev.get("ts", 0) - start
                    if dur > 0:
                        durations.append(dur)
            elif ph == "X":
                dur = ev.get("dur", 0)
                if dur > 0:
                    durations.append(dur)

    if not durations:
        return None

    # Return average frame time in microseconds
    return sum(durations) / len(durations)


def extract_draw_calls_from_log(log_path: Path) -> Optional[float]:
    """Extract average draw calls per frame from GPU stats log."""
    if not log_path.exists():
        return None

    text = log_path.read_text(encoding="utf-8", errors="replace")
    draws = []

    for m in GPU_STATS_RE.finditer(text):
        total = int(m.group("uber_gpu_draws_total"))
        draws.append(total)

    if not draws:
        return None

    # Skip first few frames (warmup may have landed in capture)
    skip = min(5, len(draws) // 4)
    draws = draws[skip:]

    if not draws:
        return None

    return sum(draws) / len(draws)


def extract_draw_calls_from_trace(trace_path: Path) -> Optional[float]:
    """Extract draw calls from trace instant events."""
    if not trace_path.exists():
        return None

    try:
        data = json.loads(trace_path.read_text(encoding="utf-8"))
        events = data.get("traceEvents") or []
    except (json.JSONDecodeError, KeyError):
        return None

    draws = []
    for ev in events:
        if ev.get("ph") == "i" and ev.get("name") == "gpu_draws_total":
            args = ev.get("args") or {}
            val = args.get("value")
            if val is not None:
                draws.append(int(val))

    if not draws:
        return None

    skip = min(5, len(draws) // 4)
    draws = draws[skip:]
    if not draws:
        return None

    return sum(draws) / len(draws)


# ---------------------------------------------------------------------------
# Threshold judgment
# ---------------------------------------------------------------------------

def judge_metric(value: float, soft: float, hard: float, unit: str) -> str:
    """
    Judge a metric value against thresholds.
    Returns: "pass", "warn", or "FAIL"

    For fps: higher is better (soft > hard)
    For count/us: lower is better (soft < hard)
    """
    if unit == "fps":
        # Higher is better
        if value >= soft:
            return "pass"
        elif value >= hard:
            return "warn"
        else:
            return "FAIL"
    else:
        # Lower is better (count, us, ms)
        if value <= soft:
            return "pass"
        elif value <= hard:
            return "warn"
        else:
            return "FAIL"


# ---------------------------------------------------------------------------
# Scene runner
# ---------------------------------------------------------------------------

def run_scene(
    scene_id: str,
    scene_cfg: dict,
    exe_dir: Path,
    output_dir: Path,
    iters: int = 1,
) -> SceneResult:
    """Run a single scene and extract metrics."""
    scene_name = scene_cfg.get("name", scene_id)
    track = scene_cfg.get("track", "")
    exe_name = scene_cfg.get("exe", "")
    args = scene_cfg.get("args", [])
    warmup_ms = scene_cfg.get("warmup_ms", 2000)
    run_ms = scene_cfg.get("run_ms", 3000)
    env_cfg = scene_cfg.get("env", {})
    metrics_cfg = scene_cfg.get("metrics", {})

    # Resolve exe path
    exe_path = exe_dir / (exe_name + (".exe" if sys.platform == "win32" else ""))
    if not exe_path.exists():
        return SceneResult(
            scene_id=scene_id,
            scene_name=scene_name,
            track=track,
            error=f"Executable not found: {exe_path}",
            exe_path=str(exe_path),
            timestamp=datetime.now().isoformat(),
        )

    # Prepare output paths
    timestamp_str = datetime.now().strftime("%Y%m%d_%H%M%S")
    trace_path = output_dir / f"trace_{scene_id}_{timestamp_str}.json"
    log_path = output_dir / f"log_{scene_id}_{timestamp_str}.txt"

    # Build environment
    env = os.environ.copy()
    for k, v in env_cfg.items():
        env[k] = str(v)
    env["DONG_PROFILER_OUTPUT"] = str(trace_path)
    env["DONG_BENCH_WARMUP_MS"] = str(warmup_ms)
    env["DONG_BENCH_RUN_MS"] = str(run_ms)

    # Run iterations and collect best metrics
    all_fps = []
    all_draws = []
    all_cpu_frame = []

    for it in range(iters):
        # Update trace path for each iter
        iter_trace = output_dir / f"trace_{scene_id}_{timestamp_str}_i{it}.json"
        iter_log = output_dir / f"log_{scene_id}_{timestamp_str}_i{it}.txt"
        env["DONG_PROFILER_OUTPUT"] = str(iter_trace)

        cmd = [str(exe_path)] + [str(a) for a in args]
        timeout_s = (warmup_ms + run_ms) / 1000.0 + 30.0

        print(f"  [{scene_id}] iter {it+1}/{iters}: {' '.join(cmd)}")
        sys.stdout.flush()

        t0 = time.time()
        try:
            proc = subprocess.run(
                cmd,
                env=env,
                cwd=str(exe_dir),
                timeout=timeout_s,
                capture_output=True,
                check=False,
            )
            elapsed = time.time() - t0

            # Write log (stderr contains GPU stats)
            iter_log.write_text(
                proc.stderr.decode("utf-8", errors="replace")
                + "\n---STDOUT---\n"
                + proc.stdout.decode("utf-8", errors="replace"),
                encoding="utf-8",
            )

            if proc.returncode != 0:
                print(f"    WARNING: exit code {proc.returncode}")

        except subprocess.TimeoutExpired:
            print(f"    WARNING: timed out after {timeout_s:.0f}s")
            continue
        except OSError as e:
            return SceneResult(
                scene_id=scene_id,
                scene_name=scene_name,
                track=track,
                error=f"Failed to run: {e}",
                exe_path=str(exe_path),
                timestamp=datetime.now().isoformat(),
            )

        # Extract metrics from this iteration
        fps = extract_fps_from_trace(iter_trace, run_ms)
        if fps is not None:
            all_fps.append(fps)

        draws = extract_draw_calls_from_log(iter_log)
        if draws is None:
            draws = extract_draw_calls_from_trace(iter_trace)
        if draws is not None:
            all_draws.append(draws)

        cpu_frame = extract_cpu_frame_us_from_trace(iter_trace)
        if cpu_frame is not None:
            all_cpu_frame.append(cpu_frame)

    # Use the last trace/log for the result path
    final_trace = output_dir / f"trace_{scene_id}_{timestamp_str}_i{iters-1}.json"
    final_log = output_dir / f"log_{scene_id}_{timestamp_str}_i{iters-1}.txt"

    # Build metric results
    metric_results = []
    metric_values = {
        "fps_static": (sum(all_fps) / len(all_fps)) if all_fps else None,
        "draw_call": (sum(all_draws) / len(all_draws)) if all_draws else None,
        "cpu_frame_us": (sum(all_cpu_frame) / len(all_cpu_frame)) if all_cpu_frame else None,
    }

    for metric_name, mcfg in metrics_cfg.items():
        value = metric_values.get(metric_name)
        soft = mcfg.get("soft", 0)
        hard = mcfg.get("hard", 0)
        unit = mcfg.get("unit", "")
        desc = mcfg.get("description", "")

        if value is None:
            verdict = "warn"
            value = -1.0
            desc += " [NO DATA]"
        else:
            verdict = judge_metric(value, soft, hard, unit)

        metric_results.append(MetricResult(
            name=metric_name,
            value=round(value, 2),
            unit=unit,
            soft=soft,
            hard=hard,
            verdict=verdict,
            description=desc,
        ))

    return SceneResult(
        scene_id=scene_id,
        scene_name=scene_name,
        track=track,
        metrics=metric_results,
        exe_path=str(exe_path),
        duration_s=elapsed if "elapsed" in dir() else 0.0,
        timestamp=datetime.now().isoformat(),
    )


# ---------------------------------------------------------------------------
# Reporting
# ---------------------------------------------------------------------------

def generate_markdown_report(result: RunResult) -> str:
    """Generate a Markdown table report."""
    lines = []
    lines.append(f"# Dong Perf Baseline Report")
    lines.append(f"")
    lines.append(f"**Machine**: {result.machine}")
    lines.append(f"**Timestamp**: {result.timestamp}")
    lines.append(f"**Status**: {'FAIL' if result.any_hard_fail else 'PASS'}")
    lines.append(f"")

    for sr in result.scenes:
        lines.append(f"## {sr.scene_id}: {sr.scene_name}")
        lines.append(f"")
        if sr.error:
            lines.append(f"> SKIPPED: {sr.error}")
            lines.append(f"")
            continue

        lines.append(f"| Metric | Value | Soft | Hard | Verdict |")
        lines.append(f"|--------|------:|-----:|-----:|:-------:|")
        for m in sr.metrics:
            icon = {"pass": "OK", "warn": "WARN", "FAIL": "FAIL"}[m.verdict]
            lines.append(
                f"| {m.name} | {m.value:.1f} {m.unit} | {m.soft} | {m.hard} | {icon} |"
            )
        lines.append(f"")

    return "\n".join(lines)


def generate_diff_report(current: RunResult, prev_path: Path) -> str:
    """Compare current results with a previous run."""
    try:
        prev_data = json.loads(prev_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, FileNotFoundError) as e:
        return f"Cannot load previous results: {e}"

    lines = []
    lines.append("# Perf Diff Report")
    lines.append("")

    # Build lookup of previous metrics
    prev_metrics: Dict[str, Dict[str, float]] = {}
    for scene in prev_data.get("scenes", []):
        sid = scene.get("scene_id", "")
        for m in scene.get("metrics", []):
            prev_metrics.setdefault(sid, {})[m["name"]] = m["value"]

    lines.append("| Scene | Metric | Prev | Curr | Delta | Verdict |")
    lines.append("|-------|--------|-----:|-----:|------:|:-------:|")

    for sr in current.scenes:
        if sr.error:
            continue
        prev_scene = prev_metrics.get(sr.scene_id, {})
        for m in sr.metrics:
            prev_val = prev_scene.get(m.name)
            if prev_val is not None and prev_val > 0:
                delta_pct = ((m.value - prev_val) / prev_val) * 100
                delta_str = f"{delta_pct:+.1f}%"
            else:
                delta_str = "N/A"
                prev_val = -1
            lines.append(
                f"| {sr.scene_id} | {m.name} | {prev_val:.1f} | {m.value:.1f} | {delta_str} | {m.verdict} |"
            )

    lines.append("")
    return "\n".join(lines)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Dong performance baseline runner",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("--scene", type=str, default=None,
                        help="Run specific scene ID (e.g., S1). Can be comma-separated.")
    parser.add_argument("--iters", type=int, default=1,
                        help="Iterations per scene (default: 1)")
    parser.add_argument("--ci", action="store_true",
                        help="CI mode: exit 1 if any hard threshold violated")
    parser.add_argument("--report", choices=["md", "json", "both"], default="both",
                        help="Report format (default: both)")
    parser.add_argument("--machine", type=str, default=None,
                        help="Machine identifier for report")
    parser.add_argument("--diff", type=Path, default=None,
                        help="Path to previous results JSON for diff comparison")
    parser.add_argument("--scenes-file", type=Path, default=None,
                        help="Path to perf_scenes.yaml (default: auto-detect)")
    parser.add_argument("--exe-dir", type=Path, default=None,
                        help="Path to exe directory (default: zig-out/bin)")
    parser.add_argument("--output-dir", type=Path, default=None,
                        help="Output directory for results (default: tmp/perf)")
    args = parser.parse_args()

    # Resolve paths
    script_dir = Path(__file__).resolve().parent
    project_dir = script_dir.parent.parent  # dong/

    scenes_file = args.scenes_file or Path(
        os.environ.get("DONG_PERF_SCENES", str(script_dir / "perf_scenes.yaml"))
    )
    exe_dir = args.exe_dir or Path(
        os.environ.get("DONG_PERF_EXE_DIR", str(project_dir / "zig-out" / "bin"))
    )
    output_dir = args.output_dir or (project_dir / "tmp" / "perf")
    output_dir.mkdir(parents=True, exist_ok=True)

    machine_id = args.machine or os.environ.get("COMPUTERNAME", os.environ.get("HOSTNAME", "unknown"))

    # Load scenes
    if not scenes_file.exists():
        print(f"ERROR: Scenes file not found: {scenes_file}", file=sys.stderr)
        return 2

    print(f"Loading scenes from: {scenes_file}")
    scenes_data = _load_yaml(scenes_file)

    if not scenes_data:
        print("ERROR: No scenes found in config", file=sys.stderr)
        return 2

    # Filter scenes if requested
    scene_ids = list(scenes_data.keys())
    if args.scene:
        requested = [s.strip() for s in args.scene.split(",")]
        scene_ids = [s for s in requested if s in scenes_data]
        missing = [s for s in requested if s not in scenes_data]
        if missing:
            print(f"WARNING: Unknown scenes: {missing}", file=sys.stderr)

    print(f"Scenes to run: {scene_ids}")
    print(f"Exe directory: {exe_dir}")
    print(f"Output directory: {output_dir}")
    print(f"Iterations: {args.iters}")
    print()

    # Run scenes
    run_result = RunResult(
        machine=machine_id,
        timestamp=datetime.now().isoformat(),
    )

    for sid in scene_ids:
        scene_cfg = scenes_data[sid]
        print(f"--- Running scene: {sid} ({scene_cfg.get('name', '')}) ---")
        sr = run_scene(sid, scene_cfg, exe_dir, output_dir, iters=args.iters)
        run_result.scenes.append(sr)

        if sr.error:
            print(f"  SKIPPED: {sr.error}")
        else:
            for m in sr.metrics:
                status_icon = {"pass": "OK", "warn": "!!", "FAIL": "XX"}[m.verdict]
                print(f"  [{status_icon}] {m.name}: {m.value:.1f} {m.unit} "
                      f"(soft={m.soft}, hard={m.hard})")
                if m.verdict == "FAIL":
                    run_result.any_hard_fail = True
        print()

    # Save JSON results
    timestamp_str = datetime.now().strftime("%Y%m%d_%H%M%S")
    json_path = output_dir / f"results_{timestamp_str}.json"

    # Convert to serializable dict
    result_dict = {
        "machine": run_result.machine,
        "timestamp": run_result.timestamp,
        "any_hard_fail": run_result.any_hard_fail,
        "scenes": [],
    }
    for sr in run_result.scenes:
        scene_dict = {
            "scene_id": sr.scene_id,
            "scene_name": sr.scene_name,
            "track": sr.track,
            "error": sr.error,
            "exe_path": sr.exe_path,
            "duration_s": sr.duration_s,
            "timestamp": sr.timestamp,
            "metrics": [asdict(m) for m in sr.metrics],
        }
        result_dict["scenes"].append(scene_dict)

    json_path.write_text(json.dumps(result_dict, indent=2), encoding="utf-8")
    print(f"Results saved to: {json_path}")

    # Generate reports
    if args.report in ("md", "both"):
        md_report = generate_markdown_report(run_result)
        md_path = output_dir / f"report_{timestamp_str}.md"
        md_path.write_text(md_report, encoding="utf-8")
        print(f"Markdown report: {md_path}")
        print()
        print(md_report)

    if args.diff:
        diff_report = generate_diff_report(run_result, args.diff)
        diff_path = output_dir / f"diff_{timestamp_str}.md"
        diff_path.write_text(diff_report, encoding="utf-8")
        print(f"\nDiff report: {diff_path}")
        print(diff_report)

    # CI exit code
    if args.ci and run_result.any_hard_fail:
        print("\nCI FAILURE: One or more hard thresholds violated.", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
