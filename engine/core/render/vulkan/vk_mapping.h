#pragma once

#include "base/mesh/MeshRes.h"
#include "base/ShaderProgram.h"
#include "base/GPUBuffer.h"
#include "vk_render_base.h"

namespace Echo
{
    class VKMapping
    {
    public:
        // mapping primitive topology type
        static VkPrimitiveTopology MapPrimitiveTopology(MeshRes::TopologyType type);

        // mapping vertex semantic
        static String MapVertexSemanticString(VertexSemantic semantic);

        // mapping gpubffer type
        static VkBufferUsageFlags MapGpuBufferUsageFlags(GPUBuffer::GPUBufferType type);

        // Mapping VkFormat
        static VkFormat MapVertexFormat(PixelFormat pixelFormat);

        // Mapping Uniform type
        static ShaderParamType MapUniformType(const spirv_cross::SPIRType& spirType);
    };
}
