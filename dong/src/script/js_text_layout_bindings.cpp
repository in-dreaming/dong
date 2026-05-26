#include "quickjs_compat.h"
#include "js_bindings.hpp"
#include "../render/text_layout_core.hpp"
#include "../render/overlay_draw.hpp"
#include "../render/text_shaper.hpp"
#include <string>
#include <vector>
#include <cmath>
#include <functional>

namespace dong::script {

// Static cache: prepare() once, layoutNextLine() many times per text+font combo.
// This is the core optimization pretext teaches — separate measurement from layout.
static render::TextLayoutCore s_layout_core;
static size_t s_cached_key = 0;
static render::PreparedText s_cached_prepared;

static size_t makeKey(const std::string& text, const std::string& family,
                      const std::string& weight, const std::string& style,
                      float font_size, float line_height) {
    size_t h = std::hash<std::string>{}(text);
    h ^= std::hash<std::string>{}(family) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<std::string>{}(weight) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<std::string>{}(style)  + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<float>{}(font_size)    + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<float>{}(line_height)  + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
}

static std::string jsStr(JSContext* ctx, JSValueConst obj, const char* key,
                         const char* fallback = "") {
    JSValue v = JS_GetPropertyStr(ctx, obj, key);
    if (JS_IsUndefined(v) || JS_IsNull(v)) { JS_FreeValue(ctx, v); return fallback; }
    const char* s = JS_ToCString(ctx, v);
    std::string r = s ? s : fallback;
    if (s) JS_FreeCString(ctx, s);
    JS_FreeValue(ctx, v);
    return r;
}

static double jsNum(JSContext* ctx, JSValueConst obj, const char* key, double fallback = 0) {
    JSValue v = JS_GetPropertyStr(ctx, obj, key);
    if (JS_IsUndefined(v) || JS_IsNull(v)) { JS_FreeValue(ctx, v); return fallback; }
    double d = 0; JS_ToFloat64(ctx, &d, v); JS_FreeValue(ctx, v); return d;
}

static int64_t jsArrLen(JSContext* ctx, JSValueConst arr) {
    JSValue v = JS_GetPropertyStr(ctx, arr, "length");
    int64_t n = 0; JS_ToInt64(ctx, &n, v); JS_FreeValue(ctx, v); return n;
}

static render::OverlayDraw& resolveOverlayDraw(JSContext* ctx) {
    auto* bindings = static_cast<JSBindings*>(JS_GetContextOpaque(ctx));
    if (bindings && bindings->overlay_draw_) {
        return *bindings->overlay_draw_;
    }
    // Fallback for legacy contexts that don't wire per-view storage.
    return render::OverlayDraw::instance();
}

// Horizontal interval
struct Slot { float left, right; };

static std::vector<Slot> carveSlots(Slot base, const std::vector<Slot>& blocked) {
    std::vector<Slot> free = {base};
    for (const auto& b : blocked) {
        std::vector<Slot> next;
        for (const auto& f : free) {
            if (b.right <= f.left || b.left >= f.right) { next.push_back(f); continue; }
            if (b.left > f.left) next.push_back({f.left, b.left});
            if (b.right < f.right) next.push_back({b.right, f.right});
        }
        free = std::move(next);
    }
    std::vector<Slot> result;
    for (const auto& s : free)
        if (s.right - s.left > 1.0f) result.push_back(s);
    return result;
}

static bool circleBlockedInterval(float cx, float cy, float r,
                                  float h_pad, float v_pad,
                                  float band_top, float band_bottom,
                                  Slot& out) {
    float ry = r + v_pad;
    if (band_bottom <= cy - ry || band_top >= cy + ry) return false;
    float dy = 0;
    if (band_top > cy) dy = band_top - cy;
    else if (band_bottom < cy) dy = cy - band_bottom;
    if (dy >= r) return false;
    float half = std::sqrt(r * r - dy * dy);
    out = {cx - half - h_pad, cx + half + h_pad};
    return out.right > out.left;
}

static bool rectBlockedInterval(float rx, float ry, float rw, float rh,
                                float h_pad, float v_pad,
                                float band_top, float band_bottom,
                                Slot& out) {
    if (band_bottom <= ry - v_pad || band_top >= ry + rh + v_pad) return false;
    out = {rx - h_pad, rx + rw + h_pad};
    return out.right > out.left;
}

struct CircleDef { float cx, cy, r, h_pad, v_pad; };
struct RectDef   { float x, y, w, h, h_pad, v_pad; };

// dong.textLayout({text, font, lineHeight, columns, obstacles}) → {columns: [{lines}]}
static JSValue js_dong_textLayout(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 1 || !JS_IsObject(argv[0]))
        return JS_ThrowTypeError(ctx, "textLayout: config object required");
    JSValueConst cfg = argv[0];

