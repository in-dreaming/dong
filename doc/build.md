


# examples

windows(llvm)
```
zig build examples -Dtarget=x86_64-windows-gnu  -Doptimize=ReleaseSmall
```

mac
```
zig build examples   -Doptimize=ReleaseSmall
```

zig build examples   -Doptimize=Debug

zig build examples -Doptimize=ReleaseSmall
zig build run-feature-tests


dong_app.exe

html_render_test.exe data/tests/cursor_test.html zig-out/tmp/features/cursor_test.bmp 1024 1024 3
.\dong_app.exe --html .\data\tests\cursor_test.html


# 方式 2: 直接运行
zig-out/bin/run_feature_tests.exe

# 查看报告
start zig-out/tmp/features/report.html