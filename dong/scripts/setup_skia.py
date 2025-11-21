#!/usr/bin/env python3
"""
Skia precompiled library setup script
Supports macOS, Windows, and Linux
"""

import os
import sys
import json
import urllib.request
import zipfile
import shutil
from pathlib import Path

# Skia binaries source configuration
SKIA_SOURCES = {
    "darwin": {
        "name": "macOS (ARM64)",
        # Use official Skia releases or community builds
        "url": "https://github.com/aseprite/skia/releases/download/m102-f2d8e0f/Skia-macOS-Release-arm64.zip",
        "dir": "mac"
    },
    "win32": {
        "name": "Windows (x64)",
        # Windows build - you may need to adjust based on availability
        "url": "https://github.com/aseprite/skia/releases/download/m102-f2d8e0f/Skia-Windows-Release-x64.zip",
        "dir": "windows"
    },
    "linux": {
        "name": "Linux (x64)",
        # Linux build
        "url": "https://github.com/aseprite/skia/releases/download/m102-f2d8e0f/Skia-Linux-Release-x64.zip",
        "dir": "linux"
    }
}

def get_platform():
    """Detect current platform"""
    if sys.platform == "darwin":
        return "darwin"
    elif sys.platform == "win32":
        return "win32"
    elif sys.platform.startswith("linux"):
        return "linux"
    else:
        raise RuntimeError(f"Unsupported platform: {sys.platform}")

def download_file(url, dest):
    """Download file with progress"""
    print(f"Downloading: {url}")
    try:
        urllib.request.urlretrieve(url, dest)
        print(f"Downloaded to: {dest}")
    except Exception as e:
        print(f"Download failed: {e}")
        return False
    return True

def extract_skia(zip_path, extract_dir):
    """Extract Skia archive"""
    print(f"Extracting: {zip_path}")
    try:
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)
        print(f"Extracted to: {extract_dir}")
    except Exception as e:
        print(f"Extraction failed: {e}")
        return False
    return True

def setup_skia(platform=None):
    """Main setup function"""
    if platform is None:
        platform = get_platform()
    
    if platform not in SKIA_SOURCES:
        print(f"Error: Unsupported platform {platform}")
        return False
    
    config = SKIA_SOURCES[platform]
    print(f"\nSetting up Skia for {config['name']}")
    
    # Paths
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    skia_dir = project_root / "third_party" / "skia" / config["dir"]
    
    # Create directory if needed
    skia_dir.mkdir(parents=True, exist_ok=True)
    
    # Check if already set up
    if (skia_dir / "include").exists() and (skia_dir / "lib").exists():
        print(f"✓ Skia already set up at {skia_dir}")
        return True
    
    # Download
    zip_name = f"Skia-{platform}.zip"
    zip_path = project_root / zip_name
    
    if not zip_path.exists():
        if not download_file(config["url"], str(zip_path)):
            print("Failed to download Skia")
            return False
    else:
        print(f"Using cached: {zip_path}")
    
    # Extract
    temp_extract = project_root / "skia_temp"
    if not extract_skia(str(zip_path), str(temp_extract)):
        print("Failed to extract Skia")
        return False
    
    # Move to final location
    try:
        # Find the extracted Skia folder (might be nested)
        extracted_items = list(temp_extract.iterdir())
        
        # Handle common extraction patterns
        if len(extracted_items) == 1 and extracted_items[0].is_dir():
            source = extracted_items[0]
        else:
            source = temp_extract
        
        # Copy include and lib directories
        src_include = source / "include"
        src_lib = source / "lib"
        
        if src_include.exists():
            dst_include = skia_dir / "include"
            if dst_include.exists():
                shutil.rmtree(dst_include)
            shutil.copytree(src_include, dst_include)
            print(f"✓ Copied headers to {dst_include}")
        
        if src_lib.exists():
            dst_lib = skia_dir / "lib"
            if dst_lib.exists():
                shutil.rmtree(dst_lib)
            shutil.copytree(src_lib, dst_lib)
            print(f"✓ Copied libraries to {dst_lib}")
        
        # Cleanup
        shutil.rmtree(temp_extract)
        zip_path.unlink()
        
        print(f"\n✓ Skia setup complete at {skia_dir}")
        return True
        
    except Exception as e:
        print(f"Failed to setup: {e}")
        if temp_extract.exists():
            shutil.rmtree(temp_extract)
        return False

if __name__ == "__main__":
    platform_arg = sys.argv[1] if len(sys.argv) > 1 else None
    
    if not setup_skia(platform_arg):
        sys.exit(1)
    
    print("\n=== Next Steps ===")
    print("1. Run: cd dong")
    print("2. Build: zig build")
