#!/usr/bin/env python3
"""
DXC (DirectX Shader Compiler) Download Script

Downloads the appropriate DXC binaries from GitHub releases for the current platform.
DXC is required for HLSL shader compilation on Vulkan backends.

Supported platforms:
- Windows x64/arm64
- Linux x64
- macOS x64/arm64 (via Homebrew recommended)

Usage:
    python scripts/download_dxc.py [--version VERSION] [--output DIR]

Example:
    python scripts/download_dxc.py --version 2025_07_14 --output third_party/dxc_2025_07_14
"""

import os
import sys
import platform
import argparse
import urllib.request
import zipfile
import tarfile
import tempfile
import shutil
from pathlib import Path

# DXC GitHub release info
DXC_REPO = "microsoft/DirectXShaderCompiler"
DEFAULT_VERSION = "2025_07_14"  # Format: YYYY_MM_DD
GITHUB_API_BASE = "https://api.github.com/repos"
GITHUB_RELEASE_BASE = "https://github.com"


def get_platform_info():
    """Detect current platform and return appropriate DXC download info."""
    system = platform.system().lower()
    machine = platform.machine().lower()

    # Normalize architecture names
    arch_map = {
        'x86_64': 'x64',
        'amd64': 'x64',
        'aarch64': 'arm64',
        'arm64': 'arm64',
    }
    arch = arch_map.get(machine, machine)

    if system == 'windows':
        return {
            'os': 'windows',
            'arch': arch,
            'lib_name': 'dxcompiler.dll',
            'lib_ext': '.dll',
            'archive_ext': '.zip',
        }
    elif system == 'linux':
        return {
            'os': 'linux',
            'arch': arch,
            'lib_name': 'libdxcompiler.so',
            'lib_ext': '.so',
            'archive_ext': '.tar.gz',
        }
    elif system == 'darwin':
        return {
            'os': 'macos',
            'arch': arch,
            'lib_name': 'libdxcompiler.dylib',
            'lib_ext': '.dylib',
            'archive_ext': '.tar.gz',
            'homebrew_hint': 'brew install dxc',
        }
    else:
        raise RuntimeError(f"Unsupported platform: {system}")


def get_release_url(version: str, platform_info: dict) -> str:
    """Construct the GitHub release download URL."""
    # DXC release naming convention varies, common patterns:
    # - dxc_<version>_<arch>.zip for Windows
    # - linux_dxc_<version>.x86_64.tar.gz for Linux

    os_name = platform_info['os']
    arch = platform_info['arch']

    # Tag format: v1.8.YYMM.DD or similar
    # We'll try to find the right asset from the release
    tag = f"v1.8.{version.replace('_', '.')}"

    if os_name == 'windows':
        asset_name = f"dxc_{version}_{arch}.zip"
    elif os_name == 'linux':
        asset_name = f"linux_dxc_{version}.x86_64.tar.gz"
    else:
        # macOS - recommend homebrew
        return None

    return f"{GITHUB_RELEASE_BASE}/{DXC_REPO}/releases/download/{tag}/{asset_name}"


def download_file(url: str, dest_path: Path) -> bool:
    """Download a file from URL to destination."""
    print(f"Downloading: {url}")
    try:
        with urllib.request.urlopen(url, timeout=60) as response:
            with open(dest_path, 'wb') as out_file:
                shutil.copyfileobj(response, out_file)
        return True
    except Exception as e:
        print(f"Download failed: {e}")
        return False


def extract_archive(archive_path: Path, dest_dir: Path, archive_ext: str):
    """Extract archive to destination directory."""
    print(f"Extracting to: {dest_dir}")
    dest_dir.mkdir(parents=True, exist_ok=True)

    if archive_ext == '.zip':
        with zipfile.ZipFile(archive_path, 'r') as zf:
            zf.extractall(dest_dir)
    elif archive_ext in ('.tar.gz', '.tgz'):
        with tarfile.open(archive_path, 'r:gz') as tf:
            tf.extractall(dest_dir)
    elif archive_ext == '.tar':
        with tarfile.open(archive_path, 'r') as tf:
            tf.extractall(dest_dir)
    else:
        raise ValueError(f"Unknown archive format: {archive_ext}")


