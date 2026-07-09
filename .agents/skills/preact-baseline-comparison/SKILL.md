---
name: preact-baseline-comparison
description: Use when comparing React/Preact component rendering between Chrome baseline and Dong engine across multiple frames. Handles ES module execution, component hydration timing, and framework-specific rendering anomalies.
---

# Preact/React Baseline Comparison & Analysis

## Overview

Specialized skill for analyzing multi-frame visual comparisons of React/Preact components rendered in Dong vs Chrome baseline. Unlike native HTML rendering, React/Preact require:

- **ES Module Execution**: Proper QuickJS module loading with pending job queue handling
- **Component Hydration Timing**: Accounting for ReactDOM.render() or Preact's diffing cycles
- **Dynamic DOM Creation**: Content appears AFTER JS execution, not in initial HTML
- **Bundle Compilation**: build → bundle.js → module load → component render

核心原则：
1. 框架差异优先：React hydration timing ≠ HTML parse completion
2. 多帧分析：检测渐进式更新、闪烁、动画不一致
3. 根因归纳：区分"框架问题"vs"引擎问题"vs"bundle/config问题"
4. 可落地建议：给出修复方向（JS执行、layout重算、paint管道、资源生命周期）

## When to Use

- 已运行 `python run_preact_baseline_compare.py --case <name>` 或全量运行
- 输出文件包含：`{test_name}_base.png`（Chrome baseline）、`{test_name}_f0.bmp`～`{test_name}_fN.bmp`（Dong多帧）、`{test_name}_report.json`（LLM分析）
- 需要理解"为什么Dong与Chrome不一致"且"这是否是可接受的差异"
- 要优先修复哪些问题、修复优先级如何

## Required Inputs

- `test_output_dir`: 运行 `run_preact_baseline_compare.py` 的输出目录（包含单个测试子目录）
- `test_name`: 测试名称（如 `discovery-buttons`、`discovery-cards`）
- `html_file` (optional): 对应的 HTML 文件，用于 JSX/component 语义理解
- `report_json` (optional): `{test_name}_report.json` 来自 vl_tool_multi.py 的分析

## Analysis Workflow

### Phase 1: 理解测试框架结构

1. **识别framework**：Preact/React/其他 ES module框架？
2. **定位entry point**：HTML中的 `<script type="module" src="./bundle.js"></script>` 或类似
3. **确认bundle内容**：bundle.js 包含 JSX→component render 的完整链条？
4. **检查hydration target**：`<div id="root"></div>` 或其他选择器被正确渲染？

### Phase 2: 基线vs多帧分析

**如果只有1帧（frames[0]）：**
- 重点：baseline（Chrome空/部分）vs Dong单帧对比
- 可能原因：JS未执行、hydration失败、bundle加载超时

**如果有多帧（frames[0]～frames[N]）：**
- 检查帧间一致性：后续帧与frame[0]是否相同？
- 若帧间变化：是否为正常动画/state update vs 闪烁/未同步
- 若帧稳定但与baseline不符：指向"框架执行一致，但样式/layout/paint异常"

### Phase 3: 差异归类（Preact/React 特有维度）

#### 分类A: 内容不可见 / 完全缺失
- **现象**：baseline显示UI，Dong全黑或#root为空
- **可能根因**：
  - Bundle加载失败（网络、路径错误）
  - JS执行超时（`DONG_SCRIPT_TIMEOUT_MS` 过短）
  - Module加载失败（QuickJS module loader bug）
  - Hydration异常（render()调用未执行或异常）
- **验证方向**：检查 engine logs、bundle URL、JS console errors

#### 分类B: 内容可见但样式/颜色不符
- **现象**：元素都在，但fill/stroke/背景色/font与baseline不同
- **可能根因**：
  - CSS cascade解析差异（selector specificity、继承链）
  - 样式资源缺失（@font-face、background-image）
  - 颜色计算（rgba、透明度处理）
  - 字体fallback（系统字体与bundled字体不一致）
- **验证方向**：运行 `--llm` 获取LLM severity 分类

#### 分类C: 布局/位置不符
- **现象**：元素尺寸、间距、对齐与baseline明显不同
- **可能根因**：
  - Flex/Grid计算差异（Yoga layout engine参数）
  - Box model bug（padding/margin/border重复计算）
  - Text shaping（文本高度、换行点）
  - Aspect ratio / min-max constraints 交互
- **验证方向**：启用debug layout bounding boxes、对比Yoga输出

#### 分类D: 文本/字形异常
- **现象**：文字模糊、位移、错位、字距不对
- **可能根因**：
  - Font rasterization (FreeType subpixel rendering)
  - Text shaping (HarfBuzz glyph clusters)
  - Baseline对齐 (vertical-align, line-height)
  - 系统字体缺失 (-apple-system, Helvetica fallback)
- **验证方向**：检查 glyph atlas、font metrics debug output

