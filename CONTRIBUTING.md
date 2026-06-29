# 贡献指南

感谢你对 Dong 的关注！本文档面向人类贡献者。AI 编码助手另请参阅 [AGENTS.md](./AGENTS.md)。

## 开发环境

| 工具 | 版本 / 说明 |
|------|-------------|
| [Zig](https://ziglang.org/) | 0.16（主构建系统） |
| CMake + Ninja | SDL3、部分依赖 |
| Python 3.10+ | 脚本与测试工具 |
| Node.js | 可选，从源码构建 React/Preact bundle |
| Visual Studio Build Tools | Windows 必需（`clang-cl`） |

详细构建步骤见 [docs/getting-started/build-from-source.md](./docs/getting-started/build-from-source.md)。

克隆后初始化子模块（`gpu` 含嵌套依赖，必须 recursive）：

```bash
git submodule update --init --recursive
```

## 快速构建与测试

所有命令在 `dong/` 子目录执行：

```bash
cd dong

# 构建全部示例
zig build examples

# 功能测试
zig build run-feature-tests

# 单页无头渲染
zig build run-html-test -- data/tests/test_grid_basic.html output.bmp 800 600

# 批量渲染测试页
zig build render-all-tests
```

Debug 构建：

```bash
zig build examples -Doptimize=Debug
```

## 代码组织

```
dong/
├── src/           # Core：DOM、CSS、Layout、Script、Render
├── backends/sdl/  # SDL GPU 后端与着色器
├── backends/gpu/  # in-dreaming/gpu 后端 stub
├── third_party/gpu/  # in-dreaming/gpu 子模块（独立构建）
├── appcore/       # 高层 C API（dong_app、scene3d）
├── include/       # 公开 C 头文件
├── examples/      # 示例程序
└── scripts/tools/ # 测试与基准脚本
```

**核心原则**：

- `dong.dll`（Core）平台无关，平台能力通过 Plugin 注入
- 优先最小正确改动，避免过度抽象
- 滚轮语义：`delta_y` 正值 = 向下滚动

## 提交 Pull Request

1. Fork 仓库并创建特性分支
2. 确保相关测试通过（至少 `run-feature-tests`）
3. 渲染相关改动建议跑 `html_render_test` 或 `run_baseline_compare.py`
4. PR 描述说明**为什么**改、如何验证
5. 大功能请先开 Issue 讨论方向（参考 [定位文档](./docs/overview/positioning.md)）

## 测试工具

位于 `dong/scripts/tools/`：

| 脚本 | 用途 |
|------|------|
| `html_baseline_render.py` | Playwright 浏览器基线渲染 |
| `run_baseline_compare.py` | 自动对比 Dong vs 浏览器 |
| `compare_screenshots.py` | 像素级截图对比 |
| `perf_baseline.py` | 三轨性能基准 |
| `run_multiframe_regress.py` | 多帧非确定性检测 |

## 日志与调试

```bash
# 日志级别
set DONG_LOG_LEVEL=DEBUG
set DONG_LOG_TO_STDOUT=1

# GPU 统计
set DONG_GPU_STATS=1

# Chrome Trace
set DONG_PROFILER=1
set DONG_PROFILER_OUTPUT=trace.json
```

完整环境变量列表见 [docs/reference/env-vars.md](./docs/reference/env-vars.md)。

## 文档

- **用户 / 集成方文档**：[`docs/`](./docs/)
- **内部维护文档**（Phase spec、调试笔记等）：[`docs/developer/`](./docs/developer/)

修改公开行为时请同步更新 `docs/` 中对应页面。

## 行为准则

请保持尊重、建设性的交流。如有骚扰或不当行为，维护者可拒绝参与。

## 许可证

贡献代码即表示你同意在 [MIT License](./LICENSE) 下发布你的贡献。
