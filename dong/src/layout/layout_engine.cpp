#include "layout_engine.hpp"
#include "../core/log.h"
#include "../core/profiler.h"
#include <yoga/YGNode.h>
#include <yoga/YGConfig.h>
#include <yoga/Yoga.h>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cmath>
#include <cstdlib>

#include "../render/text_shaper.hpp"

namespace dong::layout {

// DirtyRect implementation
void DirtyRect::expand(float px, float py, float pw, float ph) {
    if (pw <= 0 || ph <= 0) return;
    
    if (is_empty) {
        x = px;
        y = py;
        width = pw;
        height = ph;
        is_empty = false;
    } else {
        float new_x = std::min(x, px);
        float new_y = std::min(y, py);
        float new_right = std::max(x + width, px + pw);
        float new_bottom = std::max(y + height, py + ph);
        x = new_x;
        y = new_y;
        width = new_right - new_x;
        height = new_bottom - new_y;
    }
}

bool DirtyRect::intersects(float px, float py, float pw, float ph) const {
    if (is_empty || pw <= 0 || ph <= 0) return false;
    return !(px + pw <= x || px >= x + width ||
             py + ph <= y || py >= y + height);
}

namespace {

using dong::render::TextShaper;
using dong::render::TextShapeRequest;
using dong::render::ShapedText;
using dong::render::TextMeasureCacheKey;
using dong::render::TextMeasureResult;
using dong::render::TextMeasureCache;

static std::string collapseWhitespace(const std::string& input) {
    if (input.empty()) return "";
    std::string output;
    output.reserve(input.size());
    bool in_space = false;
    for (char c : input) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!in_space) {
                output.push_back(' ');
                in_space = true;
            }
        } else {
            output.push_back(c);
            in_space = false;
        }
    }
    size_t first = output.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = output.find_last_not_of(' ');
    return output.substr(first, last - first + 1);
}

// 璁＄畻鍏冪礌鐨勫唴鍦ㄦ枃鏈搴︼紙鍖呭惈 padding锛夛紝鐢ㄤ簬鎸夐挳绛?inline-block 鍏冪礌鐨勮嚜閫傚簲瀹藉害
// 绗﹀悎 CSS 鏍囧噯锛歸idth: auto 鏃讹紝鍏冪礌瀹藉害 = 鍐呭瀹藉害 + padding + border
float computeIntrinsicTextWidth(const dom::DOMNodePtr& node) {
    if (!node) return 0.0f;
    const auto& style = node->getComputedStyle();
    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

    // 收集纯文本子节点
    bool has_text_child = false;
    std::string raw_text;
    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::TEXT) {
            raw_text += child->getTextContent();
            has_text_child = true;
        }
    }
    if (!has_text_child) return 0.0f;

    std::string text = collapseWhitespace(raw_text);
    if (text.empty()) return 0.0f;

    // 优化策略4：先查缓存
    TextMeasureCacheKey cache_key{
        text,
        style.font_family,
        style.font_weight,
        style.font_style,
        font_size,
        style.letter_spacing_em,
        style.word_spacing_px
    };
    
    TextMeasureResult cached_result;
    auto& cache = TextMeasureCache::instance();
    
    float content_width = 0.0f;
    
    if (cache.lookup(cache_key, cached_result) && cached_result.valid) {
        // 缓存命中，直接使用缓存的宽度
        content_width = cached_result.content_width_px;
    } else {
        // 缓存未命中，执行 shaping
        TextShaper shaper;
        TextShapeRequest req{};
        req.text = text;
        req.font_family = style.font_family;
        req.font_weight = style.font_weight;
        req.font_style = style.font_style;
        req.font_size = font_size;

        ShapedText shaped{};
        if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) {
            return 0.0f;
        }

    
        const float scale = shaped.scale_to_pixels;
        if (scale <= 0.0f || !std::isfinite(scale)) {
            return 0.0f;
        }

        // 注意：这里的"intrinsic width"必须与 Painter 的换行测量口径一致。
        float min_x_units = shaped.glyphs.front().pen_x_units;
        float max_x_units = shaped.glyphs.front().pen_x_units + shaped.glyphs.front().advance_x_units;
        for (const auto& g : shaped.glyphs) {
            min_x_units = std::min(min_x_units, g.pen_x_units);
            max_x_units = std::max(max_x_units, g.pen_x_units + g.advance_x_units);
        }

        content_width = (max_x_units - min_x_units) * scale;
        if (content_width < 0.0f || !std::isfinite(content_width)) {
            content_width = shaped.width_units * scale;
        }
        if (content_width < 0.0f) content_width = 0.0f;

        // letter-spacing
        if (style.letter_spacing_em != 0.0f) {
            const float letter_spacing_px = style.letter_spacing_em * font_size;
            const int glyph_count = static_cast<int>(shaped.glyphs.size());
            if (glyph_count > 1) {
                content_width += letter_spacing_px * static_cast<float>(glyph_count - 1);
            }
        }

        // word-spacing
        // NOTE: Painter 的 glyph placement 是"空格之后的 glyph 整体右移"，
        // 因此行尾空格不会带来额外宽度；测量时要扣掉这部分，避免提前换行/宽度估计偏大。
        int space_count = 0;
        if (style.word_spacing_px != 0.0f) {
            for (const auto& g : shaped.glyphs) {
                if (g.cluster < text.size() && text[g.cluster] == ' ') {
                    ++space_count;
                }
            }
            int effective_spaces = space_count;
            if (!shaped.glyphs.empty()) {
                const auto& g_last = shaped.glyphs.back();
                if (g_last.cluster < text.size() && text[g_last.cluster] == ' ') {
                    effective_spaces = std::max(0, effective_spaces - 1);
                }
            }
            if (effective_spaces > 0) {
                content_width += style.word_spacing_px * static_cast<float>(effective_spaces);
            }
        }

        
        // 缓存结果
        TextMeasureResult result;
        result.content_width_px = content_width;
        result.line_height_px = shaped.line_height_units * scale;
        result.scale_to_pixels = scale;
        result.ascent_units = shaped.ascent_units;
        result.descent_units = shaped.descent_units;
        result.line_height_units = shaped.line_height_units;
        result.glyph_count = shaped.glyphs.size();
        result.space_count = space_count;
        result.valid = true;
        cache.insert(cache_key, result);
    }

    // 加上 padding 和 border
    float pad_left = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
    float pad_right = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
    float border_w = style.border_width > 0.0f ? style.border_width : 0.0f;

    float result = content_width + pad_left + pad_right + border_w * 2.0f;
    
    if (result > 10000.0f || result < 0.0f || !std::isfinite(result)) {
        return 0.0f;
    }
    
    return result;
}

float computeIntrinsicTextHeight(const dom::DOMNodePtr& node) {
    if (!node) return 0.0f;
    const auto& style = node->getComputedStyle();
    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

    // 鏀堕泦绾枃鏈瓙鑺傜偣锛堝拷鐣ュ祵濂楀厓绱狅級
    bool has_text_child = false;
    bool has_element_child = false;
    std::string raw_text;
    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::TEXT) {
            raw_text += child->getTextContent();
            has_text_child = true;
        } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            has_element_child = true;
        }
    }
    const std::string tag = node->getTagName();
    const bool tag_prefers_text =
        tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
        tag == "h5" || tag == "h6" || tag == "p" || tag == "span" ||
        tag == "button" || tag == "code" || tag == "div" || tag == "footer";
    if (!has_text_child || (!tag_prefers_text && has_element_child)) {
        return 0.0f;
    }

    std::string text = collapseWhitespace(raw_text);
    if (text.empty()) return 0.0f;

    // 优化策略4：先查缓存
    TextMeasureCacheKey cache_key{
        text,
        style.font_family,
        style.font_weight,
        style.font_style,
        font_size,
        style.letter_spacing_em,
        style.word_spacing_px
    };
    
    TextMeasureResult cached_result;
    auto& cache = TextMeasureCache::instance();
    
    float effective_line_height = 0.0f;
    
    if (cache.lookup(cache_key, cached_result) && cached_result.valid && cached_result.line_height_px > 0.0f) {
        // Cache hit
        effective_line_height = cached_result.line_height_px;
    } else {
        // Cache miss - perform shaping
        TextShaper shaper;
        TextShapeRequest req{};
        req.text = text;
        req.font_family = style.font_family;
        req.font_weight = style.font_weight;
        req.font_style = style.font_style;
        req.font_size = font_size;


        ShapedText shaped{};
        if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) {
            DONG_LOG_INFO("[computeIntrinsicTextHeight] shaping failed for text='%s' font_size=%.1f",
                    text.c_str(), font_size);
            return 0.0f;
        }



        const float scale = shaped.scale_to_pixels;
    
        // Validate scale
        if (scale <= 0.0f || scale > 1.0f || !std::isfinite(scale)) {
            DONG_LOG_INFO("[computeIntrinsicTextHeight] INVALID scale=%.6f for text='%s'", scale, text.substr(0, 20).c_str());
            return font_size * 1.2f;  // fallback
        }
    
        effective_line_height = shaped.line_height_units * scale;
    
        // Validate line_height_units
        if (shaped.line_height_units <= 0.0f || shaped.line_height_units > 100000.0f || !std::isfinite(shaped.line_height_units)) {
            DONG_LOG_INFO("[computeIntrinsicTextHeight] INVALID line_height_units=%.1f for text='%s'", 
                    shaped.line_height_units, text.substr(0, 20).c_str());
            return font_size * 1.2f;  // fallback
        }

        // IMPORTANT:
        // Do NOT insert a partial TextMeasureResult here. applyDOMStylesToYoga() may call
        // computeIntrinsicTextHeight() before computeIntrinsicTextWidth(), and a partial cache
        // entry (content_width_px==0) will poison intrinsic width (e.g. buttons shrink to padding).
        float content_width_px = 0.0f;
        {
            float min_x_units = shaped.glyphs.front().pen_x_units;
            float max_x_units = shaped.glyphs.front().pen_x_units + shaped.glyphs.front().advance_x_units;
            for (const auto& g : shaped.glyphs) {
                min_x_units = std::min(min_x_units, g.pen_x_units);
                max_x_units = std::max(max_x_units, g.pen_x_units + g.advance_x_units);
            }

            content_width_px = (max_x_units - min_x_units) * scale;
            if (content_width_px < 0.0f || !std::isfinite(content_width_px)) {
                content_width_px = shaped.width_units * scale;
            }
            if (content_width_px < 0.0f) {
                content_width_px = 0.0f;
            }

            // letter-spacing
            if (style.letter_spacing_em != 0.0f) {
                const float letter_spacing_px = style.letter_spacing_em * font_size;
                const int glyph_count = static_cast<int>(shaped.glyphs.size());
                if (glyph_count > 1) {
                    content_width_px += letter_spacing_px * static_cast<float>(glyph_count - 1);
                }
            }
        }

        // word-spacing
        int space_count = 0;
        if (style.word_spacing_px != 0.0f) {
            for (const auto& g : shaped.glyphs) {
                if (g.cluster < text.size() && text[g.cluster] == ' ') {
                    ++space_count;
                }
            }
            if (space_count > 0) {
                content_width_px += style.word_spacing_px * static_cast<float>(space_count);
            }
        }

        TextMeasureResult result_to_cache;
        result_to_cache.content_width_px = content_width_px;
        result_to_cache.line_height_px = effective_line_height;
        result_to_cache.scale_to_pixels = scale;
        result_to_cache.ascent_units = shaped.ascent_units;
        result_to_cache.descent_units = shaped.descent_units;
        result_to_cache.line_height_units = shaped.line_height_units;
        result_to_cache.glyph_count = shaped.glyphs.size();
        result_to_cache.space_count = space_count;
        result_to_cache.valid = true;
        cache.insert(cache_key, result_to_cache);
    }
    
    // Apply minimum line height (approximating browser's line-height: normal)
    const float min_line_height = font_size * 1.2f;
    if (effective_line_height < min_line_height) {
        effective_line_height = min_line_height;
    }

    float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
    float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;

    float result = effective_line_height + pad_top + pad_bottom;
    
    // Final validation
    if (result > 10000.0f || result < 0.0f || !std::isfinite(result)) {
        DONG_LOG_INFO("[computeIntrinsicTextHeight] INVALID result=%.1f for text='%s' (effective_line_height=%.1f)",
                result, text.substr(0, 20).c_str(), effective_line_height);
        return font_size * 1.2f;  // fallback
    }
    
    return result;
}

