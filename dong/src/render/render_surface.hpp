#pragma once

#include <cstdint>
#include <memory>

namespace dong::render {

// 渲染输出的抽象接口，支持 CPU 和 GPU 两种后端
class RenderSurface {
public:
    enum class Type {
        CPU_BUFFER,  // CPU 像素缓冲
        GPU_TEXTURE  // GPU 纹理
    };

    virtual ~RenderSurface() = default;

    // 获取表面类型
    virtual Type getType() const = 0;

    // 获取尺寸
    virtual uint32_t getWidth() const = 0;
    virtual uint32_t getHeight() const = 0;

    // 获取 CPU 缓冲（仅当类型为 CPU_BUFFER 时有效）
    virtual void* getCPUBuffer() = 0;
    virtual const void* getCPUBuffer() const = 0;

    // 获取 GPU 纹理 ID（仅当类型为 GPU_TEXTURE 时有效）
    virtual uint32_t getGPUTextureID() const { return 0; }

    // 标记缓冲为脏（需要更新）
    virtual void markDirty() = 0;
    virtual bool isDirty() const = 0;

    // 清空缓冲
    virtual void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) = 0;

    // 锁定缓冲以进行写操作（GPU 表面可能需要同步）
    virtual void lock() = 0;
    virtual void unlock() = 0;

    // 获取行宽（字节数）
    virtual uint32_t getStride() const = 0;
};

using RenderSurfacePtr = std::unique_ptr<RenderSurface>;

// CPU 像素缓冲实现
class CPUBufferSurface : public RenderSurface {
public:
    CPUBufferSurface(uint32_t width, uint32_t height);
    ~CPUBufferSurface();

    Type getType() const override { return Type::CPU_BUFFER; }
    uint32_t getWidth() const override { return width_; }
    uint32_t getHeight() const override { return height_; }
    uint32_t getStride() const override { return width_ * 4; } // RGBA

    void* getCPUBuffer() override { return buffer_; }
    const void* getCPUBuffer() const override { return buffer_; }

    void markDirty() override { is_dirty_ = true; }
    bool isDirty() const override { return is_dirty_; }

    void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) override;
    void lock() override {}   // CPU 缓冲无需加锁
    void unlock() override {} // CPU 缓冲无需解锁

    // 重新分配缓冲
    void resize(uint32_t new_width, uint32_t new_height);

private:
    uint32_t width_;
    uint32_t height_;
    uint8_t* buffer_;
    bool is_dirty_;
};

// GPU 纹理表面（未来扩展）
class GPUTextureSurface : public RenderSurface {
public:
    GPUTextureSurface(uint32_t width, uint32_t height, uint32_t texture_id);
    ~GPUTextureSurface();

    Type getType() const override { return Type::GPU_TEXTURE; }
    uint32_t getWidth() const override { return width_; }
    uint32_t getHeight() const override { return height_; }
    uint32_t getStride() const override { return width_ * 4; }

    uint32_t getGPUTextureID() const override { return texture_id_; }

    void* getCPUBuffer() override { return nullptr; } // GPU 表面无 CPU 缓冲
    const void* getCPUBuffer() const override { return nullptr; }

    void markDirty() override { is_dirty_ = true; }
    bool isDirty() const override { return is_dirty_; }

    void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) override;
    void lock() override;   // GPU 表面需要同步
    void unlock() override; // GPU 表面需要同步

private:
    uint32_t width_;
    uint32_t height_;
    uint32_t texture_id_;
    bool is_dirty_;
};

} // namespace dong::render
