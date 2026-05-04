# Run profiler for optimized dong engine build
# Execute from dong directory: .\run_profile.ps1

Set-Location $PSScriptRoot

Write-Host "=== Dong Engine Performance Test ===" -ForegroundColor Cyan
Write-Host ""

# Check if build exists
if (-not (Test-Path "build-cmake\3d_screen_script.exe")) {
    Write-Host "ERROR: build-cmake\3d_screen_script.exe not found" -ForegroundColor Red
    Write-Host "Please build the project first with: cmake --build build-cmake --config Release"
    exit 1
}

# Create traces directory if needed
if (-not (Test-Path "tmp\traces")) {
    New-Item -ItemType Directory -Path "tmp\traces" -Force | Out-Null
}

Write-Host "Running profiler with Python..." -ForegroundColor Yellow
python scripts/tools/auto_profile_loop.py `
    --build-dir ./build-cmake `
    --target 3d_screen_script `
    --out-dir ./tmp/traces `
    --warmup-ms 2000 `
    --run-ms 5000 `
    --no-build `
    --iters 1

Write-Host ""
Write-Host "=== Test Complete ===" -ForegroundColor Green
Write-Host "Check tmp/traces for results"

# Show latest trace files
Write-Host ""
Write-Host "Latest trace files:" -ForegroundColor Yellow
Get-ChildItem "tmp\traces\*.json" -ErrorAction SilentlyContinue | 
    Sort-Object LastWriteTime -Descending | 
    Select-Object -First 3 | 
    ForEach-Object { Write-Host "  $_" }

Write-Host ""
Write-Host "To analyze results, run:" -ForegroundColor Cyan
Write-Host "  python scripts/tools/trace_sum.py tmp/traces/<trace_file>.json --top 40"
