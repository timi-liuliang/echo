#pragma once

#include "engine/core/render/interface/mesh/Mesh.h"
#include "engine/core/render/interface/GPUBuffer.h"
#include "vk_render_base.h"

namespace Echo
{
    class VKMapping
    {
    public:
        // mapping primitive topology type
        static VkPrimitiveTopology MapPrimitiveTopology(Mesh::TopologyType type);

        // mapping gpubffer type
        static VkBufferUsageFlags MapGpuBufferType(GPUBuffer::GPUBufferType type);
    };
}
