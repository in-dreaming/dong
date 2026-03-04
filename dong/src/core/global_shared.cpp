#include "global_shared.hpp"

#include "../render/glyph_atlas.hpp"
#include "../render/resource_manager.hpp"
#include "../render/font_resolver.hpp"
#include "dong_gpu_driver.h"
#include "log.h"

#include <cassert>

namespace dong {

// Static members
std::unique_ptr<GlobalShared> GlobalShared::s_instance;
std::mutex GlobalShared::s_mutex;

GlobalShared::~GlobalShared() {
    shutdownInternal();
}

GlobalShared* GlobalShared::instance() {
    return s_instance.get();
}

bool GlobalShared::initialize(DongGPUDriver* driver) {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (s_instance) {
        DONG_LOG_WARN("GlobalShared already initialized");
        return true;
    }
    
    if (!driver) {
        DONG_LOG_ERROR("GlobalShared::initialize: driver is required");
        return false;
    }
    
    s_instance.reset(new GlobalShared());
    if (!s_instance->initInternal(driver)) {
        s_instance.reset();
        return false;
    }
    
    DONG_LOG_INFO("GlobalShared initialized successfully");
    return true;
}

void GlobalShared::shutdown() {
    std::lock_guard<std::mutex> lock(s_mutex);
    
    if (!s_instance) {
        return;
    }
    
    if (s_instance->ref_count_.load() > 0) {
        DONG_LOG_WARN("GlobalShared::shutdown called with %d active references", 
                      s_instance->ref_count_.load());
    }
    
    s_instance.reset();
    DONG_LOG_INFO("GlobalShared shutdown");
}

bool GlobalShared::isInitialized() {
    return s_instance != nullptr && s_instance->initialized_.load();
}

void GlobalShared::addRef() {
    ref_count_.fetch_add(1, std::memory_order_relaxed);
    DONG_LOG_DEBUG("GlobalShared addRef: %d", ref_count_.load());
}

void GlobalShared::release() {
    int prev = ref_count_.fetch_sub(1, std::memory_order_relaxed);
    DONG_LOG_DEBUG("GlobalShared release: %d -> %d", prev, prev - 1);
    
    if (prev <= 0) {
        DONG_LOG_ERROR("GlobalShared::release called with ref_count <= 0");
    }
}

int GlobalShared::refCount() const {
    return ref_count_.load(std::memory_order_relaxed);
}

bool GlobalShared::initInternal(DongGPUDriver* driver) {
    gpu_driver_ = driver;
    
    // 初始化共享 GlyphAtlas
    // 使用多个 tier 支持不同字号
    DONG_LOG_INFO("GlobalShared: Initializing shared GlyphAtlas tiers");
    
    struct TierConfig {
        uint32_t bitmap_px;
        float distance_range;
    };
    
    const TierConfig tiers[] = {
        {128u,  16.0f},  // 9-14px: large range for high downscaling ratio (Godot uses 14.0)
        {192u,  14.0f},  // 14-22px
        {256u,  11.0f},  // 22-36px
        {384u,  12.0f},  // 36px+
    };
    
    for (const auto& tier : tiers) {
        auto atlas = std::make_unique<render::GlyphAtlas>(driver);
        if (!atlas->initialize(2048, 2048, tier.bitmap_px, tier.distance_range)) {
            DONG_LOG_ERROR("GlobalShared: Failed to initialize glyph atlas tier %u", tier.bitmap_px);
            return false;
        }
        glyph_atlas_tiers_.push_back({tier.bitmap_px, tier.distance_range, std::move(atlas)});
        DONG_LOG_INFO("GlobalShared: Created glyph atlas tier %upx", tier.bitmap_px);
    }
    
    initialized_.store(true, std::memory_order_release);
    return true;
}

void GlobalShared::shutdownInternal() {
    if (!initialized_.load()) {
        return;
    }
    
    DONG_LOG_INFO("GlobalShared: Shutting down internal resources");
    
    glyph_atlas_tiers_.clear();
    
    gpu_driver_ = nullptr;
    initialized_.store(false, std::memory_order_release);
}

GlobalShared::GlyphAtlasTier* GlobalShared::getGlyphAtlasTier(uint32_t bitmap_px) {
    // 找到最接近的 tier
    GlyphAtlasTier* best = nullptr;
    uint32_t best_diff = UINT32_MAX;
    
    for (auto& tier : glyph_atlas_tiers_) {
        uint32_t diff = (tier.bitmap_px >= bitmap_px) 
            ? (tier.bitmap_px - bitmap_px) 
            : (bitmap_px - tier.bitmap_px) * 2; // 优先选择不高于目标的
        
        if (diff < best_diff) {
            best_diff = diff;
            best = &tier;
        }
    }
    
    return best;
}

GlobalShared::GlyphAtlasTier* GlobalShared::getGlyphAtlasTierForFontSize(float font_size) {
    // 根据字号选择 tier
    const float clamped = std::max(font_size, 6.0f);
    
    uint32_t target_px;
    if (clamped <= 14.0f) target_px = 128u;
    else if (clamped <= 22.0f) target_px = 192u;
    else if (clamped <= 36.0f) target_px = 256u;
    else target_px = 384u;
    
    return getGlyphAtlasTier(target_px);
}

GlobalShared::Stats GlobalShared::getStats() const {
    Stats stats;
    stats.view_ref_count = ref_count_.load();
    
    // 计算 GlyphAtlas 内存使用
    for (const auto& tier : glyph_atlas_tiers_) {
        if (tier.atlas) {
            uint32_t pages = tier.atlas->getPageCount();
            uint32_t w = tier.atlas->getWidth();
            uint32_t h = tier.atlas->getHeight();
            stats.glyph_atlas_memory_bytes += static_cast<size_t>(pages) * w * h * 4; // RGBA8
        }
    }
    
    stats.total_memory_bytes = stats.glyph_atlas_memory_bytes;
    
    return stats;
}

} // namespace dong
