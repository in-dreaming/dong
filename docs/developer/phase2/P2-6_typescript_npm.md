# P2-6 — TypeScript 类型 + npm 包发布

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 2 P2-6
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

把 dong 的所有 JS 层 API 公开为可在 npm 安装的、带完整 TypeScript 类型的包：

- `@dong/core-types`：dong 注入到 QuickJS 的全局 API 类型（`dong.*` `document.*` 子集差异等）
- `@dong/react`：react-reconciler Host Config + 工具函数（已有，加 types + 发布）
- `@dong/preact`：Preact 适配（已有，加 types + 发布）
- `@dong/ui`：组件库（与 P2-5 协同）
- `@dong/cli`：脚手架 `npm create dong-app`

让业务方在自家项目里用 `npm install @dong/core-types @dong/react` 即可享受 IntelliSense / 类型检查。

---

## 2. 现状

| 项 | 现状 |
|---|---|
| dong/react/ | ✅，esbuild bundle，无 npm 发布 |
| dong/preact/ | ✅，同上 |
| TypeScript types | ❌（`.d.ts` 一份没有） |
| npm 账号 / org | ❌（需要注册 `@dong` org） |
| 模板项目 | ❌（业务方手写 esbuild config） |

---

## 3. 设计

### 3.1 包目录结构

新增（与现有 `dong/react/` `dong/preact/` 平级）：

```
dong/packages/
├── core-types/
│   ├── package.json
│   ├── tsconfig.json
│   ├── src/
│   │   ├── globals.d.ts          # window.dong / window.document 子集差异
│   │   ├── overlay.d.ts          # dong.drawRect 等
│   │   ├── scene-graph.d.ts      # dong.scene.*
│   │   ├── text-layout.d.ts      # dong.textLayout / renderText
│   │   ├── inspector.d.ts        # devtools 接口（受限暴露）
│   │   └── index.d.ts
│   └── README.md
│
├── react/                         # 移自 dong/react/
│   ├── package.json
│   ├── tsconfig.json
│   ├── src/
│   │   ├── dong-react-renderer.ts # 重写为 .ts
│   │   ├── polyfills.ts
│   │   └── index.ts
│   └── tests/
│
├── preact/                        # 移自 dong/preact/
│
├── ui/                            # P2-5 产出
│
└── cli/
    ├── package.json
    ├── bin/
    │   └── create-dong-app.js
    ├── templates/
    │   ├── vanilla/
    │   ├── react/
    │   ├── preact/
    │   └── react-ui/
    └── README.md
```

`dong/react/` 与 `dong/preact/` 老路径保留兼容期；新版从 `packages/` 走。

### 3.2 monorepo 工具

