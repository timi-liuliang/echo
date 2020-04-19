#pragma once

#include "PixelUtil.h"

namespace Echo
{
	// variable name hints:
	// sx_48 = 16/48-bit fixed-point x-position in source
	// stepx = difference between adjacent sx_48 values
	// sx1 = lower-bound integer x-position in source
	// sx2 = upper-bound integer x-position in source
	// sxf = fractional weight between sx1 and sx2
	// x,y,z = location of output pixel in destination

	// nearest-neighbor resampler, does not convert formats.
	// templated on bytes-per-pixel to allow compiler optimizations, such
	// as simplifying memcpy() and replacing multiplies with bitshifts
	template<ui32 elemsize> struct NearestResampler
	{
		static void Scale(const PixelBox& src, const PixelBox& dst)
		{
			// assert(src.format == dst.format);

			// srcdata stays at beginning, pdst is a moving pointer
			Byte* srcdata = (Byte*)src.pData;
			Byte* pdst = (Byte*)dst.pData;

			// sx_48,sy_48,sz_48 represent current position in source
			// using 16/48-bit fixed precision, incremented by steps
			ui64 stepx = ((ui64)src.getWidth() << 48) / dst.getWidth();
			ui64 stepy = ((ui64)src.getHeight() << 48) / dst.getHeight();
			ui64 stepz = ((ui64)src.getDepth() << 48) / dst.getDepth();

			// note: ((stepz>>1) - 1) is an extra half-step increment to adjust
			// for the center of the destination pixel, not the top-left corner
			ui64 sz_48 = (stepz >> 1) - 1;
			for(ui32 z = dst.front; z < dst.back; z++, sz_48 += stepz)
			{
				ui32 srczoff = (ui32)(sz_48 >> 48) * src.slicePitch;

				ui64 sy_48 = (stepy >> 1) - 1;
				for(ui32 y = dst.top; y < dst.bottom; y++, sy_48 += stepy)
				{
					ui32 srcyoff = (ui32)(sy_48 >> 48) * src.rowPitch;

					ui64 sx_48 = (stepx >> 1) - 1;
					for(size_t x = dst.left; x < dst.right; x++, sx_48 += stepx)
					{
						Byte* psrc = srcdata + elemsize*((ui32)(sx_48 >> 48) + srcyoff + srczoff);
						memcpy(pdst, psrc, elemsize);
						pdst += elemsize;
					}
					pdst += elemsize*dst.getRowSkip();
				}
				pdst += elemsize*dst.getSliceSkip();
			} // for
		}
	};

