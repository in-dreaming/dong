# 环境变量

Dong 通过 `DONG_*` 环境变量配置运行时行为。

## 日志

| 变量 | 说明 | 默认 |
|------|------|------|
| `DONG_LOG_LEVEL` | `ERROR` / `WARN` / `INFO` / `DEBUG` | `INFO` |
| `DONG_LOG_TO_STDOUT` | `1` 输出到 stdout | stderr |

## 脚本引擎

| 变量 | 说明 | 默认 |
|------|------|------|
| `DONG_SCRIPT_TIMEOUT_MS` | JS 执行超时（毫秒） | 较低，Preact 建议 `10000` |

## 文本渲染

| 变量 | 说明 | 默认 |
|------|------|------|
| `DONG_TEXT_RENDERER` | `slug` / `msdf` / `auto` | `auto` |

## GPU / 渲染

| 变量 | 说明 | 默认 |
|------|------|------|
| `DONG_GPU_STATS` | `1` 每帧输出 GPU 统计 | 关 |
| `DONG_FORCE_FULL_REPAINT` | `1` 强制全量重绘 | 关 |
| `DONG_ATLAS_FORMAT` | `BC7` / `ASTC` 压缩纹理图集 | RGBA8 |

## Profiler

| 变量 | 说明 | 默认 |
|------|------|------|
| `DONG_PROFILER` | `1` 启用 Chrome Trace | 关 |
| `DONG_PROFILER_OUTPUT` | Trace 输出路径 | `trace.json` |

## 3D Scene

| 变量 | 说明 | 默认 |
|------|------|------|
| `DONG_SCENE3D_OVERLAY_TICK_HZ` | 使用 overlay API 的 screen 刷新率 | `30` |

## 调试

| 变量 | 说明 |
|------|------|
| `DONG_DEBUG_EVENT_BRIDGE` | 事件桥调试日志 |
| `DONG_DEBUG_VIDEO` | 视频播放调试日志 |

Windows 设置示例：

```cmd
set DONG_LOG_LEVEL=DEBUG
set DONG_GPU_STATS=1
set DONG_SCRIPT_TIMEOUT_MS=10000
```

Linux / macOS：

```bash
export DONG_LOG_LEVEL=DEBUG
export DONG_GPU_STATS=1
export DONG_SCRIPT_TIMEOUT_MS=10000
```

## 相关文档

- [快速开始](../getting-started/quickstart.md)
- [贡献指南](../../CONTRIBUTING.md) § 日志与调试
