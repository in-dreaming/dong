/**
 * Placeholder for SDL-only demos when DONG_BACKEND=gpu.
 */
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
static const char* exe_basename(void) {
    static char name[260];
    DWORD n = GetModuleFileNameA(NULL, name, sizeof(name));
    if (n == 0 || n >= sizeof(name)) {
        return "demo";
    }
    const char* slash = strrchr(name, '\\');
    return slash ? slash + 1 : name;
}
#else
#include <libgen.h>
static const char* exe_basename(void) {
    extern char** environ;
    (void)environ;
    return "demo";
}
#endif

int main(int argc, char** argv) {
    const char* name = (argc > 0 && argv[0]) ? argv[0] : exe_basename();
    fprintf(stderr,
            "[Dong] %s requires -Dbackend=sdl (SDL GPU / AppCore).\n"
            "  GPU backend HTML viewer: dong_app.exe --html <file>\n"
            "  GPU backend smoke test: gpu_ui_inject.exe\n",
            name);
    return 2;
}
