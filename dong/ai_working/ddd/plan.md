# DDD Plan: CSS List System - list-style Properties and ::marker Pseudo-element

## Problem Statement

### What We're Building
Complete CSS list styling system including:
1. CSS properties: `list-style-type`, `list-style-position`, `list-style`, `list-style-image`
2. `::marker` pseudo-element for custom list marker styling  
3. Automatic list marker generation for `<ul>`, `<ol>`, `<li>` elements

### Why It Matters
Lists are fundamental HTML elements used in virtually all web UIs. Current implementation lacks proper list styling, causing:
- No visible bullets/numbers on `<ul>`/`<ol>` elements
- Cannot customize marker appearance (color, size, font)
- Missing semantic visual distinction between list types
- Poor UI framework compatibility (many frameworks expect standard list rendering)

### User Value
- **Visual correctness**: Lists render with appropriate markers (bullets, numbers, etc.)
- **Customization**: Style markers independently from list content via `::marker` pseudo-element
- **Standards compliance**: Matches CSS Lists and Counters Module Level 3
- **Framework compatibility**: Enables proper rendering of UI frameworks that use lists

### Problem Being Solved
Close the gap between Dong's list rendering and web standards, enabling:
1. Default markers for unordered lists (disc, circle, square)
2. Default numbering for ordered lists (decimal, lower-alpha, upper-roman, etc.)
3. Custom marker styling through CSS
4. Marker positioning (inside vs outside)

## Proposed Solution

### High-Level Approach

**Three-Module Architecture (Bricks and Studs)**:

1. **CSS Property Module** (`dong/src/dom/css/list_style.hpp/cpp`)
   - Parse and store list-style properties
   - Default values based on HTML element type
   - Shorthand expansion

2. **Marker Generator Module** (`dong/src/render/list_marker.hpp/cpp`)
   - Generate marker text based on list-style-type
   - Counter management for ordered lists
   - Support standard marker types (disc, circle, square, decimal, alpha, roman, etc.)

3. **Marker Rendering Module** (`dong/src/render/painter/painter_marker.cpp`)
   - Render `::marker` pseudo-elements
   - Position markers (inside vs outside)
   - Apply marker-specific styles

**Integration Points**:
- CSS Parser: Register list-style properties in property handler table
- Style Engine: Create `::marker` pseudo-elements during style resolution
- Painter: Render markers during pseudo-element pass
- Layout: Reserve space for outside markers in layout calculations

## Alternatives Considered

### Alternative 1: Inline Marker Rendering (Rejected)
**Approach**: Render markers directly in `painter.cpp` without pseudo-elements

**Pros**:
- Simpler initial implementation
- Fewer files to modify

**Cons**:
- Cannot style markers independently (no `::marker` selector support)
- Violates web standards
- Harder to extend for custom marker content
- Couples rendering logic with layout logic

### Alternative 2: Full Counter System First (Deferred to P2)
**Approach**: Implement complete CSS counter system (`counter-reset`, `counter-increment`, `counter()`, `counters()`) before list markers

**Pros**:
- More powerful, handles complex numbering
- Enables custom counters for non-list elements

**Cons**:
- High complexity, low ROI for initial implementation
- List markers can work with simple internal counters
- Over-engineering for the common case

**Decision**: Start with simple per-list counters, expand to full counter system in P2 if needed

### Alternative 3: Chosen Approach - Pseudo-element Based System
**Approach**: Use `::marker` pseudo-elements with simple counter management

**Pros**:
- Standards-compliant (`::marker` is CSS spec)
- Enables independent marker styling
- Reuses existing pseudo-element infrastructure
- Clear module boundaries
- Extensible to full counter system later

**Cons**:
- Slightly more complex than inline rendering
- Need to modify pseudo-element creation logic

**Why This**: Best balance of simplicity, standards compliance, and extensibility

## Architecture & Design

### Key Interfaces (Studs)

#### 1. CSS Property Interface
```cpp
// ComputedStyle additions (dong/src/dom/css/computed_style.hpp)
struct ComputedStyle {
    // List styling
    std::string list_style_type = "none";      // disc, circle, square, decimal, etc.
    std::string list_style_position = "outside"; // outside, inside
    std::string list_style_image = "none";     // URL or none

    // Marker-specific properties (only apply to ::marker pseudo-elements)
    // These are inherited from parent list item to marker
};
```

