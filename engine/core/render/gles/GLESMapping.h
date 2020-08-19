#pragma once

#include <base/Texture.h>
#include <base/ShaderProgram.h>
#include "engine/core/render/base/GPUBuffer.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "GLESRenderState.h"

namespace Echo
{
	#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
	#endif

	#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
	#endif

	#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
	#endif

	#ifndef GL_ETC1_RGB8_OES
	#define GL_ETC1_RGB8_OES				0x8D64
	#endif

	#ifndef GL_ATC_RGB_AMD
	#define GL_ATC_RGB_AMD					0x8C92
	#endif

	#ifndef GL_ATC_RGBA_EXPLICIT_ALPHA_AMD
	#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD                          0x8C93
	#endif

	#ifndef GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD
	#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD                      0x87EE
	#endif

	#ifndef GL_COMPRESSED_RGB8_ETC2
	#define GL_COMPRESSED_RGB8_ETC2         0X9274
	#endif

	#ifndef GL_COMPRESSED_SRGB8_ETC2
	#define GL_COMPRESSED_SRGB8_ETC2        0x9275
	#endif

	#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
	#define GL_COMPRESSED_RGBA8_ETC2_EAC        0x9278
	#endif

	#ifndef GL_COMRRESSED_SRGB8_ALPHA8_ETC2_EAC
	#define GL_COMRRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
	#endif

	class GLES2Mapping
	{
	public:
		static GLenum			MapStencilOperation(DepthStencilState::StencilOperation op);
		static GLenum			MapBlendOperation(BlendState::BlendOperation op);
		static GLenum			MapBlendFactor(BlendState::BlendFactor factor);
		static GLint			MapAddressMode(SamplerState::AddressMode mode);

		static GLenum g_halfFloatDataType;
		static GLenum g_halfFloatInternalFormat;

		static inline GLES2Error MapErrorCode(GLenum err)
		{
			switch (err)
			{
			case GL_NO_ERROR:			return GLES2ERR_NO_ERROR;
			case GL_INVALID_ENUM:		return GLES2ERR_INVALID_ENUM;
			case GL_INVALID_VALUE:		return GLES2ERR_INVALID_VALUE;
			case GL_INVALID_OPERATION:	return GLES2ERR_INVALID_OPERATION;
			case GL_OUT_OF_MEMORY:		return GLES2ERR_OUT_OF_MEMORY;
			default:
				{
					EchoAssert(false);
					return GLES2ERR_NO_ERROR;
				}
			}
		}

		static inline GLenum MapGPUBufferUsage(Dword usage)
		{
			if(	(usage & GPUBuffer::GBU_GPU_READ) &&
				!(usage & GPUBuffer::GBU_CPU_READ) && !(usage & GPUBuffer::GBU_CPU_WRITE))
				return GL_STATIC_DRAW;
			else
				return GL_DYNAMIC_DRAW;
		}

