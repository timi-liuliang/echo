#include "pixel_format.h"
#include "pixel_conversions.h"
#include "engine/core/util/AssertX.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	/** Pixel format database */
	PixelFormatDesc g_pixFmtDescArr[PF_COUNT] =
	{
		//-----------------------------------------------------------------------
		{
			"PF_UNKNOWN", 
			/* Flags */
			0, 
			/* Channel count */
			0, 
			/* Channel format */
			PCF_NONE, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_NONE, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			0, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0, 
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_R8_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_LUMINANCE | PFF_NATIVEENDIAN,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0xFF, 0, 0, 0, 
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_R8_SNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_LUMINANCE | PFF_NATIVEENDIAN, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0xFF, 0, 0, 0, 
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_R8_UINT", 
			/* Flags */
			PFF_LUMINANCE | PFF_NATIVEENDIAN, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UINT, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0xFF, 0, 0, 0, 
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_R8_SINT", 
			/* Flags */
			PFF_LUMINANCE | PFF_NATIVEENDIAN, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SINT, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0xFF, 0, 0, 0, 
			0, 0, 0, 0
		},
		//-----------------------------------------------------------------------
		{
			"PF_A8_UNORM",
			/* Flags */
			PFF_NORMALIZED | PFF_ALPHA | PFF_NATIVEENDIAN,
			/* Channel count */
			1,
			/* Channel format */
			PCF_NONE, PCF_NONE, PCF_NONE, PCF_A,
			/* Channel type */
			PCT_NONE, PCT_NONE, PCT_NONE, PCT_UNORM,
			/* Channel bits */
			0, 0, 0, 8,
			/* Masks and shifts */
			0, 0, 0, 0xFF,
			0, 0, 0, 0
		},
		//-----------------------------------------------------------------------
		{
			"PF_RG8_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_NATIVEENDIAN, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 8, 0, 0, 
			/* Masks and shifts */
			0xFF00, 0x00FF, 0, 0, 
			8, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RG8_SNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_NATIVEENDIAN, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SNORM, PCT_SNORM, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 8, 0, 0, 
			/* Masks and shifts */
			0xFF00, 0x00FF, 0, 0, 
			8, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RG8_UINT", 
			/* Flags */
			PFF_NATIVEENDIAN, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UINT, PCT_UINT, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 8, 0, 0, 
			/* Masks and shifts */
			0xFF00, 0x00FF, 0, 0, 
			8, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RG8_SINT", 
			/* Flags */
			PFF_NATIVEENDIAN, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SINT, PCT_SINT, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 8, 0, 0, 
			/* Masks and shifts */
			0xFF00, 0x00FF, 0, 0, 
			8, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGB8_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_NATIVEENDIAN, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_UNORM, PCT_NONE, 
			/* Channel bits */
			8, 8, 8, 0, 
			/* Masks and shifts */
			0xFF0000, 0x00FF00, 0x0000FF, 0, 
			16, 8, 0, 0
		}, 
	//-----------------------------------------------------------------------
		{
			"PF_RGB8_SNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_NATIVEENDIAN, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_SNORM, PCT_SNORM, PCT_SNORM, PCT_NONE, 
			/* Channel bits */
			8, 8, 8, 0, 
			/* Masks and shifts */
			0xFF0000, 0x00FF00, 0x0000FF, 0, 
			16, 8, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGB8_UINT", 
			/* Flags */
			PFF_NATIVEENDIAN, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_UINT, PCT_UINT, PCT_UINT, PCT_NONE, 
			/* Channel bits */
			8, 8, 8, 0, 
			/* Masks and shifts */
			0xFF0000, 0x00FF00, 0x0000FF, 0, 
			16, 8, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGB8_SINT", 
			/* Flags */
			PFF_NATIVEENDIAN, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_SINT, PCT_SINT, PCT_SINT, PCT_NONE, 
			/* Channel bits */
			8, 8, 8, 0, 
			/* Masks and shifts */
			0xFF0000, 0x00FF00, 0x0000FF, 0, 
			16, 8, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_BGR8_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_NATIVEENDIAN, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_UNORM, PCT_NONE, 
			/* Channel bits */
			8, 8, 8, 0, 
			/* Masks and shifts */
			0x0000FF, 0x00FF00, 0xFF0000, 0, 
			0, 8, 16, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGBA8_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_ALPHA | PFF_NATIVEENDIAN, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_UNORM, PCT_UNORM, 
			/* Channel bits */
			8, 8, 8, 8, 
			/* Masks and shifts */
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, 
			24, 16, 8, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGBA8_SNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_ALPHA | PFF_NATIVEENDIAN, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_SNORM, PCT_SNORM, PCT_SNORM, PCT_SNORM, 
			/* Channel bits */
			8, 8, 8, 8, 
			/* Masks and shifts */
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, 
			24, 16, 8, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGBA8_UINT", 
			/* Flags */
			PFF_ALPHA | PFF_NATIVEENDIAN, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_UINT, PCT_UINT, PCT_UINT, PCT_UINT, 
			/* Channel bits */
			8, 8, 8, 8, 
			/* Masks and shifts */
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, 
			24, 16, 8, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGBA8_SINT", 
			/* Flags */
			PFF_ALPHA | PFF_NATIVEENDIAN, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_SINT, PCT_SINT, PCT_SINT, PCT_SINT, 
			/* Channel bits */
			8, 8, 8, 8, 
			/* Masks and shifts */
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, 
			24, 16, 8, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_BGRA8_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_ALPHA | PFF_NATIVEENDIAN, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_UNORM, PCT_UNORM, 
			/* Channel bits */
			8, 8, 8, 8, 
			/* Masks and shifts */
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
			16, 8, 0, 24,
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_R16_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_LUMINANCE, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 0, 0, 0, 
			/* Masks and shifts */
			0xFFFF, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_R16_SNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_LUMINANCE, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 0, 0, 0, 
			/* Masks and shifts */
			0xFFFF, 0, 0, 0,
			0, 0, 0, 0
		},
		//-----------------------------------------------------------------------
		{
			"PF_R16_UINT",
			/* Flags */
			PFF_LUMINANCE,
			/* Channel count */
			1,
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE,
			/* Channel type */
			PCT_UINT, PCT_NONE, PCT_NONE, PCT_NONE,
			/* Channel bits */
			16, 0, 0, 0,
			/* Masks and shifts */
			0xFFFF, 0, 0, 0,
			0, 0, 0, 0
		},
		//-----------------------------------------------------------------------
		{
			"PF_R16_SINT", 
			/* Flags */
			PFF_LUMINANCE, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SINT, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 0, 0, 0, 
			/* Masks and shifts */
			0xFFFF, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_R16_FLOAT", 
			/* Flags */
			PFF_LUMINANCE | PFF_FLOAT, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RG16_UNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 16, 0, 0, 
			/* Masks and shifts */
			0xFFFF0000, 0x0000FFFF, 0, 0,
			16, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RG16_SNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SNORM, PCT_SNORM, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 16, 0, 0, 
			/* Masks and shifts */
			0xFFFF0000, 0x0000FFFF, 0, 0,
			16, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RG16_UINT", 
			/* Flags */
			0,
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UINT, PCT_UINT, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 16, 0, 0, 
			/* Masks and shifts */
			0xFFFF0000, 0x0000FFFF, 0, 0,
			16, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RG16_SINT", 
			/* Flags */
			PFF_NATIVEENDIAN, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SINT, PCT_SINT, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 16, 0, 0, 
			/* Masks and shifts */
			0xFFFF0000, 0x0000FFFF, 0, 0,
			16, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RG16_FLOAT", 
			/* Flags */
			PFF_FLOAT, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 16, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGB16_UNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_UNORM, PCT_NONE, 
			/* Channel bits */
			16, 16, 16, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGB16_SNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			3, 
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_SNORM, PCT_SNORM, PCT_SNORM, PCT_NONE, 
			/* Channel bits */
			16, 16, 16, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGB16_UINT", 
			/* Flags */
			0,
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_UINT, PCT_UINT, PCT_UINT, PCT_NONE, 
			/* Channel bits */
			16, 16, 16, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGB16_SINT", 
			/* Flags */
			PFF_NATIVEENDIAN,
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_SINT, PCT_SINT, PCT_SINT, PCT_NONE, 
			/* Channel bits */
			16, 16, 16, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGB16_FLOAT", 
			/* Flags */
			PFF_FLOAT, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_NONE, 
			/* Channel bits */
			16, 16, 16, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGBA16_UNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_UNORM, PCT_UNORM, 
			/* Channel bits */
			16, 16, 16, 16, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGBA16_SNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_SNORM, PCT_SNORM, PCT_SNORM, PCT_SNORM, 
			/* Channel bits */
			16, 16, 16, 16, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGBA16_UINT", 
			/* Flags */
			PFF_NATIVEENDIAN, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_UINT, PCT_UINT, PCT_UINT, PCT_UINT, 
			/* Channel bits */
			16, 16, 16, 16, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGBA16_SINT", 
			/* Flags */
			PFF_NATIVEENDIAN, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_SINT, PCT_SINT, PCT_SINT, PCT_SINT, 
			/* Channel bits */
			16, 16, 16, 16, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_RGBA16_FLOAT", 
			/* Flags */
			PFF_FLOAT, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, 
			/* Channel bits */
			16, 16, 16, 16, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_R32_UNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//-----------------------------------------------------------------------
		{
			"PF_R32_SNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_R32_UINT", 
			/* Flags */
			0, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UINT, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_R32_SINT", 
			/* Flags */
			0, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SINT, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_R32_FLOAT", 
			/* Flags */
			PFF_FLOAT, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_R, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RG32_UNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 32, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RG32_SNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SNORM, PCT_SNORM, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 32, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RG32_UINT", 
			/* Flags */
			0, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UINT, PCT_UINT, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 32, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RG32_SINT", 
			/* Flags */
			0, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_SINT, PCT_SINT, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 32, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RG32_FLOAT", 
			/* Flags */
			PFF_FLOAT, 
			/* Channel count */
			2, 
			/* Channel format */
			PCF_R, PCF_G, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 32, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RGB32_UNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_UNORM, PCT_NONE, 
			/* Channel bits */
			32, 32, 32, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RGB32_SNORM", 
			/* Flags */
			PFF_NORMALIZED, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_SNORM, PCT_SNORM, PCT_SNORM, PCT_NONE, 
			/* Channel bits */
			32, 32, 32, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RGB32_UINT", 
			/* Flags */
			0, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_UINT, PCT_UINT, PCT_UINT, PCT_NONE, 
			/* Channel bits */
			32, 32, 32, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RGB32_SINT", 
			/* Flags */
			0, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_SINT, PCT_SINT, PCT_SINT, PCT_NONE, 
			/* Channel bits */
			32, 32, 32, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RGB32_FLOAT", 
			/* Flags */
			PFF_FLOAT, 
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_NONE, 
			/* Channel bits */
			32, 32, 32, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//---------------------------------------------------------------------
		{
			"PF_RGBA32_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_ALPHA,
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_UNORM, PCT_UNORM, PCT_UNORM, PCT_UNORM, 
			/* Channel bits */
			32, 32, 32, 32, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_RGBA32_SNORM", 
				/* Flags */
				PFF_NORMALIZED | PFF_ALPHA,
				/* Channel count */
				4, 
				/* Channel format */
				PCF_R, PCF_G, PCF_B, PCF_A, 
				/* Channel type */
				PCT_SNORM, PCT_SNORM, PCT_SNORM, PCT_SNORM, 
				/* Channel bits */
				32, 32, 32, 32, 
				/* Masks and shifts */
				0, 0, 0, 0,
				0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
		"PF_RGBA32_UINT", 
			/* Flags */
			PFF_ALPHA, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_UINT, PCT_UINT, PCT_UINT, PCT_UINT, 
			/* Channel bits */
			32, 32, 32, 32, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_RGBA32_SINT", 
			/* Flags */
			PFF_ALPHA, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_SINT, PCT_SINT, PCT_SINT, PCT_SINT, 
			/* Channel bits */
			32, 32, 32, 32, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_RGBA32_FLOAT", 
			/* Flags */
			PFF_ALPHA | PFF_FLOAT, 
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, 
			/* Channel bits */
			32, 32, 32, 32, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_D16_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_DEPTH | PFF_NATIVEENDIAN,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_D, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 0, 0, 0, 
			/* Masks and shifts */
			0xFF, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_D24_UNORM_S8_UINT", 
			/* Flags */
			PFF_NORMALIZED | PFF_DEPTH | PFF_STENCIL,
			/* Channel count */
			2, 
			/* Channel format */
			PCF_D, PCF_S, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_UINT, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 8, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_D32_FLOAT", 
			/* Flags */
			PFF_DEPTH | PFF_FLOAT, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_D, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			32, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC1_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			4, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC1_UNORM_SRGB", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA, 
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM_SRGB, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			4, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC2_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC2_UNORM_SRGB", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM_SRGB, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC3_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC3_UNORM_SRGB", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM_SRGB, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC4_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC4_UNORM_SRGB", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM_SRGB, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			8, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC5_UNORM", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_BC5_UNORM_SRGB", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			1, 
			/* Channel format */
			PCF_BC, PCF_NONE, PCF_NONE, PCF_NONE, 
			/* Channel type */
			PCT_UNORM_SRGB, PCT_NONE, PCT_NONE, PCT_NONE, 
			/* Channel bits */
			16, 0, 0, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 
		//--------------------------------------------------------------------
		{
			"PF_PVRTCI_2bpp_RGBA", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, 
			/* Channel bits */
			2, 2, 2, 2, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 

		{
			"PF_PVRTC1_2bpp_RGB", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED,
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_NONE, 
			/* Channel bits */
			3, 3, 2, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 

		{
			"PF_PVRTCI_4bpp_RGBA", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, 
			/* Channel bits */
			4, 4, 4, 4, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		}, 

		{
			"PF_PVRTCI_4bpp_RGB", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED,
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_NONE, 
			/* Channel bits */
			5, 5, 6, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		},

		{
			"PF_ETC1", 
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED,
			/* Channel count */
			3, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_NONE, 
			/* Channel bits */
			4, 4, 4, 0, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		},

		{
			"PF_ETC2_RGB",
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED,
			/* Channel count */
			3,
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_NONE,
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_NONE,
			/* Channel bits */
			4, 4, 4, 0,
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		},

		{
			"PF_ETC2_RGBA",
			/* Flags */
			PFF_NORMALIZED | PFF_COMPRESSED | PFF_ALPHA,
			/* Channel count */
			4,
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A,
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_FLOAT,
			/* Channel bits */
			4, 4, 4, 4,
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		},

		{
			"PF_PVRTC_RGBA_4444", 
			/* Flags */
			PFF_NORMALIZED | PFF_ALPHA,
			/* Channel count */
			4, 
			/* Channel format */
			PCF_R, PCF_G, PCF_B, PCF_A, 
			/* Channel type */
			PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, PCT_FLOAT, 
			/* Channel bits */
			4, 4, 4, 4, 
			/* Masks and shifts */
			0, 0, 0, 0,
			0, 0, 0, 0
		},
	};

	const PixelFormatDesc& GetPixelFormatDesc(PixelFormat pixFmt)
	{
		const int ord = (int)pixFmt;
		EchoAssert(ord >= 0 && ord < PF_COUNT);

		return g_pixFmtDescArr[ord];
	}
}
