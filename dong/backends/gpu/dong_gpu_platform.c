#include "dong_gpu_platform.h"
#include "dong_platform.h"

int dong_gpu_platform_init(void) {
    DongPlatform* platform = dong_platform_get();
    if (!platform) {
        return 0;
    }
    // Stub: full DongGPUDriver wiring via in-dreaming/gpu comes later.
    return 1;
}

void dong_gpu_platform_shutdown(void) {
    dong_platform_reset();
}
