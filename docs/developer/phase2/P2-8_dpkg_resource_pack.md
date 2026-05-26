# P2-8 — GPU 资源打包 `.dpkg`

> 上游：[`docs/roadmap.md`](../roadmap.md) Phase 2 P2-8
> 性能门槛：[`docs/developer/perf-budget.md`](../perf_budget.md) § 5（启动 < 200 ms）
>
> 状态：方案 v1 / 2026-04-17

---

## 1. 目标

提供 dong 的"资源包"格式 `.dpkg`，把字体 / 图片 atlas / shader / HTML / JS bundle 等离线打包，启动时**直接 mmap + 上传 GPU**：

- 启动到首帧 ≤ 200 ms（含字体 + 图片 + bundle eval）
- 单文件分发（一个 `.dpkg` 含整个 UI module）
- 跨平台一致格式（little-endian + 平台无关字段）
- 增量更新（patch 模式，下章 v2）

---

## 2. 现状

| 资源 | 现状 |
|---|---|
| 字体 | 启动时从磁盘加载 + FreeType + (msdfgen 或 Slug 曲线生成) |
| 图片 | 按需 decode 上传 |
| Shader | 启动编译 HLSL（DXC）；移动端有 SPIRV 预编译脚本 |
| React/Preact bundle | esbuild 输出 .js + 启动时 eval |
| 总启动延迟 | ~600 ms（典型场景）/ React 含 ≤ 1500 ms（[`perf_budget.md`](../perf_budget.md) § 5）|

---

## 3. 设计

### 3.1 文件格式

`.dpkg` 是一个**自描述二进制容器**：

```
+--------------------------------------------+
| Header (64 bytes)                          |
|   magic "DPKG"                              |
|   version u32                               |
|   index_offset u64                          |
|   index_count u32                           |
|   data_size u64                             |
|   features bitmask u32                      |
|   reserved[24]                              |
+--------------------------------------------+
| Payloads (mmap-friendly, 4KB aligned)      |
|   payload[0]                                |
|   payload[1]                                |
|   ...                                       |
+--------------------------------------------+
| Index Table (at index_offset)              |
|   IndexEntry {                              |
|       u32 type;     // FONT / IMAGE / ...   |
|       u32 flags;                            |
|       u64 offset;   // payload start        |
|       u64 length;                           |
|       u64 uncompressed_length;              |
|       char id[64];  // utf8, null-padded    |
|       u8 sha256[32];                        |
|   } x index_count                           |
+--------------------------------------------+
```

**关键属性**：
- 4 KB 对齐 → mmap 后 OS 直接 page-in，零拷贝。
- payload 可压缩（zstd）；header 标识。
- index 在末尾 → 写入时一次 append；读取时按 index 寻址。
- sha256 → 完整性校验 + 增量 patch 用。

### 3.2 资源类型

| Type | 用途 | 处理 |
|---|---|---|
| `FONT_FACE` | TTF/OTF 字体 | 直接 mmap 到 FreeType |
| `FONT_GLYPH_ATLAS` | 预渲染的 MSDF atlas（多档分辨率）| 上传 GPU，跳 msdfgen |
| `FONT_SLUG_CACHE` | 预生成的 Slug curve+band 纹理 | 上传 GPU，跳 outline_loader |
| `IMAGE_ATLAS` | 预打包图片 atlas（PNG / BC7 / ASTC） | mmap + 直接上传（GPU 压缩格式更省时间） |
| `IMAGE_SINGLE` | 单图（用于动态 src）| 仍 lazy decode |
| `SHADER_BINARY` | 平台特定字节码（SPIRV / DXIL / MSL） | 直接送 SDL_GPU；跳 DXC 编译 |
| `HTML` | HTML 文件 | mmap，dong load |
| `CSS` | CSS 文件 | 同上 |
| `JS_BUNDLE` | esbuild bundle.js | mmap + QuickJS eval；可选 QuickJS bytecode 缓存（第二次启动跳 parse） |
| `JS_BYTECODE` | QuickJS 预编译字节码（`qjsc` 风格） | 二次启动直接 run |
| `META_MANIFEST` | 入口路径 / 版本 / 资源映射 | json |

### 3.3 打包工具：`dpkg-pack`

