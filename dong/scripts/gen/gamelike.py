#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
离线游戏UI资源下载器
自动下载 Font Awesome 6 的 CSS 与字体文件，并修改 HTML 引用为本地路径。
"""

import os
import re
import urllib.request
import urllib.parse
from pathlib import Path

# ---------- 配置 ----------
# 原始 Font Awesome CDN 地址（与之前生成的 HTML 中一致）
FONTAWESOME_CSS_URL = "https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css"
# 本地资源存放根目录
LOCAL_ROOT = "local_resources"
# Font Awesome 本地存放目录
FA_LOCAL_DIR = os.path.join(LOCAL_ROOT, "font-awesome")
FA_CSS_DIR = os.path.join(FA_LOCAL_DIR, "css")
FA_WEBFONTS_DIR = os.path.join(FA_LOCAL_DIR, "webfonts")

# 原始 HTML 内容（即之前生成的游戏 UI 代码）
ORIGINAL_HTML = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>游戏UI · 科幻风格面板</title>
    <!-- Font Awesome 6 (免费图标库, 用于技能/物品图标) -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
    <style>
        /* 重置 & 基础 — 暗黑游戏氛围 */
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', 'Orbitron', 'Rajdhani', system-ui, sans-serif;
        }

        body {
            min-height: 100vh;
            background: radial-gradient(circle at 30% 20%, #1d2f3f, #0b0e17);
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 16px;
            /* 模拟微弱扫描线/网格，增加游戏质感 */
            background-image: 
                linear-gradient(rgba(0, 255, 255, 0.02) 1px, transparent 1px),
                linear-gradient(90deg, rgba(0, 255, 255, 0.02) 1px, transparent 1px);
            background-size: 40px 40px;
        }

        /* 主面板 — 半透明钢化玻璃 + 霓虹边框 */
        .game-panel {
            max-width: 1000px;
            width: 100%;
            background: rgba(8, 18, 28, 0.85);
            backdrop-filter: blur(4px); /* 轻微毛玻璃，现代游戏常用 */
            border: 2px solid #00ffff;
            border-radius: 32px;
            padding: 28px 24px;
            box-shadow: 
                0 0 30px rgba(0, 255, 255, 0.3),
                inset 0 0 20px rgba(0, 255, 255, 0.2);
            color: #e6f5ff;
            transition: all 0.2s;
            /* 装饰角线 — 像游戏界面里的 tactical 细节 */
            position: relative;
        }

        /* 四角装饰短线 (模仿游戏UI边角) */
        .game-panel::before {
            content: '';
            position: absolute;
            top: 10px; left: 10px;
            width: 30px; height: 30px;
            border-top: 3px solid #00ffff;
            border-left: 3px solid #00ffff;
            border-radius: 16px 0 0 0;
            filter: drop-shadow(0 0 6px cyan);
        }
        .game-panel::after {
            content: '';
            position: absolute;
            bottom: 10px; right: 10px;
            width: 30px; height: 30px;
            border-bottom: 3px solid #00ffff;
            border-right: 3px solid #00ffff;
            border-radius: 0 0 16px 0;
            filter: drop-shadow(0 0 6px cyan);
        }

        /* ----- 区块通用样式 (内嵌面板) ----- */
        .section-title {
            display: flex;
            align-items: baseline;
            gap: 8px;
            margin-bottom: 18px;
            border-bottom: 1px solid #00ffff;
            padding-bottom: 6px;
            text-transform: uppercase;
            letter-spacing: 2px;
            font-weight: 500;
            text-shadow: 0 0 8px cyan;
        }
        .section-title i {
            color: #00ffff;
            font-size: 1.2rem;
        }
        .section-title span {
            font-size: 0.9rem;
            color: #88ffff;
            margin-left: auto;
        }

        /* ----- 角色状态区 (头像 + 血蓝经验) ----- */
        .character-panel {
            display: flex;
            gap: 24px;
            flex-wrap: wrap;
            margin-bottom: 32px;
            background: rgba(0, 20, 30, 0.5);
            border-radius: 28px;
            padding: 20px 20px;
            border: 1px solid #00ffff88;
            box-shadow: inset 0 0 20px rgba(0, 255, 255, 0.1);
        }

        /* 头像 — 游戏风格圆形金属框 */
        .avatar {
            width: 110px;
            height: 110px;
            border-radius: 50%;
            background: linear-gradient(145deg, #1f4a5c, #0c1f2b);
            border: 3px solid #00ffff;
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 0 30px #00ffff66, inset 0 0 10px #00ffff;
            flex-shrink: 0;
        }
        .avatar i {
            font-size: 5rem;
            color: #b5f0ff;
            filter: drop-shadow(0 0 5px cyan);
        }

        /* 右侧数值区 */
        .stats {
            flex: 1;
            display: flex;
            flex-direction: column;
            justify-content: space-around;
            gap: 12px;
        }

        .name-level {
            display: flex;
            align-items: baseline;
            gap: 12px;
            flex-wrap: wrap;
        }
        .char-name {
            font-size: 2rem;
            font-weight: 600;
            text-transform: uppercase;
            background: linear-gradient(180deg, #fff, #aaffff);
            -webkit-background-clip: text;
            background-clip: text;
            color: transparent;
            letter-spacing: 2px;
            text-shadow: 0 0 10px cyan;
        }
        .char-level {
            background: #0a2630;
            padding: 4px 12px;
            border-radius: 40px;
            border: 1px solid cyan;
            font-size: 1.1rem;
            font-weight: bold;
            color: #aaffff;
        }
        .char-class {
            color: #99ccff;
            font-size: 1rem;
            letter-spacing: 1px;
            text-transform: uppercase;
            opacity: 0.9;
            margin-left: 4px;
        }

        /* 进度条样式 (HP/MP/EXP) */
        .stat-row {
            display: flex;
            align-items: center;
            gap: 10px;
            font-size: 0.95rem;
            width: 100%;
        }
        .stat-label {
            width: 45px;
            font-weight: 600;
            color: #c4ffff;
            text-shadow: 0 0 5px cyan;
        }
        .progress-bar {
            flex: 1;
            height: 18px;
            background: #102630;
            border-radius: 40px;
            border: 1px solid #00ccaa;
            overflow: hidden;
            position: relative;
            box-shadow: inset 0 2px 5px #00000080;
        }
        .fill {
            height: 100%;
            width: 0%; /* 由行内样式覆盖 */
            border-radius: 40px;
            box-shadow: 0 0 12px currentColor;
            transition: width 0.2s ease;
        }
        .hp-fill { background: linear-gradient(90deg, #ff4466, #ff8866); }
        .mp-fill { background: linear-gradient(90deg, #44aaff, #88ddff); }
        .exp-fill { background: linear-gradient(90deg, #ffcc44, #ffee88); }

        .stat-value {
            min-width: 65px;
            text-align: right;
            color: #ffffff;
            text-shadow: 0 0 6px #00ffff;
            font-weight: 500;
        }

        /* ----- 技能快捷栏 (像MMO底部技能) ----- */
        .skills-panel {
            margin-bottom: 32px;
        }

        .skills-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 18px;
            margin-top: 8px;
        }

        .skill-item {
            background: rgba(0, 30, 40, 0.7);
            border: 2px solid #0ff;
            border-radius: 20px;
            padding: 20px 8px 12px 8px;
            text-align: center;
            transition: 0.2s;
            box-shadow: 0 0 15px rgba(0, 255, 255, 0.2), inset 0 0 10px rgba(0,255,255,0.1);
            cursor: default;
            backdrop-filter: blur(2px);
        }
        .skill-item:hover {
            transform: scale(1.03);
            border-color: #ffcc00;
            box-shadow: 0 0 25px #ffcc00, inset 0 0 15px #ffffaa;
        }
        .skill-item i {
            font-size: 3rem;
            color: #ccf9ff;
            filter: drop-shadow(0 0 8px cyan);
            margin-bottom: 8px;
        }
        .hotkey {
            display: inline-block;
            background: #0a2028;
            border: 1px solid cyan;
            border-radius: 30px;
            padding: 4px 16px;
            font-weight: bold;
            color: #0ff;
            letter-spacing: 1px;
            font-size: 1.2rem;
            box-shadow: inset 0 1px 4px black;
        }

        /* ----- 背包/物品栏 + 货币行 ----- */
        .inventory-panel {
            margin-bottom: 8px;
        }

        .inventory-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 18px;
            margin: 15px 0 20px 0;
        }

        .item-slot {
            background: rgba(0, 20, 30, 0.8);
            border: 2px solid #0ff;
            border-radius: 20px;
            padding: 20px 5px;
            text-align: center;
            aspect-ratio: 1 / 1;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            transition: 0.2s;
            box-shadow: 0 0 10px cyan;
        }
        .item-slot:hover {
            border-color: #ffaa00;
            box-shadow: 0 0 20px #ffaa00;
            background: rgba(20, 40, 50, 0.9);
        }
        .item-slot i {
            font-size: 2.8rem;
            color: #d5f0ff;
            filter: drop-shadow(0 0 6px white);
        }
        .item-slot span {
            margin-top: 6px;
            font-size: 0.8rem;
            color: #aaddff;
            letter-spacing: 0.5px;
        }

        /* 货币条 (金币/宝石) */
        .currency-row {
            display: flex;
            align-items: center;
            gap: 25px;
            background: rgba(0, 255, 255, 0.08);
            border: 1px solid #0ff;
            border-radius: 60px;
            padding: 12px 25px;
            margin-top: 10px;
            flex-wrap: wrap;
            justify-content: flex-start;
            backdrop-filter: blur(2px);
        }
        .currency-item {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 1.4rem;
            font-weight: 600;
            color: #f0f9ff;
            text-shadow: 0 0 8px cyan;
        }
        .currency-item i {
            color: #f4d03f;
            filter: drop-shadow(0 0 5px gold);
        }
        .currency-item .divider {
            width: 2px;
            height: 28px;
            background: cyan;
            margin: 0 8px;
            opacity: 0.5;
        }
        .currency-item small {
            font-size: 1rem;
            color: #88ffff;
            margin-left: 5px;
        }

        /* 额外装饰: 右上角信号/电量风格(游戏感) */
        .system-tag {
            position: absolute;
            top: 20px;
            right: 30px;
            display: flex;
            gap: 8px;
            font-size: 0.8rem;
            color: #aaffff;
            letter-spacing: 2px;
            background: rgba(0, 20, 30, 0.7);
            padding: 4px 12px;
            border-radius: 30px;
            border: 1px solid cyan;
        }
        .system-tag i {
            color: cyan;
        }

        /* 响应式: 小屏幕调整 */
        @media (max-width: 700px) {
            .game-panel { padding: 20px 15px; }
            .character-panel { flex-direction: column; align-items: center; }
            .avatar { width: 90px; height: 90px; }
            .avatar i { font-size: 4rem; }
            .char-name { font-size: 1.8rem; }
            .skills-grid, .inventory-grid { gap: 10px; }
            .skill-item i { font-size: 2.2rem; }
            .item-slot i { font-size: 2rem; }
            .currency-item { font-size: 1.2rem; }
        }
    </style>
</head>
<body>
    <div class="game-panel">
        <!-- 顶部装饰信号 — 像游戏里的系统状态 -->
        <div class="system-tag">
            <i class="fas fa-wifi"></i> <span>ONLINE</span>
            <i class="fas fa-battery-full" style="margin-left: 8px;"></i> <span>100%</span>
        </div>

        <!-- ========= 角色状态区 ========= -->
        <div class="character-panel">
            <!-- 头像 (游戏icon) -->
            <div class="avatar">
                <i class="fas fa-user-astronaut"></i>
            </div>
            <!-- 状态数值区 -->
            <div class="stats">
                <div class="name-level">
                    <span class="char-name">Lancelot</span>
                    <span class="char-level">Lv.18</span>
                    <span class="char-class">· 人类战士</span>
                </div>

                <!-- 生命条 HP -->
                <div class="stat-row">
                    <span class="stat-label"><i class="fas fa-heart" style="color:#ff5577;"></i> HP</span>
                    <div class="progress-bar">
                        <div class="fill hp-fill" style="width: 76%;"></div>
                    </div>
                    <span class="stat-value">760 / 1000</span>
                </div>

                <!-- 法力条 MP -->
                <div class="stat-row">
                    <span class="stat-label"><i class="fas fa-bolt" style="color:#3d9eff;"></i> MP</span>
                    <div class="progress-bar">
                        <div class="fill mp-fill" style="width: 42%;"></div>
                    </div>
                    <span class="stat-value">210 / 500</span>
                </div>

                <!-- 经验条 EXP -->
                <div class="stat-row">
                    <span class="stat-label"><i class="fas fa-star" style="color:#ffcc44;"></i> EXP</span>
                    <div class="progress-bar">
                        <div class="fill exp-fill" style="width: 64%;"></div>
                    </div>
                    <span class="stat-value">3250 / 5000</span>
                </div>
            </div>
        </div>

        <!-- ========= 技能栏 (像游戏底部技能) ========= -->
        <div class="skills-panel">
            <div class="section-title">
                <i class="fas fa-bolt"></i>
                <h3>战斗技能</h3>
                <span>4 / 4  equipped</span>
            </div>
            <div class="skills-grid">
                <!-- 技能1  Q -->
                <div class="skill-item">
                    <i class="fas fa-fire"></i>
                    <div class="hotkey">Q</div>
                </div>
                <!-- 技能2  W -->
                <div class="skill-item">
                    <i class="fas fa-shield-halved"></i>
                    <div class="hotkey">W</div>
                </div>
                <!-- 技能3  E -->
                <div class="skill-item">
                    <i class="fas fa-wind"></i>
                    <div class="hotkey">E</div>
                </div>
                <!-- 技能4  R (终极) -->
                <div class="skill-item">
                    <i class="fas fa-skull"></i>
                    <div class="hotkey">R</div>
                </div>
            </div>
        </div>

        <!-- ========= 背包物品栏 ========= -->
        <div class="inventory-panel">
            <div class="section-title">
                <i class="fas fa-backpack"></i>
                <h3>随身背包</h3>
                <span>8 / 16  slots</span>
            </div>

            <!-- 物品格子 (4个展示) -->
            <div class="inventory-grid">
                <div class="item-slot">
                    <i class="fas fa-sword"></i>
                    <span>霜之哀伤</span>
                </div>
                <div class="item-slot">
                    <i class="fas fa-flask"></i>
                    <span>治疗药剂</span>
                </div>
                <div class="item-slot">
                    <i class="fas fa-gem"></i>
                    <span>魔力水晶</span>
                </div>
                <div class="item-slot">
                    <i class="fas fa-hat-wizard"></i>
                    <span>巫师帽</span>
                </div>
            </div>

            <!-- 货币行 (金币/钻石) 很游戏风格 -->
            <div class="currency-row">
                <div class="currency-item">
                    <i class="fas fa-coins"></i> 12,450 <small>gold</small>
                </div>
                <div class="currency-item">
                    <i class="fas fa-gem" style="color:#b9f2ff;"></i> 84 <small>shards</small>
                </div>
                <div class="currency-item">
                    <i class="fas fa-trophy" style="color:#ffd966;"></i> 荣誉 3560
                </div>
            </div>
        </div>

        <!-- 极简底部状态条 (模拟游戏HUD小提示) -->
        <div style="display: flex; justify-content: space-between; margin-top: 24px; border-top: 1px solid #0ff; padding-top: 15px; font-size: 0.85rem; color: #9cc8ff;">
            <span><i class="fas fa-map-pin" style="color: cyan;"></i> 诅咒林地 · 北境</span>
            <span><i class="fas fa-clock" style="color: cyan;"></i> 19:42  |  霜月</span>
            <span><i class="fas fa-users" style="color: cyan;"></i> 小队  (3/5)</span>
        </div>
    </div>
</body>
</html>
"""

