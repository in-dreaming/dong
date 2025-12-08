# Zig Build + C/C++ 工程调试配置指南

本文档介绍如何在 VSCode 中配置 Zig 构建系统的 C/C++ 项目调试环境。

## 前置条件

1. **Zig 编译器** - 已安装并配置到 PATH
2. **VSCode** - 或基于 VSCode 的 IDE
3. **CodeLLDB 扩展** - macOS/Linux 推荐使用 `vadimcn.vscode-lldb`

## 项目结构

```
project/
├── build.zig           # Zig 构建脚本
├── src/                # 源代码
├── examples/           # 示例程序
├── zig-out/
│   └── bin/            # 编译输出的可执行文件
└── .vscode/
    ├── launch.json     # 调试配置
    └── tasks.json      # 构建任务
```

## 配置步骤

### 1. 安装 CodeLLDB 扩展

在 VSCode 扩展市场搜索 `CodeLLDB` 并安装，或在 `.vscode/extensions.json` 中添加：

```json
{
    "recommendations": [
        "vadimcn.vscode-lldb"
    ]
}
```

### 2. 配置构建任务 (tasks.json)

创建 `.vscode/tasks.json`：

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build-debug",
            "type": "shell",
            "command": "zig build -Doptimize=Debug",
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "problemMatcher": [
                {
                    "owner": "zig",
                    "fileLocation": ["relative", "${workspaceFolder}"],
                    "pattern": {
                        "regexp": "^(.+):(\\d+):(\\d+):\\s+(error|warning|note):\\s+(.*)$",
                        "file": 1,
                        "line": 2,
                        "column": 3,
                        "severity": 4,
                        "message": 5
                    }
                },
                "$gcc"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "build-release",
            "type": "shell",
            "command": "zig build -Doptimize=ReleaseFast",
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "problemMatcher": ["$gcc"],
            "group": "build"
        },
        {
            "label": "clean",
            "type": "shell",
            "command": "rm -rf zig-out zig-cache .zig-cache",
            "problemMatcher": []
        }
    ]
}
```

**关键点：**
- `command`: 使用 `zig build` 命令
- `-Doptimize=Debug`: 生成带调试符号的二进制
- `problemMatcher`: 解析 Zig/GCC 风格的编译错误

### 3. 配置调试启动 (launch.json)

创建 `.vscode/launch.json`：

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "type": "lldb",
            "request": "launch",
            "name": "Debug Program",
            "program": "${workspaceFolder}/zig-out/bin/your_program",
            "args": [],
            "cwd": "${workspaceFolder}",
            "preLaunchTask": "build-debug",
            "stopOnEntry": false,
            "console": "integratedTerminal",
            "sourceLanguages": ["cpp", "c"]
        },
        {
            "type": "lldb",
            "request": "launch",
            "name": "Debug Current File",
            "program": "${workspaceFolder}/zig-out/bin/${fileBasenameNoExtension}",
            "args": [],
            "cwd": "${workspaceFolder}",
            "preLaunchTask": "build-debug",
            "console": "integratedTerminal",
            "sourceLanguages": ["cpp", "c"]
        },
        {
            "type": "lldb",
            "request": "attach",
            "name": "Attach to Process",
            "pid": "${command:pickProcess}"
        }
    ]
}
```

**配置说明：**

| 字段 | 说明 |
|------|------|
| `type` | 使用 `lldb` (macOS/Linux) 或 `cppdbg` (Windows) |
| `program` | 可执行文件路径，Zig 默认输出到 `zig-out/bin/` |
| `preLaunchTask` | 调试前自动执行的构建任务 |
| `cwd` | 程序运行时的工作目录 |
| `console` | 终端类型，推荐 `integratedTerminal` |
| `sourceLanguages` | 源码语言，帮助调试器正确解析 |

### 4. build.zig 中启用调试符号

确保 `build.zig` 支持 Debug 模式：

```zig
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});  // 支持 -Doptimize=Debug

    const exe = b.addExecutable(.{
        .name = "my_program",
        .target = target,
        .optimize = optimize,  // 传递优化级别
    });

    // 添加 C/C++ 源文件
    exe.addCSourceFile(.{
        .file = b.path("src/main.c"),
        .flags = &.{},  // Debug 模式下 Zig 自动添加 -g
    });

    exe.linkLibC();
    b.installArtifact(exe);
}
```

**注意：** 使用 `-Doptimize=Debug` 时，Zig 会自动为 C/C++ 代码添加 `-g` 调试符号。

## 使用方法

### 开始调试

1. 在源码中设置断点（点击行号左侧）
2. 按 `F5` 或点击调试面板的绿色播放按钮
3. 选择调试配置

### 常用快捷键

| 操作 | macOS | Windows/Linux |
|------|-------|---------------|
| 开始/继续调试 | `F5` | `F5` |
| 单步跳过 | `F10` | `F10` |
| 单步进入 | `F11` | `F11` |
| 单步跳出 | `Shift+F11` | `Shift+F11` |
| 停止调试 | `Shift+F5` | `Shift+F5` |
| 重启调试 | `Cmd+Shift+F5` | `Ctrl+Shift+F5` |
| 切换断点 | `F9` | `F9` |

### 调试面板功能

- **变量 (Variables)**: 查看局部变量、全局变量
- **监视 (Watch)**: 添加表达式监视
- **调用堆栈 (Call Stack)**: 查看函数调用链
- **断点 (Breakpoints)**: 管理所有断点

## 高级配置

### 环境变量

```json
{
    "type": "lldb",
    "request": "launch",
    "name": "Debug with Env",
    "program": "${workspaceFolder}/zig-out/bin/my_program",
    "env": {
        "DEBUG_MODE": "1",
        "LOG_LEVEL": "verbose"
    }
}
```

### 命令行参数

```json
{
    "type": "lldb",
    "request": "launch",
    "name": "Debug with Args",
    "program": "${workspaceFolder}/zig-out/bin/my_program",
    "args": ["--config", "debug.json", "-v"]
}
```

### 源码映射

当源码路径与编译路径不一致时：

```json
{
    "type": "lldb",
    "request": "launch",
    "name": "Debug with Source Map",
    "program": "${workspaceFolder}/zig-out/bin/my_program",
    "sourceMap": {
        "/build/path": "${workspaceFolder}"
    }
}
```

### LLDB 初始化命令

```json
{
    "type": "lldb",
    "request": "launch",
    "name": "Debug with LLDB Commands",
    "program": "${workspaceFolder}/zig-out/bin/my_program",
    "initCommands": [
        "settings set target.process.stop-on-sharedlibrary-events false",
        "breakpoint set -n main"
    ]
}
```

## 常见问题

### Q: 断点不生效？

1. 确保使用 `-Doptimize=Debug` 构建
2. 检查源码路径是否正确
3. 尝试清理后重新构建：`zig build clean && zig build -Doptimize=Debug`

### Q: 找不到可执行文件？

检查 `program` 路径是否正确，Zig 默认输出到 `zig-out/bin/`。

### Q: 变量显示 `<optimized out>`？

说明使用了 Release 模式编译，改用 Debug 模式：

```bash
zig build -Doptimize=Debug
```

### Q: macOS 上调试器无法启动？

可能需要授权开发者工具：

```bash
sudo DevToolsSecurity -enable
```

## 参考资源

- [Zig Build System](https://ziglang.org/documentation/master/#Build-System)
- [CodeLLDB 文档](https://github.com/vadimcn/codelldb/blob/master/MANUAL.md)
- [VSCode 调试文档](https://code.visualstudio.com/docs/editor/debugging)