```bash
# 一条命令打包当前 dong project 目录
dpkg-pack ./game-ui --output game-ui.dpkg \
   --target windows-x64 \
   --compression zstd \
   --pre-render-fonts "Inter,Noto Sans CJK SC" \
   --image-format bc7 \
   --shader-format dxil \
   --js-bytecode

# 选项：
#   --target            平台（影响 shader 格式 + 图片格式 + endianness）
#   --pre-render-fonts  把字体预生成 atlas（MSDF + Slug 双路径都做）
#   --image-format      bc7 / astc / png（含意：是否 GPU 压缩）
#   --js-bytecode       QuickJS 预编译
#   --strip             去除 debug 信息
#   --split LIMIT       超过 LIMIT MB 分多包
```

### 3.4 运行时加载

```c
// dong/include/dong_pack.h（新增）
typedef struct DongPack DongPack;

DongPack* dong_pack_open(const char* path);            // mmap + verify magic
DongPack* dong_pack_open_from_memory(const void* data, size_t size);
void      dong_pack_close(DongPack* pkg);

// 资源访问（零拷贝指针 + size）
typedef struct {
    const void* data;
    size_t size;
    int    is_compressed;
    uint32_t flags;
} DongPackPayload;

int dong_pack_get(DongPack* pkg, const char* id, DongPackPayload* out);

// 把 pack 注入到 engine（覆盖默认 fs / atlas / shader）
int dong_engine_attach_pack(dong_engine_t* eng, DongPack* pkg);
```

`dong_engine_attach_pack` 之后：
- HTML / CSS / JS load → 优先从 pack 取，找不到再走文件系统。
- 字体 fontFace.fromUrl → 查 pack 的 FONT_GLYPH_ATLAS 直接上传。
- Image atlas → 同上。
- Shader → SDL plugin 优先用 pack 中的预编译字节码。

### 3.5 加载性能保证

| 阶段 | 现状 | dpkg 后 |
|---|---|---|
| Open + mmap | N/A | < 5 ms |
| Index parse | N/A | < 1 ms |
| FreeType + Slug 字体 | ~150 ms | ~10 ms（直接 mmap 已生成的 atlas）|
| 图片 atlas decode + upload | ~80 ms | ~20 ms（GPU 压缩格式直传）|
| Shader compile | ~100 ms | ~5 ms（直接送字节码）|
| JS bundle parse + eval | ~150 ms | ~30 ms（QuickJS bytecode + eval） |
| **总启动到首帧** | ~600 ms | **~100 ms** |

### 3.6 打包工作流（业务方）

```bash
# 开发期：用 file system，不需要 dpkg
dong_app --html main.html

# Build 期：dpkg-pack 把 production assets 打包
dpkg-pack ./build --output app.dpkg --target windows-x64

# Distribution
dong_app --pkg app.dpkg
# 或
dong_app --pkg app.dpkg --html main.html  # main.html 路径相对 pack 内
```

dong-editor (P2-7) 内一键 build 也调 dpkg-pack。

### 3.7 增量 patch（v2 留 hook）

预留：
- 多 pack 叠加（`--pkg base.dpkg --pkg patch.dpkg`），后者覆盖前者同 id 资源。
- 业务方下载 patch.dpkg 即可热更资源（不更新 dong native）。

v1 仅做"覆盖"语义；v2 评估增量算法（bsdiff 等）。

### 3.8 平台差异处理

| 平台 | 差异 | 处理 |
|---|---|---|
| Windows | DXIL shader / BC7 atlas | dpkg-pack `--target windows-x64` |
| macOS | MSL shader / ASTC 可选 | `--target macos` |
| Linux | SPIRV shader / BC7 | `--target linux-x64` |
| iOS | MSL / ASTC | `--target ios` |
| Android | SPIRV / ASTC | `--target android-arm64` |

每平台一个 dpkg；不混跨平台（避免膨胀）。

> 业务侧 build 系统按 target 自动选 dpkg。

---

## 4. 实施步骤

| Step | 范围 |
|---|---|
| **S1 — `.dpkg` 文件格式 + reader** | mmap + index parse + payload 取 |
| **S2 — `dpkg-pack` 工具基础** | 打包 HTML/CSS/JS（无字体/图片/shader 预处理） |
| **S3 — 字体预生成（MSDF atlas + Slug curve/band）** | 工具内 freetype + msdfgen 生成 |
| **S4 — 图片 atlas 预打包（含 BC7/ASTC 压缩）** | 调用 P0-1 之前已有的 BC/ASTC encoder |
| **S5 — Shader 预编译（DXIL/SPIRV/MSL）** | 复用 `precompile_shaders.py` 逻辑 |
| **S6 — QuickJS bytecode 预编译（qjsc 风格）** | 第二次启动跳 parse |
| **S7 — `dong_engine_attach_pack` + fs / atlas / shader 路径接入** | 启动延迟达成 < 200 ms |
| **S8 — Multi-pack overlay** | patch 机制基础 |

