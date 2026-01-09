#pragma once

#include <cstdint>
#include <memory>

namespace dong::render {

// Forward declarations
struct GPUCommandList;
class DisplayList;
struct LayerTree;

// Platform-agnostic GPU driver interface
// This allows the core library to drive GPU rendering without depending on SDL
class IGPUDriver {
public:
    virtual ~IGPUDriver() = default;
    
    virtual bool initialize() = 0;
    virtual void prepareResources(const GPUCommandList& commands) = 0;
    virtual void beginFrame() = 0;
    virtual void execute(const GPUCommandList& commands) = 0;
    virtual void endFrame() = 0;
};

// Factory function type for creating GPU drivers
// Implemented by platform-specific code (e.g., SDL plugin)
using GPUDriverFactory = std::unique_ptr<IGPUDriver>(*)(void* device, void* window);

} // namespace dong::render