// Overload: estimate multi-line height using an available content width.
// This fixes cases where Yoga has no TEXT nodes (so container height won't include wrapped lines),
// but Painter still draws wrapped text.
float computeIntrinsicTextHeight(const dom::DOMNodePtr& node, float parent_content_width_px) {
    if (!node) return 0.0f;

    const auto& style = node->getComputedStyle();
    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;

    bool has_text_child = false;
    bool has_element_child = false;
    std::string raw_text;
    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::TEXT) {
            raw_text += child->getTextContent();
            has_text_child = true;
        } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            has_element_child = true;
        }
    }

    const std::string tag = node->getTagName();
    const bool tag_prefers_text =
        tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
        tag == "h5" || tag == "h6" || tag == "p" || tag == "span" ||
        tag == "button" || tag == "code" || tag == "div" || tag == "footer";

    if (!has_text_child || (!tag_prefers_text && has_element_child)) {
        return 0.0f;
    }

    std::string text = collapseWhitespace(raw_text);
    if (text.empty()) return 0.0f;

    float wrap_width_px = 0.0f;
    {
        const float pad_h = (style.padding_left.isPixel() ? style.padding_left.value : 0.0f) +
                            (style.padding_right.isPixel() ? style.padding_right.value : 0.0f);
        const float border_h = (style.border_width > 0.0f ? style.border_width : 0.0f) * 2.0f;

        if (style.width.isPixel()) {
            wrap_width_px = style.width.value;
            if (style.box_sizing == "border-box") {
                wrap_width_px -= (pad_h + border_h);
            }
        } else if (style.width.isPercent() && parent_content_width_px > 0.0f) {
            wrap_width_px = parent_content_width_px * style.width.value / 100.0f;
        } else if (style.width.isAuto() && parent_content_width_px > 0.0f) {
            // width:auto 的 block 元素会填满 containing block 的内容宽度
            wrap_width_px = parent_content_width_px;
        }

        if (wrap_width_px < 0.0f || !std::isfinite(wrap_width_px)) {
            wrap_width_px = 0.0f;
        }
    }

    // 没有宽度约束时，退回单行高度逻辑
    if (wrap_width_px <= 0.0f) {
        return computeIntrinsicTextHeight(node);
    }

    TextShaper shaper;
    TextShapeRequest req{};
    req.text = text;
    req.font_family = style.font_family;
    req.font_weight = style.font_weight;
    req.font_style = style.font_style;
    req.font_size = font_size;

    ShapedText shaped{};
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) {
        return 0.0f;
    }

    const float scale = shaped.scale_to_pixels;
    if (scale <= 0.0f || !std::isfinite(scale)) {
        return font_size * 1.2f;
    }

    float effective_line_height = shaped.line_height_units * scale;
    const float min_line_height = font_size * 1.2f;
    if (effective_line_height < min_line_height) {
        effective_line_height = min_line_height;
    }

    const float letter_spacing_px = (style.letter_spacing_em != 0.0f) ? (style.letter_spacing_em * font_size) : 0.0f;
    const float letter_spacing_units = (letter_spacing_px != 0.0f && scale > 0.0f) ? (letter_spacing_px / scale) : 0.0f;
    const float word_spacing_units = (style.word_spacing_px != 0.0f && scale > 0.0f) ? (style.word_spacing_px / scale) : 0.0f;

    // break points：空格后 + 非 ASCII 字符后
    std::vector<size_t> break_points;
    break_points.reserve(text.size() + 1);
    auto push_break = [&](size_t byte_index) {
        if (!break_points.empty() && break_points.back() == byte_index) return;
        break_points.push_back(byte_index);
    };

    size_t i_byte = 0;
    while (i_byte < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i_byte]);
        size_t char_len = 1;
        if ((c & 0b10000000) == 0) {
            i_byte += 1;
            if (c == ' ') {
                push_break(i_byte);
            }
            continue;
        } else if ((c & 0b11100000) == 0b11000000) {
            char_len = 2;
        } else if ((c & 0b11110000) == 0b11100000) {
            char_len = 3;
        } else if ((c & 0b11111000) == 0b11110000) {
            char_len = 4;
        }
        i_byte += char_len;
        push_break(i_byte);
    }
    if (break_points.empty() || break_points.back() != text.size()) {
        break_points.push_back(text.size());
    }

    const auto& glyphs = shaped.glyphs;
    auto measure_range_units = [&](size_t byte_start, size_t byte_end) -> float {
        int first = -1;
        int last = -1;
        int glyph_count = 0;
        int space_count = 0;

        for (size_t gi = 0; gi < glyphs.size(); ++gi) {
            uint32_t cluster = glyphs[gi].cluster;
            if (cluster < byte_start) continue;
            if (cluster >= byte_end) break;
            if (first == -1) first = static_cast<int>(gi);
            last = static_cast<int>(gi);
            ++glyph_count;
            if (cluster < text.size() && text[cluster] == ' ') {
                ++space_count;
            }
        }

        if (first == -1 || last == -1 || glyph_count == 0) return 0.0f;

        const auto& g_first = glyphs[static_cast<size_t>(first)];
        const auto& g_last = glyphs[static_cast<size_t>(last)];

        float left = g_first.pen_x_units;
        float right = g_last.pen_x_units + g_last.advance_x_units;
        float w = right - left;

        if (glyph_count > 1 && letter_spacing_units != 0.0f) {
            w += letter_spacing_units * static_cast<float>(glyph_count - 1);
        }

        if (space_count > 0 && word_spacing_units != 0.0f) {
            int effective_spaces = space_count;
            // 行尾空格不应增加宽度
            if (g_last.cluster < text.size() && text[g_last.cluster] == ' ') {
                effective_spaces = std::max(0, effective_spaces - 1);
            }
            w += word_spacing_units * static_cast<float>(effective_spaces);
        }

        return w > 0.0f ? w : 0.0f;
    };

    int line_count = 0;
    size_t line_start = 0;
    while (line_start < text.size()) {
        while (line_start < text.size() && text[line_start] == ' ') {
            ++line_start;
        }
        if (line_start >= text.size()) break;

        size_t best_break = text.size();
        bool found_any = false;

        for (size_t bp : break_points) {
            if (bp <= line_start) continue;
            float w_px = measure_range_units(line_start, bp) * scale;
            if (w_px <= wrap_width_px + 0.1f) {
                found_any = true;
                best_break = bp;
            } else {
                break;
            }
        }

        if (!found_any) {
            for (size_t bp : break_points) {
                if (bp > line_start) {
                    best_break = bp;
                    break;
                }
            }
        }

        if (best_break <= line_start) {
            best_break = std::min(line_start + 1, text.size());
        }

        ++line_count;
        line_start = best_break;
    }

    if (line_count <= 0) line_count = 1;

    float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
    float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;

    float result = effective_line_height * static_cast<float>(line_count) + pad_top + pad_bottom;

    if (result > 10000.0f || result < 0.0f || !std::isfinite(result)) {
        return font_size * 1.2f;
    }

    return result;
}

// Collapse vertical margins between two sibling block-like elements so that

// the gap between them is closer to CSS's max(margin-bottom, margin-top)
// instead of a simple sum. This is a simplified approximation that focuses
// on pixel margins and common block layouts.
void collapseVerticalMarginBetweenSiblings(const dom::DOMNodePtr& prev_node,
                                           YGNode* prev_yoga,
                                           const dom::DOMNodePtr& curr_node,
                                           YGNode* curr_yoga) {
    if (!prev_node || !curr_node || !prev_yoga || !curr_yoga) {
        return;
    }

    const auto& prev_style = prev_node->getComputedStyle();
    const auto& curr_style = curr_node->getComputedStyle();

    // Do not attempt to collapse margins for flex items or non-visible blocks here.
    if (prev_style.layout_mode == dom::LayoutMode::Flex ||
        curr_style.layout_mode == dom::LayoutMode::Flex) {
        return;
    }

    float prev_mb = 0.0f;
    if (prev_style.margin_bottom.isPixel()) {
        prev_mb = prev_style.margin_bottom.value;
    }

    float curr_mt = 0.0f;
    if (curr_style.margin_top.isPixel()) {
        curr_mt = curr_style.margin_top.value;
    }

    if (prev_mb <= 0.0f || curr_mt <= 0.0f) {
        return;
    }

    // We want gap = prev_mb + adjusted_top ~= max(prev_mb, curr_mt).
    // Setting adjusted_top = max(curr_mt - prev_mb, 0) achieves that:
    // - If curr_mt >= prev_mb: gap = prev_mb + (curr_mt - prev_mb) = curr_mt
    // - If curr_mt <  prev_mb: gap = prev_mb
    float adjusted_top = curr_mt - prev_mb;
    if (adjusted_top < 0.0f) {
        adjusted_top = 0.0f;
    }

    YGNodeStyleSetMargin(curr_yoga, YGEdgeTop, adjusted_top);
}

dom::DOMNodePtr firstElementChild(const dom::DOMNodePtr& node) {
    if (!node) {
        return nullptr;
    }
    for (const auto& child : node->getChildren()) {
        if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            return child;
        }
    }
    return nullptr;
}

struct InlineMetrics {
    float content_width_px = 0.0f;        // 绾枃鏈唴瀹瑰搴︼紙涓嶅惈 padding锛?
    float line_height_px = 0.0f;
    float baseline_from_content_top_px = 0.0f;
    float padding_left_px = 0.0f;         // 宸︿晶 padding
    float padding_right_px = 0.0f;        // 鍙充晶 padding
    float total_width_px = 0.0f;          // 瀹屾暣瀹藉害 = content + padding
};

static bool computeInlineMetricsForNode(const dom::DOMNodePtr& node,
                                        InlineMetrics& out_metrics,
                                        float fallback_font_size_px) {
    if (!node) {
        return false;
    }

    const auto& style = node->getComputedStyle();
    float font_size_px = style.font_size > 0.0f ? style.font_size : fallback_font_size_px;
    if (font_size_px <= 0.0f) {
        font_size_px = 16.0f;
    }

    // 鏀堕泦璇ュ厓绱犵洿鎺ュ瓙鑺傜偣涓殑鏂囨湰锛屽拷鐣ュ祵濂楀厓绱狅紝淇濇寔涓?Painter 鏂囨湰缁樺埗涓€鑷?
    bool has_text_child = false;
    bool has_element_child = false;
    std::string raw_text;
    for (const auto& child : node->getChildren()) {
        if (!child) continue;
        if (child->getType() == dom::DOMNode::NodeType::TEXT) {
            raw_text += child->getTextContent();
            has_text_child = true;
        } else if (child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            has_element_child = true;
        }
    }

    const std::string tag = node->getTagName();
    const bool tag_prefers_text =
        tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
        tag == "h5" || tag == "h6" || tag == "p" || tag == "span" ||
        tag == "button" || tag == "code" || tag == "div" || tag == "footer";

    if (!has_text_child || (!tag_prefers_text && has_element_child)) {
        return false;
    }

    std::string text = collapseWhitespace(raw_text);
    if (text.empty()) {
        return false;
    }

    TextShaper shaper;
    TextShapeRequest req{};
    req.text = text;
    req.font_family = style.font_family;
    req.font_weight = style.font_weight;
    req.font_style = style.font_style;
    req.font_size = font_size_px;

    ShapedText shaped{};
    if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) {
        return false;
    }


        const float scale = shaped.scale_to_pixels;

    // 宽度：用 glyph 的 min/max pen_x 来测量（更接近 Painter 的 measure_range_units 口径）
    float min_x_units = shaped.glyphs.front().pen_x_units;
    float max_x_units = shaped.glyphs.front().pen_x_units + shaped.glyphs.front().advance_x_units;
    for (const auto& g : shaped.glyphs) {
        min_x_units = std::min(min_x_units, g.pen_x_units);
        max_x_units = std::max(max_x_units, g.pen_x_units + g.advance_x_units);
    }

    float content_width_px = (max_x_units - min_x_units) * scale;
    if (content_width_px < 0.0f || !std::isfinite(content_width_px)) {
        content_width_px = shaped.width_units * scale;
    }
    if (content_width_px < 0.0f) {
        content_width_px = 0.0f;
    }

    // letter-spacing：Painter 以 glyph 数量近似（glyph_count - 1）
    if (style.letter_spacing_em != 0.0f) {
        const float letter_spacing_px = style.letter_spacing_em * font_size_px;
        const int glyph_count = static_cast<int>(shaped.glyphs.size());
        if (glyph_count > 1) {
            content_width_px += letter_spacing_px * static_cast<float>(glyph_count - 1);
        }
    }

    // word-spacing：对 cluster 落在空格字符的 glyph 计数
    // 注意：与 Painter 的测量/placement 口径保持一致，行尾空格不应增加宽度。
    if (style.word_spacing_px != 0.0f) {
        int space_count = 0;
        for (const auto& g : shaped.glyphs) {
            if (g.cluster < text.size() && text[g.cluster] == ' ') {
                ++space_count;
            }
        }
        int effective_spaces = space_count;
        if (!shaped.glyphs.empty()) {
            const auto& g_last = shaped.glyphs.back();
            if (g_last.cluster < text.size() && text[g_last.cluster] == ' ') {
                effective_spaces = std::max(0, effective_spaces - 1);
            }
        }
        if (effective_spaces > 0) {
            content_width_px += style.word_spacing_px * static_cast<float>(effective_spaces);
        }
    }


    out_metrics.content_width_px = content_width_px;

    // 符合 CSS 盒模型标准：计算包含 padding 的完整宽度
    out_metrics.padding_left_px = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
    out_metrics.padding_right_px = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
    out_metrics.total_width_px = out_metrics.content_width_px +
                                  out_metrics.padding_left_px +
                                  out_metrics.padding_right_px;

    // 琛岄珮涓?baseline锛氬鐢?Painter 涓殑搴﹂噺閫昏緫锛屼繚璇佷竴鑷?
    float line_height_units = shaped.line_height_units;
    float ascent_units = shaped.ascent_units;
    float descent_units = shaped.descent_units;

    // 搴旂敤 CSS line-height 灞炴€?
    if (style.line_height > 0.0f) {
        if (style.line_height_is_unitless) {
            // 鍊嶆暟锛歭ine-height * font-size
            float css_line_height_px = style.line_height * font_size_px;
            line_height_units = css_line_height_px / std::max(scale, 1e-3f);
        } else {
            // 鍍忕礌鍊?
            line_height_units = style.line_height / std::max(scale, 1e-3f);
        }
    }

    if (line_height_units <= 0.0f) {
        line_height_units = font_size_px / std::max(scale, 1e-3f);
    }
    // 瀵?`line-height: normal` 鍋氫竴涓祻瑙堝櫒绾у埆鐨勪笅闄愶細涓嶅皯浜?1.2 * font-size锛?
    // 淇濊瘉琛岀洅楂樺害瓒冲瀹圭撼澶у瓧鍙?glyph锛岄伩鍏嶅潡绾у厔寮熷厓绱犲帇浣忎笂涓€琛屾枃鏈€?
    const float min_line_height_px = font_size_px * 1.2f;
    if (line_height_units * scale < min_line_height_px) {
        line_height_units = min_line_height_px / std::max(scale, 1e-3f);
    }
    if (ascent_units <= 0.0f) {
        ascent_units = font_size_px / std::max(scale, 1e-3f);
    }

    float descent_abs_units = descent_units < 0.0f ? -descent_units : 0.0f;
    float metrics_height_units = ascent_units + descent_abs_units;
    if (metrics_height_units <= 0.0f) {
        metrics_height_units = line_height_units;
    }
    float extra_leading_units = line_height_units - metrics_height_units;
    if (extra_leading_units < 0.0f) {
        extra_leading_units = 0.0f;
    }
    float top_leading_units = extra_leading_units * 0.5f;

    out_metrics.line_height_px = line_height_units * scale;
    out_metrics.baseline_from_content_top_px = (top_leading_units + ascent_units) * scale;
    return true;
}

bool isInlineLevelDisplay(const std::string& display) {
    return display == "inline" || display == "inline-block";
}

bool isInlineFormattingContext(const dom::DOMNodePtr& node) {
    if (!node || node->getType() != dom::DOMNode::NodeType::ELEMENT) {
        return false;
    }

    const auto& style = node->getComputedStyle();

    // 涓嶅湪鏍圭骇澶у鍣ㄤ笂鍚敤鍐呰仈鏍煎紡鍖栵紝閬垮厤涓€娆℃€ч噸鍐欏お澶氬竷灞€
    const std::string tag = node->getTagName();
    if (tag == "html" || tag == "body") {
        return false;
    }

    if (style.display == "none" || style.display == "flex") {
        return false;
    }

    bool has_inline_child = false;
    bool has_block_like_child = false;

    for (const auto& child : node->getChildren()) {
        if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
            continue;
        }
        const auto& cs = child->getComputedStyle();
        
        if (isInlineLevelDisplay(cs.display)) {
            has_inline_child = true;
        } else if (cs.display == "block" || cs.display == "flex" || cs.display == "none") {
            has_block_like_child = true;
        }
    }

    // 浠呭綋瀛樺湪 inline/inline-block 瀛愬厓绱狅紝涓旀病鏈夋贩鍏ュ叾浠?block/flex 瀛愬厓绱犳椂锛?
    // 灏嗚瀹瑰櫒瑙嗕负鍐呰仈鏍煎紡鍖栦笂涓嬫枃銆傝繖鏍峰彲浠ュ畨鍏ㄥ湴瑕嗙洊 align_basic_layout 绛夊満鏅紝
    // 鍙堜笉褰卞搷閫氱敤 block 甯冨眬銆?
    return has_inline_child && !has_block_like_child;
}

} // anonymous namespace

