#pragma once

#include "interface/mesh/Mesh.h"
#include "interface/ShaderProgram.h"
#include "engine/core/render/interface/GPUBuffer.h"
#include "vk_render_base.h"

namespace Echo
{
    class VKMapping
    {
    public:
        // mapping primitive topology type
        static VkPrimitiveTopology MapPrimitiveTopology(Mesh::TopologyType type);

        // mapping vertex semantic
        static String VKMapping::MapVertexSemanticString(VertexSemantic semantic);

        // mapping gpubffer type
        static VkBufferUsageFlags MapGpuBufferUsageFlags(GPUBuffer::GPUBufferType type);

        // Mapping VkFormat
        static VkFormat MapVertexFormat(PixelFormat pixelFormat);

        // Mapping Uniform type
        static ShaderParamType MapUniformType(const spirv_cross::SPIRType& spirType);
    };
}
