#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace dong::dom {

// ── Display ─────────────────────────────────────────────
enum class CSSDisplay : uint8_t {
    Block, Inline, InlineBlock, Flex, InlineFlex, None, FlowRoot, Contents,
    Table, InlineTable, TableCaption, TableRow, TableCell,
    TableRowGroup, TableHeaderGroup, TableFooterGroup,
    TableColumnGroup, TableColumn,
    ListItem, Grid, InlineGrid
};

inline const char* toString(CSSDisplay v) {
    switch (v) {
    case CSSDisplay::Block:            return "block";
    case CSSDisplay::Inline:           return "inline";
    case CSSDisplay::InlineBlock:      return "inline-block";
    case CSSDisplay::Flex:             return "flex";
    case CSSDisplay::InlineFlex:       return "inline-flex";
    case CSSDisplay::None:             return "none";
    case CSSDisplay::FlowRoot:         return "flow-root";
    case CSSDisplay::Contents:         return "contents";
    case CSSDisplay::Table:            return "table";
    case CSSDisplay::InlineTable:      return "inline-table";
    case CSSDisplay::TableCaption:     return "table-caption";
    case CSSDisplay::TableRow:         return "table-row";
    case CSSDisplay::TableCell:        return "table-cell";
    case CSSDisplay::TableRowGroup:    return "table-row-group";
    case CSSDisplay::TableHeaderGroup: return "table-header-group";
    case CSSDisplay::TableFooterGroup: return "table-footer-group";
    case CSSDisplay::TableColumnGroup: return "table-column-group";
    case CSSDisplay::TableColumn:      return "table-column";
    case CSSDisplay::ListItem:         return "list-item";
    case CSSDisplay::Grid:             return "grid";
    case CSSDisplay::InlineGrid:       return "inline-grid";
    }
    return "block";
}

inline CSSDisplay displayFromString(std::string_view s) {
    if (s == "block")              return CSSDisplay::Block;
    if (s == "inline")             return CSSDisplay::Inline;
    if (s == "inline-block")       return CSSDisplay::InlineBlock;
    if (s == "flex")               return CSSDisplay::Flex;
    if (s == "inline-flex")        return CSSDisplay::InlineFlex;
    if (s == "none")               return CSSDisplay::None;
    if (s == "flow-root")          return CSSDisplay::FlowRoot;
    if (s == "contents")           return CSSDisplay::Contents;
    if (s == "table")              return CSSDisplay::Table;
    if (s == "inline-table")       return CSSDisplay::InlineTable;
    if (s == "table-caption")      return CSSDisplay::TableCaption;
    if (s == "table-row")          return CSSDisplay::TableRow;
    if (s == "table-cell")         return CSSDisplay::TableCell;
    if (s == "table-row-group")    return CSSDisplay::TableRowGroup;
    if (s == "table-header-group") return CSSDisplay::TableHeaderGroup;
    if (s == "table-footer-group") return CSSDisplay::TableFooterGroup;
    if (s == "table-column-group") return CSSDisplay::TableColumnGroup;
    if (s == "table-column")       return CSSDisplay::TableColumn;
    if (s == "list-item")          return CSSDisplay::ListItem;
    if (s == "grid")               return CSSDisplay::Grid;
    if (s == "inline-grid")        return CSSDisplay::InlineGrid;
    return CSSDisplay::Block;
}

// ── Position ────────────────────────────────────────────
enum class CSSPosition : uint8_t {
    Static, Relative, Absolute, Fixed, Sticky
};

inline const char* toString(CSSPosition v) {
    switch (v) {
    case CSSPosition::Static:   return "static";
    case CSSPosition::Relative: return "relative";
    case CSSPosition::Absolute: return "absolute";
    case CSSPosition::Fixed:    return "fixed";
    case CSSPosition::Sticky:   return "sticky";
    }
    return "static";
}

inline CSSPosition positionFromString(std::string_view s) {
    if (s == "relative") return CSSPosition::Relative;
    if (s == "absolute") return CSSPosition::Absolute;
    if (s == "fixed")    return CSSPosition::Fixed;
    if (s == "sticky")   return CSSPosition::Sticky;
    return CSSPosition::Static;
}

// ── BoxSizing ───────────────────────────────────────────
enum class CSSBoxSizing : uint8_t { ContentBox, BorderBox };

inline const char* toString(CSSBoxSizing v) {
    return v == CSSBoxSizing::BorderBox ? "border-box" : "content-box";
}

inline CSSBoxSizing boxSizingFromString(std::string_view s) {
    return s == "border-box" ? CSSBoxSizing::BorderBox : CSSBoxSizing::ContentBox;
}

// ── Float ───────────────────────────────────────────────
enum class CSSFloat : uint8_t { None, Left, Right };

inline const char* toString(CSSFloat v) {
    switch (v) {
    case CSSFloat::Left:  return "left";
    case CSSFloat::Right: return "right";
    default:              return "none";
    }
}

inline CSSFloat floatFromString(std::string_view s) {
    if (s == "left")  return CSSFloat::Left;
    if (s == "right") return CSSFloat::Right;
    return CSSFloat::None;
}

// ── Clear ───────────────────────────────────────────────
enum class CSSClear : uint8_t { None, Left, Right, Both };

inline const char* toString(CSSClear v) {
    switch (v) {
    case CSSClear::Left:  return "left";
    case CSSClear::Right: return "right";
    case CSSClear::Both:  return "both";
    default:              return "none";
    }
}

inline CSSClear clearFromString(std::string_view s) {
    if (s == "left")  return CSSClear::Left;
    if (s == "right") return CSSClear::Right;
    if (s == "both")  return CSSClear::Both;
    return CSSClear::None;
}

// ── FlexDirection ───────────────────────────────────────
enum class CSSFlexDirection : uint8_t { Row, RowReverse, Column, ColumnReverse };

inline const char* toString(CSSFlexDirection v) {
    switch (v) {
    case CSSFlexDirection::RowReverse:    return "row-reverse";
    case CSSFlexDirection::Column:        return "column";
    case CSSFlexDirection::ColumnReverse: return "column-reverse";
    default:                              return "row";
    }
}

