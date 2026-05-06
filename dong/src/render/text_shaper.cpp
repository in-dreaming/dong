#include "text_shaper.hpp"

#include "font_metrics.hpp"
#include "font_resolver.hpp"

#include <hb-ft.h>
#include <hb.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../core/log.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace dong::render {

namespace {

// Approximate CSS `line-height: normal` as ~1.2 * font-size.
constexpr float kDefaultLineHeightMultiplier = 1.2f;

// Helper function to set language on HarfBuzz buffer
void setHbLanguage(hb_buffer_t* buffer, const std::string& lang) {
    if (lang.empty()) {
        return;  // Use default language guessing
    }

    // HarfBuzz expects BCP 47 language tags (e.g., "en-US", "tr")
    // The lang attribute already follows BCP 47 format
    hb_language_t hb_lang = hb_language_from_string(lang.c_str(), -1);
    if (hb_lang != HB_LANGUAGE_INVALID) {
        hb_buffer_set_language(buffer, hb_lang);
    }
}

struct Utf8DecodeResult {
    uint32_t codepoint = 0;
    uint8_t length = 0;
    bool ok = false;
};

Utf8DecodeResult decodeUtf8At(const std::string& text, size_t byte_index) {
    Utf8DecodeResult r{};
    if (byte_index >= text.size()) {
        return r;
    }

    const uint8_t b0 = static_cast<uint8_t>(text[byte_index]);
    if ((b0 & 0x80) == 0) {
        r.codepoint = b0;
        r.length = 1;
        r.ok = true;
        return r;
    }

    int needed = 0;
    uint32_t cp = 0;
    if ((b0 & 0xE0) == 0xC0) {
        needed = 2;
        cp = (b0 & 0x1F);
    } else if ((b0 & 0xF0) == 0xE0) {
        needed = 3;
        cp = (b0 & 0x0F);
    } else if ((b0 & 0xF8) == 0xF0) {
        needed = 4;
        cp = (b0 & 0x07);
    } else {
        // Invalid leading byte.
        r.length = 1;
        return r;
    }

    if (byte_index + static_cast<size_t>(needed) > text.size()) {
        // Truncated sequence.
        r.length = 1;
        return r;
    }

    for (int i = 1; i < needed; ++i) {
        const uint8_t bx = static_cast<uint8_t>(text[byte_index + static_cast<size_t>(i)]);
        if ((bx & 0xC0) != 0x80) {
            r.length = 1;
            return r;
        }
        cp = (cp << 6) | (bx & 0x3F);
    }

    r.codepoint = cp;
    r.length = static_cast<uint8_t>(needed);
    r.ok = true;
    return r;
}

uint32_t extractCodepoint(const std::string& text, size_t byte_index) {
    Utf8DecodeResult r = decodeUtf8At(text, byte_index);
    return r.ok ? r.codepoint : 0;
}

// Return a conservative byte length for stepping through a UTF-8 string.
size_t utf8CharLen(uint8_t first_byte) {
    if ((first_byte & 0x80) == 0) return 1;
    if ((first_byte & 0xE0) == 0xC0) return 2;
    if ((first_byte & 0xF0) == 0xE0) return 3;
    if ((first_byte & 0xF8) == 0xF0) return 4;
    return 1;
}

struct ShaperDebugFlags {
    bool glyphs = false;
    bool ft = false;
};

ShaperDebugFlags getDebugFlags() {
    ShaperDebugFlags f{};
    f.glyphs = (std::getenv("DONG_DEBUG_TEXT_SHAPER_GLYPHS") != nullptr);
    f.ft = (std::getenv("DONG_DEBUG_TEXT_SHAPER_FT") != nullptr);
    return f;
}

struct CachedFontInfo {
    FT_Face face = nullptr;
    hb_font_t* hb_font = nullptr;
    uint32_t units_per_em = 0;
};

using FontCache = std::unordered_map<std::string, CachedFontInfo>;
FontCache g_font_cache;

CachedFontInfo* getOrCreateFontInfo(const std::string& font_path) {
    auto it = g_font_cache.find(font_path);
    if (it != g_font_cache.end()) {
        return &it->second;
    }

    FT_Face face = getOrCreateDesignUnitsFace(font_path);
    if (!face) {
        return nullptr;
    }

    hb_font_t* hb_font = hb_ft_font_create_referenced(face);
    if (!hb_font) {
        return nullptr;
    }

    // hb_ft_font_create_referenced() already attaches hb-ft font funcs and uses the provided FT_Face.
    // Do NOT call hb_ft_font_set_funcs() here: that API is for hb_font_t created from hb_face_t and it
    // will create a new internal FT_Face, which breaks metrics for our cached face.

    const uint32_t units_per_em = face->units_per_EM;
    hb_font_set_scale(hb_font, static_cast<int>(units_per_em), static_cast<int>(units_per_em));

    CachedFontInfo info;
    info.face = face;
    info.hb_font = hb_font;
    info.units_per_em = units_per_em;

    g_font_cache[font_path] = info;
    return &g_font_cache[font_path];
}

struct HbBufferHolder {
    hb_buffer_t* buffer = nullptr;
    HbBufferHolder() {
        buffer = hb_buffer_create();
    }
    ~HbBufferHolder() {
        if (buffer) {
            hb_buffer_destroy(buffer);
            buffer = nullptr;
        }
    }
};

hb_buffer_t* getThreadLocalHbBuffer() {
    thread_local HbBufferHolder tl_buffer;
    hb_buffer_t* buffer = tl_buffer.buffer;
    hb_buffer_reset(buffer);
    return buffer;
}

void addUtf8SpanToHbBuffer(hb_buffer_t* buffer, const char* ptr, int span_len) {
    hb_buffer_add_utf8(buffer, ptr, span_len, 0, span_len);
    hb_buffer_guess_segment_properties(buffer);
}

bool hbHasAnyNonZeroAdvance(const hb_glyph_position_t* positions, unsigned glyph_count) {
    for (unsigned i = 0; i < glyph_count; ++i) {
        if (positions[i].x_advance != 0 || positions[i].y_advance != 0) {
            return true;
        }
    }
    return false;
}

bool shouldFallbackToFreeTypePositions(const hb_glyph_position_t* positions,
                                      unsigned glyph_count,
                                      uint32_t primary_units_per_em,
                                      uint32_t seg_units_per_em) {
    if (glyph_count == 0) {
        return false;
    }

    const bool any_nonzero_adv = hbHasAnyNonZeroAdvance(positions, glyph_count);
    const bool use_ft_positions = !any_nonzero_adv;
    if (!use_ft_positions) {
        return false;
    }

    static bool s_warned = false;
    if (!s_warned) {
        DONG_LOG_WARN("[shapeSpan] HarfBuzz produced zero advances; falling back to FreeType NO_SCALE metrics (primary_upem=%u seg_upem=%u)",
                      primary_units_per_em, seg_units_per_em);
        s_warned = true;
    }
    return true;
}

float computeUnitsScale(uint32_t primary_units_per_em, uint32_t seg_units_per_em) {
    if (seg_units_per_em == 0 || primary_units_per_em == 0) {
        return 1.0f;
    }
    return static_cast<float>(primary_units_per_em) / static_cast<float>(seg_units_per_em);
}

uint32_t mapEmojiCodepointToOutlineFallback(uint32_t codepoint) {
    // For engines without COLR/bitmap emoji rendering, map a few common emoji
    // to outline-capable symbols so UI remains legible and interactive.
    switch (codepoint) {
        case 0x1F525: return 0x2668; // FIRE -> HOT SPRINGS
        case 0x1F480: return 0x2620; // SKULL -> SKULL AND CROSSBONES
        default: return codepoint;
    }
}

bool hasOutlineGlyph(FT_Face face, FT_UInt glyph_id) {
    if (!face || glyph_id == 0) return false;

    // Color fonts (COLR v0/v1, CBDT, sbix) are handled by bitmap fallback in renderer
    if (FT_HAS_COLOR(face)) {
        return true;
    }

    if (FT_Load_Glyph(face, glyph_id, FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE) != 0 || !face->glyph) {
        return false;
    }
    return face->glyph->format == FT_GLYPH_FORMAT_OUTLINE;
}

uint32_t chooseGlyphIdForFreeType(CachedFontInfo* font_info, uint32_t hb_gid, uint32_t unicode_codepoint) {
    if (!font_info || !font_info->face) {
        return hb_gid;
    }

    uint32_t glyph_id = hb_gid;

    if (unicode_codepoint != 0) {
        // Skip emoji-to-outline fallback for COLR fonts - they render natively via layers
        const bool font_has_colr = FT_HAS_COLOR(font_info->face);

        if (!font_has_colr) {
            const uint32_t mapped = mapEmojiCodepointToOutlineFallback(unicode_codepoint);
            if (mapped != unicode_codepoint) {
                const FT_UInt mapped_gid = FT_Get_Char_Index(font_info->face, mapped);
                if (mapped_gid != 0 && hasOutlineGlyph(font_info->face, mapped_gid)) {
                    glyph_id = static_cast<uint32_t>(mapped_gid);
                }
            }
        }

        const FT_UInt ft_gid = FT_Get_Char_Index(font_info->face, unicode_codepoint);
        if (glyph_id == hb_gid && ft_gid != 0 && hasOutlineGlyph(font_info->face, ft_gid)) {
            glyph_id = static_cast<uint32_t>(ft_gid);
        }
    }

    if (glyph_id >= static_cast<uint32_t>(font_info->face->num_glyphs)) {
        glyph_id = 0;
    }

    return glyph_id;
}

float computeFreeTypeAdvanceXUnits(CachedFontInfo* font_info, uint32_t glyph_id, float units_scale, const ShaperDebugFlags& dbg) {
    if (!font_info || !font_info->face || glyph_id == 0) {
        return 0.0f;
    }

    // With FT_LOAD_NO_SCALE, FreeType reports advances in font units (not 26.6).
    if (FT_Load_Glyph(font_info->face, glyph_id, FT_LOAD_NO_SCALE) != 0 || !font_info->face->glyph) {
        return 0.0f;
    }

    if (dbg.ft) {
        DONG_LOG_DEBUG("[shapeSpan] FT_LOAD_NO_SCALE raw advance.x=%ld", (long)font_info->face->glyph->advance.x);
    }

    return static_cast<float>(font_info->face->glyph->advance.x) * units_scale;
}

ShapedGlyph makeShapedGlyph(uint32_t glyph_id,
                            float pen_x_units,
                            float pen_y_units,
                            float x_offset_units,
                            float y_offset_units,
                            float x_advance_units,
                            uint32_t cluster_byte_index,
                            uint16_t font_path_index,
                            uint32_t units_per_em) {
    ShapedGlyph glyph{};
    glyph.glyph_id = glyph_id;
    glyph.pen_x_units = pen_x_units + x_offset_units;
    glyph.pen_y_units = pen_y_units - y_offset_units;
    glyph.advance_x_units = x_advance_units;
    glyph.cluster = cluster_byte_index;
    glyph.font_path_index = font_path_index;
    glyph.units_per_em = units_per_em;
    return glyph;
}

void appendHbGlyphRun(const std::string& text,
                      size_t byte_start,
                      const hb_glyph_info_t* infos,
                      const hb_glyph_position_t* positions,
                      unsigned glyph_count,
                      CachedFontInfo* font_info,
                      uint16_t font_path_index,
                      bool use_ft_positions,
                      float units_scale,
                      const ShaperDebugFlags& dbg,
                      float& pen_x_units,
                      float& pen_y_units,
                      std::vector<ShapedGlyph>& out_glyphs) {
    for (unsigned i = 0; i < glyph_count; ++i) {
        const hb_glyph_info_t& info = infos[i];
        const hb_glyph_position_t& pos = positions[i];

        const uint32_t unicode_codepoint = extractCodepoint(text, byte_start + info.cluster);

        uint32_t glyph_id = chooseGlyphIdForFreeType(font_info, info.codepoint, unicode_codepoint);
        const float x_offset_units = static_cast<float>(pos.x_offset) * units_scale;
        const float y_offset_units = static_cast<float>(pos.y_offset) * units_scale;
        const float y_advance_units = static_cast<float>(pos.y_advance) * units_scale;

        float x_advance_units = 0.0f;
        if (!use_ft_positions) {
            x_advance_units = static_cast<float>(pos.x_advance) * units_scale;
        } else {
            x_advance_units = computeFreeTypeAdvanceXUnits(font_info, glyph_id, units_scale, dbg);
        }

        if (dbg.glyphs) {
            DONG_LOG_DEBUG("[shapeSpan] glyph_id=%u unicode=U+%04X cluster=%u adv=%.2f",
                           glyph_id, unicode_codepoint, info.cluster, x_advance_units);
        }

        if (x_advance_units > 100000.0f || x_advance_units < -100000.0f) {
            DONG_LOG_INFO("[shapeSpan] WARN: glyph_id=%u x_advance_units=%.1f (raw=%d scale=%.3f)",
                          glyph_id, x_advance_units, pos.x_advance, units_scale);
        }

        out_glyphs.push_back(makeShapedGlyph(
            glyph_id,
            pen_x_units,
            pen_y_units,
            x_offset_units,
            y_offset_units,
            x_advance_units,
            static_cast<uint32_t>(byte_start + info.cluster),
            font_path_index,
            font_info->units_per_em));

        pen_x_units += x_advance_units;
        pen_y_units -= y_advance_units;
    }
}

// Shape a UTF-8 byte span using a single HarfBuzz call.
// primary_units_per_em: the primary font units-per-em for mapping fallback font advances into a unified unit space.
bool shapeSpan(const std::string& text,
               size_t byte_start,
               size_t byte_end,
               CachedFontInfo* font_info,
               uint16_t font_path_index,
               uint32_t primary_units_per_em,
               float& pen_x_units,
               float& pen_y_units,
               std::vector<ShapedGlyph>& out_glyphs,
               const std::string& lang = "") {
    if (!font_info || !font_info->hb_font) {
        return false;
    }
    if (byte_end <= byte_start) {
        return true;
    }

    const int span_len = static_cast<int>(byte_end - byte_start);
    hb_buffer_t* buffer = getThreadLocalHbBuffer();
    addUtf8SpanToHbBuffer(buffer, text.c_str() + byte_start, span_len);

    // Set language for HarfBuzz shaping
    setHbLanguage(buffer, lang);

    hb_shape(font_info->hb_font, buffer, nullptr, 0);

    unsigned glyph_count = 0;
    const hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    const hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, &glyph_count);

