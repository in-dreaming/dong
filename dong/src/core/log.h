#pragma once

/**
 * Dong Engine 日志分级系统（平台无关）
 *
 * 注意：core 侧不依赖 SDL。
 *
 * 日志输出策略：
 * - 优先使用 Platform 注入的 `DongLogger`。
 * - 若未注入，则使用 Platform 提供的默认 logger。
 * - 再无 logger 时，fallback 到 stderr/stdout。
 */

#include "dong_platform.h"
#include "dong_logger.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <chrono>
#include <atomic>
#include <mutex>
#include <cstring>

// =============================================================================
// Runtime log filtering (defaults to WARN)
// =============================================================================
// 说明：历史上 DONG 日志只有编译期筛选，DEBUG 打开时会导致性能显著下降。
// 这里增加一个运行时门控：默认只输出 INFO/WARN/ERROR。
//
// 通过环境变量控制：
//   DONG_LOG_LEVEL=error|warn|info|debug|trace|none
//
// 注意：这里的“level”采用 DongLoggerLevel 的顺序：TRACE(0) < DEBUG(1) < INFO(2) < WARN(3) < ERROR(4)

inline std::once_flag g_dong_log_level_once;
inline std::atomic<int> g_dong_log_level_runtime{static_cast<int>(DONG_LOGGER_LEVEL_WARN)};

static inline DongLoggerLevel dongParseRuntimeLogLevel(const char* v) {
    if (!v || !v[0]) {
        return DONG_LOGGER_LEVEL_WARN;
    }

    if (std::strcmp(v, "0") == 0 || std::strcmp(v, "none") == 0 || std::strcmp(v, "off") == 0) {
        return static_cast<DongLoggerLevel>(static_cast<int>(DONG_LOGGER_LEVEL_ERROR) + 1); // sentinel: disable all
    }
    if (std::strcmp(v, "error") == 0) return DONG_LOGGER_LEVEL_ERROR;
    if (std::strcmp(v, "warn") == 0 || std::strcmp(v, "warning") == 0) return DONG_LOGGER_LEVEL_WARN;
    if (std::strcmp(v, "info") == 0) return DONG_LOGGER_LEVEL_INFO;
    if (std::strcmp(v, "debug") == 0) return DONG_LOGGER_LEVEL_DEBUG;
    if (std::strcmp(v, "trace") == 0 || std::strcmp(v, "verbose") == 0) return DONG_LOGGER_LEVEL_TRACE;

    // fallback: unknown value -> WARN
    return DONG_LOGGER_LEVEL_WARN;
}

static inline void dongInitRuntimeLogLevelOnce() {
    std::call_once(g_dong_log_level_once, []() {
        const char* v = std::getenv("DONG_LOG_LEVEL");
        const DongLoggerLevel parsed = dongParseRuntimeLogLevel(v);
        g_dong_log_level_runtime.store(static_cast<int>(parsed), std::memory_order_relaxed);
    });
}

static inline bool dongLogShouldLog(DongLoggerLevel level) {
    dongInitRuntimeLogLevelOnce();
    const int thr = g_dong_log_level_runtime.load(std::memory_order_relaxed);
    if (thr > static_cast<int>(DONG_LOGGER_LEVEL_ERROR)) {
        return false; // none/off
    }
    return static_cast<int>(level) >= thr;
}

// =============================================================================
// Compile-time log filtering
// =============================================================================

#define DONG_LOG_LEVEL_NONE    0
#define DONG_LOG_LEVEL_ERROR   1
#define DONG_LOG_LEVEL_WARN    2
#define DONG_LOG_LEVEL_INFO    3
#define DONG_LOG_LEVEL_DEBUG   4
#define DONG_LOG_LEVEL_VERBOSE 5

#ifndef DONG_LOG_LEVEL
#define DONG_LOG_LEVEL DONG_LOG_LEVEL_DEBUG
#endif

#ifndef DONG_LOG_STDERR
#define DONG_LOG_STDERR stderr
#endif

static inline FILE* dongLogFallbackStream() {
    const char* v = std::getenv("DONG_LOG_TO_STDOUT");
    if (v && (v[0] == '1' || v[0] == 'y' || v[0] == 'Y' || v[0] == 't' || v[0] == 'T')) {
        return stdout;
    }
    return DONG_LOG_STDERR;
}

static inline const char* dongLogLevelPrefix(DongLoggerLevel level) {
    switch (level) {
        case DONG_LOGGER_LEVEL_TRACE: return "[TRACE] ";
        case DONG_LOGGER_LEVEL_DEBUG: return "[DEBUG] ";
        case DONG_LOGGER_LEVEL_INFO:  return "[INFO] ";
        case DONG_LOGGER_LEVEL_WARN:  return "[WARN] ";
        case DONG_LOGGER_LEVEL_ERROR: return "[ERROR] ";
        default: return "[LOG] ";
    }
}