    JSValue text_val = JS_GetPropertyStr(ctx, cfg, "text");
    const char* raw = JS_ToCString(ctx, text_val);
    JS_FreeValue(ctx, text_val);
    if (!raw) return JS_ThrowTypeError(ctx, "textLayout: 'text' required");
    std::string text(raw);
    JS_FreeCString(ctx, raw);

    JSValue font_val = JS_GetPropertyStr(ctx, cfg, "font");
    std::string family = "sans-serif", weight = "normal", style = "normal";
    float font_size = 16;
    if (JS_IsObject(font_val)) {
        family = jsStr(ctx, font_val, "family", "sans-serif");
        weight = jsStr(ctx, font_val, "weight", "normal");
        style  = jsStr(ctx, font_val, "style", "normal");
        font_size = (float)jsNum(ctx, font_val, "size", 16);
    }
    JS_FreeValue(ctx, font_val);

    float line_height = (float)jsNum(ctx, cfg, "lineHeight", 0);

    // Phase 1: prepare (segment + measure, cached widths) — skipped if text+font unchanged
    size_t key = makeKey(text, family, weight, style, font_size, line_height);
    if (key != s_cached_key) {
        s_cached_prepared = s_layout_core.prepare(text, family, weight, style, font_size, line_height);
        s_cached_key = key;
    }
    const auto& prepared = s_cached_prepared;

    // Parse obstacles
    std::vector<CircleDef> circles;
    std::vector<RectDef> rects;
    JSValue obs_val = JS_GetPropertyStr(ctx, cfg, "obstacles");
    if (JS_IsObject(obs_val)) {
        JSValue ca = JS_GetPropertyStr(ctx, obs_val, "circles");
        if (JS_IsArray(ctx, ca)) {
            for (int64_t i = 0, n = jsArrLen(ctx, ca); i < n; i++) {
                JSValue it = JS_GetPropertyUint32(ctx, ca, (uint32_t)i);
                circles.push_back({(float)jsNum(ctx, it, "cx"), (float)jsNum(ctx, it, "cy"),
                                   (float)jsNum(ctx, it, "r"),
                                   (float)jsNum(ctx, it, "hPad"), (float)jsNum(ctx, it, "vPad")});
                JS_FreeValue(ctx, it);
            }
        }
        JS_FreeValue(ctx, ca);
        JSValue ra = JS_GetPropertyStr(ctx, obs_val, "rects");
        if (JS_IsArray(ctx, ra)) {
            for (int64_t i = 0, n = jsArrLen(ctx, ra); i < n; i++) {
                JSValue it = JS_GetPropertyUint32(ctx, ra, (uint32_t)i);
                rects.push_back({(float)jsNum(ctx, it, "x"), (float)jsNum(ctx, it, "y"),
                                 (float)jsNum(ctx, it, "w"), (float)jsNum(ctx, it, "h"),
                                 (float)jsNum(ctx, it, "hPad"), (float)jsNum(ctx, it, "vPad")});
                JS_FreeValue(ctx, it);
            }
        }
        JS_FreeValue(ctx, ra);
    }
    JS_FreeValue(ctx, obs_val);

    // Parse columns
    JSValue cols_val = JS_GetPropertyStr(ctx, cfg, "columns");
    if (!JS_IsArray(ctx, cols_val)) {
        JS_FreeValue(ctx, cols_val);
        return JS_ThrowTypeError(ctx, "textLayout: 'columns' array required");
    }

