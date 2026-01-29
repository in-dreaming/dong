#!/usr/bin/env python3
"""
字体渲染自动化对比测试工具

功能：
1. 使用 dong 引擎离屏渲染 HTML 到 BMP
2. 使用 Playwright (Chromium) 渲染同一份 HTML 作为基准
3. 对比两张图片，生成差异报告
4. 支持批量测试多个 HTML 文件

使用方法:
    cd dong
    python scripts/tools/auto_font_render_compare.py [options]

示例:
    # 测试所有字体测试用例
    python scripts/tools/auto_font_render_compare.py --pattern "chinese_font*"
    
    # 测试特定文件
    python scripts/tools/auto_font_render_compare.py --case chinese_font_sizes_test
    
    # 指定输出目录
    python scripts/tools/auto_font_render_compare.py --out tmp/font_compare
"""

import subprocess
import sys
import os
import argparse
import json
import tempfile
from pathlib import Path
from datetime import datetime
from dataclasses import dataclass, asdict
from typing import List, Optional, Tuple
import shutil

# 测试配置
DEFAULT_TEST_CASES = [
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

# 渲染尺寸
RENDER_WIDTH = 800
RENDER_HEIGHT = 600


@dataclass
class CompareResult:
    """对比结果"""
    name: str
    html_path: str
    status: str  # 'success', 'dong_failed', 'baseline_failed', 'compare_failed'
    dong_output: Optional[str] = None
    baseline_output: Optional[str] = None
    comparison_output: Optional[str] = None
    diff_mean: float = 0.0
    diff_max: float = 0.0
    diff_percent: float = 0.0
    error_msg: str = ""


def find_html_render_test() -> Optional[Path]:
    """查找 html_render_test 可执行文件"""
    possible_paths = [
        Path("zig-out/bin/html_render_test.exe"),
        Path("build-cmake/html_render_test.exe"),
        Path("zig-out/bin/html_render_test"),
        Path("build-cmake/html_render_test"),
    ]
    
    for path in possible_paths:
        if path.exists():
            return path
    
    return None


def render_with_dong_offscreen(
    html_path: Path,
    output_path: Path,
    width: int = RENDER_WIDTH,
    height: int = RENDER_HEIGHT
) -> bool:
    """
    使用 dong 引擎离屏渲染 HTML
    
    使用 html_render_test 工具的离屏渲染模式
    """
    render_exe = find_html_render_test()
    if not render_exe:
        print(f"Error: html_render_test not found")
        print("请先构建项目: zig build examples")
        return False
    
    cmd = [
        str(render_exe),
        str(html_path),
        str(output_path),
        str(width),
        str(height)
    ]
    
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            cwd=".",
            timeout=30
        )
        if result.returncode != 0:
            print(f"Dong render error: {result.stderr}")
            return False
        return output_path.exists()
    except subprocess.TimeoutExpired:
        print("Dong render timeout")
        return False
    except Exception as e:
        print(f"Dong render exception: {e}")
        return False


def render_with_playwright(
    html_path: Path,
    output_path: Path,
    width: int = RENDER_WIDTH,
    height: int = RENDER_HEIGHT
) -> bool:
    """使用 Playwright 渲染浏览器基准"""
    try:
        from playwright.sync_api import sync_playwright
        
        with sync_playwright() as p:
            browser = p.chromium.launch()
            page = browser.new_page(viewport={
                'width': width,
                'height': height,
                'deviceScaleFactor': 1.0
            })
            
            # 加载 HTML 文件
            html_absolute = html_path.absolute().as_uri()
            page.goto(html_absolute, wait_until='networkidle')
            
            # 等待字体加载和渲染完成
            page.wait_for_timeout(1000)
            
            # 截图
            page.screenshot(path=str(output_path), full_page=False)
            browser.close()
            
            return output_path.exists()
            
    except ImportError:
        print("Error: Playwright not installed")
        print("Install with: pip install playwright")
        print("Then: playwright install chromium")
        return False
    except Exception as e:
        print(f"Playwright render error: {e}")
        return False


