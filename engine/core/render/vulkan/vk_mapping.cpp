#include "vk_mapping.h"

namespace Echo
{
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