LayoutNode::~LayoutNode() {
    if (yoga_node) {
        YGNodeFree(yoga_node);
    }
}

Engine::Engine() {
    yoga_config = YGConfigNew();
    if (!yoga_config) {
        std::cerr << "Failed to create Yoga config\n";
    }
}

Engine::~Engine() {
    layout_cache.clear();
    if (yoga_config) {
        YGConfigFree(yoga_config);
    }
}

void Engine::calculateLayout(dom::DOMNodePtr root, float width, float height) {
    DONG_PROFILE_FUNCTION();
    
    if (!root || !yoga_config) return;

    // Cache viewport size for resolving vw/vh units in style mapping.
    viewport_width_ = width;
    viewport_height_ = height;

    // Reset dirty rect for this layout pass
    dirty_rect_ = DirtyRect();

    // Check if any node is dirty; if not, skip layout entirely
    std::function<bool(const dom::DOMNodePtr&)> hasAnyDirtyNode =
        [&hasAnyDirtyNode](const dom::DOMNodePtr& node) -> bool {
            if (!node) return false;
            if (node->isLayoutDirty()) return true;
            for (const auto& child : node->getChildren()) {
                if (hasAnyDirtyNode(child)) return true;
            }
            return false;
        };

    if (!hasAnyDirtyNode(root)) {
        // Everything is clean, no need to relayout
        return;
    }

    // Rebuild layout tree from scratch each time for now to avoid stale Yoga nodes
    layout_cache.clear();

    // Prefer the first real element (e.g., <html>) as the Yoga root instead of the #document node
    dom::DOMNodePtr layout_root_dom = root;
    if (root->getType() != dom::DOMNode::NodeType::ELEMENT) {
        auto element_child = firstElementChild(root);
        if (element_child) {
            layout_root_dom = element_child;
        }
    }

    if (!layout_root_dom) return;

    // Cache root font size for resolving rem units (best-effort).
    {
        const auto& cs = layout_root_dom->getComputedStyle();
        if (cs.font_size > 0.0f && std::isfinite(cs.font_size)) {
            root_font_size_ = cs.font_size;
        } else {
            root_font_size_ = 16.0f;
        }
    }

    // Create Yoga tree from the chosen DOM root
    YGNode* yoga_root = createYogaNode(layout_root_dom);
    if (!yoga_root) return;

    // Set viewport size on Yoga root (now the <html> element in most cases)
    YGNodeStyleSetWidth(yoga_root, width);
    YGNodeStyleSetHeight(yoga_root, height);

    // Calculate layout
    YGNodeCalculateLayout(yoga_root, width, height, YGDirectionLTR);

    // Extract layout info back to cache (recursively). Convert Yoga's relative
    // positions into absolute coordinates so the painter can use them directly.
    // 
    // WORKAROUND: Yoga sometimes calculates huge height values (> 100000) for elements
    // that are siblings of scroll containers. This appears to be a Yoga bug related to
    // overflow handling. We detect and correct these values during extraction.
    std::function<void(dom::DOMNodePtr, YGNode*, float, float, float)> extractLayoutRecursive =
        [this, &extractLayoutRecursive](dom::DOMNodePtr dom_node, YGNode* yoga_node,
                                        float parent_x, float parent_y, float cumulative_sibling_height) {
            if (!dom_node || !yoga_node) return;

            auto& node_layout = layout_cache[dom_node.get()];
            bool is_new_layout = false;
            if (!node_layout) {
                node_layout = std::make_unique<LayoutNode>();
                is_new_layout = true;
            }
            node_layout->yoga_node = yoga_node;

            const float left = YGNodeLayoutGetLeft(yoga_node);
            const float top = YGNodeLayoutGetTop(yoga_node);
            const float width = YGNodeLayoutGetWidth(yoga_node);
            const float height = YGNodeLayoutGetHeight(yoga_node);

            float new_x = parent_x + left;
            float new_y = parent_y + top;
            float new_width = width;
            float new_height = height;
            
            // Workaround: detect and fix Yoga layout calculation bugs
            // Sometimes Yoga returns huge values (> 100000) for height, which indicates a bug
            // related to scroll containers. We detect and correct these values.
            const auto& style = dom_node->getComputedStyle();
            const std::string node_tag = dom_node->getTagName();
            
            if (new_height > 100000.0f || !std::isfinite(new_height)) {
                // Try to use the style height if available
                if (style.height.isPixel()) {
                    new_height = style.height.value;
                } else if (node_tag == "input") {
                    // input 鍏冪礌锛氫娇鐢?font-size + padding 璁＄畻楂樺害
                    float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
                    float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
                    float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;
                    new_height = font_size * 1.2f + pad_top + pad_bottom;
                } else {
                    // Fallback to a reasonable default based on content
                    for (const auto& child : dom_node->getChildren()) {
                        if (child && child->getType() == dom::DOMNode::NodeType::TEXT) {
                            float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
                            new_height = font_size * 1.2f;
                            break;
                        }
                    }
                }
            }
            
            // Fix huge top values by using cumulative sibling height tracking
            if (top > 100000.0f || !std::isfinite(top)) {
                // Use the cumulative height from previous siblings
                new_y = parent_y + cumulative_sibling_height;
            }

            // Check if layout changed: position or size differs
            bool layout_changed = is_new_layout || 
                                  node_layout->x != new_x ||
                                  node_layout->y != new_y ||
                                  node_layout->width != new_width ||
                                  node_layout->height != new_height;

            node_layout->x = new_x;
            node_layout->y = new_y;
            node_layout->width = new_width;
            node_layout->height = new_height;
            node_layout->layout_recalculated = true;
            
            // Debug: log button-row and button layout
            const std::string debug_class = dom_node->getAttribute("class");
            const std::string tag = dom_node->getTagName();

            // Keep legacy fields and new layout struct in sync (absolute coords)
            node_layout->layout.position[0] = node_layout->x;
            node_layout->layout.position[1] = node_layout->y;
            node_layout->layout.dimensions[0] = node_layout->width;
            node_layout->layout.dimensions[1] = node_layout->height;

            // If this node's layout changed, add its area to dirty rect
            if (layout_changed) {
                dirty_rect_.expand(new_x, new_y, new_width, new_height);
            }

            // Recursively extract child layouts. Only ELEMENT nodes get Yoga children.
            // Track cumulative height of siblings for position correction
            uint32_t child_count = YGNodeGetChildCount(yoga_node);
            uint32_t yoga_child_index = 0;
            float child_cumulative_height = 0.0f;
            
            // Get parent padding for child positioning
            float parent_padding_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
            
            for (const auto& child_dom : dom_node->getChildren()) {
                if (!child_dom || child_dom->getType() != dom::DOMNode::NodeType::ELEMENT) {
                    continue;
                }
                if (yoga_child_index >= child_count) {
                    break;
                }
                YGNode* child_yoga = YGNodeGetChild(yoga_node, yoga_child_index);
                if (child_yoga) {
                    // Pass cumulative sibling height for position correction
                    extractLayoutRecursive(child_dom, child_yoga,
                                            node_layout->x, node_layout->y,
                                            parent_padding_top + child_cumulative_height);
                    
                    // Get the child's corrected height for cumulative tracking
                    auto child_layout = layout_cache.find(child_dom.get());
                    if (child_layout != layout_cache.end() && child_layout->second) {
                        float child_h = child_layout->second->height;
                        // Also add margin-bottom if present
                        const auto& child_style = child_dom->getComputedStyle();
                        float margin_bottom = child_style.margin_bottom.isPixel() ? child_style.margin_bottom.value : 0.0f;
                        child_cumulative_height += child_h + margin_bottom;
                    }
                }
                ++yoga_child_index;
            }
        };

    extractLayoutRecursive(layout_root_dom, yoga_root, 0.0f, 0.0f, 0.0f);

    // After Yoga layout, apply Block Formatting Context adjustments:
    // - margin: auto horizontal centering
    // - margin-left: auto / margin-right: auto alignment
    layoutBlockFormattingContext(layout_root_dom);

    // After Yoga layout, run inline formatting context layout to adjust
    // inline/inline-block children inside suitable containers.
    layoutInlineFormattingContexts(layout_root_dom);

    // Then layout positioned elements (position:absolute) relative to their containing blocks.
    layoutPositionedElements(layout_root_dom);

    // Ensure the #document node itself still has a layout entry so rendering can start from it
    if (layout_root_dom.get() != root.get()) {
        auto& doc_layout = layout_cache[root.get()];
        if (!doc_layout) {
            doc_layout = std::make_unique<LayoutNode>();
        }
        doc_layout->x = 0.0f;
        doc_layout->y = 0.0f;
        doc_layout->width = width;
        doc_layout->height = height;
        doc_layout->layout.position[0] = 0.0f;
        doc_layout->layout.position[1] = 0.0f;
        doc_layout->layout.dimensions[0] = width;
        doc_layout->layout.dimensions[1] = height;
        doc_layout->yoga_node = nullptr; // #document is synthetic; it doesn't have a corresponding Yoga node
    }
}

const LayoutNode* Engine::getLayout(dom::DOMNodePtr node) const {
    if (!node) return nullptr;
    
    auto it = layout_cache.find(node.get());
    if (it != layout_cache.end()) {
        return it->second.get();
    }
    return nullptr;
}

void Engine::markDirty(dom::DOMNodePtr node) {
    if (!node) return;

    // Find corresponding Yoga node and mark dirty
    auto layout = getLayout(node);
    if (layout && layout->yoga_node) {
        YGNodeMarkDirty(layout->yoga_node);
    }
}

YGNode* Engine::createYogaNode(dom::DOMNodePtr dom_node) {
    if (!dom_node || !yoga_config) return nullptr;

    // Reuse existing Yoga node for this DOM node if it already exists in the cache,
    // otherwise create a new one. This avoids reallocating the Yoga tree on every
    // layout pass and lets us rely on Yoga's own dirty-marking for subtrees.
    auto& node_layout = layout_cache[dom_node.get()];
    if (!node_layout) {
        node_layout = std::make_unique<LayoutNode>();
    }

    YGNode* yoga_node = node_layout->yoga_node;
    if (!yoga_node) {
        yoga_node = YGNodeNewWithConfig(yoga_config);
        if (!yoga_node) return nullptr;
        node_layout->yoga_node = yoga_node;
    } else {
        // Clear existing children so we can rebuild the hierarchy to match DOM
        const uint32_t child_count = YGNodeGetChildCount(yoga_node);
        for (uint32_t i = 0; i < child_count; ++i) {
            YGNodeRemoveChild(yoga_node, YGNodeGetChild(yoga_node, 0));
        }
    }

    // Apply DOM styles to Yoga node
    applyDOMStylesToYoga(dom_node, yoga_node);

    // Recursively create or reuse Yoga nodes for children
    const auto& parent_style = dom_node->getComputedStyle();
    const bool parent_is_block_like = (parent_style.layout_mode == dom::LayoutMode::Block);

    dom::DOMNodePtr prev_element_child;
    YGNode* prev_child_yoga = nullptr;

    for (const auto& child : dom_node->getChildren()) {
        if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
            YGNode* child_yoga = createYogaNode(child);
            if (child_yoga) {
                // Approximate vertical margin collapsing between sibling block items
                // for non-flex parents, so that gaps match CSS block layout better.
                if (parent_is_block_like && prev_child_yoga) {
                    collapseVerticalMarginBetweenSiblings(prev_element_child, prev_child_yoga,
                                                          child, child_yoga);
                }

                YGNodeInsertChild(yoga_node, child_yoga, YGNodeGetChildCount(yoga_node));

                auto& child_layout = layout_cache[child.get()];
                if (!child_layout) {
                    child_layout = std::make_unique<LayoutNode>();
                }
                child_layout->yoga_node = child_yoga;

                prev_element_child = child;
                prev_child_yoga = child_yoga;
            }
        }
    }

    return yoga_node;
}