    if (glyph_count > 0) {
        out_glyphs.reserve(out_glyphs.size() + static_cast<size_t>(glyph_count));
    }

    const ShaperDebugFlags dbg = getDebugFlags();
    const bool use_ft_positions = shouldFallbackToFreeTypePositions(positions, glyph_count, primary_units_per_em, font_info->units_per_em);
    const float units_scale = computeUnitsScale(primary_units_per_em, font_info->units_per_em);

    appendHbGlyphRun(text,
                     byte_start,
                     infos,
                     positions,
                     glyph_count,
                     font_info,
                     font_path_index,
                     use_ft_positions,
                     units_scale,
                     dbg,
                     pen_x_units,
                     pen_y_units,
                     out_glyphs);

    return true;
}

struct FontChoice {
    std::string_view path;
    CachedFontInfo* info = nullptr;
};

class FontChooser {
public:
    FontChooser(const std::string& primary_path,
                CachedFontInfo* primary_font,
                const std::vector<std::string>& fallbacks,
                std::vector<std::string>& out_font_paths)
        : primary_path_(primary_path)
        , primary_font_(primary_font)
        , fallbacks_(fallbacks)
        , out_font_paths_(out_font_paths) {}

    FontChoice choose(uint32_t codepoint) {
        if (!primary_font_ || !primary_font_->face) {
            return {std::string_view(primary_path_), primary_font_};
        }

        if (codepoint <= 127) {
            return {std::string_view(primary_path_), primary_font_};
        }

        const FT_UInt glyph_index = FT_Get_Char_Index(primary_font_->face, codepoint);
        if (glyph_index != 0) {
            return {std::string_view(primary_path_), primary_font_};
        }

        for (const auto& fallback_path : fallbacks_) {
            CachedFontInfo* fallback_font = getOrCreateFontInfo(fallback_path);
            if (!fallback_font || !fallback_font->face) {
                continue;
            }

            const FT_UInt fallback_glyph = FT_Get_Char_Index(fallback_font->face, codepoint);
            if (fallback_glyph != 0) {
                DONG_LOG_DEBUG("[TextShaper] Fallback for U+%04X: '%s' (units_per_em=%u, primary=%u)",
                               codepoint,
                               fallback_path.c_str(),
                               fallback_font->units_per_em,
                               primary_font_->units_per_em);
                return {std::string_view(fallback_path), fallback_font};
            }
        }

        return {std::string_view(primary_path_), primary_font_};
    }

