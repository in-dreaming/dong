#include "scene3d_world_gpu.h"

#include "dong_gpu_backend_driver.h"
#include "scene3d_world_draw.hpp"

#include "dong_gpu_driver.h"

#include "gpu/gpu.h"
#include "gpu/platform/gpu_surface.h"

#include "../../src/core/log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#ifndef DONG_GPU_SHADER_DIR
#define DONG_GPU_SHADER_DIR "shaders"
#endif

namespace {
// Debug-only: dumps the just-rendered (pre-present) backbuffer to a BMP so the composited 3D
// scene can be pixel-verified without relying on OS screen capture (unreliable over RDP/virtual
// desktops). Enable with DONG_GPU_DUMP_WORLD_DIR=<dir> [DONG_GPU_DUMP_WORLD_EVERY=<N>, default 30].
bool writeWorldDebugBMP(const char* path, uint32_t width, uint32_t height, const uint8_t* rgba) {
    if (!path || !rgba || width == 0 || height == 0) {
        return false;
    }
    const uint32_t row_bytes = ((width * 3 + 3) / 4) * 4;
    const uint32_t pixel_bytes = row_bytes * height;
    const uint32_t file_size = 14 + 40 + pixel_bytes;
    FILE* f = std::fopen(path, "wb");
    if (!f) {
        return false;
    }
    uint8_t header[14] = {'B', 'M'};
    header[2] = static_cast<uint8_t>(file_size);
    header[3] = static_cast<uint8_t>(file_size >> 8);
    header[4] = static_cast<uint8_t>(file_size >> 16);
    header[5] = static_cast<uint8_t>(file_size >> 24);
    header[10] = 54;
    std::fwrite(header, 1, 14, f);

    uint8_t info[40] = {0};
    info[0] = 40;
    info[4] = static_cast<uint8_t>(width);
    info[5] = static_cast<uint8_t>(width >> 8);
    info[6] = static_cast<uint8_t>(width >> 16);
    info[7] = static_cast<uint8_t>(width >> 24);
    info[8] = static_cast<uint8_t>(height);
    info[9] = static_cast<uint8_t>(height >> 8);
    info[10] = static_cast<uint8_t>(height >> 16);
    info[11] = static_cast<uint8_t>(height >> 24);
    info[12] = 1;
    info[14] = 24;
    std::fwrite(info, 1, 40, f);

    std::vector<uint8_t> row(row_bytes, 0);
    for (int32_t y = static_cast<int32_t>(height) - 1; y >= 0; --y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* p = rgba + (static_cast<uint32_t>(y) * width + x) * 4;
            row[x * 3 + 0] = p[2];
            row[x * 3 + 1] = p[1];
            row[x * 3 + 2] = p[0];
        }
        std::fwrite(row.data(), 1, row_bytes, f);
    }
    std::fclose(f);
    return true;
}

void maybeDumpWorldBackbuffer(GpuDevice device, GpuCommandQueue queue, GpuSurfaceTexture backbuffer, uint32_t width,
                              uint32_t height) {
    static const char* dir = std::getenv("DONG_GPU_DUMP_WORLD_DIR");
    if (!dir || !backbuffer || !device || width == 0 || height == 0) {
        return;
    }
    static const char* every_env = std::getenv("DONG_GPU_DUMP_WORLD_EVERY");
    const uint64_t every = every_env ? static_cast<uint64_t>(std::atoi(every_env)) : 30;
    static uint64_t counter = 0;
    const uint64_t current = counter++;
    if (every == 0 || (current % every) != 0) {
        return;
    }

    // readTexture() reads whatever is currently in the resource; the just-submitted draw/blit
    // must be finished on the GPU first or the readback races the render and captures stale data.
    if (queue) {
        gpuQueueWaitOnHost(queue);
    }

    std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);
    uint32_t actual_w = 0;
    uint32_t actual_h = 0;
    if (gpuSurfaceTextureReadbackRGBA(device, backbuffer, pixels.data(), pixels.size(), &actual_w, &actual_h) !=
        GPU_SUCCESS) {
        std::fprintf(stderr, "[Scene3DWorldGpu] world frame dump readback failed\n");
        return;
    }
    char path[512];
    std::snprintf(path, sizeof(path), "%s/world_backbuffer_%06llu.bmp", dir, static_cast<unsigned long long>(current));
    if (writeWorldDebugBMP(path, actual_w ? actual_w : width, actual_h ? actual_h : height, pixels.data())) {
        std::fprintf(stderr, "[Scene3DWorldGpu] dumped world backbuffer -> %s\n", path);
    }
}
} // namespace

