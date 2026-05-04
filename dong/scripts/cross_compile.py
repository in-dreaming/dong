#!/usr/bin/env python3
"""
Dong Cross-Compilation Helper

Simplifies cross-compilation to different target platforms.

Usage:
    python scripts/cross_compile.py [target] [--clean]

Targets:
    linux-x64       Linux x86_64 (glibc)
    linux-arm64     Linux aarch64 (glibc)
    linux-musl-x64  Linux x86_64 (musl, static)
    linux-musl-arm64 Linux aarch64 (musl, static)

Note: macOS and iOS targets require the macOS SDK (only available on macOS host)
"""

import os
import sys
import subprocess
import shutil
import argparse
from pathlib import Path

# Target configurations
TARGETS = {
    "linux-x64": {
        "triple": "x86_64-linux-gnu",
        "output": "zig-out-linux-x64",
        "description": "Linux x86_64 (glibc)",
    },
    "linux-arm64": {
        "triple": "aarch64-linux-gnu",
        "output": "zig-out-linux-arm64",
        "description": "Linux ARM64 (glibc)",
    },
    "linux-musl-x64": {
        "triple": "x86_64-linux-musl",
        "output": "zig-out-linux-musl-x64",
        "description": "Linux x86_64 (musl, static)",
    },
    "linux-musl-arm64": {
        "triple": "aarch64-linux-musl",
        "output": "zig-out-linux-musl-arm64",
        "description": "Linux ARM64 (musl, static)",
    },
}


def run_build(target_config: dict, clean: bool = False) -> int:
    """Run cross-compilation for a target."""
    output_dir = target_config["output"]
    triple = target_config["triple"]

    # Clean if requested
    if clean and Path(output_dir).exists():
        print(f"Cleaning {output_dir}...")
        shutil.rmtree(output_dir)

    # Build command
    cmd = [
        "zig", "build",
        f"-Dtarget={triple}",
        "-Dlibs-only=true",
        f"-p", output_dir,
        "deps"
    ]

    print(f"\n{'='*60}")
    print(f"Building for: {target_config['description']}")
    print(f"Target: {triple}")
    print(f"Output: {output_dir}")
    print(f"{'='*60}\n")

    result = subprocess.run(cmd)

    if result.returncode == 0:
        # List output files
        lib_dir = Path(output_dir) / "lib"
        if lib_dir.exists():
            print(f"\nGenerated libraries in {lib_dir}:")
            for f in sorted(lib_dir.glob("*.a")):
                size = f.stat().st_size / 1024 / 1024  # MB
                print(f"  {f.name}: {size:.1f} MB")

    return result.returncode


def main():
    parser = argparse.ArgumentParser(
        description="Dong Cross-Compilation Helper",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    python scripts/cross_compile.py linux-x64
    python scripts/cross_compile.py linux-arm64 --clean
    python scripts/cross_compile.py all
        """
    )
    parser.add_argument(
        "target",
        choices=list(TARGETS.keys()) + ["all", "list"],
        help="Target platform to build for"
    )
    parser.add_argument(
        "--clean", "-c",
        action="store_true",
        help="Clean output directory before building"
    )

    args = parser.parse_args()

    # Change to dong directory
    script_dir = Path(__file__).parent
    dong_dir = script_dir.parent
    os.chdir(dong_dir)

    if args.target == "list":
        print("Available targets:")
        for name, config in TARGETS.items():
            print(f"  {name:20} - {config['description']}")
        return 0

    if args.target == "all":
        failed = []
        for name, config in TARGETS.items():
            ret = run_build(config, args.clean)
            if ret != 0:
                failed.append(name)

        print(f"\n{'='*60}")
        print("Summary:")
        print(f"  Succeeded: {len(TARGETS) - len(failed)}")
        print(f"  Failed: {len(failed)}")
        if failed:
            print(f"  Failed targets: {', '.join(failed)}")
        print(f"{'='*60}")
        return 1 if failed else 0
    else:
        config = TARGETS[args.target]
        return run_build(config, args.clean)


if __name__ == "__main__":
    sys.exit(main())
