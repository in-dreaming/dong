// Dong Glyph Atlas Interface
// Abstract interface for glyph atlas operations
//
// This allows the Core layer to use glyph atlases without depending on
// the concrete implementation (which may use SDL GPU or other backends).

#ifndef DONG_GLYPH_ATLAS_H
#define DONG_GLYPH_ATLAS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle to a glyph atlas
typedef struct dong_glyph_atlas_t dong_glyph_atlas_t;

// Opaque handle to a font face
typedef struct dong_font_face_t dong_font_face_t;

// Glyph metrics (design units)
typedef struct dong_glyph_metrics_t {
    float advance_x_units;
    float bearing_x_units;
    float bearing_y_units;
    float width_units;
    float height_units;
    
    // MSDF metadata
    float msdf_scale;
    float msdf_translate_x;
    float msdf_translate_y;
    
    // Bounds
    float bounds_left;
    float bounds_bottom;
    float bounds_right;
    float bounds_top;
    
    // Logical bbox
    float logical_left;
    float logical_bottom;
    float logical_right;
    float logical_top;
    
    uint32_t units_per_em;
} dong_glyph_metrics_t;

// Atlas entry (UV coordinates + metrics)
typedef struct dong_atlas_entry_t {
    uint32_t atlas_page;
    float u0, v0, u1, v1;
    dong_glyph_metrics_t metrics;
} dong_atlas_entry_t;

// Font information
typedef struct dong_font_info_t {
    const char* family_name;
    float size;
    float weight;
} dong_font_info_t;

// =============================================================================
// GlyphAtlas Interface VTable
// =============================================================================

typedef struct dong_glyph_atlas_vtable_t {
    // Create/Destroy
    dong_glyph_atlas_t* (*create)(void* gpu_driver, uint32_t width, uint32_t height);
    void (*destroy)(dong_glyph_atlas_t* atlas);
    
    // Initialize/Cleanup
    bool (*initialize)(dong_glyph_atlas_t* atlas);
    void (*cleanup)(dong_glyph_atlas_t* atlas);
    
    // Glyph operations
    bool (*get_or_render_glyph)(
        dong_glyph_atlas_t* atlas,
        dong_font_face_t* font,
        uint32_t glyph_index,
        uint32_t pixel_size,
        dong_atlas_entry_t* out_entry
    );
    
    // Check if glyph exists in atlas
    bool (*has_glyph)(
        dong_glyph_atlas_t* atlas,
        dong_font_face_t* font,
        uint32_t glyph_index,
        uint32_t pixel_size
    );
    
    // Get atlas texture for rendering
    void* (*get_atlas_texture)(dong_glyph_atlas_t* atlas, uint32_t page_index);
    uint32_t (*get_page_count)(dong_glyph_atlas_t* atlas);
    
    // Clear atlas
    void (*clear)(dong_glyph_atlas_t* atlas);
    
    // Stats
    uint32_t (*get_glyph_count)(dong_glyph_atlas_t* atlas);
    float (*get_usage_ratio)(dong_glyph_atlas_t* atlas);
} dong_glyph_atlas_vtable_t;

// =============================================================================
// FontFace Interface VTable
// =============================================================================

typedef struct dong_font_face_vtable_t {
    // Load font from file
    dong_font_face_t* (*load_from_file)(const char* path, uint32_t face_index);
    
    // Load font from memory
    dong_font_face_t* (*load_from_memory)(
        const uint8_t* data,
        size_t data_size,
        uint32_t face_index
    );
    
    // Destroy font face
    void (*destroy)(dong_font_face_t* face);
    
    // Get font info
    bool (*get_info)(dong_font_face_t* face, dong_font_info_t* out_info);
    
    // Get glyph index for character
    uint32_t (*get_glyph_index)(dong_font_face_t* face, uint32_t charcode);
    
    // Get metrics
    bool (*get_metrics)(dong_font_face_t* face, dong_glyph_metrics_t* out_metrics);
} dong_font_face_vtable_t;

// =============================================================================
// Global Interface Registration
// =============================================================================

// Register glyph atlas implementation (call during platform init)
void dong_glyph_atlas_set_vtable(const dong_glyph_atlas_vtable_t* vtable);
void dong_font_face_set_vtable(const dong_font_face_vtable_t* vtable);

// Get current vtable (for internal use)
const dong_glyph_atlas_vtable_t* dong_glyph_atlas_get_vtable(void);
const dong_font_face_vtable_t* dong_font_face_get_vtable(void);

// =============================================================================
// Convenience Functions
// =============================================================================

// Create atlas using registered vtable
dong_glyph_atlas_t* dong_glyph_atlas_create(void* gpu_driver, uint32_t width, uint32_t height);
void dong_glyph_atlas_destroy(dong_glyph_atlas_t* atlas);

// Load font using registered vtable
dong_font_face_t* dong_font_face_load(const char* path);
void dong_font_face_free(dong_font_face_t* face);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // DONG_GLYPH_ATLAS_H
