/**
 * @file dong_sdl_image_atlas.c
 * @brief SDL GPU backend implementation for ImageAtlas
 *
 * Supports block-aligned allocation for compressed formats (ASTC, BC).
 */

#include "dong_sdl_image_atlas.h"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// Internal Types
// =============================================================================

typedef struct AtlasPage {
    SDL_GPUTexture* texture;
    uint32_t width;
    uint32_t height;
    uint32_t cursor_x;      // Next free X position
    uint32_t cursor_y;      // Current row Y position
    uint32_t row_height;    // Height of current row
} AtlasPage;

typedef struct SDLImageAtlas {
    DongImageAtlas base;
    SDL_GPUDevice* device;
    AtlasPage* pages;
    uint32_t page_count;
    uint32_t block_w;
    uint32_t block_h;
    SDL_GPUTextureFormat sdl_format;
} SDLImageAtlas;

// =============================================================================
// Format Mapping
// =============================================================================

int dong_sdl_image_format_to_gpu_format(DongImageFormat format) {
    switch (format) {
        // Uncompressed
        case DONG_IMAGE_FORMAT_R8:
            return SDL_GPU_TEXTUREFORMAT_R8_UNORM;
        case DONG_IMAGE_FORMAT_RG8:
            return SDL_GPU_TEXTUREFORMAT_R8G8_UNORM;
        case DONG_IMAGE_FORMAT_RGBA8:
            return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        case DONG_IMAGE_FORMAT_RGBA16F:
            return SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
        case DONG_IMAGE_FORMAT_RGBA32F:
            return SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;

        // ASTC
        case DONG_IMAGE_FORMAT_ASTC_4x4:
            return SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM;
        case DONG_IMAGE_FORMAT_ASTC_5x5:
            return SDL_GPU_TEXTUREFORMAT_ASTC_5x5_UNORM;
        case DONG_IMAGE_FORMAT_ASTC_6x6:
            return SDL_GPU_TEXTUREFORMAT_ASTC_6x6_UNORM;
        case DONG_IMAGE_FORMAT_ASTC_8x8:
            return SDL_GPU_TEXTUREFORMAT_ASTC_8x8_UNORM;

        // BC formats
        case DONG_IMAGE_FORMAT_BC1:
            return SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM;
        case DONG_IMAGE_FORMAT_BC2:
            return SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM;
        case DONG_IMAGE_FORMAT_BC3:
            return SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM;
        case DONG_IMAGE_FORMAT_BC4:
            return SDL_GPU_TEXTUREFORMAT_BC4_R_UNORM;
        case DONG_IMAGE_FORMAT_BC5:
            return SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM;
        case DONG_IMAGE_FORMAT_BC6H:
            return SDL_GPU_TEXTUREFORMAT_BC6H_RGB_UFLOAT;
        case DONG_IMAGE_FORMAT_BC7:
            return SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM;

        default:
            return 0; // SDL_GPU_TEXTUREFORMAT_INVALID
    }
}

// =============================================================================
// Internal Functions
// =============================================================================

static AtlasPage* create_page(SDLImageAtlas* atlas) {
    if (atlas->page_count >= atlas->base.config.max_pages) {
        return NULL;
    }

    uint32_t idx = atlas->page_count;

    SDL_GPUTextureCreateInfo tex_info = {0};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = atlas->sdl_format;
    tex_info.width = atlas->base.config.width;
    tex_info.height = atlas->base.config.height;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;

    SDL_GPUTexture* texture = SDL_CreateGPUTexture(atlas->device, &tex_info);
    if (!texture) {
        SDL_Log("[SDLImageAtlas] Failed to create page texture: %s", SDL_GetError());
        return NULL;
    }

    atlas->pages[idx].texture = texture;
    atlas->pages[idx].width = atlas->base.config.width;
    atlas->pages[idx].height = atlas->base.config.height;
    atlas->pages[idx].cursor_x = 0;
    atlas->pages[idx].cursor_y = 0;
    atlas->pages[idx].row_height = 0;

    atlas->page_count++;

    SDL_Log("[SDLImageAtlas] Created page %u (%ux%u, format=%d)",
            idx, atlas->base.config.width, atlas->base.config.height, atlas->sdl_format);

    return &atlas->pages[idx];
}

