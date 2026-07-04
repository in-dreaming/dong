# T12 — HTML 内联事件 build 期提取

- **Area**: toolchain（Dong 构建工具链）
- **性质**: 纯实现型 — 对标物：QuickJS 运行时内联 handler 的行为（同一 HTML 在 QuickJS 路径的点击效果）；生成代码形态按 T15 决策记录执行
- **优先级**: P1
- **依赖**: T08（事件槽 API 定稿后，生成的 handler 才能读 event）、**T15**（跨模块状态模型定稿后，生成代码里的共享变量才有正确形态）
- **预估**: 2–3 人天
- **前置阅读**: `docs/developer/porffor/tasks/setup.md` §3 §4 §9

## 背景

QuickJS 路径靠运行时 `scanAndRegisterInlineEventHandlers` 支持 `<button onclick="count++">`。Porffor 无运行时 eval，评估结论：**build 期从 HTML 提取内联 handler，生成 Porffor 模块 + manifest 绑定，高可行，不改 Porffor**。

> 现状注意（事实 F1/F12）：当前管线是「每 handler 一个独立模块，文件体 = 模块 main」，没有 `export function` 语法支撑，且 handler 与主脚本**不共享全局变量**——下方示例里的 `count` 在 T15 落地前不是共享的。生成器必须按 T15 的结论产码：同模块多 export（方案 A）或 `dong_state_*` 状态槽调用（方案 B）。

## 工作内容

1. **扫描**：build 工具遍历 HTML，收集所有 `on*` 属性（onclick、oninput、onchange、onkeydown…集合与 T08 支持的事件类型对齐）与 `<script>` 内联块（后者归 T14，本任务只登记告警）。
2. **代码生成**：每个内联 handler 生成一个模块源文件：

```html
<button id="inc" onclick="count = count + 1; setText('label', count)">
```

   生成（示意）：

```js
// gen/inc__onclick.js
export function inc__onclick() {
  count = count + 1;
  setText('label', count);
}
```

   - export 命名规则：`<元素id>__<事件名>`；无 id 的元素 build 期自动注入生成 id（`__porf_auto_N`）。
   - handler 体原样搬运；顶部自动拼接 prelude import。
3. **manifest**：为每个 handler 追加 manifest 条目（nodeId 定位 + 事件类型 + export 名），复用现有 `data-porffor-module` 机制。
4. **HTML 清理**：产物 HTML 中移除 `on*` 属性（运行时不再有任何 handler 扫描）。
5. **失败策略**：handler 体含 Porffor 编译不过的语法（闭包引用局部变量等）→ 构建报错并输出文件/行号 + 指引（「改用全局变量/状态槽（T15）或逻辑下沉 C++」），不允许静默跳过（配合 T13 的 `dropped` 标记可显式豁免）。
6. **接管运行时 stub（F12）**：完成后移除/接管 `engine_view.cpp` 对 `scanAndRegisterInlineEventHandlers()` 的调用与 Porffor 侧的 warning stub。

## 验收标准

1. 含 `onclick`/`oninput` 的样例 HTML，一条构建命令完成 提取 → porf 编译 → manifest 输出，运行时点击行为与 QuickJS 参照版一致（对照录屏/多帧截图，QuickJS 退役前仍可作 baseline）。
2. 跨 handler 共享状态用例通过（`onclick="count++"` 两次点击后文本为 "2"，按 T15 模型产码）。
3. 无 id 元素的自动 id 注入用例通过。
4. 编译失败的 handler 报错信息含源 HTML 位置。
5. 标记为 `dropped`（T13 约定）的 HTML 被整体跳过。
6. 运行时 stub 调用点被接管/移除（F12）。
