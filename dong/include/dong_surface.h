#ifndef DONG_SURFACE_H
#define DONG_SURFACE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DLL export/import macros
// =============================================================================
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_BUILDING_DLL
        #define DONG_SURFACE_API __declspec(dllexport)
    #else
        #define DONG_SURFACE_API __declspec(dllimport)
    #endif
#else
    #define DONG_SURFACE_API __attribute__((visibility("default")))
#endif

// =============================================================================
// Surface Abstraction
// =============================================================================
// A Surface represents a render target - either GPU texture or CPU buffer.
// Surfaces are created via SurfaceFactory and used by the rendering system.
//
// The Surface interface allows the core engine to work with any backend's
// render targets without knowing the implementation details.
// =============================================================================

// Forward declarations
typedef struct DongSurface DongSurface;
typedef struct DongSurfaceFactory DongSurfaceFactory;

// Include GPU types for texture handle
#include "dong_gpu_driver.h"

// =============================================================================
// Surface Types
// =============================================================================

typedef enum DongSurfaceType {
    DONG_SURFACE_TYPE_CPU_BUFFER = 0,   // CPU-side pixel buffer (RGBA8)
    DONG_SURFACE_TYPE_GPU_TEXTURE = 1,  // GPU texture render target
} DongSurfaceType;

typedef enum DongSurfaceFlags {
    DONG_SURFACE_FLAG_NONE = 0,
    DONG_SURFACE_FLAG_READABLE = 1 << 0,  // Can read back pixels to CPU
    DONG_SURFACE_FLAG_PRESENTABLE = 1 << 1,  // Can present to screen
} DongSurfaceFlags;

// =============================================================================
// Surface Descriptor
// =============================================================================

typedef struct DongSurfaceDesc {
    uint32_t width;
    uint32_t height;
    DongSurfaceType type;
    uint32_t flags;  // Bitmask of DongSurfaceFlags
    const char* debug_name;  // Optional debug label
} DongSurfaceDesc;

// =============================================================================
// Dirty Bounds
// =============================================================================

typedef struct DongDirtyBounds {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    int valid;  // Non-zero if bounds are valid/dirty
} DongDirtyBounds;

// =============================================================================
// Surface Virtual Table
// =============================================================================

typedef struct DongSurfaceVTable {
    // Properties
    DongSurfaceType (*get_type)(DongSurface* surface);
    uint32_t (*get_width)(DongSurface* surface);
    uint32_t (*get_height)(DongSurface* surface);
    uint32_t (*get_stride)(DongSurface* surface);  // Bytes per row

    // GPU Texture Access (only valid for GPU_TEXTURE type)
    DongGPUTexture (*get_texture)(DongSurface* surface);

    // CPU Buffer Access (only valid for CPU_BUFFER type, or GPU with readback)
    void* (*lock_pixels)(DongSurface* surface);
    void (*unlock_pixels)(DongSurface* surface);

    // Dirty tracking
    void (*mark_dirty)(DongSurface* surface);
    void (*mark_dirty_rect)(DongSurface* surface, int32_t x, int32_t y, int32_t w, int32_t h);
    int (*is_dirty)(DongSurface* surface);
    void (*get_dirty_bounds)(DongSurface* surface, DongDirtyBounds* out_bounds);
    void (*clear_dirty)(DongSurface* surface);

    // Clear surface to a solid color
    void (*clear)(DongSurface* surface, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    // Resize surface (may reallocate underlying storage)
    int (*resize)(DongSurface* surface, uint32_t new_width, uint32_t new_height);
} DongSurfaceVTable;

// =============================================================================
// Surface Structure
// =============================================================================

struct DongSurface {
    const DongSurfaceVTable* vtable;
    void* user_data;  // Backend-specific data
};

// =============================================================================
// Surface Factory Virtual Table
// =============================================================================

typedef struct DongSurfaceFactoryVTable {
    // Create a new surface
    DongSurface* (*create_surface)(DongSurfaceFactory* factory, const DongSurfaceDesc* desc);

    // Destroy a surface (free resources)
    void (*destroy_surface)(DongSurfaceFactory* factory, DongSurface* surface);

    // Create a surface backed by an external GPU texture
    DongSurface* (*create_surface_from_texture)(DongSurfaceFactory* factory,
                                                 DongGPUTexture texture,
                                                 uint32_t width, uint32_t height);
} DongSurfaceFactoryVTable;

// =============================================================================
// Surface Factory Structure
// =============================================================================

struct DongSurfaceFactory {
    const DongSurfaceFactoryVTable* vtable;
    void* user_data;  // Backend-specific data (e.g., GPU device reference)
};

// =============================================================================
// Convenience Functions (call through vtable)
// =============================================================================

static inline DongSurfaceType dong_surface_get_type(DongSurface* s) {
    return (s && s->vtable && s->vtable->get_type) ? s->vtable->get_type(s) : DONG_SURFACE_TYPE_CPU_BUFFER;
}

static inline uint32_t dong_surface_get_width(DongSurface* s) {
    return (s && s->vtable && s->vtable->get_width) ? s->vtable->get_width(s) : 0;
}

static inline uint32_t dong_surface_get_height(DongSurface* s) {
    return (s && s->vtable && s->vtable->get_height) ? s->vtable->get_height(s) : 0;
}

static inline uint32_t dong_surface_get_stride(DongSurface* s) {
    return (s && s->vtable && s->vtable->get_stride) ? s->vtable->get_stride(s) : 0;
}

static inline DongGPUTexture dong_surface_get_texture(DongSurface* s) {
    return (s && s->vtable && s->vtable->get_texture) ? s->vtable->get_texture(s) : NULL;
}

static inline void* dong_surface_lock_pixels(DongSurface* s) {
    return (s && s->vtable && s->vtable->lock_pixels) ? s->vtable->lock_pixels(s) : NULL;
}

static inline void dong_surface_unlock_pixels(DongSurface* s) {
    if (s && s->vtable && s->vtable->unlock_pixels) s->vtable->unlock_pixels(s);
}

static inline void dong_surface_mark_dirty(DongSurface* s) {
    if (s && s->vtable && s->vtable->mark_dirty) s->vtable->mark_dirty(s);
}

static inline int dong_surface_is_dirty(DongSurface* s) {
    return (s && s->vtable && s->vtable->is_dirty) ? s->vtable->is_dirty(s) : 0;
}

static inline void dong_surface_clear_dirty(DongSurface* s) {
    if (s && s->vtable && s->vtable->clear_dirty) s->vtable->clear_dirty(s);
}

static inline void dong_surface_clear(DongSurface* s, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (s && s->vtable && s->vtable->clear) s->vtable->clear(s, r, g, b, a);
}

static inline int dong_surface_resize(DongSurface* s, uint32_t w, uint32_t h) {
    return (s && s->vtable && s->vtable->resize) ? s->vtable->resize(s, w, h) : 0;
}

// Factory convenience functions
static inline DongSurface* dong_surface_factory_create(DongSurfaceFactory* f, const DongSurfaceDesc* desc) {
    return (f && f->vtable && f->vtable->create_surface) ? f->vtable->create_surface(f, desc) : NULL;
}

static inline void dong_surface_factory_destroy(DongSurfaceFactory* f, DongSurface* s) {
    if (f && f->vtable && f->vtable->destroy_surface) f->vtable->destroy_surface(f, s);
}

#ifdef __cplusplus
}
#endif

#endif // DONG_SURFACE_H