    uint16_t indexFor(std::string_view path) {
        for (uint16_t idx = 0; idx < static_cast<uint16_t>(out_font_paths_.size()); ++idx) {
            if (out_font_paths_[idx] == path) {
                return idx;
            }
        }
        out_font_paths_.push_back(std::string(path));
        return static_cast<uint16_t>(out_font_paths_.size() - 1);
    }

private:
    const std::string& primary_path_;
    CachedFontInfo* primary_font_ = nullptr;
    const std::vector<std::string>& fallbacks_;
    std::vector<std::string>& out_font_paths_;
};

void shapeByFontSegments(const std::string& text,
                         FontChooser& chooser,
                         uint32_t primary_units_per_em,
                         float& pen_x_units,
                         float& pen_y_units,
                         std::vector<ShapedGlyph>& out_glyphs,
                         const std::string& lang = "") {
    size_t seg_start = 0;
    size_t i = 0;

    FontChoice seg_choice = chooser.choose(extractCodepoint(text, 0));
    uint16_t seg_font_index = chooser.indexFor(seg_choice.path);

    while (i < text.size()) {
        const size_t char_len = utf8CharLen(static_cast<uint8_t>(text[i]));
        const size_t char_end = std::min(i + char_len, text.size());
        const uint32_t codepoint = extractCodepoint(text, i);

        FontChoice choice = chooser.choose(codepoint);
        if (choice.path != seg_choice.path) {
            if (i > seg_start) {
                (void)shapeSpan(text, seg_start, i, seg_choice.info, seg_font_index, primary_units_per_em,
                                pen_x_units, pen_y_units, out_glyphs, lang);
            }
            seg_start = i;
            seg_choice = choice;
            seg_font_index = chooser.indexFor(seg_choice.path);
        }

        i = char_end;
    }

    if (i > seg_start) {
        (void)shapeSpan(text, seg_start, i, seg_choice.info, seg_font_index, primary_units_per_em,
                        pen_x_units, pen_y_units, out_glyphs, lang);
    }
}