    // Phase 2: layout (pure arithmetic per column/line)
    JSValue result = JS_NewObject(ctx);
    JSValue result_cols = JS_NewArray(ctx);
    render::TextCursor cursor{};
    float lh = prepared.line_height_px;

    for (int64_t ci = 0, cn = jsArrLen(ctx, cols_val); ci < cn; ci++) {
        JSValue cc = JS_GetPropertyUint32(ctx, cols_val, (uint32_t)ci);
        float rx = (float)jsNum(ctx, cc, "x");
        float ry = (float)jsNum(ctx, cc, "y");
        float rw = (float)jsNum(ctx, cc, "width", 300);
        float rh_val = (float)jsNum(ctx, cc, "height", 600);
        JS_FreeValue(ctx, cc);

        JSValue col_obj = JS_NewObject(ctx);
        JSValue lines_arr = JS_NewArray(ctx);
        uint32_t li = 0;
        float y = ry;

        while (!cursor.at_end(prepared) && y + lh <= ry + rh_val + 0.5f) {
            // Collect blocked intervals for this line band
            std::vector<Slot> blocked;
            Slot interval;
            for (const auto& c : circles)
                if (circleBlockedInterval(c.cx, c.cy, c.r, c.h_pad, c.v_pad, y, y + lh, interval))
                    blocked.push_back(interval);
            for (const auto& r : rects)
                if (rectBlockedInterval(r.x, r.y, r.w, r.h, r.h_pad, r.v_pad, y, y + lh, interval))
                    blocked.push_back(interval);

            auto slots = carveSlots({rx, rx + rw}, blocked);
            if (slots.empty()) { y += lh; continue; }

            // Fill ALL slots left-to-right (tight wrap)
            for (const auto& slot : slots) {
                if (cursor.at_end(prepared)) break;
                float sw = slot.right - slot.left;
                if (sw < prepared.font_size) continue;

                render::LayoutLine line;
                if (s_layout_core.layoutNextLine(prepared, cursor, sw, line)) {
                    JSValue lo = JS_NewObject(ctx);
                    JS_SetPropertyStr(ctx, lo, "x", JS_NewFloat64(ctx, slot.left));
                    JS_SetPropertyStr(ctx, lo, "y", JS_NewFloat64(ctx, y));
                    JS_SetPropertyStr(ctx, lo, "width", JS_NewFloat64(ctx, line.width));
                    JS_SetPropertyStr(ctx, lo, "text", JS_NewString(ctx, line.text.c_str()));
                    JS_SetPropertyUint32(ctx, lines_arr, li++, lo);
                }
            }
            y += lh;
        }

        JS_SetPropertyStr(ctx, col_obj, "lines", lines_arr);
        JS_SetPropertyUint32(ctx, result_cols, (uint32_t)ci, col_obj);
    }
    JS_FreeValue(ctx, cols_val);

    JS_SetPropertyStr(ctx, result, "columns", result_cols);
    JS_SetPropertyStr(ctx, result, "lineHeight", JS_NewFloat64(ctx, lh));
    JS_SetPropertyStr(ctx, result, "ascent", JS_NewFloat64(ctx, prepared.ascent_px));
    return result;
}

// dong.clearOverlay() — clear all direct-draw items
static JSValue js_dong_clearOverlay(JSContext* ctx, JSValueConst, int, JSValueConst*) {
    resolveOverlayDraw(ctx).clear();
    return JS_UNDEFINED;
}

static render::Color parseHexColor(const std::string& s, render::Color fallback = {0.2f, 0.2f, 0.2f, 1.0f}) {
    if (s.size() < 7 || s[0] != '#') return fallback;
    auto hex2f = [](const char* p) -> float {
        int v = 0;
        for (int i = 0; i < 2; i++) {
            char c = p[i];
            v = v * 16 + (c >= 'a' ? c - 'a' + 10 : c >= 'A' ? c - 'A' + 10 : c - '0');
        }
        return v / 255.0f;
    };
    render::Color c;
    c.r = hex2f(s.c_str() + 1);
    c.g = hex2f(s.c_str() + 3);
    c.b = hex2f(s.c_str() + 5);
    c.a = s.size() >= 9 ? hex2f(s.c_str() + 7) : 1.0f;
    return c;
}

