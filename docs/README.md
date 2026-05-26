# Dong 文档

> **单一文档入口** — 所有资料位于 `docs/` 目录。

---

## Getting Started

| 文档 | 说明 |
|------|------|
| [快速开始](./getting-started/quickstart.md) | 5 分钟跑起第一个 demo |
| [从源码构建](./getting-started/build-from-source.md) | 依赖、交叉编译、配置 |
| [调试指南](./guide/debugging.md) | 日志、Profiler、无头渲染 |

## Overview

| 文档 | 说明 |
|------|------|
| [定位与边界](./overview/positioning.md) | Dong 是什么、目标场景、能力边界 |
| [路线图](./roadmap.md) | 三阶段交付计划摘要 |
| [完整路线图](./developer/roadmap-full.md) | Phase DoD 与决策记录 |

## Guide

| 文档 | 说明 |
|------|------|
| [渲染模式](./guide/render-modes.md) | DOM / Scene Graph / Direct Draw |
| [React / Preact](./guide/react-preact.md) | JSX 集成与运行 |
| [Unreal Engine 集成（草案）](./guide/integrations/unreal-engine.md) | UE adapter 方向 |
| [Unity 集成（草案）](./guide/integrations/unity.md) | Unity adapter 方向 |

## Architecture

| 文档 | 说明 |
|------|------|
| [架构概览](./architecture/overview.md) | 库分层、模块职责 |
| [渲染管线优化](./architecture/render-pipeline.md) | Uber Quad Pipeline |
| [文本渲染](./architecture/text-rendering.md) | MSDF / Slug 双渲染器 |

## Reference

| 文档 | 说明 |
|------|------|
| [特性索引](./reference/features-index.md) | 已交付能力 + Case 列表 |
| [CSS 子集](./reference/css-subset.md) | 支持的 CSS 属性 |
| [JavaScript API](./reference/js-api.md) | `dong.*` 与 DOM 扩展 |
| [Dong 扩展](./reference/extensions.md) | 私有 HTML/CSS/JS/C API |
| [环境变量](./reference/env-vars.md) | `DONG_*` 配置 |
| [第三方许可证](./reference/third-party-licenses.md) | 依赖库许可证 |

## Developer（维护者）

Phase spec、性能预算、编排、深度架构规格：

→ [developer/README.md](./developer/README.md)

## 示例资源

HTML 测试页位于 `dong/examples/data/`，构建输出在 `dong/zig-out/bin/data/`。文档中的 Case 名称均相对于 `data/`。
