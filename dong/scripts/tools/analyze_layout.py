import struct
import sys

def analyze_bmp_layout(filename):
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
        
        print(f"=== BMP Layout Analysis: {filename} ===")
        print(f"Dimensions: {width} x {height}")
        
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
        
        # 分析每一行的非白色像素分布
        print("\n=== Row-by-Row Non-White Pixel Analysis ===")
        text_rows = []
        for y in range(height):
            non_white = 0
            min_x = width
            max_x = -1
            for x in range(width):
                r, g, b = pixels[y][x]
                brightness = (r + g + b) // 3
                if brightness < 250:  # 非白色
                    non_white += 1
                    if x < min_x:
                        min_x = x
                    if x > max_x:
                        max_x = x
            
            if non_white > 0:
                text_rows.append({
                    'y': y,
                    'count': non_white,
                    'x_range': (min_x, max_x),
                    'width': max_x - min_x + 1 if max_x >= min_x else 0
                })
        
        # 输出有文字的行
        print(f"Total rows with text: {len(text_rows)}")
        if text_rows:
            print("\nFirst 30 rows with text:")
            for i, row_info in enumerate(text_rows[:30]):
                y = row_info['y']
                count = row_info['count']
                x_min, x_max = row_info['x_range']
                w = row_info['width']
                print(f"  y={y:3d}: {count:4d} pixels, x=[{x_min:3d}, {x_max:3d}], width={w:3d}px")
        
        # 检测文字块（连续的行）
        print("\n=== Text Block Detection ===")
        if text_rows:
            blocks = []
            current_block = {
                'y_start': text_rows[0]['y'],
                'y_end': text_rows[0]['y'],
                'rows': [text_rows[0]]
            }
            
            for i in range(1, len(text_rows)):
                if text_rows[i]['y'] - text_rows[i-1]['y'] <= 2:  # 连续或接近
                    current_block['y_end'] = text_rows[i]['y']
                    current_block['rows'].append(text_rows[i])
                else:
                    blocks.append(current_block)
                    current_block = {
                        'y_start': text_rows[i]['y'],
                        'y_end': text_rows[i]['y'],
                        'rows': [text_rows[i]]
                    }
            blocks.append(current_block)
            
            print(f"Detected {len(blocks)} text blocks:")
            for idx, block in enumerate(blocks):
                y_start = block['y_start']
                y_end = block['y_end']
                height = y_end - y_start + 1
                total_pixels = sum(r['count'] for r in block['rows'])
                avg_width = sum(r['width'] for r in block['rows']) / len(block['rows'])
                x_min = min(r['x_range'][0] for r in block['rows'])
                x_max = max(r['x_range'][1] for r in block['rows'])
                
                # 采样中心像素颜色
                mid_y = (y_start + y_end) // 2
                mid_x = (x_min + x_max) // 2
                sample_r, sample_g, sample_b = pixels[mid_y][mid_x]
                
                print(f"\n  Block {idx+1}:")
                print(f"    Y range: [{y_start}, {y_end}], height={height}px")
                print(f"    X range: [{x_min}, {x_max}], avg_width={avg_width:.1f}px")
                print(f"    Total pixels: {total_pixels}")
                print(f"    Sample color at ({mid_x}, {mid_y}): RGB({sample_r}, {sample_g}, {sample_b})")

if __name__ == '__main__':
    filename = sys.argv[1] if len(sys.argv) > 1 else 'gpu_texture_output.bmp'
    analyze_bmp_layout(filename)
