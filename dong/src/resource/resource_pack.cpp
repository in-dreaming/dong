#include "resource_pack.hpp"
#include "../core/log.h"
#include <cstring>
#include <fstream>

namespace dong::resource {

bool ResourcePack::loadFromFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) {
        DONG_LOG_ERROR("[ResourcePack] Failed to open: %s", path.c_str());
        return false;
    }

    auto file_size = static_cast<uint64_t>(f.tellg());
    f.seekg(0);

    owned_data_.resize(file_size);
    f.read(reinterpret_cast<char*>(owned_data_.data()), file_size);
    f.close();

    return loadFromMemory(owned_data_.data(), file_size);
}

bool ResourcePack::loadFromMemory(const void* data, uint64_t size) {
    if (!data || size < sizeof(DpkgHeader)) {
        DONG_LOG_ERROR("[ResourcePack] Invalid data (too small)");
        return false;
    }

    base_ptr_ = static_cast<const uint8_t*>(data);
    total_size_ = size;

    // Parse header
    DpkgHeader header;
    std::memcpy(&header, base_ptr_, sizeof(header));

    if (header.magic != DPKG_MAGIC) {
        DONG_LOG_ERROR("[ResourcePack] Invalid magic (expected DPKG)");
        return false;
    }
    if (header.version != DPKG_VERSION) {
        DONG_LOG_ERROR("[ResourcePack] Version mismatch (got %u, expected %u)",
                      header.version, DPKG_VERSION);
        return false;
    }

    uint32_t entry_count = header.entry_count;

    // Parse entry table
    uint64_t entry_table_offset = sizeof(DpkgHeader);
    uint64_t entry_table_size = entry_count * sizeof(DpkgEntryDesc);

    if (entry_table_offset + entry_table_size > size) {
        DONG_LOG_ERROR("[ResourcePack] Entry table exceeds file size");
        return false;
    }

    // String table starts after entry table
    uint64_t string_table_offset = entry_table_offset + entry_table_size;

    entries_.clear();
    entries_.reserve(entry_count);

    for (uint32_t i = 0; i < entry_count; ++i) {
        DpkgEntryDesc desc;
        std::memcpy(&desc, base_ptr_ + entry_table_offset + i * sizeof(DpkgEntryDesc), sizeof(desc));

        ResourceEntry entry;
        entry.type = desc.type;

        // Read name from string table
        uint64_t name_pos = string_table_offset + desc.name_offset;
        if (name_pos < size) {
            entry.name = reinterpret_cast<const char*>(base_ptr_ + name_pos);
        }

        // Point to data
        if (desc.data_offset + desc.data_size <= size) {
            entry.data = base_ptr_ + desc.data_offset;
            entry.size = desc.data_size;
        }

        entries_.push_back(std::move(entry));
    }

    DONG_LOG_INFO("[ResourcePack] Loaded %u entries from %llu bytes", entry_count, size);
    return true;
}

const ResourceEntry* ResourcePack::getEntry(uint32_t index) const {
    if (index >= entries_.size()) return nullptr;
    return &entries_[index];
}

const ResourceEntry* ResourcePack::findByName(const std::string& name) const {
    for (const auto& e : entries_) {
        if (e.name == name) return &e;
    }
    return nullptr;
}

const ResourceEntry* ResourcePack::findByTypeAndName(uint32_t type, const std::string& name) const {
    for (const auto& e : entries_) {
        if (e.type == type && e.name == name) return &e;
    }
    return nullptr;
}

} // namespace dong::resource