struct Scene3DWorldGpu {
    GpuDevice device = nullptr;
    GpuShaderCompiler compiler = nullptr;
    GpuShaderProgram program = nullptr;
    GpuRenderPipeline pipeline = nullptr;
    GpuSamplerHandle sampler{};
    GpuTextureHandle depth_tex{};
    GpuTextureHandle depth_view{};
    uint32_t depth_w = 0;
    uint32_t depth_h = 0;
    GpuFormat color_format = GPU_FORMAT_RGBA8_UNORM;
    GpuFormat depth_format = GPU_FORMAT_D32_FLOAT;
};

static GpuFormat pick_depth_format(GpuDevice device) {
    const GpuFormat candidates[] = {GPU_FORMAT_D32_FLOAT, GPU_FORMAT_D16_UNORM};
    for (GpuFormat fmt : candidates) {
        GpuTextureDesc probe{};
        probe.type = GPU_TEXTURE_TYPE_2D;
        probe.width = 4;
        probe.height = 4;
        probe.depth = 1;
        probe.arrayLength = 1;
        probe.mipCount = 1;
        probe.format = fmt;
        probe.sampleCount = 1;
        probe.usage = GPU_TEXTURE_USAGE_DEPTH_STENCIL;
        GpuTextureHandle tmp{};
        if (gpuCreateTexture(device, &probe, &tmp) == GPU_SUCCESS) {
            gpuDestroyTexture(device, tmp);
            return fmt;
        }
    }
    return GPU_FORMAT_D32_FLOAT;
}

static bool ensure_depth(Scene3DWorldGpu* world, uint32_t w, uint32_t h) {
    if (!world || !world->device || w == 0 || h == 0) {
        return false;
    }
    if (world->depth_tex.index != 0 && world->depth_w == w && world->depth_h == h) {
        return true;
    }
    if (world->depth_view.index != 0) {
        gpuDestroyTextureView(world->device, world->depth_view);
        world->depth_view = {};
    }
    if (world->depth_tex.index != 0) {
        gpuDestroyTexture(world->device, world->depth_tex);
        world->depth_tex = {};
    }

    GpuTextureDesc desc{};
    desc.type = GPU_TEXTURE_TYPE_2D;
    desc.width = w;
    desc.height = h;
    desc.depth = 1;
    desc.arrayLength = 1;
    desc.mipCount = 1;
    desc.format = world->depth_format;
    desc.sampleCount = 1;
    desc.usage = GPU_TEXTURE_USAGE_DEPTH_STENCIL;
    desc.label = "scene3d_depth";
    if (gpuCreateTexture(world->device, &desc, &world->depth_tex) != GPU_SUCCESS) {
        return false;
    }
    if (gpuCreateTextureView(world->device, world->depth_tex, GPU_TEXTURE_VIEW_TYPE_DEPTH_STENCIL,
                             &world->depth_view) != GPU_SUCCESS) {
        return false;
    }
    world->depth_w = w;
    world->depth_h = h;
    return true;
}