def download_file(url, dest_path):
    """下载文件并保存到 dest_path，自动创建目录"""
    print(f"正在下载: {url}")
    try:
        # 创建父目录
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)
        urllib.request.urlretrieve(url, dest_path)
        print(f"已保存: {dest_path}")
    except Exception as e:
        print(f"下载失败 {url}: {e}")
        raise

def extract_font_urls(css_content, base_url):
    """
    从CSS内容中提取所有字体文件的URL（.woff2, .woff, .ttf, .eot）
    返回绝对URL列表
    """
    # 匹配 url(...) 中的路径，可能带引号
    pattern = r'url\([\'"]?(.*?)[\'"]?\)'
    urls = re.findall(pattern, css_content)
    font_extensions = ('.woff2', '.woff', '.ttf', '.eot')
    font_urls = []
    for u in urls:
        # 去除可能的查询参数（如 ?v=...），但保留路径
        clean_u = u.split('?')[0]
        if clean_u.lower().endswith(font_extensions):
            # 拼接绝对URL
            absolute_url = urllib.parse.urljoin(base_url, u)
            font_urls.append(absolute_url)
    return font_urls

def main():
    print("开始准备离线资源...")

    # 创建本地目录
    os.makedirs(FA_CSS_DIR, exist_ok=True)
    os.makedirs(FA_WEBFONTS_DIR, exist_ok=True)

    # 下载 CSS 文件到本地
    css_filename = os.path.basename(FONTAWESOME_CSS_URL)
    local_css_path = os.path.join(FA_CSS_DIR, css_filename)
    download_file(FONTAWESOME_CSS_URL, local_css_path)

    # 读取下载的CSS内容，提取字体URL
    with open(local_css_path, 'r', encoding='utf-8') as f:
        css_content = f.read()

    # CSS 的基础URL（用于解析相对路径）
    base_css_url = FONTAWESOME_CSS_URL[:FONTAWESOME_CSS_URL.rfind('/')+1]

    font_urls = extract_font_urls(css_content, base_css_url)
    print(f"发现 {len(font_urls)} 个字体文件。")

    # 下载每个字体文件到本地 webfonts 目录
    for font_url in font_urls:
        # 提取文件名（例如 fa-solid-900.woff2）
        font_filename = os.path.basename(font_url.split('?')[0])
        local_font_path = os.path.join(FA_WEBFONTS_DIR, font_filename)
        download_file(font_url, local_font_path)

    # 修改 HTML 中的 CSS 引用为本地路径
    local_css_ref = os.path.join(FA_LOCAL_DIR, "css", css_filename).replace("\\", "/")  # 确保web路径使用正斜杠
    modified_html = ORIGINAL_HTML.replace(
        'https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css',
        local_css_ref
    )

    # 写入新的 HTML 文件
    output_html = "game_ui_offline.html"
    with open(output_html, 'w', encoding='utf-8') as f:
        f.write(modified_html)
    print(f"\n✅ 离线 HTML 已生成: {output_html}")
    print("现在您可以在无网络环境下打开此文件，图标将正常显示。")

if __name__ == "__main__":
    main()