	// default floating-point linear resampler, does format conversion
	struct LinearResampler
	{
		static void Scale(const PixelBox& src, const PixelBox& dst)
		{
			ui32 srcelemsize = PixelUtil::GetPixelSize(src.pixFmt);
			ui32 dstelemsize = PixelUtil::GetPixelSize(dst.pixFmt);

			// srcdata stays at beginning, pdst is a moving pointer
			Byte* srcdata = (Byte*)src.pData;
			Byte* pdst = (Byte*)dst.pData;

			// sx_48,sy_48,sz_48 represent current position in source
			// using 16/48-bit fixed precision, incremented by steps
			ui64 stepx = ((ui64)src.getWidth() << 48) / dst.getWidth();
			ui64 stepy = ((ui64)src.getHeight() << 48) / dst.getHeight();
			ui64 stepz = ((ui64)src.getDepth() << 48) / dst.getDepth();

			// temp is 16/16 bit fixed precision, used to adjust a source
			// coordinate (x, y, or z) backwards by half a pixel so that the
			// integer bits represent the first sample (eg, sx1) and the
			// fractional bits are the blend weight of the second sample
			ui32 temp;

			// note: ((stepz>>1) - 1) is an extra half-step increment to adjust
			// for the center of the destination pixel, not the top-left corner
			ui64 sz_48 = (stepz >> 1) - 1;
			for(ui32 z = dst.front; z < dst.back; z++, sz_48+=stepz)
			{
				temp = static_cast<ui32>(sz_48 >> 32);
				temp = (temp > 0x8000)? temp - 0x8000 : 0;
				ui32 sz1 = temp >> 16;				 // src z, sample #1
				ui32 sz2 = Math::Min(sz1+1, src.getDepth()-1);// src z, sample #2
				float szf = (temp & 0xFFFF) / 65536.f; // weight of sample #2

				ui64 sy_48 = (stepy >> 1) - 1;
				for(ui32 y = dst.top; y < dst.bottom; y++, sy_48+=stepy)
				{
					temp = static_cast<ui32>(sy_48 >> 32);
					temp = (temp > 0x8000)? temp - 0x8000 : 0;
					ui32 sy1 = temp >> 16;					// src y #1
					ui32 sy2 = Math::Min(sy1+1, src.getHeight()-1);// src y #2
					float syf = (temp & 0xFFFF) / 65536.f; // weight of #2

					ui64 sx_48 = (stepx >> 1) - 1;
					for(ui32 x = dst.left; x < dst.right; x++, sx_48 += stepx)
					{
						temp = static_cast<ui32>(sx_48 >> 32);
						temp = (temp > 0x8000)? temp - 0x8000 : 0;
						ui32 sx1 = temp >> 16;					// src x #1
						ui32 sx2 = Math::Min(sx1+1,src.getWidth()-1);// src x #2
						float sxf = (temp & 0xFFFF) / 65536.f; // weight of #2

						Color x1y1z1, x2y1z1, x1y2z1, x2y2z1;
						Color x1y1z2, x2y1z2, x1y2z2, x2y2z2;

#define UNPACK(dst,x,y,z) PixelUtil::UnpackColor(dst, src.pixFmt, srcdata + srcelemsize*((x)+(y)*src.rowPitch+(z)*src.slicePitch))

						UNPACK(x1y1z1, sx1, sy1, sz1); UNPACK(x2y1z1, sx2, sy1, sz1);
						UNPACK(x1y2z1, sx1, sy2, sz1); UNPACK(x2y2z1, sx2, sy2, sz1);
						UNPACK(x1y1z2, sx1, sy1, sz2); UNPACK(x2y1z2, sx2, sy1, sz2);
						UNPACK(x1y2z2, sx1, sy2, sz2); UNPACK(x2y2z2, sx2, sy2, sz2);
#undef UNPACK

						Color accum = 
							x1y1z1 * (Real)((1.0f - sxf)*(1.0f - syf)*(1.0f - szf)) +
							x2y1z1 * (Real)(        sxf *(1.0f - syf)*(1.0f - szf)) +
							x1y2z1 * (Real)((1.0f - sxf)*        syf *(1.0f - szf)) +
							x2y2z1 * (Real)(        sxf *        syf *(1.0f - szf)) +
							x1y1z2 * (Real)((1.0f - sxf)*(1.0f - syf)*        szf ) +
							x2y1z2 * (Real)(        sxf *(1.0f - syf)*        szf ) +
							x1y2z2 * (Real)((1.0f - sxf)*        syf *        szf ) +
							x2y2z2 * (Real)(        sxf *        syf *        szf );

						PixelUtil::PackColor(accum, dst.pixFmt, pdst);

						pdst += dstelemsize;
					} // for

					pdst += dstelemsize*dst.getRowSkip();
				} // for

				pdst += dstelemsize*dst.getSliceSkip();
			} // for
		}
	};