		static inline GLenum MapDataType(PixelFormat pixFmt)
		{
			switch(pixFmt)
			{
			case PF_R8_UNORM:			return GL_UNSIGNED_BYTE;
			case PF_R8_SNORM:			return GL_BYTE;
			case PF_R8_UINT:			return GL_UNSIGNED_BYTE;
			case PF_R8_SINT:			return GL_BYTE;

			case PF_A8_UNORM:			return GL_UNSIGNED_BYTE;

			case PF_RG8_UNORM:			return GL_UNSIGNED_BYTE;
			case PF_RG8_SNORM:			return GL_BYTE;
			case PF_RG8_UINT:			return GL_UNSIGNED_BYTE;
			case PF_RG8_SINT:			return GL_BYTE;

			case PF_RGB8_UNORM:			return GL_UNSIGNED_BYTE;
			case PF_RGB8_SNORM:			return GL_BYTE;
			case PF_RGB8_UINT:			return GL_UNSIGNED_BYTE;
			case PF_RGB8_SINT:			return GL_BYTE;

			case PF_BGR8_UNORM:			return GL_UNSIGNED_BYTE;

			case PF_RGBA8_UNORM:		return GL_UNSIGNED_BYTE;
			case PF_RGBA8_SNORM:		return GL_BYTE;
			case PF_RGBA8_UINT:			return GL_UNSIGNED_BYTE;
			case PF_RGBA8_SINT:			return GL_BYTE;

			case PF_BGRA8_UNORM:		return GL_UNSIGNED_BYTE;

			case PF_R16_UNORM:			return GL_UNSIGNED_SHORT;
			case PF_R16_SNORM:			return GL_SHORT;
			case PF_R16_UINT:			return GL_UNSIGNED_SHORT;
			case PF_R16_SINT:			return GL_SHORT;

			case PF_RG16_UNORM:			return GL_UNSIGNED_SHORT;
			case PF_RG16_SNORM:			return GL_SHORT;
			case PF_RG16_UINT:			return GL_UNSIGNED_SHORT;
			case PF_RG16_SINT:			return GL_SHORT;

			case PF_RGB16_UNORM:		return GL_UNSIGNED_SHORT;
			case PF_RGB16_SNORM:		return GL_SHORT;
			case PF_RGB16_UINT:			return GL_UNSIGNED_SHORT;
			case PF_RGB16_SINT:			return GL_SHORT;

			case PF_RGBA16_UNORM:		return GL_UNSIGNED_SHORT;
			case PF_RGBA16_SNORM:		return GL_SHORT;
			case PF_RGBA16_UINT:		return GL_UNSIGNED_SHORT;
			case PF_RGBA16_SINT:		return GL_SHORT;
			case PF_R32_UNORM:			return GL_UNSIGNED_INT;
			case PF_R32_SNORM:			return GL_INT;
			case PF_R32_UINT:			return GL_UNSIGNED_INT;
			case PF_R32_SINT:			return GL_INT;
			case PF_R32_FLOAT:			return GL_FLOAT;

			case PF_RG32_UNORM:			return GL_UNSIGNED_INT;
			case PF_RG32_SNORM:			return GL_INT;
			case PF_RG32_UINT:			return GL_UNSIGNED_INT;
			case PF_RG32_SINT:			return GL_INT;
			case PF_RG32_FLOAT:			return GL_FLOAT;

			case PF_RGB32_UNORM:		return GL_UNSIGNED_INT;
			case PF_RGB32_SNORM:		return GL_INT;
			case PF_RGB32_UINT:			return GL_UNSIGNED_INT;
			case PF_RGB32_SINT:			return GL_INT;
			case PF_RGB32_FLOAT:		return GL_FLOAT;

			case PF_RGBA32_UNORM:		return GL_UNSIGNED_INT;
			case PF_RGBA32_SNORM:		return GL_INT;
			case PF_RGBA32_UINT:		return GL_UNSIGNED_INT;
			case PF_RGBA32_SINT:		return GL_INT;
			case PF_RGBA32_FLOAT:		return GL_FLOAT;

			case PF_D16_UNORM:			return GL_UNSIGNED_SHORT;
			case PF_D32_FLOAT:			return GL_FLOAT;
			default:
				{
					EchoAssertX("Unsupported pixel format [%s].", PixelUtil::GetPixelFormatName(pixFmt).c_str());
					return GL_BYTE;
				}
			}
		}