static bool compile_world_pipeline(Scene3DWorldGpu* world, const char* shader_dir) {
    if (!world || !world->device || !shader_dir) {
        return false;
    }

    char shader_path[512];
    std::snprintf(shader_path, sizeof(shader_path), "%s/scene3d_textured.slang", shader_dir);

    if (gpuCreateShaderCompiler(world->device, &world->compiler) != GPU_SUCCESS) {
        return false;
    }

    GpuShaderCompileDesc compile_desc{};
    compile_desc.sourcePath = shader_path;
    compile_desc.entryPoint = "vsMain";
    compile_desc.fragmentEntryPoint = "fsMain";
    compile_desc.target = GPU_SHADER_TARGET_SPIRV;
    if (gpuCompileShader(world->compiler, &compile_desc, &world->program) != GPU_SUCCESS) {
        DONG_LOG_ERROR("scene3d_world: shader compile failed: %s",
                       gpuGetShaderCompileDiagnostic(world->compiler));
        return false;
    }

    world->pipeline = static_cast<GpuRenderPipeline>(dong_gpu_backend_create_scene3d_render_pipeline(
        world->device, world->program, static_cast<uint32_t>(world->color_format),
        static_cast<uint32_t>(world->depth_format)));
    if (!world->pipeline) {
        DONG_LOG_ERROR("scene3d_world: pipeline creation failed (color_fmt=%u depth_fmt=%u)",
                       static_cast<unsigned>(world->color_format), static_cast<unsigned>(world->depth_format));
        return false;
    }

    GpuSamplerDesc samp_desc{};
    samp_desc.minFilter = GPU_FILTER_LINEAR;
    samp_desc.magFilter = GPU_FILTER_LINEAR;
    samp_desc.mipFilter = GPU_FILTER_LINEAR;
    samp_desc.addressModeU = GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samp_desc.addressModeV = GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samp_desc.addressModeW = GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if (gpuCreateSampler(world->device, &samp_desc, &world->sampler) != GPU_SUCCESS) {
        return false;
    }

    return true;
}

extern "C" Scene3DWorldGpu* scene3d_world_gpu_create(void* device, uint32_t color_format,
                                                     const char* shader_dir) {
    auto* world = new Scene3DWorldGpu();
    world->device = static_cast<GpuDevice>(device);
    world->color_format = static_cast<GpuFormat>(color_format);
    world->depth_format = pick_depth_format(world->device);
    if (!compile_world_pipeline(world, shader_dir)) {
        scene3d_world_gpu_destroy(world);
        return nullptr;
    }
    return world;
}

extern "C" void scene3d_world_gpu_destroy(Scene3DWorldGpu* world) {
    if (!world) {
        return;
    }
    if (world->device) {
        if (world->sampler.index != 0) {
            gpuDestroySampler(world->device, world->sampler);
        }
        if (world->pipeline) {
            dong_gpu_backend_destroy_scene3d_render_pipeline(world->pipeline);
            world->pipeline = nullptr;
        }
        if (world->program) {
            gpuDestroyShaderProgram(world->program);
        }
        if (world->compiler) {
            gpuDestroyShaderCompiler(world->compiler);
        }
        if (world->depth_view.index != 0) {
            gpuDestroyTextureView(world->device, world->depth_view);
        }
        if (world->depth_tex.index != 0) {
            gpuDestroyTexture(world->device, world->depth_tex);
        }
    }
    delete world;
}

struct Scene3DWorldUniforms {
    float mvp[16];
    float model[16];
    float color[4];
    float highlight[4];
};

