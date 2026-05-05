#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dong::resource {

// .dpkg file format constants
static constexpr uint32_t DPKG_MAGIC = 0x474B5044;  // "DPKG" in little-endian
static constexpr uint32_t DPKG_VERSION = 1;

struct DpkgHeader {
    uint32_t magic = DPKG_MAGIC;
    uint32_t version = DPKG_VERSION;
    uint32_t entry_count = 0;
    uint32_t flags = 0;
    uint8_t reserved[16] = {};
};
static_assert(sizeof(DpkgHeader) == 32, "DpkgHeader must be 32 bytes");

struct DpkgEntryDesc {
    uint32_t type = 0;          // DONG_RESOURCE_FONT/ATLAS/SHADER/IMAGE
    uint32_t name_offset = 0;   // offset into string table
    uint64_t data_offset = 0;   // offset into data section
    uint64_t data_size = 0;     // size in pack (may be compressed)
    uint64_t original_size = 0; // uncompressed size (0 = not compressed)
};
static_assert(sizeof(DpkgEntryDesc) == 32, "DpkgEntryDesc must be 32 bytes");

// In-memory representation of a loaded resource pack
struct ResourceEntry {
    std::string name;
    uint32_t type = 0;
    const uint8_t* data = nullptr;  // pointer into pack data (zero-copy)
    uint64_t size = 0;
};

class ResourcePack {
public:
    // Load from file
    bool loadFromFile(const std::string& path);

    // Load from memory (zero-copy mode — data must remain valid)
    bool loadFromMemory(const void* data, uint64_t size);

    uint32_t entryCount() const { return static_cast<uint32_t>(entries_.size()); }
    const ResourceEntry* getEntry(uint32_t index) const;
    const ResourceEntry* findByName(const std::string& name) const;
    const ResourceEntry* findByTypeAndName(uint32_t type, const std::string& name) const;

private:
    std::vector<ResourceEntry> entries_;
    std::vector<uint8_t> owned_data_;  // owned copy when loaded from file
    const uint8_t* base_ptr_ = nullptr;
    uint64_t total_size_ = 0;
};

} // namespace dong::resource
