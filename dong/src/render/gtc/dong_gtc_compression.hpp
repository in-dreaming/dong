#include "dong_gtc_format_map.hpp"

#include "../../../third_party/gpu_texture_compress/include/shared_types.h"
#include "../../../third_party/gpu_texture_compress/src/compute_dispatch.h"
#include "../../../third_party/gpu_texture_compress/src/shader_compiler.h"
#include "../../../third_party/gpu_texture_compress/src/texture_loader.h"

#include <SDL3/SDL_gpu.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace dong::gtc_internal {

class CompressionService {
public:
    explicit CompressionService(SDL_GPUDevice* device, std::string shader_dir)
        : device_(device)
        , shader_dir_(std::move(shader_dir))
        , compiler_(device)
        , dispatch_(device)
        , loader_(device) {}

    bool prepare(DongImageFormat fmt) {
        ::gtc::GtcFormat gtc_fmt{};
        if (!image_format_to_gtc(fmt, &gtc_fmt)) {
            return false;
        }
        const int key = static_cast<int>(gtc_fmt);
        if (pipelines_.count(key) && pipelines_[key]) {
            return true;
        }

        const auto& info = ::gtc::get_format_info(gtc_fmt);
        const std::string path = shader_dir_ + "/" + info.shader_file;
        ::gtc::CompiledShader shader = compiler_.compile_compute(path, "MainCS");
        if (shader.bytecode.empty()) {
            return false;
        }

        SDL_GPUComputePipeline* pipeline = dispatch_.create_pipeline(
            shader,
            /*num_samplers=*/1,
            /*num_readonly_storage_textures=*/0,
            /*num_readwrite_storage_textures=*/0,
            /*num_readonly_storage_buffers=*/0,
            /*num_readwrite_storage_buffers=*/1,
            /*num_uniform_buffers=*/1);
        if (!pipeline) {
            return false;
        }
        pipelines_[key] = pipeline;
        return true;
    }

    bool compress_rgba(const uint8_t* rgba, uint32_t width, uint32_t height,
                         DongImageFormat dst_fmt, int quality, int flags,
                         std::vector<uint8_t>& out_bytes) {
        if (!rgba || width == 0 || height == 0) {
            return false;
        }
        if (!prepare(dst_fmt)) {
            return false;
        }

        ::gtc::GtcFormat gtc_fmt{};
        if (!image_format_to_gtc(dst_fmt, &gtc_fmt)) {
            return false;
        }

        ::gtc::TextureData cpu_tex;
        cpu_tex.width = width;
        cpu_tex.height = height;
        cpu_tex.channels = 4;
        cpu_tex.pixels.assign(rgba, rgba + (size_t)width * height * 4);

        ::gtc::GpuTexture src = loader_.upload_to_gpu(cpu_tex);
        if (!src.texture) {
            return false;
        }

        const bool ok = compress_gpu_texture(src, gtc_fmt, quality, flags, out_bytes);
        loader_.release(src);
        return ok;
    }

    bool compress_to_texture_region(const uint8_t* rgba, uint32_t width, uint32_t height,
                                    DongImageFormat dst_fmt, SDL_GPUTexture* dst_texture,
                                    uint32_t dst_x, uint32_t dst_y) {
        std::vector<uint8_t> compressed;
        if (!compress_rgba(rgba, width, height, dst_fmt, 1, 0, compressed)) {
            return false;
        }
        if (!dst_texture || compressed.empty()) {
            return false;
        }

        SDL_GPUTransferBufferCreateInfo tb_info{};
        tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        tb_info.size = (Uint32)compressed.size();
        SDL_GPUTransferBuffer* upload_buf = SDL_CreateGPUTransferBuffer(device_, &tb_info);
        if (!upload_buf) {
            return false;
        }

        void* mapped = SDL_MapGPUTransferBuffer(device_, upload_buf, false);
        if (!mapped) {
            SDL_ReleaseGPUTransferBuffer(device_, upload_buf);
            return false;
        }
        std::memcpy(mapped, compressed.data(), compressed.size());
        SDL_UnmapGPUTransferBuffer(device_, upload_buf);

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device_);
        if (!cmd) {
            SDL_ReleaseGPUTransferBuffer(device_, upload_buf);
            return false;
        }

        SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd);
        if (!copy_pass) {
            SDL_ReleaseGPUTransferBuffer(device_, upload_buf);
            return false;
        }

        SDL_GPUTextureTransferInfo src_info{};
        src_info.transfer_buffer = upload_buf;
        src_info.offset = 0;
        src_info.pixels_per_row = width;
        src_info.rows_per_layer = height;

        SDL_GPUTextureRegion dst_region{};
        dst_region.texture = dst_texture;
        dst_region.x = dst_x;
        dst_region.y = dst_y;
        dst_region.z = 0;
        dst_region.w = width;
        dst_region.h = height;
        dst_region.d = 1;

        SDL_UploadToGPUTexture(copy_pass, &src_info, &dst_region, true);
        SDL_EndGPUCopyPass(copy_pass);
        SDL_SubmitGPUCommandBuffer(cmd);

        SDL_ReleaseGPUTransferBuffer(device_, upload_buf);
        return true;
    }

private:
    bool compress_gpu_texture(const ::gtc::GpuTexture& source, ::gtc::GtcFormat format,
                              int quality_level, int flags, std::vector<uint8_t>& out_bytes) {
        const int key = static_cast<int>(format);
        auto it = pipelines_.find(key);
        if (it == pipelines_.end() || !it->second) {
            return false;
        }

        const auto& info = ::gtc::get_format_info(format);
        const uint32_t blocks_x = (source.width + info.block_width - 1) / info.block_width;
        const uint32_t blocks_y = (source.height + info.block_height - 1) / info.block_height;
        const uint32_t output_size = blocks_x * blocks_y * info.block_bytes;

        SDL_GPUBuffer* output_buffer = dispatch_.create_buffer(
            output_size, SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE);
        if (!output_buffer) {
            return false;
        }

        ::gtc::CompressParams params{};
        params.TexWidth = (int32_t)source.width;
        params.TexHeight = (int32_t)source.height;
        params.BlocksX = (int32_t)blocks_x;
        params.BlocksY = (int32_t)blocks_y;
        params.QualityLevel = quality_level;
        params.Flags = flags;
        params.Pad0 = 0.0f;
        params.Pad1 = 0.0f;

        ::gtc::ComputeBindings bindings;
        ::gtc::ComputeBindings::SamplerBinding tex_bind;
        tex_bind.texture = source.texture;
        tex_bind.sampler = dispatch_.create_point_sampler();
        bindings.texture_samplers.push_back(tex_bind);
        bindings.readwrite_storage_buffers.push_back(output_buffer);
        bindings.uniform_data = &params;
        bindings.uniform_size = sizeof(params);

        const ::gtc::DispatchDims dims = dispatch_.calc_dispatch(
            source.width, source.height, info.block_width, info.block_height, 8, 8);

        if (!dispatch_.dispatch_sync(it->second, bindings, dims)) {
            SDL_ReleaseGPUBuffer(device_, output_buffer);
            return false;
        }

        out_bytes.resize(output_size);
        const bool downloaded = dispatch_.download_buffer(output_buffer, out_bytes.data(), output_size);
        SDL_ReleaseGPUBuffer(device_, output_buffer);
        return downloaded;
    }

    SDL_GPUDevice* device_;
    std::string shader_dir_;
    ::gtc::ShaderCompiler compiler_;
    ::gtc::ComputeDispatch dispatch_;
    ::gtc::TextureLoader loader_;
    std::unordered_map<int, SDL_GPUComputePipeline*> pipelines_;
};

} // namespace dong::gtc_internal
