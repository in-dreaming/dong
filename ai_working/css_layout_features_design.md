# CSS Layout Features Architecture Design

**Date**: 2026-02-23
**Status**: Design Proposal
**Features**: position: sticky, aspect-ratio, display: contents, display: flow-root

---

## Executive Summary

This document proposes architectural approaches for implementing four CSS layout features in the Dong rendering engine. Following the philosophy of ruthless simplicity and modular design, each feature is analyzed with multiple implementation strategies, trade-offs, and recommendations.

**Current Architecture Context**:
- Layout engine uses Yoga (flexbox) as core layout algorithm
- Three-pass layout: Yoga layout → IFC (inline formatting) → Positioned elements
- LayoutNode cache stores computed positions (x, y, width, height)
- Positioned elements handled in separate pass (`layoutPositionedElements`)
- ComputedStyle stores CSS properties, mapped to Yoga styles

---

## Feature 1: position: sticky

### Problem Analysis

**Scroll-aware positioning** that switches between relative and fixed positioning based on scroll offset. Element flows in normal layout until scroll threshold, then "sticks" to viewport edge.

**Key Challenges**:
- Requires scroll position awareness (currently not tracked in layout engine)
- Dual-mode rendering: relative → fixed transition
- Containing block constraints (sticky element can't escape parent bounds)
- Multiple sticky elements in same container (stacking behavior)

### Approach A: Scroll-Time Position Override (Recommended)

**Philosophy**: Treat sticky as "relative positioning with scroll-aware offsets"

**Architecture**:
```
┌─────────────────────────────────────┐
│ Layout Engine (normal flow)         │
│  - Compute as position: relative    │
│  - Store "sticky threshold" in      │
│    LayoutNode metadata              │
└─────────────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────┐
│ Renderer (Painter)                  │
│  - Check scroll_offset vs threshold │
│  - Apply fixed-like offset if stuck │
│  - Clamp to parent bounds           │
└─────────────────────────────────────┘
```

**Data Structures**:
```cpp
// In LayoutNode
struct StickyMetadata {
    float threshold_top = 0.0f;     // Offset from scroll position where sticking occurs
    float threshold_bottom = 0.0f;
    float original_y = 0.0f;        // Pre-scroll Y position
    float parent_max_y = 0.0f;      // Parent bottom bound (for clamping)
    bool is_stuck = false;
};
std::unique_ptr<StickyMetadata> sticky_info;
```

**Implementation Steps**:
1. **Layout Phase**: Compute as `position: relative`, calculate sticky thresholds from `top`/`bottom` values
2. **Render Phase**: Check `scroll_offset` against threshold, override position if stuck
3. **Parent Bounds**: Clamp sticky position to parent's content box

**Trade-offs**:
- ✅ Simple: Minimal changes to layout engine
- ✅ Separation: Scroll logic stays in renderer (view layer)
- ✅ Incremental: No full relayout on scroll
- ⚠️ Scroll awareness: Requires passing scroll offset to painter
- ❌ Reflow: Changes to sticky elements don't affect siblings (by design, but non-standard)

---

### Approach B: Layout-Time Sticky Container

**Philosophy**: Make scroll container aware during layout, compute sticky offsets in layout pass

**Architecture**:
```
┌─────────────────────────────────────┐
│ ScrollAwareLayoutEngine             │
│  - Track scroll containers          │
│  - Compute sticky positions during  │
│    layout based on scroll state     │
│  - Trigger relayout on scroll       │
└─────────────────────────────────────┘
```

**Data Structures**:
```cpp
// In Engine
struct ScrollContainer {
    dom::DOMNodePtr node;
    float scroll_offset_x = 0.0f;
    float scroll_offset_y = 0.0f;
    std::vector<dom::DOMNodePtr> sticky_children;
};
std::unordered_map<void*, ScrollContainer> scroll_containers_;
```

**Trade-offs**:
- ✅ Standard: Position computed during layout (CSS spec compliant)
- ✅ Siblings: Sticky position affects sibling layout
- ❌ Performance: Relayout on every scroll (expensive)
- ❌ Complexity: Scroll state pollutes layout engine
- ❌ Coupling: Ties layout to view-layer scroll events

---

### Recommendation: **Approach A**

**Rationale**:
- Aligns with "ruthless simplicity" - no layout engine changes for scroll state
- Matches existing `position: absolute` model (separate pass)
- Performance: O(sticky_elements) per frame vs O(DOM) relayout
- Clean separation: Scroll is a view concern, not layout concern

**Caveat**: Does not affect sibling layout (non-standard but acceptable for v1)

---

## Feature 2: aspect-ratio

### Problem Analysis

**Maintain width/height ratio** during layout. If one dimension is `auto`, compute from the other using aspect ratio.

**Key Challenges**:
- Yoga doesn't natively support aspect ratio
- Interaction with `width: auto`, `height: auto`
- Flexbox integration (aspect ratio affects flex sizing)
- Min/max constraints override

### Approach A: Pre-Layout Dimension Calculation (Recommended)

**Philosophy**: Resolve aspect ratio before Yoga layout, set as fixed dimensions or flex-basis

**Architecture**:
```
┌─────────────────────────────────────┐
│ applyDOMStylesToYoga                │
│  1. Check aspect_ratio != 0         │
│  2. If width known, height = auto → │
│     compute height = width / ratio  │
│  3. If height known, width = auto → │
│     compute width = height * ratio  │
│  4. Set YGNode dimensions           │
└─────────────────────────────────────┘
```

**Data Structures**:
```cpp
// In ComputedStyle
float aspect_ratio = 0.0f;  // 0 = no constraint, >0 = width/height ratio
```

**Implementation Steps**:
1. Parse `aspect-ratio: 16 / 9` → `aspect_ratio = 1.777f`
2. In `applyDOMStylesToYoga`:
   ```cpp
   if (style.aspect_ratio > 0.0f) {
       bool width_auto = style.width.isAuto();
       bool height_auto = style.height.isAuto();

       if (!width_auto && height_auto) {
           float w = resolveWidth(style.width);
           YGNodeStyleSetHeight(yoga_node, w / style.aspect_ratio);
       } else if (width_auto && !height_auto) {
           float h = resolveHeight(style.height);
           YGNodeStyleSetWidth(yoga_node, h * style.aspect_ratio);
       }
       // Both auto: use intrinsic size + aspect ratio
       // Both fixed: aspect ratio has no effect
   }
   ```
3. Handle min/max constraints: Apply after aspect ratio, clamp result

**Trade-offs**:
- ✅ Simple: Single calculation before layout
- ✅ Yoga-compatible: Works with existing layout engine
- ✅ Flex-aware: Integrates naturally with flex sizing
- ⚠️ Intrinsic sizing: Requires fallback for `width: auto`, `height: auto` (use intrinsic content size)
- ❌ Dynamic resize: Requires relayout if container size changes

---

### Approach B: Yoga Aspect Ratio Extension

**Philosophy**: Add aspect ratio support directly to Yoga layout algorithm

**Architecture**:
```
┌─────────────────────────────────────┐
│ Modified Yoga Layout                │
│  - Add YGNodeStyleSetAspectRatio()  │
│  - Adjust measure function          │
│  - Respect during flex layout       │
└─────────────────────────────────────┘
```

**Trade-offs**:
- ✅ Native: First-class support in layout algorithm
- ✅ Dynamic: Handles all edge cases (flex grow/shrink, intrinsic sizing)
- ❌ Complexity: Requires forking/modifying Yoga library
- ❌ Maintenance: Must merge upstream Yoga changes
- ❌ Overkill: Heavy solution for single feature

---

### Recommendation: **Approach A**

**Rationale**:
- "Library vs custom code": Don't modify Yoga when pre-processing suffices
- Handles 90% of use cases (fixed width → auto height, vice versa)
- Simple to implement, debug, and maintain
- Edge cases (both auto + aspect ratio) can use intrinsic sizing fallback

---

## Feature 3: display: contents

### Problem Analysis

**Element is layout-invisible**: Acts as if it doesn't exist in the DOM tree. Its children become direct children of its parent for layout purposes.

**Key Challenges**:
- DOM structure ≠ layout structure (breaks 1:1 mapping)
- Event targeting: Click on child should bubble through "contents" element
- Styling: Pseudo-elements (::before/::after) still render
- Yoga integration: Need to "skip" node in tree construction

### Approach A: Yoga Tree Surgery (Recommended)

**Philosophy**: Omit `display: contents` elements when building Yoga tree, attach their children to grandparent

**Architecture**:
```
DOM Tree:
  <div id="parent">
    <div id="contents" style="display: contents">
      <div id="child1"></div>
      <div id="child2"></div>
    </div>
  </div>

Yoga Tree:
  YGNode(parent)
    ├─ YGNode(child1)
    └─ YGNode(child2)
  (no YGNode for "contents")

Layout Cache:
  layout_cache[contents] = nullptr  // Mark as layout-skipped
```

**Implementation Steps**:
1. In `buildChildYogaNodes`:
   ```cpp
   void buildChildYogaNodes(dom::DOMNodePtr parent, YGNode* parent_yoga) {
       for (auto& child : parent->getChildren()) {
           if (child->getComputedStyle().display == "contents") {
               // Skip this element, but process its children
               buildChildYogaNodes(child, parent_yoga);  // Recurse, pass grandparent
               layout_cache[child.get()] = nullptr;      // Mark as skipped
           } else {
               YGNode* child_yoga = createYogaNode(child);
               YGNodeInsertChild(parent_yoga, child_yoga, ...);
               buildChildYogaNodes(child, child_yoga);
           }
       }
   }
   ```
2. In `Painter`: Check if `layout_cache[node] == nullptr` before rendering (skip box model, but still process children)
3. Event handling: Traverse through `display: contents` elements during event bubbling

**Trade-offs**:
- ✅ Clean: Minimal code changes, reuses existing tree construction
- ✅ Performance: No layout overhead for contents elements
- ✅ Correctness: Matches CSS spec (element is layout-invisible)
- ⚠️ Cache miss: `layout_cache[node] == nullptr` requires null checks
- ⚠️ Pseudo-elements: Still need to render ::before/::after (special case)

---

### Approach B: Placeholder Yoga Node

**Philosophy**: Create zero-size Yoga node for `display: contents`, forward layout to children

**Architecture**:
```cpp
if (style.display == "contents") {
    YGNode* placeholder = YGNodeNew(yoga_config);
    YGNodeStyleSetFlexDirection(placeholder, YGFlexDirectionRow);
    YGNodeStyleSetFlexWrap(placeholder, YGFlexWrapWrap);
    YGNodeStyleSetPadding(placeholder, YGEdgeAll, 0);
    YGNodeStyleSetMargin(placeholder, YGEdgeAll, 0);
    // Children attach to placeholder
}
```

**Trade-offs**:
- ✅ Yoga-compatible: Maintains 1:1 DOM-Yoga mapping
- ❌ Complexity: Placeholder nodes pollute layout tree
- ❌ Memory: Extra Yoga nodes consume memory
- ❌ Performance: Layout still processes placeholder nodes

---

### Recommendation: **Approach A**

**Rationale**:
- "Ruthless simplicity": Don't create unnecessary nodes
- Matches semantic intent: Element is *not* in layout tree
- Performance: Skip layout entirely for contents elements
- Edge cases (pseudo-elements) can be handled in painter

---

## Feature 4: display: flow-root

### Problem Analysis

**Establishes new Block Formatting Context (BFC)**: Contains floats, prevents margin collapse with external elements.

**Key Challenges**:
- Dong doesn't fully support floats yet (partial support in ComputedStyle)
- BFC mainly affects float containment and margin collapse
- Yoga doesn't model BFCs (flexbox doesn't interact with floats)

