#ifndef __ECHO_MATERIALDESC_H_
#define __ECHO_MATERIALDESC_H_

#include "String.h"

namespace Echo
{
	static const String s_ColorMask[6] = 
	{
		"CMASK_RED",
		"CMASK_GREEN",
		"CMASK_BLUE",
		"CMASK_ALPHA",
		"CMASK_COLOR",
		"CMASK_ALL",
	};

	static const String s_BlendOperation[5] =
	{
		"BOP_ADD", 
		"BOP_SUB", 
		"BOP_REV_SUB", 
		"BOP_MIN", 
		"BOP_MAX",
	};

	static const String s_BlendFactor[11] =
	{
		"BF_ZERO",
		"BF_ONE",
		"BF_SRC_ALPHA",
		"BF_DST_ALPHA",
		"BF_INV_SRC_ALPHA",
		"BF_INV_DST_ALPHA",
		"BF_SRC_COLOR",
		"BF_DST_COLOR",
		"BF_INV_SRC_COLOR",
		"BF_INV_DST_COLOR",
		"BF_SRC_ALPHA_SAT", 
	};

	static const String s_CullMode[3] =
	{
		"CULL_NONE", 
		"CULL_FRONT", 
		"CULL_BACK"
	};

	static const String s_ShadeModel[2] =
	{
		"SM_FLAT", 
		"SM_GOURAND"
	};

	static const String s_PolygonMode[3] =
	{
		"PM_POINT", 
		"PM_LINE", 
		"PM_FILL"
	};

	static const String s_StencilOperation[8] = 
	{
		"SOP_KEEP",
		"SOP_ZERO",
		"SOP_REPLACE",
		"SOP_INCR",
		"SOP_DECR",
		"SOP_INVERT",
		"SOP_INCR_WRAP",
		"SOP_DECR_WRAP"
	};

	static const String s_ComparisonFunc[8] =
	{
		"CF_NEVER",
		"CF_LESS",
		"CF_LESS_EQUAL",
		"CF_EQUAL",
		"CF_GREATER_EQUAL",
		"CF_GREATER",
		"CF_ALWAYS",
		"CF_NOT_EQUAL"
	};

	static const String s_AddressMode[4] = 
	{
		"AM_WRAP", 
		"AM_MIRROR", 
		"AM_CLAMP", 
		"AM_BORDER"
	};

	static const String s_FilterOption[4] =
	{
		"FO_NONE",
		"FO_POINT",
		"FO_LINEAR",
		"FO_ANISOTROPIC"
	};

	static const String s_VertexSemantic[9] =
	{
		"VS_POSITION", 
		"VS_NORMAL", 
		"VS_COLOR", 
		"VS_TEXCOORD", 
		"VS_TEXCOORD1",
		"VS_BLENDINDICES", 
		"VS_BLENDWEIGHTS", 
		"VS_TANGENT", 
		"VS_BINORMAL"
	};

	static const String s_PixelFormat[PF_COUNT] = 
	{
		"PF_UNKNOWN",	
		"PF_R8_UNORM", 
		"PF_R8_SNORM", 
		"PF_R8_UINT", 
		"PF_R8_SINT",
		"PF_A8_UNORM",
		"PF_RG8_UNORM", 
		"PF_RG8_SNORM", 
		"PF_RG8_UINT", 
		"PF_RG8_SINT", 
		"PF_RGB8_UNORM", 
		"PF_RGB8_SNORM", 
		"PF_RGB8_UINT", 
		"PF_RGB8_SINT", 
		"PF_BGR8_UNORM", 
		"PF_RGBA8_UNORM", 
		"PF_RGBA8_SNORM", 
		"PF_RGBA8_UINT", 
		"PF_RGBA8_SINT", 
		"PF_BGRA8_UNORM", 
		"PF_R16_UNORM", 
		"PF_R16_SNORM", 
		"PF_R16_UINT", 
		"PF_R16_SINT", 
		"PF_R16_FLOAT", 
		"PF_RG16_UNORM", 
		"PF_RG16_SNORM", 
		"PF_RG16_UINT", 
		"PF_RG16_SINT", 
		"PF_RG16_FLOAT", 
		"PF_RGB16_UNORM", 
		"PF_RGB16_SNORM", 
		"PF_RGB16_UINT", 
		"PF_RGB16_SINT", 
		"PF_RGB16_FLOAT", 
		"PF_RGBA16_UNORM", 
		"PF_RGBA16_SNORM", 
		"PF_RGBA16_UINT", 
		"PF_RGBA16_SINT", 
		"PF_RGBA16_FLOAT", 
		"PF_R32_UNORM", 
		"PF_R32_SNORM", 
		"PF_R32_UINT", 
		"PF_R32_SINT", 
		"PF_R32_FLOAT", 
		"PF_RG32_UNORM", 			
		"PF_RG32_SNORM", 			
		"PF_RG32_UINT", 			
		"PF_RG32_SINT", 			
		"PF_RG32_FLOAT", 
		"PF_RGB32_UNORM",			
		"PF_RGB32_SNORM",			
		"PF_RGB32_UINT",			
		"PF_RGB32_SINT",			
		"PF_RGB32_FLOAT", 
		"PF_RGBA32_UNORM", 		
		"PF_RGBA32_SNORM", 		
		"PF_RGBA32_UINT", 		
		"PF_RGBA32_SINT", 		
		"PF_RGBA32_FLOAT", 
		"PF_D16_UNORM", 
		"PF_D24_UNORM_S8_UINT", 
		"PF_D32_FLOAT", 
		"PF_BC1_UNORM", 
		"PF_BC1_UNORM_SRGB", 
		"PF_BC2_UNORM", 
		"PF_BC2_UNORM_SRGB", 
		"PF_BC3_UNORM", 
		"PF_BC3_UNORM_SRGB", 
		"PF_BC4_UNORM", 
		"PF_BC4_SNORM", 
		"PF_BC5_UNORM", 
		"PF_BC5_SNORM", 
		"PF_PVRTC1_2bpp_RGBA",
		"PF_PVRTC1_2bpp_RGB",
		"PF_PVRTC1_4bpp_RGBA",
		"PF_PVRTC1_4bpp_RGB",
		"PF_ETC1",
		"PF_PVRTC_RGBA_4444"
	};
}

#endif
