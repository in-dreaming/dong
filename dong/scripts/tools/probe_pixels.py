from PIL import Image

# Known offscreen size
WIDTH, HEIGHT = 800, 400

# Helper: convert logical screen coords (origin top-left) to BMP row index used by writeBMP
# writeBMP wrote rows from y=HEIGHT-1 down to 0, so file_y = HEIGHT-1 - screen_y

def sample_points(img_path, points):
    img = Image.open(img_path).convert("RGB")
    print(f"Image size: {img.size}")
    for name, sx, sy in points:
        file_y = HEIGHT - 1 - sy
        file_x = sx
        r, g, b = img.getpixel((file_x, file_y))
        print(f"{name}: screen=({sx},{sy}) -> file=({file_x},{file_y}) RGB=({r},{g},{b})")

if __name__ == "__main__":
    # Sample a few glyph centers based on GLYPH_CALC logs
    # 1) H1 first glyph: approx pos=(32.3,51.6), size=(31.5,36.3)
    h1_center_x = int(32.3 + 31.5 / 2)
    h1_center_y = int(51.6 + 36.3 / 2)

    # 2) info text first glyph: pos≈(30.4,148.3), size≈(14.0,17.2)
    info_center_x = int(30.4 + 14.0 / 2)
    info_center_y = int(148.3 + 17.2 / 2)

    # 3) highlight text first glyph: pos≈(32.5,164.5), size≈(8.1,17.1)
    hi_center_x = int(32.5 + 8.1 / 2)
    hi_center_y = int(164.5 + 17.1 / 2)

    pts = [
        ("h1_center", h1_center_x, h1_center_y),
        ("info_center", info_center_x, info_center_y),
        ("highlight_center", hi_center_x, hi_center_y),
    ]

    sample_points("gpu_texture_output.bmp", pts)