void Engine::applyDOMStylesToYoga(dom::DOMNodePtr dom_node, YGNode* yoga_node) {
    if (!dom_node || !yoga_node) return;

    const auto& style = dom_node->getComputedStyle();
    const std::string& tag = dom_node->getTagName();

    float parent_content_w = 0.0f;
    float parent_content_h = 0.0f;
    if (auto parent = dom_node->getParent()) {
        if (parent->getType() == dom::DOMNode::NodeType::ELEMENT) {
            const auto& ps = parent->getComputedStyle();

            auto resolve_padding_border_content_w = [&](const dom::ComputedStyle& s) -> float {
                // Percent widths for children resolve against the parent's content box.
                // If the parent has an explicit width, derive its content width from box-sizing.
                float pad_h = 0.0f;
                if (s.padding_left.isPixel()) pad_h += s.padding_left.value;
                if (s.padding_right.isPixel()) pad_h += s.padding_right.value;
                const float border_h = (s.border_width > 0.0f ? s.border_width : 0.0f) * 2.0f;

                if (s.width.isPixel()) {
                    float content_w = s.width.value;
                    if (s.box_sizing == "border-box") {
                        content_w -= (pad_h + border_h);
                    }
                    return std::max(content_w, 0.0f);
                }

                // Best-effort: resolve viewport units to pixels. Percent/calc with percent is unknown here.
                auto is_viewport_unit = [](dom::CSSValue::Unit u) {
                    using Unit = dom::CSSValue::Unit;
                    return u == Unit::VW || u == Unit::VH || u == Unit::VMIN || u == Unit::VMAX;
                };
                if (is_viewport_unit(s.width.unit)) {
                    float border_box_w = s.width.resolvePixels(0.0f, root_font_size_, viewport_width_, viewport_height_);
                    float content_w = border_box_w;
                    if (s.box_sizing == "border-box") {
                        content_w -= (pad_h + border_h);
                    }
                    return std::max(content_w, 0.0f);
                }

                return 0.0f;
            };

            auto resolve_padding_border_content_h = [&](const dom::ComputedStyle& s) -> float {
                float pad_v = 0.0f;
                if (s.padding_top.isPixel()) pad_v += s.padding_top.value;
                if (s.padding_bottom.isPixel()) pad_v += s.padding_bottom.value;
                const float border_v = (s.border_width > 0.0f ? s.border_width : 0.0f) * 2.0f;

                if (s.height.isPixel()) {
                    float content_h = s.height.value;
                    if (s.box_sizing == "border-box") {
                        content_h -= (pad_v + border_v);
                    }
                    return std::max(content_h, 0.0f);
                }

                auto is_viewport_unit = [](dom::CSSValue::Unit u) {
                    using Unit = dom::CSSValue::Unit;
                    return u == Unit::VW || u == Unit::VH || u == Unit::VMIN || u == Unit::VMAX;
                };
                if (is_viewport_unit(s.height.unit)) {
                    float border_box_h = s.height.resolvePixels(0.0f, root_font_size_, viewport_width_, viewport_height_);
                    float content_h = border_box_h;
                    if (s.box_sizing == "border-box") {
                        content_h -= (pad_v + border_v);
                    }
                    return std::max(content_h, 0.0f);
                }

                return 0.0f;
            };

            parent_content_w = resolve_padding_border_content_w(ps);
            parent_content_h = resolve_padding_border_content_h(ps);
        }
    }

    mapComputedStylesToYoga(style, yoga_node, parent_content_w, parent_content_h);


    // 濡傛灉褰撳墠鑺傜偣鏄唴鑱旀牸寮忓寲涓婁笅鏂囷紙鍖呭惈 inline/inline-block 瀛愬厓绱狅級锛?
    // 璁剧疆涓?flex-direction: row锛岃 Yoga 鑳芥纭绠楀鍣ㄩ珮搴?
    if (isInlineFormattingContext(dom_node)) {
        YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRow);
        YGNodeStyleSetFlexWrap(yoga_node, YGWrapWrap); // 鍏佽鎹㈣
    }

    bool width_converted_to_max = false;
    float converted_width_value = 0.0f;
    if (style.width.isPercent() && style.width.value >= 100.0f &&
        style.max_width.isPixel() && style.max_width.value > 0.0f) {
        width_converted_to_max = true;
        converted_width_value = style.max_width.value;
        YGNodeStyleSetWidth(yoga_node, converted_width_value);
    }

    // If parent is a row-direction flex container and this node has an explicit width
    // but no custom flex-basis, propagate that width as the flex-basis so Yoga treats it
    // as the main-axis base size. Otherwise, percent widths on flex items shrink to content.
    bool parent_row_flex = false;
    if (auto parent = dom_node->getParent()) {
        if (parent->getType() == dom::DOMNode::NodeType::ELEMENT) {
            const auto& parent_style = parent->getComputedStyle();
            if (parent_style.layout_mode == dom::LayoutMode::Flex) {
                std::string dir = parent_style.flex_direction;
                if (dir.empty()) dir = "row";
                if (dir == "row" || dir == "row-reverse") {
                    parent_row_flex = true;
                }
            }
        }
    }

    bool has_explicit_width = style.width.isPixel() || style.width.isPercent() || width_converted_to_max;
    bool has_custom_flex_basis = style.flex_basis.unit != dom::CSSValue::Unit::AUTO;
    if (parent_row_flex && has_explicit_width && !has_custom_flex_basis) {
        if (width_converted_to_max) {
            YGNodeStyleSetFlexBasis(yoga_node, converted_width_value);
        } else if (style.width.isPixel()) {
            YGNodeStyleSetFlexBasis(yoga_node, style.width.value);
        } else if (style.width.isPercent()) {
            YGNodeStyleSetFlexBasisPercent(yoga_node, style.width.value);
        }
    }

    // 鍩轰簬鐪熷疄瀛椾綋搴﹂噺鐨?intrinsic sizing锛?
    // 濡傛灉鏄枃鏈富瀵肩殑鍧楃骇鍏冪礌锛屼笖楂樺害涓?auto锛屽垯浣跨敤 TextShaper
    // 璁＄畻涓€琛屾枃瀛楃殑琛岄珮锛屽苟灏?padding 涓€璧风撼鍏ユ渶灏忛珮搴︼紝閬垮厤鏂囨湰瀹瑰櫒楂樺害
    // 浠呯敱 padding 鍐冲畾锛屽鑷翠笌娴忚鍣ㄥ樊寮傝繃澶с€?
    // 
    // 绗﹀悎 CSS 鏍囧噯锛氫粎璁剧疆 min-height锛岃瀹瑰櫒鑳藉鏍规嵁瀛愬厓绱犲唴瀹硅嚜鐒舵墿灞曪紝
    // 閬垮厤鏄惧紡閿佸畾 height 瀵艰嚧澶氬瓙鍏冪礌瀹瑰櫒琚帇缂┿€?
    if ((tag == "h1" || tag == "h2" || tag == "h3" || tag == "h4" ||
         tag == "h5" || tag == "h6" || tag == "p" || tag == "button" ||
         tag == "div") &&
        style.height.isAuto()) {
        float intrinsic_h = computeIntrinsicTextHeight(dom_node, parent_content_w);

        
        if (intrinsic_h > 0.0f && intrinsic_h < 10000.0f) {
            YGNodeStyleSetMinHeight(yoga_node, intrinsic_h);
            // 绉婚櫎鏄惧紡 height 璁剧疆锛岃 Yoga 鏍规嵁鍐呭鑷姩璁＄畻楂樺害
            // YGNodeStyleSetHeight(yoga_node, intrinsic_h);
        }
    }
    
    // input 鍏冪礌鏄?replaced element锛岄渶瑕佽缃粯璁ょ殑鍐呭湪灏哄
    // CSS 鏍囧噯锛歩nput 鍏冪礌鏈夐粯璁ょ殑瀹藉害鍜岄珮搴?
    if (tag == "input") {
        // \u68c0\u67e5 input \u7c7b\u578b\uff0ccheckbox/radio \u6709\u56fa\u5b9a\u7684 UA \u6837\u5f0f\u5c3a\u5bf8
        bool is_checkbox_or_radio = false;
        if (dom_node->hasAttribute("type")) {
            std::string input_type = dom_node->getAttribute("type");
            std::transform(input_type.begin(), input_type.end(), input_type.begin(),
                          [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            is_checkbox_or_radio = (input_type == "checkbox" || input_type == "radio");
        }
        
        if (is_checkbox_or_radio) {
            // checkbox/radio \u663e\u5f0f\u8bbe\u7f6e\u4e3a UA CSS \u5c3a\u5bf8 (13x13px)\uff0c\u786e\u4fdd Yoga \u6b63\u786e\u5e03\u5c40
            YGNodeStyleSetWidth(yoga_node, 13.0f);
            YGNodeStyleSetHeight(yoga_node, 13.0f);
        } else {
            // \u8ba1\u7b97 input \u7684\u9ad8\u5ea6\uff1afont-size + padding
            float font_size = style.font_size > 0.0f ? style.font_size : 16.0f;
            float pad_top = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
            float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;
            float input_height = font_size * 1.2f + pad_top + pad_bottom;
            
            // \u5f3a\u5236\u8bbe\u7f6e input \u9ad8\u5ea6\uff0c\u4e0d\u7ba1 style.height \u662f\u4ec0\u4e48
            YGNodeStyleSetHeight(yoga_node, input_height);
            
            // input \u9ed8\u8ba4\u662f block \u7ea7\u522b\uff0c\u5bbd\u5ea6 100%\uff08\u7531 CSS \u8bbe\u7f6e\uff09
            // \u4f46\u5982\u679c\u6ca1\u6709\u663e\u793a\u5bbd\u5ea6\uff0c\u7ed9\u4e00\u4e2a\u9ed8\u8ba4\u6700\u5c0f\u5bbd\u5ea6
            if (style.width.isAuto()) {
                YGNodeStyleSetMinWidth(yoga_node, 150.0f);  // \u9ed8\u8ba4\u6700\u5c0f\u5bbd\u5ea6
            }
        }
    }
    
    // 绗﹀悎 CSS 鏍囧噯锛氫负 button 鍏冪礌璁剧疆鍩轰簬鏂囨湰鍐呭鐨勬渶灏忓搴?
    // 鎸夐挳榛樿鏄?inline-block锛屽搴﹀簲鑷€傖應鍐呭 + padding
    // 杩欑'淇濇寜閽笉浼氥?因?flex 瀹瑰櫒鍘嬬缉鑰屾埅鏂枃瀛?
    if (tag == "button" && style.width.isAuto()) {
        float intrinsic_w = computeIntrinsicTextWidth(dom_node);
        if (intrinsic_w > 0.0f && intrinsic_w < 10000.0f) {
            YGNodeStyleSetMinWidth(yoga_node, intrinsic_w);
        }
        if (std::getenv("DONG_DEBUG_BUTTON_WRAP")) {
            std::string text = collapseWhitespace(dom_node->getTextContent());
            DONG_LOG_INFO("[LayoutEngine] button intrinsic_w=%.2f text='%s' display=%s layout_mode=%d width_auto=%d",
                          intrinsic_w,
                          text.c_str(),
                          style.display.c_str(),
                          static_cast<int>(style.layout_mode),
                          style.width.isAuto() ? 1 : 0);
        }
    }


    // Flex items that are inline/inline-block often contain text, but Yoga tree excludes TEXT nodes.
    // Without a best-effort intrinsic min-size, these items collapse to padding/border only, and
    // their painted text will overlap siblings.
    bool parent_is_flex = false;
    if (auto parent = dom_node->getParent()) {
        if (parent->getType() == dom::DOMNode::NodeType::ELEMENT) {
            parent_is_flex = (parent->getComputedStyle().layout_mode == dom::LayoutMode::Flex);
        }
    }

    const bool is_inline_level = (style.display == "inline" || style.display == "inline-block");
    // 为所有包含文本的 inline 元素设置最小宽度，防止被压缩导致文本截断
    if (is_inline_level && (style.width.isAuto() || style.height.isAuto())) {
        std::string text = collapseWhitespace(dom_node->getTextContent());
        if (!text.empty()) {
            float font_size_px = style.font_size > 0.0f ? style.font_size : 16.0f;
            if (!std::isfinite(font_size_px) || font_size_px <= 0.0f) {
                font_size_px = 16.0f;
            }

            TextShaper shaper;
            TextShapeRequest req{};
            req.text = text;
            req.font_family = style.font_family;
            req.font_weight = style.font_weight;
            req.font_style = style.font_style;
            req.font_size = font_size_px;

            ShapedText shaped{};
            if (shaper.shape(req, shaped) && !shaped.glyphs.empty() &&
                std::isfinite(shaped.scale_to_pixels) && shaped.scale_to_pixels > 0.0f) {

                const float scale = shaped.scale_to_pixels;

                float min_x_units = shaped.glyphs.front().pen_x_units;
                float max_x_units = shaped.glyphs.front().pen_x_units + shaped.glyphs.front().advance_x_units;
                for (const auto& g : shaped.glyphs) {
                    min_x_units = std::min(min_x_units, g.pen_x_units);
                    max_x_units = std::max(max_x_units, g.pen_x_units + g.advance_x_units);
                }

                float content_width_px = (max_x_units - min_x_units) * scale;
                if (content_width_px < 0.0f || !std::isfinite(content_width_px)) {
                    content_width_px = shaped.width_units * scale;
                }
                if (content_width_px < 0.0f) content_width_px = 0.0f;

                if (style.letter_spacing_em != 0.0f) {
                    const float letter_spacing_px = style.letter_spacing_em * font_size_px;
                    const int glyph_count = static_cast<int>(shaped.glyphs.size());
                    if (glyph_count > 1) {
                        content_width_px += letter_spacing_px * static_cast<float>(glyph_count - 1);
                    }
                }

                if (style.word_spacing_px != 0.0f) {
                    int space_count = 0;
                    for (const auto& g : shaped.glyphs) {
                        if (g.cluster < text.size() && text[g.cluster] == ' ') {
                            ++space_count;
                        }
                    }
                    int effective_spaces = space_count;
                    if (!shaped.glyphs.empty()) {
                        const auto& g_last = shaped.glyphs.back();
                        if (g_last.cluster < text.size() && text[g_last.cluster] == ' ') {
                            effective_spaces = std::max(0, effective_spaces - 1);
                        }
                    }
                    if (effective_spaces > 0) {
                        content_width_px += style.word_spacing_px * static_cast<float>(effective_spaces);
                    }
                }

                const float pad_l = style.padding_left.isPixel() ? style.padding_left.value : 0.0f;
                const float pad_r = style.padding_right.isPixel() ? style.padding_right.value : 0.0f;
                const float pad_t = style.padding_top.isPixel() ? style.padding_top.value : 0.0f;
                const float pad_b = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;
                const float border_w = style.border_width > 0.0f ? style.border_width : 0.0f;

                if (style.width.isAuto()) {
                    const float min_w = content_width_px + pad_l + pad_r + border_w * 2.0f;
                    if (min_w > 0.0f && min_w < 100000.0f && std::isfinite(min_w)) {
                        YGNodeStyleSetMinWidth(yoga_node, min_w);
                    }
                }

                if (style.height.isAuto()) {
                    float line_height_units = shaped.line_height_units;
                    if (style.line_height > 0.0f) {
                        if (style.line_height_is_unitless) {
                            float css_line_height_px = style.line_height * font_size_px;
                            line_height_units = css_line_height_px / std::max(scale, 1e-3f);
                        } else {
                            line_height_units = style.line_height / std::max(scale, 1e-3f);
                        }
                    }
                    if (line_height_units <= 0.0f || !std::isfinite(line_height_units)) {
                        line_height_units = font_size_px / std::max(scale, 1e-3f);
                    }
                    const float min_line_height_px = font_size_px * 1.2f;
                    float line_height_px = line_height_units * scale;
                    if (line_height_px < min_line_height_px) {
                        line_height_px = min_line_height_px;
                    }

                    const float min_h = line_height_px + pad_t + pad_b + border_w * 2.0f;
                    if (min_h > 0.0f && min_h < 100000.0f && std::isfinite(min_h)) {
                        YGNodeStyleSetMinHeight(yoga_node, min_h);
                    }
                }
            }
        }
    }
}


void Engine::mapComputedStylesToYoga(const dom::ComputedStyle& style, YGNode* yoga_node,
                                     float parent_content_width_px, float parent_content_height_px) {
    if (!yoga_node) return;


    // Set display based on layout mode
    switch (style.layout_mode) {
        case dom::LayoutMode::None:
            YGNodeStyleSetDisplay(yoga_node, YGDisplayNone);
            break;
        case dom::LayoutMode::Block:
        case dom::LayoutMode::Inline:
        case dom::LayoutMode::Flex:
        default:
            YGNodeStyleSetDisplay(yoga_node, YGDisplayFlex);
            break;
    }

    // Set flex direction
    // For flex layout, respect flex_direction; for block/inline, approximate as vertical stack
    if (style.layout_mode == dom::LayoutMode::Flex) {
        if (style.flex_direction == "column") {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionColumn);
        } else if (style.flex_direction == "column-reverse") {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionColumnReverse);
        } else if (style.flex_direction == "row-reverse") {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRowReverse);
        } else {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRow);
        }

        // Flex wrap
        if (style.flex_wrap == "wrap") {
            YGNodeStyleSetFlexWrap(yoga_node, YGWrapWrap);
        } else if (style.flex_wrap == "wrap-reverse") {
            YGNodeStyleSetFlexWrap(yoga_node, YGWrapWrapReverse);
        } else {
            YGNodeStyleSetFlexWrap(yoga_node, YGWrapNoWrap);
        }
    } else {

        // Approximate block/inline layout as a vertical stack
        YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionColumn);
        
        // 浣嗗鏋滄槸 inline-block 鍏冪礌锛岃缃负 row 鏂瑰悜锛岃 Yoga 鑳芥纭绠楀叾灏哄
        if (style.display == "inline-block") {
            YGNodeStyleSetFlexDirection(yoga_node, YGFlexDirectionRow);
        }
    }

    // Set justify content
    if (style.justify_content == "flex-end") {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifyFlexEnd);
    } else if (style.justify_content == "center") {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifyCenter);
    } else if (style.justify_content == "space-between") {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifySpaceBetween);
    } else if (style.justify_content == "space-around") {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifySpaceAround);
    } else {
        YGNodeStyleSetJustifyContent(yoga_node, YGJustifyFlexStart);
    }

    // Set align items
    // CSS initial value is `stretch`.
    // Use `stretch` as fallback for empty/unknown values so block layout (column) also fills width.
    if (style.align_items == "flex-start" || style.align_items == "start") {
        YGNodeStyleSetAlignItems(yoga_node, YGAlignFlexStart);
    } else if (style.align_items == "flex-end" || style.align_items == "end") {
        YGNodeStyleSetAlignItems(yoga_node, YGAlignFlexEnd);
    } else if (style.align_items == "center") {
        YGNodeStyleSetAlignItems(yoga_node, YGAlignCenter);
    } else if (style.align_items == "baseline") {
        YGNodeStyleSetAlignItems(yoga_node, YGAlignBaseline);
    } else {
        // stretch / normal / empty / unknown
        YGNodeStyleSetAlignItems(yoga_node, YGAlignStretch);
    }


    // Set gap for flex containers
    if (style.gap > 0.0f) {
        YGNodeStyleSetGap(yoga_node, YGGutterAll, style.gap);
    }
    
    // Set overflow for scroll containers
    // When overflow is hidden/scroll/auto, Yoga should not expand to fit content
    if (style.overflow == "scroll") {
        YGNodeStyleSetOverflow(yoga_node, YGOverflowScroll);
    } else if (style.overflow == "hidden" || style.overflow == "auto") {
        YGNodeStyleSetOverflow(yoga_node, YGOverflowHidden);
    } else {
        YGNodeStyleSetOverflow(yoga_node, YGOverflowVisible);
    }

    // Set position type and offsets (relative/absolute)
    if (style.position == "absolute") {
        YGNodeStyleSetPositionType(yoga_node, YGPositionTypeAbsolute);
    } else {
        // static/relative/fixed 鐩墠缁熶竴鏄犲皠涓?Yoga 鐨?relative锛?
        // 浣嗛€氳繃浣嶇疆灞炴€э紙top/right/bottom/left锛夊疄鐜?relative 鍋忕Щ銆?
        YGNodeStyleSetPositionType(yoga_node, YGPositionTypeRelative);
    }

    auto setPositionIfNeeded = [&](YGEdge edge, const dom::CSSValue& v) {
        using Unit = dom::CSSValue::Unit;
        if (v.unit == Unit::PIXEL) {
            YGNodeStyleSetPosition(yoga_node, edge, v.value);
        } else if (v.unit == Unit::PERCENT) {
            YGNodeStyleSetPositionPercent(yoga_node, edge, v.value);
        }
    };

    setPositionIfNeeded(YGEdgeTop, style.top);
    setPositionIfNeeded(YGEdgeRight, style.right);
    setPositionIfNeeded(YGEdgeBottom, style.bottom);
    setPositionIfNeeded(YGEdgeLeft, style.left);

    // Set dimensions
    // Note: Yoga uses border-box model internally. When CSS box-sizing is content-box,
    // we need to add padding and border to the specified width/height.
    bool has_explicit_width = false;
    bool has_explicit_height = false;

    // Calculate padding and border for content-box adjustment
    float pad_h = 0.0f; // horizontal padding (left + right)
    float pad_v = 0.0f; // vertical padding (top + bottom)
    float border_h = 0.0f; // horizontal border (left + right)
    float border_v = 0.0f; // vertical border (top + bottom)
    
    auto effective_border_width = [&](float side_width, const std::string& side_style) -> float {
        float w = (side_width >= 0.0f) ? side_width : style.border_width;
        if (w < 0.0f) w = 0.0f;
        const std::string& st = !side_style.empty() ? side_style : style.border_style;
        if (st == "none" || st == "hidden") return 0.0f;
        return w;
    };

    if (style.box_sizing == "content-box") {
        if (style.padding_left.isPixel()) pad_h += style.padding_left.value;
        if (style.padding_right.isPixel()) pad_h += style.padding_right.value;
        if (style.padding_top.isPixel()) pad_v += style.padding_top.value;
        if (style.padding_bottom.isPixel()) pad_v += style.padding_bottom.value;

        const float bl = effective_border_width(style.border_left_width, style.border_left_style);
        const float br = effective_border_width(style.border_right_width, style.border_right_style);
        const float bt = effective_border_width(style.border_top_width, style.border_top_style);
        const float bb = effective_border_width(style.border_bottom_width, style.border_bottom_style);
        border_h = bl + br;
        border_v = bt + bb;
    }


    auto is_viewport_unit = [](dom::CSSValue::Unit u) {
        using Unit = dom::CSSValue::Unit;
        return u == Unit::VW || u == Unit::VH || u == Unit::VMIN || u == Unit::VMAX;
    };

    auto resolve_viewport_px = [this](const dom::CSSValue& v) {
        // Best-effort: resolve only viewport-related units here.
        return v.resolvePixels(0.0f, root_font_size_, viewport_width_, viewport_height_);
    };

    auto resolve_length_px_for_layout = [&](const dom::CSSValue& v, float percent_base_px) -> float {
        using Unit = dom::CSSValue::Unit;
        if (v.unit == Unit::PIXEL) return v.value;
        if (v.unit == Unit::PERCENT) return percent_base_px * v.value / 100.0f;
        if (is_viewport_unit(v.unit)) return resolve_viewport_px(v);
        if (v.unit == Unit::CALC) {
            // Best-effort: treat parent_size as the percent base (works for calc(% +/- px)).
            return v.resolvePixels(percent_base_px, root_font_size_, viewport_width_, viewport_height_);
        }
        return 0.0f;
    };

    if (style.width.isPixel()) {
        float width_for_yoga = style.width.value;
        if (style.box_sizing == "content-box") {
            // content-box: CSS width is content width, add padding and border for Yoga
            width_for_yoga += pad_h + border_h;
        }
        YGNodeStyleSetWidth(yoga_node, width_for_yoga);
        has_explicit_width = true;
    } else if (style.width.isPercent()) {
        YGNodeStyleSetWidthPercent(yoga_node, style.width.value);
        has_explicit_width = true;
    } else if (is_viewport_unit(style.width.unit)) {
        float width_for_yoga = resolve_viewport_px(style.width);
        if (style.box_sizing == "content-box") {
            width_for_yoga += pad_h + border_h;
        }
        if (width_for_yoga > 0.0f && std::isfinite(width_for_yoga)) {
            YGNodeStyleSetWidth(yoga_node, width_for_yoga);
            has_explicit_width = true;
        }
    } else if (style.width.isCalc()) {
        float width_px = resolve_length_px_for_layout(style.width, parent_content_width_px);
        if (style.box_sizing == "content-box") {
            width_px += pad_h + border_h;
        }
        if (width_px < 0.0f) width_px = 0.0f;
        if (width_px > 0.0f && std::isfinite(width_px)) {
            YGNodeStyleSetWidth(yoga_node, width_px);
            has_explicit_width = true;
        }
    }


    if (style.height.isPixel()) {
        float height_for_yoga = style.height.value;
        if (style.box_sizing == "content-box") {
            // content-box: CSS height is content height, add padding and border for Yoga
            height_for_yoga += pad_v + border_v;
        }
        YGNodeStyleSetHeight(yoga_node, height_for_yoga);
        has_explicit_height = true;
        
        // 绗﹀悎 CSS 鏍囧噯锛氱Щ闄?overflow 瀹瑰櫒鐨?max-height 闄愬埗
        // 璁╁唴瀹规牴鎹?CSS height 灞炴€ц嚜鐒跺竷灞€锛岄伩鍏嶉潪鏍囧噯鍘嬬缉
        // if (style.overflow == "scroll" || style.overflow == "hidden" || style.overflow == "auto") {
        //     YGNodeStyleSetMaxHeight(yoga_node, height_for_yoga);
        // }
    } else if (style.height.isPercent()) {
        YGNodeStyleSetHeightPercent(yoga_node, style.height.value);
        has_explicit_height = true;
    } else if (is_viewport_unit(style.height.unit)) {
        float height_for_yoga = resolve_viewport_px(style.height);
        if (style.box_sizing == "content-box") {
            height_for_yoga += pad_v + border_v;
        }
        if (height_for_yoga > 0.0f && std::isfinite(height_for_yoga)) {
            YGNodeStyleSetHeight(yoga_node, height_for_yoga);
            has_explicit_height = true;
        }
    } else if (style.height.isCalc()) {
        float height_px = resolve_length_px_for_layout(style.height, parent_content_height_px);
        if (style.box_sizing == "content-box") {
            height_px += pad_v + border_v;
        }
        if (height_px < 0.0f) height_px = 0.0f;
        if (height_px > 0.0f && std::isfinite(height_px)) {
            YGNodeStyleSetHeight(yoga_node, height_px);
            has_explicit_height = true;
        }
    }


    // Fallback: for block-level elements without an explicit width,
    // stretch to full available width. This approximates HTML block layout
    // when we don't have intrinsic content measurement.
    //
    // IMPORTANT: Do not apply this fallback to:
    // - position:absolute elements (should use intrinsic/shrink-to-fit sizing)
    // - inline-block elements (should size to content, not stretch to 100%)
    // Otherwise Yoga will give them a 100% width box, which breaks the
    // expected CSS semantics.
    const bool is_inline_block = (style.display == "inline-block");
    if (!has_explicit_width &&
        style.layout_mode == dom::LayoutMode::Block &&
        style.position != "absolute" &&
        !is_inline_block) {
        YGNodeStyleSetWidthPercent(yoga_node, 100.0f);
    }

    // Set margins
    // Note: margin: auto is handled differently depending on context:
    // - In flex containers: Yoga's YGNodeStyleSetMarginAuto handles it correctly
    // - In block formatting context: layoutBlockFormattingContext() handles it after Yoga layout
    if (style.margin_top.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeTop, style.margin_top.value);
    } else if (style.margin_top.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeTop, style.margin_top.value);
    } else if (style.margin_top.isAuto()) {
        // margin-top: auto - in flex context, this enables vertical centering
        YGNodeStyleSetMarginAuto(yoga_node, YGEdgeTop);
    }

    if (style.margin_right.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeRight, style.margin_right.value);
    } else if (style.margin_right.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeRight, style.margin_right.value);
    } else if (style.margin_right.isAuto()) {
        // margin-right: auto - in flex context, pushes element to the left
        YGNodeStyleSetMarginAuto(yoga_node, YGEdgeRight);
    }

    if (style.margin_bottom.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeBottom, style.margin_bottom.value);
    } else if (style.margin_bottom.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeBottom, style.margin_bottom.value);
    } else if (style.margin_bottom.isAuto()) {
        // margin-bottom: auto - in flex context, this enables vertical centering
        YGNodeStyleSetMarginAuto(yoga_node, YGEdgeBottom);
    }

    if (style.margin_left.isPixel()) {
        YGNodeStyleSetMargin(yoga_node, YGEdgeLeft, style.margin_left.value);
    } else if (style.margin_left.isPercent()) {
        YGNodeStyleSetMarginPercent(yoga_node, YGEdgeLeft, style.margin_left.value);
    } else if (style.margin_left.isAuto()) {
        // margin-left: auto - in flex context, pushes element to the right
        YGNodeStyleSetMarginAuto(yoga_node, YGEdgeLeft);
    }

    // Set padding
    if (style.padding_top.isPixel()) {
        YGNodeStyleSetPadding(yoga_node, YGEdgeTop, style.padding_top.value);
    } else if (style.padding_top.isPercent()) {
        YGNodeStyleSetPaddingPercent(yoga_node, YGEdgeTop, style.padding_top.value);
    }
    if (style.padding_right.isPixel()) {
        YGNodeStyleSetPadding(yoga_node, YGEdgeRight, style.padding_right.value);
    } else if (style.padding_right.isPercent()) {
        YGNodeStyleSetPaddingPercent(yoga_node, YGEdgeRight, style.padding_right.value);
    }
    if (style.padding_bottom.isPixel()) {
        YGNodeStyleSetPadding(yoga_node, YGEdgeBottom, style.padding_bottom.value);
    } else if (style.padding_bottom.isPercent()) {
        YGNodeStyleSetPaddingPercent(yoga_node, YGEdgeBottom, style.padding_bottom.value);
    }
    if (style.padding_left.isPixel()) {
        YGNodeStyleSetPadding(yoga_node, YGEdgeLeft, style.padding_left.value);
    } else if (style.padding_left.isPercent()) {
        YGNodeStyleSetPaddingPercent(yoga_node, YGEdgeLeft, style.padding_left.value);
    }

    // Set flex grow/shrink
    float flex_grow = style.flex_grow;
    float flex_shrink = style.flex_shrink;
    dom::CSSValue flex_basis = style.flex_basis;

    // Support `flex: <number>` shorthand as flex-grow with a 0 basis
    if (style.flex != 0.0f) {
        if (flex_grow == 0.0f) {
            flex_grow = style.flex;
        }
        // Keep existing shrink unless explicitly overridden
        if (flex_shrink == 1.0f) {
            flex_shrink = 1.0f;
        }
        // When flex shorthand is used and flex-basis is auto, default to 0
        if (flex_basis.unit == dom::CSSValue::Unit::AUTO) {
            flex_basis = dom::CSSValue(0.0f, dom::CSSValue::Unit::PIXEL);
        }
    }

    YGNodeStyleSetFlexGrow(yoga_node, flex_grow);
    YGNodeStyleSetFlexShrink(yoga_node, flex_shrink);
    
    // For flex containers without explicit height, prevent shrinking to 0
    // by setting flex-shrink to 0 when they are children of column flex containers
    if (style.layout_mode == dom::LayoutMode::Flex && style.height.isAuto()) {
        YGNodeStyleSetFlexShrink(yoga_node, 0.0f);
    }
    
    // 绗﹀悎 CSS Flexbox 鏍囧噯锛?
    // 褰撳厓绱犳湁鏄惧紡 height 鏃讹紝鍦?column 鏂瑰悜鐨?flex 瀹瑰櫒涓簲璇ュ皧閲嶈楂樺害
    // 閫氳繃璁剧疆 flex-basis 涓烘樉寮忛珮搴﹀€硷紝骞剁姝㈡敹缂╂潵瀹炵幇
    if (style.height.isPixel()) {
        // 鏈夋樉寮忓儚绱犻珮搴︾殑鍏冪礌锛岃缃?flex-shrink: 0 闃叉琚帇缂?
        // 杩欑鍚?CSS 鏍囧噯锛氭樉寮忓昂瀵稿簲璇ヨ灏婇噸
        YGNodeStyleSetFlexShrink(yoga_node, 0.0f);
    }
    
    if (flex_basis.isPixel()) {
        YGNodeStyleSetFlexBasis(yoga_node, flex_basis.value);
    } else if (flex_basis.isPercent()) {
        YGNodeStyleSetFlexBasisPercent(yoga_node, flex_basis.value);
    } else {
        // flex-basis: auto 鏃讹紝Yoga 浼氫娇鐢?width/height 灞炴€т綔涓哄熀鍑?
        // 杩欐槸绗﹀悎 CSS 鏍囧噯鐨勮涓?
        YGNodeStyleSetFlexBasisAuto(yoga_node);
    }

    // Map border width into Yoga so that border participates in box model sizing
    {
        const float bl = effective_border_width(style.border_left_width, style.border_left_style);
        const float br = effective_border_width(style.border_right_width, style.border_right_style);
        const float bt = effective_border_width(style.border_top_width, style.border_top_style);
        const float bb = effective_border_width(style.border_bottom_width, style.border_bottom_style);

        if (bl > 0.0f) YGNodeStyleSetBorder(yoga_node, YGEdgeLeft, bl);
        if (bt > 0.0f) YGNodeStyleSetBorder(yoga_node, YGEdgeTop, bt);
        if (br > 0.0f) YGNodeStyleSetBorder(yoga_node, YGEdgeRight, br);
        if (bb > 0.0f) YGNodeStyleSetBorder(yoga_node, YGEdgeBottom, bb);
    }


    // Set min/max width and height
    // Note: min/max constraints also need box-sizing adjustment for content-box
    if (style.min_width.isPixel()) {
        float min_w = style.min_width.value;
        if (style.box_sizing == "content-box") {
            min_w += pad_h + border_h;
        }
        YGNodeStyleSetMinWidth(yoga_node, min_w);
    } else if (style.min_width.isPercent()) {
        YGNodeStyleSetMinWidthPercent(yoga_node, style.min_width.value);
    }
    if (style.max_width.isPixel()) {
        float max_w = style.max_width.value;
        if (style.box_sizing == "content-box") {
            max_w += pad_h + border_h;
        }
        YGNodeStyleSetMaxWidth(yoga_node, max_w);
    } else if (style.max_width.isPercent()) {
        YGNodeStyleSetMaxWidthPercent(yoga_node, style.max_width.value);
    }
    if (style.min_height.isPixel()) {
        float min_h = style.min_height.value;
        if (style.box_sizing == "content-box") {
            min_h += pad_v + border_v;
        }
        YGNodeStyleSetMinHeight(yoga_node, min_h);
    } else if (style.min_height.isPercent()) {
        YGNodeStyleSetMinHeightPercent(yoga_node, style.min_height.value);
    }
    if (style.max_height.isPixel()) {
        float max_h = style.max_height.value;
        if (style.box_sizing == "content-box") {
            max_h += pad_v + border_v;
        }
        YGNodeStyleSetMaxHeight(yoga_node, max_h);
    } else if (style.max_height.isPercent()) {
        YGNodeStyleSetMaxHeightPercent(yoga_node, style.max_height.value);
    }
}

