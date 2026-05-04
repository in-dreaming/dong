# dong-react

React renderer for the Dong HTML/CSS engine via `react-reconciler`.

## Quick Start (Zig Build Integration)

React examples are automatically copied to `zig-out/bin/data/react-*/` during `zig build`.
Pre-built bundles in `dist/` are used by default.

```bash
cd dong

# Normal build — copies pre-built React bundles to zig-out/bin/data/
zig build

# Rebuild React bundles from source (requires Node.js)
zig build react

# Run a React example
set DONG_SCRIPT_TIMEOUT_MS=10000
zig-out/bin/dong_app --html data/react-counter/index.html
```

## Manual Build (without Zig)

```bash
cd dong/react
npm install
npm run build:all    # or: node build.mjs counter
```

## Architecture

```
JSX components → esbuild bundle (IIFE) → <script src="bundle.js">
                                              ↓
                                         QuickJS eval
                                              ↓
                                     react-reconciler diff
                                              ↓
                                   dong-react-renderer (Host Config)
                                              ↓
                                      Dong DOM API calls
                                              ↓
                                   Yoga Layout → GPU Render
```

See `doc/arch/react.md` for the full architecture document.

## Examples

- **counter**: Minimal React + useState demo
- **game-ui**: Game HUD with health bars, score, minimap, and inventory

## Build

```bash
node build.mjs <example-name>
```

Output goes to `dist/<name>/bundle.js`.
