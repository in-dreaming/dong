#pragma once

/**
 * Dong Engine - Global Shared Resources
 * 
 * 进程级共享资源管理器，用于多 View 场景（如 3D 屏幕）的资源共享。
 * 
 * 设计原则：
 * 1. 延迟初始化 - 首次使用时创建
 * 2. 引用计数 - View 创建/销毁时自动管理
 * 3. 线程安全 - 支持多线程 View 创建
 * 
 * 当前共享资源：
 * - GlyphAtlas: 字体 MSDF 纹理（最大内存节省）
 *   23 个独立 View → 1 个共享 = 节省 ~1GB GPU 内存
 */

#include <cstdint>
#include <memory>
#include <mutex>
#include <atomic>
#include <vector>

// Forward declarations
struct DongGPUDriver;

namespace dong {
namespace render {
class GlyphAtlas;
} // namespace render

/**
 * GlobalShared - 进程级共享资源管理器
 * 
 * 使用方式：
 *   // 初始化（主线程，应用启动时）
 *   GlobalShared::initialize(gpu_driver);
 *   
 *   // View 创建时获取引用
 *   auto* shared = GlobalShared::instance();
 *   shared->addRef();
 *   
 *   // 访问共享资源
 *   auto* tier = shared->getGlyphAtlasTierForFontSize(16.0f);
 *   
 *   // View 销毁时释放
 *   shared->release();
 *   
 *   // 应用关闭时清理
 *   GlobalShared::shutdown();
 */
class GlobalShared {
public:
    // GlyphAtlas Tier 结构
    struct GlyphAtlasTier {
        uint32_t bitmap_px = 0;
        float distance_range = 0.0f;
        std::unique_ptr<render::GlyphAtlas> atlas;
    };

    // 获取单例实例（未初始化时返回 nullptr）
    static GlobalShared* instance();
    
    // 初始化全局共享资源（线程安全，只能调用一次）
    static bool initialize(DongGPUDriver* driver);
    
    // 关闭全局共享资源（等待所有引用释放）
    static void shutdown();
    
    // 检查是否已初始化
    static bool isInitialized();
    
    // 引用计数管理
    void addRef();
    void release();
    int refCount() const;
    
    // 访问共享 GlyphAtlas Tier
    // bitmap_px: 目标位图分辨率 (128, 192, 256, 384)
    GlyphAtlasTier* getGlyphAtlasTier(uint32_t bitmap_px);
    
    // 根据字体大小自动选择合适的 tier
    GlyphAtlasTier* getGlyphAtlasTierForFontSize(float font_size);
    
    // 获取 GPU Driver
    DongGPUDriver* gpuDriver() { return gpu_driver_; }
    
    // 统计信息
    struct Stats {
        size_t glyph_atlas_memory_bytes = 0;
        size_t image_atlas_memory_bytes = 0;
        size_t total_memory_bytes = 0;
        int view_ref_count = 0;
        int glyph_atlas_tier_count = 0;
    };
    Stats getStats() const;
    
    // 禁止拷贝
    GlobalShared(const GlobalShared&) = delete;
    GlobalShared& operator=(const GlobalShared&) = delete;
    
public:
    ~GlobalShared();
    
private:
    GlobalShared() = default;
    
    bool initInternal(DongGPUDriver* driver);
    void shutdownInternal();
    
    static std::unique_ptr<GlobalShared> s_instance;
    static std::mutex s_mutex;
    
    std::atomic<int> ref_count_{0};
    std::atomic<bool> initialized_{false};
    
    // 共享资源
    std::vector<GlyphAtlasTier> glyph_atlas_tiers_;
    
    DongGPUDriver* gpu_driver_ = nullptr;
};

} // namespace dong
