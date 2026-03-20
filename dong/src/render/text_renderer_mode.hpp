#pragma once

#include <cstdint>

namespace dong::render {

// Text rendering backend selection mode.
// Controls which algorithm is used to render glyph runs.
enum class TextRendererMode : uint8_t {
    Auto,   // Engine picks the best available (currently defaults to MSDF)
    Msdf,   // Force MSDF atlas-based rendering
    Slug,   // Force Slug analytical rendering (falls back to MSDF if unavailable)
};

// Result of resolving a text renderer mode for a specific glyph run.
struct TextRendererSelection {
    TextRendererMode requested = TextRendererMode::Auto;
    TextRendererMode resolved = TextRendererMode::Msdf;
    bool fallback_used = false;
    const char* reason = nullptr;  // human-readable reason (null if no fallback)
};

// Convert mode to string for logging.
inline const char* textRendererModeToString(TextRendererMode mode) {
    switch (mode) {
        case TextRendererMode::Auto: return "Auto";
        case TextRendererMode::Msdf: return "MSDF";
        case TextRendererMode::Slug: return "Slug";
    }
    return "Unknown";
}

} // namespace dong::render
