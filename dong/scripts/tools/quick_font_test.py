#!/usr/bin/env python3
"""
一键字体渲染测试工具

最简单的用法，无需参数：
    python scripts/tools/quick_font_test.py

会自动：
1. 找到 html_render_test 并渲染 dong 结果
2. 使用 Playwright 渲染浏览器基准（如果可用）
3. 生成对比图
"""

import sys
import os
from pathlib import Path

# 添加脚本目录到路径
sys.path.insert(0, str(Path(__file__).parent))

from auto_font_render_compare import (
    run_single_test,
    generate_html_report,
    DEFAULT_TEST_CASES,
    CompareResult
)


def main():
    print("=" * 70)
    print("一键字体渲染测试")
    print("=" * 70)
    
    # 默认使用 chinese_font_sizes_test（最全的测试）
    test_case = {
        "name": "chinese_font_sizes",
        "html": "examples/data/tests/chinese_font_sizes_test.html",
        "desc": "全尺寸字体测试 (9px-72px)"
    }
    
    # 如果指定了参数，使用指定的测试
    if len(sys.argv) > 1:
        test_name = sys.argv[1]
        # 移除 .html 后缀如果存在
        test_name = test_name.replace('.html', '')
        
        # 在默认列表中查找
        for tc in DEFAULT_TEST_CASES:
            if tc['name'] == test_name:
                test_case = tc
                break
        else:
            # 尝试作为文件路径
            html_path = Path(test_name)
            if html_path.exists():
                test_case = {
                    "name": html_path.stem,
                    "html": str(html_path),
                    "desc": f"Custom test: {html_path.name}"
                }
            else:
                # 尝试在 tests 目录中查找
                html_path = Path(f"examples/data/tests/{test_name}.html")
                if html_path.exists():
                    test_case = {
                        "name": test_name,
                        "html": str(html_path),
                        "desc": f"Test: {test_name}"
                    }
                else:
                    print(f"错误: 找不到测试文件 '{test_name}'")
                    print("可用测试:")
                    for tc in DEFAULT_TEST_CASES:
                        print(f"  - {tc['name']}")
                    return 1
    
    print(f"\n测试: {test_case['desc']}")
    print(f"文件: {test_case['html']}")
    
    # 创建输出目录
    from datetime import datetime
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_dir = Path(f"tmp/font_test/{test_case['name']}_{timestamp}")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"输出: {output_dir}")
    print()
    
    # 运行测试
    result = run_single_test(test_case, output_dir)
    
    # 生成简单报告
    print("\n" + "=" * 70)
    print("测试结果")
    print("=" * 70)
    
    if result.status == "success":
        print(f"✓ 测试通过")
        print(f"  差异均值: {result.diff_mean:.2f}")
        print(f"  差异最大值: {result.diff_max:.2f}")
        print(f"  差异像素比例: {result.diff_percent:.2f}%")
        print()
        print(f"对比图: {result.comparison_output}")
        
        # 尝试打开图片
        try:
            import platform
            comparison_path = Path(result.comparison_output)
            if comparison_path.exists():
                if platform.system() == "Windows":
                    os.startfile(comparison_path)
                elif platform.system() == "Darwin":
                    os.system(f'open "{comparison_path}"')
                else:
                    os.system(f'xdg-open "{comparison_path}"')
        except Exception as e:
            pass  # 忽略打开错误
            
        return 0
    else:
        print(f"✗ 测试失败: {result.status}")
        if result.error_msg:
            print(f"  错误: {result.error_msg}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