---

## 5. 通过验证规则

### 5.1 Hard

| 项 | 阈值 |
|---|---|
| `dpkg-pack` 跑通典型 dong project（含字体 + 5 张 image + 2 shader + 1 bundle） | 必须 |
| `.dpkg` 文件 mmap 后 dong_app 加载并渲染首帧 ≤ 200 ms（[`perf_budget.md`](../perf_budget.md) § 5 Hard） | 必须 |
| 同一 HTML 内容用 .dpkg 加载 vs 文件系统加载像素一致 | < 1% diff |
| sha256 校验失败拒绝加载 + console.error | 必须 |
| 文件系统 fallback：pack 中找不到 id 时走 fs | 必须 |
| 跨 5 平台 target 都能产出可用 dpkg（CI matrix） | 必须 |
| `dong_pack_close` 不泄漏 mmap | ASAN |

### 5.2 Soft

| 项 | 阈值 |
|---|---|
| 启动到首帧 | ≤ 100 ms（Soft） |
| `.dpkg` 体积比 raw 文件总和 ≤ 80%（zstd 压缩） | 期望 |
| GPU 压缩 atlas vs PNG：上传时间下降 ≥ 60% | 期望 |
| QuickJS bytecode vs source eval：parse 时间下降 ≥ 80% | 期望 |
| 多 pack overlay 优先级正确 | 必须 |

### 5.3 必须新增的产物

| 产物 | 说明 |
|---|---|
| `dong/include/dong_pack.h` | C ABI |
| `dong/src/pack/` | reader 实现 |
| `dong/scripts/dpkg_pack.py`（或独立 binary）| 打包工具 |
| `dong/tests/pack/test_dpkg_format.cpp` | 格式单元 |
| `examples/data/packed_demo.dpkg` | 示例 pack |
| `dong/scripts/test_dpkg_startup_perf.py` | 启动 perf 自动测 |

### 5.4 验证命令

```bash
# 打包
python dong/scripts/dpkg_pack.py examples/data/gamelikeui --output gameui.dpkg --target windows-x64

# 运行
dong_app.exe --pkg gameui.dpkg

# 启动 perf
python dong/scripts/test_dpkg_startup_perf.py
# 期望：cold start < 200 ms

# 像素对比
DONG_BENCH_AUTOSTOP=1 dong_app.exe --pkg gameui.dpkg --output frame.bmp
DONG_BENCH_AUTOSTOP=1 dong_app.exe --html data/gamelikeui/game_ui2.html --output frame_fs.bmp
python dong/scripts/tools/compare_screenshots.py frame.bmp frame_fs.bmp
```

---

## 6. 风险与回退

| 风险 | 缓解 |
|---|---|
| mmap 在某些 Android 版本受限 | fallback 到内存读 |
| 字体预生成的 atlas 与字号不匹配 | 打包时业务侧指定字号集；运行时若需字号未预生成 fallback to runtime |
| 图片 BC7 / ASTC 在某些 GPU 不支持 | dpkg 内同时存 PNG fallback；运行时根据 GPU caps 选 |
| QuickJS bytecode 与 dong 内 QuickJS 版本绑定 | bytecode header 含版本；不匹配 fallback to source |
| Shader bytecode 与驱动版本兼容 | DXIL / SPIRV 都是稳定 ABI；MSL 可能要重编译；提供 `--regen-shader-on-mismatch` |
| 业务方 IDE 不能直接编辑 .dpkg | 文档：开发期不打包；只在分发时打 |
| 多 pack overlay 顺序复杂 | 文档明示后载覆盖前；有 verbose log |

---

## 7. 不在本方案范围

- ❌ 增量二进制 patch（v2，bsdiff / xdelta）
- ❌ 在线 patch 下载 / hot patch（v2）
- ❌ 加密 / DRM（业务侧自己处理）
- ❌ 动态生成 dpkg（仅 build-time）
- ❌ 跨平台单 dpkg（多 target 共一个文件；体积爆炸不划算）
- ❌ Streaming load（按需 chunk）—— 用 mmap 已足够好

---

## 8. 完成后更新

- [ ] `docs/reference/features-index.md` 新增 § "资源包 .dpkg"
- [ ] `docs/developer/perf-budget.md` § 5 启动延迟实测填入
- [ ] `dong/include/dong_pack.h` 文档化
- [ ] `dong/CLAUDE.md` `AGENTS.md` 加 `--pkg` 启动参数 + `dpkg-pack` 命令
- [ ] CI matrix 加 dpkg 打包验证 step
