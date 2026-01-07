#pragma once

/**
 * Dong Engine 日志分级系统
 * 
 * 日志级别：
 *   - DONG_LOG_LEVEL_ERROR   (1): 错误信息，始终输出
 *   - DONG_LOG_LEVEL_WARN    (2): 警告信息
 *   - DONG_LOG_LEVEL_INFO    (3): 一般信息
 *   - DONG_LOG_LEVEL_DEBUG   (4): 调试信息
 *   - DONG_LOG_LEVEL_VERBOSE (5): 详细信息（如每个 glyph 的渲染参数）
 * 
 * 使用方法：
 *   在编译时定义 DONG_LOG_LEVEL 来控制日志级别
 *   例如：-DDONG_LOG_LEVEL=3 只输出 ERROR/WARN/INFO
 *   
 *   默认级别为 INFO (3)，不输出 DEBUG 和 VERBOSE
 * 
 * 宏定义：
 *   DONG_LOG_ERROR(fmt, ...)   - 错误日志
 *   DONG_LOG_WARN(fmt, ...)    - 警告日志
 *   DONG_LOG_INFO(fmt, ...)    - 信息日志
 *   DONG_LOG_DEBUG(fmt, ...)   - 调试日志
 *   DONG_LOG_VERBOSE(fmt, ...) - 详细日志（glyph 级别）
 */

#include <SDL3/SDL_log.h>

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

// 日志宏定义 - 编译时条件，零运行时开销
#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_ERROR
    #define DONG_LOG_ERROR(fmt, ...) SDL_Log("[ERROR] " fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_ERROR(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_WARN
    #define DONG_LOG_WARN(fmt, ...) SDL_Log("[WARN] " fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_WARN(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_INFO
    #define DONG_LOG_INFO(fmt, ...) SDL_Log("[INFO] " fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_INFO(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_DEBUG
    #define DONG_LOG_DEBUG(fmt, ...) SDL_Log("[DEBUG] " fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_DEBUG(fmt, ...) ((void)0)
#endif

#if DONG_LOG_LEVEL >= DONG_LOG_LEVEL_VERBOSE
    #define DONG_LOG_VERBOSE(fmt, ...) SDL_Log("[VERBOSE] " fmt, ##__VA_ARGS__)
#else
    #define DONG_LOG_VERBOSE(fmt, ...) ((void)0)
#endif
