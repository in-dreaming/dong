#include "../../include/dong_resource_pack.h"
#include "resource_pack.hpp"

struct dong_resource_pack {
    dong::resource::ResourcePack pack;
};

dong_resource_pack_t* dong_resource_pack_load(const char* path) {
    if (!path) return nullptr;
    auto* rp = new dong_resource_pack();
    if (!rp->pack.loadFromFile(path)) {
        delete rp;
        return nullptr;
    }
    return rp;
}

dong_resource_pack_t* dong_resource_pack_load_memory(const void* data, uint64_t size) {
    if (!data || size == 0) return nullptr;
    auto* rp = new dong_resource_pack();
    if (!rp->pack.loadFromMemory(data, size)) {
        delete rp;
        return nullptr;
    }
    return rp;
}

void dong_resource_pack_free(dong_resource_pack_t* pack) {
    delete pack;
}

uint32_t dong_resource_pack_entry_count(const dong_resource_pack_t* pack) {
    return pack ? pack->pack.entryCount() : 0;
}

const char* dong_resource_pack_entry_name(const dong_resource_pack_t* pack, uint32_t index) {
    if (!pack) return nullptr;
    auto* e = pack->pack.getEntry(index);
    return e ? e->name.c_str() : nullptr;
}

dong_resource_type_t dong_resource_pack_entry_type(const dong_resource_pack_t* pack, uint32_t index) {
    if (!pack) return DONG_RESOURCE_FONT;
    auto* e = pack->pack.getEntry(index);
    return e ? static_cast<dong_resource_type_t>(e->type) : DONG_RESOURCE_FONT;
}

const void* dong_resource_pack_entry_data(const dong_resource_pack_t* pack, uint32_t index, uint64_t* out_size) {
    if (!pack) return nullptr;
    auto* e = pack->pack.getEntry(index);
    if (!e) return nullptr;
    if (out_size) *out_size = e->size;
    return e->data;
}

void dong_engine_attach_resource_pack(dong_engine_t* engine, dong_resource_pack_t* pack) {
    // TODO: Wire into engine's font resolver and shader loader
    (void)engine;
    (void)pack;
}
