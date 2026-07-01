#include "gpu_image_atlas.hpp"

#include "../../src/core/log.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace dong::gpu_backend {

#ifdef DONG_HAS_IN_DREAMING_GPU

struct AtlasPage {
    DongGPUTexture texture = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t cursor_x = 0;
    uint32_t cursor_y = 0;
    uint32_t row_height = 0;
};

struct GpuImageAtlas {
    DongImageAtlas base{};
    GpuResourceManager* resources = nullptr;
    AtlasPage* pages = nullptr;
    uint32_t page_count = 0;
    uint32_t block_w = 1;
    uint32_t block_h = 1;
};

static DongGPUTextureFormat mapImageFormat(DongImageFormat format) {
    switch (format) {
    case DONG_IMAGE_FORMAT_RGBA8:
        return DONG_GPU_TEXTURE_FORMAT_RGBA8_UNORM;
    default:
        return DONG_GPU_TEXTURE_FORMAT_RGBA8_UNORM;
    }
}

static AtlasPage* create_page(GpuImageAtlas* atlas) {
    if (atlas->page_count >= atlas->base.config.max_pages) {
        return nullptr;
    }
    const uint32_t idx = atlas->page_count;
    DongGPUTextureDesc desc{};
    desc.width = atlas->base.config.width;
    desc.height = atlas->base.config.height;
    desc.format = mapImageFormat(atlas->base.config.format);
    desc.usage = DONG_GPU_TEXTURE_USAGE_SAMPLER | DONG_GPU_TEXTURE_USAGE_TRANSFER_DST;
    desc.mip_levels = 1;
    desc.debug_name = "dong_image_atlas_page";

    DongGPUTexture tex = atlas->resources->createTexture(&desc);
    if (!tex) {
        return nullptr;
    }

    atlas->pages[idx].texture = tex;
    atlas->pages[idx].width = desc.width;
    atlas->pages[idx].height = desc.height;
    atlas->pages[idx].cursor_x = 0;
    atlas->pages[idx].cursor_y = 0;
    atlas->pages[idx].row_height = 0;
    atlas->page_count++;
    return &atlas->pages[idx];
}

static AtlasPage* find_page_for_alloc(GpuImageAtlas* atlas, uint32_t alloc_w, uint32_t alloc_h) {
    uint32_t padding = atlas->base.config.padding;
    padding = ((padding + atlas->block_w - 1) / atlas->block_w) * atlas->block_w;

    for (uint32_t i = 0; i < atlas->page_count; ++i) {
        AtlasPage* page = &atlas->pages[i];
        uint32_t test_x = page->cursor_x;
        uint32_t test_y = page->cursor_y;
        uint32_t test_row_h = page->row_height;
        if (test_x + alloc_w + padding > page->width) {
            test_x = 0;
            test_y += test_row_h + padding;
            test_y = ((test_y + atlas->block_h - 1) / atlas->block_h) * atlas->block_h;
            test_row_h = 0;
        }
        if (test_y + alloc_h <= page->height) {
            return page;
        }
    }
    return create_page(atlas);
}

static DongAtlasResult atlas_alloc(DongImageAtlas* base, uint32_t width, uint32_t height, DongAtlasEntry* out_entry) {
    if (!base || !out_entry || width == 0 || height == 0) {
        return DONG_ATLAS_ERR_INVALID_ARG;
    }
    auto* atlas = reinterpret_cast<GpuImageAtlas*>(base);
    uint32_t alloc_w = ((width + atlas->block_w - 1) / atlas->block_w) * atlas->block_w;
    uint32_t alloc_h = ((height + atlas->block_h - 1) / atlas->block_h) * atlas->block_h;
    if (alloc_w > atlas->base.config.width || alloc_h > atlas->base.config.height) {
        return DONG_ATLAS_ERR_OUT_OF_SPACE;
    }

    AtlasPage* page = find_page_for_alloc(atlas, alloc_w, alloc_h);
    if (!page) {
        return DONG_ATLAS_ERR_OUT_OF_SPACE;
    }

    uint32_t padding = atlas->base.config.padding;
    padding = ((padding + atlas->block_w - 1) / atlas->block_w) * atlas->block_w;
    if (page->cursor_x + alloc_w + padding > page->width) {
        page->cursor_x = 0;
        page->cursor_y += page->row_height + padding;
        page->cursor_y = ((page->cursor_y + atlas->block_h - 1) / atlas->block_h) * atlas->block_h;
        page->row_height = 0;
    }

    out_entry->x = page->cursor_x;
    out_entry->y = page->cursor_y;
    out_entry->width = width;
    out_entry->height = height;
    out_entry->u0 = static_cast<float>(page->cursor_x) / static_cast<float>(page->width);
    out_entry->v0 = static_cast<float>(page->cursor_y) / static_cast<float>(page->height);
    out_entry->u1 = static_cast<float>(page->cursor_x + width) / static_cast<float>(page->width);
    out_entry->v1 = static_cast<float>(page->cursor_y + height) / static_cast<float>(page->height);

    for (uint32_t i = 0; i < atlas->page_count; ++i) {
        if (&atlas->pages[i] == page) {
            out_entry->atlas_page = i;
            break;
        }
    }

    page->cursor_x += alloc_w + padding;
    page->row_height = std::max(page->row_height, alloc_h);
    return DONG_ATLAS_OK;
}

