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

    // TODO: Initialize platform-specific resources if needed
    // - Font management
    // - Color space
    // - DPI settings

    initialized_ = true;
    return true;
}

void Context::shutdown() {
    if (shutdown_called_) {
        return; // Already shut down
    }

    if (initialized_) {
        std::cout << "[Context] Shutting down " << getName() << std::endl;
        // TODO: Clean up platform resources
    }

    shutdown_called_ = true;
    initialized_ = false;
}

} // namespace dong