#### 分类E: 帧间不一致 / 闪烁
- **现象**：frames[0]、frames[1] 内容不同，或帧与baseline对比时忽明忽暗
- **可能根因**：
  - State update同步问题（JS_ExecutePendingJob 队列处理不完整）
  - Paint cache失效（同一DOM重复paint）
  - GPU资源生命周期问题（纹理重用、bind state污染）
  - 动画/过渡状态捕捉（CSS animation中间帧）
- **验证方向**：对比相邻帧diff、检查JS state变化

### Phase 4: LLM Severity 映射

从 `{test_name}_report.json` 中的 `llm.overall_severity` 读取：

| Severity | 含义 | 优先级 | 行动 |
|----------|------|--------|------|
| **NONE** | 完美1:1匹配 | ✓ Pass | 无需修复；可用于生产 |
| **WARNING** | 样式/AA/细微差异 | 高 | 调查根因；可能无需修复（规范允许差异） |
| **CRITICAL** | 内容缺失、布局崩坏 | 紧急 | 立即修复；标志框架/引擎关键bug |

## Output Format

### 场景1: NONE severity（完美匹配）

```
✓ Test: discovery-inbox
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Status: PASS
Severity: NONE

Summary:
  All frames show fully rendered UI matching baseline with zero pixel differences.
  Component hydration completed successfully; no timing or resource issues detected.

Verification Checklist:
  ✓ Bundle.js loaded and executed
  ✓ Preact rendered to #root without errors
  ✓ CSS styling applied correctly
  ✓ Multi-frame stability confirmed (no flickering)

Conclusion:
  This test is production-ready. Dong correctly handles Preact component lifecycle.
```

### 场景2: WARNING severity（可接受差异）

```
⚠ Test: discovery-buttons
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Status: PASS (with noted differences)
Severity: WARNING

Anomalies Detected:
  1. Color Fill Variance (raster category)
     - Region: Central button area
     - Symptom: Buttons display blue tint vs Chrome white
     - Assessment: Likely anti-aliasing or color blend mode difference
     - Action: Verify GPU blend state management; check shader blending

  2. Border Radius Edge Artifacts (raster category)
     - Region: Button corners
     - Symptom: Slight jagging at rounded corners
     - Assessment: Subpixel rasterization variance expected on different GPU stacks
     - Action: No action needed; within acceptable rendering tolerance

  3. Text Positioning Variance (text category)
     - Region: Button labels
     - Symptom: 1-2 pixel vertical offset
     - Assessment: Line-height/baseline calculation difference
     - Action: Investigate text shaper configuration; check FreeType metrics

Root Cause Hierarchy:
  1. [PRIMARY] Rasterization pipeline (GPU subpixel rendering state)
  2. [SECONDARY] Text metrics (baseline alignment, line-height)
  3. [TERTIARY] CSS cascade (inherited font properties)

Recommended Fixes (Priority Order):
  1. Verify GPU blend mode state reset between frames
  2. Check text shaper baseline calculation against browser behavior
  3. Validate CSS font-weight inheritance for button text

Engine Checklist:
  - [ ] Inspect paint/pipeline state reset (blend modes, shaders)
  - [ ] Run text metric comparison (FreeType vs browser layout)
  - [ ] Validate CSS inheritance chain for buttons

Production Decision:
  ACCEPTABLE. The observed differences are cosmetic (AA, subpixel rendering).
  Component functionality is not impaired. Suitable for production use with
  note that visual fidelity may differ slightly from web baseline.
```

### 场景3: CRITICAL severity（必须修复）

```
✗ Test: discovery-cards
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Status: FAIL
Severity: CRITICAL

Content Missing / Major Rendering Failure:
  Baseline: Chrome renders complete "Card Components" UI with icons and titles
  Dong Frame 0: Entirely black / #root div is empty
  Dong Frame 1+: Same black rendering

Root Cause Analysis:
  1. [SUSPECTED] Bundle.js not loaded
     - Check: Does html_render_test output show JS errors?
     - Verify: Is ./bundle.js accessible from data/preact-discovery-cards/?
     
  2. [SUSPECTED] Module execution timeout
     - Evidence: Image shows empty state suggests render() never called
     - Check: DONG_SCRIPT_TIMEOUT_MS sufficient? (default 10s)
     - Try: Increase to --script-timeout-ms 15000 in run_preact_baseline_compare.py
     
  3. [SUSPECTED] Hydration failure
     - Evidence: #root exists but no children
     - Check: Does console show render() errors in browser vs Dong?
     - Verify: Preact version compatibility in bundle.js

Investigation Steps:
  1. Run with verbose logging:
     ```
     DONG_LOG_LEVEL=DEBUG python run_preact_baseline_compare.py --case discovery-cards
     ```
  
  2. Check bundle integrity:
     ```
     file zig-out/bin/data/preact-discovery-cards/bundle.js
     ```
  
  3. Run single-frame with extended timeout:
     ```
     python run_preact_baseline_compare.py --case discovery-cards --frames 1 --script-timeout-ms 15000
     ```
  
  4. Compare with working test (e.g., discovery-inbox):
     - Are bundle sizes similar?
     - Is HTML structure identical (type="module", #root)?

Next Actions:
  → BLOCK: Do not merge or deploy until CRITICAL resolved
  → INVESTIGATE: Enable debug logging and trace JS execution
  → FIX: Root cause likely bundle/module loading, not Dong engine
```

