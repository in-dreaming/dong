/*
 * Windows compatibility layer for alloca.h
 * 
 * Note: __builtin_alloca must be called directly, not through a wrapper function,
 * because the allocated memory is released when the function returns.
 */
#ifndef QUICKJS_COMPAT_ALLOCA_H
#define QUICKJS_COMPAT_ALLOCA_H

#ifdef _WIN32
#include <malloc.h>
#include <stdio.h>

#ifdef __clang__
/* Use clang's built-in alloca */
#define alloca(size) __builtin_alloca(size)
#else
#define alloca _alloca
#endif
#endif

#endif /* QUICKJS_COMPAT_ALLOCA_H */