		static inline GLenum MapInternalFormat(PixelFormat pixFmt)
		{
			switch(pixFmt)
			{
			case PF_R8_UNORM:				return GL_LUMINANCE;
			case PF_R8_SNORM:				return GL_LUMINANCE;
			case PF_R8_UINT:				return GL_LUMINANCE;
			case PF_R8_SINT:				return GL_LUMINANCE;

			case PF_A8_UNORM:				return GL_ALPHA;

			case PF_RG8_UNORM:				return GL_LUMINANCE_ALPHA;
			case PF_RG8_SNORM:				return GL_LUMINANCE_ALPHA;
			case PF_RG8_UINT:				return GL_LUMINANCE_ALPHA;
			case PF_RG8_SINT:				return GL_LUMINANCE_ALPHA;

			case PF_RGB8_UNORM:				return GL_RGB;
			case PF_RGB8_SNORM:				return GL_RGB;
			case PF_RGB8_UINT:				return GL_RGB;
			case PF_RGB8_SINT:				return GL_RGB;

			case PF_BGR8_UNORM:				return GL_RGB;

			case PF_RGBA8_UNORM:			return GL_RGBA;
			case PF_RGBA8_SNORM:			return GL_RGBA;
			case PF_RGBA8_UINT:				return GL_RGBA;
			case PF_RGBA8_SINT:				return GL_RGBA;

				//case PF_R16_UNORM:
				//case PF_R16_SNORM:
				//case PF_R16_UINT:
				//case PF_R16_SINT:
			case PF_R16_FLOAT:				return GL_LUMINANCE;

				//case PF_RG16_UNORM:
				//case PF_RG16_SNORM:
				//case PF_RG16_UINT:
				//case PF_RG16_SINT:
			case PF_RG16_FLOAT:				return GL_LUMINANCE_ALPHA;

				//case PF_RGB16_UNORM:
				//case PF_RGB16_SNORM:
				//case PF_RGB16_UINT:
				//case PF_RGB16_SINT:
			case PF_RGB16_FLOAT:			return GL_RGB;

				//case PF_RGBA16_UNORM:
				//case PF_RGBA16_SNORM:
				//case PF_RGBA16_UINT:
				//case PF_RGBA16_SINT:

			case PF_RGBA16_FLOAT:			return g_halfFloatInternalFormat;

			case PF_PVRTC_RGBA_4444:
			case PF_RGBA32_FLOAT:
				return GL_RGBA;

				//case PF_R32_UNORM:
				//case PF_R32_SNORM:
				//case PF_R32_UINT:
				//case PF_R32_SINT:
			case PF_R32_FLOAT:				return GL_LUMINANCE;

				//case PF_RG32_UNORM:
				//case PF_RG32_SNORM:
				//case PF_RG32_UINT:
				//case PF_RG32_SINT:
			case PF_RG32_FLOAT:				return GL_LUMINANCE_ALPHA;

				//case PF_RGB32_UNORM:
				//case PF_RGB32_SNORM:
				//case PF_RGB32_UINT:
				//case PF_RGB32_SINT:
			case PF_RGB32_FLOAT:			return GL_RGB;

				//case PF_RGBA32_UNORM:
				//case PF_RGBA32_SNORM:
				//case PF_RGBA32_UINT:
				//case PF_RGBA32_SINT:

			case PF_D16_UNORM:				return GL_DEPTH_COMPONENT16;
			case PF_ETC1:					return GL_ETC1_RGB8_OES;
			case PF_ETC2_RGB:				return GL_COMPRESSED_RGB8_ETC2;
			case PF_ETC2_RGBA:				return GL_COMPRESSED_RGBA8_ETC2_EAC;
			default:
				{
					EchoAssertX("Unsupported pixel format [%s].", PixelUtil::GetPixelFormatName(pixFmt).c_str());
					return GL_BYTE;
				}
			}
		}

		static inline GLenum MapFormat(PixelFormat pixFmt)
		{
			switch(pixFmt)
			{
			case PF_R8_UNORM:				return GL_LUMINANCE;
			case PF_R8_SNORM:				return GL_LUMINANCE;
			case PF_R8_UINT:				return GL_LUMINANCE;
			case PF_R8_SINT:				return GL_LUMINANCE;

			case PF_A8_UNORM:				return GL_ALPHA;

			case PF_RG8_UNORM:				return GL_LUMINANCE_ALPHA;
			case PF_RG8_SNORM:				return GL_LUMINANCE_ALPHA;
			case PF_RG8_UINT:				return GL_LUMINANCE_ALPHA;
			case PF_RG8_SINT:				return GL_LUMINANCE_ALPHA;

			case PF_RGB8_UNORM:				return GL_RGB;
			case PF_RGB8_SNORM:				return GL_RGB;
			case PF_RGB8_UINT:				return GL_RGB;
			case PF_RGB8_SINT:				return GL_RGB;

			case PF_BGR8_UNORM:				return GL_RGB;

			case PF_RGBA8_UNORM:			return GL_RGBA;
			case PF_RGBA8_SNORM:			return GL_RGBA;
			case PF_RGBA8_UINT:				return GL_RGBA;
			case PF_RGBA8_SINT:				return GL_RGBA;

				//case PF_R16_UNORM:
				//case PF_R16_SNORM:
				//case PF_R16_UINT:
				//case PF_R16_SINT:
			case PF_R16_FLOAT:				return GL_LUMINANCE;

				//case PF_RG16_UNORM:
				//case PF_RG16_SNORM:
				//case PF_RG16_UINT:
				//case PF_RG16_SINT:
			case PF_RG16_FLOAT:				return GL_LUMINANCE_ALPHA;

				//case PF_RGB16_UNORM:
				//case PF_RGB16_SNORM:
				//case PF_RGB16_UINT:
				//case PF_RGB16_SINT:
			case PF_RGB16_FLOAT:			return GL_RGB;

				//case PF_RGBA16_UNORM:
				//case PF_RGBA16_SNORM:
				//case PF_RGBA16_UINT:
				//case PF_RGBA16_SINT:
			case PF_PVRTC_RGBA_4444:
			case PF_RGBA16_FLOAT:			return GL_RGBA;

				//case PF_R32_UNORM:
				//case PF_R32_SNORM:
				//case PF_R32_UINT:
				//case PF_R32_SINT:
#ifdef ECHO_PLATFORM_WINDOWS
			// According to OpenGLES3 spec, `GL_LUMINANCE` is not renderable
			// angle(QtANGLE) implement in file: %QTDIR%/src/3rdparty/angle/src/libGLESv2/formatutils.cpp
			case PF_R32_FLOAT:				return GL_RED;
#else
			case PF_R32_FLOAT:				return GL_LUMINANCE;
#endif
				//case PF_RG32_UNORM:
				//case PF_RG32_SNORM:
				//case PF_RG32_UINT:
				//case PF_RG32_SINT:
			case PF_RG32_FLOAT:				return GL_LUMINANCE_ALPHA;

				//case PF_RGB32_UNORM:
				//case PF_RGB32_SNORM:
				//case PF_RGB32_UINT:
				//case PF_RGB32_SINT:
			case PF_RGB32_FLOAT:			return GL_RGB;

				//case PF_RGBA32_UNORM:
				//case PF_RGBA32_SNORM:
				//case PF_RGBA32_UINT:
				//case PF_RGBA32_SINT:
			case PF_RGBA32_FLOAT:			return GL_RGBA;

			case PF_D16_UNORM:				return GL_DEPTH_COMPONENT;
			case PF_D32_FLOAT:				return GL_DEPTH_COMPONENT;
			default:
				{
					EchoAssertX("Unsupported pixel format [%s].", PixelUtil::GetPixelFormatName(pixFmt).c_str());
					return GL_BYTE;
				}
			}
		}

