#include "quickjs_compat.h"
#include "../render/text_layout_core.hpp"
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

void registerTextLayoutAPI(JSContext* ctx) {
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue dong_obj = JS_GetPropertyStr(ctx, global, "dong");
    if (JS_IsUndefined(dong_obj) || JS_IsNull(dong_obj)) {
        JS_FreeValue(ctx, dong_obj);
        dong_obj = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, dong_obj, "textLayout",
                          JS_NewCFunction(ctx, js_dong_textLayout, "textLayout", 1));
        JS_SetPropertyStr(ctx, global, "dong", dong_obj);
    } else {
        JS_SetPropertyStr(ctx, dong_obj, "textLayout",
                          JS_NewCFunction(ctx, js_dong_textLayout, "textLayout", 1));
        JS_FreeValue(ctx, dong_obj);
    }
    JS_FreeValue(ctx, global);
}

} // namespace dong::script