static AtlasPage* find_page_for_alloc(SDLImageAtlas* atlas, uint32_t alloc_w, uint32_t alloc_h) {
    uint32_t padding = atlas->base.config.padding;

    // Align padding to block size
    padding = ((padding + atlas->block_w - 1) / atlas->block_w) * atlas->block_w;

    // Try existing pages
    for (uint32_t i = 0; i < atlas->page_count; i++) {
        AtlasPage* page = &atlas->pages[i];

        // Try current row
        uint32_t test_x = page->cursor_x;
        uint32_t test_y = page->cursor_y;
        uint32_t test_row_h = page->row_height;

        if (test_x + alloc_w + padding > page->width) {
            // Move to next row
            test_x = 0;
            test_y += test_row_h + padding;
            // Align Y to block boundary
            test_y = ((test_y + atlas->block_h - 1) / atlas->block_h) * atlas->block_h;
            test_row_h = 0;
        }

        if (test_y + alloc_h <= page->height) {
            return page;
        }
    }

    // Need new page
    return create_page(atlas);
}

// =============================================================================
// VTable Implementation
// =============================================================================

static DongAtlasResult sdl_atlas_alloc(DongImageAtlas* base_atlas,
                                        uint32_t width,
                                        uint32_t height,
                                        DongAtlasEntry* out_entry) {
    if (!base_atlas || !out_entry || width == 0 || height == 0) {
        return DONG_ATLAS_ERR_INVALID_ARG;
    }

    SDLImageAtlas* atlas = (SDLImageAtlas*)base_atlas;

    // Align allocation to block size
    uint32_t alloc_w = ((width + atlas->block_w - 1) / atlas->block_w) * atlas->block_w;
    uint32_t alloc_h = ((height + atlas->block_h - 1) / atlas->block_h) * atlas->block_h;

    // Check if allocation fits in atlas
    if (alloc_w > atlas->base.config.width || alloc_h > atlas->base.config.height) {
        SDL_Log("[SDLImageAtlas] Allocation too large: %ux%u (max %ux%u)",
                alloc_w, alloc_h, atlas->base.config.width, atlas->base.config.height);
        return DONG_ATLAS_ERR_OUT_OF_SPACE;
    }

    AtlasPage* page = find_page_for_alloc(atlas, alloc_w, alloc_h);
    if (!page) {
        SDL_Log("[SDLImageAtlas] No space available for %ux%u allocation", alloc_w, alloc_h);
        return DONG_ATLAS_ERR_OUT_OF_SPACE;
    }

    uint32_t padding = atlas->base.config.padding;
    padding = ((padding + atlas->block_w - 1) / atlas->block_w) * atlas->block_w;

    // Check if we need to move to next row
    if (page->cursor_x + alloc_w + padding > page->width) {
        page->cursor_x = 0;
        page->cursor_y += page->row_height + padding;
        // Align Y to block boundary
        page->cursor_y = ((page->cursor_y + atlas->block_h - 1) / atlas->block_h) * atlas->block_h;
        page->row_height = 0;
    }

    // Allocate
    uint32_t x = page->cursor_x;
    uint32_t y = page->cursor_y;

    page->cursor_x += alloc_w + padding;
    if (alloc_h > page->row_height) {
        page->row_height = alloc_h;
    }

    // Fill output
    out_entry->atlas_page = (uint32_t)(page - atlas->pages);
    out_entry->x = x;
    out_entry->y = y;
    out_entry->width = width;   // Original requested size
    out_entry->height = height;
    out_entry->u0 = (float)x / (float)page->width;
    out_entry->v0 = (float)y / (float)page->height;
    out_entry->u1 = (float)(x + width) / (float)page->width;
    out_entry->v1 = (float)(y + height) / (float)page->height;

    return DONG_ATLAS_OK;
}

