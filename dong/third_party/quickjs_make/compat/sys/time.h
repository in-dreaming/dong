/*
 * Windows compatibility layer for sys/time.h
 * Provides gettimeofday() for MSVC/clang-cl builds
 */
#ifndef QUICKJS_COMPAT_SYS_TIME_H
#define QUICKJS_COMPAT_SYS_TIME_H

#ifdef _WIN32

#include <winsock2.h>  /* for struct timeval */
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/* gettimeofday implementation for Windows */
static inline int gettimeofday(struct timeval *tv, void *tz)
{
    (void)tz;
    if (tv) {
        FILETIME ft;
        ULARGE_INTEGER uli;
        
        GetSystemTimeAsFileTime(&ft);
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        
        /* Convert from 100-nanosecond intervals since Jan 1, 1601
           to seconds and microseconds since Jan 1, 1970 */
        uli.QuadPart -= 116444736000000000ULL;
        tv->tv_sec = (long)(uli.QuadPart / 10000000ULL);
        tv->tv_usec = (long)((uli.QuadPart % 10000000ULL) / 10);
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#else
/* Non-Windows: use the system header */
#include_next <sys/time.h>
#endif

#endif /* QUICKJS_COMPAT_SYS_TIME_H */
