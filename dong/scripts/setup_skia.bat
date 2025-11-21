@echo off
REM Skia setup script for Windows

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR:~0,-1%
cd /d "%PROJECT_ROOT%\.."

echo Dong Engine - Skia Setup
echo =========================
echo Platform: Windows
echo.

REM Run Python setup
python "%SCRIPT_DIR%setup_skia.py" %*

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Skia setup successful!
    echo.
    echo You can now build with:
    echo   cd "%PROJECT_ROOT%\.."
    echo   zig build
) else (
    echo.
    echo Skia setup failed
    exit /b 1
)