// dong.renderText({lines: [{x,y,text}], font: {family,size,weight,style}, color: "#rrggbb"})
// Bypasses DOM entirely: shapes text via cached TextShaper, merges ALL lines into
// a SINGLE DrawGlyphRunData to minimize GPU draw calls (1 instead of N).
static JSValue js_dong_renderText(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 1 || !JS_IsObject(argv[0]))
        return JS_ThrowTypeError(ctx, "renderText: config object required");
    JSValueConst cfg = argv[0];

    JSValue font_val = JS_GetPropertyStr(ctx, cfg, "font");
    std::string family = "sans-serif", weight = "normal", style = "normal";
    float font_size = 16;
    if (JS_IsObject(font_val)) {
        family = jsStr(ctx, font_val, "family", "sans-serif");
        weight = jsStr(ctx, font_val, "weight", "normal");
        style  = jsStr(ctx, font_val, "style", "normal");
        font_size = (float)jsNum(ctx, font_val, "size", 16);
    }
    JS_FreeValue(ctx, font_val);

    render::Color color = parseHexColor(jsStr(ctx, cfg, "color", "#333333"));

    JSValue lines_val = JS_GetPropertyStr(ctx, cfg, "lines");
    if (!JS_IsArray(ctx, lines_val)) {
        JS_FreeValue(ctx, lines_val);
        return JS_ThrowTypeError(ctx, "renderText: 'lines' array required");
    }

    // Merge all lines into a single DrawGlyphRunData.
    // Each line's glyphs are offset by the line's (x, y) position in design units.
    render::DrawGlyphRunData merged;
    merged.color = color;
    merged.font_size = font_size;
    merged.font_family = family;
    merged.font_weight = weight;
    merged.font_style = style;
    merged.baseline_x = 0;
    merged.baseline_y = 0;

    float min_x = 1e9f, min_y = 1e9f, max_x = -1e9f, max_y = -1e9f;
    bool first_shape = true;

    render::TextShaper shaper;
    int64_t num_lines = jsArrLen(ctx, lines_val);
    for (int64_t i = 0; i < num_lines; i++) {
        JSValue line_obj = JS_GetPropertyUint32(ctx, lines_val, (uint32_t)i);
        float x = (float)jsNum(ctx, line_obj, "x");
        float y = (float)jsNum(ctx, line_obj, "y");
        std::string text = jsStr(ctx, line_obj, "text", "");
        JS_FreeValue(ctx, line_obj);

        if (text.empty()) continue;

        render::TextShapeRequest req;
        req.text = text;
        req.font_family = family;
        req.font_weight = weight;
        req.font_style = style;
        req.font_size = font_size;

        render::ShapedText shaped;
        if (!shaper.shape(req, shaped) || shaped.glyphs.empty()) continue;

        if (first_shape) {
            merged.font_paths = shaped.font_paths;
            merged.font_path = shaped.font_path;
            merged.units_per_em = shaped.units_per_em;
            merged.scale_to_pixels = shaped.scale_to_pixels;
            first_shape = false;
        }

        float ascent = shaped.ascent_units * shaped.scale_to_pixels;
        float baseline_y = y + ascent;
        float inv_scale = (merged.scale_to_pixels > 0) ? (1.0f / merged.scale_to_pixels) : 1.0f;

        // Convert pixel-space line origin to design units and offset each glyph
        float origin_x_units = x * inv_scale;
        float origin_y_units = baseline_y * inv_scale;

        for (const auto& sg : shaped.glyphs) {
            render::GlyphInstance gi;
            gi.glyph_id = sg.glyph_id;
            gi.pen_x_units = sg.pen_x_units + origin_x_units;
            gi.pen_y_units = sg.pen_y_units + origin_y_units;
            gi.font_path_index = sg.font_path_index;
            gi.units_per_em = sg.units_per_em;
            merged.glyphs.push_back(gi);
        }

        float text_width = shaped.width_units * shaped.scale_to_pixels;
        float text_height = ascent + std::abs(shaped.descent_units * shaped.scale_to_pixels);
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        max_x = std::max(max_x, x + text_width);
        max_y = std::max(max_y, y + text_height);
    }
    JS_FreeValue(ctx, lines_val);

    if (!merged.glyphs.empty()) {
        merged.rect = {min_x, min_y, max_x - min_x, max_y - min_y};
        auto& overlay = resolveOverlayDraw(ctx);
        overlay.addGlyphRun(std::move(merged));
    }

    return JS_UNDEFINED;
}