#### 2. Marker Generator Interface
```cpp
// dong/src/render/list_marker.hpp
namespace dong::render {

struct MarkerContent {
    std::string text;        // Generated marker text (e.g., "•", "1.", "a)")
    bool is_text = true;     // true for text, false for image
    std::string image_url;   // If list-style-image is used
};

class ListMarkerGenerator {
public:
    // Generate marker content for a list item
    static MarkerContent generateMarker(
        const std::string& list_style_type,
        int counter_value,           // For ordered lists
        const std::string& list_style_image
    );

private:
    // Helper: decimal → "1."
    static std::string formatDecimal(int value);

    // Helper: lower-alpha → "a)"
    static std::string formatLowerAlpha(int value);

    // Helper: upper-roman → "IV."
    static std::string formatUpperRoman(int value);

    // ... more formatters
};

} // namespace dong::render
```

#### 3. Marker Rendering Interface
```cpp
// dong/src/render/painter.hpp additions
class Painter {
    // Render ::marker pseudo-element with proper positioning
    void renderMarkerPseudoElement(
        const dom::DOMNodePtr& marker_pseudo,
        const dom::DOMNodePtr& list_item,
        const Rect& list_item_rect,
        DisplayListBuilder& builder
    );
};
```

### Module Boundaries

#### Module 1: CSS Property Handling
**File**: `dong/src/dom/css/` (modifications to existing files)
- **Responsibility**: Parse and store list-style properties
- **Input**: CSS declaration strings ("list-style: disc inside")
- **Output**: Populated ComputedStyle fields
- **No dependencies**: Only depends on base CSS parsing infrastructure

#### Module 2: Marker Generation
**File**: `dong/src/render/list_marker.hpp/cpp`
- **Responsibility**: Generate marker text/content from list properties
- **Input**: `list_style_type`, counter value, `list_style_image`
- **Output**: MarkerContent struct with text or image URL
- **Dependencies**: None (pure function, no DOM/layout dependencies)

#### Module 3: Marker Rendering
**File**: `dong/src/render/painter/painter_marker.cpp`
- **Responsibility**: Render markers in correct position with proper styling
- **Input**: `::marker` pseudo-element, parent list item layout
- **Output**: DisplayList commands for marker rendering
- **Dependencies**: Painter, TextShaper, LayoutEngine

#### Module 4: Pseudo-element Creation
**File**: `dong/src/dom/css/style_engine.cpp` (modifications)
- **Responsibility**: Create `::marker` pseudo-elements for `<li>` elements
- **Input**: DOM tree with list items
- **Output**: `::marker` pseudo-elements attached to list items
- **Dependencies**: StyleEngine, DOMNode

### Data Models

#### 1. List Counter State
```cpp
// Stored per <ul>/<ol> element (in DOMNode extended data or context)
struct ListCounterState {
    int current_value = 1;  // Current counter value
    int start_value = 1;    // From <ol start="5"> attribute
};
```

#### 2. Marker Content
```cpp
struct MarkerContent {
    std::string text;         // "•", "1.", "a)", etc.
    bool is_text = true;      // true for text, false for image
    std::string image_url;    // For list-style-image
};
```

## Files to Change

### Non-Code Files (Phase 2)

- [ ] `doc/todo.md` - Mark P1/列表体系闭环 tasks as complete
- [ ] `doc/css_features.md` - Add list styling section documenting implementation
- [ ] `CLAUDE.md` - Add list system to feature list
- [ ] `dong/examples/data/tests/` - Add test cases (see Test Strategy section)

### Code Files (Phase 4)

#### CSS Property System
- [ ] `dong/src/dom/css/computed_style.hpp` - Add list-style properties (3 new fields)
- [ ] `dong/src/dom/css/computed_style.cpp` - Add default values, inheritance logic
- [ ] `dong/src/dom/css/css_parser.cpp` - Add property handlers for list-style-* (dispatch table entries)
- [ ] `dong/src/dom/css/style_engine.cpp` - Add `::marker` pseudo-element creation for `<li>` elements

#### Marker Generation
- [ ] `dong/src/render/list_marker.hpp` - NEW: ListMarkerGenerator class, MarkerContent struct
- [ ] `dong/src/render/list_marker.cpp` - NEW: Implement marker text generation (disc, circle, square, decimal, alpha, roman)

