#pragma once

/**
 * Dong Engine 日志分级系统（平台无关）
 *
 * 注意：core 侧不依赖 SDL。平台层可在插件中接管/桥接日志。
 */

#include <cstdio>
#include <chrono>

// 日志级别定义
#define DONG_LOG_LEVEL_NONE    0
#define DONG_LOG_LEVEL_ERROR   1
#define DONG_LOG_LEVEL_WARN    2
#define DONG_LOG_LEVEL_INFO    3
#define DONG_LOG_LEVEL_DEBUG   4
#define DONG_LOG_LEVEL_VERBOSE 5

// 默认日志级别：INFO（不输出 DEBUG 和 VERBOSE）
#ifndef DONG_LOG_LEVEL
#define DONG_LOG_LEVEL DONG_LOG_LEVEL_INFO
#endif

#ifndef DONG_LOG_STDERR
#define DONG_LOG_STDERR stderr
#endif

#define DONG_LOG__PRINTF(prefix, fmt, ...) \
    do { \
        std::fprintf(DONG_LOG_STDERR, "%s", prefix); \
        std::fprintf(DONG_LOG_STDERR, fmt, ##__VA_ARGS__); \
        std::fprintf(DONG_LOG_STDERR, "\n"); \
    } while (0)

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_ERROR
    #define DONG_LOG_ERROR(fmt, ...) DONG_LOG__PRINTF("[ERROR] ", fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_ERROR(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_WARN
    #define DONG_LOG_WARN(fmt, ...) DONG_LOG__PRINTF("[WARN] ", fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_WARN(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_INFO
    #define DONG_LOG_INFO(fmt, ...) DONG_LOG__PRINTF("[INFO] ", fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_INFO(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_DEBUG
    #define DONG_LOG_DEBUG(fmt, ...) DONG_LOG__PRINTF("[DEBUG] ", fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_DEBUG(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_VERBOSE
    #define DONG_LOG_VERBOSE(fmt, ...) DONG_LOG__PRINTF("[VERBOSE] ", fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_VERBOSE(fmt, ...) ((void)0)
#endif

// =============================================================================
// 性能计时工具
// =============================================================================

// 启用/禁用性能计时（设为 0 可完全禁用，无运行时开销）
#ifndef DONG_PERF_ENABLED
#define DONG_PERF_ENABLED 1
#endif

// 性能计时阈值（毫秒），超过此值才输出警告
#ifndef DONG_PERF_WARN_THRESHOLD_MS
#define DONG_PERF_WARN_THRESHOLD_MS 16.0
#endif

#if DONG_PERF_ENABLED

// RAII 计时器类
class DongPerfTimer {
public:
    DongPerfTimer(const char* name, double warn_threshold_ms = DONG_PERF_WARN_THRESHOLD_MS)
        : m_name(name), m_warn_threshold_ms(warn_threshold_ms),
          m_start(std::chrono::high_resolution_clock::now()) {}
    
    ~DongPerfTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - m_start).count();
        if (ms >= m_warn_threshold_ms) {
            std::fprintf(stderr, "[PERF_WARN] %s: %.3f ms (threshold: %.1f ms)\n", 
                m_name, ms, m_warn_threshold_ms);
        }
    }

    // 手动获取当前耗时（不结束计时）
    double elapsed_ms() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - m_start).count();
    }

private:
    const char* m_name;
    double m_warn_threshold_ms;
    std::chrono::high_resolution_clock::time_point m_start;
};

// 作用域计时宏（超阈值自动警告）
#define DONG_PERF_SCOPE(name) DongPerfTimer __dong_perf_timer_##__LINE__(name)
#define DONG_PERF_SCOPE_THRESHOLD(name, threshold_ms) DongPerfTimer __dong_perf_timer_##__LINE__(name, threshold_ms)

// 手动计时宏
#define DONG_PERF_START(var) auto var##_start = std::chrono::high_resolution_clock::now()
#define DONG_PERF_END_LOG(var, name) do { \
    auto var##_end = std::chrono::high_resolution_clock::now(); \
    double var##_ms = std::chrono::duration<double, std::milli>(var##_end - var##_start).count(); \
    std::fprintf(stderr, "[PERF] %s: %.3f ms\n", name, var##_ms); \
} while(0)

#else
// 禁用时为空操作
#define DONG_PERF_SCOPE(name) ((void)0)
#define DONG_PERF_SCOPE_THRESHOLD(name, threshold_ms) ((void)0)
#define DONG_PERF_START(var) ((void)0)
#define DONG_PERF_END_LOG(var, name) ((void)0)
#endif
