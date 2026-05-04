# VS Code 调试使用说明

## 重要提示

**Code Runner 的错误不影响调试！** Code Runner 是用于快速运行代码的扩展，但调试使用的是 CodeLLDB 扩展。

## 正确的调试方法

### 方法 1：使用调试面板（推荐）

1. **确保已安装 CodeLLDB 扩展**
   - 按 `Cmd+Shift+X` 打开扩展面板
   - 搜索并安装 "CodeLLDB"

2. **设置断点**
   - 打开 `dong/src/core/view.cpp`
   - 在第 267 行点击左侧设置断点（红点）

3. **启动调试**
   - 按 `F5` 或点击左侧调试面板
   - 选择 "Debug interactive_demo"
   - 点击绿色播放按钮

4. **调试会自动：**
   - 运行 `build-debug` 任务（从 `dong/` 目录构建）
   - 启动 CodeLLDB 调试器
   - 在断点处停止

### 方法 2：手动构建后调试

1. **构建项目**
   ```bash
   cd /Users/lcle/Documents/workspace/self/engine/dong/dong
   zig build examples -Doptimize=Debug
   ```

2. **启动调试**
   - 按 `F5`
   - 选择 "Debug interactive_demo"

## Code Runner 问题说明

Code Runner 尝试从 `dong/examples/` 目录运行命令，但 `build.zig` 需要从 `dong/` 目录运行。

**解决方案：**
- **不要使用 Code Runner 运行 C++ 文件**
- 使用 VS Code 的调试功能（F5）
- 或手动在终端运行：
  ```bash
  cd /Users/lcle/Documents/workspace/self/engine/dong/dong
  zig build examples -Doptimize=Debug
  ./zig-out/bin/interactive_demo
  ```

## 工作目录说明

- **构建命令必须从 `dong/` 目录运行**
- **调试配置已正确设置工作目录为 `dong/`**
- **Code Runner 配置已更新，但建议不使用它**

## 验证调试配置

1. 检查可执行文件是否存在：
   ```bash
   ls -la /Users/lcle/Documents/workspace/self/engine/dong/dong/zig-out/bin/interactive_demo
   ```

2. 检查构建是否成功：
   ```bash
   cd /Users/lcle/Documents/workspace/self/engine/dong/dong
   zig build examples -Doptimize=Debug
   ```

3. 如果构建成功，调试应该可以工作。

## 常见问题

### Q: Code Runner 报错怎么办？
A: 忽略它。使用 F5 调试，不要使用 Code Runner。

### Q: 调试启动失败？
A: 检查：
1. CodeLLDB 扩展是否安装
2. 可执行文件是否存在（先构建）
3. 查看调试控制台的错误信息

### Q: 断点不生效？
A: 确保：
1. 构建的是 Debug 版本（`-Doptimize=Debug`）
2. 源代码路径正确
3. 断点设置在可执行代码行（不是注释或空行）

## 快捷键

- `F5` - 开始/继续调试
- `F9` - 切换断点
- `F10` - 单步跳过
- `F11` - 单步进入
- `Shift+F11` - 单步跳出
- `Shift+F5` - 停止调试
- `Cmd+Shift+B` - 构建
