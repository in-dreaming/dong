目标：
自动生成 HTML/CSS/资源测试用例，比较参考渲染（Chrome/Headless 浏览器）与自研渲染引擎的渲染结果；若不一致则自动最小化重现、生成可操作的 bug 报告与修复建议，尝试可选的自动修复（若启用），验证修复，生成回归测试，然后继续生成下一个用例。循环直到覆盖预定义的所有 HTML/CSS 特性或满足停止策略。

假定能力（vibing 应有或可调用）：
- 能生成任意 HTML/CSS/JS/资源文件并提供 URL 或本地路径。
- 能在 headless Chrome/Chromium 中加载并截图（参考渲染），并导出 DOM 栈、computed styles、布局信息（layout tree）与字体/资源加载日志。
- 能把同一 HTML 输入给自研渲染引擎并获取渲染位图（PNG）、渲染树或等价调试信息。
- 能运行图像比较（像素差、SSIM、MSE、pHash），能生成差异热图。
- 能运行最小化/二分缩减算法（delta-debugging）自动去掉不必要的元素/样式以得到最小重现用例。
- 能生成并保存测试工件（HTML、CSS、截图、diff、DOM、日志、bug 报告、回归测试）。
- 可选：能应用代码修复补丁或配置改动到自研引擎代码库并触发重测（如果不安全则只生成 patch 建议）。

循环步骤（每个 cycle）：
1. 选择或生成下一个测试用例描述（见“用例生成策略”）。生成输出：{id, feature_tag, seed, html, css, assets}.
2. 在参考渲染器（Chrome）渲染并采集：ref_png, ref_dom.json, ref_layout.json, resource_logs。
3. 在自研引擎渲染并采集：eng_png, eng_render_tree.json, eng_logs。
4. 对齐两张截图（自动对齐 viewport/scale/设备像素比）。计算度量：
   - MSE（像素均方误差）
   - SSIM（结构相似度）
   - Percent different pixels（阈值例如颜色差 > 8）
   - pHash distance（感知差异）
   输出 diff_heatmap.png 与数值报告。
5. 判定 PASS/FAIL：
   - 若 SSIM >= 0.99 且 percent-diff <= 0.2%（默认阈值，可配置） => PASS，记录为覆盖该 feature。
   - 否则 => FAIL，进入缩减与修复流程。
6. FAIL 时自动缩减（最小重现）：
   - 使用 delta-debugging：重复删除 DOM 节点 / 注释 CSS 规则 / 逐步降低复杂度，直到得到最小化 HTML/CSS（能仍重现差异且包含最少元素）。
   - 产出 min_repro.html + min_repro.css + min_ref.png + min_eng.png + min_diff.png。
7. 生成 Bug 报告（结构化）包括：
   - 用例 id、feature_tag、seed、环境（browser version, DPR, engine version）
   - 最小重现 HTML/CSS（可直接粘贴）
   - 期望（reference screenshot）与实际（engine screenshot）及 diff heatmap
   - DOM/布局差异指示（ref 与 eng 的关键 computed style 列表）
   - 候选根因清单（基于差异特征，例如：box-sizing mismatch；floats/layout rounding；font fallback/metrics；text shaping/ligature；transform order；z-index stacking；replaced element intrinsic sizing；subpixel anti-aliasing；css property not implemented）
   - 优先修复建议（具体到代码层面的建议，例如：“实现 CSS box-sizing:border-box 计算时将 padding 加入 layout width 的步骤”；或“在 text shaping pipeline 中启用 HarfBuzz 风格的字形替换”）
   - 建议回归测试（将 min_repro.html 放入回归用例库）
8. 尝试自动修复（可选，启用时）：
   - 若 engine 支持 feature-toggle 或配置（例如：font-fallback-policy, subpixel-rounding-mode, use_precise_text_layout），依次尝试开启/关闭相关 toggle 并重新渲染以验证是否修复。
   - 若能通过 config 修复，记录为 CONFIG-FIX（并加入回归测试）。
   - 若启用自动 patch：生成 patch 文件（或 PR 草案）包含最小修改（代码片段/伪补丁），并附上测试结果。**默认不自动提交到主仓库，需人工 review。**
9. 修复验证：
   - 对通过 config/patch 修复的用例，重新运行 reference 与 engine 渲染并比较；若 PASS，则标为已修复并把用例标记为 COVERED，同时生成回归用例。
   - 若无法修复或修复失败，标为 NEEDS-DEV-INVESTIGATION。
10. 记录与报告：
    - 所有 artifacts（HTML/CSS、screenshots、diff、DOM、logs、bug 报告、patch 建议）上传到存储并关联 issue/tracking id。
    - 更新 coverage matrix（feature -> PASS/FAIL/NEEDS-INVESTIGATION）。
11. 选择下一个用例并重复循环，直到满足停止条件。

