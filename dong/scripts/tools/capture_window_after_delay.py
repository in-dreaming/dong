#!/usr/bin/env python3
"""Capture a process main window after a delay (PrintWindow + BMP)."""
from __future__ import annotations

import argparse
import ctypes
import struct
import subprocess
import sys
import time
from ctypes import wintypes

user32 = ctypes.windll.user32
gdi32 = ctypes.windll.gdi32

EnumWindowsProc = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)


def find_visible_window_for_pid(pid: int) -> int | None:
    found: list[int] = []

    def cb(hwnd, _):
        window_pid = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(window_pid))
        if window_pid.value == pid and user32.IsWindowVisible(hwnd):
            found.append(hwnd)
        return True

    user32.EnumWindows(EnumWindowsProc(cb), 0)
    return found[0] if found else None


def capture_print_window(hwnd: int, out_path: str) -> tuple[int, int, bytes]:
    rect = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    w, h = rect.right - rect.left, rect.bottom - rect.top
    if w <= 0 or h <= 0:
        raise RuntimeError(f"invalid window rect {w}x{h}")

    hdc_window = user32.GetDC(hwnd)
    hdc_mem = gdi32.CreateCompatibleDC(hdc_window)
    hbm = gdi32.CreateCompatibleBitmap(hdc_window, w, h)
    gdi32.SelectObject(hdc_mem, hbm)
    ok = user32.PrintWindow(hwnd, hdc_mem, 2)
    if not ok:
        user32.ReleaseDC(hwnd, hdc_window)
        gdi32.DeleteObject(hbm)
        gdi32.DeleteDC(hdc_mem)
        raise RuntimeError("PrintWindow failed")

    class BITMAPINFOHEADER(ctypes.Structure):
        _fields_ = [
            ("biSize", wintypes.DWORD),
            ("biWidth", wintypes.LONG),
            ("biHeight", wintypes.LONG),
            ("biPlanes", wintypes.WORD),
            ("biBitCount", wintypes.WORD),
            ("biCompression", wintypes.DWORD),
            ("biSizeImage", wintypes.DWORD),
            ("biXPelsPerMeter", wintypes.LONG),
            ("biYPelsPerMeter", wintypes.LONG),
            ("biClrUsed", wintypes.DWORD),
            ("biClrImportant", wintypes.DWORD),
        ]

    bmi = BITMAPINFOHEADER()
    bmi.biSize = ctypes.sizeof(BITMAPINFOHEADER)
    bmi.biWidth = w
    bmi.biHeight = -h
    bmi.biPlanes = 1
    bmi.biBitCount = 32
    buf = (ctypes.c_char * (w * h * 4))()
    gdi32.GetDIBits(hdc_mem, hbm, 0, h, buf, ctypes.byref(bmi), 0)

    user32.ReleaseDC(hwnd, hdc_window)
    gdi32.DeleteObject(hbm)
    gdi32.DeleteDC(hdc_mem)

    data = bytes(buf)
    row = ((w * 3 + 3) // 4) * 4
    pixel_bytes = row * h
    file_size = 14 + 40 + pixel_bytes
    with open(out_path, "wb") as f:
        f.write(b"BM")
        f.write(struct.pack("<I", file_size))
        f.write(b"\0\0\0\0")
        f.write(struct.pack("<I", 54))
        f.write(struct.pack("<I", 40))
        f.write(struct.pack("<i", w))
        f.write(struct.pack("<i", h))
        f.write(struct.pack("<HH", 1, 24))
        f.write(struct.pack("<I", 0))
        f.write(struct.pack("<I", pixel_bytes))
        for y in range(h):
            row_bytes = bytearray(row)
            for x in range(w):
                i = (y * w + x) * 4
                b, g, r, _a = data[i], data[i + 1], data[i + 2], data[i + 3]
                row_bytes[x * 3 : x * 3 + 3] = bytes([b, g, r])
            f.write(row_bytes)

    return w, h, data


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("command", nargs=argparse.REMAINDER, help="command to launch")
    ap.add_argument("--delay", type=float, default=10.0)
    ap.add_argument("--output", required=True)
    args = ap.parse_args()
    if not args.command:
        print("ERROR: missing command", file=sys.stderr)
        return 1

    proc = subprocess.Popen(args.command)
    time.sleep(args.delay)
    hwnd = find_visible_window_for_pid(proc.pid)
    if not hwnd:
        proc.terminate()
        print(f"ERROR: no visible window for pid {proc.pid}", file=sys.stderr)
        return 1

    try:
        w, h, data = capture_print_window(hwnd, args.output)
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            proc.kill()

    cx, cy = w // 2, h // 2
    i = (cy * w + cx) * 4
    bright = sum(
        1
        for p in range(0, len(data), 4)
        if data[p] > 200 and data[p + 1] > 200 and data[p + 2] > 200
    )
    print(f"hwnd={hwnd} size={w}x{h} center_rgba=({data[i+2]},{data[i+1]},{data[i]},{data[i+3]}) bright={bright}")
    print(f"saved {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
