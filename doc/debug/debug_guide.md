# 调试指南 - Zig Build + C/C++ 项目

## 1. 构建 Debug 版本

### 基本命令

```bash
cd dong

# Debug 模式构建所有示例（默认 optimize=Debug）
zig build examples -Doptimize=Debug

# Release 模式（用于性能测试）
zig build examples -Doptimize=ReleaseSmall
```

### 构建特定示例

```bash
# 构建 interactive_demo（包含在 examples 中）
zig build examples -Doptimize=Debug

# 运行
./zig-out/bin/interactive_demo
```

**注意：** `interactive_demo` 是通过 `zig build examples` 构建的，不是单独的构建目标。

## 2. 使用 LLDB/GDB 调试

### macOS (LLDB)

```bash
# 构建 Debug 版本
zig build examples -Doptimize=Debug

# 使用 LLDB 启动
lldb ./zig-out/bin/interactive_demo

# 在 LLDB 中：
(lldb) breakpoint set --name main
(lldb) breakpoint set --file view.cpp --line 294
(lldb) run
(lldb) continue
(lldb) print commands_dirty_
(lldb) frame variable
```

### Linux (GDB)

```bash
# 构建 Debug 版本
zig build examples -Doptimize=Debug

# 使用 GDB 启动
gdb ./zig-out/bin/interactive_demo

# 在 GDB 中：
(gdb) break main
(gdb) break view.cpp:294
(gdb) run
(gdb) continue
(gdb) print commands_dirty_
(gdb) info locals
```

### VS Code 调试配置

创建 `.vscode/launch.json`:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "type": "lldb",
            "request": "launch",
            "name": "Debug interactive_demo",
            "program": "${workspaceFolder}/dong/zig-out/bin/interactive_demo",
            "args": [],
            "cwd": "${workspaceFolder}/dong",
            "preLaunchTask": "build-debug"
        }
    ]
}
```

创建 `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build-debug",
            "type": "shell",
            "command": "zig build examples -Doptimize=Debug",
            "options": {
                "cwd": "${workspaceFolder}/dong"
            },
            "problemMatcher": []
        }
    ]
}
```

## 3. 启用调试日志

### 环境变量

```bash
# 启用图层缓存调试日志
export DONG_DEBUG_LAYER_CACHE=1

# 启用绘制批次调试日志
export DONG_DEBUG_DRAW_BATCHES=1

# 启用图层树调试日志
export DONG_DEBUG_LAYER_TREE=1

# 启用图层缓存功能（默认关闭）
export DONG_LAYER_CACHE=1

# 运行程序
./zig-out/bin/interactive_demo
```

### 一次性设置

```bash
# 所有调试选项
DONG_DEBUG_LAYER_CACHE=1 \
DONG_DEBUG_DRAW_BATCHES=1 \
DONG_DEBUG_LAYER_TREE=1 \
DONG_LAYER_CACHE=1 \
./zig-out/bin/interactive_demo
```

### 查看日志输出

日志会输出到：
- **stdout/stderr**: 通过 `SDL_Log` 输出
- **系统日志**: macOS 上可通过 Console.app 查看

重定向到文件：
```bash
DONG_DEBUG_LAYER_CACHE=1 ./zig-out/bin/interactive_demo 2>&1 | tee debug.log
```

## 4. 关键调试点

### 渲染管线调试

在以下位置添加断点或日志：

1. **View::update()** (`dong/src/core/view.cpp:201`)
   - 检查 `commands_dirty_` 状态
   - 检查是否重建 DisplayList

2. **GPUDriverSDL::beginFrame()** (`dong/src/render/gpu_driver_sdl.cpp:1129`)
   - 检查 `in_use` 标志重置
   - 检查 `frame_index_` 递增

3. **GPUDriverSDL::execute()** (`dong/src/render/gpu_driver_sdl.cpp:1340`)
   - 检查 swapchain 纹理获取
   - 检查图层缓存查找

4. **BeginIsolatedLayer** (`dong/src/render/gpu_driver_sdl.cpp:1686`)
   - 检查 `layer_dirty` 标志
   - 检查缓存查找逻辑
   - 检查 `valid_for_cache` 状态

5. **EndIsolatedLayer** (`dong/src/render/gpu_driver_sdl.cpp:1891`)
   - 检查图层合成
   - 检查 `valid_for_cache` 设置

### 添加临时调试日志

在关键位置添加：

```cpp
// 在 view.cpp 的 update() 中
SDL_Log("[DEBUG] View::update frame: commands_dirty_=%d, cached_cmd_list_ size=%zu",
        commands_dirty_ ? 1 : 0,
        cached_cmd_list_ ? cached_cmd_list_->commands.size() : 0);

