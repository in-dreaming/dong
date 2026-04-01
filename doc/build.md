


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

二进制大小优化
-Doptimize-size=true

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

# cross package

`package/package-zip` 支持跟随 `-p/--prefix`，可直接对交叉编译前缀做 SDK 打包（libs-only 模式只打包 `include/` + `lib/`）。

示例（Linux x64 glibc）：

```
zig build -Dtarget=x86_64-linux-gnu -Dlibs-only=true -p zig-out-linux-x64 package
zig build -Dtarget=x86_64-linux-gnu -Dlibs-only=true -p zig-out-linux-x64 package-zip
```

输出：
- `zig-out-linux-x64/dist/dong_sdk_x86_64-linux-gnu/`
- `zig-out-linux-x64/dist/dong_sdk_x86_64-linux-gnu.zip`

# package

// to package
zig build package
// TO zip
zig build package-zip