	// float32 linear resampler, converts FLOAT32_RGB/FLOAT32_RGBA only.
	// avoids overhead of pixel unpack/repack function calls
	struct LinearResamplerFloat32
	{
		static void Scale(const PixelBox& src, const PixelBox& dst)
		{
			ui32 srcchannels = PixelUtil::GetPixelSize(src.pixFmt) / sizeof(float);
			ui32 dstchannels = PixelUtil::GetPixelSize(dst.pixFmt) / sizeof(float);
			// assert(srcchannels == 3 || srcchannels == 4);
			// assert(dstchannels == 3 || dstchannels == 4);

			// srcdata stays at beginning, pdst is a moving pointer
			float* srcdata = (float*)src.pData;
			float* pdst = (float*)dst.pData;

			// sx_48,sy_48,sz_48 represent current position in source
			// using 16/48-bit fixed precision, incremented by steps
			ui64 stepx = ((ui64)src.getWidth() << 48) / dst.getWidth();
			ui64 stepy = ((ui64)src.getHeight() << 48) / dst.getHeight();
			ui64 stepz = ((ui64)src.getDepth() << 48) / dst.getDepth();

			// temp is 16/16 bit fixed precision, used to adjust a source
			// coordinate (x, y, or z) backwards by half a pixel so that the
			// integer bits represent the first sample (eg, sx1) and the
			// fractional bits are the blend weight of the second sample
			ui32 temp;

			// note: ((stepz>>1) - 1) is an extra half-step increment to adjust
			// for the center of the destination pixel, not the top-left corner
			ui64 sz_48 = (stepz >> 1) - 1;
			for(ui32 z = dst.front; z < dst.back; z++, sz_48+=stepz)
			{
				temp = static_cast<ui32>(sz_48 >> 32);
				temp = (temp > 0x8000)? temp - 0x8000 : 0;
				ui32 sz1 = temp >> 16;				 // src z, sample #1
				ui32 sz2 = Math::Min(sz1+1,src.getDepth()-1);// src z, sample #2
				float szf = (temp & 0xFFFF) / 65536.f; // weight of sample #2

				ui64 sy_48 = (stepy >> 1) - 1;
				for(ui32 y = dst.top; y < dst.bottom; ++y, sy_48 += stepy)
				{
					temp = static_cast<ui32>(sy_48 >> 32);
					temp = (temp > 0x8000)? temp - 0x8000 : 0;
					ui32 sy1 = temp >> 16;					// src y #1
					ui32 sy2 = Math::Min(sy1+1, src.getHeight()-1);// src y #2
					float syf = (temp & 0xFFFF) / 65536.f; // weight of #2

					ui64 sx_48 = (stepx >> 1) - 1;
					for(ui32 x = dst.left; x < dst.right; ++x, sx_48 += stepx)
					{
						temp = static_cast<unsigned int>(sx_48 >> 32);
						temp = (temp > 0x8000)? temp - 0x8000 : 0;
						ui32 sx1 = temp >> 16;					// src x #1
						ui32 sx2 = Math::Min(sx1+1,src.getWidth()-1);// src x #2
						float sxf = (temp & 0xFFFF) / 65536.f; // weight of #2

						// process R,G,B,A simultaneously for cache coherence?
						float accum[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

#define ACCUM3(x, y, z, factor) { float f = factor;	ui32 off = (x+y*src.rowPitch+z*src.slicePitch)*srcchannels; accum[0]+=srcdata[off+0]*f; accum[1]+=srcdata[off+1]*f;	accum[2]+=srcdata[off+2]*f; }

#define ACCUM4(x, y, z, factor) { float f = factor;	ui32 off = (x+y*src.rowPitch+z*src.slicePitch)*srcchannels; accum[0]+=srcdata[off+0]*f; accum[1]+=srcdata[off+1]*f; accum[2]+=srcdata[off+2]*f; accum[3]+=srcdata[off+3]*f; }

						if(srcchannels == 3 || dstchannels == 3) 
						{
							// RGB, no alpha
							ACCUM3(sx1,sy1,sz1,(1.0f-sxf)*(1.0f-syf)*(1.0f-szf));
							ACCUM3(sx2,sy1,sz1,      sxf *(1.0f-syf)*(1.0f-szf));
							ACCUM3(sx1,sy2,sz1,(1.0f-sxf)*      syf *(1.0f-szf));
							ACCUM3(sx2,sy2,sz1,      sxf *      syf *(1.0f-szf));
							ACCUM3(sx1,sy1,sz2,(1.0f-sxf)*(1.0f-syf)*      szf );
							ACCUM3(sx2,sy1,sz2,      sxf *(1.0f-syf)*      szf );
							ACCUM3(sx1,sy2,sz2,(1.0f-sxf)*      syf *      szf );
							ACCUM3(sx2,sy2,sz2,      sxf *      syf *      szf );
							accum[3] = 1.0f;
						} 
						else 
						{
							// RGBA
							ACCUM4(sx1,sy1,sz1,(1.0f-sxf)*(1.0f-syf)*(1.0f-szf));
							ACCUM4(sx2,sy1,sz1,      sxf *(1.0f-syf)*(1.0f-szf));
							ACCUM4(sx1,sy2,sz1,(1.0f-sxf)*      syf *(1.0f-szf));
							ACCUM4(sx2,sy2,sz1,      sxf *      syf *(1.0f-szf));
							ACCUM4(sx1,sy1,sz2,(1.0f-sxf)*(1.0f-syf)*      szf );
							ACCUM4(sx2,sy1,sz2,      sxf *(1.0f-syf)*      szf );
							ACCUM4(sx1,sy2,sz2,(1.0f-sxf)*      syf *      szf );
							ACCUM4(sx2,sy2,sz2,      sxf *      syf *      szf );
						}

						memcpy(pdst, accum, sizeof(float)*dstchannels);
#undef ACCUM3
#undef ACCUM4

						pdst += dstchannels;
					}
					pdst += dstchannels*dst.getRowSkip();
				}
				pdst += dstchannels*dst.getSliceSkip();
			}
		}
	};