static DongAtlasResult atlas_upload(DongImageAtlas* base, const DongAtlasEntry* entry, const void* data, size_t data_size) {
    if (!base || !entry || !data) {
        return DONG_ATLAS_ERR_INVALID_ARG;
    }
    auto* atlas = reinterpret_cast<GpuImageAtlas*>(base);
    if (entry->atlas_page >= atlas->page_count) {
        return DONG_ATLAS_ERR_INVALID_ARG;
    }
    AtlasPage& page = atlas->pages[entry->atlas_page];
    const uint32_t stride = entry->width * 4;
    const size_t expected = static_cast<size_t>(stride) * entry->height;
    if (data_size < expected) {
        return DONG_ATLAS_ERR_INVALID_ARG;
    }
    if (atlas->resources->uploadTextureSubrect(page.texture, data, entry->x, entry->y, entry->width, entry->height,
                                               stride, nullptr) != 0) {
        return DONG_ATLAS_ERR_INTERNAL;
    }
    return DONG_ATLAS_OK;
}

static void* atlas_get_texture(DongImageAtlas* base, uint32_t page_index) {
    auto* atlas = reinterpret_cast<GpuImageAtlas*>(base);
    if (!atlas || page_index >= atlas->page_count) {
        return nullptr;
    }
    return atlas->pages[page_index].texture;
}

static void atlas_clear(DongImageAtlas* base) {
    auto* atlas = reinterpret_cast<GpuImageAtlas*>(base);
    if (!atlas) {
        return;
    }
    for (uint32_t i = 0; i < atlas->page_count; ++i) {
        atlas->pages[i].cursor_x = 0;
        atlas->pages[i].cursor_y = 0;
        atlas->pages[i].row_height = 0;
    }
}

static void atlas_destroy(DongImageAtlas* base) {
    if (!base) {
        return;
    }
    auto* atlas = reinterpret_cast<GpuImageAtlas*>(base);
    if (atlas->resources) {
        for (uint32_t i = 0; i < atlas->page_count; ++i) {
            if (atlas->pages[i].texture) {
                atlas->resources->destroyTexture(atlas->pages[i].texture);
            }
        }
    }
    std::free(atlas->pages);
    delete atlas;
}

static const DongImageAtlasVTable s_atlas_vtable = {
    atlas_alloc,
    atlas_upload,
    atlas_get_texture,
    atlas_clear,
    atlas_destroy,
};

#endif

DongImageAtlas* gpu_image_atlas_create(GpuResourceManager* resources, const DongAtlasConfig* config) {
#ifdef DONG_HAS_IN_DREAMING_GPU
    if (!resources || !config) {
        return nullptr;
    }
    auto* atlas = new GpuImageAtlas();
    atlas->resources = resources;
    atlas->base.config = *config;
    if (atlas->base.config.width == 0) {
        atlas->base.config.width = 2048;
    }
    if (atlas->base.config.height == 0) {
        atlas->base.config.height = 2048;
    }
    if (atlas->base.config.max_pages == 0) {
        atlas->base.config.max_pages = 4;
    }
    if (atlas->base.config.format == DONG_IMAGE_FORMAT_UNKNOWN) {
        atlas->base.config.format = DONG_IMAGE_FORMAT_RGBA8;
    }
    if (atlas->base.config.padding == 0) {
        atlas->base.config.padding = 2;
    }
    atlas->pages = static_cast<AtlasPage*>(std::calloc(atlas->base.config.max_pages, sizeof(AtlasPage)));
    if (!atlas->pages) {
        delete atlas;
        return nullptr;
    }

    atlas->base.vtable = &s_atlas_vtable;
    return &atlas->base;
#else
    (void)resources;
    (void)config;
    return nullptr;
#endif
}

void gpu_image_atlas_destroy(DongImageAtlas* atlas) {
    if (atlas && atlas->vtable && atlas->vtable->destroy) {
        atlas->vtable->destroy(atlas);
    }
}

} // namespace dong::gpu_backend
