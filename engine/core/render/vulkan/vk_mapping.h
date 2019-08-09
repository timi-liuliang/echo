#pragma once

#include "engine/core/render/interface/GPUBuffer.h"
#include "vk_render_base.h"

namespace Echo
{
    class VKMapping
    {
    public:
        // mapping gpubffer type
        static VkBufferUsageFlags MapGpuBufferType(GPUBuffer::GPUBufferType type);
    };
}
