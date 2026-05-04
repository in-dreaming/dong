# P1-4 — Live Reload

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 1 P1-4
> 性能门槛：[`doc/perf_budget.md`](../perf_budget.md) § 3.2（live reload 端到端）
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

让 dong 业务方在编辑 HTML / CSS / JS / 图片资源时，**自动热更新**到运行中的 view，**500 ms 内** 看到效果，**尽量保留组件状态**：

- `dong_app --watch` 启动监听；改文件 → 自动 reload。
- React/Preact bundler watch 集成：bundle.js 变化触发。
- CSS Hot-swap（不重置 DOM 状态、不重置 JS 状态）。
- HTML / JS 变化时整 view reload（保留 scroll / focus 尽量恢复）。
- 图片资源变化时仅替换 atlas slot（不刷整页）。

---

## 2. 现状

| 项 | 状态 |
|---|---|
| 文件 watch | ❌ |
| `dong_engine_load_html` 重新加载 | ✅（但每次会丢失所有状态） |
| CSS-only 重应用 | ❌（任何 reload 走 full HTML 路径） |
| Image atlas 单条替换 | 部分（image_decoder 重 decode 即可，但需手动触发） |
| 状态保留（scroll/focus）| ❌ |

---

## 3. 设计

### 3.1 启动入口

```bash
# dong_app
dong_app --html my.html --watch                   # 监听 my.html 同目录所有资源
dong_app --html my.html --watch --watch-dir ./assets  # 加额外目录
dong_app --html my.html --watch --bundle-cmd "npm run watch"  # 同时 spawn bundler

# 也可走环境变量
DONG_HOT_RELOAD=1 dong_app --html my.html
```

### 3.2 Watch 实现

**优先**：复用 `efsw`/`watchexec`/原生 OS API（`inotify` / `FSEvents` / `ReadDirectoryChangesW`）。dong 自己**不**实现 watcher，引入轻量第三方库。

