#!/bin/bash

# Test CSS Global Keywords Implementation
# This script renders test HTML files and creates a summary

cd "$(dirname "$0")"

OUTDIR="/tmp/css_global_keywords_test"
mkdir -p "$OUTDIR"

echo "=========================================="
echo "CSS Global Keywords Test Suite"
echo "=========================================="
echo ""

# Test files
TESTS=(
    "css_global_keywords_inherit"
    "css_global_keywords_initial"
    "css_global_keywords_unset"
    "css_global_keywords_comprehensive"
)

SIZES=(
    "800:800"
    "800:800"
    "800:900"
    "900:1400"
)

echo "Rendering test cases..."
echo ""

for i in "${!TESTS[@]}"; do
    TEST="${TESTS[$i]}"
    SIZE="${SIZES[$i]}"
    WIDTH="${SIZE%:*}"
    HEIGHT="${SIZE#*:}"

    HTML="examples/data/tests/${TEST}.html"
    BMP="$OUTDIR/${TEST}.bmp"

    if [ ! -f "$HTML" ]; then
        echo "❌ Test file not found: $HTML"
        continue
    fi

    echo "📝 Test: $TEST (${WIDTH}x${HEIGHT})"

    # Render with dong engine
    if ./zig-out/bin/html_render_test "$HTML" "$BMP" "$WIDTH" "$HEIGHT" > "$OUTDIR/${TEST}.log" 2>&1; then
        SIZE=$(ls -lh "$BMP" | awk '{print $5}')
        echo "   ✅ Rendered: $BMP ($SIZE)"
    else
        echo "   ❌ Failed to render"
        echo "   Log: $OUTDIR/${TEST}.log"
    fi
    echo ""
done

echo "=========================================="
echo "Summary"
echo "=========================================="
echo ""
echo "Output directory: $OUTDIR"
echo ""
echo "Generated files:"
ls -lh "$OUTDIR"/*.bmp 2>/dev/null | awk '{print "  " $9 " - " $5}'
echo ""
echo "To view results, open the BMP files in an image viewer"
echo "Example: open $OUTDIR/*.bmp"
echo ""
echo "Test completed!"
