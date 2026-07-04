# T17 — 多 View / 模块多实例支持

- **Area**: dong-host + porffor-fork
- **性质**: **两段式** — 阶段 0：spike（2c 全局变量落点）+ 方案 A/B/C 选型，产出**决策记录**并评审；阶段 1：按决策实现。实现对标物：QuickJS 路径的 per-View 隔离行为
- **优先级**: P1（3D 多屏是 Dong 的核心场景，全面切换后没有 QuickJS 的 per-View context 可用）
- **依赖**: 无硬依赖；建议 T15 定稿模块结构后动工（模块组织方式互相影响）
- **预估**: 调研 spike 1 人天 + 实现 3–5 人天（视方案）
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3（事实 F10）

## 背景（事实 F10）

QuickJS 路径每个 View 有独立 JSContext；Porffor 路径现状是**进程级单例**：

- `g_host` / `g_registry` 是全局单例（`dong_porf_host.cpp` 匿名命名空间）；
- 2c 产物的内存是**全局 C 变量**（`dong_porf_<mod>_memory` / `_memory_pages`，见 `registry.c`）；
- `PorfforHost::setBindings` 只能绑一个 View 的 JSBindings。

结果：同一个 Porffor 模块无法被两个 View 同时实例化（状态互踩）。而 Dong 的招牌场景是 3D 多屏（23 个 HTML 屏共存，见 `3d_screens` 示例）——全面切换后这是必须解决的能力，不是优化。

## 方案调研（任务内 spike 后定稿）

关键前置问题：**2c 产物的 JS 全局变量落在哪**——`_memory` 里（则换 memory 指针即可换实例）还是 C 全局变量里（则 swap memory 不够）。先编译一个含全局变量的模块看产物。

- **A) 2c reentrant 模式（fork，最彻底）**：memory / globals 全部收进一个上下文结构体，入口函数带 `ctx*` 参数。改动大，收益是真正的多实例与线程安全。
- **B) 实例快照 / swap（dong-host）**：为每个 (View, module) 分配独立 memory 块 + C 全局变量快照，调用前整体 swap（现有 `PorfforHost_setActiveModule` 机制的推广）。若 JS 全局在 C 变量里，需要 2c 输出「全局变量清单」供快照（小的 fork 配合）。
- **C) 链接期复制**：每 View 链接一份带不同符号前缀的模块副本。零 fork 改动，二进制膨胀，View 数量 build 期锁死——只配作垫底方案。

同时把 host 侧单例结构理顺：`PorfforHost` / `PorfforScriptRegistry` 改为 per-View（或 per-View 上下文表），`JSBindings` 绑定关系明确。

## 验收标准

1. spike 结论（全局变量落点 + 方案选型 + 理由）记录在案。
2. 两个 View 加载同一模块，各自计数器（依赖 T15 的状态模型）互不干扰。
3. `3d_screens` 示例的 Porffor 版 smoke：≥3 个屏各自运行独立模块实例，无踩踏。
4. per-View 的 host / registry / bindings 结构关系图进文档；setup §3 事实 F10 更新。

## 风险

- 方案 B 的「C 全局变量快照」在模块多、全局多时 swap 成本可观，需按帧内 callExport 次数评估；
- 与 T15 的模块结构改造有耦合（多 export 单模块 × 多实例），两个任务的产物要联调一次。