候选：
- [`efsw`](https://github.com/SpartanJ/efsw)（C++，BSD-2，跨平台，~2k 行）— 推荐
- 自带最小实现（仅 mtime poll，500 ms 周期）— fallback

启动后 dong_app 内一个 watcher 线程：

```cpp
watcher.add(html_dir);
watcher.on_change([&](const std::string& path) {
    main_loop_queue.push(ReloadEvent{path});
});
```

事件投递到主线程消费，避免在 watcher 线程内调 dong API。

### 3.3 文件类型分派

| 扩展 | 策略 |
|---|---|
| `.html` | Full reload + 状态保留尝试 |
| `.css`（`<link>` 引用的） | CSS hot-swap（不重 DOM） |
| `.js` | Full reload（QuickJS 模块状态难局部保留） |
| `bundle.js` (React/Preact) | Full reload + 由 React 自身 hooks 保留状态（与社区方案一致） |
| `.png/.jpg/.svg/.webp` | Atlas slot 替换 |
| `.ttf/.otf` | 重新 register font + 全文字 invalidate |
| 其他 | 忽略 |

判定方式：URL/路径正则 + content-type；HTML 内引用过的资源才纳入。

### 3.4 状态保留契约

Reload 时主动保存 / 恢复：

| 状态 | 保存 | 恢复 |
|---|---|---|
| `window.scrollX/Y` | 每次 reload 前读 | reload 后写回 |
| `document.activeElement` | 取 selector（id 或 querySelector path） | reload 后查找 + focus() |
| Focus selection range | offset 保存 | 恢复 |
| JS `localStorage` / `sessionStorage` | 自然保留（不主动清） | — |
| React/Preact `useState` | 由 React Refresh 保留（启用 `--bundle-cmd "npm run watch"` 自带） | — |
| Scroll position（含子滚动容器） | 遍历 scrollable 收集 | 恢复 |
| `<video>` 进度 | 暂不保留 | — |
| `<input>` 未提交值 | 保留 | 恢复 |

提供 JS hook：

```js
window.dong = window.dong || {};
window.dong.onBeforeReload = (saveState) => {
    saveState({ myKey: someValue });
};
window.dong.onAfterReload = (state) => {
    if (state.myKey) restore(state.myKey);
};
```

应用复杂状态自己负责。

### 3.5 CSS Hot-swap

CSS reload **不**走 full HTML reload：

1. Watcher 检测 `assets/style.css` 变更。
2. 主线程：
   - 读新内容
   - 找到对应 `CSSStyleSheet` 对象（按 href 匹配；inline `<style>` 不在此路径）
   - 调 `stylesheet.replaceWith(new_text)` （等同 W3C `CSSStyleSheet.replaceSync`）
   - StyleEngine `invalidateAllRules`（仅 selector match 重跑；DOM/Layout 增量）
   - 与 [P0-6](../phase0/P0-6_partial_damage_rect.md) 协作：触发 Style invalidation kind
3. 当帧 paint 反映新样式

CSS hot-swap **不丢任何 DOM/JS 状态**；改 color / animation / margin 等用户体验最佳。

### 3.6 HTML / JS 整 view reload

```cpp
on_html_change(path):
    snapshot = capture_state(view);   // scroll/focus/etc.
    new_html = read_file(path);
    dong_engine_load_html(view, new_html);
    queue_after_first_paint([snapshot] {
        restore_state(view, snapshot);
    });
    log_info("Reloaded {} in {} ms", path, dur_ms);
```

`queue_after_first_paint` 用既有 `MutationObserver` / `requestAnimationFrame` 等价机制保证布局稳定后再恢复。

### 3.7 图片资源替换

Watcher → `dong_engine_invalidate_image(view, src)` → ImageDecoder 重 decode → atlas 同槽位替换 → `markNeedsRepaint`。

DOM 树不动；只触发 paint kind invalidation。

### 3.8 Bundler 集成（React/Preact）

```bash
dong_app --html data/react-app/index.html --bundle-cmd "cd data/react-app && npm run watch"
```

dong_app 同时 fork bundler 子进程；bundler 输出的 `bundle.js` 由 watcher 监测；变化即触发 full reload。

React Refresh / Preact Refresh 集成留给 bundler 自身（esbuild 有插件；vite 自带）；dong 不掺和。

### 3.9 错误时的优雅降级

| 场景 | 行为 |
|---|---|
| HTML 解析失败 | 保留旧 view + console.error；不黑屏 |
| CSS 解析失败 | 同上；保留旧 stylesheet |
| JS eval 失败 | 同上；console.error |
| 图片 decode 失败 | 保留旧 atlas |
| Watcher 自身崩 | log warn + 退化为 manual mode；不影响 view 运行 |

### 3.10 与 DevTools 的协同

DevTools Console panel 显示 reload 事件：

```
[12:34:56] [reload] CSS hot-swap: assets/style.css (12ms)
[12:35:02] [reload] Full reload: index.html (180ms)
[12:35:08] [reload] Image: assets/icon.png (45ms)
```

每次 reload 后 perf counter 重置，DevTools 自动捕获。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — Watcher 基础（efsw 集成 + 主线程事件队列）** | dong_app `--watch` 启动；console log 打印变更事件 |
| **S2 — Full HTML reload** | watcher → load_html；无状态保留 |
| **S3 — 状态保留（scroll/focus/input）** | 加 capture/restore；JS hook |
| **S4 — CSS hot-swap** | StyleSheet.replaceWith；与 P0-6 invalidation 联动 |
| **S5 — 图片资源 invalidate** | image atlas slot 替换 |
| **S6 — `--bundle-cmd` spawn 子进程 + bundle.js watch** | React/Preact demo 验证 |
| **S7 — DevTools 集成 + 错误降级** | log channel；优雅 fallback |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 编辑 `.html` 文件保存后端到端到画面更新 | ≤ 500 ms（Soft 见 perf_budget） |
| CSS hot-swap 不丢 scroll / focus / input value | 必须 |
| CSS hot-swap 不重建 DOM 树（DevTools 验证 node_id 不变） | 必须 |
| HTML reload 后 scroll / focus / input value 自动恢复（前提 selector 仍可命中） | 必须 |
| 图片资源替换不重 layout（仅 paint） | 必须；DevTools damage rect 仅 paint kind |
| 解析失败时保留旧 view + console.error | 必须 |
| 关闭 `--watch` 时性能与未集成 watcher 一致 | 必须 |
| watcher 线程崩溃不导致 dong_app 崩溃 | 必须 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| HTML reload 端到端 | ≤ 500 ms（Soft）/ ≤ 1500 ms（Hard，见 perf_budget S2）|
| CSS hot-swap 端到端 | ≤ 100 ms |
| 图片替换端到端 | ≤ 200 ms |
| Bundler watch 联动延迟 | ≤ 1000 ms（含 bundler build 时间） |
| Watcher CPU 占用 | < 1%（idle） |

### 5.3 必须新增的测试

| 测试 | 内容 |
|---|---|
| `tests/reload/test_watcher_basic.cpp` | watcher 触发；不卡死；线程安全 |
| `tests/reload/test_html_reload_state.cpp` | scroll / focus / input value 恢复 |
| `tests/reload/test_css_hot_swap.cpp` | DOM 不重建；样式新值生效 |
| `tests/reload/test_image_invalidate.cpp` | atlas 替换；不 relayout |
| `examples/data/reload/test_reload_app.html` | 手动 sanity demo |
| `dong/scripts/test_reload_e2e.py` | 自动改文件 + 测延迟 |

### 5.4 验证命令

```bash
# 自动 e2e
python dong/scripts/test_reload_e2e.py
# 期望：全部 < 500ms

# Bundler 集成
dong_app.exe --html data/react-counter/index.html \
   --watch --bundle-cmd "cd data/react-counter && npx esbuild --watch"
# 修改 src/Counter.jsx → 期望计数 state 保留（React Refresh）

# 错误降级
echo "<<<broken>>>" > data/test.html
# 期望：旧 view 保留 + console.error
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| `efsw` 在某些 Linux 文件系统（NFS / 容器 overlay）不可靠 | fallback 到 mtime poll；env var 切换 |
| Editor "atomic save"（写临时 + rename）触发多次事件 | watcher 内部 250 ms debounce |
| CSS hot-swap 对 inline `<style>` 不生效（仅 `<link>` 跟踪） | 文档明示；inline style 改动需走 full reload |
| State restore 命中失败（selector 找不到）静默 | DevTools console.warn |
| QuickJS reload 后 `setInterval` / `requestAnimationFrame` 残留 | dong 在 reload 前 cancel 全部 timer / rAF |
| Bundler 子进程 zombie | dong_app 退出时 SIGTERM bundler |
| 监听目录过大 | 默认仅监听 HTML 同目录 + 显式 `--watch-dir`；不递归用户主目录 |

---

## 7. 不在本方案范围

- ❌ 远程 reload（开发机改文件 → 设备热更新；走 P2 / 业务方自建）
- ❌ Module Hot Replacement（HMR 级别 ESM 模块替换；dong 不用 ESM）
- ❌ JS 状态完整保留（QuickJS 没有 Snapshot；只能依赖 React Refresh）
- ❌ Time-travel debugging
- ❌ Production 模式 hot reload（仅开发工具，release build 不带 watcher）

---

## 8. 完成后更新

- [ ] `doc/重要特性.md` 新增 § "Live Reload"
- [ ] `doc/roadmap.md` Phase 1 P1-4 状态 ✅
- [ ] `doc/perf_budget.md` § 3.2 live reload 实测填入
- [ ] `dong/CLAUDE.md` 与 `dong/AGENTS.md` 加 `dong_app --watch` 条目
- [ ] React/Preact examples README 加 watch 用法