用 [`pnpm` workspaces](https://pnpm.io/workspaces)（轻量）：

```yaml
# pnpm-workspace.yaml
packages:
  - 'dong/packages/*'
  - 'dong/dong-ui'
```

构建工具：esbuild（与 dong/react 一致）+ tsup（type 生成）。

### 3.3 `@dong/core-types` 内容

**最关键**：业务方写 dong app 必装。包含：

```ts
// globals.d.ts
declare global {
    interface Window {
        dong: DongGlobal;
    }
}

export interface DongGlobal {
    // 已经在 dong 内的全局
    drawRect(x: number, y: number, w: number, h: number, color: string): void;
    drawCircle(x: number, y: number, r: number, color: string): void;
    renderText(opts: RenderTextOptions): void;
    clearOverlay(): void;

    // Scene Graph
    scene: {
        find(name: string): SceneNode | null;
        set(id: number, prop: string, value: unknown): void;
        on(id: number, event: string, fn: (e: SceneEvent) => void): void;
    };

    // Text layout
    textLayout(opts: TextLayoutOptions): TextLayoutResult;

    // Focus nav (P0-4)
    focusNav(dir: 'up' | 'down' | 'left' | 'right' | 'next' | 'prev'): void;

    // HDR (P1-6)
    queryHDRCapabilities(): DongHDRCapabilities | null;

    // World Space (P2-1/2/3) - 通常不在 JS 用，标可选
}
```

并提供 dong **特有事件类型**：

```ts
// 标准 web 类型 + 扩展
export interface DongCompositionEvent extends CompositionEvent {
    /* dong 特有字段 */
}
export interface DongGamepadEvent extends Event {
    button: 'A' | 'B' | 'X' | 'Y' | 'DpadUp' | 'DpadDown' | ...;
    pressed: boolean;
    gamepadId: number;
}
```

### 3.4 `@dong/react` types

把现有 `dong-react-renderer.js` 改写 TypeScript，导出严格类型：

```ts
import type { ReactNode } from 'react';

export interface DongRenderOptions {
    onError?: (err: Error) => void;
    debug?: boolean;
}

export function render(element: ReactNode, container: Element, opts?: DongRenderOptions): RootHandle;
export function unmountComponentAtNode(container: Element): void;

// JSX 内 dong 特有 props 增量
declare module 'react' {
    interface CSSProperties {
        '--hdr-boost'?: number;
        '--du-color-primary'?: string;
        // ... 其他 dong 私有 CSS variable
    }
    interface DOMAttributes<T> {
        onCompositionStart?: (e: DongCompositionEvent) => void;
        onCompositionUpdate?: (e: DongCompositionEvent) => void;
        onCompositionEnd?: (e: DongCompositionEvent) => void;
        onGamepadButton?: (e: DongGamepadEvent) => void;
    }
    interface IntrinsicElements {
        'host-view': React.HTMLAttributes<HTMLElement> & {
            slot?: string;
            'intrinsic-width'?: number;
            'intrinsic-height'?: number;
        };
        'dong-lottie': React.HTMLAttributes<HTMLElement> & {
            src: string;
            loop?: boolean | number;
            speed?: number;
            autoplay?: boolean | number;
        };
        'dong-rive': React.HTMLAttributes<HTMLElement> & {
            src: string;
            'state-machine'?: string;
        };
    }
}
```

业务方装 `@dong/react` 后写 JSX：

```tsx
// 自动获得 host-view / dong-lottie 的类型补全和检查
<host-view slot="3d-preview" intrinsic-width={400} intrinsic-height={300} />
<dong-lottie src="loading.json" loop autoplay />
```

### 3.5 `@dong/cli`

```bash
npm create dong-app my-app
# 交互式：
#   ? 模板: vanilla | react | preact | react+ui
#   ? 包名: my-app
# 自动生成：
#   - package.json
#   - vite/esbuild 配置
#   - HTML 入口
#   - 示例代码
#   - dong_app launch script
```

模板示例（react+ui）：

```
my-app/
├── package.json                  # 依赖 @dong/core-types @dong/react @dong/ui react
├── tsconfig.json
├── esbuild.config.mjs
├── src/
│   ├── main.tsx
│   └── App.tsx
├── public/
│   └── index.html
└── README.md                     # 含 dong_app 启动指引
```

### 3.6 版本与发布

- 所有包跟随 dong release tag 同版本（如 `1.0.0`）。
- 发布到公开 npm（首次需注册 `@dong` org）。
- `prepublishOnly` 跑 `tsc --noEmit + tests`。
- 自动化：GitHub Action 在 release 时 `npm publish`。

### 3.7 与 dong native 的兼容性矩阵

文档清单（`compatibility.md`）：

| @dong/core-types 版本 | 兼容 dong native 版本 |
|---|---|
| 1.0.x | dong 1.0.x |
| 1.1.x | dong 1.0.x – 1.1.x |
| ... | ... |

类型若引用了 dong 未实现的 API → 标 `@experimental` 或 `// @minVersion 1.2.0`。

### 3.8 与 P2-5 dong-ui 的协调

dong-ui 依赖 `@dong/core-types` + `@dong/react`（peer dependency）。

发布顺序：
1. `@dong/core-types` 先发
2. `@dong/react` `@dong/preact` 跟进
3. `@dong/ui` 最后
4. `@dong/cli` 任何时候

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — `@dong/core-types` 包** | 完整 globals.d.ts + 所有 dong 已暴露的 JS API 类型 |
| **S2 — `@dong/react` TS 重写 + 发布** | dong-react-renderer.ts；jest/vitest 单元 |
| **S3 — `@dong/preact` 同步** | 包装 |
| **S4 — `@dong/cli`** | create-dong-app + 4 模板 |
| **S5 — 发布到 npm + GitHub Actions 自动化** | 含 `prepublishOnly` 检查 |
| **S6 — `@dong/ui` 类型 + 发布** | 与 P2-5 联动 |
| **S7 — 兼容矩阵 + 文档** | 写完整 README |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| 4 个 npm 包都能 `npm install` 成功 | 必须 |
| `npm create dong-app my-app` 端到端：30 秒内创建可运行项目 | 必须 |
| `tsc --noEmit` 在所有包内通过 | 必须 |
| 模板项目 `npm run dev` 可启动 + dong_app 可加载 | 必须 |
| `host-view` `dong-lottie` `dong-rive` 在 JSX 内有类型检查与补全 | 必须 |
| dong 私有 CSS variable 在 `style={{...}}` 内有补全 | 必须 |
| GitHub Actions 在 git tag 后自动发布 npm | 必须 |
| 包大小（minified）：core-types < 5 KB；react < 30 KB；ui < 100 KB | 必须 |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 业务方安装到使用 < 5 分钟 | 期望 |
| Type 完整覆盖率 ≥ 90% | 期望 |
| README 含 quickstart + advanced | 必须 |
| 4 个模板代表四种典型用例 | 必须 |
| npm 包描述、关键字 SEO 友好 | 期望 |

### 5.3 必须新增的产物

| 产物 | 说明 |
|---|---|
| `dong/packages/core-types/` | 完整类型定义 |
| `dong/packages/react/` | TS 重写 + tests |
| `dong/packages/preact/` | 同上 |
| `dong/packages/cli/` | scaffold 工具 |
| `.github/workflows/npm-publish.yml` | 自动发布 |
| `dong/packages/README.md` | monorepo 总入口 |
| `docs/compatibility.md` | 版本矩阵 |

### 5.4 验证命令

```bash
# 本地构建
cd dong && pnpm install && pnpm -r build

# 类型检查
pnpm -r exec tsc --noEmit

# 测试
pnpm -r test

# 端到端 scaffold 验证
npx create-dong-app /tmp/test-app --template react+ui
cd /tmp/test-app && npm run dev

# 发布 dry-run
pnpm -r publish --dry-run
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| `@dong` npm org 名被占 | 提前注册；备用名 `@dong-engine` |
| 类型与实际 dong native 行为不符 | 自动化：每次 release 跑兼容性测试（用 type 写一段 dong app 跑） |
| 业务方 lock 旧 type 但 native 升级不兼容 | 兼容矩阵 + semver；major bump 强制业务侧迁移 |
| TypeScript 学习成本拒劝业务方 | vanilla 模板不强制 TS；types 是可选增益 |
| 模板与实际 dong_app 启动方式脱节 | 模板含 `npm run dong:dev` 脚本，自动找到 dong_app 二进制 |
| 包发布权限滥用 | npm 2FA + GitHub Actions Token 单独管理 |

---

## 7. 不在本方案范围

- ❌ Vite plugin / Webpack loader（业务侧用现成 esbuild）
- ❌ Storybook 集成
- ❌ Visual regression in CI（业务侧用 dong baseline 工具自己做）
- ❌ 在线 playground（StackBlitz 等；P3 评估）
- ❌ Pre-built binaries（dong_app 还是各自下载；不打进 npm）
- ❌ 自带 i18n / Redux / Zustand 适配
- ❌ ESM 直接在 dong 内运行（dong QuickJS 仍走 IIFE）

---

## 8. 完成后更新

- [ ] `docs/reference/features-index.md` 新增 § "TypeScript / npm 生态"
- [ ] `docs/developer/arch/react-reconciler-spec.md` § 7 用法替换为 npm install + tsx
- [ ] `dong/CLAUDE.md` `dong/AGENTS.md` 加 npm 包安装段
- [ ] dong 官网（如有）加 quick start
