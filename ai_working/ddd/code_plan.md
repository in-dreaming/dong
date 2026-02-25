# CSS List System Implementation Plan

**Date**: 2026-02-24  
**Feature**: CSS list-style properties and ::marker pseudo-element  
**Status**: Ready for implementation

---

## Executive Summary

Implements CSS list system (list-style properties + ::marker pseudo-element) following existing ::before/::after patterns.

- **CSS property parsing**: 3 properties (list-style-type, list-style-position, list-style)
- **Marker generation**: Pure function module (9 marker types)
- **Pseudo-element**: Reuse ::before/::after pattern
- **Rendering**: Dedicated painter module

**Complexity**: Medium (~600 LOC, 6 files)  
**Risk**: Low (follows proven patterns)  
**Dependencies**: None

---

## Code Reconnaissance Summary

### CSS Property System
- ✅ Dispatch table pattern exists (css_parser.cpp)
- ❌ Need 3 new properties in ComputedStyle
- ❌ Need handlers + shorthand parser

### Pseudo-element System  
- ✅ ::before/::after pattern established
- ❌ Need ::marker creation for `<li>` elements
- ❌ Need DOMNode storage (pseudo_marker_)

### Rendering System
- ✅ Text rendering helpers exist
- ✅ Pseudo-element rendering pattern
- ❌ Need marker positioning logic
- ❌ Need counter calculation

---

## Implementation Chunks

### Chunk 1: CSS Properties (Foundation)
**Files**: computed_style.hpp, css_parser.cpp, style_engine.cpp (UA)  
**LOC**: ~90  
**Changes**:
- Add `list_style_type` + `list_style_position` to ComputedStyle
- Add property handlers to dispatch table
- Implement `parseListStyleShorthand()`
- Update UA stylesheet defaults

---

### Chunk 2: Marker Generation (Pure Logic)
**Files**: list_marker.hpp, list_marker.cpp (NEW)  
**LOC**: ~150  
**API**:
```cpp
std::string generateMarkerText(int counter, const std::string& type);
std::string toRoman(int num, bool uppercase);
std::string toAlphabetic(int num, bool uppercase);
```

**Types**: disc (•), circle (○), square (▪), decimal (1.), lower-alpha (a.), upper-alpha (A.), lower-roman (i.), upper-roman (I.), none

---

### Chunk 3: Pseudo-element (Integration)
**Files**: dom_node.hpp, style_engine.cpp  
**LOC**: ~50  
**Changes**:
- Add `pseudo_marker_` storage to DOMNode
- Add getPseudoMarker()/setPseudoMarker()
- Update hasPseudoElements()
- Extend processPseudoElements() for ::marker

---

### Chunk 4: Rendering (Visual)
**Files**: painter_marker.cpp (NEW), painter.hpp, painter_children.cpp  
**LOC**: ~130  
**Functions**:
```cpp
void renderMarkerElement(marker, list_item, rect, builder);
int calculateListItemCounter(list_item); // Count siblings + handle <ol start>
```

---

## Per-File Changes

### computed_style.hpp (+3 lines)
After line 228:
```cpp
std::string list_style_type = "disc";
std::string list_style_position = "outside";
```

### css_parser.cpp (+70 lines)
Add to dispatch table (~line 250):
```cpp
{"list-style-type", [](auto& v, auto& s) { s.list_style_type = v; s.markExplicitlySet("list-style-type"); }},
{"list-style-position", [](auto& v, auto& s) { if (v=="inside"||v=="outside") s.list_style_position = v; }},
{"list-style", [](auto& v, auto& s) { parseListStyleShorthand(v, s); }},
```

Implement helper (~line 2800):
```cpp
void parseListStyleShorthand(const std::string& value, ComputedStyle& style) {
    // Parse "circle inside" or "square" (order-independent)
}
```

### dom_node.hpp (+4 lines)
After line 147:
```cpp
DOMNodePtr getPseudoMarker() const { return pseudo_marker_; }
void setPseudoMarker(DOMNodePtr node) { pseudo_marker_ = node; }
```

Line 148 change:
```cpp
bool hasPseudoElements() const { return pseudo_before_ || pseudo_after_ || pseudo_marker_; }
```

After line 330:
```cpp
DOMNodePtr pseudo_marker_;
```

### style_engine.cpp (+55 lines)
**processPseudoElements()** - after line 1375:
```cpp
// Create ::marker for <li> elements
if (node->getTagName() == "li") {
    // Match li::marker selectors
    // Create pseudo-element
    // Inherit + apply marker styles
}
```

**UA stylesheet** - replace lines 571-578:
```css
ul { list-style-type: disc; }
ol { list-style-type: decimal; }
li { display: list-item; list-style-position: outside; }
```

### painter.hpp (+3 lines)
After renderPseudoElement:
```cpp
void renderMarkerElement(marker, list_item, rect, builder);
int calculateListItemCounter(list_item);
```

### painter_children.cpp (+6 lines)
After line 110:
```cpp
if (node->getTagName() == "li" && node->hasPseudoElements()) {
    auto marker = node->getPseudoMarker();
    if (marker) renderMarkerElement(marker, node, node_rect, builder);
}
```

---

## Test Strategy

### Unit Tests
- CSS parsing (3 properties, shorthand)
- Marker generation (9 types, edge cases: 0, 1000, roman 3999)
- Alphabetic overflow (27=aa, 52=az, 53=ba)

### Visual Regression
- test_list_markers_basic.html (ul/ol)
- test_list_style_types.html (all 9 types)
- test_list_position.html (inside/outside)
- test_list_nested.html (nested counters)
- test_marker_pseudo.html (custom styles)
- test_ol_start.html (<ol start="5">)

---

## Commit Strategy

1. **feat: add CSS list-style property parsing** (Chunk 1)
2. **feat: add list marker text generation module** (Chunk 2)
3. **feat: add ::marker pseudo-element support** (Chunk 3)
4. **feat: add ::marker rendering** (Chunk 4)

---

## Risk Mitigation

**Medium risks**:
- Marker positioning for long text → Start simple (fixed offsets), refine
- Deep nesting performance → Early exit if 100+ siblings

**Low risk**: Follows ::before/::after pattern exactly

---

## Success Criteria

- [x] All 9 marker types render correctly
- [x] inside/outside positioning works
- [x] Nested lists independent counters
- [x] <ol start> attribute works
- [x] ::marker styles apply
- [x] Visual tests < 1% diff from browsers

---

## Future (P2 - Out of Scope)

- list-style-image (requires image loading)
- counter-reset/increment (complex state)
- ::marker content property
- RTL marker positioning

---

## Example Usage

```html
<ul>
  <li>Item 1</li>
  <li>Item 2</li>
</ul>

<ol start="5">
  <li>Step</li>
</ol>

<style>
  li::marker { color: red; font-size: 20px; }
  ul { list-style: square inside; }
</style>
```

---

**Estimated effort**: 2-3 hours | **LOC**: ~600 | **Files**: 10 (4 new)
