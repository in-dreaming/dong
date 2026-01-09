import base64
import requests
import sys
import json
import re

# --- 配置 ---
# 假设你的大模型 API 密钥存储在环境变量中
# 例如: export OPENAI_API_KEY='your-api-key'
# 或者你可以直接在这里硬编码，但不推荐
# API_KEY = os.environ.get("YOUR_LLM_API_KEY")
# MODEL_API_ENDPOINT = "YOUR_LLM_API_ENDPOINT" # e.g., "https://api.openai.com/v1/chat/completions"
# MODEL_NAME = "gpt-4o" # Or "claude-3-opus-20240229", etc.

# --- 示例配置 (使用 OpenAI API) ---
API_KEY = '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'#os.environ.get("OPENAI_API_KEY")
MODEL_API_ENDPOINT = "https://openrouter.ai/api/v1/chat/completions"
MODEL_NAME = "x-ai/grok-4.1-fast"
# --- 结束示例配置 ---


def get_image_mime_type(image_path):
    """根据文件扩展名返回正确的 MIME 类型"""
    ext = image_path.lower().split('.')[-1]
    mime_types = {
        'png': 'image/png',
        'jpg': 'image/jpeg',
        'jpeg': 'image/jpeg',
        'gif': 'image/gif',
        'webp': 'image/webp'
    }
    return mime_types.get(ext, 'image/png')  # 默认使用 PNG

def encode_image_to_base64(image_path):
    """将本地图片文件编码为 Base64 字符串，返回 (base64_data, mime_type)"""
    try:
        with open(image_path, "rb") as image_file:
            base64_data = base64.b64encode(image_file.read()).decode('utf-8')
            mime_type = get_image_mime_type(image_path)
            return base64_data, mime_type
    except FileNotFoundError:
        print(f"Error: Image file not found at {image_path}", file=sys.stderr)
        return None, None
    except Exception as e:
        print(f"Error encoding image {image_path}: {e}", file=sys.stderr)
        return None, None

