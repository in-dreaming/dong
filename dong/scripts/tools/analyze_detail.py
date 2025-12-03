import struct
import sys

def analyze_bmp_detail(filename):
    with open(filename, 'rb') as f:
        # Read BMP header
        header = f.read(14)
        if header[0:2] != b'BM':
            print(f"Not a BMP file")
            return
        
        # Read DIB header
        dib_header = f.read(40)
        width = struct.unpack('<I', dib_header[4:8])[0]
        height = struct.unpack('<I', dib_header[8:12])[0]
        
        print(f"=== Detail Analysis of {filename} ===")
        print(f"Dimensions: {width} x {height}\n")
        
        # Read all pixels
        f.seek(54)
        pixels = []
        for y in range(height):
            row = []
            for x in range(width):
                bgr = f.read(3)
                if len(bgr) < 3:
                    break
                b, g, r = bgr[0], bgr[1], bgr[2]
                row.append((r, g, b))
            pixels.append(row)
        
        # Find non-white pixels and analyze their RGB channels
        non_white = []
        channel_diffs = []
        
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[y][x]
                brightness = (r + g + b) // 3
                
                if brightness < 240:  # Consider anything < 240 as potential text
                    non_white.append((x, y, r, g, b, brightness))
                    
                    # Check if RGB channels differ
                    max_diff = max(abs(r-g), abs(g-b), abs(r-b))
                    if max_diff > 5:  # Channels should be same for grayscale
                        channel_diffs.append((x, y, r, g, b, max_diff))
        
        print(f"Found {len(non_white)} non-white pixels (brightness < 240)")
        print(f"Found {len(channel_diffs)} pixels with RGB channel differences (>5)")
        
        if channel_diffs:
            print(f"\n=== Pixels with RGB differences (first 30) ===")
            for i, (x, y, r, g, b, diff) in enumerate(channel_diffs[:30]):
                print(f"  ({x:4d}, {y:3d}): RGB({r:3d},{g:3d},{b:3d}) max_diff={diff:3d}")
        
        # Sample a region around text
        if non_white:
            print(f"\n=== Sampling 5x5 region around first text pixel ===")
            x0, y0 = non_white[0][0], non_white[0][1]
            for dy in range(-2, 3):
                for dx in range(-2, 3):
                    nx, ny = x0 + dx, y0 + dy
                    if 0 <= nx < width and 0 <= ny < height:
                        r, g, b = pixels[ny][nx]
                        marker = "**" if (dx == 0 and dy == 0) else "  "
                        print(f"{marker}({nx:4d},{ny:3d}):RGB({r:3d},{g:3d},{b:3d})", end="  ")
                print()

if __name__ == "__main__":
    filename = sys.argv[1] if len(sys.argv) > 1 else 'gpu_texture_output.bmp'
    analyze_bmp_detail(filename)
