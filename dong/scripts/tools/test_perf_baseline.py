#!/usr/bin/env python3
"""
test_perf_baseline.py - Self-tests for perf_baseline.py

Verifies:
  1. YAML parsing (both PyYAML and fallback)
  2. Threshold judgment logic (pass, warn, FAIL)
  3. Report generation with mock data

Usage:
  python scripts/tools/test_perf_baseline.py
"""
from __future__ import annotations

import json
import sys
import tempfile
from pathlib import Path

# Add scripts/tools to path so we can import perf_baseline
sys.path.insert(0, str(Path(__file__).resolve().parent))

from perf_baseline import (
    MetricResult,
    RunResult,
    SceneResult,
    _load_yaml,
    _parse_simple_yaml,
    generate_markdown_report,
    judge_metric,
)


# ---------------------------------------------------------------------------
# Test data
# ---------------------------------------------------------------------------

SAMPLE_YAML = """\
# Performance scenes for testing
S1:
  name: "Game HUD (DOM)"
  track: dom
  exe: dong_app
  args: ["--html", "data/gamelikeui/game_ui2.html"]
  warmup_ms: 2000
  run_ms: 3000
  env:
    DONG_PROFILER: "1"
    DONG_GPU_STATS: "1"
    DONG_BENCH_AUTOSTOP: "1"
  metrics:
    fps_static:
      soft: 240
      hard: 120
      unit: fps
      description: "Frames per second"
    draw_call:
      soft: 30
      hard: 50
      unit: count
      description: "GPU draw calls per frame"

S3:
  name: "Multi-Screen 3D"
  track: multi
  exe: 3d_screens_simple
  args: []
  warmup_ms: 2000
  run_ms: 3000
  env:
    DONG_PROFILER: "1"
  metrics:
    fps_static:
      soft: 60
      hard: 30
      unit: fps
      description: "FPS with all screens"
"""


def test_yaml_parsing():
    """Test the fallback YAML parser."""
    print("Test: YAML parsing...")
    result = _parse_simple_yaml(SAMPLE_YAML)

    assert "S1" in result, f"Missing S1 key, got: {list(result.keys())}"
    assert "S3" in result, f"Missing S3 key, got: {list(result.keys())}"

    s1 = result["S1"]
    assert s1["name"] == "Game HUD (DOM)", f"Got name: {s1['name']}"
    assert s1["track"] == "dom", f"Got track: {s1['track']}"
    assert s1["exe"] == "dong_app", f"Got exe: {s1['exe']}"
    assert s1["warmup_ms"] == 2000, f"Got warmup_ms: {s1['warmup_ms']}"
    assert s1["run_ms"] == 3000, f"Got run_ms: {s1['run_ms']}"
    assert s1["args"] == ["--html", "data/gamelikeui/game_ui2.html"], f"Got args: {s1['args']}"

    # Check env
    env = s1["env"]
    assert env["DONG_PROFILER"] == "1", f"Got DONG_PROFILER: {env['DONG_PROFILER']}"
    assert env["DONG_GPU_STATS"] == "1", f"Got DONG_GPU_STATS: {env['DONG_GPU_STATS']}"
    assert env["DONG_BENCH_AUTOSTOP"] == "1", f"Got DONG_BENCH_AUTOSTOP: {env['DONG_BENCH_AUTOSTOP']}"

    # Check metrics
    metrics = s1["metrics"]
    assert "fps_static" in metrics, f"Missing fps_static, got: {list(metrics.keys())}"
    assert "draw_call" in metrics, f"Missing draw_call, got: {list(metrics.keys())}"

    fps = metrics["fps_static"]
    assert fps["soft"] == 240, f"Got soft: {fps['soft']}"
    assert fps["hard"] == 120, f"Got hard: {fps['hard']}"
    assert fps["unit"] == "fps", f"Got unit: {fps['unit']}"

    draw = metrics["draw_call"]
    assert draw["soft"] == 30, f"Got soft: {draw['soft']}"
    assert draw["hard"] == 50, f"Got hard: {draw['hard']}"

    # S3
    s3 = result["S3"]
    assert s3["name"] == "Multi-Screen 3D", f"Got name: {s3['name']}"
    assert s3["args"] == [], f"Got args: {s3['args']}"

    print("  PASSED")


