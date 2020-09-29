#pragma once

#include "pixel_box.h"

namespace Echo
{
	class PixelUtil
	{
	public:
		static String				GetPixelFormatName(PixelFormat pixFmt);
		static ui32					GetPixelBits(PixelFormat pixFmt);
		static ui32					GetPixelSize(PixelFormat pixFmt);
		static Dword				GetFlags(PixelFormat pixFmt);
		static bool					IsNormalized(PixelFormat pixFmt);
		static bool					HasAlpha(PixelFormat pixFmt);
		static bool					IsFloatingPoint(PixelFormat pixFmt);
		static bool					IsCompressed(PixelFormat pixFmt);
		static bool					IsDepth(PixelFormat pixFmt);
		static bool					IsStencil(PixelFormat pixFmt);
		static bool					IsLuminance(PixelFormat pixFmt);
		static bool					IsNativeEndian(PixelFormat pixFmt);
		static bool					IsAccessible(PixelFormat pixFmt);
		static PixelChannelType		GetChannelType(PixelFormat pixFmt, ui32 chIdx);
		static ui32					GetChannelCount(PixelFormat pixFmt);
		static ui32					GetMemorySize(ui32 width, ui32 height, ui32 depth, PixelFormat pixFmt);
		static ui32					CalcLevelSize(ui32 width, ui32 height, ui32 depth, ui32 level, PixelFormat pixFmt);
		static ui32					CalcSurfaceSize(ui32 width, ui32 height, ui32 depth, ui32 mipmaps, PixelFormat pixFmt);
		static void					UnpackColor(Color& color, PixelFormat pixFmt, const void* pSrc);
		static void					UnpackColor(float& r, float& g, float& b, float& a, PixelFormat pixFmt, const void* pSrc);
		static void					PackColor(const Color& color, PixelFormat pixFmt, void* pDest);
		static void					PackColor(float r, float g, float b, float a, PixelFormat pixFmt, void* pDest);
		static Color				ConvertColor(const Color& color, PixelFormat pixFmt);
		static int					optionalPO2(int n);
		static bool					isOptionalPO2(int n);

		// Convert pixels from one format to another
		static void					BulkPixelConversion(void* pSrcData, PixelFormat srcPixFmt, void* pDestData, PixelFormat dstPixFmt, ui32 count);
		static void					BulkPixelConversion(const PixelBox& src, const PixelBox& dst);
	};
}