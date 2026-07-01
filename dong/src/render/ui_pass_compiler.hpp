#pragma once

#include "dong_ui_pass_bundle.h"
#include "gpu_ir.hpp"

namespace dong::render {

class UiPassCompiler {
public:
    DongUiPassBundle compile(const GPUCommandList& list);
};

} // namespace dong::render