float Engine::parsePixelValue(const dom::CSSValue& value, float parent_size) {
    switch (value.unit) {
        case dom::CSSValue::Unit::PIXEL:
            return value.value;
        case dom::CSSValue::Unit::PERCENT:
            // If given as percent but we need pixels, use 0 as fallback
            // (caller should use parsePercentValue for percentages)
            return 0.0f;
        case dom::CSSValue::Unit::AUTO:
        case dom::CSSValue::Unit::INHERIT:
            return 0.0f;
        default:
            return 0.0f;
    }
}

float Engine::parsePercentValue(const dom::CSSValue& value, float parent_size) {
    if (value.unit == dom::CSSValue::Unit::PERCENT && parent_size > 0) {
        return (value.value / 100.0f) * parent_size;
    }
    return 0.0f;
}

void Engine::destroyYogaNode(YGNode* node) {
    if (node) {
        YGNodeFree(node);
    }
}

// ============================================================================
// Block Formatting Context: margin: auto handling
// ============================================================================
//
// CSS 2.1 搂10.3.3: Block-level, non-replaced elements in normal flow
//
// The constraint equation for horizontal layout:
//   margin-left + border-left + padding-left + width + padding-right + border-right + margin-right = containing block width
//
// When margin-left and margin-right are both 'auto':
//   - If width is not 'auto': margins are equal, centering the element
//   - If width is 'auto': margins compute to 0
//
// When only margin-left is 'auto':
//   - Element is right-aligned within containing block
//
// When only margin-right is 'auto':
//   - Element is left-aligned (default behavior, no adjustment needed)
//
// Note: margin-top: auto and margin-bottom: auto compute to 0 in block formatting context
// (they only have special meaning in flex/grid contexts)
//
void Engine::layoutBlockFormattingContext(dom::DOMNodePtr root) {
    if (!root) {
        return;
    }

    std::function<void(const dom::DOMNodePtr&, const dom::DOMNodePtr&)> walk;
    walk = [this, &walk](const dom::DOMNodePtr& node, const dom::DOMNodePtr& parent) {
        if (!node) {
            return;
        }

        const auto& style = node->getComputedStyle();

        // Only process block-level elements in normal flow
        // Skip: display:none, position:absolute/fixed, inline/inline-block, flex items
        const bool is_block_in_flow =
            style.layout_mode == dom::LayoutMode::Block &&
            style.position != "absolute" &&
            style.position != "fixed" &&
            style.display != "inline" &&
            style.display != "inline-block";

        if (is_block_in_flow && parent) {
            const bool margin_left_auto = style.margin_left.isAuto();
            const bool margin_right_auto = style.margin_right.isAuto();

            // Check if parent is a flex container - margin:auto has different semantics in flex
            const auto& parent_style = parent->getComputedStyle();
            const bool parent_is_flex = (parent_style.layout_mode == dom::LayoutMode::Flex);

            // Only apply block formatting context margin:auto if parent is NOT flex
            // (Yoga handles margin:auto correctly for flex items)
            if (!parent_is_flex && (margin_left_auto || margin_right_auto)) {
                auto it_layout = layout_cache.find(node.get());
                auto it_parent_layout = layout_cache.find(parent.get());

                if (it_layout != layout_cache.end() && it_layout->second &&
                    it_parent_layout != layout_cache.end() && it_parent_layout->second) {
                    
                    LayoutNode* layout = it_layout->second.get();
                    LayoutNode* parent_layout = it_parent_layout->second.get();

                    // Get containing block's content width (parent width minus its padding)
                    float parent_pad_left = parent_style.padding_left.isPixel() ? parent_style.padding_left.value : 0.0f;
                    float parent_pad_right = parent_style.padding_right.isPixel() ? parent_style.padding_right.value : 0.0f;
                    float containing_block_width = parent_layout->width - parent_pad_left - parent_pad_right;

                    // Element's box width (already computed by Yoga, includes border/padding if box-sizing: border-box)
                    float element_width = layout->width;

                    // Get explicit margin values (non-auto margins)
                    float margin_left_px = 0.0f;
                    float margin_right_px = 0.0f;
                    if (!margin_left_auto && style.margin_left.isPixel()) {
                        margin_left_px = style.margin_left.value;
                    } else if (!margin_left_auto && style.margin_left.isPercent()) {
                        margin_left_px = parsePercentValue(style.margin_left, containing_block_width);
                    }
                    if (!margin_right_auto && style.margin_right.isPixel()) {
                        margin_right_px = style.margin_right.value;
                    } else if (!margin_right_auto && style.margin_right.isPercent()) {
                        margin_right_px = parsePercentValue(style.margin_right, containing_block_width);
                    }

                    // Calculate remaining space
                    float remaining_space = containing_block_width - element_width - margin_left_px - margin_right_px;
                    if (remaining_space < 0.0f) {
                        remaining_space = 0.0f;
                    }

                    // Calculate new X position based on margin:auto rules
                    float new_x = layout->x;
                    float content_start_x = parent_layout->x + parent_pad_left;

                    if (margin_left_auto && margin_right_auto) {
                        // Both auto: center the element
                        // margin-left = margin-right = remaining_space / 2
                        float auto_margin = remaining_space / 2.0f;
                        new_x = content_start_x + auto_margin;
                    } else if (margin_left_auto && !margin_right_auto) {
                        // Only margin-left is auto: right-align
                        // margin-left = remaining_space
                        new_x = content_start_x + remaining_space;
                    } else if (!margin_left_auto && margin_right_auto) {
                        // Only margin-right is auto: left-align (default)
                        // margin-right absorbs remaining space, element stays at left
                        new_x = content_start_x + margin_left_px;
                    }

                    // Apply the new position if changed
                    if (layout->x != new_x) {
                        float old_x = layout->x;
                        layout->x = new_x;
                        layout->layout.position[0] = new_x;

                        // Mark dirty region
                        dirty_rect_.expand(old_x, layout->y, layout->width, layout->height);
                        dirty_rect_.expand(new_x, layout->y, layout->width, layout->height);

                        // Recursively update all descendant positions
                        // Since we changed this node's X, all children need their absolute X updated
                        float delta_x = new_x - old_x;
                        std::function<void(const dom::DOMNodePtr&)> updateChildPositions;
                        updateChildPositions = [this, delta_x, &updateChildPositions](const dom::DOMNodePtr& child_node) {
                            if (!child_node) return;
                            for (const auto& grandchild : child_node->getChildren()) {
                                if (!grandchild || grandchild->getType() != dom::DOMNode::NodeType::ELEMENT) {
                                    continue;
                                }
                                auto it = layout_cache.find(grandchild.get());
                                if (it != layout_cache.end() && it->second) {
                                    LayoutNode* child_layout = it->second.get();
                                    child_layout->x += delta_x;
                                    child_layout->layout.position[0] = child_layout->x;
                                    dirty_rect_.expand(child_layout->x, child_layout->y,
                                                       child_layout->width, child_layout->height);
                                }
                                updateChildPositions(grandchild);
                            }
                        };
                        updateChildPositions(node);
                    }
                }
            }
        }

        // Recursively process children
        for (const auto& child : node->getChildren()) {
            if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                walk(child, node);
            }
        }
    };

    // Start from root with no parent
    walk(root, nullptr);
}

