@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

:: 字体渲染对比测试脚本 (Windows)
:: 用法: run_font_compare.bat [test_name]
:: 示例: run_font_compare.bat chinese_font_sizes_test

set "SCRIPT_DIR=%~dp0"
set "DONG_ROOT=%SCRIPT_DIR%\..\.."
cd /d "%DONG_ROOT%"

echo ============================================
echo 字体渲染自动化对比测试
echo ============================================

:: 检查参数
if "%~1"=="" (
    echo 用法: %~nx0 [test_name]
    echo 示例: %~nx0 chinese_font_sizes_test
    echo.
    echo 可用的测试文件:
    dir /b "examples\data\tests\chinese_font*" 2>nul
    exit /b 1
)

set "TEST_NAME=%~1"
set "HTML_FILE=examples\data\tests\%TEST_NAME%.html"

if not exist "%HTML_FILE%" (
    echo 错误: 测试文件不存在: %HTML_FILE%
    exit /b 1
)

:: 设置输出目录
set "OUTPUT_DIR=tmp\font_compare\%TEST_NAME%_%date:~0,4%%date:~5,2%%date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%"
set "OUTPUT_DIR=%OUTPUT_DIR: =0%"
mkdir "%OUTPUT_DIR%" 2>nul

echo.
echo 测试文件: %HTML_FILE%
echo 输出目录: %OUTPUT_DIR%
echo.

:: 步骤1: 使用 dong 渲染
echo [1/3] Dong 引擎离屏渲染...
set "DONG_OUTPUT=%OUTPUT_DIR%\dong_render.bmp"

:: 查找 html_render_test.exe
if exist "zig-out\bin\html_render_test.exe" (
    set "RENDER_EXE=zig-out\bin\html_render_test.exe"
) else if exist "build-cmake\html_render_test.exe" (
    set "RENDER_EXE=build-cmake\html_render_test.exe"
) else (
    echo 错误: html_render_test.exe 未找到
    echo 请先构建项目: zig build examples
    exit /b 1
)

echo   使用: %RENDER_EXE%
"%RENDER_EXE%" "%HTML_FILE%" "%DONG_OUTPUT%" 800 600

if not exist "%DONG_OUTPUT%" (
    echo 错误: Dong 渲染失败
    exit /b 1
)
echo   ✓ Dong 渲染完成: %DONG_OUTPUT%

:: 步骤2: 使用 Python + Playwright 渲染浏览器基准
echo.
echo [2/3] 浏览器基准渲染...
set "BASELINE_OUTPUT=%OUTPUT_DIR%\browser_baseline.png"

python -c "
import sys
sys.path.insert(0, 'scripts/tools')
from auto_font_render_compare import render_with_playwright
from pathlib import Path
result = render_with_playwright(Path('%HTML_FILE%'), Path('%BASELINE_OUTPUT%'), 800, 600)
sys.exit(0 if result else 1)
"

if not exist "%BASELINE_OUTPUT%" (
    echo   ⚠ 浏览器渲染失败或未安装 Playwright
    echo   安装: pip install playwright
    echo   然后: playwright install chromium
) else (
    echo   ✓ 浏览器基准完成: %BASELINE_OUTPUT%
)

:: 步骤3: 对比图片
echo.
echo [3/3] 图片对比...
set "COMPARE_OUTPUT=%OUTPUT_DIR%\comparison.png"

python -c "
import sys
sys.path.insert(0, 'scripts/tools')
from auto_font_render_compare import compare_images
from pathlib import Path
success, mean, max_val, percent = compare_images(Path('%DONG_OUTPUT%'), Path('%BASELINE_OUTPUT%'), Path('%COMPARE_OUTPUT%'))
sys.exit(0 if success else 1)
"

if exist "%COMPARE_OUTPUT%" (
    echo   ✓ 对比完成: %COMPARE_OUTPUT%
)

:: 总结
echo.
echo ============================================
echo 测试完成!
echo ============================================
echo 输出目录: %OUTPUT_DIR%
echo.
echo 文件列表:
dir /b "%OUTPUT_DIR%"
echo.

:: 尝试打开对比图
if exist "%COMPARE_OUTPUT%" (
    echo 正在打开对比图...
    start "" "%COMPARE_OUTPUT%"
)

exit /b 0
