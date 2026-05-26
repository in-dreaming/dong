# 路线图

> 完整 Phase spec 与完成判定见 [developer/roadmap-full.md](./developer/roadmap-full.md)。

## 原则

1. **先性能、再可嵌入、再生态**
2. 每个 Phase 有可测的 Definition of Done
3. 三轨（DOM / Scene / Direct Draw）均需 perf 基线覆盖
4. HTML/CSS 缺口按「游戏 UI 是否需要」补，非按 web P0/P1

## Phase 0 — 游戏运行时性能（0–3 个月）

聚焦：**性能** + **游戏交互必备**

| 项 | 说明 |
|----|------|
| Uber Quad Pipeline | 合批 rect/round/shadow，降低 draw call |
| 9-slice panel | `border-image` GPU 渲染 |
| CSS mask + conic-gradient | 圆形血条等 |
| Gamepad 空间导航 | D-pad 焦点跳转 |
| IME composition | 中文/日文输入 |
| Display List 局部失效 | editor 拖动不全树重建 |
| 三轨 perf 基线脚本 | `perf_baseline.py` CI 对比 |

**完成判定摘要**：`game_ui2` 1080p/60Hz 稳定 240+ FPS；中文 IME 正确；手柄可完成完整 UI 路径。

## Phase 1 — 可嵌入与工程化（3–9 个月）

聚焦：**宿主嵌入** + **开发体验**

| 项 | 说明 |
|----|------|
| DongDrawList C ABI | RT-free draw command 导出 |
| `<host-view>` | HTML 内嵌宿主渲染 |
| DevTools v1 | F12 DOM inspector |
| Live Reload | `--watch` ≤500ms 重载 |
| CSS Grid subset | 现代 Grid 布局 |
| HDR 输出 | 10-bit swapchain |
| JS 引擎 benchmark | QuickJS vs 备选评估 |

## Phase 2 — 生态与世界空间（9–18 个月）

聚焦：**3D primitive** + **组件生态**

| 项 | 说明 |
|----|------|
| `dong_world_text_t` | 世界空间 billboard 文字 |
| `dong_decal_t` / `dong_world_overlay_t` | 贴花与世界 overlay |
| Lottie / Rive | 矢量动画 |
| `dong-ui` 组件库 | ≥20 组件 + 主题 |
| TypeScript + npm | `@dong/react` 等 |
| `.dpkg` 资源打包 | 启动 <200ms |
| 可视化编辑器 | dogfood |

## 明确不做

- Chromium / Coherent 兼容路线
- 第四种渲染轨道
- `dong_scene3d_*` 扩展（阴影/PBR/骨骼）
- Web Worker / WebGL / Service Worker 等 Web 平台 API
- Core 团队短期投入 UE/Unity adapter 工程

## 相关文档

- [定位与边界](./overview/positioning.md)
- [Unreal Engine 集成草案](./guide/integrations/unreal-engine.md)
- [Unity 集成草案](./guide/integrations/unity.md)
