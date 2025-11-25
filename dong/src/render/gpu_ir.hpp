#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include "display_list.hpp"

namespace dong::render {

// GPU 级别的命令类型（与具体后端无关）
enum class GPUCommandType : uint8_t {
    BeginFrame,
    EndFrame,
    BeginPass,
    EndPass,
    SetViewport,
    SetPipeline,
    BindTexture,
    BindSampler,
    BindInstanceBuffer,
    DrawInstancedQuads,
    DrawImageQuad,
    DrawRoundedRectQuad,
};

struct GPUViewport {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

// 为了保持 IR 简单，这里用整型 id 代表各种资源（pipeline / texture / buffer）
using PipelineId = uint32_t;
using TextureId = uint32_t;
using BufferId = uint32_t;

struct GPUCommand {
    GPUCommandType type;

    GPUViewport viewport{};
    PipelineId pipeline = 0;
    TextureId texture = 0;
    BufferId instance_buffer = 0;
    uint32_t instance_count = 0;

    // 最小实现：为 DrawRect / DrawRoundedRect / DrawImage 携带一份立即模式的矩形和颜色数据
    Rect rect;      // 目标矩形（像素坐标）
    Color color;    // 颜色或调制色（用于纯色/圆角矩形、后续也可用于调制图片）
    float radius = 0.0f; // 圆角矩形半径（仅在 DrawRoundedRectQuad 时使用）

    // 图片绘制专用字段（仅在 DrawImageQuad 时使用）
    std::string image_src; // 原始图片资源标识（路径）
    float opacity = 1.0f;  // 图片整体透明度
};

struct GPUCommandList {
    std::vector<GPUCommand> commands;
};

// DisplayList → GPUCommandList 的编译器骨架
class GPUCompiler {
public:
    // 简化版编译接口：只处理最基础的 rect / image 类型
    void compile(const DisplayList& dl, GPUCommandList& out) {
        out.commands.clear();

        // 一个最小的实现：
        // - 为整个 DisplayList 生成一对 BeginFrame/EndFrame
        // - 在其中包一对 BeginPass/EndPass
        GPUCommand begin_frame{};
        begin_frame.type = GPUCommandType::BeginFrame;
        out.commands.push_back(begin_frame);

        GPUCommand begin_pass{};
        begin_pass.type = GPUCommandType::BeginPass;
        out.commands.push_back(begin_pass);

        for (const auto& item : dl.items) {
            switch (item.type) {
            case DisplayItemType::DrawRect: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::DrawInstancedQuads;
                cmd.instance_count = 1;
                cmd.rect = item.rect.rect;
                cmd.color = item.rect.color;
                out.commands.push_back(cmd);
                break;
            }
            case DisplayItemType::DrawRoundedRect: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::DrawRoundedRectQuad;
                cmd.instance_count = 1;
                cmd.rect = item.rounded_rect.rect;
                cmd.color = item.rounded_rect.color;
                cmd.radius = item.rounded_rect.radius;
                out.commands.push_back(cmd);
                break;
            }
            case DisplayItemType::DrawImage: {
                GPUCommand cmd{};
                cmd.type = GPUCommandType::DrawImageQuad;
                cmd.instance_count = 1;
                cmd.rect = item.image.rect;
                cmd.opacity = item.image.opacity;
                cmd.image_src = item.image.src;
                out.commands.push_back(cmd);
                break;
            }
            default:
                // 其他类型先忽略，后续再逐步接入
                break;
            }
        }

        GPUCommand end_pass{};
        end_pass.type = GPUCommandType::EndPass;
        out.commands.push_back(end_pass);

        GPUCommand end_frame{};
        end_frame.type = GPUCommandType::EndFrame;
        out.commands.push_back(end_frame);
    }
};

} // namespace dong::render