def compare_images(
    dong_path: Path,
    baseline_path: Path,
    output_path: Path
) -> Tuple[bool, float, float, float]:
    """
    对比两张图片
    
    返回: (success, diff_mean, diff_max, diff_percent)
    """
    try:
        from PIL import Image, ImageDraw, ImageFont
        import numpy as np
        
        # 加载图片
        img1 = Image.open(dong_path).convert('RGB')
        img2 = Image.open(baseline_path).convert('RGB')
        
        # 确保尺寸一致
        if img1.size != img2.size:
            img2 = img2.resize(img1.size, Image.Resampling.LANCZOS)
        
        # 计算像素差异
        arr1 = np.array(img1).astype(float)
        arr2 = np.array(img2).astype(float)
        diff = np.abs(arr1 - arr2)
        
        # 计算统计值
        diff_mean = np.mean(diff)
        diff_max = np.max(diff)
        
        # 计算差异像素百分比（阈值=10）
        threshold = 10
        diff_pixels = np.sum(diff > threshold)
        total_pixels = diff.shape[0] * diff.shape[1] * diff.shape[2]
        diff_percent = 100.0 * diff_pixels / total_pixels
        
        # 创建差异可视化
        diff_enhanced = np.clip(diff * 4, 0, 255).astype(np.uint8)
        diff_img = Image.fromarray(diff_enhanced)
        
        # 创建三列对比图
        total_width = img1.width * 3 + 20  # 20px for separators
        total_height = img1.height + 40    # 40px for labels
        comparison = Image.new('RGB', (total_width, total_height), (240, 240, 240))
        
        # 粘贴图片
        comparison.paste(img1, (0, 40))
        comparison.paste(img2, (img1.width + 10, 40))
        comparison.paste(diff_img, (img1.width * 2 + 20, 40))
        
        # 添加标签和统计信息
        draw = ImageDraw.Draw(comparison)
        try:
            font = ImageFont.truetype("arial.ttf", 14)
            font_small = ImageFont.truetype("arial.ttf", 12)
        except:
            font = ImageFont.load_default()
            font_small = font
        
        # 列标签
        draw.text((10, 10), f"Dong Engine", fill=(0, 0, 0), font=font)
        draw.text((img1.width + 20, 10), f"Browser Baseline", fill=(0, 0, 0), font=font)
        draw.text((img1.width * 2 + 30, 10), f"Difference", fill=(0, 0, 0), font=font)
        
        # 统计信息
        stats_text = f"Mean: {diff_mean:.1f} | Max: {diff_max:.1f} | Diff%: {diff_percent:.2f}%"
        draw.text((10, total_height - 20), stats_text, fill=(100, 100, 100), font=font_small)
        
        comparison.save(output_path, quality=95)
        print(f"  Comparison saved: {output_path}")
        print(f"  Stats: {stats_text}")
        
        return True, diff_mean, diff_max, diff_percent
        
    except ImportError:
        print("  Warning: PIL/numpy not available, copying files only")
        shutil.copy(dong_path, output_path.parent / f"{dong_path.stem}_dong{dong_path.suffix}")
        shutil.copy(baseline_path, output_path.parent / f"{baseline_path.stem}_baseline{baseline_path.suffix}")
        return True, 0.0, 0.0, 0.0
    except Exception as e:
        print(f"  Compare error: {e}")
        return False, 0.0, 0.0, 0.0


def run_single_test(
    test_case: dict,
    output_dir: Path,
    width: int = RENDER_WIDTH,
    height: int = RENDER_HEIGHT
) -> CompareResult:
    """运行单个测试用例"""
    name = test_case["name"]
    html_path = Path(test_case["html"])
    
    result = CompareResult(
        name=name,
        html_path=str(html_path),
        status="pending"
    )
    
    print(f"\n{'='*60}")
    print(f"Testing: {test_case['desc']}")
    print(f"HTML: {html_path}")
    
    if not html_path.exists():
        result.status = "dong_failed"
        result.error_msg = f"HTML file not found: {html_path}"
        return result
    
    # 定义输出路径
    case_dir = output_dir / name
    case_dir.mkdir(parents=True, exist_ok=True)
    
    dong_output = case_dir / "dong_render.bmp"
    baseline_output = case_dir / "browser_baseline.png"
    comparison_output = case_dir / "comparison.png"
    
    # 步骤1: 使用 dong 离屏渲染
    print("\n[1/3] Rendering with Dong engine (offscreen)...")
    if render_with_dong_offscreen(html_path, dong_output, width, height):
        print(f"  ✓ Dong render: {dong_output}")
        result.dong_output = str(dong_output)
    else:
        print("  ✗ Dong render failed")
        result.status = "dong_failed"
        result.error_msg = "Dong rendering failed"
        return result
    
    # 步骤2: 使用 Playwright 渲染浏览器基准
    print("\n[2/3] Rendering browser baseline...")
    if render_with_playwright(html_path, baseline_output, width, height):
        print(f"  ✓ Baseline: {baseline_output}")
        result.baseline_output = str(baseline_output)
    else:
        print("  ✗ Baseline render failed (Playwright may not be installed)")
        result.status = "baseline_failed"
        result.error_msg = "Baseline rendering failed"
        return result
    
    # 步骤3: 对比
    print("\n[3/3] Comparing images...")
    success, diff_mean, diff_max, diff_percent = compare_images(
        dong_output, baseline_output, comparison_output
    )
    
    if success:
        result.status = "success"
        result.comparison_output = str(comparison_output)
        result.diff_mean = diff_mean
        result.diff_max = diff_max
        result.diff_percent = diff_percent
    else:
        result.status = "compare_failed"
        result.error_msg = "Image comparison failed"
    
    return result


