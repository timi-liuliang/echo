#pragma once

#include "base/mesh/mesh.h"
#include "base/shader_program.h"
#include "base/gpu_buffer.h"
#include "vk_render_base.h"

namespace Echo
{
    class VKMapping
    {
    public:
        // mapping primitive topology type
        static VkPrimitiveTopology MapPrimitiveTopology(Mesh::TopologyType type);

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