#### Marker Rendering
- [ ] `dong/src/render/painter.hpp` - Add renderMarkerPseudoElement() declaration
- [ ] `dong/src/render/painter/painter_marker.cpp` - NEW: Implement marker rendering logic
- [ ] `dong/src/render/painter/painter_children.cpp` - Integrate marker rendering in pseudo-element pass

#### Layout Integration
- [ ] `dong/src/layout/layout_engine.cpp` - Reserve space for outside markers (if needed, may defer)

#### Build System
- [ ] `dong/build.zig` - Add new source files to build (list_marker.cpp, painter_marker.cpp)

## Philosophy Alignment

### Ruthless Simplicity

✅ **Start minimal**:
- Begin with most common list types (disc, circle, square, decimal, lower-alpha, upper-alpha)
- Simple per-list counters (not full CSS counter system)
- Basic marker positioning (inside/outside, no complex alignment yet)

✅ **Avoid future-proofing**:
- NOT implementing: Full `counter-reset`/`counter-increment` system (P2)
- NOT implementing: Complex marker types (hebrew, georgian, etc.) initially
- NOT implementing: `list-style-image` initially (can add later)
- NOT implementing: Multi-level counter nesting (`counters()` function)

✅ **Clear over clever**:
- Marker generation is pure function: `(type, value) → string`
- Separate modules: parsing → generation → rendering
- Explicit counter management (no hidden state)

### Modular Design

✅ **Bricks (Self-contained modules)**:
1. **ListMarkerGenerator**: Pure function, no external state
2. **Marker CSS Properties**: Standard property registration pattern
3. **Marker Rendering**: Isolated in painter_marker.cpp

✅ **Studs (Connection points)**:
- **CSS → Marker Generator**: `ComputedStyle.list_style_type` + counter value
- **Marker Generator → Renderer**: `MarkerContent` struct
- **Renderer → DisplayList**: Standard painter commands (text/image)
- **StyleEngine → Painter**: `::marker` pseudo-elements

✅ **Regeneratable**:
Each module can be regenerated from this spec:
- Marker generator: Look up type, format number, return string
- CSS properties: Add fields, add handlers, done
- Rendering: Get marker content, position, render

### Implementation Chunks

**Chunk 1: CSS Property Registration**
- Add fields to ComputedStyle
- Register property handlers
- Test: Parse `list-style: disc inside` correctly

**Chunk 2: Marker Generation**
- Implement ListMarkerGenerator
- Test: `generateMarker("disc", 0)` → "•"
- Test: `generateMarker("decimal", 5)` → "5."

**Chunk 3: Pseudo-element Creation**
- Modify StyleEngine to create `::marker` for `<li>`
- Test: `<li>` has marker pseudo-element after style resolution

**Chunk 4: Marker Rendering**
- Implement painter_marker.cpp
- Test: Markers appear in test cases

**Chunk 5: Counter Management**
- Track counter state per `<ol>` element
- Increment for each `<li>` child
- Test: `<ol><li>` renders "1.", next `<li>` renders "2."

**Chunk 6: Default Styles**
- Add UA stylesheet rules for `<ul>`, `<ol>`, `<li>` defaults
- Ensure `list-style-type: disc` default for `<ul>`
- Ensure `list-style-type: decimal` default for `<ol>`
- Test: Lists render correctly without explicit styles

**Chunk 7: Build Integration**
- Update `build.zig` - Add new source files
- Test: Project builds successfully

## Test Strategy

### Unit Tests

**CSS Property Parsing** (`dong/examples/data/tests/test_list_style_parsing.html`):
```html
<style>
  .test1 { list-style-type: disc; }
  .test2 { list-style-position: inside; }
  .test3 { list-style: square outside; }
</style>
<ul class="test1"><li>Should parse disc</li></ul>
```

**Marker Generation** (C++ unit test or HTML test):
- Test: disc → "•"
- Test: decimal 1 → "1."
- Test: lower-alpha 3 → "c)"
- Test: upper-roman 4 → "IV."

### Integration Tests