inline CSSFlexDirection flexDirectionFromString(std::string_view s) {
    if (s == "row-reverse")    return CSSFlexDirection::RowReverse;
    if (s == "column")         return CSSFlexDirection::Column;
    if (s == "column-reverse") return CSSFlexDirection::ColumnReverse;
    return CSSFlexDirection::Row;
}

// ── FlexWrap ────────────────────────────────────────────
enum class CSSFlexWrap : uint8_t { Nowrap, Wrap, WrapReverse };

inline const char* toString(CSSFlexWrap v) {
    switch (v) {
    case CSSFlexWrap::Wrap:        return "wrap";
    case CSSFlexWrap::WrapReverse: return "wrap-reverse";
    default:                       return "nowrap";
    }
}

inline CSSFlexWrap flexWrapFromString(std::string_view s) {
    if (s == "wrap")         return CSSFlexWrap::Wrap;
    if (s == "wrap-reverse") return CSSFlexWrap::WrapReverse;
    return CSSFlexWrap::Nowrap;
}

// ── JustifyContent ──────────────────────────────────────
enum class CSSJustifyContent : uint8_t {
    FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround, SpaceEvenly,
    Start, End
};

inline const char* toString(CSSJustifyContent v) {
    switch (v) {
    case CSSJustifyContent::FlexEnd:      return "flex-end";
    case CSSJustifyContent::Center:       return "center";
    case CSSJustifyContent::SpaceBetween: return "space-between";
    case CSSJustifyContent::SpaceAround:  return "space-around";
    case CSSJustifyContent::SpaceEvenly:  return "space-evenly";
    case CSSJustifyContent::Start:        return "start";
    case CSSJustifyContent::End:          return "end";
    default:                              return "flex-start";
    }
}

inline CSSJustifyContent justifyContentFromString(std::string_view s) {
    if (s == "flex-end")       return CSSJustifyContent::FlexEnd;
    if (s == "center")         return CSSJustifyContent::Center;
    if (s == "space-between")  return CSSJustifyContent::SpaceBetween;
    if (s == "space-around")   return CSSJustifyContent::SpaceAround;
    if (s == "space-evenly")   return CSSJustifyContent::SpaceEvenly;
    if (s == "start")          return CSSJustifyContent::Start;
    if (s == "end")            return CSSJustifyContent::End;
    return CSSJustifyContent::FlexStart;
}

// ── AlignItems / JustifyItems ───────────────────────────
enum class CSSAlignItems : uint8_t {
    Stretch, Normal, FlexStart, FlexEnd, Center, Baseline, Start, End
};

inline const char* toString(CSSAlignItems v) {
    switch (v) {
    case CSSAlignItems::Normal:    return "normal";
    case CSSAlignItems::FlexStart: return "flex-start";
    case CSSAlignItems::FlexEnd:   return "flex-end";
    case CSSAlignItems::Center:    return "center";
    case CSSAlignItems::Baseline:  return "baseline";
    case CSSAlignItems::Start:     return "start";
    case CSSAlignItems::End:       return "end";
    default:                       return "stretch";
    }
}

inline CSSAlignItems alignItemsFromString(std::string_view s) {
    if (s == "normal")     return CSSAlignItems::Normal;
    if (s == "flex-start") return CSSAlignItems::FlexStart;
    if (s == "flex-end")   return CSSAlignItems::FlexEnd;
    if (s == "center")     return CSSAlignItems::Center;
    if (s == "baseline")   return CSSAlignItems::Baseline;
    if (s == "start")      return CSSAlignItems::Start;
    if (s == "end")        return CSSAlignItems::End;
    return CSSAlignItems::Stretch;
}

// ── AlignContent ────────────────────────────────────────
enum class CSSAlignContent : uint8_t {
    Stretch, Normal, FlexStart, FlexEnd, Center,
    SpaceBetween, SpaceAround, Start, End
};

inline const char* toString(CSSAlignContent v) {
    switch (v) {
    case CSSAlignContent::Normal:       return "normal";
    case CSSAlignContent::FlexStart:    return "flex-start";
    case CSSAlignContent::FlexEnd:      return "flex-end";
    case CSSAlignContent::Center:       return "center";
    case CSSAlignContent::SpaceBetween: return "space-between";
    case CSSAlignContent::SpaceAround:  return "space-around";
    case CSSAlignContent::Start:        return "start";
    case CSSAlignContent::End:          return "end";
    default:                            return "stretch";
    }
}

inline CSSAlignContent alignContentFromString(std::string_view s) {
    if (s == "normal")        return CSSAlignContent::Normal;
    if (s == "flex-start")    return CSSAlignContent::FlexStart;
    if (s == "flex-end")      return CSSAlignContent::FlexEnd;
    if (s == "center")        return CSSAlignContent::Center;
    if (s == "space-between") return CSSAlignContent::SpaceBetween;
    if (s == "space-around")  return CSSAlignContent::SpaceAround;
    if (s == "start")         return CSSAlignContent::Start;
    if (s == "end")           return CSSAlignContent::End;
    return CSSAlignContent::Stretch;
}

// ── AlignSelf / JustifySelf ─────────────────────────────
enum class CSSAlignSelf : uint8_t {
    Auto, Stretch, FlexStart, FlexEnd, Center, Baseline, Start, End
};

inline const char* toString(CSSAlignSelf v) {
    switch (v) {
    case CSSAlignSelf::Stretch:   return "stretch";
    case CSSAlignSelf::FlexStart: return "flex-start";
    case CSSAlignSelf::FlexEnd:   return "flex-end";
    case CSSAlignSelf::Center:    return "center";
    case CSSAlignSelf::Baseline:  return "baseline";
    case CSSAlignSelf::Start:     return "start";
    case CSSAlignSelf::End:       return "end";
    default:                      return "auto";
    }
}