static DongAtlasResult sdl_atlas_upload(DongImageAtlas* base_atlas,
                                         const DongAtlasEntry* entry,
                                         const void* data,
                                         size_t data_size) {
    if (!base_atlas || !entry || !data || data_size == 0) {
        return DONG_ATLAS_ERR_INVALID_ARG;
    }

    SDLImageAtlas* atlas = (SDLImageAtlas*)base_atlas;

    if (entry->atlas_page >= atlas->page_count) {
        return DONG_ATLAS_ERR_INVALID_ARG;
    }

    AtlasPage* page = &atlas->pages[entry->atlas_page];

    // Calculate aligned dimensions for compressed formats
    uint32_t alloc_w = ((entry->width + atlas->block_w - 1) / atlas->block_w) * atlas->block_w;
    uint32_t alloc_h = ((entry->height + atlas->block_h - 1) / atlas->block_h) * atlas->block_h;

    // Verify data size
    size_t expected_size = dong_image_format_calc_size(atlas->base.config.format, alloc_w, alloc_h);
    if (data_size < expected_size) {
        SDL_Log("[SDLImageAtlas] Data size mismatch: got %zu, expected %zu",
                data_size, expected_size);
        return DONG_ATLAS_ERR_INVALID_ARG;
    }

    // Create transfer buffer
    SDL_GPUTransferBufferCreateInfo transfer_info = {0};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = (Uint32)data_size;

    SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(atlas->device, &transfer_info);
    if (!transfer_buf) {
        SDL_Log("[SDLImageAtlas] Failed to create transfer buffer: %s", SDL_GetError());
        return DONG_ATLAS_ERR_INTERNAL;
    }

    // Map and copy data
    void* mapped = SDL_MapGPUTransferBuffer(atlas->device, transfer_buf, false);
    if (!mapped) {
        SDL_Log("[SDLImageAtlas] Failed to map transfer buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(atlas->device, transfer_buf);
        return DONG_ATLAS_ERR_INTERNAL;
    }

    memcpy(mapped, data, data_size);
    SDL_UnmapGPUTransferBuffer(atlas->device, transfer_buf);

    // Begin copy pass
    SDL_GPUCommandBuffer* cmd_buf = SDL_AcquireGPUCommandBuffer(atlas->device);
    if (!cmd_buf) {
        SDL_Log("[SDLImageAtlas] Failed to acquire command buffer: %s", SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(atlas->device, transfer_buf);
        return DONG_ATLAS_ERR_INTERNAL;
    }

    SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buf);
    if (!copy_pass) {
        SDL_Log("[SDLImageAtlas] Failed to begin copy pass: %s", SDL_GetError());
        SDL_SubmitGPUCommandBuffer(cmd_buf);
        SDL_ReleaseGPUTransferBuffer(atlas->device, transfer_buf);
        return DONG_ATLAS_ERR_INTERNAL;
    }

    SDL_GPUTextureTransferInfo tex_transfer = {0};
    tex_transfer.transfer_buffer = transfer_buf;
    tex_transfer.offset = 0;

    // For compressed formats, pixels_per_row is in blocks
    if (dong_image_format_is_compressed(atlas->base.config.format)) {
        tex_transfer.pixels_per_row = alloc_w / atlas->block_w;
        tex_transfer.rows_per_layer = alloc_h / atlas->block_h;
    } else {
        tex_transfer.pixels_per_row = alloc_w;
        tex_transfer.rows_per_layer = alloc_h;
    }

    SDL_GPUTextureRegion region = {0};
    region.texture = page->texture;
    region.mip_level = 0;
    region.layer = 0;
    region.x = entry->x;
    region.y = entry->y;
    region.z = 0;
    region.w = alloc_w;
    region.h = alloc_h;
    region.d = 1;

    SDL_UploadToGPUTexture(copy_pass, &tex_transfer, &region, false);
    SDL_EndGPUCopyPass(copy_pass);

    // Submit and wait
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buf);
    if (fence) {
        SDL_GPUFence* fences[] = { fence };
        SDL_WaitForGPUFences(atlas->device, true, fences, 1);
        SDL_ReleaseGPUFence(atlas->device, fence);
    } else {
        // Fallback: wait for idle
        SDL_WaitForGPUIdle(atlas->device);
    }

    SDL_ReleaseGPUTransferBuffer(atlas->device, transfer_buf);

    return DONG_ATLAS_OK;
}