// dong.drawRect({x, y, w, h, color, radius?, strokeWidth?})
static JSValue js_dong_drawRect(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 1 || !JS_IsObject(argv[0]))
        return JS_ThrowTypeError(ctx, "drawRect: config object required");
    JSValueConst cfg = argv[0];
    float x = (float)jsNum(ctx, cfg, "x");
    float y = (float)jsNum(ctx, cfg, "y");
    float w = (float)jsNum(ctx, cfg, "w");
    float h = (float)jsNum(ctx, cfg, "h");
    render::Color color = parseHexColor(jsStr(ctx, cfg, "color", "#ffffff"));
    float radius = (float)jsNum(ctx, cfg, "radius", 0);
    float strokeWidth = (float)jsNum(ctx, cfg, "strokeWidth", 0);

    auto& overlay = resolveOverlayDraw(ctx);
    if (radius > 0.01f || strokeWidth > 0.01f) {
        overlay.addRoundedRect({x, y, w, h}, color, radius, strokeWidth);
    } else {
        overlay.addRect({x, y, w, h}, color);
    }
    return JS_UNDEFINED;
}

// dong.drawCircle({cx, cy, r, color, strokeWidth?})
static JSValue js_dong_drawCircle(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv) {
    if (argc < 1 || !JS_IsObject(argv[0]))
        return JS_ThrowTypeError(ctx, "drawCircle: config object required");
    JSValueConst cfg = argv[0];
    float cx = (float)jsNum(ctx, cfg, "cx");
    float cy = (float)jsNum(ctx, cfg, "cy");
    float r  = (float)jsNum(ctx, cfg, "r");
    render::Color color = parseHexColor(jsStr(ctx, cfg, "color", "#ffffff"));
    float strokeWidth = (float)jsNum(ctx, cfg, "strokeWidth", 0);

    resolveOverlayDraw(ctx).addCircle(cx, cy, r, color, strokeWidth);
    return JS_UNDEFINED;
}

void registerTextLayoutAPI(JSContext* ctx) {
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue dong_obj = JS_GetPropertyStr(ctx, global, "dong");
    if (JS_IsUndefined(dong_obj) || JS_IsNull(dong_obj)) {
        JS_FreeValue(ctx, dong_obj);
        dong_obj = JS_NewObject(ctx);
    }
    JS_SetPropertyStr(ctx, dong_obj, "textLayout",
                      JS_NewCFunction(ctx, js_dong_textLayout, "textLayout", 1));
    JS_SetPropertyStr(ctx, dong_obj, "renderText",
                      JS_NewCFunction(ctx, js_dong_renderText, "renderText", 1));
    JS_SetPropertyStr(ctx, dong_obj, "clearOverlay",
                      JS_NewCFunction(ctx, js_dong_clearOverlay, "clearOverlay", 0));
    JS_SetPropertyStr(ctx, dong_obj, "drawRect",
                      JS_NewCFunction(ctx, js_dong_drawRect, "drawRect", 1));
    JS_SetPropertyStr(ctx, dong_obj, "drawCircle",
                      JS_NewCFunction(ctx, js_dong_drawCircle, "drawCircle", 1));
    JS_SetPropertyStr(ctx, global, "dong", dong_obj);
    // dong_obj ownership transferred to global
    JS_FreeValue(ctx, global);
}

} // namespace dong::script