def analyze_rendering_with_llm(base64_image_data, mime_type, html_string, debug=False):
    """
    调用大模型 API 分析图片和 HTML，并返回分级结果。
    """
    if not API_KEY:
        return "Error: LLM API key is not configured."

    headers = {
        "Content-Type": "application/json",
        "Authorization": f"Bearer {API_KEY}"
    }

    # --- Prompt 工程 ---
    # 指导模型进行分析和分级
    system_prompt = """
您是一位专业的 web UI 渲染分析助手。
您将收到一个代表 UI 结构的 HTML 字符串和一个 Base64 编码的渲染输出图像。
您的任务是通过比较 HTML 结构和图像中的视觉输出，分析潜在的渲染异常。

请分析以下内容：
1. **布局差异：**视觉输出是否与 HTML 中定义的布局匹配（例如，元素位置、大小、间距、溢出）？
2. **样式不匹配：**颜色、字体、边框或其他样式是否按照常见的 Web 标准或 HTML 结构所隐含的方式正确渲染？
3. **渲染瑕疵：**是否存在任何视觉故障、锯齿、形状渲染不正确或元素显示不完整的情况？
4. **内容渲染：**文本内容是否在其边界内正确显示？

根据您的分析，请提供一份简洁的摘要，说明检测到的所有渲染异常。
将每个异常分为以下三个严重级别之一：
- **严重：**导致 UI 无法使用或无法正确显示的主要功能或视觉故障。
- **警告：**显著的视觉问题，会降低用户体验或外观，但不会破坏核心功能。
- **信息：**轻微的视觉不一致或改进建议，不太可能影响可用性，但可以进行改进。

请以以下结构的 JSON 字符串格式输出您的分析结果：
{
    "overall_severity": "CRITICAL" | "WARNING" | "INFO" | "NONE",
    "anomalies": [
    {
        "severity": "CRITICAL" | "WARNING" | "INFO",
        "description": "Detailed description of the anomaly."
    }
    // ... more anomalies
    ]
}

**重要：** 只有在经过仔细、全面的分析后，确认图片与 HTML 完全一致且没有任何问题时，才返回 NONE。
如果发现任何差异、不一致或潜在问题，即使很轻微，也应该记录在 anomalies 中。
    """

    payload = {
        "model": MODEL_NAME,
        "messages": [
            {
                "role": "system",
                "content": system_prompt
            },
            {
                "role": "user",
                "content": [
                    {
                        "type": "text",
                        "text": """请仔细分析提供的图片和 HTML 代码。

重要提示：
1. 图片是 HTML 页面的实际渲染截图
2. 请对比图片中的视觉效果与 HTML 代码中定义的样式
3. 必须仔细检查以下方面：
   - 元素的位置、大小、间距是否正确
   - 颜色、字体、边框是否按 HTML/CSS 定义渲染
   - 是否有元素缺失、重叠、错位
   - 文本内容是否正确显示
   - 布局是否符合 HTML 结构

请务必详细分析，不要轻易返回 NONE。如果发现任何不一致，即使是轻微的，也应该报告。"""
                    },
                    {
                        "type": "image_url",
                        "image_url": {
                            "url": f"data:{mime_type};base64,{base64_image_data}"
                        }
                    },
                    {
                        "type": "text",
                        "text": f"HTML String:\n```html\n{html_string}\n```"
                    }
                ]
            }
        ],
        "max_tokens": 1000 # Adjust as needed
    }

    try:
        if debug:
            # 检查 payload 大小
            import json as json_module
            payload_str = json_module.dumps(payload)
            payload_size = len(payload_str.encode('utf-8'))
            image_data_size = len(base64_image_data)
            print(f"[DEBUG] Payload size: {payload_size / 1024:.2f} KB", file=sys.stderr)
            print(f"[DEBUG] Image base64 size: {image_data_size / 1024:.2f} KB", file=sys.stderr)
            print(f"[DEBUG] MIME type: {mime_type}", file=sys.stderr)
            print(f"[DEBUG] Model: {MODEL_NAME}", file=sys.stderr)
            print(f"[DEBUG] HTML length: {len(html_string)} chars", file=sys.stderr)
        
        response = requests.post(MODEL_API_ENDPOINT, headers=headers, json=payload)
        response.raise_for_status() # Raise an exception for bad status codes
        result_json = response.json()
        
        if debug:
            # 检查 API 响应结构
            print(f"[DEBUG] API Response keys: {list(result_json.keys())}", file=sys.stderr)
            if 'choices' in result_json and len(result_json['choices']) > 0:
                choice = result_json['choices'][0]
                print(f"[DEBUG] Choice keys: {list(choice.keys())}", file=sys.stderr)
                if 'message' in choice:
                    print(f"[DEBUG] Message keys: {list(choice['message'].keys())}", file=sys.stderr)
                    # 检查是否有 finish_reason
                    if 'finish_reason' in choice:
                        print(f"[DEBUG] Finish reason: {choice['finish_reason']}", file=sys.stderr)

        # 提取模型生成的 JSON 结果
        # 注意：具体提取路径可能因模型 API 不同而异
        # 对于 OpenAI, 通常在 choices[0].message.content
        analysis_content = result_json['choices'][0]['message']['content']
        
        # 调试输出：打印原始响应
        if debug:
            print(f"[DEBUG] Raw LLM response:\n{analysis_content}\n", file=sys.stderr)

        # 尝试从 markdown 代码块中提取 JSON（如果存在）
        json_match = re.search(r'```(?:json)?\s*(\{.*?\})\s*```', analysis_content, re.DOTALL)
        if json_match:
            analysis_content = json_match.group(1)
        else:
            # 尝试直接查找 JSON 对象
            json_match = re.search(r'\{.*\}', analysis_content, re.DOTALL)
            if json_match:
                analysis_content = json_match.group(0)

        # 尝试解析 JSON
        try:
            analysis_data = json.loads(analysis_content)
            
            # 如果结果是 NONE，检查是否真的没有异常，还是模型没有正确分析
            if debug and analysis_data.get('overall_severity') == 'NONE' and len(analysis_data.get('anomalies', [])) == 0:
                print("[DEBUG] Warning: Model returned NONE. This might indicate:", file=sys.stderr)
                print("  1. Model cannot see/process the image", file=sys.stderr)
                print("  2. Model doesn't support vision capabilities", file=sys.stderr)
                print("  3. Image format is incorrect", file=sys.stderr)
                print(f"  4. Using MIME type: {mime_type}", file=sys.stderr)
            
            return json.dumps(analysis_data, indent=2, ensure_ascii=False) # Return formatted JSON string
        except json.JSONDecodeError:
            # 如果模型没有返回有效的 JSON，返回原始内容并注明
            return f"LLM returned non-JSON output. Raw output:\n---\n{analysis_content}\n---"

    except requests.exceptions.RequestException as e:
        return f"Error calling LLM API: {e}"
    except KeyError as e:
        return f"Error parsing LLM API response (KeyError: {e}). Response: {result_json}"
    except Exception as e:
        return f"An unexpected error occurred: {e}"

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python vision_analysis_proxy.py <path_to_image> <path_to_html_file>")
        sys.exit(1)

    image_path = sys.argv[1]
    html_file_path = sys.argv[2]

    # 1. 读取 HTML 文件
    try:
        with open(html_file_path, 'r', encoding='utf-8') as f:
            html_content = f.read()
    except FileNotFoundError:
        print(f"Error: HTML file not found at {html_file_path}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error reading HTML file {html_file_path}: {e}", file=sys.stderr)
        sys.exit(1)

    # 2. 编码图片为 Base64
    base64_image_data, mime_type = encode_image_to_base64(image_path)
    if not base64_image_data:
        sys.exit(1) # 编码失败，已打印错误

    # 3. 调用分析函数（启用调试模式）
    analysis_result = analyze_rendering_with_llm(base64_image_data, mime_type, html_content, debug=True)

    # 4. 输出分析结果
    print(analysis_result)