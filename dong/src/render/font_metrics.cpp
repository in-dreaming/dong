#include "font_metrics.hpp"

#include <SDL3/SDL_log.h>

namespace dong::render {

namespace {

FT_Library g_ft_library = nullptr;
bool g_ft_initialized = false;

// key: font_path#pixel_size
std::unordered_map<std::string, FT_Face> g_face_cache;

std::string makeFaceKey(const std::string& font_path, uint32_t pixel_size) {
    std::string key = font_path;
    key.push_back('#');
    key.append(std::to_string(pixel_size));
    return key;
}

bool ensureLibraryInitialized() {
    if (g_ft_initialized) {
        return g_ft_library != nullptr;
    }

    if (FT_Init_FreeType(&g_ft_library) != 0) {
        SDL_Log("FontMetrics: failed to initialize FreeType library");
        g_ft_library = nullptr;
        g_ft_initialized = true;
        return false;
    }

    g_ft_initialized = true;
    return true;
}

} // namespace

bool initializeFontLibrary() {
    return ensureLibraryInitialized();
}

FT_Library getSharedFreeTypeLibrary() {
    if (!ensureLibraryInitialized()) {
        return nullptr;
    }
    return g_ft_library;
}

FT_Face getOrCreateFontFace(const std::string& font_path, uint32_t pixel_size) {
    if (font_path.empty()) {
        return nullptr;
    }
    if (!ensureLibraryInitialized()) {
        return nullptr;
    }

    const std::string key = makeFaceKey(font_path, pixel_size);
    auto it = g_face_cache.find(key);
    if (it != g_face_cache.end()) {
        return it->second;
    }

    FT_Face face = nullptr;
    if (FT_New_Face(g_ft_library, font_path.c_str(), 0, &face) != 0) {
        SDL_Log("FontMetrics: failed to load font face '%s'", font_path.c_str());
        return nullptr;
    }

    FT_Set_Pixel_Sizes(face, 0, pixel_size);

    g_face_cache.emplace(key, face);
    return face;
}

} // namespace dong::render
