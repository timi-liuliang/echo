#ifndef __ECHO_PIXELCONVERSIONS_H__
#define __ECHO_PIXELCONVERSIONS_H__

#include <engine/core/Base/echo_def.h>

namespace Echo
{
#pragma pack(push, 1)

	template <PixelFormat PIX_FMT> struct PixelType {};

	template <> struct PixelType<PF_R8_UNORM> { ui8 r; };
	template <> struct PixelType<PF_R8_SNORM> { i8 r; };
	template <> struct PixelType<PF_R8_UINT> { ui8 r; };
	template <> struct PixelType<PF_R8_SINT> { i8 r; };

	template <> struct PixelType<PF_RG8_UNORM> { ui8 r, g; };
	template <> struct PixelType<PF_RG8_SNORM> { i8 r, g; };
	template <> struct PixelType<PF_RG8_UINT> { ui8 r, g; };
	template <> struct PixelType<PF_RG8_SINT> { i8 r, g; };

	template <> struct PixelType<PF_RGB8_UNORM> { ui8 r, g, b; };
	template <> struct PixelType<PF_RGB8_SNORM> { i8 r, g, b; };
	template <> struct PixelType<PF_RGB8_UINT> { ui8 r, g, b; };
	template <> struct PixelType<PF_RGB8_SINT> { i8 r, g, b; };

	template <> struct PixelType<PF_BGR8_UNORM> { ui8 b, g, r; };

	template <> struct PixelType<PF_RGBA8_UNORM> { ui8 r, g, b, a; };
	template <> struct PixelType<PF_RGBA8_SNORM> { i8 r, g, b, a; };
	template <> struct PixelType<PF_RGBA8_UINT> { ui8 r, g, b, a; };
	template <> struct PixelType<PF_RGBA8_SINT> { i8 r, g, b, a; };

	template <> struct PixelType<PF_BGRA8_UNORM> { ui8 b, g, r, a; };

	template <> struct PixelType<PF_R16_UNORM> { ui16 r; };
	template <> struct PixelType<PF_R16_SNORM> { i16 r; };
	template <> struct PixelType<PF_R16_UINT> { ui16 r; };
	template <> struct PixelType<PF_R16_SINT> { i16 r; };

	template <> struct PixelType<PF_RG16_UNORM> { ui16 r, g; };
	template <> struct PixelType<PF_RG16_SNORM> { i16 r, g; };
	template <> struct PixelType<PF_RG16_UINT> { ui16 r, g; };
	template <> struct PixelType<PF_RG16_SINT> { i16 r, g; };

	template <> struct PixelType<PF_RGB16_UNORM> { ui16 r, g, b; };
	template <> struct PixelType<PF_RGB16_SNORM> { i16 r, g, b; };
	template <> struct PixelType<PF_RGB16_UINT> { ui16 r, g, b; };
	template <> struct PixelType<PF_RGB16_SINT> { i16 r, g, b; };

	template <> struct PixelType<PF_RGBA16_UNORM> { ui16 r, g, b, a; };
	template <> struct PixelType<PF_RGBA16_SNORM> { i16 r, g, b, a; };
	template <> struct PixelType<PF_RGBA16_UINT> { ui16 r, g, b, a; };
	template <> struct PixelType<PF_RGBA16_SINT> { i16 r, g, b, a; };

	template <> struct PixelType<PF_R32_UNORM> { ui32 r; };
	template <> struct PixelType<PF_R32_SNORM> { i32 r; };
	template <> struct PixelType<PF_R32_UINT> { ui32 r; };
	template <> struct PixelType<PF_R32_SINT> { i32 r; };
	template <> struct PixelType<PF_R32_FLOAT> { float r; };

	template <> struct PixelType<PF_RG32_UNORM> { ui32 r, g; };
	template <> struct PixelType<PF_RG32_SNORM> { i32 r, g; };
	template <> struct PixelType<PF_RG32_UINT> { ui32 r, g; };
	template <> struct PixelType<PF_RG32_SINT> { i32 r, g; };
	template <> struct PixelType<PF_RG32_FLOAT> { float r, g; };

	template <> struct PixelType<PF_RGB32_UNORM> { ui32 r, g, b; };
	template <> struct PixelType<PF_RGB32_SNORM> { i32 r, g, b; };
	template <> struct PixelType<PF_RGB32_UINT> { ui32 r, g, b; };
	template <> struct PixelType<PF_RGB32_SINT> { i32 r, g, b; };
	template <> struct PixelType<PF_RGB32_FLOAT> { float r, g, b; };

	template <> struct PixelType<PF_RGBA32_UNORM> { ui32 r, g, b, a; };
	template <> struct PixelType<PF_RGBA32_SNORM> { i32 r, g, b, a; };
	template <> struct PixelType<PF_RGBA32_UINT> { ui32 r, g, b, a; };
	template <> struct PixelType<PF_RGBA32_SINT> { i32 r, g, b, a; };
	template <> struct PixelType<PF_RGBA32_FLOAT> { float r, g, b, a; };

#pragma pack(pop)

	//////////////////////////////////////////////////////////////////////////

#define FMTCONVERTERID(from, to) (((from)<<8)|(to))

