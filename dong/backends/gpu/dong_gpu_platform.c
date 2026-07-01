#include "dong_gpu_platform.h"
#include "dong_gpu_backend_driver.h"
#include "dong_platform.h"
#include "dong_gtc.h"
#include "dong_ui_graph.h"

static DongGPUDriver* g_gpu_driver = NULL;

DongGPUDriver* dong_gpu_platform_get_driver(void) {
    return g_gpu_driver;
}

int dong_gpu_platform_init_ex(int embedded_mode) {
    DongPlatform* platform = dong_platform_get();
    if (!platform) {
        return 0;
    }

    g_gpu_driver = dong_gpu_backend_create_driver();
    if (!g_gpu_driver) {
        return 0;
    }

    dong_gpu_driver_set_embedded_mode(g_gpu_driver, embedded_mode);

    if (!dong_gpu_driver_initialize(g_gpu_driver)) {
        dong_gpu_backend_destroy_driver(g_gpu_driver);
        g_gpu_driver = NULL;
        return 0;
    }

    dong_platform_set_gpu_driver(platform, g_gpu_driver);
    return 1;
}

int dong_gpu_platform_init(void) {
    return dong_gpu_platform_init_ex(1);
}

void dong_gpu_platform_shutdown(void) {
    if (g_gpu_driver) {
        DongPlatform* platform = dong_platform_get();
        if (platform) {
            dong_platform_set_gpu_driver(platform, NULL);
        }
        dong_gpu_driver_shutdown(g_gpu_driver);
        dong_gtc_destroy(dong_gtc_get_default());
        dong_gpu_backend_destroy_driver(g_gpu_driver);
        g_gpu_driver = NULL;
    }
    dong_platform_reset();
}