def generate_html_report(results: List[CompareResult], output_dir: Path):
    """生成 HTML 报告"""
    report_path = output_dir / "report.html"
    
    html_content = """<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Font Rendering Comparison Report</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            margin: 0;
            padding: 20px;
            background: #f5f5f5;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            border-radius: 12px;
            margin-bottom: 30px;
        }
        .header h1 {
            margin: 0 0 10px 0;
            font-size: 28px;
        }
        .header .timestamp {
            opacity: 0.8;
            font-size: 14px;
        }
        .summary {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 15px;
            margin-bottom: 30px;
        }
        .stat-card {
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            text-align: center;
        }
        .stat-card .value {
            font-size: 32px;
            font-weight: bold;
            color: #667eea;
        }
        .stat-card .label {
            color: #666;
            margin-top: 5px;
            font-size: 14px;
        }
        .test-case {
            background: white;
            border-radius: 12px;
            margin-bottom: 20px;
            overflow: hidden;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        .test-header {
            padding: 20px;
            border-bottom: 1px solid #eee;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .test-header h3 {
            margin: 0;
            font-size: 18px;
        }
        .status-badge {
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: bold;
        }
        .status-success {
            background: #d4edda;
            color: #155724;
        }
        .status-failed {
            background: #f8d7da;
            color: #721c24;
        }
        .test-content {
            padding: 20px;
        }
        .comparison-image {
            width: 100%;
            border-radius: 8px;
            border: 1px solid #ddd;
        }
        .stats-row {
            display: flex;
            gap: 20px;
            margin-top: 15px;
            padding-top: 15px;
            border-top: 1px solid #eee;
        }
        .stats-row .stat {
            font-size: 14px;
            color: #666;
        }
        .stats-row .stat strong {
            color: #333;
        }
        .error-msg {
            background: #f8d7da;
            color: #721c24;
            padding: 15px;
            border-radius: 8px;
            margin-top: 10px;
        }
    </style>
</head>
<body>
"""
    
    # Header
    html_content += f"""
    <div class="header">
        <h1>📝 Font Rendering Comparison Report</h1>
        <div class="timestamp">Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</div>
    </div>
"""
    
    # Summary stats
    total = len(results)
    success = sum(1 for r in results if r.status == 'success')
    failed = total - success
    avg_diff = sum(r.diff_percent for r in results if r.status == 'success') / max(success, 1)
    
    html_content += f"""
    <div class="summary">
        <div class="stat-card">
            <div class="value">{total}</div>
            <div class="label">Total Tests</div>
        </div>
        <div class="stat-card">
            <div class="value" style="color: #28a745;">{success}</div>
            <div class="label">Passed</div>
        </div>
        <div class="stat-card">
            <div class="value" style="color: #dc3545;">{failed}</div>
            <div class="label">Failed</div>
        </div>
        <div class="stat-card">
            <div class="value">{avg_diff:.2f}%</div>
            <div class="label">Avg Diff</div>
        </div>
    </div>
"""
    
    # Test cases
    for result in results:
        status_class = "status-success" if result.status == 'success' else "status-failed"
        
        html_content += f"""
    <div class="test-case">
        <div class="test-header">
            <h3>{result.name}</h3>
            <span class="status-badge {status_class}">{result.status}</span>
        </div>
        <div class="test-content">
"""
        
        if result.status == 'success' and result.comparison_output:
            # 使用相对路径
            comp_rel = os.path.relpath(result.comparison_output, output_dir)
            html_content += f"""
            <img class="comparison-image" src="{comp_rel}" alt="Comparison">
            <div class="stats-row">
                <div class="stat"><strong>Mean Diff:</strong> {result.diff_mean:.2f}</div>
                <div class="stat"><strong>Max Diff:</strong> {result.diff_max:.2f}</div>
                <div class="stat"><strong>Diff Pixels:</strong> {result.diff_percent:.2f}%</div>
            </div>
"""
        elif result.error_msg:
            html_content += f"""
            <div class="error-msg">{result.error_msg}</div>
"""
        
        html_content += """
        </div>
    </div>
"""
    
    html_content += """
</body>
</html>
"""
    
    report_path.write_text(html_content, encoding='utf-8')
    print(f"\nHTML Report: {report_path}")