// 在 gpu_driver_sdl.cpp 的 BeginIsolatedLayer 中
SDL_Log("[DEBUG] BeginIsolatedLayer: layer_id=%llu, layer_dirty=%d, cache_entry=%p, valid=%d",
        static_cast<unsigned long long>(layer_id),
        layer_dirty ? 1 : 0,
        (void*)cache_entry,
        cache_entry ? (cache_entry->valid_for_cache ? 1 : 0) : -1);
```

## 5. 常见问题调试

### 问题：后续帧内容缺失

**检查清单：**

1. **检查 `commands_dirty_` 状态**
   ```cpp
   // 在 view.cpp:267 添加
   SDL_Log("[DEBUG] need_rebuild=%d, commands_dirty_=%d", need_rebuild, commands_dirty_);
   ```

2. **检查图层缓存状态**
   ```bash
   DONG_DEBUG_LAYER_CACHE=1 DONG_LAYER_CACHE=1 ./zig-out/bin/interactive_demo
   ```
   查看日志中是否有 "reuse layer" 或 "rasterize" 消息

3. **检查 `in_use` 标志重置**
   ```cpp
   // 在 gpu_driver_sdl.cpp:1147 添加
   SDL_Log("[DEBUG] beginFrame: resetting %zu layer render targets", layer_render_targets_.size());
   ```

4. **检查 swapchain 纹理获取**
   ```cpp
   // 在 gpu_driver_sdl.cpp:1365 添加
   SDL_Log("[DEBUG] Acquired swapchain texture: %p, size=%ux%u", 
           (void*)swapchain_texture, w, h);
   ```

### 问题：图层缓存不工作

**检查：**
1. 确认 `DONG_LAYER_CACHE=1` 已设置
2. 检查 `layer_cache_enabled_` 是否为 true
3. 检查 `valid_for_cache` 是否在 `EndIsolatedLayer` 中被设置为 true
4. 检查 `layer_id` 是否匹配

## 6. 性能分析

### 使用 Instruments (macOS)

```bash
# 构建 Release 版本
zig build examples -Doptimize=ReleaseSmall

# 使用 Instruments 分析
instruments -t "Time Profiler" ./zig-out/bin/interactive_demo
```

### 使用 perf (Linux)

```bash
perf record ./zig-out/bin/interactive_demo
perf report
```

## 7. 内存调试

### 使用 AddressSanitizer

修改 `build.zig`，在构建选项中添加：
```zig
exe.addCSourceFiles(.{ .files = ..., .flags = &.{"-fsanitize=address"} });
exe.linkLibC();
```

然后运行：
```bash
zig build examples -Doptimize=Debug
./zig-out/bin/interactive_demo
```

### 使用 Valgrind (Linux)

```bash
valgrind --leak-check=full ./zig-out/bin/interactive_demo
```

## 8. 快速调试脚本

创建 `debug.sh`:

```bash
#!/bin/bash
set -e

cd "$(dirname "$0")/dong"

echo "Building Debug version..."
zig build examples -Doptimize=Debug

echo "Running with debug logs..."
DONG_DEBUG_LAYER_CACHE=1 \
DONG_DEBUG_DRAW_BATCHES=1 \
DONG_DEBUG_LAYER_TREE=1 \
DONG_LAYER_CACHE=1 \
./zig-out/bin/interactive_demo 2>&1 | tee ../debug.log

echo "Log saved to debug.log"
```

使用方法：
```bash
chmod +x debug.sh
./debug.sh
```

## 9. 调试技巧

1. **使用条件断点**
   ```lldb
   (lldb) breakpoint set --file view.cpp --line 267 --condition 'commands_dirty_ == false'
   ```

2. **监控变量变化**
   ```lldb
   (lldb) watchpoint set variable commands_dirty_
   ```

3. **打印调用栈**
   ```lldb
   (lldb) bt
   (lldb) frame select 1
   (lldb) frame variable
   ```

4. **执行表达式**
   ```lldb
   (lldb) expr cached_cmd_list_->commands.size()
   ```

## 10. 日志分析

### 关键日志模式

查找以下模式来诊断问题：

```bash
# 查找图层缓存相关
grep -E "layer-cache|BeginIsolatedLayer|EndIsolatedLayer" debug.log

# 查找命令列表重建
grep -E "Building DisplayList|Compiling DisplayList" debug.log

# 查找帧信息
grep -E "frame=|beginFrame|endFrame" debug.log

# 查找错误
grep -E "ERROR|failed|null" debug.log
```