		static inline GLenum MapPrimitiveTopology(Mesh::TopologyType type)
		{
			switch(type)
			{
			case Mesh::TT_POINTLIST:		return GL_POINTS;
			case Mesh::TT_LINELIST:			return GL_LINES;
			case Mesh::TT_LINESTRIP:		return GL_LINE_STRIP;
			case Mesh::TT_TRIANGLELIST:		return GL_TRIANGLES;
			case Mesh::TT_TRIANGLESTRIP:	return GL_TRIANGLE_STRIP;
			default:						return GL_TRIANGLES;
			}
		}

		static inline GLenum MapComparisonFunc(RenderState::ComparisonFunc func)
		{
			switch(func)
			{
			case RenderState::CF_NEVER:			return GL_NEVER;
			case RenderState::CF_ALWAYS:		return GL_ALWAYS;
			case RenderState::CF_LESS:			return GL_LESS;
			case RenderState::CF_LESS_EQUAL:	return GL_LEQUAL;
			case RenderState::CF_EQUAL:			return GL_EQUAL;
			case RenderState::CF_NOT_EQUAL:		return GL_NOTEQUAL;
			case RenderState::CF_GREATER_EQUAL:	return GL_GEQUAL;
			case RenderState::CF_GREATER:		return GL_GREATER;
			default:
				{
					EchoAssert(false)
						return GL_EQUAL;
				}
			}
		}

		static inline GLenum MapTextureType(Texture::TexType type)
		{
			switch(type)
			{
			case Texture::TT_2D:			return GL_TEXTURE_2D;
			case Texture::TT_Cube:			return GL_TEXTURE_CUBE_MAP;
			default:
				{
					EchoAssert(false);
					return GL_TEXTURE_2D;
				}
			}
		}

		static inline String MapVertexSemanticString(VertexSemantic semantic)
		{
			switch(semantic)
			{
			case VS_POSITION:			return "a_Position";
			case VS_BLENDINDICES:		return "a_Joint";
			case VS_BLENDWEIGHTS:		return "a_Weight";
			case VS_COLOR:				return "a_Color";
			case VS_NORMAL:				return "a_Normal";
			case VS_TEXCOORD0:			return "a_UV";
			case VS_TEXCOORD1:			return "a_UV1";
			case VS_TANGENT:			return "a_Tangent";
			case VS_BINORMAL:			return "a_Binormal";
            default:                    return "";
			}
		}

		INLINE static ShaderParamType MapUniformType( GLenum uniformType)
		{
			switch( uniformType)
			{
			case GL_FLOAT:				return SPT_FLOAT;
			case GL_FLOAT_VEC2:         return SPT_VEC2;
			case GL_FLOAT_VEC3:         return SPT_VEC3;
			case GL_FLOAT_VEC4:         return SPT_VEC4;
			case GL_INT:                return SPT_INT;
			case GL_FLOAT_MAT4:         return SPT_MAT4;
			case GL_SAMPLER_2D:         return SPT_TEXTURE;
			case GL_SAMPLER_CUBE:       return SPT_TEXTURE;
			default:					return SPT_UNKNOWN;
			}
		}
	};
}