### Approach A: Semantic Marker (Recommended)

**Philosophy**: Treat as metadata for future float/margin-collapse implementation, no immediate layout changes

**Architecture**:
```cpp
// In ComputedStyle
bool creates_block_formatting_context = false;

// Computed during style resolution
if (display == "flow-root" ||
    overflow != "visible" ||
    position == "absolute" ||
    float_value != "none") {
    creates_block_formatting_context = true;
}
```

**Implementation Steps**:
1. Add `creates_block_formatting_context` flag to `ComputedStyle`
2. Set flag when `display: flow-root` is parsed
3. **No layout changes yet** (floats not implemented)
4. Document for future: When floats are added, check flag to contain floats within element

**Trade-offs**:
- ✅ Future-proof: Prepares for float implementation
- ✅ Zero cost: No runtime overhead
- ✅ Correct semantics: Parses and stores intent
- ⚠️ No effect: Feature is a no-op until floats implemented
- ❌ Incomplete: Doesn't actually create BFC behavior

---

### Approach B: Overflow Aliasing

**Philosophy**: `display: flow-root` behaves like `overflow: auto` (which creates BFC in CSS)

**Architecture**:
```cpp
// In style application
if (style.display == "flow-root") {
    style.overflow = "auto";  // Alias to overflow: auto
}
```

