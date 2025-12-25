/*
 * Windows compatibility layer for alloca.h
 */
#ifndef QUICKJS_COMPAT_ALLOCA_H
#define QUICKJS_COMPAT_ALLOCA_H

#ifdef _WIN32
#include <malloc.h>
#define alloca _alloca
#endif

#endif /* QUICKJS_COMPAT_ALLOCA_H */
