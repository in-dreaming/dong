


# examples

windows(llvm)
```
zig build examples -Dtarget=x86_64-windows-gnu  -Doptimize=ReleaseSmall
```

mac
```
zig build examples   -Doptimize=ReleaseSmall
```

zig build examples
- -Doptimize=Debug
- -Doptimize=ReleaseSmall

zig build
zig build run-feature-tests


dong_app.exe

html_render_test.exe data/tests/cursor_test.html zig-out/tmp/features/cursor_test.bmp 1024 1024 3
.\dong_app.exe --html .\data\tests\cursor_test.html


# 方式 2: 直接运行
zig-out/bin/run_feature_tests.exe

# 查看报告
start zig-out/tmp/features/report.html

# profiler

 .\3d_screen_script.exe --profile trace.json

# cross
交叉编译功能完成总结

  交叉编译功能已成功调通。以下是支持的目标平台：

  ✅ 已验证可工作的目标
  目标: Linux x64 (glibc)
  命令: zig build -Dtarget=x86_64-linux-gnu -Dlibs-only=true -p
    zig-out-linux-x64 deps
  输出目录: zig-out-linux-x64/lib/
  ────────────────────────────────────────
  目标: Linux ARM64 (glibc)
  命令: zig build -Dtarget=aarch64-linux-gnu -Dlibs-only=true -p
    zig-out-linux-arm64 deps
  输出目录: zig-out-linux-arm64/lib/
  ────────────────────────────────────────
  目标: Linux x64 (musl)
  命令: zig build -Dtarget=x86_64-linux-musl -Dlibs-only=true -p
    zig-out-linux-musl-x64 deps
  输出目录: zig-out-linux-musl-x64/lib/
  ────────────────────────────────────────
  目标: Linux ARM64 (musl)
  命令: zig build -Dtarget=aarch64-linux-musl -Dlibs-only=true -p
    zig-out-linux-musl-arm64 deps
  输出目录: zig-out-linux-musl-arm64/lib/
  生成的库文件

  python scripts/cross_compile.py all    # 构建所有Linux目标
  python scripts/cross_compile.py list   # 列出可用目标
  
  - macOS/iOS: 需要macOS SDK，只能在macOS主机上交叉编译
  - Android: 需要Android NDK sysroot（Zig不内置Bionic libc）
  - musl目标: 生成完全静态链接的库，适合容器/嵌入式环境
  - 
# 编译
构建步骤
  步骤: zig build
  说明: 完整构建 (依赖 + 主程序 + 示例)
  构建方法: 混合
  ────────────────────────────────────────
  步骤: zig build deps
  说明: 所有第三方依赖
  构建方法: 纯Zig + SDL3 CMake
  ────────────────────────────────────────
  步骤: zig build quickjs
  说明: QuickJS
  构建方法: 纯Zig
  ────────────────────────────────────────
  步骤: zig build lexbor
  说明: Lexbor HTML解析器
  构建方法: 纯Zig
  ────────────────────────────────────────
  步骤: zig build yoga
  说明: Yoga布局引擎
  构建方法: 纯Zig
  ────────────────────────────────────────
  步骤: zig build freetype
  说明: FreeType字体库
  构建方法: 纯Zig
  ────────────────────────────────────────
  步骤: zig build harfbuzz
  说明: HarfBuzz文本整形
  构建方法: 纯Zig
  ────────────────────────────────────────
  步骤: zig build msdfgen
  说明: msdfgen字形生成
  构建方法: 纯Zig
  ────────────────────────────────────────
  步骤: zig build dong-core
  说明: Dong核心库
  构建方法: 纯Zig
  ────────────────────────────────────────
  步骤: zig build sdl-backend
  说明: SDL后端
  构建方法: 纯Zig
  ────────────────────────────────────────
  步骤: zig build sdl3
  说明: SDL3
  构建方法: CMake
  交叉编译支持