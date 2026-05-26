# 调试指南

## Debug 构建

```bash
cd dong
zig build examples -Doptimize=Debug
./zig-out/bin/dong_app --html data/tests/test_grid_basic.html
```

Release 性能测试：

```bash
zig build examples -Doptimize=ReleaseSmall
```

## 日志

```bash
set DONG_LOG_LEVEL=DEBUG
set DONG_LOG_TO_STDOUT=1
set DONG_GPU_STATS=1
```

完整列表见 [环境变量参考](../reference/env-vars.md)。

## 文本渲染切换

```bash
set DONG_TEXT_RENDERER=slug    # 推荐
set DONG_TEXT_RENDERER=msdf
set DONG_TEXT_RENDERER=auto
```

## Chrome Trace Profiler

```bash
set DONG_PROFILER=1
set DONG_PROFILER_OUTPUT=trace.json
dong_app.exe --html data/tests/test_dual_mode.html
```

在 Chrome `chrome://tracing` 或 Perfetto 打开 `trace.json`。

## 无头渲染对比

```bash
zig build run-html-test -- data/tests/test_grid_basic.html out.bmp 800 600
python scripts/tools/run_baseline_compare.py   # 与浏览器 baseline 对比
```

## LLDB / GDB

```bash
lldb ./zig-out/bin/dong_app
(lldb) b engine_view.cpp:123
(lldb) run --html data/preact-counter/index.html
```

Windows 可用 Visual Studio 附加进程，或 CodeLLDB 扩展（见 `.vscode/launch.json`）。

## 多帧 / 交互回归

- `scripts/tools/run_multiframe_regress.py` — 非确定性检测
- `scripts/tools/html_baseline_render.py` — Playwright 浏览器基线

维护者详细笔记：[developer/debug/](../developer/debug/)
