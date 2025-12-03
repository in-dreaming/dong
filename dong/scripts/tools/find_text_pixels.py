import struct

with open('gpu_texture_output.bmp', 'rb') as f:
    # 读取整个图像
    f.seek(54)
    pixels = []
    for y in range(400):
        for x in range(800):
            bgr = f.read(3)
            b, g, r = bgr[0], bgr[1], bgr[2]
            pixels.append((x, y, r, g, b))
    
    # 找出与背景色RGB(245,245,245)差异较大的像素
    print("=== Non-Background Pixels (first 100) ===")
    count = 0
    for x, y, r, g, b in pixels:
        if abs(r - 245) > 10 or abs(g - 245) > 10 or abs(b - 245) > 10:
            print(f"  ({x:3d}, {y:3d}): RGB({r:3d}, {g:3d}, {b:3d})")
            count += 1
            if count >= 100:
                break
    
    print(f"\nTotal non-background pixels found: {count}")
    
    if count == 0:
        print("\n=== Checking for ANY variation from RGB(245,245,245) ===")
        count2 = 0
        variations = {}
        for x, y, r, g, b in pixels:
            if r != 245 or g != 245 or b != 245:
                key = (r, g, b)
                if key not in variations:
                    variations[key] = []
                variations[key].append((x, y))
                count2 += 1
        
        print(f"Total pixels with variation: {count2}")
        print(f"Unique colors: {len(variations)}")
        
        if variations:
            print("\nTop 10 color variations:")
            sorted_colors = sorted(variations.items(), key=lambda x: len(x[1]), reverse=True)
            for i, (color, positions) in enumerate(sorted_colors[:10]):
                r, g, b = color
                print(f"  RGB({r:3d}, {g:3d}, {b:3d}): {len(positions)} pixels")
                if i < 3:  # 显示前3种颜色的前5个位置
                    for x, y in positions[:5]:
                        print(f"    at ({x:3d}, {y:3d})")