void applyPrimaryFontMetrics(CachedFontInfo* primary_font, const std::string& primary_font_path, ShapedText& out_text) {
    UnifiedFontMetrics font_metrics;
    if (primary_font && primary_font->face && getFontMetrics(primary_font->face, font_metrics)) {
        out_text.ascent_units = font_metrics.ascent_units;
        out_text.descent_units = font_metrics.descent_units;
        out_text.line_height_units = font_metrics.height_units;

        DONG_LOG_DEBUG("[TextShaper] font='%s' units_per_em=%u ascent=%.1f descent=%.1f line_height=%.1f",
                       primary_font_path.c_str(),
                       font_metrics.units_per_em,
                       font_metrics.ascent_units,
                       font_metrics.descent_units,
                       font_metrics.height_units);
    }

    if (out_text.line_height_units <= 0.0f && primary_font) {
        out_text.line_height_units = static_cast<float>(primary_font->units_per_em) * kDefaultLineHeightMultiplier;
    }

    if (out_text.ascent_units <= 0.0f && primary_font) {
        out_text.ascent_units = static_cast<float>(primary_font->units_per_em);
    }
}

} // namespace

bool TextShaper::shape(const TextShapeRequest& request, ShapedText& out_text) {
    out_text = {};
    if (request.text.empty()) {
        return false;
    }

    ShapedCacheKey cache_key{request.text, request.font_family, request.font_weight,
                            request.font_style, request.font_size, request.lang};
    auto& cache = getShapedCache();
    auto it = cache.find(cache_key);
    if (it != cache.end()) {
        out_text = it->second;
        return true;
    }

    const std::string primary_font_path = resolveFontPath(request.font_family, request.font_weight, request.font_style);
    if (primary_font_path.empty()) {
        DONG_LOG_INFO("TextShaper: failed to resolve font family '%s'", request.font_family.c_str());
        return false;
    }

    CachedFontInfo* primary_font = getOrCreateFontInfo(primary_font_path);
    if (!primary_font) {
        DONG_LOG_INFO("TextShaper: failed to load primary font '%s'", primary_font_path.c_str());
        return false;
    }

    out_text.font_paths.clear();
    out_text.font_path = primary_font_path;
    out_text.font_paths.push_back(primary_font_path);

    out_text.units_per_em = primary_font->units_per_em;
    out_text.scale_to_pixels = request.font_size / static_cast<float>(primary_font->units_per_em);
    out_text.glyphs.clear();

    float pen_x_units = 0.0f;
    float pen_y_units = 0.0f;

    std::vector<std::string> fallback_fonts = getEmojiFallbackFonts();
    const std::vector<std::string>& cjk_fallbacks = getCJKFallbackFonts();
    fallback_fonts.insert(fallback_fonts.end(), cjk_fallbacks.begin(), cjk_fallbacks.end());
    FontChooser chooser(primary_font_path, primary_font, fallback_fonts, out_text.font_paths);

    shapeByFontSegments(request.text, chooser, primary_font->units_per_em, pen_x_units, pen_y_units, out_text.glyphs, request.lang);

    applyPrimaryFontMetrics(primary_font, primary_font_path, out_text);

    out_text.width_units = pen_x_units;

    if (cache.size() >= kMaxShapedCacheEntries) {
        cache.clear();
    }
    cache[cache_key] = out_text;

    return true;
}

} // namespace dong::render
