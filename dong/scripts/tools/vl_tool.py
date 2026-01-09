import base64
import requests
import sys
import os

# 读取图片并编码
image_path = sys.argv[1]
with open(image_path, "rb") as image_file:
    base64_image = base64.b64encode(image_file.read()).decode('utf-8')

# 调用 API (这里以 OpenAI 为例)
headers = {
    "Content-Type": "application/json",
    "Authorization": f"Bearer {os.environ['OPENAI_API_KEY']}"
}
payload = {
    "model": "gpt-4o",
    "messages": [
        {
            "role": "user",
            "content": [
                {"type": "text", "text": "这是我的 UI 框架渲染出的图片。请详细检查：1. 布局是否异常？2. 颜色或图形是否有伪影？3. 简要描述你看到的内容。"},
                {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{base64_image}"}}
            ]
        }
    ]
}
response = requests.post("https://api.openai.com/v1/chat/completions", headers=headers, json=payload)
print(response.json()['choices'][0]['message']['content'])