inline CSSAlignSelf alignSelfFromString(std::string_view s) {
    if (s == "stretch")    return CSSAlignSelf::Stretch;
    if (s == "flex-start") return CSSAlignSelf::FlexStart;
    if (s == "flex-end")   return CSSAlignSelf::FlexEnd;
    if (s == "center")     return CSSAlignSelf::Center;
    if (s == "baseline")   return CSSAlignSelf::Baseline;
    if (s == "start")      return CSSAlignSelf::Start;
    if (s == "end")        return CSSAlignSelf::End;
    return CSSAlignSelf::Auto;
}

// ── Overflow ────────────────────────────────────────────
enum class CSSOverflow : uint8_t { Visible, Hidden, Scroll, Auto, Clip };

inline const char* toString(CSSOverflow v) {
    switch (v) {
    case CSSOverflow::Hidden: return "hidden";
    case CSSOverflow::Scroll: return "scroll";
    case CSSOverflow::Auto:   return "auto";
    case CSSOverflow::Clip:   return "clip";
    default:                  return "visible";
    }
}

inline CSSOverflow overflowFromString(std::string_view s) {
    if (s == "hidden") return CSSOverflow::Hidden;
    if (s == "scroll") return CSSOverflow::Scroll;
    if (s == "auto")   return CSSOverflow::Auto;
    if (s == "clip")   return CSSOverflow::Clip;
    return CSSOverflow::Visible;
}

// ── OverscrollBehavior ──────────────────────────────────
enum class CSSOverscrollBehavior : uint8_t { Auto, Contain, None };

inline const char* toString(CSSOverscrollBehavior v) {
    switch (v) {
    case CSSOverscrollBehavior::Contain: return "contain";
    case CSSOverscrollBehavior::None:    return "none";
    default:                             return "auto";
    }
}

inline CSSOverscrollBehavior overscrollBehaviorFromString(std::string_view s) {
    if (s == "contain") return CSSOverscrollBehavior::Contain;
    if (s == "none")    return CSSOverscrollBehavior::None;
    return CSSOverscrollBehavior::Auto;
}

// ── ScrollBehavior ──────────────────────────────────────
enum class CSSScrollBehavior : uint8_t { Auto, Smooth };

inline const char* toString(CSSScrollBehavior v) {
    return v == CSSScrollBehavior::Smooth ? "smooth" : "auto";
}

inline CSSScrollBehavior scrollBehaviorFromString(std::string_view s) {
    return s == "smooth" ? CSSScrollBehavior::Smooth : CSSScrollBehavior::Auto;
}

// ── BorderStyle ─────────────────────────────────────────
enum class CSSBorderStyle : uint8_t {
    None, Hidden, Solid, Dashed, Dotted, Double, Groove, Ridge, Inset, Outset
};

inline const char* toString(CSSBorderStyle v) {
    switch (v) {
    case CSSBorderStyle::Hidden: return "hidden";
    case CSSBorderStyle::Solid:  return "solid";
    case CSSBorderStyle::Dashed: return "dashed";
    case CSSBorderStyle::Dotted: return "dotted";
    case CSSBorderStyle::Double: return "double";
    case CSSBorderStyle::Groove: return "groove";
    case CSSBorderStyle::Ridge:  return "ridge";
    case CSSBorderStyle::Inset:  return "inset";
    case CSSBorderStyle::Outset: return "outset";
    default:                     return "none";
    }
}

inline CSSBorderStyle borderStyleFromString(std::string_view s) {
    if (s == "solid")  return CSSBorderStyle::Solid;
    if (s == "hidden") return CSSBorderStyle::Hidden;
    if (s == "dashed") return CSSBorderStyle::Dashed;
    if (s == "dotted") return CSSBorderStyle::Dotted;
    if (s == "double") return CSSBorderStyle::Double;
    if (s == "groove") return CSSBorderStyle::Groove;
    if (s == "ridge")  return CSSBorderStyle::Ridge;
    if (s == "inset")  return CSSBorderStyle::Inset;
    if (s == "outset") return CSSBorderStyle::Outset;
    return CSSBorderStyle::None;
}

// Sentinel value for "unset per-side border style" (falls back to shorthand)
constexpr CSSBorderStyle CSSBorderStyleUnset = static_cast<CSSBorderStyle>(255);

// ── TextAlign ───────────────────────────────────────────
enum class CSSTextAlign : uint8_t {
    Left, Right, Center, Justify, Start, End
};

inline const char* toString(CSSTextAlign v) {
    switch (v) {
    case CSSTextAlign::Right:   return "right";
    case CSSTextAlign::Center:  return "center";
    case CSSTextAlign::Justify: return "justify";
    case CSSTextAlign::Start:   return "start";
    case CSSTextAlign::End:     return "end";
    default:                    return "left";
    }
}

inline CSSTextAlign textAlignFromString(std::string_view s) {
    if (s == "right")   return CSSTextAlign::Right;
    if (s == "center")  return CSSTextAlign::Center;
    if (s == "justify") return CSSTextAlign::Justify;
    if (s == "start")   return CSSTextAlign::Start;
    if (s == "end")     return CSSTextAlign::End;
    return CSSTextAlign::Left;
}

// ── TextAlignLast ───────────────────────────────────────
enum class CSSTextAlignLast : uint8_t {
    Auto, Left, Right, Center, Justify, Start, End
};

inline const char* toString(CSSTextAlignLast v) {
    switch (v) {
    case CSSTextAlignLast::Left:    return "left";
    case CSSTextAlignLast::Right:   return "right";
    case CSSTextAlignLast::Center:  return "center";
    case CSSTextAlignLast::Justify: return "justify";
    case CSSTextAlignLast::Start:   return "start";
    case CSSTextAlignLast::End:     return "end";
    default:                        return "auto";
    }
}

inline CSSTextAlignLast textAlignLastFromString(std::string_view s) {
    if (s == "left")    return CSSTextAlignLast::Left;
    if (s == "right")   return CSSTextAlignLast::Right;
    if (s == "center")  return CSSTextAlignLast::Center;
    if (s == "justify") return CSSTextAlignLast::Justify;
    if (s == "start")   return CSSTextAlignLast::Start;
    if (s == "end")     return CSSTextAlignLast::End;
    return CSSTextAlignLast::Auto;
}