void Engine::layoutInlineFormattingContexts(dom::DOMNodePtr root) {
    if (!root) {
        return;
    }

    std::function<void(const dom::DOMNodePtr&)> walk;
    walk = [this, &walk](const dom::DOMNodePtr& node) {
        if (!node) {
            return;
        }

        if (isInlineFormattingContext(node)) {
            auto it_container = layout_cache.find(node.get());
            if (it_container != layout_cache.end() && it_container->second) {
                LayoutNode* container_layout = it_container->second.get();
                const auto& container_style = node->getComputedStyle();

                float container_x = container_layout->x;
                float container_y = container_layout->y;
                float container_w = container_layout->width;

                float pad_left = container_style.padding_left.isPixel() ? container_style.padding_left.value : 0.0f;
                float pad_right = container_style.padding_right.isPixel() ? container_style.padding_right.value : 0.0f;
                float pad_top = container_style.padding_top.isPixel() ? container_style.padding_top.value : 0.0f;

                float content_x = container_x + pad_left;
                float content_y = container_y + pad_top;
                float content_w = container_w - pad_left - pad_right;
                if (content_w <= 0.0f) {
                    content_w = container_w;
                }

                InlineMetrics container_metrics{};
                bool has_container_text_metrics = computeInlineMetricsForNode(node, container_metrics, container_style.font_size);
                float container_baseline_from_border_top = 0.0f;
                float container_line_height_px = 0.0f;
                if (has_container_text_metrics) {
                    float border_w_container = container_style.border_width;
                    if (border_w_container < 0.0f) {
                        border_w_container = 0.0f;
                    }
                    container_line_height_px = container_metrics.line_height_px;
                    container_baseline_from_border_top =
                        border_w_container +
                        (container_style.padding_top.isPixel() ? container_style.padding_top.value : 0.0f) +
                        container_metrics.baseline_from_content_top_px;
                }

                struct InlineItem {
                    dom::DOMNodePtr node;
                    float margin_left = 0.0f;
                    float margin_right = 0.0f;
                    float preferred_width = 0.0f;
                    float preferred_height = 0.0f;
                    float baseline_from_border_top = 0.0f;
                    float line_height_px = 0.0f;
                    float offset_x_in_content = 0.0f;
                    std::string vertical_align = "baseline"; // baseline, top, middle, bottom
                };

                std::vector<InlineItem> items;
                items.reserve(node->getChildren().size());

                for (const auto& child : node->getChildren()) {
                    if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                        continue;
                    }

                    const auto& child_style = child->getComputedStyle();
                    
                    if (child_style.position == "absolute") {
                        // 缁濆瀹氫綅鍏冪礌涓嶅弬涓庡唴鑱旀牸寮忓寲涓婁笅鏂囷紝鐢变笓闂ㄧ殑瀹氫綅甯冨眬闃舵澶勭悊
                        continue;
                    }
                    if (!isInlineLevelDisplay(child_style.display)) {
                        continue;
                    }

                    InlineMetrics metrics{};
                    bool has_text_metrics = computeInlineMetricsForNode(child, metrics, container_style.font_size);

                    // 鍗充娇娌℃湁鏂囨湰搴﹂噺锛屽彧瑕佹湁鏄惧紡 width/height锛屼篃搴旇鍙備笌甯冨眬
                    bool has_explicit_width = child_style.width.isPixel() || child_style.width.isPercent();
                    bool has_explicit_height = child_style.height.isPixel() || child_style.height.isPercent();
                    bool has_min_width = child_style.min_width.isPixel() || child_style.min_width.isPercent();
                    bool has_min_height = child_style.min_height.isPixel() || child_style.min_height.isPercent();
                    
                    if (!has_text_metrics && !has_explicit_width && !has_explicit_height && !has_min_width && !has_min_height) {
                        // 鏃㈡病鏈夋枃鏈唴瀹癸紝涔熸病鏈夋樉寮忓昂瀵革紝璺宠繃
                        continue;
                    }

                    InlineItem item{};
                    item.node = child;

                    item.margin_left = parsePixelValue(child_style.margin_left, content_w);
                    item.margin_right = parsePixelValue(child_style.margin_right, content_w);

                    float pad_l = child_style.padding_left.isPixel() ? child_style.padding_left.value : 0.0f;
                    float pad_r = child_style.padding_right.isPixel() ? child_style.padding_right.value : 0.0f;
                    float pad_t = child_style.padding_top.isPixel() ? child_style.padding_top.value : 0.0f;
                    float pad_b = child_style.padding_bottom.isPixel() ? child_style.padding_bottom.value : 0.0f;
                    float border_w = child_style.border_width;
                    if (border_w < 0.0f) {
                        border_w = 0.0f;
                    }

                    // 璁＄畻瀹藉害锛氫紭鍏堜娇鐢?CSS 鏄惧紡鎸囧畾鐨勫搴?
                    float width_px = 0.0f;
                    if (child_style.width.isPixel()) {
                        width_px = child_style.width.value;
                    } else if (child_style.width.isPercent()) {
                        width_px = parsePercentValue(child_style.width, content_w);
                    } else if (has_text_metrics) {
                        // 绗﹀悎 CSS 鏍囧噯锛氫娇鐢ㄥ寘鍚?padding 鐨勫畬鏁村搴?+ border
                        width_px = metrics.total_width_px + border_w * 2.0f;
                    }
                    if (width_px <= 0.0f && has_text_metrics) {
                        width_px = metrics.line_height_px;
                    }
                    if (width_px <= 0.0f) {
                        width_px = 16.0f; // fallback
                    }
                    // 应用 min-width
                    if (child_style.min_width.isPixel() && child_style.min_width.value > width_px) {
                        width_px = child_style.min_width.value;
                    } else if (child_style.min_width.isPercent()) {
                        float min_w = parsePercentValue(child_style.min_width, content_w);
                        if (min_w > width_px) {
                            width_px = min_w;
                        }
                    }

                    // 璁＄畻楂樺害锛氫紭鍏堜娇鐢?CSS 鎸囧畾鐨勯珮搴︼紝鍚﹀垯浣跨敤鏂囨湰琛岄珮 + padding + border
                    float height_px = 0.0f;
                    if (child_style.height.isPixel()) {
                        height_px = child_style.height.value;
                    } else if (child_style.height.isPercent()) {
                        // 鐧惧垎姣旈珮搴︽殏涓嶆敮鎸侊紝浣跨敤鍐呭楂樺害
                        if (has_text_metrics) {
                            height_px = metrics.line_height_px + pad_t + pad_b + border_w * 2.0f;
                        }
                    } else if (has_text_metrics) {
                        height_px = metrics.line_height_px + pad_t + pad_b + border_w * 2.0f;
                    }
                    if (height_px <= 0.0f && has_text_metrics) {
                        height_px = metrics.line_height_px;
                    }
                    if (height_px <= 0.0f) {
                        height_px = 16.0f; // fallback
                    }
                    // 应用 min-height
                    if (child_style.min_height.isPixel() && child_style.min_height.value > height_px) {
                        height_px = child_style.min_height.value;
                    } else if (child_style.min_height.isPercent()) {
                        // 百分比 min-height 暂不支持
                    }

                    item.preferred_width = width_px;
                    item.preferred_height = height_px;
                    item.line_height_px = height_px;  // 浣跨敤鍏冪礌楂樺害浣滀负琛岄珮璐＄尞
                    
                    // baseline 璁＄畻锛氭湁鏂囨湰鏃朵娇鐢ㄦ枃鏈?baseline锛屽惁鍒欎娇鐢ㄥ簳閮ㄥ榻愯繎浼?
                    if (has_text_metrics) {
                        item.baseline_from_border_top = border_w + pad_t + metrics.baseline_from_content_top_px;
                    } else {
                        // 鏃犳枃鏈唴瀹规椂锛宐aseline 杩戜技涓哄厓绱犲簳閮紙绗﹀悎 CSS inline-block 鐨勯粯璁よ涓猴級
                        item.baseline_from_border_top = height_px;
                    }
                    item.vertical_align = child_style.vertical_align;

                    items.push_back(item);
                }

                if (!items.empty() && content_w > 0.0f) {
                    struct LineInfo {
                        std::vector<size_t> item_indices;
                        float max_baseline_from_border_top = 0.0f;
                        float max_line_height_px = 0.0f;
                    };

                    std::vector<LineInfo> lines;
                    LineInfo current_line{};
                    if (has_container_text_metrics) {
                        current_line.max_baseline_from_border_top = container_baseline_from_border_top;
                        current_line.max_line_height_px = container_line_height_px;
                    }
                    float current_line_used_w = 0.0f;

                    for (size_t i = 0; i < items.size(); ++i) {
                        InlineItem& item = items[i];
                        float total_w = item.margin_left + item.preferred_width + item.margin_right;

                        if (!current_line.item_indices.empty() && current_line_used_w + total_w > content_w + 0.1f) {
                            lines.push_back(current_line);
                            current_line = LineInfo{};
                            if (has_container_text_metrics) {
                                current_line.max_baseline_from_border_top = container_baseline_from_border_top;
                                current_line.max_line_height_px = container_line_height_px;
                            }
                            current_line_used_w = 0.0f;
                        }

                        item.offset_x_in_content = current_line_used_w + item.margin_left;
                        current_line.item_indices.push_back(i);
                        current_line_used_w += total_w;

                        if (item.baseline_from_border_top > current_line.max_baseline_from_border_top) {
                            current_line.max_baseline_from_border_top = item.baseline_from_border_top;
                        }
                        if (item.line_height_px > current_line.max_line_height_px) {
                            current_line.max_line_height_px = item.line_height_px;
                        }
                    }

                    if (!current_line.item_indices.empty()) {
                        lines.push_back(current_line);
                    }

                    float current_line_top = content_y;
                    for (const LineInfo& line : lines) {
                        float baseline_y = current_line_top + line.max_baseline_from_border_top;

                        for (size_t idx : line.item_indices) {
                            InlineItem& item = items[idx];
                            auto it_child_layout = layout_cache.find(item.node.get());
                            if (it_child_layout == layout_cache.end() || !it_child_layout->second) {
                                continue;
                            }
                            LayoutNode* child_layout = it_child_layout->second.get();

                            float new_x = content_x + item.offset_x_in_content;
                            float new_y = 0.0f;

                            // 鏍规嵁 vertical-align 璁＄畻 Y 鍧愭爣
                            const std::string& va = item.vertical_align;
                            if (va == "top") {
                                // 椤堕儴瀵归綈锛氬厓绱犻《閮ㄤ笌琛岄《閮ㄥ榻?
                                new_y = current_line_top;
                            } else if (va == "bottom") {
                                // 搴曢儴瀵归綈锛氬厓绱犲簳閮ㄤ笌琛屽簳閮ㄥ榻?
                                new_y = current_line_top + line.max_line_height_px - item.preferred_height;
                            } else if (va == "middle") {
                                // 涓棿瀵归綈锛氬厓绱犱腑蹇冧笌琛屼腑蹇冨榻?
                                float line_center = current_line_top + line.max_line_height_px * 0.5f;
                                new_y = line_center - item.preferred_height * 0.5f;
                            } else {
                                // baseline锛堥粯璁わ級锛氬厓绱?baseline 涓庤 baseline 瀵归綈
                                new_y = baseline_y - item.baseline_from_border_top;
                            }

                            bool layout_changed = (child_layout->x != new_x) || (child_layout->y != new_y) ||
                                                  (child_layout->width != item.preferred_width) ||
                                                  (child_layout->height != item.preferred_height);

                            child_layout->x = new_x;
                            child_layout->y = new_y;
                            child_layout->width = item.preferred_width;
                            // 浣跨敤璁＄畻鍑虹殑 preferred_height锛屽畠宸茬粡鑰冭檻浜?CSS 鎸囧畾鐨勯珮搴?
                            child_layout->height = item.preferred_height;

                            child_layout->layout.position[0] = child_layout->x;
                            child_layout->layout.position[1] = child_layout->y;
                            child_layout->layout.dimensions[0] = child_layout->width;
                            child_layout->layout.dimensions[1] = child_layout->height;

                            if (layout_changed) {
                                dirty_rect_.expand(child_layout->x,
                                                   child_layout->y,
                                                   child_layout->width,
                                                   child_layout->height);
                            }
                        }

                        current_line_top += line.max_line_height_px;
                    }
                    
                    // 更新容器高度：IFC 容器的高度应该包含所有行的高度
                    // 计算内容区域的总高度
                    float total_content_height = current_line_top - content_y;
                    if (total_content_height > 0.0f) {
                        float pad_bottom = container_style.padding_bottom.isPixel() ? container_style.padding_bottom.value : 0.0f;
                        float new_container_height = pad_top + total_content_height + pad_bottom;
                        
                        // 只有当计算出的高度大于 Yoga 计算的高度时才更新
                        // （如果 CSS 指定了固定高度，应该尊重它）
                        if (!container_style.height.isPixel() && new_container_height > container_layout->height) {
                            container_layout->height = new_container_height;
                            container_layout->layout.dimensions[1] = new_container_height;
                            dirty_rect_.expand(container_layout->x, container_layout->y,
                                             container_layout->width, container_layout->height);
                        }
                    }
                }
            }
        }

        for (const auto& child : node->getChildren()) {
            if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                walk(child);
            }
        }
    };

    walk(root);
    
    // 第二遍：自底向上传播高度变化
    // 当 IFC 容器的高度被更新后，其父容器（block 容器）的高度也需要更新
    std::function<float(const dom::DOMNodePtr&)> propagateHeights;
    propagateHeights = [this, &propagateHeights](const dom::DOMNodePtr& node) -> float {
        if (!node || node->getType() != dom::DOMNode::NodeType::ELEMENT) {
            return 0.0f;
        }
        
        const auto& style = node->getComputedStyle();
        if (style.display == "none") {
            return 0.0f;
        }
        
        auto it = layout_cache.find(node.get());
        if (it == layout_cache.end() || !it->second) {
            return 0.0f;
        }
        LayoutNode* layout = it->second.get();
        
        // 如果是绝对定位元素，不参与高度计算
        if (style.position == "absolute" || style.position == "fixed") {
            return 0.0f;
        }
        
        // 递归计算子元素的最大底部位置
        float max_child_bottom = 0.0f;
        for (const auto& child : node->getChildren()) {
            if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                continue;
            }
            const auto& child_style = child->getComputedStyle();
            if (child_style.display == "none" || 
                child_style.position == "absolute" || 
                child_style.position == "fixed") {
                continue;
            }
            
            float child_bottom = propagateHeights(child);
            if (child_bottom > max_child_bottom) {
                max_child_bottom = child_bottom;
            }
        }
        
        // 如果子元素的底部超出了当前容器的高度，更新容器高度
        if (max_child_bottom > 0.0f && !style.height.isPixel()) {
            float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;
            float border_bottom = style.border_width > 0.0f ? style.border_width : 0.0f;
            float required_height = max_child_bottom - layout->y + pad_bottom + border_bottom;
            
            if (required_height > layout->height) {
                layout->height = required_height;
                layout->layout.dimensions[1] = required_height;
                dirty_rect_.expand(layout->x, layout->y, layout->width, layout->height);
            }
        }
        
        // 返回当前元素的底部位置
        return layout->y + layout->height;
    };
    
    propagateHeights(root);
    
    // 第三遍：修正因 IFC 高度变化导致的兄弟元素 Y 坐标偏移
    // 只处理那些包含 IFC 容器的父元素，并且只调整后续兄弟的 Y 坐标
    std::function<void(const dom::DOMNodePtr&, float)> adjustSiblingPositions;
    adjustSiblingPositions = [this, &adjustSiblingPositions](const dom::DOMNodePtr& node, float parent_delta_y) {
        if (!node || node->getType() != dom::DOMNode::NodeType::ELEMENT) {
            return;
        }
        
        const auto& style = node->getComputedStyle();
        if (style.display == "none") {
            return;
        }
        
        auto it = layout_cache.find(node.get());
        if (it == layout_cache.end() || !it->second) {
            return;
        }
        LayoutNode* layout = it->second.get();
        
        // 如果父元素移动了，先移动当前元素
        if (std::abs(parent_delta_y) > 0.1f) {
            layout->y += parent_delta_y;
            layout->layout.position[1] = layout->y;
            dirty_rect_.expand(layout->x, layout->y, layout->width, layout->height);
        }
        
        // 递归处理子元素（先修正子树内部的 IFC 高度/行布局，再回到本层做兄弟 Y 修正）
        for (const auto& child : node->getChildren()) {
            if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                const auto& child_style = child->getComputedStyle();
                if (child_style.display != "none" &&
                    child_style.position != "absolute" &&
                    child_style.position != "fixed") {
                    adjustSiblingPositions(child, 0.0f);
                }
            }
        }

        if (style.display != "flex" && style.display != "inline-flex") {
        // 检查是否需要调整子元素的 Y 坐标
        // 背景：我们在第二遍/第三遍里可能会"补齐"某些容器（尤其是含 IFC 后代）的高度，
        // 但 Yoga 已经给出了兄弟元素的 Y。如果不把后续兄弟整体下移，会出现
        // "后一个 block 覆盖前一个（因为绘制顺序在后）"的现象。
        //
        // 策略：对当前容器的**正常流 block 子元素**做一次"最小下移修正"，确保：
        // child.y >= prev.y + prev.height + margin-bottom(prev) + margin-top(child)
        // 仅当 child 被放得过高时才下移（不做上移），减少对 Yoga/flex 结果的干扰。
        {
            std::function<void(const dom::DOMNodePtr&, float)> shiftSubtreeY;
            shiftSubtreeY = [this, &shiftSubtreeY](const dom::DOMNodePtr& n, float dy) {
                if (!n || n->getType() != dom::DOMNode::NodeType::ELEMENT) return;
                auto itn = layout_cache.find(n.get());
                if (itn != layout_cache.end() && itn->second) {
                    LayoutNode* ln = itn->second.get();
                    ln->y += dy;
                    ln->layout.position[1] = ln->y;
                    dirty_rect_.expand(ln->x, ln->y, ln->width, ln->height);
                }
                for (const auto& ch : n->getChildren()) {
                    if (!ch || ch->getType() != dom::DOMNode::NodeType::ELEMENT) continue;
                    const auto& cs = ch->getComputedStyle();
                    if (cs.display == "none") continue;
                    // 绝对/固定定位会在后续 layoutPositionedElements 阶段重新计算位置
                    if (cs.position == "absolute" || cs.position == "fixed") continue;
                    shiftSubtreeY(ch, dy);
                }
            };

            float cumulative_delta_y = 0.0f;
            dom::DOMNodePtr prev_flow_child = nullptr;

            for (const auto& child : node->getChildren()) {
                if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                    continue;
                }

                const auto& child_style = child->getComputedStyle();
                if (child_style.display == "none" ||
                    child_style.position == "absolute" ||
                    child_style.position == "fixed" ||
                    child_style.display == "inline" ||
                    child_style.display == "inline-block") {
                    continue;
                }

                auto child_it = layout_cache.find(child.get());
                if (child_it == layout_cache.end() || !child_it->second) {
                    continue;
                }
                LayoutNode* child_layout = child_it->second.get();

                if (prev_flow_child) {
                    auto prev_it = layout_cache.find(prev_flow_child.get());
                    if (prev_it != layout_cache.end() && prev_it->second) {
                        LayoutNode* prev_layout = prev_it->second.get();
                        const auto& prev_style = prev_flow_child->getComputedStyle();

                        float margin_bottom = prev_style.margin_bottom.isPixel()
                                             ? prev_style.margin_bottom.value : 0.0f;
                        float margin_top = child_style.margin_top.isPixel()
                                          ? child_style.margin_top.value : 0.0f;

                        float expected_y = prev_layout->y + prev_layout->height + margin_bottom + margin_top;
                        float delta = expected_y - child_layout->y;
                        if (delta > 0.1f) {
                            if (std::getenv("DONG_DEBUG_IFC_SHIFT")) {
                                const std::string parent_cls = node->hasAttribute("class") ? node->getAttribute("class") : std::string();
                                const std::string prev_cls = prev_flow_child->hasAttribute("class") ? prev_flow_child->getAttribute("class") : std::string();
                                const std::string child_cls = child->hasAttribute("class") ? child->getAttribute("class") : std::string();
                                DONG_LOG_INFO("[IFC_SHIFT] parent='%s' prev='%s' child='%s' prev_y=%.1f prev_h=%.1f child_y=%.1f expected_y=%.1f delta=%.1f",
                                    parent_cls.c_str(), prev_cls.c_str(), child_cls.c_str(),
                                    prev_layout->y, prev_layout->height, child_layout->y, expected_y, delta);
                            }
                            cumulative_delta_y += delta;
                        }
                    }
                }

                if (std::abs(cumulative_delta_y) > 0.1f) {
                    shiftSubtreeY(child, cumulative_delta_y);
                }

                prev_flow_child = child;
            }

            // 如果有子元素移动了，更新容器高度
            if (std::abs(cumulative_delta_y) > 0.1f && !style.height.isPixel()) {
                float pad_bottom = style.padding_bottom.isPixel() ? style.padding_bottom.value : 0.0f;

                // 找到最后一个可见子元素的底部
                float max_bottom = layout->y;
                for (const auto& child : node->getChildren()) {
                    if (!child || child->getType() != dom::DOMNode::NodeType::ELEMENT) {
                        continue;
                    }
                    const auto& child_style = child->getComputedStyle();
                    if (child_style.display == "none" ||
                        child_style.position == "absolute" ||
                        child_style.position == "fixed") {
                        continue;
                    }
                    auto child_it = layout_cache.find(child.get());
                    if (child_it != layout_cache.end() && child_it->second) {
                        float margin_bottom = child_style.margin_bottom.isPixel()
                                             ? child_style.margin_bottom.value : 0.0f;
                        float child_bottom = child_it->second->y + child_it->second->height + margin_bottom;
                        if (child_bottom > max_bottom) {
                            max_bottom = child_bottom;
                        }
                    }
                }

                float new_height = max_bottom - layout->y + pad_bottom;
                if (new_height > layout->height) {
                    layout->height = new_height;
                    layout->layout.dimensions[1] = new_height;
                    dirty_rect_.expand(layout->x, layout->y, layout->width, layout->height);
                }
            }
        }
        }
    };
    
    adjustSiblingPositions(root, 0.0f);
}