## Common Patterns & Fixes

### Pattern 1: All tests fail with CRITICAL (Content Missing)

**Root Cause**: Framework issue, not test-specific
- Bundle build broken (esbuild not running)
- Module loader bug in Dong engine
- Script timeout too short for bundle eval

**Fix Priority**:
1. Verify `zig build preact` succeeds and produces all bundle.js files
2. Check if ANY preact test passes (discovery-inbox) → narrows to specific bundle vs general loader
3. Increase `DONG_SCRIPT_TIMEOUT_MS` to 15000 and retry
4. If still fails: debug QuickJS module loading in `src/script/module_loader.cpp`

### Pattern 2: Tests pass (NONE) on Linux but fail on Windows

**Root Cause**: File path normalization or font availability
- `./bundle.js` → path separator mismatch on Windows
- System font names differ (-apple-system fails on Windows)
- GPU backend variance (HLSL vs GLSL shader compilation)

**Fix Priority**:
1. Verify bundle paths use forward slashes in HTML
2. Check font fallback chain in CSS (e.g., Arial instead of -apple-system)
3. Compare GPU driver output (DirectX on Windows vs OpenGL on Linux)

### Pattern 3: WARNING for all components in a test directory

**Root Cause**: Shared resource issue or CSS import
- Shared stylesheet not applied to all components
- Font not embedded in bundle (external @font-face)
- Base styles or CSS reset missing

**Fix Priority**:
1. Check component imports: do all JSX files include shared CSS?
2. Verify bundle includes all fonts via font-face
3. Inspect CSS cascade in bundled CSS file

### Pattern 4: Frame-to-frame flickering (Diff in frames[0] but not frames[1])

**Root Cause**: JS state update or animation timing
- Component state changes between frames (expected in dynamic UI)
- CSS animation mid-frame capture
- Event handler triggered during render

**Fix Priority**:
1. Verify `--frame-ms 100` captures stable state (try `--frame-ms 500`)
2. Check if component has CSS animations or transitions
3. Review JS state machine: is initial render vs post-update intentional?

## Guardrails

1. **Don't over-fit to single frame**: Multi-frame stability matters more than pixel-perfect frame[0]
2. **Don't ignore severity**: NONE = production-ready; WARNING = investigate; CRITICAL = block
3. **Don't blame engine for bundle issues**: Many "rendering" failures are actually bundle/module loading
4. **Don't expect pixel-perfect AA**: Anti-aliasing differences between GPU stacks are expected
5. **Don't forget hydration timing**: React hydration != HTML parse; always set `--react-wait-ms` 500+ for baseline stability
6. **Don't compare across different viewport sizes**: Always run with same `--width` and `--height` for baseline

## Integration with Dong Pipeline

### Workflow Integration

```
1. Developer writes Preact component in JSX
   ↓
2. Run: zig build preact  (esbuild compiles to bundle.js)
   ↓
3. Run: python run_preact_baseline_compare.py --case <name> --llm
   ↓
4. Use this skill to analyze output JSON + images
   ↓
5. File bug in Dong repo if CRITICAL, or accept WARNING
   ↓
6. Merge component + baseline to test suite
```

### Debug Command Templates

```bash
# Generate baseline for single test with debug logging
DONG_LOG_LEVEL=DEBUG python run_preact_baseline_compare.py --case discovery-buttons --llm

# Increase hydration wait and script timeout for slow bundles
python run_preact_baseline_compare.py --case discovery-cards --react-wait-ms 1000 --script-timeout-ms 15000

# Single frame to isolate render issue
python run_preact_baseline_compare.py --case discovery-inbox --frames 1

# Batch run with skip list
python run_preact_baseline_compare.py --skip counter game-ui

# Full test suite with LLM analysis
python run_preact_baseline_compare.py --llm --width 1024 --height 768
```

## Key Metrics

Track these across baseline runs to detect regressions:

| Metric | Good | Warning | Critical |
|--------|------|---------|----------|
| NONE severity % | 100% | 50-99% | <50% |
| Average pixel diff % | <0.1% | 0.1-1% | >1% |
| Multi-frame consistency | 100% | 95-99% | <95% |
| Bundle load success | 100% | 100% | <100% |
| Avg timeout margin | >5s | 1-5s | <1s |

Use `run_preact_baseline_compare.py --llm` and aggregate `overall_severity` across all tests to monitor engine health.
