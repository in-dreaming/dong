#!/usr/bin/env python3
"""
字体渲染优化验证脚本

使用方法:
1. 先构建项目: zig build examples
2. 运行此脚本: python scripts/tools/verify_font_optimization.py

此脚本会:
1. 使用html_render_test渲染中文字体测试HTML
2. 使用Playwright渲染浏览器基准
3. 对比两者的渲染结果
"""

import subprocess
import sys
import os
from pathlib import Path
import json

# 测试配置
TEST_CASES = [
    {
        "name": "chinese_font_tiny",
        "html": "examples/data/tests/chinese_font_tiny_test.html",
        "desc": "超小字体测试 (9px-12px)"
    },
    {
        "name": "chinese_font_small", 
        "html": "examples/data/tests/chinese_font_small_test.html",
        "desc": "小字体测试 (13px-16px)"
    },
    {
        "name": "chinese_font_sizes",
        "html": "examples/data/tests/chinese_font_sizes_test.html", 
        "desc": "全尺寸字体测试 (9px-72px)"
    },
    {
        "name": "chinese_font_large",
        "html": "examples/data/tests/chinese_font_large_test.html",
        "desc": "大字体测试 (28px-72px)"
    }
]

OUTPUT_DIR = Path("tmp/font_optimization_test")


def run_command(cmd, cwd=None):
    """运行命令并返回结果"""
    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=cwd)
        if result.returncode != 0:
            print(f"Error: {result.stderr}")
            return False
        return True
    except Exception as e:
        print(f"Exception: {e}")
        return False


def render_with_dong(html_path, output_path, width=800, height=600):
    """使用dong引擎渲染HTML"""
    # 构建html_render_test路径
    render_test_exe = Path("zig-out/bin/html_render_test.exe")
    if not render_test_exe.exists():
        # 尝试其他可能的路径
        render_test_exe = Path("build-cmake/html_render_test.exe")
    
    if not render_test_exe.exists():
        print(f"Error: html_render_test not found at {render_test_exe}")
        print("请先构建项目: zig build examples")
        return False
    
    cmd = [
        str(render_test_exe),
        str(html_path),
        str(output_path),
        str(width),
        str(height)
    ]
    
    return run_command(cmd, cwd=".")


def render_baseline(html_path, output_path, width=800, height=600):
    """使用Playwright渲染浏览器基准"""
    baseline_script = Path("scripts/tools/html_baseline_render.py")
    if not baseline_script.exists():
        print(f"Error: {baseline_script} not found")
        return False
    
    cmd = [
        sys.executable,
        str(baseline_script),
        str(html_path),
        "--out", str(output_path),
        "--width", str(width),
        "--height", str(height),
        "--wait-ms", "500"
    ]
    
    return run_command(cmd, cwd=".")


def compare_images(dong_path, baseline_path, output_path):
    """对比两张图片并生成差异图"""
    try:
        from PIL import Image, ImageDraw, ImageFont
        import numpy as np
        
        img1 = Image.open(dong_path).convert('RGB')
        img2 = Image.open(baseline_path).convert('RGB')
        
        # 确保尺寸一致
        if img1.size != img2.size:
            img2 = img2.resize(img1.size, Image.Resampling.LANCZOS)
        
        # 计算差异
        arr1 = np.array(img1).astype(float)
        arr2 = np.array(img2).astype(float)
        diff = np.abs(arr1 - arr2)
        
        # 增强差异以便观察
        diff_enhanced = np.clip(diff * 3, 0, 255).astype(np.uint8)
        diff_img = Image.fromarray(diff_enhanced)
        
        # 创建对比图
        width = img1.width * 3
        height = img1.height
        comparison = Image.new('RGB', (width, height))
        
        comparison.paste(img1, (0, 0))
        comparison.paste(img2, (img1.width, 0))
        comparison.paste(diff_img, (img1.width * 2, 0))
        
        # 添加标签
        draw = ImageDraw.Draw(comparison)
        try:
            font = ImageFont.truetype("arial.ttf", 20)
        except:
            font = ImageFont.load_default()
        
        draw.text((10, 10), "Dong Engine", fill=(255, 0, 0), font=font)
        draw.text((img1.width + 10, 10), "Browser (Baseline)", fill=(0, 255, 0), font=font)
        draw.text((img1.width * 2 + 10, 10), "Difference", fill=(0, 0, 255), font=font)
        
        comparison.save(output_path)
        print(f"Comparison saved to: {output_path}")
        
        # 计算差异统计
        diff_mean = np.mean(diff)
        diff_max = np.max(diff)
        print(f"  Mean difference: {diff_mean:.2f}")
        print(f"  Max difference: {diff_max:.2f}")
        
        return True
    except ImportError:
        print("  PIL or numpy not available, skipping comparison")
        return False
    except Exception as e:
        print(f"  Error comparing images: {e}")
        return False


def main():
    """主函数"""
    print("=" * 60)
    print("字体渲染优化验证")
    print("=" * 60)
    
    # 创建输出目录
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    # 检查依赖
    try:
        from PIL import Image
        import numpy as np
        print("Dependencies: PIL and numpy available")
    except ImportError:
        print("Warning: PIL or numpy not available, comparison features disabled")
        print("Install with: pip install pillow numpy")
    
    results = []
    
    for test in TEST_CASES:
        print(f"\n{'='*60}")
        print(f"Testing: {test['desc']}")
        print(f"HTML: {test['html']}")
        
        html_path = Path(test['html'])
        if not html_path.exists():
            print(f"Error: HTML file not found: {html_path}")
            continue
        
        # 定义输出路径
        dong_output = OUTPUT_DIR / f"{test['name']}_dong.bmp"
        baseline_output = OUTPUT_DIR / f"{test['name']}_baseline.png"
        comparison_output = OUTPUT_DIR / f"{test['name']}_compare.png"
        
        # 使用dong渲染
        print("\n[1/3] Rendering with Dong engine...")
        if render_with_dong(html_path, dong_output):
            print(f"  Saved: {dong_output}")
        else:
            print("  Failed!")
            results.append({"name": test['name'], "status": "dong_failed"})
            continue
        
        # 渲染浏览器基准
        print("\n[2/3] Rendering browser baseline...")
        if render_baseline(html_path, baseline_output):
            print(f"  Saved: {baseline_output}")
        else:
            print("  Failed to render baseline (Playwright may not be installed)")
            results.append({"name": test['name'], "status": "baseline_failed"})
            continue
        
        # 对比
        print("\n[3/3] Comparing results...")
        if compare_images(dong_output, baseline_output, comparison_output):
            results.append({"name": test['name'], "status": "success"})
        else:
            results.append({"name": test['name'], "status": "compare_skipped"})
    
    # 总结
    print(f"\n{'='*60}")
    print("Summary")
    print(f"{'='*60}")
    
    success_count = sum(1 for r in results if r['status'] == 'success')
    print(f"Completed: {success_count}/{len(TEST_CASES)} tests")
    
    for r in results:
        status_icon = {
            'success': '✓',
            'compare_skipped': '~',
            'dong_failed': '✗',
            'baseline_failed': '✗'
        }.get(r['status'], '?')
        print(f"  {status_icon} {r['name']}: {r['status']}")
    
    print(f"\nOutput directory: {OUTPUT_DIR.absolute()}")
    print("\n优化要点:")
    print("1. 小字体(9px-14px)现在使用48px MSDF纹理 + 4px range")
    print("2. 增加了128px和192px tiers用于大字体")
    print("3. 优化了tier选择算法，确保每个字号使用最适合的分辨率")
    print("4. 改进了着色器抗锯齿算法，大字体边缘更平滑")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
