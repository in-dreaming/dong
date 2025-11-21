# Skia Prebuilt Binary Downloader for Windows
# This script downloads pre-compiled Skia binaries for Windows

param(
    [string]$SkiaVersion = "m124",  # Skia version, adjust as needed
    [string]$OutputDir = "third_party/skia"
)

# Platform detection
$Platform = "win64"  # Windows 64-bit

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
    Write-Host "Created directory: $OutputDir"
}

# Recommended Skia prebuilt sources for Windows
# Option 1: Aseprite's Skia builds (recommended, optimized for games)
$AsepriteSources = @(
    "https://github.com/aseprite/skia/releases/download/m124-16f7b74d39/skia-win64.zip",
    "https://github.com/aseprite/skia/releases/download/m123-65e3d5a7a7/skia-win64.zip",
)

Write-Host "Skia Prebuilt Download Script"
Write-Host "=============================="
Write-Host "Platform: $Platform"
Write-Host "Output: $OutputDir"
Write-Host ""
Write-Host "To download Skia prebuilt binaries:"
Write-Host ""
Write-Host "1. Visit: https://github.com/aseprite/skia/releases"
Write-Host "2. Download the appropriate Windows binary (skia-win64.zip)"
Write-Host "3. Extract to: $OutputDir"
Write-Host ""
Write-Host "Expected structure after extraction:"
Write-Host "  $OutputDir/"
Write-Host "    ├── include/     (Skia headers)"
Write-Host "    └── lib/         (skia.lib or libskia.a)"
Write-Host ""
Write-Host "Or use this one-liner to download from Aseprite:"
Write-Host "powershell -Command `"(New-Object System.Net.WebClient).DownloadFile('https://github.com/aseprite/skia/releases/download/m124-16f7b74d39/skia-win64.zip', 'skia-win64.zip'); Expand-Archive -Path 'skia-win64.zip' -DestinationPath '$OutputDir'; Remove-Item 'skia-win64.zip'`""
Write-Host ""
