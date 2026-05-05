/**
 * @file dong_resource_pack.h
 * @brief GPU Resource Pack API (P2-8)
 *
 * Pre-packaged binary format (.dpkg) containing fonts, glyph atlas data,
 * and pre-compiled shaders for fast startup. Loading a resource pack avoids
 * runtime font discovery, font file parsing, and shader compilation on
 * first frame.
 *
 * File format (.dpkg):
 *   Header (32 bytes):
 *     - magic: "DPKG" (4 bytes)
 *     - version: uint32 (currently 1)
 *     - entry_count: uint32
 *     - flags: uint32 (reserved)
 *     - reserved: 16 bytes
 *   Entry table (entry_count * 32 bytes each):
 *     - type: uint32 (FONT=1, ATLAS=2, SHADER=3)
 *     - name_offset: uint32 (offset into string table)
 *     - data_offset: uint64 (offset into data section)
 *     - data_size: uint64 (compressed size)
 *     - original_size: uint64 (uncompressed size, 0 = not compressed)
 *   String table (variable length, null-terminated strings)
 *   Data section (concatenated binary blobs)
 *
 * Usage:
 *   dong_resource_pack_t* pack = dong_resource_pack_load("game_ui.dpkg");
 *   dong_engine_attach_resource_pack(engine, pack);
 *   // ... engine now uses pre-loaded resources instead of runtime discovery
 *   dong_resource_pack_free(pack);
 *
 * Packing tool:
 *   python scripts/tools/pack_resources.py --fonts fonts/ --shaders shaders/ -o game_ui.dpkg
 */

#ifndef DONG_RESOURCE_PACK_H
#define DONG_RESOURCE_PACK_H

#include "dong.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dong_resource_pack dong_resource_pack_t;

/** Resource entry types. */
typedef enum {
    DONG_RESOURCE_FONT    = 1,  /**< TrueType/OpenType font file */
    DONG_RESOURCE_ATLAS   = 2,  /**< Pre-built glyph atlas texture data */
    DONG_RESOURCE_SHADER  = 3,  /**< Pre-compiled shader bytecode (SPIRV/DXIL/MSL) */
    DONG_RESOURCE_IMAGE   = 4,  /**< Pre-decoded image (PNG/JPEG → raw RGBA) */
} dong_resource_type_t;

/**
 * Load a resource pack from a file path.
 * Returns NULL on failure (file not found, invalid format, version mismatch).
 */
DONG_API dong_resource_pack_t* dong_resource_pack_load(const char* path);

/**
 * Load a resource pack from memory.
 * The data pointer must remain valid for the lifetime of the pack (zero-copy).
 */
DONG_API dong_resource_pack_t* dong_resource_pack_load_memory(const void* data, uint64_t size);

/** Free a resource pack. */
DONG_API void dong_resource_pack_free(dong_resource_pack_t* pack);

/** Get the number of entries in a resource pack. */
DONG_API uint32_t dong_resource_pack_entry_count(const dong_resource_pack_t* pack);

/** Get the name of an entry by index. */
DONG_API const char* dong_resource_pack_entry_name(const dong_resource_pack_t* pack, uint32_t index);

/** Get the type of an entry by index. */
DONG_API dong_resource_type_t dong_resource_pack_entry_type(const dong_resource_pack_t* pack, uint32_t index);

/** Get the data of an entry by index (pointer + size). */
DONG_API const void* dong_resource_pack_entry_data(const dong_resource_pack_t* pack, uint32_t index, uint64_t* out_size);

/**
 * Attach a resource pack to an engine instance.
 * The engine will prefer resources from the pack over runtime discovery.
 * Multiple packs can be attached (searched in LIFO order).
 */
DONG_API void dong_engine_attach_resource_pack(dong_engine_t* engine, dong_resource_pack_t* pack);

#ifdef __cplusplus
}
#endif

#endif /* DONG_RESOURCE_PACK_H */
