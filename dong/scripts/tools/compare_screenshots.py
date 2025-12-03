import struct
import sys
from typing import List, Tuple

BMP_HEADER_SIZE = 14
DIB_HEADER_SIZE = 40

RGB = Tuple[int, int, int]
PixelGrid = List[List[RGB]]


def read_bmp_pixels(path: str) -> Tuple[int, int, PixelGrid]:
    with open(path, "rb") as f:
        header = f.read(BMP_HEADER_SIZE)
        if header[0:2] != b"BM":
            raise ValueError(f"{path!r} is not a BMP file")

        dib = f.read(DIB_HEADER_SIZE)
        width = struct.unpack("<I", dib[4:8])[0]
        height = struct.unpack("<I", dib[8:12])[0]

        # NOTE: we assume 24-bit BGR, uncompressed, like engine outputs.
        # Existing analysis scripts in this repo make the same assumption.

        # Pixel data starts at offset 54 for this BMP variant.
        f.seek(BMP_HEADER_SIZE + DIB_HEADER_SIZE)

        pixels: PixelGrid = []
        for _y in range(height):
            row: List[RGB] = []
            for _x in range(width):
                bgr = f.read(3)
                if len(bgr) < 3:
                    raise ValueError(f"Unexpected EOF while reading pixels from {path!r}")
                b, g, r = bgr[0], bgr[1], bgr[2]
                row.append((r, g, b))
            pixels.append(row)

        return width, height, pixels


def compare_bmps(ref_path: str, test_path: str) -> None:
    print(f"=== Comparing BMP screenshots ===")
    print(f"Reference: {ref_path}")
    print(f"Test:      {test_path}\n")

    ref_w, ref_h, ref_px = read_bmp_pixels(ref_path)
    test_w, test_h, test_px = read_bmp_pixels(test_path)

    print(f"Reference size: {ref_w} x {ref_h}")
    print(f"Test size:      {test_w} x {test_h}")

    if (ref_w, ref_h) != (test_w, test_h):
        print("\n[ERROR] Dimensions differ, cannot do 1:1 pixel comparison.")
        return

    width, height = ref_w, ref_h
    total_pixels = width * height

    # Per-channel stats
    sum_abs_r = 0
    sum_abs_g = 0
    sum_abs_b = 0
    max_abs_r = 0
    max_abs_g = 0
    max_abs_b = 0

    # Count pixels whose max-channel diff exceeds thresholds
    diff_gt_0 = 0
    diff_gt_1 = 0
    diff_gt_2 = 0

    for y in range(height):
        ref_row = ref_px[y]
        test_row = test_px[y]
        for x in range(width):
            rr, rg, rb = ref_row[x]
            tr, tg, tb = test_row[x]

            dr = abs(int(rr) - int(tr))
            dg = abs(int(rg) - int(tg))
            db = abs(int(rb) - int(tb))

            sum_abs_r += dr
            sum_abs_g += dg
            sum_abs_b += db

            if dr > max_abs_r:
                max_abs_r = dr
            if dg > max_abs_g:
                max_abs_g = dg
            if db > max_abs_b:
                max_abs_b = db

            max_delta = max(dr, dg, db)
            if max_delta > 0:
                diff_gt_0 += 1
            if max_delta > 1:
                diff_gt_1 += 1
            if max_delta > 2:
                diff_gt_2 += 1

    mean_abs_r = sum_abs_r / total_pixels
    mean_abs_g = sum_abs_g / total_pixels
    mean_abs_b = sum_abs_b / total_pixels

    print("\n=== Per-channel absolute difference ===")
    print(f"Max |ΔR| = {max_abs_r}")
    print(f"Max |ΔG| = {max_abs_g}")
    print(f"Max |ΔB| = {max_abs_b}")
    print(f"Mean |ΔR| = {mean_abs_r:.3f}")
    print(f"Mean |ΔG| = {mean_abs_g:.3f}")
    print(f"Mean |ΔB| = {mean_abs_b:.3f}")

    print("\n=== Pixel counts by max-channel diff ===")
    def pct(n: int) -> float:
        return 100.0 * n / total_pixels if total_pixels > 0 else 0.0

    print(f"Δ>0  : {diff_gt_0:8d} pixels ({pct(diff_gt_0):6.3f}%)")
    print(f"Δ>1  : {diff_gt_1:8d} pixels ({pct(diff_gt_1):6.3f}%)")
    print(f"Δ>2  : {diff_gt_2:8d} pixels ({pct(diff_gt_2):6.3f}%)")

    print("\nNote:")
    print("- align2.md 的颜色验收标准是：每通道误差 ≤ 1–2。")
    print("- 你可以主要关注 Δ>1 / Δ>2 的像素比例是否在可接受范围内。")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 compare_screenshots.py <ref.bmp> <test.bmp>")
        sys.exit(1)

    compare_bmps(sys.argv[1], sys.argv[2])
