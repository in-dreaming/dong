#pragma once

/**
 * Dong Engine Profiler - 多线程性能分析器
 * 
 * 特性：
 * - 线程安全：每线程独立缓冲区 + 无锁收集
 * - Chrome Trace JSON 输出（chrome://tracing 可视化）
 * - RAII scope 打点（自动 begin/end）
 * - 支持嵌套调用栈
 * - 低开销设计（禁用时零开销）
 */

#include <cstdint>

// DLL export/import macros
#if defined(_WIN32) || defined(_WIN64)
    #ifdef DONG_BUILDING_DLL
        #define DONG_PROFILER_API __declspec(dllexport)
    #else
        #define DONG_PROFILER_API __declspec(dllimport)
    #endif
#else
    #define DONG_PROFILER_API __attribute__((visibility("default")))
#endif

// ============================================================================
// 编译开关
// ============================================================================

#ifndef DONG_PROFILER_ENABLED
#define DONG_PROFILER_ENABLED 1
#endif

// ============================================================================
// C API（跨语言兼容）
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// 初始化 profiler（可选，首次使用时自动初始化）
DONG_PROFILER_API void dong_profiler_init(void);

// 关闭 profiler 并释放资源
DONG_PROFILER_API void dong_profiler_shutdown(void);

// 开始一个 profiler 事件
DONG_PROFILER_API void dong_profiler_begin(const char* name, const char* category);

// 结束当前事件
DONG_PROFILER_API void dong_profiler_end(void);

// 记录瞬时事件（无持续时间）
DONG_PROFILER_API void dong_profiler_instant(const char* name, const char* category);

// 开始新的一帧（用于帧边界标记）
DONG_PROFILER_API void dong_profiler_frame_begin(void);

// 结束当前帧
DONG_PROFILER_API void dong_profiler_frame_end(void);

// 导出 Chrome Trace JSON 到文件
// 返回 0 成功，-1 失败
DONG_PROFILER_API int dong_profiler_dump(const char* filepath);

// 清空所有已记录的事件
DONG_PROFILER_API void dong_profiler_clear(void);

// 获取当前帧号
DONG_PROFILER_API uint64_t dong_profiler_get_frame(void);

// 设置是否启用（运行时开关）
DONG_PROFILER_API void dong_profiler_set_enabled(int enabled);

// 获取是否启用
DONG_PROFILER_API int dong_profiler_is_enabled(void);

#ifdef __cplusplus
}
#endif

// ============================================================================
// C++ API
// ============================================================================

#ifdef __cplusplus

namespace dong {

/**
 * RAII Profiler Scope
 * 构造时自动 begin，析构时自动 end
 */
class ProfilerScope {
public:
    inline ProfilerScope(const char* name, const char* category = "default") {
#if DONG_PROFILER_ENABLED
        dong_profiler_begin(name, category);
#endif
        (void)name;
        (void)category;
    }
    
    inline ~ProfilerScope() {
#if DONG_PROFILER_ENABLED
        dong_profiler_end();
#endif
    }
    
    // 禁止拷贝
    ProfilerScope(const ProfilerScope&) = delete;
    ProfilerScope& operator=(const ProfilerScope&) = delete;
};

/**
 * Frame Scope - 帧边界标记
 */
class ProfilerFrameScope {
public:
    inline ProfilerFrameScope() {
#if DONG_PROFILER_ENABLED
        dong_profiler_frame_begin();
#endif
    }
    
    inline ~ProfilerFrameScope() {
#if DONG_PROFILER_ENABLED
        dong_profiler_frame_end();
#endif
    }
    
    ProfilerFrameScope(const ProfilerFrameScope&) = delete;
    ProfilerFrameScope& operator=(const ProfilerFrameScope&) = delete;
};

} // namespace dong

// ============================================================================
// 便捷宏
// ============================================================================

#if DONG_PROFILER_ENABLED

// 函数/作用域打点（自动使用函数名）
#define DONG_PROFILE_FUNCTION() \
    ::dong::ProfilerScope __dong_profiler_scope__(__FUNCTION__, "function")

// 自定义名称打点
#define DONG_PROFILE_SCOPE(name) \
    ::dong::ProfilerScope __dong_profiler_scope_##__LINE__(name, "scope")

// 带分类的打点
#define DONG_PROFILE_SCOPE_CAT(name, category) \
    ::dong::ProfilerScope __dong_profiler_scope_##__LINE__(name, category)

// 帧边界
#define DONG_PROFILE_FRAME() \
    ::dong::ProfilerFrameScope __dong_profiler_frame_scope__

// 瞬时事件
#define DONG_PROFILE_INSTANT(name) \
    dong_profiler_instant(name, "instant")

#else

#define DONG_PROFILE_FUNCTION()          ((void)0)
#define DONG_PROFILE_SCOPE(name)         ((void)0)
#define DONG_PROFILE_SCOPE_CAT(name, category) ((void)0)
#define DONG_PROFILE_FRAME()             ((void)0)
#define DONG_PROFILE_INSTANT(name)       ((void)0)

#endif

#endif // __cplusplus