extern "C" int scene3d_world_gpu_render(Scene3DWorldGpu* world, const Scene3DWorldFrameDesc* frame) {
    if (!world || !frame || !frame->device || !frame->queue || !frame->surface) {
        return 0;
    }

    GpuSurfaceTexture backbuffer = nullptr;
    if (gpuSurfaceAcquireNextImage(static_cast<GpuSurface>(frame->surface), &backbuffer) != GPU_SUCCESS) {
        return 0;
    }

    const uint32_t rw = frame->width > 0 ? frame->width : 1;
    const uint32_t rh = frame->height > 0 ? frame->height : 1;
    if (frame->depth_test && !ensure_depth(world, rw, rh)) {
        gpuSurfaceTextureRelease(backbuffer);
        return 0;
    }

    GpuCommandEncoder encoder =
        gpuBeginCommandEncoder(static_cast<GpuDevice>(frame->device), static_cast<GpuCommandQueue>(frame->queue));
    if (!encoder) {
        gpuSurfaceTextureRelease(backbuffer);
        return 0;
    }

    GpuRenderPassColorAttachment color_att = {};
    color_att.attachment = backbuffer;
    color_att.loadOp = GPU_LOAD_OP_CLEAR;
    color_att.storeOp = GPU_STORE_OP_STORE;
    color_att.clearValue[0] = frame->bg_r;
    color_att.clearValue[1] = frame->bg_g;
    color_att.clearValue[2] = frame->bg_b;
    color_att.clearValue[3] = frame->bg_a;

    GpuRenderPassDepthAttachment depth_att = {};
    const GpuRenderPassDepthAttachment* depth_ptr = nullptr;
    if (frame->depth_test && world->depth_view.index != 0) {
        depth_att.textureHandle = world->depth_tex;
        depth_att.viewHandle = world->depth_view;
        depth_att.depthLoadOp = GPU_LOAD_OP_CLEAR;
        depth_att.depthStoreOp = GPU_STORE_OP_DONT_CARE;
        depth_att.clearDepth = 1.0f;
        depth_ptr = &depth_att;
    }

    GpuRenderPassDesc rp_desc = {};
    rp_desc.colorAttachmentCount = 1;
    rp_desc.colorAttachments = &color_att;
    rp_desc.depthAttachment = depth_ptr;

    GpuRenderPassEncoder pass = gpuCmdBeginRenderPass(encoder, &rp_desc);
    if (!pass) {
        gpuSurfaceTextureRelease(backbuffer);
        return 0;
    }

    gpuCmdSetViewport(pass, 0.0f, 0.0f, static_cast<float>(rw), static_cast<float>(rh));
    gpuCmdBindRenderPipeline(pass, world->pipeline);

    static const bool s_debug_scene3d = std::getenv("DONG_SCENE3D_DEBUG") != nullptr;
    int null_tex = 0, view_fail = 0, bind_fail = 0, drawn = 0;
    for (int i = 0; i < frame->screen_count; ++i) {
        const Scene3DWorldScreenDraw& scr = frame->screens[i];
        if (!scr.texture) {
            ++null_tex;
            continue;
        }
        DongGpuTextureViewHandle view_handle{};
        if (!dong_gpu_backend_get_texture_shader_view(static_cast<DongGPUDriver*>(frame->driver),
                                                      static_cast<DongGPUTexture>(scr.texture),
                                                      &view_handle)) {
            ++view_fail;
            continue;
        }

        Scene3DWorldUniforms u{};
        std::memcpy(u.mvp, scr.mvp, sizeof(u.mvp));
        std::memcpy(u.model, scr.model, sizeof(u.model));
        u.color[0] = u.color[1] = u.color[2] = u.color[3] = 1.0f;
        u.highlight[0] = scr.highlighted ? 1.0f : 0.0f;

        if (s_debug_scene3d && i == 0) {
            std::fprintf(stderr, "[Scene3DWorldGpu] screen[0] mvp row0=(%.3f,%.3f,%.3f,%.3f)\n", u.mvp[0], u.mvp[1],
                         u.mvp[2], u.mvp[3]);
        }

        if (!dong_gpu_backend_scene3d_bind_textured_draw(world->device, pass, &world->sampler, &view_handle, &u,
                                                           sizeof(u))) {
            ++bind_fail;
            continue;
        }
        ++drawn;
        gpuCmdDraw(pass, 6, 1, 0, 0);
    }
    if (s_debug_scene3d) {
        std::fprintf(stderr,
                     "[Scene3DWorldGpu] screen_count=%d drawn=%d null_tex=%d view_fail=%d bind_fail=%d\n",
                     frame->screen_count, drawn, null_tex, view_fail, bind_fail);
    }

    gpuCmdEndRenderPass(pass);
    GpuCommandBuffer cmd = gpuFinishCommandEncoder(encoder);
    if (cmd) {
        gpuQueueSubmit(static_cast<GpuCommandQueue>(frame->queue), 1, &cmd);
    }

    maybeDumpWorldBackbuffer(static_cast<GpuDevice>(frame->device), static_cast<GpuCommandQueue>(frame->queue),
                              backbuffer, rw, rh);

    gpuSurfacePresent(static_cast<GpuSurface>(frame->surface));
    gpuSurfaceTextureRelease(backbuffer);
    return 1;
}
