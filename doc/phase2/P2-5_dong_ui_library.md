# P2-5 — `dong-ui` 官方组件库

> 上游：[`doc/roadmap.md`](../roadmap.md) Phase 2 P2-5
> 灵感：[`doc/ideal/frame.md`](../ideal/frame.md)（mui/base-ui）
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

为 dong 业务方提供一套**开箱即用的 UI 组件库**，避免每个项目从 `<button style="...">` 重新写起：

- 20+ 高频组件（Button / Input / Select / Slider / Tabs / Dialog / Tooltip / Toast / Tree / Menu / DropdownMenu / RadioGroup / Switch / Checkbox / Avatar / Badge / Card / Progress / Spinner / Form / Table）。
- **Headless + 主题分离**：核心组件不带样式，3 套官方主题（`clean` / `cyberpunk` / `medieval`）。
- 支持 React / Preact / Vanilla 三种用法。
- 完整支持 dong 三轨：DOM / Scene Graph / Direct Draw（仅适用的组件）。
- 无障碍 / 键盘导航 / Spatial Nav（手柄）默认开启。

---

## 2. 现状

- 业务方各自写组件；体验不一致。
- React/Preact 示例（`dong/react/examples/`）有局部组件但不成体系。
- `doc/ideal/frame.md` 仅提到 base-ui 链接。
- 没有官方 npm 包。

---

## 3. 设计

### 3.1 仓库结构

新增：`dong/dong-ui/`（与 `dong/react/` `dong/preact/` 平级）

```
dong/dong-ui/
├── package.json
├── README.md
├── core/                        # headless logic（不依赖 React/Preact）
│   ├── button.ts
│   ├── select.ts
│   ├── ...
│   └── index.ts
├── react/                       # React adapters
│   ├── Button.tsx
│   └── ...
├── preact/                      # Preact adapters（轻包装）
├── themes/
│   ├── clean/
│   │   ├── index.css
│   │   └── tokens.css           # CSS variables
│   ├── cyberpunk/
│   └── medieval/
├── examples/
│   ├── all-components.html
│   ├── react-app/
│   └── preact-app/
├── tests/
└── build.mjs                    # esbuild
```

### 3.2 设计哲学

