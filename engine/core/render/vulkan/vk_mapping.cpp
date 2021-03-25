#include "vk_mapping.h"

namespace Echo
{
    VkPrimitiveTopology VKMapping::mapPrimitiveTopology(Mesh::TopologyType type)
    {
        switch (type)
        {
        case Mesh::TT_POINTLIST:        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case Mesh::TT_LINELIST:         return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case Mesh::TT_LINESTRIP:        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case Mesh::TT_TRIANGLELIST:     return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case Mesh::TT_TRIANGLESTRIP:    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        default:                           return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    String VKMapping::mapVertexSemanticString(VertexSemantic semantic)
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

    VkBufferUsageFlags VKMapping::mapGpuBufferUsageFlags(GPUBuffer::GPUBufferType type)
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

    VkFormat VKMapping::mapVertexFormat(PixelFormat pixelFormat)
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

    VkFormat VKMapping::mapPixelFormat(PixelFormat pixFmt)
    {
		switch (pixFmt)
		{
        case PF_UNKNOWN:            return VK_FORMAT_UNDEFINED;
        case PF_R8_UINT:            return VK_FORMAT_R8_UINT;
        case PF_R16_UINT:           return VK_FORMAT_R16_UINT;
        case PF_RGB8_UNORM:         return VK_FORMAT_R8G8B8_UNORM;
        case PF_RGB8_SNORM:         return VK_FORMAT_R8G8B8_SNORM;
        case PF_RGBA8_UNORM:        return VK_FORMAT_R8G8B8A8_UNORM;
        case PF_RGBA8_SNORM:        return VK_FORMAT_R8G8B8A8_SNORM;
        case PF_RGBA8_UINT:         return VK_FORMAT_R8G8B8A8_UINT;
        case PF_RGBA8_SINT:         return VK_FORMAT_R8G8B8A8_SINT;
        case PF_BGRA8_UNORM:        return VK_FORMAT_B8G8R8A8_UNORM;
		case PF_D16_UNORM:          return VK_FORMAT_D16_UNORM;
        case PF_D24_UNORM_S8_UINT:  return VK_FORMAT_D24_UNORM_S8_UINT;
        case PF_D32_FLOAT:          return VK_FORMAT_D32_SFLOAT;
		default:  EchoLogError("MapingVertexFormat failed");  return VK_FORMAT_UNDEFINED;
		}
    }

    ShaderParamType VKMapping::mapUniformType(const spirv_cross::SPIRType& spirType)
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
        case spirv_cross::SPIRType::Image:
        case spirv_cross::SPIRType::SampledImage:
        case spirv_cross::SPIRType::Sampler:    return SPT_TEXTURE;
        default:  EchoLogError("vulkan MapUniformType failed"); return SPT_UNKNOWN;
        }
    }

    VkBlendFactor VKMapping::mapBlendFactor(BlendState::BlendFactor factor)
	{
		switch (factor)
		{
		case BlendState::BF_ZERO:			return VK_BLEND_FACTOR_ZERO;
		case BlendState::BF_ONE:			return VK_BLEND_FACTOR_ONE;
		case BlendState::BF_SRC_ALPHA:		return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendState::BF_DST_ALPHA:		return VK_BLEND_FACTOR_DST_ALPHA;
		case BlendState::BF_INV_SRC_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BlendState::BF_INV_DST_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BlendState::BF_SRC_COLOR:		return VK_BLEND_FACTOR_SRC_COLOR;
		case BlendState::BF_DST_COLOR:		return VK_BLEND_FACTOR_DST_COLOR;
		case BlendState::BF_INV_SRC_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BlendState::BF_INV_DST_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BlendState::BF_SRC_ALPHA_SAT:	return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		default:
		{
			EchoAssert(false);
			return VK_BLEND_FACTOR_ZERO;
		}
		}
	}

    VkBlendOp VKMapping::mapBlendOperation(BlendState::BlendOperation op)
	{
		switch (op)
		{
		case BlendState::BOP_ADD:			return VK_BLEND_OP_ADD;
		case BlendState::BOP_SUB:			return VK_BLEND_OP_SUBTRACT;
		case BlendState::BOP_REV_SUB:		return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendState::BOP_MIN:			return VK_BLEND_OP_MIN;
		case BlendState::BOP_MAX:			return VK_BLEND_OP_MAX;
		default:
		{
			EchoAssert(false);
			return VK_BLEND_OP_ADD;
		}
		}
	}

    VkCompareOp VKMapping::mapCompareOperation(RenderState::ComparisonFunc op)
    {
		switch (op)
		{
		case RenderState::CF_NEVER:		    return VK_COMPARE_OP_NEVER;
        case RenderState::CF_LESS:		    return VK_COMPARE_OP_LESS;
        case RenderState::CF_LESS_EQUAL:	return VK_COMPARE_OP_LESS_OR_EQUAL;
        case RenderState::CF_EQUAL:		    return VK_COMPARE_OP_EQUAL;
        case RenderState::CF_GREATER_EQUAL:	return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case RenderState::CF_GREATER:		return VK_COMPARE_OP_GREATER;
        case RenderState::CF_ALWAYS:		return VK_COMPARE_OP_ALWAYS;
        case RenderState::CF_NOT_EQUAL:		return VK_COMPARE_OP_NOT_EQUAL;
		default:
		{
			EchoAssert(false);
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		}
		}
    }

    VkPolygonMode VKMapping::mapPolygonMode(RasterizerState::PolygonMode mode)
    {
        switch (mode)
        {
        case RasterizerState::PM_POINT:     return VK_POLYGON_MODE_POINT;
        case RasterizerState::PM_LINE:      return VK_POLYGON_MODE_LINE;
        case RasterizerState::PM_FILL:      return VK_POLYGON_MODE_FILL;
        default: 
        {
            EchoLogError("Vulkan mapPolygonMode failed");
            return VK_POLYGON_MODE_FILL; 
        }
        }
    }

    VkCullModeFlagBits VKMapping::mapCullMode(RasterizerState::CullMode cullMode)
    {
        switch (cullMode)
        {
        case RasterizerState::CULL_NONE:    return VK_CULL_MODE_NONE;
        case RasterizerState::CULL_FRONT:   return VK_CULL_MODE_FRONT_BIT;
        case RasterizerState::CULL_BACK:    return VK_CULL_MODE_BACK_BIT;
        default:
        {
            EchoAssert(false);
            return VK_CULL_MODE_NONE;
        }
        }
    }
}