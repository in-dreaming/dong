# Multi-Window Docking System 实现计划

API 头文件: `dong/appcore/include/dong_dock.h`

## 架构概览

```
dong_dock_t
├── SDL_GPUDevice*              (所有窗口共享)
├── DongGPUDriver*              (Platform 单例注册)
├── dong_dock_window_t[]        (多个 OS 窗口)
│   ├── SDL_Window*
│   ├── split_node_t*           (二叉分割树根)
│   └── swapchain composite 状态
├── dong_dock_pane_t[]          (全局 pane 列表)
│   ├── dong_engine_t*
│   ├── SDL_GPUTexture*         (offscreen render target)
│   ├── title, view_name
│   └── 所属 window + split_node 引用
└── 回调: pane_close, window_close
```

核心原则: 每个 pane 始终 offscreen rendering, 窗口只负责 composite (blit).
detach/attach 只改变 pane 所在的 split tree, 不重建 engine 或 texture.

---

## P0: 基础骨架

目标: 多窗口 + split + detach/attach 可通过 API 调用工作, 无拖拽 UI.

### P0.1 dock.c 骨架 + 生命周期

文件: `appcore/src/dock.c`

- [ ] `dong_dock_impl_t` 结构体
  - SDL_GPUDevice*, primary window
  - pane 数组 (动态扩展, realloc)
  - window 数组
  - 回调函数指针
  - frame timing (同 app.c 的 PerformanceCounter 模式)
- [ ] `dong_dock_create`: SDL_Init → CreateGPUDevice → CreateWindow → ClaimWindowForGPUDevice → dong_sdl_platform_init → 初始化 primary window
- [ ] `dong_dock_destroy`: 逆序销毁 pane engine → 销毁 texture → 销毁 window → dong_sdl_platform_shutdown → DestroyGPUDevice → SDL_Quit
- [ ] `dong_dock_is_running`: 检查 window_count > 0
- [ ] `dong_dock_get_delta_time`: PerformanceCounter 差值

依赖: 无新依赖, 复用 app.c 的 SDL 初始化模式.

### P0.2 Pane 生命周期

- [ ] `dong_dock_add_pane`:
  - dong_engine_create (或 dong_engine_create_shared_js)
  - dong_engine_set_gpu
  - 创建 offscreen SDL_GPUTexture (RGBA8, COLOR_TARGET | SAMPLER)
  - 如果有 html/html_file, dong_engine_load_html
  - 将 pane 挂到 primary window 的 split tree 根 (LEAF)
- [ ] `dong_dock_remove_pane`:
  - 从 split tree 移除, sibling 提升
  - dong_engine_destroy
  - SDL_ReleaseGPUTexture
  - 如果 window 变空, 销毁 window
- [ ] Pane 属性 API: load_html, set_title, eval_script, get_engine (直接代理到 dong_engine_*)

### P0.3 Split Tree

文件: `appcore/src/dock_split.c` (纯数据结构, 无 SDL 依赖)

```c
typedef enum { SPLIT_LEAF, SPLIT_H, SPLIT_V } split_type_t;

typedef struct split_node_t {
    split_type_t type;
    float ratio;                    // 0.0~1.0, 左/上占比
    struct split_node_t* parent;
    union {
        struct {                    // SPLIT_H / SPLIT_V
            struct split_node_t* children[2];
        };
        struct {                    // SPLIT_LEAF
            dong_dock_pane_t** tabs; // tab 组 (≥1 个 pane)
            int tab_count;
            int active_tab;
        };
    };
    // 布局计算结果 (每帧更新)
    int32_t x, y;
    uint32_t w, h;
} split_node_t;
```

- [ ] `split_node_new_leaf(pane)` → 创建叶节点
- [ ] `split_node_split(node, edge, new_pane, ratio)` → 将叶节点替换为 split, 原节点和新节点成为 children
- [ ] `split_node_remove(node, pane)` → 从叶移除 pane, 如果空则 sibling 提升到 parent 位置
- [ ] `split_node_add_tab(leaf, pane)` → 给叶节点添加 tab
- [ ] `split_node_layout(root, x, y, w, h)` → 递归计算每个 leaf 的 rect
- [ ] `split_node_hit_test(root, mx, my)` → 返回命中的 leaf pane
- [ ] `split_node_free(root)` → 递归释放