	/**
	* Convert a box of pixel from one type to another. Who needs automatic code 
	* generation when we have C++ templates and the policy design pattern.
	* 
	* @param   U       Policy class to facilitate pixel-to-pixel conversion. This class
	*    has at least two typedefs: SrcType and DstType. SrcType is the source element type,
	*    dstType is the destination element type. It also has a static method, pixelConvert, that
	*    converts a srcType into a dstType.
	*/
	template <class U> 
	struct PixelBoxConverter 
	{
		static const int ID = U::ID;
		static void Conversion(const PixelBox &src, const PixelBox &dst)
		{
			typename U::SrcType *srcptr = static_cast<typename U::SrcType*>(src.pData) + (src.left + src.top * src.rowPitch + src.front * src.slicePitch);
			typename U::DstType *dstptr = static_cast<typename U::DstType*>(dst.pData) + (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch);

			const ui32 srcSliceSkip = src.getSliceSkip();
			const ui32 dstSliceSkip = dst.getSliceSkip();
			const ui32 k = src.right - src.left;

			for(ui32 z=src.front; z<src.back; ++z) 
			{
				for(ui32 y = src.top; y < src.bottom; ++y)
				{
					for(ui32 x = 0; x < k; x++)
					{
						U::PixelConvert(srcptr[x], dstptr[x]);
					}

					srcptr += src.rowPitch;
					dstptr += dst.rowPitch;
				}

				srcptr += srcSliceSkip;
				dstptr += dstSliceSkip;
			}
		}
	};

	template <PixelFormat form, PixelFormat to> 
	struct PixelConverter
	{
		static const int ID = FMTCONVERTERID(form, to);
		typedef PixelType<form>		SrcType;
		typedef PixelType<to>		DstType;    
	};

	struct RGBA8UNORM_TO_BGRA8UNORM: public PixelConverter <PF_RGBA8_UNORM, PF_BGRA8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = src.r;
			dst.g = src.g;
			dst.b = src.b;
			dst.a = src.a;
		}
	};

	struct BGRA8UNORM_TO_RGBA8UNORM: public PixelConverter <PF_BGRA8_UNORM, PF_RGBA8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = src.r;
			dst.g = src.g;
			dst.b = src.b;
			dst.a = src.a;
		}
	};

	struct BGRA8UNORM_TO_R8UNORM: public PixelConverter <PF_BGRA8_UNORM, PF_R8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = src.r;
		}
	};

	struct R8UNORM_TO_BGRA8UNORM: public PixelConverter <PF_R8_UNORM, PF_BGRA8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = src.r;
			dst.g = dst.b = 0x00;
			dst.a = 0xff;
		}
	};

	struct R8UNORM_TO_RGBA8UNORM: public PixelConverter <PF_R8_UNORM, PF_RGBA8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = src.r;
			dst.g = dst.b = 0x00;
			dst.a = 0xff;
		}
	};

	struct R8UNORM_TO_R16UNORM: public PixelConverter <PF_R8_UNORM, PF_R16_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = (ui16)src.r;
		}
	};

	struct R16UNORM_TO_R8UNORM: public PixelConverter <PF_R16_UNORM, PF_R8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = (ui8)src.r;
		}
	};

	struct RGB8UNORM_TO_BGR8UNORM: public PixelConverter <PF_RGB8_UNORM, PF_BGR8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.b = src.b;
			dst.g = src.g;
			dst.r = src.r;
		}
	};

	struct RGB8UNORM_TO_RGBA8UNORM: public PixelConverter <PF_RGB8_UNORM, PF_RGBA8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = src.r;
			dst.g = src.g;
			dst.b = src.b;
			dst.a = 0xff;
		}
	};

	struct RGB8UNORM_TO_BGRA8UNORM: public PixelConverter <PF_RGB8_UNORM, PF_BGRA8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.b = src.b;
			dst.g = src.g;
			dst.r = src.r;
			dst.a = 0xff;
		}
	};

	struct BGR8UNORM_TO_RGB8UNORM: public PixelConverter <PF_BGR8_UNORM, PF_RGB8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = src.r;
			dst.g = src.g;
			dst.b = src.b;
		}
	};

	struct BGR8UNORM_TO_BGRA8UNORM: public PixelConverter <PF_BGR8_UNORM, PF_BGRA8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.b = src.b;
			dst.g = src.g;
			dst.r = src.r;
			dst.a = 0xff;
		}
	};

	struct BGR8UNORM_TO_RGBA8UNORM: public PixelConverter <PF_BGR8_UNORM, PF_RGBA8_UNORM>
	{
		inline static void PixelConvert(const SrcType &src, DstType &dst)
		{
			dst.r = src.r;
			dst.g = src.g;
			dst.b = src.b;
			dst.a = 0xff;
		}
	};

#define CASECONVERTER(type) case type::ID : PixelBoxConverter<type>::Conversion(src, dst); return true;

	inline bool DoOptimizedConversion(const PixelBox &src, const PixelBox &dst)
	{;
	switch(FMTCONVERTERID(src.pixFmt, dst.pixFmt))
	{
		// Register converters here
		CASECONVERTER(RGBA8UNORM_TO_BGRA8UNORM);
		CASECONVERTER(BGRA8UNORM_TO_RGBA8UNORM);
		CASECONVERTER(BGRA8UNORM_TO_R8UNORM);
		CASECONVERTER(R8UNORM_TO_BGRA8UNORM);
		CASECONVERTER(R8UNORM_TO_RGBA8UNORM);
		CASECONVERTER(R8UNORM_TO_R16UNORM);
		CASECONVERTER(R16UNORM_TO_R8UNORM);
		CASECONVERTER(RGB8UNORM_TO_BGR8UNORM);
		CASECONVERTER(RGB8UNORM_TO_RGBA8UNORM);
		CASECONVERTER(RGB8UNORM_TO_BGRA8UNORM);
		CASECONVERTER(BGR8UNORM_TO_RGB8UNORM);
		CASECONVERTER(BGR8UNORM_TO_BGRA8UNORM);
		CASECONVERTER(BGR8UNORM_TO_RGBA8UNORM);

	default:
		return 0;
	}
	}
#undef CASECONVERTER
}
#endif