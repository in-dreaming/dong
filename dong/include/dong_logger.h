#ifndef DONG_LOGGER_H
#define DONG_LOGGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Dong Logger (Platform-injected)
// =============================================================================
// The core engine should not assume any particular logging backend.
// Applications/backends can inject a logger via dong_platform_set_logger().
//
// If no logger is injected, the Platform provides a default logger.

typedef enum DongLoggerLevel {
    DONG_LOGGER_LEVEL_TRACE = 0,
    DONG_LOGGER_LEVEL_DEBUG = 1,
    DONG_LOGGER_LEVEL_INFO  = 2,
    DONG_LOGGER_LEVEL_WARN  = 3,
    DONG_LOGGER_LEVEL_ERROR = 4,
} DongLoggerLevel;

typedef struct DongLogger DongLogger;

typedef struct DongLoggerVTable {
    void (*log)(DongLogger* logger, DongLoggerLevel level, const char* message);
} DongLoggerVTable;

struct DongLogger {
    const DongLoggerVTable* vtable;
    void* user_data;
};

static inline void dong_logger_log(DongLogger* logger, DongLoggerLevel level, const char* message) {
    if (logger && logger->vtable && logger->vtable->log) {
        logger->vtable->log(logger, level, message);
    }
}

#ifdef __cplusplus
}
#endif

#endif // DONG_LOGGER_H
