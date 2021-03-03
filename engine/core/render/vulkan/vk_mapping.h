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
        static VkPrimitiveTopology mapPrimitiveTopology(Mesh::TopologyType type);

        // mapping vertex semantic
        static String mapVertexSemanticString(VertexSemantic semantic);

        // mapping gpubffer type
        static VkBufferUsageFlags mapGpuBufferUsageFlags(GPUBuffer::GPUBufferType type);

        // mapping VkFormat
        static VkFormat mapVertexFormat(PixelFormat pixelFormat);

        // mapping Uniform type
        static ShaderParamType mapUniformType(const spirv_cross::SPIRType& spirType);

        // mapping blend factor
        static VkBlendFactor mapBlendFactor(BlendState::BlendFactor factor);

        // mapping blend operation
        static VkBlendOp mapBlendOperation(BlendState::BlendOperation op);

        // mapping pixel format
        static VkFormat mapPixelFormat(PixelFormat pixFmt);
    };
}
