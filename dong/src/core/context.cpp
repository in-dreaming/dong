#include "context.hpp"
#include <iostream>

namespace dong {

Context::Context() : initialized_(false), shutdown_called_(false) {
}

Context::~Context() {
    if (!shutdown_called_) {
        shutdown();
    }
}

bool Context::initialize() {
    if (initialized_) {
        return true; // Already initialized
    }

    std::cout << "[Context] Initializing " << getName() << " v" << getVersion() << std::endl;

    // Platform-specific resources are now injected via Plugin system:
    // - GPUDriver: injected via DongGPUDriver C interface
    // - ImageDecoder: injected via DongImageDecoder interface
    // - FileSystem: injected via DongFileSystem interface
    // No additional initialization needed here.

    initialized_ = true;
    return true;
}

void Context::shutdown() {
    if (shutdown_called_) {
        return; // Already shut down
    }

    if (initialized_) {
        std::cout << "[Context] Shutting down " << getName() << std::endl;
        // Clean up platform resources
        // Note: Font finder cleanup is handled automatically via static destructors
    }

    shutdown_called_ = true;
    initialized_ = false;
}

} // namespace dong