用例生成策略（优先级）：
按以下序列逐项覆盖，先高优先级的常见布局与渲染特性：
1. 盒模型与 position（normal flow / floats / absolute / fixed / sticky / z-index）
2. CSS 布局：Flexbox、Grid（包括 implicit tracks、auto-placement）
3. Inline / Block / inline-block / white-space / line-break / wrapping / text-overflow
4. 字体与文本：字形替换、ligature、kerning、字重、字体回退、字体替换、文字渲染对齐（subpixel, hinting）, emoji/rendering differences
5. 图片/视频/Canvas/SVG/嵌套 SVG（object、img srcset、sizes）
6. 渐变/背景/多重 background / background-clip
7. Border / border-radius / outline / box-shadow
8. Transforms / 3D transforms / perspective / transform-origin / filter（blur, drop-shadow）
9. Clip-path / mask / compositing / mix-blend-mode / opacity stacking context
10. CSS 动画与 transition（关键帧、动画填充模式、animation-timing-function）
11. 表单元素、输入法（IME）与控件风格（checkbox、select、placeholder、caret）
12. 表格布局（table layout algorithm）
13. Tables/Lists/CounterStyles
14. CSS 选择器细节（:nth-child, adjacent, attribute selectors, pseudo-elements ::before/::after）
15. Media queries / viewport / responsive layout / rem/em/vw/vh units / zoom
16. 浮点/像素四舍五入与 subpixel layout（不同 DPR 下）
17. 语言方向与国际化（rtl, bidi, complex scripts）
18. Accessibility-related styles（aria hidden, focus outline, focus ring）
19. Print / paged media related styles（若需）
20. Edge-case：large content, deeply nested DOM, extremely long strings, unusual unicode, fractional transforms

每个特性类别内部遵循：基础用例 → 组合用例 → 极端值/边界条件 → 随机变异（fuzz）→ 语义互斥/优先级冲突用例（cascade）

比较阈值与策略（可配置）：
- 默认 PASS 阈值：SSIM >= 0.99 且 percent-diff <= 0.2%。
- 对文本/typography 特性使用更严格的比对（SSIM >= 0.995）。
- 对动画先比较静态关键帧截图与每帧差异；若动画包含时间无关差异，改以行为级一致性（layout/position/time offsets）来判定。
- 允许微小抗锯齿/字体子像素差异，但应在视觉上不可感知（使用 pHash 与 SSIM 组合）。

减少假阳性/假阴性：
- 尝试多次截图并平均（若存在非确定性因素）。
- 收集资源加载时间/字体 fallback 信息，若参考端仍在 fallback，标为不可比较（skip）并记录。

停止条件（默认）：
- 所有 feature_tags 标记为 PASS，或
- 连续 N_no_new_fails（默认 500）个用例没有发现新 FAIL，用例池覆盖率停滞，或
- 达到最大循环次数 MAX_ITERATIONS（默认 10000），或
- 人工中止。

并行与优先级：
- 支持并行运行多条 agent，但每条 agent 使用不同 seed 与不同 feature bucket，避免重复劳动。
- 高优先级 feature（盒模型、布局、文本）优先探索。

产出格式（每个 cycle）：
{
 "id": "...",
 "feature_tag": "...",
 "seed": "...",
 "status": "PASS|FAIL|NEEDS-INVESTIGATION",
 "metrics": {SSIM, MSE, percent_diff, pHash_dist},
 "artifacts": {ref_png, eng_png, diff_png, min_repro.html, min_repro.css, ref_dom.json, eng_render_tree.json},
 "bug_report": {title, description, root_cause_candidates, suggested_fixes, patch_files_optional},
 "coverage_matrix": {...}
}

日志与可视化：
- 实时 dashboard（coverage heatmap, fails over time）
- 每个 FAIL 生成可点击 issue/PR 链接与回归用例

其他实用指令（策略/调试）：
- 在生成测试时优先使用系统默认字体、不同 DPI、不同 locale、不同 UA string，以覆盖字体/locale 引发的问题。
- 对 text 渲染问题，强制启用/禁用 font-variant-ligatures、font-feature-settings、text-rendering hints 来区分是 shaping 还是 rasterization 问题。
- 对 layer/compositing 问题，记录 stacking context 原因（position/opacity/transform/isolation）。
- 对 rounding/布局偏差，记录具体的几何误差（px 精度差值），以便定位四舍五入算法。
- 对动画/transition 的 mismatch，输出时间线（关键帧时间点）对比表。

安全与权限：
- 不自动提交到生产仓库，自动 patch 仅生成供人工审核的 PR 草案（除非明确允许自动提交）。
- 对于可能影响稳定性的引擎改动，须标注为 HIGH-RISK 并 require human sign-off。

结束语（循环行为）：
- 默认保持无限循环直到停止条件满足。每次循环后将产出 artifact 并更新 coverage；若发现新 FAIL，优先中断/调度开发人员查看最新 NEEDS-INVESTIGATION 项目。

--- 可选参数（启动时传入）：
--max-iterations N
--no-auto-patch (默认 true)
--concurrency M
--ssim-threshold 0.99
--percent-diff-threshold 0.2
--stop-no-new-fails K
--feature-buckets [list]
