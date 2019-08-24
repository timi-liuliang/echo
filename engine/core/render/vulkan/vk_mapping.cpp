#include "vk_mapping.h"

namespace Echo
{
    VkPrimitiveTopology VKMapping::MapPrimitiveTopology(Mesh::TopologyType type)
    {
        switch (type)
        {
        case Mesh::TT_POINTLIST:        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case Mesh::TT_LINELIST:         return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case Mesh::TT_LINESTRIP:        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case Mesh::TT_TRIANGLELIST:     return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case Mesh::TT_TRIANGLESTRIP:    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        default:                        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    VkBufferUsageFlags VKMapping::MapGpuBufferType(GPUBuffer::GPUBufferType type)
    {
        switch (type)
        {
        case GPUBuffer::GBT_INDEX:     return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case GPUBuffer::GBT_VERTEX:    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case GPUBuffer::GBT_UNIFORM:   return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        default:              
            {
                EchoLogError("Unknown GPUBufferType.")
                return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            }
        }
    }
}