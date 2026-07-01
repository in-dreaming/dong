#pragma once

#include "dong_image_atlas.h"
#include "gpu_resource_manager.hpp"

namespace dong::gpu_backend {

DongImageAtlas* gpu_image_atlas_create(GpuResourceManager* resources, const DongAtlasConfig* config);
void gpu_image_atlas_destroy(DongImageAtlas* atlas);

} // namespace dong::gpu_backend
