#include "pixel_util.h"
#include "pixel_conversions.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	String PixelUtil::GetPixelFormatName(PixelFormat pixFmt)
	{
		return GetPixelFormatDesc(pixFmt).name;
	}

	ui32 PixelUtil::GetPixelBits(PixelFormat pixFmt)
	{
		PixelFormatDesc pixFmtDesc = GetPixelFormatDesc(pixFmt);
		return ((ui32)pixFmtDesc.ch0Bits + (ui32)pixFmtDesc.ch1Bits + (ui32)pixFmtDesc.ch2Bits + (ui32)pixFmtDesc.ch3Bits);
	}

	ui32 PixelUtil::GetPixelSize(PixelFormat pixFmt)
	{
		return (GetPixelBits(pixFmt) / 8);
	}

	Dword PixelUtil::GetFlags(PixelFormat pixFmt)
	{
		return GetPixelFormatDesc(pixFmt).flags;
	}

	bool PixelUtil::IsNormalized(PixelFormat pixFmt)
	{
		return (GetFlags(pixFmt) & PFF_NORMALIZED) > 0;
	}

	bool PixelUtil::HasAlpha(PixelFormat pixFmt)
	{
		return (GetFlags(pixFmt) & PFF_ALPHA) > 0;
	}

	bool PixelUtil::IsFloatingPoint(PixelFormat pixFmt)
	{
		return (GetFlags(pixFmt) & PFF_FLOAT) > 0;
	}

	bool PixelUtil::IsCompressed(PixelFormat pixFmt)
	{
		return (GetFlags(pixFmt) & PFF_COMPRESSED) > 0;
	}

	bool PixelUtil::IsDepth(PixelFormat pixFmt)
	{
		return (GetFlags(pixFmt) & PFF_DEPTH) > 0;
	}

	bool PixelUtil::IsStencil(PixelFormat pixFmt)
	{
		return (GetFlags(pixFmt) & PFF_STENCIL) > 0;
	}

	bool PixelUtil::IsLuminance(PixelFormat pixFmt)
	{
		return (GetFlags(pixFmt) & PFF_LUMINANCE) > 0;
	}

	bool PixelUtil::IsNativeEndian(PixelFormat pixFmt)
	{
		return (GetFlags(pixFmt) & PFF_NATIVEENDIAN) > 0;
	}

	bool PixelUtil::IsAccessible(PixelFormat pixFmt)
	{
		if (pixFmt == PF_UNKNOWN)
			return false;

		Dword flags = GetFlags(pixFmt);
		return !((flags & PFF_COMPRESSED) || (flags & PFF_DEPTH));
	}

	PixelChannelType PixelUtil::GetChannelType(PixelFormat pixFmt, ui32 chIdx)
	{
		switch (chIdx)
		{
		case 0:		return GetPixelFormatDesc(pixFmt).ch0Type;
		case 1:		return GetPixelFormatDesc(pixFmt).ch1Type;
		case 2:		return GetPixelFormatDesc(pixFmt).ch2Type;
		case 3:		return GetPixelFormatDesc(pixFmt).ch3Type;
		default:	return PCT_NONE;
		}
	}

	ui32 PixelUtil::GetChannelCount(PixelFormat pixFmt)
	{
		return GetPixelFormatDesc(pixFmt).chCount;
	}

	ui32 PixelUtil::GetMemorySize(ui32 width, ui32 height, ui32 depth, PixelFormat pixFmt)
	{
		if (IsCompressed(pixFmt))
		{
			switch (pixFmt)
			{
				// DXT formats work by dividing the image into 4x4 blocks, then encoding each
				// 4x4 block with a certain number of bytes. 
			case PF_BC1_UNORM:
			case PF_BC1_UNORM_SRGB:
				return ((width + 3) / 4) * ((height + 3) / 4) * 8 * depth;

			case PF_BC2_UNORM:
			case PF_BC2_UNORM_SRGB:
			case PF_BC3_UNORM:
			case PF_BC3_UNORM_SRGB:
			case PF_BC4_UNORM:
			case PF_BC4_SNORM:          return ((width + 3) / 4) * ((height + 3) / 4) * 16 * depth;

			case PF_BC5_UNORM:
			case PF_BC5_SNORM:          return ((width + 3) / 4) * ((height + 3) / 4) * 32 * depth;
			case PF_PVRTC1_2bpp_RGB:
			case PF_PVRTC1_2bpp_RGBA:   return Math::Max(2 * width * height * depth / 8, static_cast<ui32>(32));
			case PF_PVRTC1_4bpp_RGB:
			case PF_PVRTC1_4bpp_RGBA:
			case PF_PVRTC_RGBA_4444:	return Math::Max(4 * width * height * depth / 8, static_cast<ui32>(32));

			case PF_ETC2_RGB:			return (ui32)(Math::Ceil(width / 4.0) * Math::Ceil(height / 4.0) * 8);
			case PF_ETC2_RGBA:			return (ui32)(Math::Ceil(width / 4.0) * Math::Ceil(height / 4.0) * 16);

			default:
				return 0;
			}
		}
		else
		{
			return (width * height * depth * GetPixelBits(pixFmt) / 8);
		}
	}

	ui32 PixelUtil::CalcLevelSize(ui32 width, ui32 height, ui32 depth, ui32 level, PixelFormat pixFmt)
	{
		width >>= level;
		height >>= level;

		width = width > 0 ? width : 1;
		height = height > 0 ? height : 1;

		return GetMemorySize(width, height, depth, pixFmt);
	}

	ui32 PixelUtil::CalcSurfaceSize(ui32 width, ui32 height, ui32 depth, ui32 mipmaps, PixelFormat pixFmt)
	{
		ui32 mips = mipmaps > 0 ? mipmaps : 1;
		ui32 surSize = 0;
		for (ui32 i = 0; i < mips; ++i)
		{
			ui32 tmpWidth = width >> i;
			ui32 tmpHeight = height >> i;

			tmpWidth = tmpWidth > 0 ? tmpWidth : 1;
			tmpHeight = tmpHeight > 0 ? tmpHeight : 1;

			surSize += GetMemorySize(tmpWidth, tmpHeight, depth, pixFmt);
		}

		return surSize;
	}

	void PixelUtil::UnpackColor(Color& color, PixelFormat pixFmt, const void* pSrc)
	{
		float r, g, b, a;
		UnpackColor(r, g, b, a, pixFmt, pSrc);
		color.set(r, g, b, a);
	}

	void PixelUtil::UnpackColor(float& r, float& g, float& b, float& a, PixelFormat pixFmt, const void* pSrc)
	{
		const PixelFormatDesc& des = GetPixelFormatDesc(pixFmt);

		if (des.flags & PFF_NATIVEENDIAN)
		{
			switch (pixFmt)
			{
			case PF_R8_UNORM:
			{
				const Real f = 1.0f / 255.0f;
				r = f * ((Byte*)pSrc)[0];
				a = 1.f;
			}break;
			case PF_RGB8_UNORM:
			{
				const Real f = 1.0f / 255.0f;
				r = f * ((Byte*)pSrc)[0];
				g = f * ((Byte*)pSrc)[1];
				b = f * ((Byte*)pSrc)[2];
				a = 1.f;
			}break;
			case PF_RGBA8_UNORM:
			{
				const Real f = 1.0f / 255.0f;
				r = f * ((Byte*)pSrc)[0];
				g = f * ((Byte*)pSrc)[1];
				b = f * ((Byte*)pSrc)[2];
				a = f * ((Byte*)pSrc)[3];
			}break;
			default:
			{
				EchoAssert(false);
			}break;
			}

			// 			// Shortcut for integer formats unpacking
			// 			const ui32 value = Math::IntRead(pSrc, GetPixelSize(pixFmt));
			// 			if(des.flags & PFF_LUMINANCE)
			// 			{
			// 				// Luminance format -- only rbits used
			// 				r = g = b = Math::FixedToFloat((value & des.ch0Mask)>>des.ch0Shift, des.ch0Bits);
			// 			}
			// 			else
			// 			{
			// 				r = Math::FixedToFloat((value & des.ch0Mask)>>des.ch0Shift, des.ch0Bits);
			// 				g = Math::FixedToFloat((value & des.ch1Mask)>>des.ch1Shift, des.ch1Bits);
			// 				b = Math::FixedToFloat((value & des.ch2Mask)>>des.ch2Shift, des.ch2Bits);
			// 			}
			// 			if(des.flags & PFF_ALPHA)
			// 			{
			// 				a = Math::FixedToFloat((value & des.ch3Mask)>>des.ch3Shift, des.ch3Bits);
			// 			}
			// 			else
			// 			{
			// 				a = 1.0f; // No alpha, default a component to full
			// 			}
		}
		else
		{
			switch (pixFmt)
			{
			case PF_R32_FLOAT:
			{
				r = g = b = ((float*)pSrc)[0];
				a = 1.0f;
			} break;
			case PF_RG32_FLOAT:
			{
				r = ((float*)pSrc)[0];
				g = b = ((float*)pSrc)[1];
				a = 1.0f;
			} break;
			case PF_RGB32_FLOAT:
			{
				r = ((float*)pSrc)[0];
				g = ((float*)pSrc)[1];
				b = ((float*)pSrc)[2];
				a = 1.0f;
			} break;
			case PF_RGBA32_FLOAT:
			{
				r = ((float*)pSrc)[0];
				g = ((float*)pSrc)[1];
				b = ((float*)pSrc)[2];
				a = ((float*)pSrc)[3];
			} break;
			case PF_R16_FLOAT:
			{
				r = g = b = Math::HalfToFloat(((ui16*)pSrc)[0]);
				a = 1.0f;
			} break;
			case PF_R16_UINT:
			{
				const Real f = 1.0f / 65535.0f;
				r = f * (float)(((ui16*)pSrc)[0]);
				a = 1.0f;
			} break;
			case PF_RG16_FLOAT:
			{
				g = Math::HalfToFloat(((ui16*)pSrc)[0]);
				r = b = Math::HalfToFloat(((ui16*)pSrc)[1]);
				a = 1.0f;
			} break;
			case PF_RGB16_FLOAT:
			{
				r = Math::HalfToFloat(((ui16*)pSrc)[0]);
				g = Math::HalfToFloat(((ui16*)pSrc)[1]);
				b = Math::HalfToFloat(((ui16*)pSrc)[2]);
				a = 1.0f;
			} break;
			case PF_RGBA16_FLOAT:
			{
				r = Math::HalfToFloat(((ui16*)pSrc)[0]);
				g = Math::HalfToFloat(((ui16*)pSrc)[1]);
				b = Math::HalfToFloat(((ui16*)pSrc)[2]);
				a = Math::HalfToFloat(((ui16*)pSrc)[3]);
			} break;
			case PF_RGB32_UNORM:
			{
				r = Math::FixedToFloat(((ui32*)pSrc)[0], 32);
				g = Math::FixedToFloat(((ui32*)pSrc)[1], 32);
				b = Math::FixedToFloat(((ui32*)pSrc)[2], 32);
				a = 1.0f;
			} break;
			case PF_RGB32_SNORM:
			{
				r = Math::FixedToFloat(((i32*)pSrc)[0], 32);
				g = Math::FixedToFloat(((i32*)pSrc)[1], 32);
				b = Math::FixedToFloat(((i32*)pSrc)[2], 32);
				a = 1.0f;
			} break;
			case PF_RGB32_UINT:
			{
				r = (float)(((ui32*)pSrc)[0]);
				g = (float)(((ui32*)pSrc)[1]);
				b = (float)(((ui32*)pSrc)[2]);
				a = 1.0f;
			} break;
			case PF_RGB32_SINT:
			{
				r = (float)(((i32*)pSrc)[0]);
				g = (float)(((i32*)pSrc)[1]);
				b = (float)(((i32*)pSrc)[2]);
				a = 1.0f;
			} break;
			case PF_RGBA32_UNORM:
			{
				r = Math::FixedToFloat(((ui32*)pSrc)[0], 32);
				g = Math::FixedToFloat(((ui32*)pSrc)[1], 32);
				b = Math::FixedToFloat(((ui32*)pSrc)[2], 32);
				a = Math::FixedToFloat(((ui32*)pSrc)[3], 32);
			} break;
			case PF_RGBA32_SNORM:
			{
				r = Math::FixedToFloat(((i32*)pSrc)[0], 32);
				g = Math::FixedToFloat(((i32*)pSrc)[1], 32);
				b = Math::FixedToFloat(((i32*)pSrc)[2], 32);
				a = Math::FixedToFloat(((i32*)pSrc)[3], 32);
			} break;
			case PF_RGBA32_UINT:
			{
				r = (float)(((ui32*)pSrc)[0]);
				g = (float)(((ui32*)pSrc)[1]);
				b = (float)(((ui32*)pSrc)[2]);
				a = (float)(((ui32*)pSrc)[3]);
			} break;
			case PF_RGBA32_SINT:
			{
				r = (float)(((i32*)pSrc)[0]);
				g = (float)(((i32*)pSrc)[1]);
				b = (float)(((i32*)pSrc)[2]);
				a = (float)(((i32*)pSrc)[3]);
			} break;
			case PF_RGB16_UNORM:
			{
				r = Math::FixedToFloat(((ui16*)pSrc)[0], 16);
				g = Math::FixedToFloat(((ui16*)pSrc)[1], 16);
				b = Math::FixedToFloat(((ui16*)pSrc)[2], 16);
				a = 1.0f;
			} break;
			case PF_RGB16_SNORM:
			{
				r = Math::FixedToFloat(((i16*)pSrc)[0], 16);
				g = Math::FixedToFloat(((i16*)pSrc)[1], 16);
				b = Math::FixedToFloat(((i16*)pSrc)[2], 16);
				a = 1.0f;
			} break;
			case PF_RGB16_UINT:
			{
				r = (float)(((ui16*)pSrc)[0]);
				g = (float)(((ui16*)pSrc)[1]);
				b = (float)(((ui16*)pSrc)[2]);
				a = 1.0f;
			} break;
			case PF_RGB16_SINT:
			{
				r = (float)(((i16*)pSrc)[0]);
				g = (float)(((i16*)pSrc)[1]);
				b = (float)(((i16*)pSrc)[2]);
				a = 1.0f;
			} break;
			case PF_RGBA16_UNORM:
			{
				r = Math::FixedToFloat(((ui16*)pSrc)[0], 16);
				g = Math::FixedToFloat(((ui16*)pSrc)[1], 16);
				b = Math::FixedToFloat(((ui16*)pSrc)[2], 16);
				a = Math::FixedToFloat(((ui16*)pSrc)[3], 16);
			} break;
			case PF_RGBA16_SNORM:
			{
				r = Math::FixedToFloat(((i16*)pSrc)[0], 16);
				g = Math::FixedToFloat(((i16*)pSrc)[1], 16);
				b = Math::FixedToFloat(((i16*)pSrc)[2], 16);
				a = Math::FixedToFloat(((i16*)pSrc)[3], 16);
			} break;
			case PF_RGBA16_UINT:
			{
				r = (float)(((ui16*)pSrc)[0]);
				g = (float)(((ui16*)pSrc)[1]);
				b = (float)(((ui16*)pSrc)[2]);
				a = (float)(((ui16*)pSrc)[3]);
			} break;
			case PF_RGBA16_SINT:
			{
				r = (float)(((i16*)pSrc)[0]);
				g = (float)(((i16*)pSrc)[1]);
				b = (float)(((i16*)pSrc)[2]);
				a = (float)(((i16*)pSrc)[3]);
			} break;
			default:
			{
				// Not yet supported
				EchoLogError("unpack from PixelFormat [%s] not implemented", GetPixelFormatName(pixFmt).c_str());
			} break;
			}
		}
	}

	void PixelUtil::PackColor(const Color& color, PixelFormat pixFmt, void* pDest)
	{
		PackColor((float)color.r, (float)color.g, (float)color.b, (float)color.a, pixFmt, pDest);
	}

	void PixelUtil::PackColor(float r, float g, float b, float a, PixelFormat pixFmt, void* pDest)
	{
		// Catch-it-all here
		const PixelFormatDesc& des = GetPixelFormatDesc(pixFmt);

		if (des.flags & PFF_NATIVEENDIAN)
		{
			// Do the packing
			const ui32 value = ((Math::FloatToFixed(r, des.ch0Bits) << des.ch0Shift) & des.ch0Mask) |
				((Math::FloatToFixed(g, des.ch1Bits) << des.ch1Shift) & des.ch1Mask) |
				((Math::FloatToFixed(b, des.ch2Bits) << des.ch2Shift) & des.ch2Mask) |
				((Math::FloatToFixed(a, des.ch3Bits) << des.ch3Shift) & des.ch3Mask);

			// And write to memory
			Math::IntWrite(pDest, GetPixelSize(pixFmt), value);
		}
		else
		{
			switch (pixFmt)
			{
			case PF_R32_FLOAT:
			{
				((float*)pDest)[0] = r;
			} break;
			case PF_RG32_FLOAT:
			{
				((float*)pDest)[0] = g;
				((float*)pDest)[1] = r;
			} break;
			case PF_RGB32_FLOAT:
			{
				((float*)pDest)[0] = r;
				((float*)pDest)[1] = g;
				((float*)pDest)[2] = b;
			} break;
			case PF_RGBA32_FLOAT:
			{
				((float*)pDest)[0] = r;
				((float*)pDest)[1] = g;
				((float*)pDest)[2] = b;
				((float*)pDest)[3] = a;
			} break;
			case PF_R16_FLOAT:
			{
				((ui16*)pDest)[0] = Math::FloatToHalf(r);
			} break;
			case PF_RG16_FLOAT:
			{
				((ui16*)pDest)[0] = Math::FloatToHalf(g);
				((ui16*)pDest)[1] = Math::FloatToHalf(r);
			} break;
			case PF_RGB16_FLOAT:
			{
				((ui16*)pDest)[0] = Math::FloatToHalf(r);
				((ui16*)pDest)[1] = Math::FloatToHalf(g);
				((ui16*)pDest)[2] = Math::FloatToHalf(b);
			} break;
			case PF_RGBA16_FLOAT:
			{
				((ui16*)pDest)[0] = Math::FloatToHalf(r);
				((ui16*)pDest)[1] = Math::FloatToHalf(g);
				((ui16*)pDest)[2] = Math::FloatToHalf(b);
				((ui16*)pDest)[3] = Math::FloatToHalf(a);
			} break;
			case PF_RGB32_UNORM:
			{
				((ui32*)pDest)[0] = (ui32)Math::FloatToFixed(r, 32);
				((ui32*)pDest)[1] = (ui32)Math::FloatToFixed(g, 32);
				((ui32*)pDest)[2] = (ui32)Math::FloatToFixed(b, 32);
			} break;
			case PF_RGB32_SNORM:
			{
				((i32*)pDest)[0] = (i32)Math::FloatToFixed(r, 32);
				((i32*)pDest)[1] = (i32)Math::FloatToFixed(g, 32);
				((i32*)pDest)[2] = (i32)Math::FloatToFixed(b, 32);
			} break;
			case PF_RGB32_UINT:
			{
				((ui32*)pDest)[0] = (ui32)r;
				((ui32*)pDest)[1] = (ui32)g;
				((ui32*)pDest)[2] = (ui32)b;
			} break;
			case PF_RGB32_SINT:
			{
				((i32*)pDest)[0] = (i32)r;
				((i32*)pDest)[1] = (i32)g;
				((i32*)pDest)[2] = (i32)b;
			} break;
			case PF_RGBA32_UNORM:
			{
				((ui32*)pDest)[0] = (ui32)Math::FloatToFixed(r, 32);
				((ui32*)pDest)[1] = (ui32)Math::FloatToFixed(g, 32);
				((ui32*)pDest)[2] = (ui32)Math::FloatToFixed(b, 32);
				((ui32*)pDest)[3] = (ui32)Math::FloatToFixed(a, 32);
			} break;
			case PF_RGBA32_SNORM:
			{
				((i32*)pDest)[0] = (i32)Math::FloatToFixed(r, 32);
				((i32*)pDest)[1] = (i32)Math::FloatToFixed(g, 32);
				((i32*)pDest)[2] = (i32)Math::FloatToFixed(b, 32);
				((i32*)pDest)[3] = (i32)Math::FloatToFixed(a, 32);
			} break;
			case PF_RGBA32_UINT:
			{
				((ui32*)pDest)[0] = (ui32)r;
				((ui32*)pDest)[1] = (ui32)g;
				((ui32*)pDest)[2] = (ui32)b;
				((ui32*)pDest)[3] = (ui32)a;
			} break;
			case PF_RGBA32_SINT:
			{
				((i32*)pDest)[0] = (i32)r;
				((i32*)pDest)[1] = (i32)g;
				((i32*)pDest)[2] = (i32)b;
				((i32*)pDest)[3] = (i32)a;
			} break;
			case PF_RGB16_UNORM:
			{
				((ui16*)pDest)[0] = (ui16)Math::FloatToFixed(r, 16);
				((ui16*)pDest)[1] = (ui16)Math::FloatToFixed(g, 16);
				((ui16*)pDest)[2] = (ui16)Math::FloatToFixed(b, 16);
			} break;
			case PF_RGB16_SNORM:
			{
				((ui16*)pDest)[0] = (ui16)Math::FloatToFixed(r, 16);
				((ui16*)pDest)[1] = (ui16)Math::FloatToFixed(g, 16);
				((ui16*)pDest)[2] = (ui16)Math::FloatToFixed(b, 16);
			} break;
			case PF_RGB16_UINT:
			{
				((ui16*)pDest)[0] = (ui16)r;
				((ui16*)pDest)[1] = (ui16)g;
				((ui16*)pDest)[2] = (ui16)b;
			} break;
			case PF_RGB16_SINT:
			{
				((i16*)pDest)[0] = (i16)r;
				((i16*)pDest)[1] = (i16)g;
				((i16*)pDest)[2] = (i16)b;
			} break;
			case PF_RGBA16_UNORM:
			{
				((ui16*)pDest)[0] = (ui16)Math::FloatToFixed(r, 16);
				((ui16*)pDest)[1] = (ui16)Math::FloatToFixed(g, 16);
				((ui16*)pDest)[2] = (ui16)Math::FloatToFixed(b, 16);
				((ui16*)pDest)[3] = (ui16)Math::FloatToFixed(a, 16);
			} break;
			case PF_RGBA16_SNORM:
			{
				((i16*)pDest)[0] = (i16)Math::FloatToFixed(r, 16);
				((i16*)pDest)[1] = (i16)Math::FloatToFixed(g, 16);
				((i16*)pDest)[2] = (i16)Math::FloatToFixed(b, 16);
				((i16*)pDest)[3] = (i16)Math::FloatToFixed(a, 16);
			} break;
			case PF_RGBA16_UINT:
			{
				((ui16*)pDest)[0] = (ui16)r;
				((ui16*)pDest)[1] = (ui16)g;
				((ui16*)pDest)[2] = (ui16)b;
				((ui16*)pDest)[3] = (ui16)a;
			} break;
			case PF_RGBA16_SINT:
			{
				((i16*)pDest)[0] = (i16)r;
				((i16*)pDest)[1] = (i16)g;
				((i16*)pDest)[2] = (i16)b;
				((i16*)pDest)[3] = (i16)a;
			} break;
			default:
				// Not yet supported
				EchoLogError("unpack from PixelFormat [%s] not implemented", GetPixelFormatName(pixFmt).c_str());
				break;
			}
		}
	}

	Color PixelUtil::ConvertColor(const Color& color, PixelFormat pixFmt)
	{
		Color retColor;

#if(FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR)
		switch (pixFmt)
		{
		case PF_BGR8_UNORM:
		case PF_BGRA8_UNORM:
		{
			retColor.r = color.b;
			retColor.g = color.g;
			retColor.b = color.r;
			retColor.a = color.a;
		} break;
		default:
			retColor = color;
		}
#else
		retColor = color;
#endif

		return retColor;
	}

	int PixelUtil::optionalPO2(int n)
	{
		--n;
		n |= n >> 16;
		n |= n >> 8;
		n |= n >> 4;
		n |= n >> 2;
		n |= n >> 1;
		++n;
		return n;
	}

	bool PixelUtil::isOptionalPO2(int n)
	{
		int n2 = optionalPO2(n);
		return n2 == n;
	}

	void PixelUtil::BulkPixelConversion(void* pSrcData, PixelFormat srcPixFmt, void* pDestData, PixelFormat dstPixFmt, ui32 count)
	{
		PixelBox src(count, 1, 1, srcPixFmt, pSrcData);
		PixelBox dst(count, 1, 1, dstPixFmt, pDestData);

		BulkPixelConversion(src, dst);
	}

	void PixelUtil::BulkPixelConversion(const PixelBox& src, const PixelBox& dst)
	{
		assert(src.getWidth() == dst.getWidth() &&
			src.getHeight() == dst.getHeight() &&
			src.getDepth() == dst.getDepth());

		// Check for compressed formats, we don't support decompression, compression or recoding
		if (PixelUtil::IsCompressed(src.pixFmt) || PixelUtil::IsCompressed(dst.pixFmt))
		{
			if (src.pixFmt == dst.pixFmt)
			{
				memcpy(dst.pData, src.pData, src.getConsecutiveSize());
				return;
			}
			else
			{
				EchoLogError("This method can not be used to compress or decompress images");
				return;
			}
		}

		// The easy case
		if (src.pixFmt == dst.pixFmt)
		{
			// Everything consecutive?
			if (src.isConsecutive() && dst.isConsecutive())
			{
				memcpy(dst.pData, src.pData, src.getConsecutiveSize());
				return;
			}

			const ui32 srcPixelSize = PixelUtil::GetPixelSize(src.pixFmt);
			const ui32 dstPixelSize = PixelUtil::GetPixelSize(dst.pixFmt);
			ui8* srcptr = static_cast<ui8*>(src.pData) + (src.left + src.top * src.rowPitch + src.front * src.slicePitch) * srcPixelSize;
			ui8* dstptr = static_cast<ui8*>(dst.pData) + (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch) * dstPixelSize;

			// Calculate pitches+skips in bytes
			const ui32 srcRowPitchBytes = src.rowPitch * srcPixelSize;
			//const ui32 srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
			const ui32 srcSliceSkipBytes = src.getSliceSkip() * srcPixelSize;

			const ui32 dstRowPitchBytes = dst.rowPitch * dstPixelSize;
			//const ui32 dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
			const ui32 dstSliceSkipBytes = dst.getSliceSkip() * dstPixelSize;

			// Otherwise, copy per row
			const ui32 rowSize = src.getWidth() * srcPixelSize;
			for (ui32 z = src.front; z < src.back; ++z)
			{
				for (ui32 y = src.top; y < src.bottom; ++y)
				{
					memcpy(dstptr, srcptr, rowSize);
					srcptr += srcRowPitchBytes;
					dstptr += dstRowPitchBytes;
				}

				srcptr += srcSliceSkipBytes;
				dstptr += dstSliceSkipBytes;
			}

			return;
		}

		// Is there a specialized, inlined, conversion?
		if (DoOptimizedConversion(src, dst))
		{
			// If so, good
			return;
		}

		const ui32 srcPixelSize = PixelUtil::GetPixelSize(src.pixFmt);
		const ui32 dstPixelSize = PixelUtil::GetPixelSize(dst.pixFmt);
		ui8* srcptr = static_cast<ui8*>(src.pData) + (src.left + src.top * src.rowPitch + src.front * src.slicePitch) * srcPixelSize;
		ui8* dstptr = static_cast<ui8*>(dst.pData) + (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch) * dstPixelSize;

		// Calculate pitches+skips in bytes
		const ui32 srcRowSkipBytes = src.getRowSkip() * srcPixelSize;
		const ui32 srcSliceSkipBytes = src.getSliceSkip() * srcPixelSize;
		const ui32 dstRowSkipBytes = dst.getRowSkip() * dstPixelSize;
		const ui32 dstSliceSkipBytes = dst.getSliceSkip() * dstPixelSize;

		// The brute force fallback
		float r = 0, g = 0, b = 0, a = 1;
		for (ui32 z = src.front; z < src.back; ++z)
		{
			for (ui32 y = src.top; y < src.bottom; ++y)
			{
				for (ui32 x = src.left; x < src.right; ++x)
				{
					UnpackColor(r, g, b, a, src.pixFmt, srcptr);
					PackColor(r, g, b, a, dst.pixFmt, dstptr);
					srcptr += srcPixelSize;
					dstptr += dstPixelSize;
				}
				srcptr += srcRowSkipBytes;
				dstptr += dstRowSkipBytes;
			}
			srcptr += srcSliceSkipBytes;
			dstptr += dstSliceSkipBytes;
		}
	}
}