	// byte linear resampler, does not do any format conversions.
	// only handles pixel formats that use 1 byte per color channel.
	// 2D only; punts 3D pixelboxes to default LinearResampler (slow).
	// templated on bytes-per-pixel to allow compiler optimizations, such
	// as unrolling loops and replacing multiplies with bitshifts
	template<ui32 channels> struct LinearResamplerByte
	{
		static void Scale(const PixelBox &src, const PixelBox &dst)
		{
			// assert(src.format == dst.format);

			// only optimized for 2D
			if(src.getDepth() > 1 || dst.getDepth() > 1)
			{
				LinearResampler::Scale(src, dst);
				return;
			}

			// srcdata stays at beginning of slice, pdst is a moving pointer
			Byte* srcdata = (Byte*)src.pData;
			Byte* pdst = (Byte*)dst.pData;

			// sx_48,sy_48 represent current position in source
			// using 16/48-bit fixed precision, incremented by steps
			ui64 stepx = ((ui64)src.getWidth() << 48) / dst.getWidth();
			ui64 stepy = ((ui64)src.getHeight() << 48) / dst.getHeight();

			// bottom 28 bits of temp are 16/12 bit fixed precision, used to
			// adjust a source coordinate backwards by half a pixel so that the
			// integer bits represent the first sample (eg, sx1) and the
			// fractional bits are the blend weight of the second sample
			ui32 temp;

			ui64 sy_48 = (stepy >> 1) - 1;
			for(ui32 y = dst.top; y < dst.bottom; ++y, sy_48 += stepy)
			{
				temp = static_cast<ui32>(sy_48 >> 36);
				temp = (temp > 0x800)? temp - 0x800: 0;
				ui32 syf = temp & 0xFFF;
				ui32 sy1 = temp >> 12;
				ui32 sy2 = Math::Min(sy1+1, src.bottom-src.top-1);
				ui32 syoff1 = sy1 * src.rowPitch;
				ui32 syoff2 = sy2 * src.rowPitch;

				ui64 sx_48 = (stepx >> 1) - 1;
				for(ui32 x = dst.left; x < dst.right; ++x, sx_48 += stepx)
				{
					temp = static_cast<ui32>(sx_48 >> 36);
					temp = (temp > 0x800)? temp - 0x800 : 0;
					unsigned int sxf = temp & 0xFFF;
					ui32 sx1 = temp >> 12;
					ui32 sx2 = Math::Min(sx1+1, src.right-src.left-1);

					ui32 sxfsyf = sxf*syf;
					for(ui32 k = 0; k < channels; ++k)
					{
						ui32 accum =
							srcdata[(sx1 + syoff1)*channels+k]*(0x1000000-(sxf<<12)-(syf<<12)+sxfsyf) +
							srcdata[(sx2 + syoff1)*channels+k]*((sxf<<12)-sxfsyf) +
							srcdata[(sx1 + syoff2)*channels+k]*((syf<<12)-sxfsyf) +
							srcdata[(sx2 + syoff2)*channels+k]*sxfsyf;
						// accum is computed using 8/24-bit fixed-point math
						// (maximum is 0xFF000000; rounding will not cause overflow)
						*pdst++ = static_cast<Byte>((accum + 0x800000) >> 24);
					}
				}

				pdst += channels*dst.getRowSkip();
			}
		}
	};
}