// ── TextDecoration ──────────────────────────────────────
// Bitmask since multiple values can be combined (e.g. "underline line-through")
enum class CSSTextDecoration : uint8_t {
    None        = 0,
    Underline   = 1 << 0,
    Overline    = 1 << 1,
    LineThrough = 1 << 2,
};

inline CSSTextDecoration operator|(CSSTextDecoration a, CSSTextDecoration b) {
    return static_cast<CSSTextDecoration>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline CSSTextDecoration operator&(CSSTextDecoration a, CSSTextDecoration b) {
    return static_cast<CSSTextDecoration>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline std::string toString(CSSTextDecoration v) {
    if (v == CSSTextDecoration::None) return "none";
    std::string result;
    if ((v & CSSTextDecoration::Underline) != CSSTextDecoration::None) {
        result += "underline";
    }
    if ((v & CSSTextDecoration::Overline) != CSSTextDecoration::None) {
        if (!result.empty()) result += ' ';
        result += "overline";
    }
    if ((v & CSSTextDecoration::LineThrough) != CSSTextDecoration::None) {
        if (!result.empty()) result += ' ';
        result += "line-through";
    }
    return result.empty() ? "none" : result;
}

inline CSSTextDecoration textDecorationFromString(std::string_view s) {
    if (s == "none" || s.empty()) return CSSTextDecoration::None;
    CSSTextDecoration result = CSSTextDecoration::None;
    if (s.find("underline") != std::string_view::npos)    result = result | CSSTextDecoration::Underline;
    if (s.find("overline") != std::string_view::npos)     result = result | CSSTextDecoration::Overline;
    if (s.find("line-through") != std::string_view::npos) result = result | CSSTextDecoration::LineThrough;
    return result;
}

// ── TextDecorationStyle ─────────────────────────────────
enum class CSSTextDecorationStyle : uint8_t { Solid, Double, Dotted, Dashed, Wavy };

inline const char* toString(CSSTextDecorationStyle v) {
    switch (v) {
    case CSSTextDecorationStyle::Double: return "double";
    case CSSTextDecorationStyle::Dotted: return "dotted";
    case CSSTextDecorationStyle::Dashed: return "dashed";
    case CSSTextDecorationStyle::Wavy:   return "wavy";
    default:                             return "solid";
    }
}

inline CSSTextDecorationStyle textDecorationStyleFromString(std::string_view s) {
    if (s == "double") return CSSTextDecorationStyle::Double;
    if (s == "dotted") return CSSTextDecorationStyle::Dotted;
    if (s == "dashed") return CSSTextDecorationStyle::Dashed;
    if (s == "wavy")   return CSSTextDecorationStyle::Wavy;
    return CSSTextDecorationStyle::Solid;
}

// ── TextTransform ───────────────────────────────────────
enum class CSSTextTransform : uint8_t { None, Uppercase, Lowercase, Capitalize };

inline const char* toString(CSSTextTransform v) {
    switch (v) {
    case CSSTextTransform::Uppercase:   return "uppercase";
    case CSSTextTransform::Lowercase:   return "lowercase";
    case CSSTextTransform::Capitalize:  return "capitalize";
    default:                            return "none";
    }
}

inline CSSTextTransform textTransformFromString(std::string_view s) {
    if (s == "uppercase")  return CSSTextTransform::Uppercase;
    if (s == "lowercase")  return CSSTextTransform::Lowercase;
    if (s == "capitalize") return CSSTextTransform::Capitalize;
    return CSSTextTransform::None;
}

// ── TextOverflow ────────────────────────────────────────
enum class CSSTextOverflow : uint8_t { Clip, Ellipsis };

inline const char* toString(CSSTextOverflow v) {
    return v == CSSTextOverflow::Ellipsis ? "ellipsis" : "clip";
}

inline CSSTextOverflow textOverflowFromString(std::string_view s) {
    return s == "ellipsis" ? CSSTextOverflow::Ellipsis : CSSTextOverflow::Clip;
}

// ── WhiteSpace ──────────────────────────────────────────
enum class CSSWhiteSpace : uint8_t {
    Normal, Nowrap, Pre, PreWrap, PreLine, BreakSpaces
};

inline const char* toString(CSSWhiteSpace v) {
    switch (v) {
    case CSSWhiteSpace::Nowrap:      return "nowrap";
    case CSSWhiteSpace::Pre:         return "pre";
    case CSSWhiteSpace::PreWrap:     return "pre-wrap";
    case CSSWhiteSpace::PreLine:     return "pre-line";
    case CSSWhiteSpace::BreakSpaces: return "break-spaces";
    default:                         return "normal";
    }
}

inline CSSWhiteSpace whiteSpaceFromString(std::string_view s) {
    if (s == "nowrap")       return CSSWhiteSpace::Nowrap;
    if (s == "pre")          return CSSWhiteSpace::Pre;
    if (s == "pre-wrap")     return CSSWhiteSpace::PreWrap;
    if (s == "pre-line")     return CSSWhiteSpace::PreLine;
    if (s == "break-spaces") return CSSWhiteSpace::BreakSpaces;
    return CSSWhiteSpace::Normal;
}

// ── WordBreak ───────────────────────────────────────────
enum class CSSWordBreak : uint8_t { Normal, BreakAll, KeepAll, BreakWord };

inline const char* toString(CSSWordBreak v) {
    switch (v) {
    case CSSWordBreak::BreakAll:  return "break-all";
    case CSSWordBreak::KeepAll:   return "keep-all";
    case CSSWordBreak::BreakWord: return "break-word";
    default:                      return "normal";
    }
}

inline CSSWordBreak wordBreakFromString(std::string_view s) {
    if (s == "break-all")  return CSSWordBreak::BreakAll;
    if (s == "keep-all")   return CSSWordBreak::KeepAll;
    if (s == "break-word") return CSSWordBreak::BreakWord;
    return CSSWordBreak::Normal;
}

// ── OverflowWrap ────────────────────────────────────────
enum class CSSOverflowWrap : uint8_t { Normal, BreakWord, Anywhere };

inline const char* toString(CSSOverflowWrap v) {
    switch (v) {
    case CSSOverflowWrap::BreakWord: return "break-word";
    case CSSOverflowWrap::Anywhere:  return "anywhere";
    default:                         return "normal";
    }
}

inline CSSOverflowWrap overflowWrapFromString(std::string_view s) {
    if (s == "break-word") return CSSOverflowWrap::BreakWord;
    if (s == "anywhere")   return CSSOverflowWrap::Anywhere;
    return CSSOverflowWrap::Normal;
}

// ── Hyphens ─────────────────────────────────────────────
enum class CSSHyphens : uint8_t { None, Manual, Auto };

inline const char* toString(CSSHyphens v) {
    switch (v) {
    case CSSHyphens::None: return "none";
    case CSSHyphens::Auto: return "auto";
    default:               return "manual";
    }
}

inline CSSHyphens hyphensFromString(std::string_view s) {
    if (s == "none") return CSSHyphens::None;
    if (s == "auto") return CSSHyphens::Auto;
    return CSSHyphens::Manual;
}

// ── Direction ───────────────────────────────────────────
enum class CSSDirection : uint8_t { Ltr, Rtl };

inline const char* toString(CSSDirection v) {
    return v == CSSDirection::Rtl ? "rtl" : "ltr";
}

inline CSSDirection directionFromString(std::string_view s) {
    return s == "rtl" ? CSSDirection::Rtl : CSSDirection::Ltr;
}

// ── UnicodeBidi ─────────────────────────────────────────
enum class CSSUnicodeBidi : uint8_t {
    Normal, Embed, Isolate, BidiOverride, IsolateOverride, Plaintext
};

inline const char* toString(CSSUnicodeBidi v) {
    switch (v) {
    case CSSUnicodeBidi::Embed:           return "embed";
    case CSSUnicodeBidi::Isolate:         return "isolate";
    case CSSUnicodeBidi::BidiOverride:    return "bidi-override";
    case CSSUnicodeBidi::IsolateOverride: return "isolate-override";
    case CSSUnicodeBidi::Plaintext:       return "plaintext";
    default:                              return "normal";
    }
}

inline CSSUnicodeBidi unicodeBidiFromString(std::string_view s) {
    if (s == "embed")            return CSSUnicodeBidi::Embed;
    if (s == "isolate")          return CSSUnicodeBidi::Isolate;
    if (s == "bidi-override")    return CSSUnicodeBidi::BidiOverride;
    if (s == "isolate-override") return CSSUnicodeBidi::IsolateOverride;
    if (s == "plaintext")        return CSSUnicodeBidi::Plaintext;
    return CSSUnicodeBidi::Normal;
}

// ── VerticalAlign ───────────────────────────────────────
enum class CSSVerticalAlign : uint8_t {
    Baseline, Top, Bottom, Middle, Sub, Super, TextTop, TextBottom
};

inline const char* toString(CSSVerticalAlign v) {
    switch (v) {
    case CSSVerticalAlign::Top:        return "top";
    case CSSVerticalAlign::Bottom:     return "bottom";
    case CSSVerticalAlign::Middle:     return "middle";
    case CSSVerticalAlign::Sub:        return "sub";
    case CSSVerticalAlign::Super:      return "super";
    case CSSVerticalAlign::TextTop:    return "text-top";
    case CSSVerticalAlign::TextBottom: return "text-bottom";
    default:                           return "baseline";
    }
}

inline CSSVerticalAlign verticalAlignFromString(std::string_view s) {
    if (s == "top")         return CSSVerticalAlign::Top;
    if (s == "bottom")      return CSSVerticalAlign::Bottom;
    if (s == "middle")      return CSSVerticalAlign::Middle;
    if (s == "sub")         return CSSVerticalAlign::Sub;
    if (s == "super")       return CSSVerticalAlign::Super;
    if (s == "text-top")    return CSSVerticalAlign::TextTop;
    if (s == "text-bottom") return CSSVerticalAlign::TextBottom;
    return CSSVerticalAlign::Baseline;
}

// ── Visibility ──────────────────────────────────────────
enum class CSSVisibility : uint8_t { Visible, Hidden, Collapse };

inline const char* toString(CSSVisibility v) {
    switch (v) {
    case CSSVisibility::Hidden:   return "hidden";
    case CSSVisibility::Collapse: return "collapse";
    default:                      return "visible";
    }
}

inline CSSVisibility visibilityFromString(std::string_view s) {
    if (s == "hidden")   return CSSVisibility::Hidden;
    if (s == "collapse") return CSSVisibility::Collapse;
    return CSSVisibility::Visible;
}

// ── ImageRendering ──────────────────────────────────────
enum class CSSImageRendering : uint8_t { Auto, Pixelated, CrispEdges };

inline const char* toString(CSSImageRendering v) {
    switch (v) {
    case CSSImageRendering::Pixelated:  return "pixelated";
    case CSSImageRendering::CrispEdges: return "crisp-edges";
    default:                            return "auto";
    }
}

inline CSSImageRendering imageRenderingFromString(std::string_view s) {
    if (s == "pixelated")   return CSSImageRendering::Pixelated;
    if (s == "crisp-edges") return CSSImageRendering::CrispEdges;
    return CSSImageRendering::Auto;
}

// ── ColorScheme ─────────────────────────────────────────
enum class CSSColorScheme : uint8_t { Normal, Light, Dark };

inline const char* toString(CSSColorScheme v) {
    switch (v) {
    case CSSColorScheme::Light: return "light";
    case CSSColorScheme::Dark:  return "dark";
    default:                    return "normal";
    }
}

inline CSSColorScheme colorSchemeFromString(std::string_view s) {
    if (s == "light") return CSSColorScheme::Light;
    if (s == "dark")  return CSSColorScheme::Dark;
    return CSSColorScheme::Normal;
}

// ── BlendMode ───────────────────────────────────────────
enum class CSSBlendMode : uint8_t {
    Normal, Multiply, Screen, Overlay, Darken, Lighten,
    ColorDodge, ColorBurn, HardLight, SoftLight, Difference, Exclusion
};

inline const char* toString(CSSBlendMode v) {
    switch (v) {
    case CSSBlendMode::Multiply:   return "multiply";
    case CSSBlendMode::Screen:     return "screen";
    case CSSBlendMode::Overlay:    return "overlay";
    case CSSBlendMode::Darken:     return "darken";
    case CSSBlendMode::Lighten:    return "lighten";
    case CSSBlendMode::ColorDodge: return "color-dodge";
    case CSSBlendMode::ColorBurn:  return "color-burn";
    case CSSBlendMode::HardLight:  return "hard-light";
    case CSSBlendMode::SoftLight:  return "soft-light";
    case CSSBlendMode::Difference: return "difference";
    case CSSBlendMode::Exclusion:  return "exclusion";
    default:                       return "normal";
    }
}

inline CSSBlendMode blendModeFromString(std::string_view s) {
    if (s == "multiply")    return CSSBlendMode::Multiply;
    if (s == "screen")      return CSSBlendMode::Screen;
    if (s == "overlay")     return CSSBlendMode::Overlay;
    if (s == "darken")      return CSSBlendMode::Darken;
    if (s == "lighten")     return CSSBlendMode::Lighten;
    if (s == "color-dodge") return CSSBlendMode::ColorDodge;
    if (s == "color-burn")  return CSSBlendMode::ColorBurn;
    if (s == "hard-light")  return CSSBlendMode::HardLight;
    if (s == "soft-light")  return CSSBlendMode::SoftLight;
    if (s == "difference")  return CSSBlendMode::Difference;
    if (s == "exclusion")   return CSSBlendMode::Exclusion;
    return CSSBlendMode::Normal;
}

// ── BackgroundRepeat ────────────────────────────────────
enum class CSSBackgroundRepeat : uint8_t { Repeat, NoRepeat, RepeatX, RepeatY, Space, Round };

inline const char* toString(CSSBackgroundRepeat v) {
    switch (v) {
    case CSSBackgroundRepeat::NoRepeat: return "no-repeat";
    case CSSBackgroundRepeat::RepeatX:  return "repeat-x";
    case CSSBackgroundRepeat::RepeatY:  return "repeat-y";
    case CSSBackgroundRepeat::Space:    return "space";
    case CSSBackgroundRepeat::Round:    return "round";
    default:                            return "repeat";
    }
}

inline CSSBackgroundRepeat backgroundRepeatFromString(std::string_view s) {
    if (s == "no-repeat") return CSSBackgroundRepeat::NoRepeat;
    if (s == "repeat-x")  return CSSBackgroundRepeat::RepeatX;
    if (s == "repeat-y")  return CSSBackgroundRepeat::RepeatY;
    if (s == "space")     return CSSBackgroundRepeat::Space;
    if (s == "round")     return CSSBackgroundRepeat::Round;
    return CSSBackgroundRepeat::Repeat;
}

// ── BackgroundAttachment ────────────────────────────────
enum class CSSBackgroundAttachment : uint8_t { Scroll, Fixed, Local };

inline const char* toString(CSSBackgroundAttachment v) {
    switch (v) {
    case CSSBackgroundAttachment::Fixed: return "fixed";
    case CSSBackgroundAttachment::Local: return "local";
    default:                             return "scroll";
    }
}

inline CSSBackgroundAttachment backgroundAttachmentFromString(std::string_view s) {
    if (s == "fixed") return CSSBackgroundAttachment::Fixed;
    if (s == "local") return CSSBackgroundAttachment::Local;
    return CSSBackgroundAttachment::Scroll;
}

// ── BackgroundClip / BackgroundOrigin ────────────────────
enum class CSSBackgroundBox : uint8_t { BorderBox, PaddingBox, ContentBox };

inline const char* toString(CSSBackgroundBox v) {
    switch (v) {
    case CSSBackgroundBox::PaddingBox: return "padding-box";
    case CSSBackgroundBox::ContentBox: return "content-box";
    default:                           return "border-box";
    }
}

inline CSSBackgroundBox backgroundBoxFromString(std::string_view s) {
    if (s == "padding-box") return CSSBackgroundBox::PaddingBox;
    if (s == "content-box") return CSSBackgroundBox::ContentBox;
    return CSSBackgroundBox::BorderBox;
}

// ── ObjectFit ───────────────────────────────────────────
enum class CSSObjectFit : uint8_t { Fill, Contain, Cover, None, ScaleDown };

inline const char* toString(CSSObjectFit v) {
    switch (v) {
    case CSSObjectFit::Contain:   return "contain";
    case CSSObjectFit::Cover:     return "cover";
    case CSSObjectFit::None:      return "none";
    case CSSObjectFit::ScaleDown: return "scale-down";
    default:                      return "fill";
    }
}

inline CSSObjectFit objectFitFromString(std::string_view s) {
    if (s == "contain")    return CSSObjectFit::Contain;
    if (s == "cover")      return CSSObjectFit::Cover;
    if (s == "none")       return CSSObjectFit::None;
    if (s == "scale-down") return CSSObjectFit::ScaleDown;
    return CSSObjectFit::Fill;
}

// ── BorderCollapse ──────────────────────────────────────
enum class CSSBorderCollapse : uint8_t { Separate, Collapse };

inline const char* toString(CSSBorderCollapse v) {
    return v == CSSBorderCollapse::Collapse ? "collapse" : "separate";
}

inline CSSBorderCollapse borderCollapseFromString(std::string_view s) {
    return s == "collapse" ? CSSBorderCollapse::Collapse : CSSBorderCollapse::Separate;
}

// ── TableLayout ─────────────────────────────────────────
enum class CSSTableLayout : uint8_t { Auto, Fixed };

inline const char* toString(CSSTableLayout v) {
    return v == CSSTableLayout::Fixed ? "fixed" : "auto";
}

inline CSSTableLayout tableLayoutFromString(std::string_view s) {
    return s == "fixed" ? CSSTableLayout::Fixed : CSSTableLayout::Auto;
}

// ── CaptionSide ─────────────────────────────────────────
enum class CSSCaptionSide : uint8_t { Top, Bottom };

inline const char* toString(CSSCaptionSide v) {
    return v == CSSCaptionSide::Bottom ? "bottom" : "top";
}

inline CSSCaptionSide captionSideFromString(std::string_view s) {
    return s == "bottom" ? CSSCaptionSide::Bottom : CSSCaptionSide::Top;
}

// ── PointerEvents ───────────────────────────────────────
enum class CSSPointerEvents : uint8_t { Auto, None };

inline const char* toString(CSSPointerEvents v) {
    return v == CSSPointerEvents::None ? "none" : "auto";
}

inline CSSPointerEvents pointerEventsFromString(std::string_view s) {
    return s == "none" ? CSSPointerEvents::None : CSSPointerEvents::Auto;
}

// ── UserSelect ──────────────────────────────────────────
enum class CSSUserSelect : uint8_t { Auto, None, Text, All };

inline const char* toString(CSSUserSelect v) {
    switch (v) {
    case CSSUserSelect::None: return "none";
    case CSSUserSelect::Text: return "text";
    case CSSUserSelect::All:  return "all";
    default:                  return "auto";
    }
}

inline CSSUserSelect userSelectFromString(std::string_view s) {
    if (s == "none") return CSSUserSelect::None;
    if (s == "text") return CSSUserSelect::Text;
    if (s == "all")  return CSSUserSelect::All;
    return CSSUserSelect::Auto;
}

// ── TouchAction ─────────────────────────────────────────
enum class CSSTouchAction : uint8_t { Auto, None, PanX, PanY, Manipulation };

inline const char* toString(CSSTouchAction v) {
    switch (v) {
    case CSSTouchAction::None:         return "none";
    case CSSTouchAction::PanX:         return "pan-x";
    case CSSTouchAction::PanY:         return "pan-y";
    case CSSTouchAction::Manipulation: return "manipulation";
    default:                           return "auto";
    }
}

inline CSSTouchAction touchActionFromString(std::string_view s) {
    if (s == "none")         return CSSTouchAction::None;
    if (s == "pan-x")        return CSSTouchAction::PanX;
    if (s == "pan-y")        return CSSTouchAction::PanY;
    if (s == "manipulation") return CSSTouchAction::Manipulation;
    return CSSTouchAction::Auto;
}

// ── Cursor ──────────────────────────────────────────────
enum class CSSCursor : uint8_t {
    Auto, Default, Pointer, Text, Move, NotAllowed, Crosshair,
    Wait, Help, EResize, WResize, NResize, SResize, Grab, Grabbing
};

inline const char* toString(CSSCursor v) {
    switch (v) {
    case CSSCursor::Default:    return "default";
    case CSSCursor::Pointer:    return "pointer";
    case CSSCursor::Text:       return "text";
    case CSSCursor::Move:       return "move";
    case CSSCursor::NotAllowed: return "not-allowed";
    case CSSCursor::Crosshair:  return "crosshair";
    case CSSCursor::Wait:       return "wait";
    case CSSCursor::Help:       return "help";
    case CSSCursor::EResize:    return "e-resize";
    case CSSCursor::WResize:    return "w-resize";
    case CSSCursor::NResize:    return "n-resize";
    case CSSCursor::SResize:    return "s-resize";
    case CSSCursor::Grab:       return "grab";
    case CSSCursor::Grabbing:   return "grabbing";
    default:                    return "auto";
    }
}

inline CSSCursor cursorFromString(std::string_view s) {
    if (s == "default")     return CSSCursor::Default;
    if (s == "pointer")     return CSSCursor::Pointer;
    if (s == "text")        return CSSCursor::Text;
    if (s == "move")        return CSSCursor::Move;
    if (s == "not-allowed") return CSSCursor::NotAllowed;
    if (s == "crosshair")   return CSSCursor::Crosshair;
    if (s == "wait")        return CSSCursor::Wait;
    if (s == "help")        return CSSCursor::Help;
    if (s == "e-resize")    return CSSCursor::EResize;
    if (s == "w-resize")    return CSSCursor::WResize;
    if (s == "n-resize")    return CSSCursor::NResize;
    if (s == "s-resize")    return CSSCursor::SResize;
    if (s == "grab")        return CSSCursor::Grab;
    if (s == "grabbing")    return CSSCursor::Grabbing;
    return CSSCursor::Auto;
}

// ── Appearance ──────────────────────────────────────────
enum class CSSAppearance : uint8_t { Auto, None };

inline const char* toString(CSSAppearance v) {
    return v == CSSAppearance::None ? "none" : "auto";
}

inline CSSAppearance appearanceFromString(std::string_view s) {
    return s == "none" ? CSSAppearance::None : CSSAppearance::Auto;
}

// ── Resize ──────────────────────────────────────────────
enum class CSSResize : uint8_t { None, Both, Horizontal, Vertical };

inline const char* toString(CSSResize v) {
    switch (v) {
    case CSSResize::Both:       return "both";
    case CSSResize::Horizontal: return "horizontal";
    case CSSResize::Vertical:   return "vertical";
    default:                    return "none";
    }
}

inline CSSResize resizeFromString(std::string_view s) {
    if (s == "both")       return CSSResize::Both;
    if (s == "horizontal") return CSSResize::Horizontal;
    if (s == "vertical")   return CSSResize::Vertical;
    return CSSResize::None;
}

// ── FontStyle ───────────────────────────────────────────
enum class CSSFontStyle : uint8_t { Normal, Italic, Oblique };

inline const char* toString(CSSFontStyle v) {
    switch (v) {
    case CSSFontStyle::Italic:  return "italic";
    case CSSFontStyle::Oblique: return "oblique";
    default:                    return "normal";
    }
}

inline CSSFontStyle fontStyleFromString(std::string_view s) {
    if (s == "italic")  return CSSFontStyle::Italic;
    if (s == "oblique") return CSSFontStyle::Oblique;
    return CSSFontStyle::Normal;
}

// ── FontWeight ──────────────────────────────────────────
enum class CSSFontWeight : uint8_t { Normal, Bold, Bolder, Lighter, W100, W200, W300, W400, W500, W600, W700, W800, W900 };

inline const char* toString(CSSFontWeight v) {
    switch (v) {
    case CSSFontWeight::Bold:    return "bold";
    case CSSFontWeight::Bolder:  return "bolder";
    case CSSFontWeight::Lighter: return "lighter";
    case CSSFontWeight::W100:    return "100";
    case CSSFontWeight::W200:    return "200";
    case CSSFontWeight::W300:    return "300";
    case CSSFontWeight::W400:    return "400";
    case CSSFontWeight::W500:    return "500";
    case CSSFontWeight::W600:    return "600";
    case CSSFontWeight::W700:    return "700";
    case CSSFontWeight::W800:    return "800";
    case CSSFontWeight::W900:    return "900";
    default:                     return "normal";
    }
}

inline CSSFontWeight fontWeightFromString(std::string_view s) {
    if (s == "bold")    return CSSFontWeight::Bold;
    if (s == "bolder")  return CSSFontWeight::Bolder;
    if (s == "lighter") return CSSFontWeight::Lighter;
    if (s == "100")     return CSSFontWeight::W100;
    if (s == "200")     return CSSFontWeight::W200;
    if (s == "300")     return CSSFontWeight::W300;
    if (s == "400")     return CSSFontWeight::W400;
    if (s == "500")     return CSSFontWeight::W500;
    if (s == "600")     return CSSFontWeight::W600;
    if (s == "700")     return CSSFontWeight::W700;
    if (s == "800")     return CSSFontWeight::W800;
    if (s == "900")     return CSSFontWeight::W900;
    return CSSFontWeight::Normal;
}

// ── FontVariant ─────────────────────────────────────────
enum class CSSFontVariant : uint8_t { Normal, SmallCaps };

inline const char* toString(CSSFontVariant v) {
    return v == CSSFontVariant::SmallCaps ? "small-caps" : "normal";
}

inline CSSFontVariant fontVariantFromString(std::string_view s) {
    return s == "small-caps" ? CSSFontVariant::SmallCaps : CSSFontVariant::Normal;
}

// ── TransformStyle ──────────────────────────────────────
enum class CSSTransformStyle : uint8_t { Flat, Preserve3d };

inline const char* toString(CSSTransformStyle v) {
    return v == CSSTransformStyle::Preserve3d ? "preserve-3d" : "flat";
}

inline CSSTransformStyle transformStyleFromString(std::string_view s) {
    return s == "preserve-3d" ? CSSTransformStyle::Preserve3d : CSSTransformStyle::Flat;
}

// ── BackfaceVisibility ──────────────────────────────────
enum class CSSBackfaceVisibility : uint8_t { Visible, Hidden };

inline const char* toString(CSSBackfaceVisibility v) {
    return v == CSSBackfaceVisibility::Hidden ? "hidden" : "visible";
}

inline CSSBackfaceVisibility backfaceVisibilityFromString(std::string_view s) {
    return s == "hidden" ? CSSBackfaceVisibility::Hidden : CSSBackfaceVisibility::Visible;
}

// ── ListStyleType ───────────────────────────────────────
enum class CSSListStyleType : uint8_t {
    None, Disc, Circle, Square, Decimal,
    LowerAlpha, UpperAlpha, LowerRoman, UpperRoman
};

inline const char* toString(CSSListStyleType v) {
    switch (v) {
    case CSSListStyleType::Disc:       return "disc";
    case CSSListStyleType::Circle:     return "circle";
    case CSSListStyleType::Square:     return "square";
    case CSSListStyleType::Decimal:    return "decimal";
    case CSSListStyleType::LowerAlpha: return "lower-alpha";
    case CSSListStyleType::UpperAlpha: return "upper-alpha";
    case CSSListStyleType::LowerRoman: return "lower-roman";
    case CSSListStyleType::UpperRoman: return "upper-roman";
    default:                           return "none";
    }
}

inline CSSListStyleType listStyleTypeFromString(std::string_view s) {
    if (s == "disc")        return CSSListStyleType::Disc;
    if (s == "circle")      return CSSListStyleType::Circle;
    if (s == "square")      return CSSListStyleType::Square;
    if (s == "decimal")     return CSSListStyleType::Decimal;
    if (s == "lower-alpha") return CSSListStyleType::LowerAlpha;
    if (s == "upper-alpha") return CSSListStyleType::UpperAlpha;
    if (s == "lower-roman") return CSSListStyleType::LowerRoman;
    if (s == "upper-roman") return CSSListStyleType::UpperRoman;
    return CSSListStyleType::None;
}

// ── ListStylePosition ───────────────────────────────────
enum class CSSListStylePosition : uint8_t { Outside, Inside };

inline const char* toString(CSSListStylePosition v) {
    return v == CSSListStylePosition::Inside ? "inside" : "outside";
}

inline CSSListStylePosition listStylePositionFromString(std::string_view s) {
    return s == "inside" ? CSSListStylePosition::Inside : CSSListStylePosition::Outside;
}

// ── PseudoType ──────────────────────────────────────────
enum class CSSPseudoType : uint8_t {
    None, Before, After, Marker, Placeholder, Selection, Backdrop
};

inline const char* toString(CSSPseudoType v) {
    switch (v) {
    case CSSPseudoType::Before:      return "before";
    case CSSPseudoType::After:       return "after";
    case CSSPseudoType::Marker:      return "marker";
    case CSSPseudoType::Placeholder: return "placeholder";
    case CSSPseudoType::Selection:   return "selection";
    case CSSPseudoType::Backdrop:    return "backdrop";
    default:                         return "";
    }
}

inline CSSPseudoType pseudoTypeFromString(std::string_view s) {
    if (s == "before")      return CSSPseudoType::Before;
    if (s == "after")       return CSSPseudoType::After;
    if (s == "marker")      return CSSPseudoType::Marker;
    if (s == "placeholder") return CSSPseudoType::Placeholder;
    if (s == "selection")   return CSSPseudoType::Selection;
    if (s == "backdrop")    return CSSPseudoType::Backdrop;
    return CSSPseudoType::None;
}

} // namespace dong::dom
