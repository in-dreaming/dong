#!/bin/bash
# Skia setup script for macOS and Linux

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Dong Engine - Skia Setup"
echo "======================="
echo "Platform: $(uname -s)"
echo ""

# Run Python setup
python3 "$SCRIPT_DIR/setup_skia.py" "$@"

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Skia setup successful!"
    echo ""
    echo "You can now build with:"
    echo "  cd $PROJECT_ROOT"
    echo "  zig build"
else
    echo ""
    echo "✗ Skia setup failed"
    exit 1
fi