void Engine::layoutPositionedElements(dom::DOMNodePtr root) {
    if (!root) {
        return;
    }

    std::function<void(const dom::DOMNodePtr&)> walk;
    walk = [this, root, &walk](const dom::DOMNodePtr& node) {
        if (!node) {
            return;
        }

        const auto& style = node->getComputedStyle();

        // Handle position: relative
        // Relative positioning: element stays in normal flow but is visually offset
        // by top/right/bottom/left values. This offset does NOT affect sibling layout.
        if (style.position == "relative") {
            auto it_layout = layout_cache.find(node.get());
            if (it_layout != layout_cache.end() && it_layout->second) {
                LayoutNode* layout = it_layout->second.get();

                // Compute offset values
                auto computeOffsetPx = [this](const dom::CSSValue& v, float reference_size) -> float {
                    if (v.isPixel()) {
                        return v.value;
                    }
                    if (v.isPercent()) {
                        return parsePercentValue(v, reference_size);
                    }
                    return 0.0f;
                };

                // For relative positioning, percentages are relative to the containing block
                // (parent's content box). We use the layout dimensions as approximation.
                float ref_w = layout->width;
                float ref_h = layout->height;
                if (auto parent = node->getParent()) {
                    auto it_parent = layout_cache.find(parent.get());
                    if (it_parent != layout_cache.end() && it_parent->second) {
                        ref_w = it_parent->second->width;
                        ref_h = it_parent->second->height;
                    }
                }

                bool has_left = style.left.isPixel() || style.left.isPercent();
                bool has_right = style.right.isPixel() || style.right.isPercent();
                bool has_top = style.top.isPixel() || style.top.isPercent();
                bool has_bottom = style.bottom.isPixel() || style.bottom.isPercent();

                float offset_x = 0.0f;
                float offset_y = 0.0f;

                // Horizontal: left takes precedence over right
                if (has_left) {
                    offset_x = computeOffsetPx(style.left, ref_w);
                } else if (has_right) {
                    offset_x = -computeOffsetPx(style.right, ref_w);
                }

                // Vertical: top takes precedence over bottom
                if (has_top) {
                    offset_y = computeOffsetPx(style.top, ref_h);
                } else if (has_bottom) {
                    offset_y = -computeOffsetPx(style.bottom, ref_h);
                }

                // Apply offset to the layout position
                if (offset_x != 0.0f || offset_y != 0.0f) {
                    float new_x = layout->x + offset_x;
                    float new_y = layout->y + offset_y;

                    bool layout_changed = (layout->x != new_x) || (layout->y != new_y);

                    layout->x = new_x;
                    layout->y = new_y;
                    layout->layout.position[0] = new_x;
                    layout->layout.position[1] = new_y;

                    if (layout_changed) {
                        dirty_rect_.expand(new_x, new_y, layout->width, layout->height);
                    }
                }
            }
        }

        // Handle position: absolute
        if (style.position == "absolute") {
            // Find containing block: nearest ancestor with position != static.
            dom::DOMNodePtr containing_block = nullptr;
            dom::DOMNodePtr current = node->getParent();
            while (current) {
                const auto& cs = current->getComputedStyle();
                if (cs.position != "static") {
                    containing_block = current;
                    break;
                }
                current = current->getParent();
            }

            if (!containing_block) {
                // Fallback to the layout root if no positioned ancestor is found.
                containing_block = root;
            }

            auto it_layout = layout_cache.find(node.get());
            auto it_cb_layout = layout_cache.find(containing_block.get());
            if (it_layout != layout_cache.end() && it_layout->second &&
                it_cb_layout != layout_cache.end() && it_cb_layout->second) {
                LayoutNode* layout = it_layout->second.get();
                LayoutNode* cb_layout = it_cb_layout->second.get();

                const auto& cb_style = containing_block->getComputedStyle();
                const auto& abs_style = node->getComputedStyle();

                std::string debug_class = node->getAttribute("class");
                bool debug_is_abs_badge = debug_class.find("abs-badge") != std::string::npos;

                float cb_x = cb_layout->x;
                float cb_y = cb_layout->y;
                float cb_w = cb_layout->width;
                float cb_h = cb_layout->height;

                if (cb_w <= 0.0f || cb_h <= 0.0f) {
                    // Degenerate containing block, keep existing layout.
                } else {
                    // 鑻?absolute 鐩掑瓙鐨?CSS width/height 涓?auto锛堟湭鏄惧紡鎸囧畾锛夛紝
                    // 鍒欐棤璁?Yoga 缁欏嚭鐨勫昂瀵镐负浣曪紝閮藉熀浜庢枃鏈唴瀹瑰仛涓€娆?intrinsic sizing锛?
                    // 瀹藉害 = 鏂囨湰鍐呭瀹?+ 姘村钩 padding + border锛?
                    // 楂樺害 = 涓€琛屾枃鏈楂?+ 鍨傜洿 padding + border銆?
                    const bool abs_width_auto = abs_style.width.isAuto();
                    const bool abs_height_auto = abs_style.height.isAuto();
                    if (abs_width_auto || abs_height_auto ||
                        layout->width <= 0.0f || layout->height <= 0.0f) {
                        InlineMetrics metrics{};
                        if (computeInlineMetricsForNode(node, metrics, abs_style.font_size)) {
                            float pad_t = abs_style.padding_top.isPixel() ? abs_style.padding_top.value : 0.0f;
                            float pad_b = abs_style.padding_bottom.isPixel() ? abs_style.padding_bottom.value : 0.0f;
                            float border_w = abs_style.border_width;
                            if (border_w < 0.0f) {
                                border_w = 0.0f;
                            }
                            // 绗﹀悎 CSS 鏍囧噯锛氫娇鐢ㄥ寘鍚?padding 鐨勫畬鏁村搴?
                            float box_w_intrinsic = metrics.total_width_px + border_w * 2.0f;
                            float box_h_intrinsic = metrics.line_height_px + pad_t + pad_b + border_w * 2.0f;

                            if (debug_is_abs_badge) {
                                DONG_LOG_DEBUG("[LayoutEngine] ABS badge intrinsic: content_w=%.2f total_w=%.2f line_h=%.2f pad_l=%.1f pad_r=%.1f pad_t=%.1f pad_b=%.1f border=%.1f -> box_w=%.2f box_h=%.2f (width_auto=%d height_auto=%d before: w=%.2f h=%.2f)",
                                        metrics.content_width_px, metrics.total_width_px, metrics.line_height_px,
                                        metrics.padding_left_px, metrics.padding_right_px, pad_t, pad_b, border_w,
                                        box_w_intrinsic, box_h_intrinsic,
                                        abs_width_auto ? 1 : 0, abs_height_auto ? 1 : 0,
                                        layout->width, layout->height);
                            }

                            if (box_w_intrinsic > 0.0f && box_h_intrinsic > 0.0f) {
                                if (abs_width_auto || layout->width <= 0.0f) {
                                    layout->width = box_w_intrinsic;
                                }
                                if (abs_height_auto || layout->height <= 0.0f) {
                                    layout->height = box_h_intrinsic;
                                }
                                layout->layout.dimensions[0] = layout->width;
                                layout->layout.dimensions[1] = layout->height;

                                if (debug_is_abs_badge) {
                                    DONG_LOG_DEBUG("[LayoutEngine] ABS badge final size: w=%.2f h=%.2f",
                                            layout->width, layout->height);
                                }
                            }
                        } else if (debug_is_abs_badge) {
                            DONG_LOG_DEBUG("[LayoutEngine] ABS badge intrinsic metrics FAILED (font_size=%.1f)",
                                    abs_style.font_size);
                        }
                    } else if (debug_is_abs_badge) {
                        DONG_LOG_DEBUG("[LayoutEngine] ABS badge intrinsic sizing skipped (width_auto=%d height_auto=%d layout_w=%.2f layout_h=%.2f)",
                                abs_width_auto ? 1 : 0, abs_height_auto ? 1 : 0,
                                layout->width, layout->height);
                    }

                    // In CSS锛屽寘鍚潡閫氬父鏄?padding box銆傝繖閲屽厛鍩轰簬 border box锛屽悗缁彲瑙嗛渶瑕佸姞涓?padding銆?
                    auto computeOffsetPx = [this](const dom::CSSValue& v, float parent_size) -> float {
                        if (v.isPixel()) {
                            return v.value;
                        }
                        if (v.isPercent()) {
                            return parsePercentValue(v, parent_size);
                        }
                        return 0.0f;
                    };

                    bool has_left = style.left.isPixel() || style.left.isPercent();
                    bool has_right = style.right.isPixel() || style.right.isPercent();
                    bool has_top = style.top.isPixel() || style.top.isPercent();
                    bool has_bottom = style.bottom.isPixel() || style.bottom.isPercent();

                    float left_px = has_left ? computeOffsetPx(style.left, cb_w) : 0.0f;
                    float right_px = has_right ? computeOffsetPx(style.right, cb_w) : 0.0f;
                    float top_px = has_top ? computeOffsetPx(style.top, cb_h) : 0.0f;
                    float bottom_px = has_bottom ? computeOffsetPx(style.bottom, cb_h) : 0.0f;

                    if (debug_is_abs_badge) {
                        DONG_LOG_DEBUG("[LayoutEngine] ABS badge offsets: has_left=%d has_right=%d has_top=%d has_bottom=%d left_px=%.1f right_px=%.1f top_px=%.1f bottom_px=%.1f cb=(%.1f,%.1f,%.1f,%.1f) box=(%.1f,%.1f)",
                                has_left ? 1 : 0, has_right ? 1 : 0,
                                has_top ? 1 : 0, has_bottom ? 1 : 0,
                                left_px, right_px, top_px, bottom_px,
                                cb_x, cb_y, cb_w, cb_h,
                                layout->width, layout->height);
                    }

                    float box_w = layout->width;
                    float box_h = layout->height;

                    float offset_x_local = 0.0f;
                    float offset_y_local = 0.0f;

                    // Horizontal: prefer left if specified; otherwise use right.
                    if (has_left) {
                        offset_x_local = left_px;
                    } else if (has_right) {
                        offset_x_local = cb_w - right_px - box_w;
                    } else {
                        // No explicit offset: keep the original relative offset within containing block.
                        offset_x_local = layout->x - cb_x;
                    }

                    // Vertical: prefer top if specified; otherwise use bottom.
                    if (has_top) {
                        offset_y_local = top_px;
                    } else if (has_bottom) {
                        offset_y_local = cb_h - bottom_px - box_h;
                    } else {
                        offset_y_local = layout->y - cb_y;
                    }

                    float new_x = cb_x + offset_x_local;
                    float new_y = cb_y + offset_y_local;

                    bool layout_changed = (layout->x != new_x) || (layout->y != new_y);

                    layout->x = new_x;
                    layout->y = new_y;
                    layout->layout.position[0] = new_x;
                    layout->layout.position[1] = new_y;

                    if (layout_changed) {
                        dirty_rect_.expand(new_x, new_y, layout->width, layout->height);
                    }
                }
            }
        }

        for (const auto& child : node->getChildren()) {
            if (child && child->getType() == dom::DOMNode::NodeType::ELEMENT) {
                walk(child);
            }
        }
    };

    walk(root);
}

} // namespace dong::layout