**Trade-offs**:
- ✅ Immediate behavior: Triggers scrollbar logic (which may create BFC-like behavior)
- ⚠️ Semantics: Not spec-compliant (flow-root ≠ overflow: auto)
- ❌ Side effects: Adds scrollbars unintentionally
- ❌ Misleading: Gives false impression of BFC support

---

### Approach C: Yoga Isolation

**Philosophy**: Use Yoga's overflow/isolation properties to approximate BFC

**Architecture**:
```cpp
if (style.display == "flow-root") {
    YGNodeStyleSetOverflow(yoga_node, YGOverflowHidden);
    // Set as isolated flex container
}
```

**Trade-offs**:
- ✅ Partial correctness: Prevents some layout leakage
- ⚠️ Yoga mismatch: Yoga doesn't model BFCs, approximation is fragile
- ❌ Confusion: Mixes CSS BFC semantics with Yoga flexbox

---

### Recommendation: **Approach A**

**Rationale**:
- "Avoid future-proofing" principle: Don't implement unused features, but...
- Exception: Semantic flag is zero-cost and enables correct incremental implementation
- Honesty: No-op is better than fake behavior (Approach B/C)
- When floats are added, flag provides correct guidance

**Note**: Document in code that this is a placeholder for future float support.

---

## Cross-Feature Interactions

