#!/usr/bin/env python3
"""
pack_resources.py - Pack fonts, shaders, and images into a .dpkg binary file.

Usage:
    python pack_resources.py --fonts fonts/ --shaders shaders/ --images images/ -o game_ui.dpkg
    python pack_resources.py --font-file NotoSans.ttf --font-file NotoSansCJK.ttf -o minimal.dpkg

The .dpkg format:
    Header (32 bytes) + Entry Table (32 bytes * N) + String Table + Data Section
"""

import argparse
import os
import struct
import sys
from pathlib import Path

DPKG_MAGIC = 0x474B5044  # "DPKG" little-endian
DPKG_VERSION = 1

RESOURCE_FONT = 1
RESOURCE_ATLAS = 2
RESOURCE_SHADER = 3
RESOURCE_IMAGE = 4

TYPE_MAP = {
    '.ttf': RESOURCE_FONT,
    '.otf': RESOURCE_FONT,
    '.woff': RESOURCE_FONT,
    '.woff2': RESOURCE_FONT,
    '.spv': RESOURCE_SHADER,
    '.spirv': RESOURCE_SHADER,
    '.dxil': RESOURCE_SHADER,
    '.metallib': RESOURCE_SHADER,
    '.png': RESOURCE_IMAGE,
    '.jpg': RESOURCE_IMAGE,
    '.jpeg': RESOURCE_IMAGE,
    '.bmp': RESOURCE_IMAGE,
    '.webp': RESOURCE_IMAGE,
}


def collect_files(dirs, file_list, type_filter=None):
    """Collect files from directories and explicit file list."""
    entries = []

    for d in (dirs or []):
        p = Path(d)
        if not p.is_dir():
            print(f"Warning: {d} is not a directory, skipping", file=sys.stderr)
            continue
        for f in sorted(p.rglob('*')):
            if f.is_file():
                ext = f.suffix.lower()
                rtype = TYPE_MAP.get(ext)
                if rtype and (type_filter is None or rtype == type_filter):
                    name = f.name  # Use filename as key
                    entries.append((name, rtype, f))

    for f in (file_list or []):
        p = Path(f)
        if p.is_file():
            ext = p.suffix.lower()
            rtype = TYPE_MAP.get(ext, RESOURCE_FONT)
            entries.append((p.name, rtype, p))

    return entries


def pack(entries, output_path):
    """Pack entries into .dpkg format."""
    entry_count = len(entries)

    # Build string table
    string_table = bytearray()
    name_offsets = []
    for name, rtype, path in entries:
        name_offsets.append(len(string_table))
        string_table.extend(name.encode('utf-8'))
        string_table.append(0)  # null terminator

    # Calculate offsets
    header_size = 32
    entry_table_size = entry_count * 32
    string_table_offset = header_size + entry_table_size
    data_section_offset = string_table_offset + len(string_table)
    # Align data section to 16 bytes
    data_section_offset = (data_section_offset + 15) & ~15

    # Read all file data
    file_data = []
    data_offsets = []
    current_offset = data_section_offset
    for name, rtype, path in entries:
        raw = path.read_bytes()
        data_offsets.append(current_offset)
        file_data.append(raw)
        current_offset += len(raw)

    # Write .dpkg file
    with open(output_path, 'wb') as f:
        # Header
        f.write(struct.pack('<IIII16s',
                           DPKG_MAGIC, DPKG_VERSION, entry_count, 0,
                           b'\x00' * 16))

        # Entry table
        for i, (name, rtype, path) in enumerate(entries):
            data_size = len(file_data[i])
            f.write(struct.pack('<IIQQQ',
                               rtype,
                               name_offsets[i],
                               data_offsets[i],
                               data_size,
                               0))  # original_size = 0 (uncompressed)

        # String table
        f.write(bytes(string_table))

        # Padding to align data section
        pad = data_section_offset - (header_size + entry_table_size + len(string_table))
        if pad > 0:
            f.write(b'\x00' * pad)

        # Data section
        for raw in file_data:
            f.write(raw)

    total_size = current_offset
    print(f"Packed {entry_count} entries into {output_path} ({total_size:,} bytes)")
    for i, (name, rtype, path) in enumerate(entries):
        type_names = {1: 'FONT', 2: 'ATLAS', 3: 'SHADER', 4: 'IMAGE'}
        print(f"  [{type_names.get(rtype, '?'):6s}] {name} ({len(file_data[i]):,} bytes)")


def main():
    parser = argparse.ArgumentParser(description='Pack resources into .dpkg format')
    parser.add_argument('--fonts', nargs='*', help='Directories containing font files')
    parser.add_argument('--shaders', nargs='*', help='Directories containing shader files')
    parser.add_argument('--images', nargs='*', help='Directories containing image files')
    parser.add_argument('--font-file', action='append', help='Individual font file to include')
    parser.add_argument('--shader-file', action='append', help='Individual shader file to include')
    parser.add_argument('-o', '--output', required=True, help='Output .dpkg file path')
    args = parser.parse_args()

    entries = []
    entries.extend(collect_files(args.fonts, args.font_file, RESOURCE_FONT))
    entries.extend(collect_files(args.shaders, args.shader_file, RESOURCE_SHADER))
    entries.extend(collect_files(args.images, None, RESOURCE_IMAGE))

    if not entries:
        print("Error: No files found to pack", file=sys.stderr)
        sys.exit(1)

    pack(entries, args.output)


if __name__ == '__main__':
    main()
