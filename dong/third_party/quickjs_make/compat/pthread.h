/*
 * Minimal pthread stub for Windows MSVC/clang-cl builds
 * QuickJS uses pthread for CONFIG_ATOMICS but we can disable it
 */
#ifndef QUICKJS_COMPAT_PTHREAD_H
#define QUICKJS_COMPAT_PTHREAD_H

#ifdef _WIN32

#include <windows.h>

typedef HANDLE pthread_t;
typedef CRITICAL_SECTION pthread_mutex_t;
typedef void* pthread_attr_t;
typedef void* pthread_mutexattr_t;

#define PTHREAD_MUTEX_INITIALIZER {0}

static inline int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
    (void)attr;
    InitializeCriticalSection(mutex);
    return 0;
}

static inline int pthread_mutex_destroy(pthread_mutex_t *mutex) {
    DeleteCriticalSection(mutex);
    return 0;
}

static inline int pthread_mutex_lock(pthread_mutex_t *mutex) {
    EnterCriticalSection(mutex);
    return 0;
}

static inline int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    LeaveCriticalSection(mutex);
    return 0;
}

/* Thread creation - minimal stub */
static inline int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                                  void *(*start_routine)(void*), void *arg) {
    (void)attr;
    *thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start_routine, arg, 0, NULL);
    return (*thread == NULL) ? -1 : 0;
}

static inline int pthread_join(pthread_t thread, void **retval) {
    (void)retval;
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

static inline pthread_t pthread_self(void) {
    return GetCurrentThread();
}

#else
/* Non-Windows: use the system header */
#include_next <pthread.h>
#endif

#endif /* QUICKJS_COMPAT_PTHREAD_H */