### sticky + aspect-ratio
- **Issue**: Sticky element with aspect ratio may need recomputation on scroll (if width changes)
- **Solution**: Aspect ratio computed pre-scroll, sticky offset applied post-layout (independent)

### contents + sticky
- **Issue**: Sticky positioning requires layout box, but `display: contents` has none
- **Solution**: CSS spec: `display: contents` on sticky element computes to `display: block` (override in style resolution)

### flow-root + contents
- **Issue**: `display: flow-root` creates BFC, but `contents` removes box model
- **Solution**: CSS spec: `display: contents` takes precedence (no BFC created)

### aspect-ratio + flow-root
- **Issue**: None (orthogonal features)
- **Solution**: Aspect ratio computed, flow-root flag set independently

---

## Implementation Roadmap

### Phase 1: Foundation (1 week)
- **Module**: `src/dom/css/computed_style.hpp`
  - Add `aspect_ratio` field
  - Add `creates_block_formatting_context` flag
- **Module**: `src/layout/layout_node.hpp`
  - Add `StickyMetadata` struct

### Phase 2: aspect-ratio (3 days)
- **Module**: `src/layout/layout_engine.cpp` → `applyDOMStylesToYoga`
  - Implement pre-layout aspect ratio resolution
  - Handle `width: auto` / `height: auto` combinations
- **Tests**: `test_aspect_ratio_fixed_width.html`, `test_aspect_ratio_flex.html`

### Phase 3: display: contents (3 days)
- **Module**: `src/layout/layout_engine.cpp` → `buildChildYogaNodes`
  - Skip Yoga node creation for `display: contents`
  - Attach children to grandparent
- **Module**: `src/render/painter.cpp`
  - Skip box rendering for null layout cache entries
- **Tests**: `test_display_contents_layout.html`, `test_display_contents_events.html`

### Phase 4: position: sticky (5 days)
- **Module**: `src/layout/layout_engine.cpp` → `layoutPositionedElements`
  - Compute sticky thresholds during positioned layout pass
- **Module**: `src/render/painter.cpp`
  - Add scroll offset parameter
  - Apply sticky position override during rendering
- **Module**: `src/core/engine_view.cpp`
  - Pass scroll offset to painter
- **Tests**: `test_sticky_scroll_threshold.html`, `test_sticky_parent_bounds.html`

### Phase 5: display: flow-root (1 day)
- **Module**: `src/dom/css/style_resolver.cpp`
  - Set `creates_block_formatting_context = true` for `display: flow-root`
- **Documentation**: Comment explaining future float support
- **Tests**: `test_flow_root_semantics.html` (verify flag set, no visual changes yet)

---

## Module Specifications

### Module: Layout Sticky Extension

**Location**: `src/layout/layout_sticky.cpp` (new file)

**Purpose**: Compute sticky positioning thresholds and metadata

**Contract**:
- Input: `dom::DOMNodePtr`, parent scroll container bounds
- Output: `StickyMetadata` populated with thresholds
- Side Effects: None (pure computation)

**Key Functions**:
- `computeStickyThresholds(node, scroll_container) -> StickyMetadata`
- `clampStickyToParentBounds(sticky_meta, parent_bounds) -> float`

**Dependencies**: `layout_engine.hpp`, `computed_style.hpp`

---

### Module: Painter Sticky Offset

