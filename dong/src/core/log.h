#pragma once

/**
 * Dong Engine 日志分级系统（平台无关）
 *
 * 注意：core 侧不依赖 SDL。平台层可在插件中接管/桥接日志。
 */

#include <cstdio>

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