static inline DongLogger* dongTryGetLogger() {
    DongPlatform* platform = dong_platform_get();
    return platform ? dong_platform_get_logger(platform) : nullptr;
}

static inline void dongLogV(DongLoggerLevel level, const char* fmt, va_list args) {
    char msg[2048];
    msg[0] = '\0';

    (void)std::vsnprintf(msg, sizeof(msg), fmt, args);

    DongLogger* logger = dongTryGetLogger();
    if (logger && logger->vtable && logger->vtable->log) {
        logger->vtable->log(logger, level, msg);
        return;
    }

    FILE* out = dongLogFallbackStream();
    std::fprintf(out, "%s%s\n", dongLogLevelPrefix(level), msg);
    std::fflush(out);
}

static inline void dongLog(DongLoggerLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    dongLogV(level, fmt, args);
    va_end(args);
}

// =============================================================================
// Public macros
// =============================================================================

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_ERROR
    #define DONG_LOG_ERROR(fmt, ...) do { \
        if (dongLogShouldLog(DONG_LOGGER_LEVEL_ERROR)) { \
            dongLog(DONG_LOGGER_LEVEL_ERROR, fmt, ##__VA_ARGS__); \
        } \
    } while(0)
#else
    #define DONG_LOG_ERROR(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_WARN
    #define DONG_LOG_WARN(fmt, ...) do { \
        if (dongLogShouldLog(DONG_LOGGER_LEVEL_WARN)) { \
            dongLog(DONG_LOGGER_LEVEL_WARN, fmt, ##__VA_ARGS__); \
        } \
    } while(0)
#else
    #define DONG_LOG_WARN(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_INFO
    #define DONG_LOG_INFO(fmt, ...) do { \
        if (dongLogShouldLog(DONG_LOGGER_LEVEL_INFO)) { \
            dongLog(DONG_LOGGER_LEVEL_INFO, fmt, ##__VA_ARGS__); \
        } \
    } while(0)
#else
    #define DONG_LOG_INFO(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_DEBUG
    #define DONG_LOG_DEBUG(fmt, ...) do { \
        if (dongLogShouldLog(DONG_LOGGER_LEVEL_DEBUG)) { \
            dongLog(DONG_LOGGER_LEVEL_DEBUG, fmt, ##__VA_ARGS__); \
        } \
    } while(0)
#else
    #define DONG_LOG_DEBUG(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_VERBOSE
    // VERBOSE 映射到 TRACE
    #define DONG_LOG_VERBOSE(fmt, ...) do { \
        if (dongLogShouldLog(DONG_LOGGER_LEVEL_TRACE)) { \
            dongLog(DONG_LOGGER_LEVEL_TRACE, fmt, ##__VA_ARGS__); \
        } \
    } while(0)
#else
    #define DONG_LOG_VERBOSE(fmt, ...) ((void)0)
#endif

// =============================================================================
// 性能计时工具
// =============================================================================

#ifndef DONG_PERF_ENABLED
#define DONG_PERF_ENABLED 0
#endif

#ifndef DONG_PERF_WARN_THRESHOLD_MS
#define DONG_PERF_WARN_THRESHOLD_MS 16.0
#endif

#if DONG_PERF_ENABLED

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

    double elapsed_ms() const {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - m_start).count();
    }

private:
    const char* m_name;
    double m_warn_threshold_ms;
    std::chrono::high_resolution_clock::time_point m_start;
};

#define DONG_PERF_SCOPE(name) DongPerfTimer __dong_perf_timer_##__LINE__(name)
#define DONG_PERF_SCOPE_THRESHOLD(name, threshold_ms) DongPerfTimer __dong_perf_timer_##__LINE__(name, threshold_ms)

#define DONG_PERF_START(var) auto var##_start = std::chrono::high_resolution_clock::now()
#define DONG_PERF_END_LOG(var, name) do { \
    auto var##_end = std::chrono::high_resolution_clock::now(); \
    double var##_ms = std::chrono::duration<double, std::milli>(var##_end - var##_start).count(); \
    std::fprintf(stderr, "[PERF] %s: %.3f ms\n", name, var##_ms); \
} while(0)

#else

#define DONG_PERF_SCOPE(name) ((void)0)
#define DONG_PERF_SCOPE_THRESHOLD(name, threshold_ms) ((void)0)
#define DONG_PERF_START(var) ((void)0)
#define DONG_PERF_END_LOG(var, name) ((void)0)

#endif