def organize_files(src_dir: Path, dest_dir: Path, platform_info: dict):
    """Organize extracted files into proper directory structure."""
    os_name = platform_info['os']
    arch = platform_info['arch']

    # Create output structure
    bin_dir = dest_dir / 'bin' / arch if os_name == 'windows' else dest_dir / 'bin' / os_name
    lib_dir = dest_dir / 'lib' / arch if os_name == 'windows' else dest_dir / 'lib'
    include_dir = dest_dir / 'include'

    bin_dir.mkdir(parents=True, exist_ok=True)
    lib_dir.mkdir(parents=True, exist_ok=True)
    include_dir.mkdir(parents=True, exist_ok=True)

    # Find and copy files
    for root, dirs, files in os.walk(src_dir):
        root_path = Path(root)
        for f in files:
            src_file = root_path / f

            # Library files
            if f.endswith(('.dll', '.so', '.dylib')):
                shutil.copy2(src_file, bin_dir / f)
                print(f"  Copied: {f} -> bin/")

            # Import libraries (Windows)
            elif f.endswith('.lib'):
                shutil.copy2(src_file, lib_dir / f)
                print(f"  Copied: {f} -> lib/")

            # Header files
            elif f.endswith('.h'):
                # Preserve directory structure for includes
                rel_path = src_file.relative_to(src_dir)
                if 'include' in str(rel_path):
                    # Extract the include subdirectory
                    parts = rel_path.parts
                    if 'include' in parts:
                        idx = parts.index('include')
                        sub_path = Path(*parts[idx+1:])
                        dest_file = include_dir / sub_path
                        dest_file.parent.mkdir(parents=True, exist_ok=True)
                        shutil.copy2(src_file, dest_file)
                        print(f"  Copied: {f} -> include/{sub_path}")


def main():
    parser = argparse.ArgumentParser(
        description='Download DXC (DirectX Shader Compiler) binaries'
    )
    parser.add_argument(
        '--version', '-v',
        default=DEFAULT_VERSION,
        help=f'DXC version to download (default: {DEFAULT_VERSION})'
    )
    parser.add_argument(
        '--output', '-o',
        default=None,
        help='Output directory (default: third_party/dxc_<version>)'
    )
    parser.add_argument(
        '--force', '-f',
        action='store_true',
        help='Force re-download even if files exist'
    )

    args = parser.parse_args()

    # Get platform info
    try:
        platform_info = get_platform_info()
    except RuntimeError as e:
        print(f"Error: {e}")
        return 1

    print(f"Platform: {platform_info['os']} {platform_info['arch']}")

    # macOS special handling
    if platform_info['os'] == 'macos':
        print("\nFor macOS, DXC is best installed via Homebrew:")
        print(f"  {platform_info.get('homebrew_hint', 'brew install dxc')}")
        print("\nAlternatively, download manually from:")
        print(f"  https://github.com/{DXC_REPO}/releases")
        return 0

    # Set output directory
    if args.output:
        output_dir = Path(args.output)
    else:
        output_dir = Path(f"third_party/dxc_{args.version}")

    # Check if already exists
    if output_dir.exists() and not args.force:
        lib_file = output_dir / 'bin' / platform_info['arch'] / platform_info['lib_name']
        if lib_file.exists():
            print(f"DXC already installed at: {output_dir}")
            print(f"Use --force to re-download")
            return 0

    # Get download URL
    url = get_release_url(args.version, platform_info)
    if not url:
        print("Could not determine download URL")
        return 1

    # Download to temp directory
    with tempfile.TemporaryDirectory() as tmp_dir:
        tmp_path = Path(tmp_dir)
        archive_name = f"dxc{platform_info['archive_ext']}"
        archive_path = tmp_path / archive_name

        if not download_file(url, archive_path):
            print("\nDownload failed. You can manually download from:")
            print(f"  https://github.com/{DXC_REPO}/releases")
            return 1

        # Extract
        extract_dir = tmp_path / 'extracted'
        extract_archive(archive_path, extract_dir, platform_info['archive_ext'])

        # Organize files
        output_dir.mkdir(parents=True, exist_ok=True)
        organize_files(extract_dir, output_dir, platform_info)

    print(f"\nDXC installed to: {output_dir}")
    print("\nAdd to build.env:")
    print(f"  DXC_LIB_PATH={output_dir}/lib/{platform_info['arch']}")

    return 0


if __name__ == '__main__':
    sys.exit(main())
