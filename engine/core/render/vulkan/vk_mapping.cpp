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

    String VKMapping::MapVertexSemanticString(VertexSemantic semantic)
    {
        switch (semantic)
        {
        case VS_POSITION:            return "a_Position";
        case VS_BLENDINDICES:        return "a_Joint";
        case VS_BLENDWEIGHTS:        return "a_Weight";
        case VS_COLOR:               return "a_Color";
        case VS_NORMAL:              return "a_Normal";
        case VS_TEXCOORD0:           return "a_UV";
        case VS_TEXCOORD1:           return "a_UV1";
        case VS_TANGENT:             return "a_Tangent";
        case VS_BINORMAL:            return "a_Binormal";
        default:                     return "";
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

    VkFormat VKMapping::MapVertexFormat(PixelFormat pixelFormat)
    {
        switch (pixelFormat)
        {
        case PF_RG32_FLOAT:     return VK_FORMAT_R32G32_SFLOAT;
        case PF_RGBA8_UNORM:    return VK_FORMAT_R8G8B8A8_UNORM;
        case PF_RGBA8_SNORM:    return VK_FORMAT_R8G8B8A8_SNORM;
        case PF_RGB32_FLOAT:    return VK_FORMAT_R32G32B32_SFLOAT;
        case PF_RGBA32_FLOAT:   return VK_FORMAT_R32G32B32A32_SFLOAT;
        default:  EchoLogError("MapingVertexFormat failed");  return VK_FORMAT_UNDEFINED;
        }
    }

    ShaderParamType VKMapping::MapUniformType(const spirv_cross::SPIRType& spirType)
    {
        switch (spirType.basetype)
        {
        case spirv_cross::SPIRType::BaseType::Int:             return SPT_INT;
        case spirv_cross::SPIRType::BaseType::Float:
        {
            if (spirType.columns == 1)
            {
                if      (spirType.vecsize == 1) return SPT_FLOAT;
                else if (spirType.vecsize == 2) return SPT_VEC2;
                else if (spirType.vecsize == 3) return SPT_VEC3;
                else if (spirType.vecsize == 4) return SPT_VEC4;
            }
            else if (spirType.columns == 4)
            {
                if      (spirType.vecsize == 4) return SPT_MAT4;
            }

            EchoLogError("vulkan MapUniformType failed"); return SPT_UNKNOWN;
        }
        case spirv_cross::SPIRType::Image:      return SPT_TEXTURE;
        case spirv_cross::SPIRType::Sampler:    return SPT_TEXTURE;
        default:  EchoLogError("vulkan MapUniformType failed"); return SPT_UNKNOWN;
        }
    }
}