**Basic List Rendering** (`dong/examples/data/tests/test_list_markers_basic.html`):
```html
<ul>
  <li>Unordered item 1 (should show disc bullet)</li>
  <li>Unordered item 2</li>
</ul>
<ol>
  <li>Ordered item 1 (should show "1.")</li>
  <li>Ordered item 2 (should show "2.")</li>
</ol>
```

**List Style Types** (`dong/examples/data/tests/test_list_style_types.html`):
```html
<style>
  .disc { list-style-type: disc; }
  .circle { list-style-type: circle; }
  .square { list-style-type: square; }
  .decimal { list-style-type: decimal; }
  .lower-alpha { list-style-type: lower-alpha; }
  .upper-alpha { list-style-type: upper-alpha; }
  .lower-roman { list-style-type: lower-roman; }
  .upper-roman { list-style-type: upper-roman; }
</style>
<!-- Test each type -->
```

**Marker Position** (`dong/examples/data/tests/test_list_position.html`):
```html
<style>
  .outside { list-style-position: outside; }
  .inside { list-style-position: inside; }
</style>
<ul class="outside"><li>Outside marker (default)</li></ul>
<ul class="inside"><li>Inside marker</li></ul>
```

**::marker Pseudo-element Styling** (`dong/examples/data/tests/test_marker_pseudo.html`):
```html
<style>
  li::marker {
    color: red;
    font-size: 20px;
    font-weight: bold;
  }
</style>
<ul>
  <li>Red bold large marker</li>
</ul>
```

**Nested Lists** (`dong/examples/data/tests/test_list_nested.html`):
```html
<ul>
  <li>Level 1
    <ul>
      <li>Level 2
        <ul>
          <li>Level 3</li>
        </ul>
      </li>
    </ul>
  </li>
</ul>
```

**Ordered List Start Attribute** (`dong/examples/data/tests/test_ol_start.html`):
```html
<ol start="5">
  <li>Should show "5."</li>
  <li>Should show "6."</li>
</ol>
```

### User Testing

**Visual Regression**:
- Use `python scripts/tools/run_baseline_compare.py` to compare against browser rendering
- Verify marker appearance matches Chrome/Firefox

**Manual Inspection**:
- Open test files in example viewer
- Verify markers appear in correct position
- Verify marker styling (color, size) works

## Implementation Approach

### Phase 2 (Docs)

**Order**:
1. Update `doc/todo.md` - Mark P1 list tasks with completion status
2. Create `doc/css_features.md` section for list styling
3. Update `CLAUDE.md` with list system feature summary
4. Create test case HTML files (empty initially, filled in Phase 4)

**Documentation Focus**:
- Implementation notes for future maintainers
- Supported list-style-type values
- Known limitations (e.g., no counter-reset yet)

### Phase 4 (Code)

**Chunk Order** (dependencies flow downward):

1. **CSS Properties** (Foundation)
   - Modify `computed_style.hpp` - Add 3 list-style fields
   - Modify `computed_style.cpp` - Add defaults, inheritance
   - Modify `css_parser.cpp` - Register handlers in dispatch table
   - **Checkpoint**: Can parse list-style properties

2. **Marker Generation** (Pure Logic)
   - Create `list_marker.hpp` - Define interfaces
   - Create `list_marker.cpp` - Implement formatters
   - **Checkpoint**: Can generate marker strings

3. **Pseudo-element Creation** (Integration Point 1)
   - Modify `style_engine.cpp` - Create `::marker` for `<li>`
   - **Checkpoint**: `<li>` elements have marker pseudo-elements

4. **Marker Rendering** (Integration Point 2)
   - Create `painter_marker.cpp` - Implement rendering
   - Modify `painter.hpp` - Add function declaration
   - Modify `painter_children.cpp` - Call marker rendering
   - **Checkpoint**: Markers render in test cases

5. **Counter Management** (Ordered Lists)
   - Extend counter state tracking (in context or DOM node)
   - Increment counters per `<li>` in `<ol>`
   - **Checkpoint**: Ordered lists show sequential numbers

6. **Default Styles** (Final Polish)
   - Add UA stylesheet rules for `<ul>`, `<ol>`, `<li>` defaults
   - Ensure `list-style-type: disc` default for `<ul>`
   - Ensure `list-style-type: decimal` default for `<ol>`
   - **Checkpoint**: Lists render correctly without explicit styles

7. **Build Integration**
   - Update `build.zig` - Add new source files
   - **Checkpoint**: Project builds successfully