**Location**: `src/render/painter_sticky.cpp` (new file)

**Purpose**: Apply scroll-aware position override during rendering

**Contract**:
- Input: `LayoutNode*`, `scroll_offset`, `StickyMetadata`
- Output: Modified position (x, y) for rendering
- Side Effects: None (does not modify layout cache)

**Key Functions**:
- `applyStickyOffset(layout, scroll_offset, sticky_meta) -> std::pair<float, float>`

**Dependencies**: `painter.hpp`, `layout_engine.hpp`

---

### Module: Aspect Ratio Resolver

**Location**: `src/layout/layout_aspect_ratio.cpp` (new file)

**Purpose**: Compute missing dimension from aspect ratio

**Contract**:
- Input: `ComputedStyle`, known dimension (width or height)
- Output: Computed dimension (float)
- Errors: Return 0.0f if ratio invalid or both dimensions auto

**Key Functions**:
- `resolveAspectRatio(style, known_width, known_height) -> std::pair<float, float>`

**Dependencies**: `computed_style.hpp`

---

## Success Criteria

### aspect-ratio
- [ ] Image with `width: 100px; aspect-ratio: 16/9` renders at 56.25px height
- [ ] Flex item with `aspect-ratio: 1` remains square during flex grow
- [ ] Min/max constraints override aspect ratio correctly

### display: contents
- [ ] Element with `display: contents` is not rendered (no box model)
- [ ] Children of contents element layout as direct children of grandparent
- [ ] Pseudo-elements (::before, ::after) still render
- [ ] Event bubbling traverses through contents element

### position: sticky
- [ ] Element scrolls normally until threshold, then sticks to top
- [ ] Sticky element clamps to parent bottom boundary
- [ ] Multiple sticky elements stack correctly
- [ ] No layout thrashing on scroll (60 FPS maintained)

### display: flow-root
- [ ] `creates_block_formatting_context` flag set to true
- [ ] No visual changes (expected, until floats implemented)
- [ ] Does not break existing layout

---

## Philosophical Alignment

### Ruthless Simplicity
- ✅ **sticky**: Scroll logic in renderer, not layout (separation of concerns)
- ✅ **aspect-ratio**: Pre-process before Yoga, no library modification
- ✅ **contents**: Skip node creation, don't fake it with placeholders
- ✅ **flow-root**: Semantic flag, no fake behavior

### Modular Design ("Bricks & Studs")
- Each feature in separate module (sticky, aspect-ratio, contents handling)
- Clear interfaces: `computeStickyThresholds`, `resolveAspectRatio`, `buildChildYogaNodes`
- Regeneratable: Each module can be rebuilt without touching others

### Trust in Emergence
- Start minimal: Phase 1-2 deliver value (aspect-ratio + contents)
- sticky can be added incrementally (Phase 4)
- flow-root is a placeholder (Phase 5), allows future float work

### Present-Moment Focus
- Implement what's needed now (aspect-ratio, contents, sticky)
- flow-root is semantic-only (don't build unused features)

---

## Alternative Approaches Considered (Summary)

| Feature | Recommended | Alternative | Why Not Alternative |
|---------|------------|-------------|---------------------|
| sticky | Scroll-time override | Layout-time reflow | Performance, complexity |
| aspect-ratio | Pre-layout calculation | Modify Yoga | Library maintenance burden |
| contents | Tree surgery | Placeholder nodes | Unnecessary memory/CPU |
| flow-root | Semantic flag | Overflow aliasing | Incorrect semantics |

---

## Open Questions

1. **Sticky + Transforms**: How does sticky interact with `transform` on ancestors? (CSS spec: sticky positioning is transform-aware)
2. **Aspect ratio + Intrinsic sizing**: How to handle `<img>` with aspect ratio + no dimensions? (Use native image aspect ratio)
3. **Contents + Accessibility**: Does `display: contents` affect ARIA tree? (Out of scope, renderer-only concern)
4. **Flow-root timeline**: When will float support be added? (Future decision, not blocking)

---

## Conclusion

All four features can be implemented following the "ruthless simplicity" philosophy:
- **sticky**: Scroll-aware rendering, not layout
- **aspect-ratio**: Pre-processing, no library changes
- **contents**: Tree surgery, skip unnecessary nodes
- **flow-root**: Semantic flag, honest about current limitations

**Total implementation estimate**: 2-3 weeks for full implementation and testing.

**Recommended order**: aspect-ratio → contents → sticky → flow-root (most value first, complexity last).
