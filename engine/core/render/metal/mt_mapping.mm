#include "mt_mapping.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    MTLPrimitiveType MTMapping::MapPrimitiveTopology(Mesh::TopologyType type)
    {
        switch(type)
        {
            case Mesh::TT_POINTLIST:        return MTLPrimitiveTypePoint;
            case Mesh::TT_LINELIST:         return MTLPrimitiveTypeLine;
            case Mesh::TT_LINESTRIP:        return MTLPrimitiveTypeLineStrip;
            case Mesh::TT_TRIANGLELIST:     return MTLPrimitiveTypeTriangle;
            case Mesh::TT_TRIANGLESTRIP:    return MTLPrimitiveTypeTriangleStrip;
            default:                        return MTLPrimitiveTypeTriangle;
        }
    }
    
    String MTMapping::MapVertexSemanticString(VertexSemantic semantic)
    {
        switch(semantic)
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

    MTLVertexFormat MTMapping::MapVertexFormat(PixelFormat pixelFormat)
    {
        switch (pixelFormat)
        {
            case PF_RG32_FLOAT:     return MTLVertexFormatFloat2;
            case PF_RGBA8_UNORM:    return MTLVertexFormatUChar4Normalized;
            case PF_RGBA8_SNORM:    return MTLVertexFormatChar4Normalized;
            case PF_RGB32_FLOAT:    return MTLVertexFormatFloat3;
            case PF_RGBA32_FLOAT:   return MTLVertexFormatFloat4;
            default:  EchoLogError("MapingVertexFormat failed");  return MTLVertexFormatInvalid;
        }
    }

    MTLPixelFormat MTMapping::MapPixelFormat(PixelFormat pixFmt)
    {
        switch(pixFmt)
        {
        case PF_R8_UNORM:               return MTLPixelFormatR8Unorm;
        case PF_R8_SNORM:               return MTLPixelFormatR8Snorm;
        case PF_R8_UINT:                return MTLPixelFormatR8Uint;
        case PF_R8_SINT:                return MTLPixelFormatR8Sint;

        case PF_A8_UNORM:               return MTLPixelFormatA8Unorm;

        case PF_RG8_UNORM:              return MTLPixelFormatRG8Unorm;
        case PF_RG8_SNORM:              return MTLPixelFormatRG8Snorm;
        case PF_RG8_UINT:               return MTLPixelFormatRG8Uint;
        case PF_RG8_SINT:               return MTLPixelFormatRG8Sint;
                
//        case PF_RGB8_UNORM:             return MTLPixelFormatRGB8Unorm;
//        case PF_RGB8_SNORM:             return MTLPixelFormatRGB8Snorm;
//        case PF_RGB8_UINT:              return MTLPixelFormatRGB8Uint;
//        case PF_RGB8_SINT:              return MTLPixelFormatRGB8Sint;

        case PF_RGBA8_UNORM:            return MTLPixelFormatRGBA8Unorm;
        case PF_RGBA8_SNORM:            return MTLPixelFormatRGBA8Snorm;
        case PF_RGBA8_UINT:             return MTLPixelFormatRGBA8Uint;
        case PF_RGBA8_SINT:             return MTLPixelFormatRGBA8Sint;

            //case PF_R16_UNORM:
            //case PF_R16_SNORM:
            //case PF_R16_UINT:
            //case PF_R16_SINT:
        case PF_R16_FLOAT:              return MTLPixelFormatR16Float;

            //case PF_RG16_UNORM:
            //case PF_RG16_SNORM:
            //case PF_RG16_UINT:
            //case PF_RG16_SINT:
        case PF_RG16_FLOAT:             return MTLPixelFormatRG16Float;


            //case PF_RGBA16_UNORM:
            //case PF_RGBA16_SNORM:
            //case PF_RGBA16_UINT:
            //case PF_RGBA16_SINT:
        case PF_PVRTC_RGBA_4444:
        case PF_RGBA16_FLOAT:           return MTLPixelFormatRGBA16Float;

            //case PF_R32_UNORM:
            //case PF_R32_SNORM:
            //case PF_R32_UINT:
            //case PF_R32_SINT:
        case PF_R32_FLOAT:              return MTLPixelFormatR32Float;

            //case PF_RG32_UNORM:
            //case PF_RG32_SNORM:
            //case PF_RG32_UINT:
            //case PF_RG32_SINT:
        case PF_RG32_FLOAT:             return MTLPixelFormatRG32Float;

            //case PF_RGBA32_UNORM:
            //case PF_RGBA32_SNORM:
            //case PF_RGBA32_UINT:
            //case PF_RGBA32_SINT:
        case PF_RGBA32_FLOAT:           return MTLPixelFormatRGBA32Float;

        case PF_D16_UNORM:              return MTLPixelFormatDepth16Unorm;
        case PF_D32_FLOAT:              return MTLPixelFormatDepth32Float;
        default:
            {
                EchoAssertX("Unsupported pixel format [%s].", PixelUtil::GetPixelFormatName(pixFmt).c_str());
                return MTLPixelFormatInvalid;
            }
        }
    }

    ShaderParamType MTMapping::MapUniformType(MTLDataType uniformType)
    {
        switch( uniformType)
        {
            case MTLDataTypeFloat:          return SPT_FLOAT;
            case MTLDataTypeFloat2:         return SPT_VEC2;
            case MTLDataTypeFloat3:         return SPT_VEC3;
            case MTLDataTypeFloat4:         return SPT_VEC4;
            case MTLDataTypeInt:            return SPT_INT;
            case MTLDataTypeFloat4x4:       return SPT_MAT4;
            case MTLDataTypeTexture:        return SPT_TEXTURE;
            default:                        return SPT_UNKNOWN;
        }
    }
}