static void* sdl_atlas_get_texture(DongImageAtlas* base_atlas, uint32_t page_index) {
    if (!base_atlas) {
        return NULL;
    }

    SDLImageAtlas* atlas = (SDLImageAtlas*)base_atlas;

    if (page_index >= atlas->page_count) {
        return NULL;
    }

    return atlas->pages[page_index].texture;
}

static void sdl_atlas_clear(DongImageAtlas* base_atlas) {
    if (!base_atlas) {
        return;
    }

    SDLImageAtlas* atlas = (SDLImageAtlas*)base_atlas;

    // Reset cursors but keep textures
    for (uint32_t i = 0; i < atlas->page_count; i++) {
        atlas->pages[i].cursor_x = 0;
        atlas->pages[i].cursor_y = 0;
        atlas->pages[i].row_height = 0;
    }
}

static void sdl_atlas_destroy(DongImageAtlas* base_atlas) {
    if (!base_atlas) {
        return;
    }

    SDLImageAtlas* atlas = (SDLImageAtlas*)base_atlas;

    // Release textures
    for (uint32_t i = 0; i < atlas->page_count; i++) {
        if (atlas->pages[i].texture) {
            SDL_ReleaseGPUTexture(atlas->device, atlas->pages[i].texture);
            atlas->pages[i].texture = NULL;
        }
    }

    free(atlas->pages);
    free(atlas);
}

static const DongImageAtlasVTable g_sdl_atlas_vtable = {
    .alloc = sdl_atlas_alloc,
    .upload = sdl_atlas_upload,
    .get_texture = sdl_atlas_get_texture,
    .clear = sdl_atlas_clear,
    .destroy = sdl_atlas_destroy,
};

// =============================================================================
// Public API
// =============================================================================

DongImageAtlas* dong_sdl_image_atlas_create(void* device, const DongAtlasConfig* config) {
    if (!device) {
        return NULL;
    }

    DongAtlasConfig cfg = config ? *config : dong_atlas_config_default();

    // Validate format
    int sdl_format = dong_sdl_image_format_to_gpu_format(cfg.format);
    if (sdl_format == 0) {
        SDL_Log("[SDLImageAtlas] Unsupported format: %d", cfg.format);
        return NULL;
    }

    // Get block size
    uint32_t block_w, block_h;
    dong_image_format_block_size(cfg.format, &block_w, &block_h);

    // Validate atlas dimensions are multiples of block size
    if (cfg.width % block_w != 0 || cfg.height % block_h != 0) {
        SDL_Log("[SDLImageAtlas] Atlas dimensions must be multiples of block size (%ux%u)",
                block_w, block_h);
        return NULL;
    }

    SDLImageAtlas* atlas = (SDLImageAtlas*)calloc(1, sizeof(SDLImageAtlas));
    if (!atlas) {
        return NULL;
    }

    atlas->base.vtable = &g_sdl_atlas_vtable;
    atlas->base.config = cfg;
    atlas->device = (SDL_GPUDevice*)device;
    atlas->block_w = block_w;
    atlas->block_h = block_h;
    atlas->sdl_format = (SDL_GPUTextureFormat)sdl_format;

    // Allocate page array
    atlas->pages = (AtlasPage*)calloc(cfg.max_pages, sizeof(AtlasPage));
    if (!atlas->pages) {
        free(atlas);
        return NULL;
    }

    atlas->page_count = 0;

    // Create initial page
    if (!create_page(atlas)) {
        free(atlas->pages);
        free(atlas);
        return NULL;
    }

    SDL_Log("[SDLImageAtlas] Created atlas: %ux%u, format=%s, block=%ux%u",
            cfg.width, cfg.height,
            dong_image_format_name(cfg.format),
            block_w, block_h);

    return (DongImageAtlas*)atlas;
}

void dong_sdl_image_atlas_destroy(DongImageAtlas* atlas) {
    if (atlas) {
        dong_atlas_destroy(atlas);
    }
}