借鉴 [Radix UI](https://radix-ui.com) / [Base UI](https://mui.com/base-ui/) / [Headless UI](https://headlessui.com)：

- **Headless core**：组件只负责行为 + ARIA + 键盘 + 状态机，不带视觉。
- **Theme override**：通过 CSS variables + class hooks 让业务自由换肤。
- **Composition over inheritance**：每个组件由若干小 part 拼成（`Tabs.Root` / `Tabs.List` / `Tabs.Trigger` / `Tabs.Panel`）。
- **dong 友好默认**：默认开启 spatial nav、IME、focus visible。

### 3.3 组件清单（v1）

| 类别 | 组件 |
|---|---|
| **基础控件** | `Button` `IconButton` `Input` `Textarea` `NumberInput` `Switch` `Checkbox` `RadioGroup` `Slider` `Select` `Combobox` |
| **导航** | `Tabs` `Menu` `DropdownMenu` `Breadcrumb` |
| **反馈** | `Dialog` `AlertDialog` `Tooltip` `Toast` `Popover` `Progress` `Spinner` |
| **数据展示** | `Avatar` `Badge` `Card` `Table`（含虚拟滚动）`Tree`（含虚拟滚动）|
| **表单** | `Form`（验证集成）`FormField` `Label` |
| **布局** | `Stack`（Flex shorthand）`Grid`（grid shorthand）`Separator` |
| **游戏专用** | `HealthBar`（圆形 / 横条）`SkillButton`（带 CD）`MinimapFrame` `DamageNumber`（基于 P2-1 World Text 包装）|

总计 **40+ 组件**；v1 至少交付 20 个核心 + 5 个游戏专用。

### 3.4 主题系统

```css
/* themes/_tokens.css 全部 token 定义 */
:root {
    --du-color-primary: #5b9bd5;
    --du-color-primary-fg: white;
    --du-color-bg: #1e1e1e;
    --du-color-fg: #e0e0e0;
    --du-color-border: #333;
    --du-color-danger: #d9534f;
    --du-color-success: #5cb85c;

    --du-radius-sm: 4px;
    --du-radius-md: 8px;
    --du-radius-lg: 12px;

    --du-spacing-xs: 4px;
    --du-spacing-sm: 8px;
    --du-spacing-md: 16px;
    --du-spacing-lg: 24px;

    --du-font-family: "Inter", system-ui, sans-serif;
    --du-font-size-sm: 12px;
    --du-font-size-md: 14px;
    --du-font-size-lg: 16px;

    --du-shadow-sm: 0 1px 2px rgba(0,0,0,0.1);
    --du-shadow-md: 0 4px 8px rgba(0,0,0,0.15);
}
```

业务方覆盖 token：

```css
:root {
    --du-color-primary: #ff5722;   /* 业务自定义橙色主题 */
}
```

3 套主题：

| 主题 | 风格 |
|---|---|
| `clean` | 现代扁平（中性灰 + 蓝 accent；适合 editor） |
| `cyberpunk` | 霓虹 + 锐利（适合科幻游戏 HUD） |
| `medieval` | 暖色 + 装饰边框（基于 9-slice panel；适合 RPG / 卡牌） |

### 3.5 React 用法

```jsx
import { Button, Dialog, Select } from '@dong/ui';
import '@dong/ui/themes/clean/index.css';

function App() {
    return (
        <Dialog>
            <Dialog.Trigger asChild>
                <Button variant="primary">Open</Button>
            </Dialog.Trigger>
            <Dialog.Content>
                <Dialog.Title>Settings</Dialog.Title>
                <Select defaultValue="en">
                    <Select.Item value="en">English</Select.Item>
                    <Select.Item value="zh">中文</Select.Item>
                </Select>
            </Dialog.Content>
        </Dialog>
    );
}
```

### 3.6 Vanilla 用法

```html
<link rel="stylesheet" href="@dong/ui/themes/clean/index.css">
<script type="module" src="@dong/ui/vanilla.js"></script>

<du-button variant="primary">Click</du-button>
<du-dialog>
    <du-dialog-trigger><du-button>Open</du-button></du-dialog-trigger>
    <du-dialog-content>
        <du-select>
            <option value="en">English</option>
            <option value="zh">中文</option>
        </du-select>
    </du-dialog-content>
</du-dialog>
```

实现：基于 dong 的标准 DOM + 自定义元素（自定义 element 由 dong-ui 注册到 `customElements`）。

> 注：dong v1 的 `customElements` 支持有限；本任务可能需要先补 Phase 0 已有的 P0-8 / Phase 1 P1-9 范围之外的 customElements 基础（评估后单独立任务）。

### 3.7 游戏专用组件（关键差异化）

#### `<HealthBar>`

```jsx
<HealthBar
  value={70} max={100}
  variant="circular"           // | "horizontal"
  thickness={8}
  color="#d9534f"
  showLabel
  labelFormat="{value}/{max}"
/>
```

实现：用 [P0-3 conic-gradient + mask](../phase0/P0-3_mask_and_conic_gradient.md) fast path → 单 draw call。

#### `<SkillButton>`

```jsx
<SkillButton
  icon="fireball.png"
  cooldownProgress={0.4}       // 0..1
  cooldownDuration={5000}
  hotkey="Q"
  onActivate={() => { ... }}
/>
```

带技能图标 + CD 圆形遮罩（conic）+ 快捷键提示 + 可点击。

#### `<DamageNumber>`

包装 P2-1 `dong_world_text_t`：

```jsx
<DamageNumber
  worldX={5} worldY={2} worldZ={-3}
  value={1234}
  color="#ff0"
  driftY={-50}            // 上飘 50 px
  fadeDuration={1000}
/>
```

### 3.8 测试与文档站

- 每个组件含 unit tests（jest / vitest）+ baseline pixel tests（dong 自己跑）。
- 文档站本身用 dong-ui + dong 写（dogfood）；含 live playground、props 表、示例代码。

```
dong/dong-ui/docs/                   # docs site source
   ├── components/...                # MDX
   └── playground/...
```

输出 static site，发布到 GitHub Pages 或 dong 官网。

### 3.9 与 P0-4 Spatial Nav 协同

每个交互组件默认设置：
- `tabindex="0"` 或可被 spatial nav 命中。
- `data-nav-default` 在容器组件（如 Dialog）上指定首焦点。
- A 键 = activate；B 键 = cancel（dialog 关闭等）。

### 3.10 与 P1-3 DevTools 协同

DevTools Element panel 支持识别 dong-ui 组件名（通过 `data-du-component="Button"` 标签）；点击直接跳转到该组件源码（v2）。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — 仓库骨架 + 5 个核心 (Button/Input/Switch/Checkbox/RadioGroup)** | clean 主题；React + Vanilla |
| **S2 — 反馈类 (Dialog/Tooltip/Toast/Popover)** | 含 portal / focus trap |
| **S3 — Select/Combobox/Menu/DropdownMenu** | 配合 P0-4 spatial nav |
| **S4 — 数据展示 (Table/Tree 含虚拟滚动) + Tabs/Breadcrumb** | 含 1k 行表格 perf |
| **S5 — Form 验证集成** | 联动 P1-9 表单验证 |
| **S6 — 游戏专用 (HealthBar/SkillButton/DamageNumber)** | 复用 P0-3 / P2-1 |
| **S7 — cyberpunk + medieval 主题** | nine-slice panel 配合 |
| **S8 — TypeScript types + npm 发布** | 与 P2-6 协同 |
| **S9 — 文档站 + 示例** | dogfood |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 至少 25 个组件交付（含 5 个游戏专用） | 必须 |
| 每个组件含 unit test + 像素 baseline | 必须 |
| 所有交互组件支持键盘 + spatial nav（手柄） | 自动测试 |
| 所有 Dialog 类支持 focus trap + ESC 关闭 | 必须 |
| 3 套主题 token 完整覆盖（无 fallback to default 漏） | 必须 |
| TypeScript types 完整 + `tsc --noEmit` 通过 | 必须 |
| `npm install @dong/ui` + import 跑通 | 必须 |
| 文档站本身用 dong-ui 渲染 + 可访问 | 必须 |
| 1k 行 Table 滚动 ≥ 60 FPS | perf |
| HealthBar 圆形 100 个 ≥ 144 FPS | perf |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| Bundle size（核心 + clean 主题） | < 80 KB minified gzipped |
| 所有组件 a11y axe-core 测试无 critical | 期望 |
| Storybook / 文档站每个组件 ≥ 3 个 example | 期望 |
| 至少 30 个组件 | 期望 |
| 业务方 npm 下载量（半年内）| ≥ 100/周 |

### 5.3 必须新增的测试

| 类别 | 内容 |
|---|---|
| Unit tests | 每个组件 Mount/Props/Events |
| Pixel baseline | 每个组件 default state 一张图 |
| Interaction | Dialog open/close, Select 选择, Tabs 切换 |
| Spatial nav | 5×5 button 网格 dpad 跳转 |
| Theme | 三套主题分别截图比对 |
| Game components | HealthBar 0% / 50% / 100% 三态 |

```bash
# Unit
cd dong/dong-ui && npm test

# Baseline (用 dong 自己跑)
python dong/scripts/tools/run_baseline_compare.py --filter dong_ui_

# Spatial nav E2E
python dong/scripts/test_dong_ui_spatial_nav.py
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| 组件数量大、维护成本高 | 严格头部清单；非清单内组件不接 PR |
| 主题切换性能差（CSS 变量太多）| 限定 token 数量 ≤ 100；测 perf |
| 无 React/Preact 包装的 vanilla 路径耦合不一致 | core 必须 framework-agnostic；React/Preact 是薄包装 |
| 文档站维护负担 | 用 markdown + dogfood，结构稳定后写一次维护成本低 |
| 业务方拒绝用，自己写 | 主动收业务方反馈；提供"渐进采用"路径（一个组件起手） |
| TypeScript 类型与 dong 自身不同步 | 与 P2-6 联动 |
| 游戏专用组件依赖 P2-1 | 严格依赖标识；P2-1 未完成 → 这些组件先标 experimental |

---

## 7. 不在本方案范围

- ❌ 完整 Storybook 集成（用 dogfood 文档站替代）
- ❌ Figma → component 自动生成
- ❌ Animation system（业务侧用 CSS animation 即可）
- ❌ i18n（业务侧自己装 i18next 等）
- ❌ Date picker / Color picker（复杂度高；留 v2）
- ❌ Drag-and-drop framework（业务侧用 dndkit 等）
- ❌ Charts / Maps（不是 UI 库范畴）

---

## 8. 完成后更新

- [ ] `doc/重要特性.md` 新增 § "dong-ui 组件库"
- [ ] `doc/positioning.md` § 5.2 P2-5 行从"待做"改"已交付"
- [ ] `doc/ideal/frame.md` 替换为正式 dong-ui 介绍
- [ ] React/Preact examples README 加 dong-ui 用法
- [ ] npm 包发布通告（与 P2-6 同步）