def main():
    parser = argparse.ArgumentParser(
        description="字体渲染自动化对比测试工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  %(prog)s                          # 运行所有默认测试
  %(prog)s --pattern "chinese_font*" # 匹配特定模式
  %(prog)s --case chinese_font_sizes # 测试单个用例
  %(prog)s --out tmp/my_compare      # 指定输出目录
        """
    )
    
    parser.add_argument(
        '--pattern',
        help='测试文件名匹配模式 (例如: chinese_font*)'
    )
    parser.add_argument(
        '--case',
        help='测试单个用例名称'
    )
    parser.add_argument(
        '--out',
        default='tmp/font_compare',
        help='输出目录 (默认: tmp/font_compare)'
    )
    parser.add_argument(
        '--width',
        type=int,
        default=RENDER_WIDTH,
        help=f'渲染宽度 (默认: {RENDER_WIDTH})'
    )
    parser.add_argument(
        '--height',
        type=int,
        default=RENDER_HEIGHT,
        help=f'渲染高度 (默认: {RENDER_HEIGHT})'
    )
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("字体渲染自动化对比测试")
    print("=" * 70)
    
    # 确定测试用例
    if args.case:
        test_cases = [tc for tc in DEFAULT_TEST_CASES if tc['name'] == args.case]
        if not test_cases:
            print(f"Error: Test case '{args.case}' not found")
            return 1
    elif args.pattern:
        import fnmatch
        test_cases = [
            tc for tc in DEFAULT_TEST_CASES
            if fnmatch.fnmatch(tc['name'], args.pattern)
        ]
        if not test_cases:
            print(f"Error: No test cases match pattern '{args.pattern}'")
            return 1
    else:
        test_cases = DEFAULT_TEST_CASES
    
    # 创建输出目录
    output_dir = Path(args.out)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"\n输出目录: {output_dir.absolute()}")
    print(f"测试用例: {len(test_cases)} 个")
    
    # 检查依赖
    try:
        from PIL import Image
        import numpy as np
        print("✓ PIL 和 numpy 已安装")
    except ImportError:
        print("⚠ PIL 或 numpy 未安装，对比功能受限")
        print("  安装: pip install pillow numpy")
    
    try:
        from playwright.sync_api import sync_playwright
        print("✓ Playwright 已安装")
    except ImportError:
        print("⚠ Playwright 未安装，无法生成浏览器基准")
        print("  安装: pip install playwright")
        print("  然后: playwright install chromium")
    
    # 运行测试
    results: List[CompareResult] = []
    
    for i, test_case in enumerate(test_cases, 1):
        print(f"\n[{i}/{len(test_cases)}] ", end="")
        result = run_single_test(test_case, output_dir, args.width, args.height)
        results.append(result)
    
    # 生成报告
    print(f"\n\n{'='*70}")
    print("生成报告...")
    generate_html_report(results, output_dir)
    
    # 控制台总结
    print(f"\n{'='*70}")
    print("测试结果总结")
    print(f"{'='*70}")
    
    for r in results:
        icon = "✓" if r.status == "success" else "✗"
        diff_str = f" ({r.diff_percent:.2f}% diff)" if r.status == "success" else ""
        print(f"{icon} {r.name}: {r.status}{diff_str}")
    
    success_count = sum(1 for r in results if r.status == 'success')
    print(f"\n总计: {success_count}/{len(results)} 通过")
    
    return 0 if success_count == len(results) else 1


if __name__ == "__main__":
    sys.exit(main())
