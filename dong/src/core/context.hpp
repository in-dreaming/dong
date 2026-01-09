#pragma once

#include <memory>

namespace dong {

class Context {
public:
    Context();
    ~Context();

    // 资源初始化
    bool initialize();
    bool isInitialized() const { return initialized_; }

    // 资源回收
    void shutdown();

    // 版本与信息
    static const char* getVersion() { return "0.1.0"; }
    static const char* getName() { return "Dong UI Engine"; }

private:
    bool initialized_;
    bool shutdown_called_;
};

using ContextPtr = std::shared_ptr<Context>;

} // namespace dong
