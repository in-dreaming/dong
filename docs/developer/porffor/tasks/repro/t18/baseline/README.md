# T18 三示例 Preact vs Porffor baseline

视口 **1024×768**。Preact 为 Chrome/Playwright 参考帧；Porffor 为 `html_render_test` Dong 原生渲染。

| 示例 | Preact HTML | Porffor HTML | 帧数 |
|------|-------------|--------------|------|
| counter | `zig-out/bin/data/preact-counter/` | `zig-out/bin/data/porf-counter/` | 1 |
| todo-classic | `zig-out/bin/data/preact-todo-classic/` | `zig-out/bin/data/porf-todo-classic/` | 1 |
| game-ui | `zig-out/bin/data/preact-game-ui/` | `zig-out/bin/data/porf-game-ui/` | 3 @ 500ms（`setInterval` 分数 tick） |

## 生成命令

```powershell
. e:\env\activate-dong-build.ps1
cd E:\ws\infra\dong\dong
node scripts\porffor_compile.mjs
zig build preact
zig build

$out = "..\docs\developer\porffor\tasks\repro\t18\baseline"
$bin = "zig-out\bin"
# Preact Chrome baseline（需 Python + Playwright）
python scripts/tools/html_baseline_render.py "$bin\data\preact-counter\index.html" --out "$out\counter\preact_f0.png" --width 1024 --height 768 --wait-ms 550
# … todo-classic, game-ui 同理

# Porffor Dong 渲染
html_render_test.exe "$bin\data\porf-counter\index.html" "$out\counter\porf.bmp" 1024 768 1
html_render_test.exe "$bin\data\porf-game-ui\index.html" "$out\game-ui\porf.bmp" 1024 768 --frames 3 --frame-ms 500
```

## 产物

- `counter/preact_f0.png` + `counter/porf.bmp`
- `todo-classic/preact_f0.png` + `todo-classic/porf.bmp`
- `game-ui/preact_f0.png` + `game-ui/porf_f{0,1,2}.bmp`