def test_yaml_file_loading():
    """Test loading YAML from a file."""
    print("Test: YAML file loading...")
    with tempfile.NamedTemporaryFile(mode="w", suffix=".yaml", delete=False, encoding="utf-8") as f:
        f.write(SAMPLE_YAML)
        tmp_path = Path(f.name)

    try:
        result = _load_yaml(tmp_path)
        assert "S1" in result
        assert result["S1"]["name"] == "Game HUD (DOM)"
        print("  PASSED")
    finally:
        tmp_path.unlink(missing_ok=True)


def test_judge_metric_fps():
    """Test threshold judgment for FPS metrics (higher is better)."""
    print("Test: judge_metric (fps)...")

    # Above soft threshold -> pass
    assert judge_metric(300.0, soft=240, hard=120, unit="fps") == "pass"
    assert judge_metric(240.0, soft=240, hard=120, unit="fps") == "pass"

    # Between soft and hard -> warn
    assert judge_metric(180.0, soft=240, hard=120, unit="fps") == "warn"
    assert judge_metric(120.0, soft=240, hard=120, unit="fps") == "warn"

    # Below hard -> FAIL
    assert judge_metric(119.9, soft=240, hard=120, unit="fps") == "FAIL"
    assert judge_metric(60.0, soft=240, hard=120, unit="fps") == "FAIL"
    assert judge_metric(0.0, soft=240, hard=120, unit="fps") == "FAIL"

    print("  PASSED")


def test_judge_metric_count():
    """Test threshold judgment for count metrics (lower is better)."""
    print("Test: judge_metric (count)...")

    # Below soft threshold -> pass
    assert judge_metric(10.0, soft=30, hard=50, unit="count") == "pass"
    assert judge_metric(30.0, soft=30, hard=50, unit="count") == "pass"

    # Between soft and hard -> warn
    assert judge_metric(31.0, soft=30, hard=50, unit="count") == "warn"
    assert judge_metric(50.0, soft=30, hard=50, unit="count") == "warn"

    # Above hard -> FAIL
    assert judge_metric(50.1, soft=30, hard=50, unit="count") == "FAIL"
    assert judge_metric(100.0, soft=30, hard=50, unit="count") == "FAIL"

    print("  PASSED")


def test_judge_metric_us():
    """Test threshold judgment for microsecond metrics (lower is better)."""
    print("Test: judge_metric (us)...")

    # Below soft -> pass
    assert judge_metric(2000.0, soft=4000, hard=8000, unit="us") == "pass"
    assert judge_metric(4000.0, soft=4000, hard=8000, unit="us") == "pass"

    # Between soft and hard -> warn
    assert judge_metric(5000.0, soft=4000, hard=8000, unit="us") == "warn"
    assert judge_metric(8000.0, soft=4000, hard=8000, unit="us") == "warn"

    # Above hard -> FAIL
    assert judge_metric(8001.0, soft=4000, hard=8000, unit="us") == "FAIL"

    print("  PASSED")