### P0.4 事件路由

在 `dong_dock_poll_events` 中:

- [ ] SDL_PollEvent → 按 event.window.windowID 找到 dock_window
- [ ] 翻译 SDL_Event → dong_app_event_t (复用 app.c 的翻译逻辑)
- [ ] mouse 事件: split_tree_hit_test → 转换为 pane 局部坐标 → dong_engine_send_*
- [ ] key/text 事件: 发送到当前 focused pane (最后点击的 pane)
- [ ] WINDOW_CLOSE_REQUESTED: 调 window_close callback → 如果允许则销毁 window
- [ ] 窗口 resize: 更新 window 尺寸 → split_node_layout 重算 → 各 pane dong_engine_resize

### P0.5 渲染 Composite

在 `dong_dock_render` 中:

```
for each pane (全局):
    dong_gpu_begin_frame_offscreen(driver, pane->texture, pane->w, pane->h)
    dong_engine_tick(pane->engine)
    dong_gpu_end_frame_offscreen(driver)

for each window:
    cmd = SDL_AcquireGPUCommandBuffer(device)
    swapchain = SDL_AcquireGPUSwapchainTexture(cmd, window->sdl_win, ...)
    clear swapchain
    for each leaf in window->split_tree:
        SDL_BlitGPUTexture(cmd, pane->texture → swapchain, leaf->rect)
    SDL_SubmitGPUCommandBuffer(cmd)
```

- [ ] 实现 `walk_and_blit(node, cmd, swapchain)` 递归遍历 split tree
- [ ] 处理 tab 组: 只 blit active_tab 的 texture

### P0.6 Detach / Attach

- [ ] `dong_dock_detach(dock, pane, x, y, w, h)`:
  1. split_node_remove 从原 tree 移除
  2. SDL_CreateWindow → SDL_ClaimWindowForGPUDevice
  3. 新建 dock_window, root = new_leaf(pane)
  4. 如果原 window 空了, 销毁原 window
  5. pane 的 engine/texture 不变, 只是 composite 目标变了

- [ ] `dong_dock_attach(dock, pane, target, edge)`:
  1. split_node_remove 从 pane 当前位置移除
  2. 如果 pane 所在 window 空了, 销毁该 window
  3. 在 target 所在的 leaf 上 split_node_split 或 split_node_add_tab

### P0.7 Demo

文件: `examples/dock_demo.cpp`

```c
dong_dock_config_t cfg = { .title = "Dock Demo", .width = 1280, .height = 720 };
dong_dock_t* dock = dong_dock_create(&cfg);

dong_dock_pane_t* p1 = dong_dock_add_pane(dock, &(dong_dock_pane_config_t){
    .title = "Editor", .html = editor_html });

dong_dock_pane_t* p2 = dong_dock_split(dock, p1, DONG_DOCK_RIGHT, 0.6f,
    &(dong_dock_pane_config_t){ .title = "Preview", .html = preview_html,
                                 .shared_js = dong_dock_pane_get_engine(p1),
                                 .view_name = "preview" });

dong_dock_pane_t* p3 = dong_dock_split(dock, p1, DONG_DOCK_BOTTOM, 0.7f,
    &(dong_dock_pane_config_t){ .title = "Console", .html = console_html });

dong_dock_run(dock, NULL, NULL);
dong_dock_destroy(dock);
```

### P0.8 CMake 集成

- [ ] `appcore/src/dock.c` 和 `appcore/src/dock_split.c` 加入 `dong_appcore` target
- [ ] `dong_dock.h` 随 appcore install
- [ ] dock_demo 加入 `add_appcore_example`

---

## P1: 分割线拖拽 + Tab 切换

### P1.1 分割线 (Divider) 交互

- [ ] 在 split_node_hit_test 中增加 divider 检测 (±4px 容差)
- [ ] 拖拽状态机: idle → hover_divider → dragging
  - dragging 时更新 split ratio, 实时 re-layout + re-blit
- [ ] 鼠标 cursor 切换: ns-resize / ew-resize (通过 SDL_SetCursor)
- [ ] 双击 divider → 重置 ratio 为 0.5

### P1.2 Tab Bar 渲染

