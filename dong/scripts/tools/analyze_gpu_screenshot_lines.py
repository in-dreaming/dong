import struct

BMP_HEADER_SIZE = 14
DIB_HEADER_SIZE = 40


def analyze_glyph_layout(filename: str = "gpu_screenshot.bmp") -> None:
    with open(filename, "rb") as f:
        header = f.read(BMP_HEADER_SIZE)
        if header[0:2] != b"BM":
            print(f"Not a BMP file: {filename}")
            return

        dib = f.read(DIB_HEADER_SIZE)
        width = struct.unpack("<I", dib[4:8])[0]
        height = struct.unpack("<I", dib[8:12])[0]

        print(f"=== Glyph Layout Analysis (screen space): {filename} ===")
        print(f"Dimensions: {width} x {height}\n")

        # Prepare per-screen-row accumulators
        row_counts = [0 for _ in range(height)]
        row_min_x = [width for _ in range(height)]
        row_max_x = [-1 for _ in range(height)]

        # BMP stores from bottom row (y_file=0) to top (y_file=height-1)
        f.seek(BMP_HEADER_SIZE + DIB_HEADER_SIZE)

        for y_file in range(height):
            # Convert to screen coordinates (origin at top-left)
            screen_y = height - 1 - y_file
            for x in range(width):
                bgr = f.read(3)
                if len(bgr) < 3:
                    break
                b, g, r = bgr[0], bgr[1], bgr[2]

                brightness = (r + g + b) // 3
                max_diff = max(abs(r - g), abs(g - b), abs(r - b))

                # Treat only real glyph pixels (dark or strongly colored)
                # Backgrounds like #ffffff and #f0f0f0 are excluded.
                is_glyph = brightness < 200 or max_diff > 30
                if not is_glyph:
                    continue

                row_counts[screen_y] += 1
                if x < row_min_x[screen_y]:
                    row_min_x[screen_y] = x
                if x > row_max_x[screen_y]:
                    row_max_x[screen_y] = x

        text_rows = [y for y in range(height) if row_counts[y] > 0]
        print(f"Total screen rows with glyph pixels: {len(text_rows)}\n")

        print("First 30 glyph rows (screen space, y from top=0):")
        for y in text_rows[:30]:
            cnt = row_counts[y]
            x_min = row_min_x[y]
            x_max = row_max_x[y]
            width_row = x_max - x_min + 1 if x_max >= x_min else 0
            print(f"  y={y:3d}: {cnt:4d} glyph px, x=[{x_min:3d}, {x_max:3d}], width={width_row:3d}px")

        print("\n=== Glyph Line Blocks (screen space) ===")
        if not text_rows:
            print("No glyph pixels detected.")
            return

        blocks = []
        current = {
            "y_start": text_rows[0],
            "y_end": text_rows[0],
            "rows": [text_rows[0]],
        }

        for y in text_rows[1:]:
            if y - current["y_end"] <= 1:
                current["y_end"] = y
                current["rows"].append(y)
            else:
                blocks.append(current)
                current = {"y_start": y, "y_end": y, "rows": [y]}
        blocks.append(current)

        print(f"Detected {len(blocks)} glyph line blocks:\n")
        for idx, blk in enumerate(blocks, start=1):
            y_start = blk["y_start"]
            y_end = blk["y_end"]
            h = y_end - y_start + 1
            total_glyph_px = sum(row_counts[y] for y in blk["rows"])
            avg_width = sum(
                (row_max_x[y] - row_min_x[y] + 1) for y in blk["rows"]
            ) / len(blk["rows"])
            x_min = min(row_min_x[y] for y in blk["rows"])
            x_max = max(row_max_x[y] for y in blk["rows"])

            print(f"  Block {idx}:")
            print(f"    Y range (screen): [{y_start}, {y_end}], height={h}px")
            print(f"    X range: [{x_min}, {x_max}], avg_width={avg_width:.1f}px")
            print(f"    Total glyph pixels: {total_glyph_px}")


if __name__ == "__main__":
    analyze_glyph_layout()
