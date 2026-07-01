#ifndef DONG_BACKEND_H
#define DONG_BACKEND_H

/* Compile-time backend selection (set by CMake for targets that opt in). */
#if defined(DONG_BACKEND_GPU)
#define DONG_BACKEND_NAME "gpu"
#elif defined(DONG_BACKEND_SDL)
#define DONG_BACKEND_NAME "sdl"
#elif defined(DONG_BACKEND_NONE)
#define DONG_BACKEND_NAME "none"
#else
#define DONG_BACKEND_NAME "unknown"
#endif

#endif /* DONG_BACKEND_H */
