import struct
import sys

def analyze_bmp(filename):
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
        
        print(f"=== Analyzing {filename} ===")
        print(f"Dimensions: {width} x {height}")
        
        # Read pixel data (BGR format in BMP)
        f.seek(54)  # Skip headers
        total_pixels = width * height
        
        black_pixels = 0
        gray_pixels = 0
        white_pixels = 0
        color_pixels = 0
        
        # Sample pixels
        sample_pixels = []
        for y in range(height):
            for x in range(width):
                bgr = f.read(3)
                if len(bgr) < 3:
                    break
                b, g, r = bgr[0], bgr[1], bgr[2]
                
                brightness = (r + g + b) // 3
                max_diff = max(abs(r-g), abs(g-b), abs(r-b))
                
                if max_diff > 30:
                    color_pixels += 1
                elif brightness < 50:
                    black_pixels += 1
                elif brightness > 200:
                    white_pixels += 1
                else:
                    gray_pixels += 1
                
                # Sample some pixels for debugging
                if len(sample_pixels) < 100 and brightness < 200:
                    sample_pixels.append((x, y, r, g, b, brightness))
        
        print(f"\nPixel Statistics:")
        print(f"  Total: {total_pixels}")
        print(f"  Black (<50): {black_pixels} ({100.0*black_pixels/total_pixels:.2f}%)")
        print(f"  Gray (50-200): {gray_pixels} ({100.0*gray_pixels/total_pixels:.2f}%)")
        print(f"  White (>200): {white_pixels} ({100.0*white_pixels/total_pixels:.2f}%)")
        print(f"  Color (varied): {color_pixels} ({100.0*color_pixels/total_pixels:.2f}%)")
        
        text_pixels = black_pixels + gray_pixels + color_pixels
        print(f"\nText pixels (non-white): {text_pixels} ({100.0*text_pixels/total_pixels:.2f}%)")
        
        if sample_pixels:
            print(f"\nSample non-white pixels (first 20):")
            for x, y, r, g, b, brightness in sample_pixels[:20]:
                print(f"  ({x:4d}, {y:3d}): RGB({r:3d},{g:3d},{b:3d}) brightness={brightness}")

if __name__ == "__main__":
    filename = sys.argv[1] if len(sys.argv) > 1 else 'gpu_texture_output.bmp'
    analyze_bmp(filename)