- [ ] 对 tab_count > 1 的叶节点, 预留顶部 TAB_BAR_HEIGHT (28px)
- [ ] Tab bar 用简单的 GPU quad + 文字渲染 (可以用 dong engine 渲染一个小 HTML strip, 也可以直接 draw colored quads)
- [ ] 方案选择:
  - **方案 A**: 每个 tab bar 是一个隐藏的 mini dong_engine (HTML tab UI) — 灵活但重
  - **方案 B**: 直接用 SDL_GPU draw quads + glyph atlas 文字 — 轻量但功能少
  - 推荐 **方案 B**, tab bar 只需要: 背景色 + 文字 + active 高亮 + close 按钮

- [ ] Tab 点击: hit_test 检测 tab bar 区域 → 切换 active_tab
- [ ] Tab close 按钮 (×): 调用 pane_close callback → remove_pane

### P1.3 Pane 焦点指示

- [ ] 当前 focused pane 边框高亮 (1px accent color)
- [ ] 渲染到 composite 阶段, 在 blit 之后 draw 边框 quad

---

## P2: Tab 拖拽 + 吸附 + 布局持久化

### P2.1 Tab 拖拽

- [ ] mouse down on tab → 开始拖拽状态
- [ ] 拖拽阈值: 移动 >8px 才进入拖拽模式 (避免误触)
- [ ] 拖拽中:
  - 半透明预览: 渲染 pane texture 的缩小版 (或简单矩形 + title)
  - 可以用 SDL_SetWindowOpacity 做半透明浮动窗口, 也可以在当前窗口上叠加渲染
- [ ] 拖拽到窗口外: 自动 detach 成新窗口
- [ ] 释放在另一个窗口/pane 上: 自动 attach

### P2.2 吸附指示器 (Drop Indicator)

- [ ] 拖拽悬停时, 目标 pane 显示 5 个吸附区域 (上下左右+中心)
- [ ] 高亮当前激活的吸附方向 (半透明蓝色覆盖)
- [ ] 实现: 在 composite 阶段叠加渲染 indicator quads
- [ ] 吸附灵敏度: pane 四边各 25% 为 edge zone, 中心 50% 为 tab zone

### P2.3 Tab 重排序

- [ ] 在同一 tab bar 内拖拽 → 交换 tab 顺序
- [ ] 视觉: tab 跟随鼠标, 其他 tab 闪开 (动画可选)

### P2.4 布局持久化

- [ ] `dong_dock_save_layout`: 递归序列化 split tree 为 JSON
  ```json
  {
    "windows": [{
      "x": 100, "y": 100, "w": 1280, "h": 720,
      "root": {
        "type": "split_h", "ratio": 0.6,
        "children": [
          { "type": "leaf", "tabs": [{"title": "Editor"}] },
          { "type": "leaf", "tabs": [{"title": "Preview"}] }
        ]
      }
    }]
  }
  ```
- [ ] `dong_dock_load_layout`: 解析 JSON → 创建 window + split tree → 对每个 pane 调用 on_pane callback (让用户加载对应内容)
- [ ] JSON parser: 用 QuickJS 的 JSON.parse (engine 已有), 或者简单手写一个 (避免多余依赖)

---

## 文件清单

```
appcore/
├── include/
│   └── dong_dock.h          ✅ 已创建
├── src/
│   ├── dock.c               P0.1~P0.7 主实现
│   └── dock_split.c         P0.3 split tree 数据结构
examples/
│   └── dock_demo.cpp        P0.7 demo
```

## 依赖关系

无新外部依赖:
- SDL3: 已有, 支持多窗口 (SDL_ClaimWindowForGPUDevice)
- dong.dll: 已有, 多 engine + shared JS 已实现
- dong_sdl_backend.dll: 已有, offscreen rendering 已验证

## 风险点

| 风险 | 影响 | 缓解 |
|------|------|------|
| SDL_ClaimWindowForGPUDevice 多窗口兼容性 | 某些驱动可能不支持 | 测试 D3D12 / Vulkan 两种 backend |
| offscreen texture 在多窗口间 blit 性能 | pane 多时 GPU 带宽 | texture format 匹配 swapchain, 避免格式转换 |
| Platform 单例 (DongGPUDriver*) 被多窗口共用 | begin_frame_offscreen 有隐式状态 | 确认 offscreen 模式不依赖 window 指针 |
| dong_sdl_platform_shutdown 只能调一次 | 多窗口关闭顺序 | dock_destroy 统一管理, 只在最后调一次 |
