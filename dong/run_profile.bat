@echo off
REM Run profiler for optimized dong engine build
REM Execute from dong directory: run_profile.bat

cd /d "%~dp0"

echo === Dong Engine Performance Test ===
echo.

REM Check if build exists
if not exist "build-cmake\3d_screen_script.exe" (
    echo ERROR: build-cmake\3d_screen_script.exe not found
    echo Please build the project first with: cmake --build build-cmake --config Release
    exit /b 1
)

REM Create traces directory if needed
if not exist "tmp\traces" mkdir tmp\traces

echo Running profiler with Python...
python scripts/tools/auto_profile_loop.py ^
    --build-dir ./build-cmake ^
    --target 3d_screen_script ^
    --out-dir ./tmp/traces ^
    --warmup-ms 2000 ^
    --run-ms 5000 ^
    --no-build ^
    --iters 1

echo.
echo === Test Complete ===
echo Check tmp/traces for results

REM Show latest trace file
echo.
echo Latest trace files:
dir /b /o-d tmp\traces\*.json 2>nul | head -3

pause