**Testing Between Chunks**:
- After Chunk 1: Test CSS property parsing
- After Chunk 2: Test marker string generation
- After Chunk 4: Test visual marker rendering
- After Chunk 5: Test ordered list numbering
- After Chunk 6: Test default list rendering

## Success Criteria

### Functional Requirements
- [ ] `<ul>` displays disc bullets by default
- [ ] `<ol>` displays decimal numbers (1., 2., 3., ...) by default
- [ ] `list-style-type` property supports: disc, circle, square, decimal, lower-alpha, upper-alpha, lower-roman, upper-roman, none
- [ ] `list-style-position` property supports: outside (default), inside
- [ ] `list-style` shorthand works (e.g., `list-style: circle inside`)
- [ ] `::marker` pseudo-element can be styled (color, font-size, font-weight)
- [ ] Nested lists render correctly with proper counter scoping
- [ ] `<ol start="N">` attribute sets initial counter value

### Visual Requirements
- [ ] Markers render in visually correct position (outside = in margin, inside = in content)
- [ ] Marker appearance matches browser rendering (per baseline comparison tool)
- [ ] Marker spacing is appropriate (not overlapping content)

### Technical Requirements
- [ ] No regressions in existing tests
- [ ] Code follows project philosophy (ruthless simplicity, modular design)
- [ ] New modules are self-contained and regeneratable
- [ ] Build system includes new files

### Documentation Requirements
- [ ] `doc/todo.md` updated
- [ ] `doc/css_features.md` has list system documentation
- [ ] CLAUDE.md mentions list system support
- [ ] Test cases created for all supported features

## Next Steps

✅ **Phase 1 Complete**: Planning Approved

**Next Phase**: Update all non-code files (docs, configs, README)

**Command**: `/ddd:2-docs`

---

## Implementation Notes

### Default List Styles (UA Stylesheet)

The engine should apply these default styles (equivalent to browser UA stylesheet):

```css
ul {
    list-style-type: disc;
    padding-left: 40px;
}

ol {
    list-style-type: decimal;
    padding-left: 40px;
}

li {
    display: list-item; /* Note: May need new display value */
}
```

### Marker Positioning Details

- **outside** (default): Marker is positioned in the margin area, outside the content box
  - Layout: Reserve space in margin (may need layout engine changes)
  - Rendering: Render marker to the left of content box

- **inside**: Marker is part of the content flow
  - Layout: Marker is first inline element in content
  - Rendering: Render marker as first inline box

### Counter Scoping

For nested lists, each `<ol>` or `<ul>` should have its own counter:

```html
<ol>          <!-- Counter: 1 -->
  <li>A       <!-- Renders: "1." -->
    <ol>      <!-- Counter: 1 (new scope) -->
      <li>B   <!-- Renders: "1." -->
      <li>C   <!-- Renders: "2." -->
    </ol>
  </li>
  <li>D       <!-- Renders: "2." -->
</ol>
```

Counter state should be tracked per list element (not globally).

### Supported Marker Types (Initial Implementation)

| list-style-type | Example | Description |
|-----------------|---------|-------------|
| disc | • | Filled circle (default for `<ul>`) |
| circle | ○ | Hollow circle |
| square | ▪ | Filled square |
| decimal | 1., 2., 3., ... | Decimal numbers (default for `<ol>`) |
| lower-alpha | a., b., c., ... | Lowercase ASCII letters |
| upper-alpha | A., B., C., ... | Uppercase ASCII letters |
| lower-roman | i., ii., iii., ... | Lowercase Roman numerals |
| upper-roman | I., II., III., ... | Uppercase Roman numerals |
| none | (no marker) | Suppress marker |

**Deferred to P2**: decimal-leading-zero, lower-greek, hebrew, cjk-ideographic, etc.

### Known Limitations (Initial Implementation)

1. **No list-style-image**: Image markers not supported initially (text-only)
2. **No counter-reset/counter-increment**: Full CSS counter system not implemented
3. **No ::marker content property**: Cannot set custom marker content via CSS
4. **Basic positioning**: Marker alignment may not perfectly match browsers in all edge cases
5. **No marker box layout**: Simplified marker positioning (may need refinement)

These limitations are acceptable for P1 (high ROI basics). Can be addressed in P2 if needed.
