# dong-preact

Lightweight Preact integration for the Dong HTML/CSS engine.

Preact uses its built-in DOM renderer — no custom reconciler needed.
Bundle sizes are ~85-90% smaller than the equivalent React versions.

## Quick Start (Zig Build Integration)

Preact examples are automatically copied to `zig-out/bin/data/preact-*/` during `zig build`.
Pre-built bundles in `dist/` are used by default.

```bash
cd dong

# Normal build — copies pre-built Preact bundles to zig-out/bin/data/
zig build

# Rebuild Preact bundles from source (requires Node.js)
zig build preact

# Run a Preact example
set DONG_SCRIPT_TIMEOUT_MS=10000
zig-out/bin/dong_app --html data/preact-counter/index.html
```

## Manual Build (without Zig)

```bash
cd dong/preact
npm install
npm run build:all    # or: node build.mjs counter
```

## Bundle Size Comparison

| Example      | React bundle | Preact bundle | Reduction |
|--------------|-------------|---------------|-----------|
| counter      | 210KB       | 22KB          | -89%      |
| game-ui      | 215KB       | 27KB          | -87%      |
| todo-classic | 217KB       | 36KB          | -83%      |

## Architecture

```
JSX components → esbuild bundle (IIFE) → <script src="bundle.js">
                                              ↓
                                         QuickJS eval
                                              ↓
                                  preact built-in DOM diff
                                              ↓
                                      Dong DOM API calls
                                              ↓
                                   Yoga Layout → GPU Render
```

Unlike React (which requires `react-reconciler` + a custom Host Config adapter),
Preact directly calls standard DOM APIs (`createElement`, `appendChild`,
`style.setProperty`, etc.) that Dong already implements.

## Examples

- **counter**: Minimal Preact + hooks demo
- **game-ui**: Game HUD with health bars, score, minimap, and inventory
- **todo-classic**: Class component todo app (via `preact/compat`)

## Build

```bash
node build.mjs <example-name>
```

Output goes to `dist/<name>/bundle.js`.