def test_markdown_report():
    """Test markdown report generation with mock data."""
    print("Test: Markdown report generation...")

    run_result = RunResult(
        machine="test-machine",
        timestamp="2026-04-30T12:00:00",
        scenes=[
            SceneResult(
                scene_id="S1",
                scene_name="Game HUD (DOM)",
                track="dom",
                metrics=[
                    MetricResult(
                        name="fps_static",
                        value=280.5,
                        unit="fps",
                        soft=240,
                        hard=120,
                        verdict="pass",
                        description="Frames per second",
                    ),
                    MetricResult(
                        name="draw_call",
                        value=25.0,
                        unit="count",
                        soft=30,
                        hard=50,
                        verdict="pass",
                        description="GPU draw calls",
                    ),
                ],
                exe_path="zig-out/bin/dong_app.exe",
                duration_s=5.2,
                timestamp="2026-04-30T12:00:00",
            ),
            SceneResult(
                scene_id="S3",
                scene_name="Multi-Screen 3D",
                track="multi",
                metrics=[
                    MetricResult(
                        name="fps_static",
                        value=25.0,
                        unit="fps",
                        soft=60,
                        hard=30,
                        verdict="FAIL",
                        description="FPS",
                    ),
                ],
                exe_path="zig-out/bin/3d_screens_simple.exe",
                duration_s=5.1,
                timestamp="2026-04-30T12:00:00",
            ),
            SceneResult(
                scene_id="S_SKIP",
                scene_name="Skipped Scene",
                track="skip",
                error="Executable not found",
                exe_path="missing.exe",
                timestamp="2026-04-30T12:00:00",
            ),
        ],
        any_hard_fail=True,
    )

    md = generate_markdown_report(run_result)

    # Verify report content
    assert "# Dong Perf Baseline Report" in md
    assert "test-machine" in md
    assert "FAIL" in md
    assert "S1: Game HUD (DOM)" in md
    assert "S3: Multi-Screen 3D" in md
    assert "280.5" in md
    assert "25.0" in md
    assert "SKIPPED" in md
    assert "| Metric | Value | Soft | Hard | Verdict |" in md

    print("  PASSED")
    print(f"  (Report is {len(md)} chars, {md.count(chr(10))} lines)")


def test_json_serialization():
    """Test that results can be serialized to JSON."""
    print("Test: JSON serialization...")

    from dataclasses import asdict

    metric = MetricResult(
        name="fps_static",
        value=280.5,
        unit="fps",
        soft=240,
        hard=120,
        verdict="pass",
        description="test",
    )

    d = asdict(metric)
    json_str = json.dumps(d)
    loaded = json.loads(json_str)

    assert loaded["name"] == "fps_static"
    assert loaded["value"] == 280.5
    assert loaded["verdict"] == "pass"

    print("  PASSED")


def test_edge_cases():
    """Test edge cases in judgment logic."""
    print("Test: Edge cases...")

    # Zero values
    assert judge_metric(0.0, soft=240, hard=120, unit="fps") == "FAIL"
    assert judge_metric(0.0, soft=30, hard=50, unit="count") == "pass"

    # Equal to boundaries
    assert judge_metric(240.0, soft=240, hard=120, unit="fps") == "pass"
    assert judge_metric(120.0, soft=240, hard=120, unit="fps") == "warn"
    assert judge_metric(30.0, soft=30, hard=50, unit="count") == "pass"
    assert judge_metric(50.0, soft=30, hard=50, unit="count") == "warn"

    # Negative values (shouldn't happen but handle gracefully)
    assert judge_metric(-1.0, soft=240, hard=120, unit="fps") == "FAIL"
    assert judge_metric(-1.0, soft=30, hard=50, unit="count") == "pass"

    print("  PASSED")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> int:
    print("=" * 60)
    print("perf_baseline.py self-tests")
    print("=" * 60)
    print()

    tests = [
        test_yaml_parsing,
        test_yaml_file_loading,
        test_judge_metric_fps,
        test_judge_metric_count,
        test_judge_metric_us,
        test_markdown_report,
        test_json_serialization,
        test_edge_cases,
    ]

    passed = 0
    failed = 0

    for test_fn in tests:
        try:
            test_fn()
            passed += 1
        except AssertionError as e:
            print(f"  FAILED: {e}")
            failed += 1
        except Exception as e:
            print(f"  ERROR: {type(e).__name__}: {e}")
            failed += 1

    print()
    print("=" * 60)
    print(f"Results: {passed} passed, {failed} failed, {passed + failed} total")
    print("=" * 60)

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
