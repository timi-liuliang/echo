#include "engine/core/log/Log.h"
#include "engine/core/io/DataStream.h"
#include "render/ImageCodec.h"
#include <freeimage/FreeImage.h>

#ifdef _WINDOWS_
#undef _WINDOWS_
#endif


#if (ECHO_ENDIAN == ECHO_ENDIAN_BIG)
#	define FREEIMAGE_BIGENDIAN
#endif

// freeimage 3.9.1~3.11.0 interoperability fix
#ifndef FREEIMAGE_COLORORDER
// we have freeimage 3.9.1, define these symbols in such way as 3.9.1 really work (do not use 3.11.0 definition, as color order was changed between these two versions on Apple systems)
#	define FREEIMAGE_COLORORDER_BGR	0
#	define FREEIMAGE_COLORORDER_RGB	1
#	if defined(FREEIMAGE_BIGENDIAN)
#		define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_RGB
#	else
#		define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_BGR
#	endif
#endif

#if defined(ECHO_PLATFORM_MAC_IOS)
#	ifdef FREEIMAGE_COLORORDER
#		undef FREEIMAGE_COLORORDER
#	endif

#	define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_RGB
#endif

namespace Echo
{

	FREE_IMAGE_FORMAT ConvertToFIImageFormat(ImageFormat imgFmt)
	{
		switch(imgFmt)
		{
		case IF_BMP:			return FIF_BMP;
		case IF_JPG:			return FIF_JPEG;
		case IF_PNG:			return FIF_PNG;
		case IF_TGA:			return FIF_TARGA;
		default:				return FIF_UNKNOWN;
		}
	}

	ImageFormat ConvertToImageFormat(FREE_IMAGE_FORMAT fiImgFmt)
	{
		switch(fiImgFmt)
		{
		case FIF_BMP:			return IF_BMP;
		case FIF_JPEG:			return IF_JPG;
		case FIF_PNG:			return IF_PNG;
		case FIF_TARGA:			return IF_TGA;
		default:				return IF_UNKNOWN;
		}
	}

	FREE_IMAGE_TYPE GetFIPixelFormat(PixelFormat pixFmt)
	{
		switch(pixFmt)
		{
		case PF_R8_UNORM:				return FIT_BITMAP;
		case PF_R8_SNORM:				return FIT_BITMAP;
		case PF_R8_UINT:				return FIT_BITMAP;
		case PF_R8_SINT:				return FIT_BITMAP;

		case PF_A8_UNORM:				return FIT_BITMAP;

		case PF_RG8_UNORM:				return FIT_BITMAP;
		case PF_RG8_SNORM:				return FIT_BITMAP;
		case PF_RG8_UINT:				return FIT_BITMAP;
		case PF_RG8_SINT:				return FIT_BITMAP;

		case PF_RGB8_UNORM:				return FIT_BITMAP;
		case PF_RGB8_SNORM:				return FIT_BITMAP;
		case PF_RGB8_UINT:				return FIT_BITMAP;
		case PF_RGB8_SINT:				return FIT_BITMAP;

		case PF_BGR8_UNORM:				return FIT_BITMAP;

		case PF_RGBA8_UNORM:			return FIT_BITMAP;
		case PF_RGBA8_SNORM:			return FIT_BITMAP;
		case PF_RGBA8_UINT:				return FIT_BITMAP;
		case PF_RGBA8_SINT:				return FIT_BITMAP;

		case PF_BGRA8_UNORM:			return FIT_BITMAP;

		case PF_R16_UNORM:				return FIT_UINT16;
		case PF_R16_SNORM:				return FIT_INT16;
		case PF_R16_UINT:				return FIT_UINT16;
		case PF_R16_SINT:				return FIT_INT16;
		case PF_R16_FLOAT:				return FIT_FLOAT;

		case PF_RG16_UNORM:				return FIT_RGB16;
		case PF_RG16_SNORM:				return FIT_RGB16;
		case PF_RG16_UINT:				return FIT_RGB16;
		case PF_RG16_SINT:				return FIT_RGB16;
		case PF_RG16_FLOAT:				return FIT_RGBF;

		case PF_RGB16_UNORM:			return FIT_RGB16;
		case PF_RGB16_SNORM:			return FIT_RGB16;
		case PF_RGB16_UINT:				return FIT_RGB16;
		case PF_RGB16_SINT:				return FIT_RGB16;
		case PF_RGB16_FLOAT:			return FIT_RGBF;

		case PF_RGBA16_UNORM:			return FIT_RGBA16;
		case PF_RGBA16_SNORM:			return FIT_RGBA16;
		case PF_RGBA16_UINT:			return FIT_RGBA16;
		case PF_RGBA16_SINT:			return FIT_RGBA16;
		case PF_RGBA16_FLOAT:			return FIT_RGBAF;

		case PF_R32_UNORM:				return FIT_UINT32;
		case PF_R32_SNORM:				return FIT_INT32;
		case PF_R32_UINT:				return FIT_UINT32;
		case PF_R32_SINT:				return FIT_INT32;
		case PF_R32_FLOAT:				return FIT_FLOAT;

		case PF_RG32_UNORM:				return FIT_UNKNOWN;
		case PF_RG32_SNORM:				return FIT_UNKNOWN;
		case PF_RG32_UINT:				return FIT_UNKNOWN;
		case PF_RG32_SINT:				return FIT_UNKNOWN;
		case PF_RG32_FLOAT:				return FIT_RGBF;

		case PF_RGB32_UNORM:			return FIT_UNKNOWN;
		case PF_RGB32_SNORM:			return FIT_UNKNOWN;
		case PF_RGB32_UINT:				return FIT_UNKNOWN;
		case PF_RGB32_SINT:				return FIT_UNKNOWN;
		case PF_RGB32_FLOAT:			return FIT_RGBF;

		case PF_RGBA32_UNORM:			return FIT_UNKNOWN;
		case PF_RGBA32_SNORM:			return FIT_UNKNOWN;
		case PF_RGBA32_UINT:			return FIT_UNKNOWN;
		case PF_RGBA32_SINT:			return FIT_UNKNOWN;
		case PF_RGBA32_FLOAT:			return FIT_RGBAF;

		case PF_D16_UNORM:				return FIT_UINT16;
		case PF_D24_UNORM_S8_UINT:		return FIT_UINT32;
		case PF_D32_FLOAT:				return FIT_FLOAT;

		default:						return FIT_UNKNOWN;
		};
	}

	void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char* message) 
	{
		// Callback method as required by FreeImage to report problems
		EchoLogError(message);
	}

	ImageCodec::ImageCodec(ImageFormat imgFmt)
		: m_imgFmt(imgFmt)
	{
	}

	ImageCodec::~ImageCodec()
	{
	}

	ImageFormat ImageCodec::getImageFormat() const
	{
		return m_imgFmt;
	}

	bool ImageCodec::codeToFile(ImageFormat imgFmt, const Buffer& buff, const Image::ImageInfo& imgInfo, const String& filename) const
	{
		// Set error handler
		FreeImage_SetOutputMessage(FreeImageErrorHandler);

		Buffer outBuff;
		if(!encode(buff, outBuff, imgInfo))
		{
			return false;
		}

		Byte* pData = NULL;
		outBuff.takeData(pData);

		FIBITMAP* pFiBitmap = (FIBITMAP*)pData;

		FREE_IMAGE_FORMAT fiImgFmt = ConvertToFIImageFormat(imgFmt);
		FreeImage_Save(fiImgFmt, pFiBitmap, filename.c_str());
		ECHO_UNUSED(pFiBitmap);

		FreeImage_Unload(pFiBitmap);

		return true;
	}

	bool ImageCodec::encode(const Buffer& inBuff, Buffer& outBuff, const Image::ImageInfo& imgInfo) const
	{
		return doEncode(m_imgFmt, inBuff, outBuff, imgInfo);
	}

	bool ImageCodec::decode(const Buffer& inBuff, Buffer& outBuff, Image::ImageInfo& imgInfo)
	{
		return doDecode(m_imgFmt, inBuff, outBuff, imgInfo);
	}

	DataStream* ImageCodec::decode(DataStream* inStream, Image::ImageInfo& imgInfo)
	{
		return doDecode(m_imgFmt, inStream, imgInfo);
	}

	ImageFormat ImageCodec::GetImageFormat(const String& filename)
	{
		FREE_IMAGE_FORMAT fiImgFmt;
		fiImgFmt = FreeImage_GetFileType(filename.c_str());
		return ConvertToImageFormat(fiImgFmt);
	}

	bool ImageCodec::doEncode(ImageFormat imgFmt, const Buffer& inBuff, Buffer& outBuff, const Image::ImageInfo& imgInfo) const
	{
		// The required format, which will adjust to the format
		// actually supported by FreeImage.
		PixelFormat requiredFormat = imgInfo.pixFmt;

		// determine the settings
		PixelFormat determiningFormat = imgInfo.pixFmt;

		//////////////////////////////////////////////////////////////////////////

		switch(determiningFormat)
		{
		case PF_RGB8_UNORM:
		case PF_RGB8_SNORM:
		case PF_RGB8_UINT:
		case PF_RGB8_SINT:

		case PF_BGR8_UNORM:

		case PF_RGBA8_UNORM:
		case PF_RGBA8_SNORM:
		case PF_RGBA8_UINT:
		case PF_RGBA8_SINT:

		case PF_BGRA8_UNORM:
			{
				// I'd like to be able to use r/g/b masks to get FreeImage to load the data
				// in it's existing format, but that doesn't work, FreeImage needs to have
				// data in RGB[A] (big endian) and BGR[A] (little endian), always.
				if (PixelUtil::HasAlpha(determiningFormat))
				{
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
					requiredFormat = PF_RGBA8_UNORM;
#else
					requiredFormat = PF_BGRA8_UNORM;
#endif
				}
				else
				{
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
					requiredFormat = PF_RGB8_UNORM;
#else
					requiredFormat = PF_BGR8_UNORM;
#endif
				}
			} break;

			// fall through
		case PF_R16_UNORM:
		case PF_R16_UINT:
			{
				requiredFormat = PF_R16_UNORM;
			} break;
		case PF_R16_SNORM:
		case PF_R16_SINT:
			{
				requiredFormat = PF_R16_SNORM;
			} break;

		case PF_RG16_UNORM:
		case PF_RG16_SNORM:
		case PF_RG16_UINT:
		case PF_RG16_SINT:

		case PF_RGB16_UNORM:
		case PF_RGB16_SNORM:
		case PF_RGB16_UINT:
		case PF_RGB16_SINT:
			{
				requiredFormat = PF_RGB16_UNORM;
			} break;

		case PF_RGBA16_UNORM:
		case PF_RGBA16_SNORM:
		case PF_RGBA16_UINT:
		case PF_RGBA16_SINT:
			{
				requiredFormat = PF_RGBA16_UNORM;
			} break;

		case PF_R32_UNORM:
		case PF_R32_UINT:
			{
				requiredFormat = PF_R32_UNORM;
			} break;
		case PF_R32_SNORM:
		case PF_R32_SINT:
			{
				requiredFormat = PF_R32_SNORM;
			} break;

		case PF_R16_FLOAT:
		case PF_R32_FLOAT:
			{
				requiredFormat = PF_R32_FLOAT;
			} break;

		case PF_RG16_FLOAT:
		case PF_RGB16_FLOAT:
		case PF_RG32_FLOAT:
		case PF_RGB32_FLOAT:
			{
				requiredFormat = PF_RGB32_FLOAT;
			} break;

		case PF_RGBA16_FLOAT:
		case PF_RGBA32_FLOAT:
			{
				requiredFormat = PF_RGBA32_FLOAT;
			} break;
		default: ;
		}

		//////////////////////////////////////////////////////////////////////////

		FREE_IMAGE_TYPE fiImgType = GetFIPixelFormat(requiredFormat);

		if(fiImgType == FIT_UNKNOWN)
		{
			EchoLogError("Don't support the pixel format [%s].", PixelUtil::GetPixelFormatName(requiredFormat).c_str());
			return false;
		}

		FREE_IMAGE_FORMAT fiImgFmt = ConvertToFIImageFormat(imgFmt);


		// Check support for this image type & bit depth
		if(!FreeImage_FIFSupportsExportType(fiImgFmt, fiImgType) || !FreeImage_FIFSupportsExportBPP(fiImgFmt, (int)PixelUtil::GetPixelBits(requiredFormat)))
		{
			// Ok, need to allocate a fallback
			// Only deal with RGBA -> RGB for now
			switch (requiredFormat)
			{
			case PF_RGBA8_UNORM:
				requiredFormat = PF_RGB8_UNORM;
				break;
			case PF_BGRA8_UNORM:
				requiredFormat = PF_BGR8_UNORM;
				break;
			default:
				break;
			};
		}

		const Byte* pSrcData = inBuff.getData();

		// Check BPP
		ui32 bpp = PixelUtil::GetPixelBits(requiredFormat);
		if(!FreeImage_FIFSupportsExportBPP(fiImgFmt, (int)bpp))
		{
			if(bpp == 32 && PixelUtil::HasAlpha(requiredFormat) && FreeImage_FIFSupportsExportBPP(fiImgFmt, 24))
			{
				// drop to 24 bit (lose alpha)
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
				requiredFormat = PF_RGB8_UNORM;
#else
				requiredFormat = PF_BGR8_UNORM;
#endif
				bpp = 24;
			}
			else if(bpp == 128 && PixelUtil::HasAlpha(requiredFormat) && FreeImage_FIFSupportsExportBPP(fiImgFmt, 96))
			{
				// drop to 96-bit floating point (lose alpha)
				requiredFormat = PF_RGB32_FLOAT;
			}
			else
			{
				EchoLogError(	"Don't support the image type ( ImageFormat [%s], BPP [%d], PixelFormat [%s] ).", 
					Image::GetImageFormatName(imgFmt).c_str(), 
					PixelUtil::GetPixelBits(requiredFormat), 
					PixelUtil::GetPixelFormatName(requiredFormat).c_str());
				return false;
			}
		}

		bool conversionRequired = false;

		PixelBox convBox(imgInfo.width, imgInfo.height, 1, requiredFormat);
		if(requiredFormat != imgInfo.pixFmt)
		{
			conversionRequired = true;
			// Allocate memory
			convBox.pData = ECHO_ALLOC_T(Byte, convBox.getConsecutiveSize());
			// perform conversion and reassign source

			Buffer memBuff(inBuff);
			PixelBox newSrc(imgInfo.width, imgInfo.height, 1, imgInfo.pixFmt, memBuff.getData());
			PixelUtil::BulkPixelConversion(newSrc, convBox);
			pSrcData = static_cast<Byte*>(convBox.pData);
		}

		FIBITMAP *pBitmap = FreeImage_AllocateT(fiImgType, static_cast<int>(imgInfo.width), static_cast<int>(imgInfo.height), bpp);
		if(!pBitmap)
		{
			if (conversionRequired)
				ECHO_FREE(convBox.pData);

			EchoLogError("FreeImage_AllocateT failed - possibly out of memory.");
			EchoAssertX(false, "FreeImage_AllocateT failed - possibly out of memory.");

			return false;
		}

		if((requiredFormat == PF_R8_UNORM) ||
			(requiredFormat == PF_R8_SNORM) ||
			(requiredFormat == PF_R8_UINT) ||
			(requiredFormat == PF_R8_SINT) ||
			(requiredFormat == PF_A8_UNORM))
		{
			// Must explicitly tell FreeImage that this is greyscale by setting a "grey" palette
			// (otherwise it will save as a normal RGB palettized image)
			FIBITMAP *pTemp = FreeImage_ConvertToGreyscale(pBitmap);
			FreeImage_Unload(pBitmap);
			pBitmap = pTemp;
		}

		ui32 dstPitch = FreeImage_GetPitch(pBitmap);
		ui32 srcPitch = imgInfo.width * PixelUtil::GetPixelSize(requiredFormat);

		// Copy data, invert scanlines and respect FreeImage pitch
		Byte* pDst = FreeImage_GetBits(pBitmap);
		ui32 dibSize = FreeImage_GetDIBSize(pBitmap);
		for(ui32 y = 0; y < imgInfo.height; ++y)
		{
			const Byte* pSrc = pSrcData + (imgInfo.height - y - 1) * srcPitch;
			memcpy(pDst, pSrc, srcPitch);
			pDst += dstPitch;
		}

		if (conversionRequired)
		{
			// delete temporary conversion area
			ECHO_FREE(convBox.pData);
		}

		//ui32 buffSize = srcPitch * imgInfo.height;
		outBuff.set(dibSize, (void*)pBitmap);

		return true;
	}

	DataStream* ImageCodec::doDecode(ImageFormat imgFmt, DataStream* inStream, Image::ImageInfo& imgInfo)
	{
		MemoryDataStream memStream(inStream, true);

		FIMEMORY* fiMem = FreeImage_OpenMemory(memStream.getPtr(), static_cast<Dword>(memStream.size()));
		FREE_IMAGE_FORMAT fiImgFmt = ConvertToFIImageFormat(imgFmt);

		FIBITMAP* fiBitmap = FreeImage_LoadFromMemory(fiImgFmt, fiMem);
		if (!fiBitmap)
		{
			EchoLogError("Error decoding image format [%s].", Image::GetImageFormatName(imgFmt).c_str());
			return NULL;
		}

		Image::ImageInfo tempImgInfo;
		tempImgInfo.depth = 1;		 // only 2D formats handled by this codec
		tempImgInfo.width = FreeImage_GetWidth(fiBitmap);
		tempImgInfo.height = FreeImage_GetHeight(fiBitmap);
		tempImgInfo.numMipmaps = 0;	 // no mipmaps in non-DDS 
		tempImgInfo.flags = 0;

		FREE_IMAGE_TYPE fiType = FreeImage_GetImageType(fiBitmap);
		FREE_IMAGE_COLOR_TYPE fiColorType = FreeImage_GetColorType(fiBitmap);
		ui32 bpp = FreeImage_GetBPP(fiBitmap);
		Byte* pSrcData = FreeImage_GetBits(fiBitmap);

		switch(fiType)
		{
		case FIT_UNKNOWN:
		case FIT_COMPLEX:
		case FIT_DOUBLE:
		default:
			{
				EchoLogError("Unknown or unsupported image format [%s].", Image::GetImageFormatName(imgFmt).c_str());
				return NULL;
			}
			break;
		case FIT_BITMAP:
			{
				// Standard image type
				// Perform any color conversions for greyscale
				if (fiColorType == FIC_MINISWHITE || fiColorType == FIC_MINISBLACK)
				{
					FIBITMAP* pNewBitmap = FreeImage_ConvertToGreyscale(fiBitmap);
					// free old bitmap and replace
					FreeImage_Unload(fiBitmap);
					fiBitmap = pNewBitmap;
					// get new formats
					bpp = FreeImage_GetBPP(fiBitmap);
				}
				// Perform any color conversions for RGB
				else if (bpp < 8 || fiColorType == FIC_PALETTE || fiColorType == FIC_CMYK)
				{
					FIBITMAP* pNewBitmap =  NULL;	
					if (FreeImage_IsTransparent(fiBitmap))
					{
						// convert to 32 bit to preserve the transparency 
						// (the alpha byte will be 0 if pixel is transparent)
						pNewBitmap = FreeImage_ConvertTo32Bits(fiBitmap);
					}
					else
					{
						// no transparency - only 3 bytes are needed
						pNewBitmap = FreeImage_ConvertTo24Bits(fiBitmap);
					}

					// free old bitmap and replace
					FreeImage_Unload(fiBitmap);
					fiBitmap = pNewBitmap;
					// get new formats
					bpp = FreeImage_GetBPP(fiBitmap);
				}

				// by this stage, 8-bit is greyscale, 16/24/32 bit are RGB[A]
				switch(bpp)
				{
				case 8:
					tempImgInfo.pixFmt = PF_R8_UNORM;
					break;
				case 16:
					tempImgInfo.pixFmt = PF_R16_UNORM;
					break;
				case 24:
					{
						// FreeImage differs per platform
						// PF_BYTE_BGR[A] for little endian (== PF_ARGB native)
						// PF_BYTE_RGB[A] for big endian (== PF_RGBA native)
						// 格式转换
					#if ECHO_ENDIAN==ECHO_ENDIAN_LITTLE
						tempImgInfo.pixFmt = PF_BGR8_UNORM;
					#else
						tempImgInfo.pixFmt = PF_RGB8_UNORM;
					#endif
					}
					break;

				case 32:
					{
						// 格式转换
					#if ECHO_ENDIAN==ECHO_ENDIAN_LITTLE
						tempImgInfo.pixFmt = PF_BGRA8_UNORM;
					#else
						tempImgInfo.pixFmt = PF_RGBA8_UNORM;
					#endif
					}
					break;
				} // switch

			} break;

		case FIT_UINT16:
			tempImgInfo.pixFmt = PF_R16_UINT;
			break;
		case FIT_INT16:
			tempImgInfo.pixFmt = PF_R16_SINT;
			break;
		case FIT_FLOAT:
			// Single-component floating point data
			tempImgInfo.pixFmt = PF_R32_FLOAT;
			break;
		case FIT_RGB16:
			tempImgInfo.pixFmt = PF_RGB16_UNORM;
			break;
		case FIT_RGBA16:
			tempImgInfo.pixFmt = PF_RGBA16_UNORM;
			break;
		case FIT_RGBF:
			tempImgInfo.pixFmt = PF_RGB32_FLOAT;
			break;
		case FIT_RGBAF:
			tempImgInfo.pixFmt = PF_RGBA32_FLOAT;
			break;
		} // switch

		ui32 srcPitch = FreeImage_GetPitch(fiBitmap);

		// Final data - invert image and trim pitch at the same time
		ui32 dstPitch = tempImgInfo.width * PixelUtil::GetPixelSize(tempImgInfo.pixFmt);
		tempImgInfo.size = dstPitch * tempImgInfo.height;

		imgInfo = tempImgInfo;

		// Bind output buffer
		MemoryDataStream* pResult = EchoNew(MemoryDataStream(tempImgInfo.size, false));

		Byte* pSrc;
		Byte* pDst = pResult->getPtr();
		for(ui32 y = 0; y < imgInfo.height; ++y)
		{
			pSrc = pSrcData + (imgInfo.height - y - 1) * srcPitch;
			memcpy(pDst, pSrc, dstPitch);
			pDst += dstPitch;
		}

		// 格式转换
		pDst = pResult->getPtr();
		if (tempImgInfo.pixFmt == PF_BGR8_UNORM)
		{
			size_t pixNum = imgInfo.width*imgInfo.height;
			for (size_t pix = 0; pix < pixNum; pix++)
			{
				int baseIdx = pix * 3;
				Echo::Math::Swap<Byte>(pDst[baseIdx + FI_RGBA_RED], pDst[baseIdx + FI_RGBA_BLUE]);
			}
			imgInfo.pixFmt = PF_RGB8_UNORM;
		}
		else if (tempImgInfo.pixFmt == PF_BGRA8_UNORM)
		{
			size_t pixNum = imgInfo.width*imgInfo.height;
			for (size_t pix = 0; pix < pixNum; pix++)
			{
				int baseIdx = pix * 4;
				Echo::Math::Swap<Byte>(pDst[baseIdx + FI_RGBA_RED], pDst[baseIdx + FI_RGBA_BLUE]);
			}

			imgInfo.pixFmt = PF_RGBA8_UNORM;
		}

		FreeImage_Unload(fiBitmap);
		FreeImage_CloseMemory(fiMem);

		return pResult;
	}

	bool ImageCodec::doDecode(ImageFormat imgFmt, const Buffer& inBuff, Buffer& outBuff, Image::ImageInfo& imgInfo)
	{
		// Buffer stream into memory (TODO: override IO functions instead?)
		Buffer memBuff(inBuff);

		FIMEMORY* fiMem = FreeImage_OpenMemory(memBuff.getData(), static_cast<Dword>(memBuff.getSize()));
		//FREE_IMAGE_FORMAT fiImgFmt = FreeImage_GetFileTypeFromMemory(fiMem);
		FREE_IMAGE_FORMAT fiImgFmt = ConvertToFIImageFormat(imgFmt);

		FIBITMAP* fiBitmap = FreeImage_LoadFromMemory(fiImgFmt, fiMem);
		if (!fiBitmap)
		{
			EchoLogError("Error decoding image format [%s].", Image::GetImageFormatName(imgFmt).c_str());
			return false;
		}

		Image::ImageInfo tempImgInfo;

		tempImgInfo.depth = 1;		 // only 2D formats handled by this codec
		tempImgInfo.width = FreeImage_GetWidth(fiBitmap);
		tempImgInfo.height = FreeImage_GetHeight(fiBitmap);
		tempImgInfo.numMipmaps = 0;	 // no mipmaps in non-DDS 
		tempImgInfo.flags = 0;

		// Must derive format first, this may perform conversions
		FREE_IMAGE_TYPE fiType = FreeImage_GetImageType(fiBitmap);
		FREE_IMAGE_COLOR_TYPE fiColorType = FreeImage_GetColorType(fiBitmap);
		ui32 bpp = FreeImage_GetBPP(fiBitmap);
		Byte* pSrcData = FreeImage_GetBits(fiBitmap);
		switch(fiType)
		{
		case FIT_UNKNOWN:
		case FIT_COMPLEX:
		case FIT_DOUBLE:
		default:
			{
				EchoLogError("Unknown or unsupported image format [%s].", Image::GetImageFormatName(imgFmt).c_str());
				return false;
			}
			break;
		case FIT_BITMAP:
			{
				// Standard image type
				// Perform any color conversions for greyscale
				if(fiColorType == FIC_MINISWHITE || fiColorType == FIC_MINISBLACK)
				{
					FIBITMAP* pNewBitmap = FreeImage_ConvertToGreyscale(fiBitmap);
					// free old bitmap and replace
					FreeImage_Unload(fiBitmap);
					fiBitmap = pNewBitmap;
					// get new formats
					bpp = FreeImage_GetBPP(fiBitmap);
				}
				// Perform any color conversions for RGB
				else if(bpp < 8 || fiColorType == FIC_PALETTE || fiColorType == FIC_CMYK)
				{
					FIBITMAP* pNewBitmap =  NULL;	
					if(FreeImage_IsTransparent(fiBitmap))
					{
						// convert to 32 bit to preserve the transparency 
						// (the alpha byte will be 0 if pixel is transparent)
						pNewBitmap = FreeImage_ConvertTo32Bits(fiBitmap);
					}
					else
					{
						// no transparency - only 3 bytes are needed
						pNewBitmap = FreeImage_ConvertTo24Bits(fiBitmap);
					}

					// free old bitmap and replace
					FreeImage_Unload(fiBitmap);
					fiBitmap = pNewBitmap;
					// get new formats
					bpp = FreeImage_GetBPP(fiBitmap);
					pSrcData = FreeImage_GetBits(fiBitmap);
				}

				// by this stage, 8-bit is greyscale, 16/24/32 bit are RGB[A]
				switch(bpp)
				{
				case 8:
					tempImgInfo.pixFmt = PF_R8_UNORM;
					break;
				case 16:
					tempImgInfo.pixFmt = PF_R16_UNORM;
					break;
				case 24:
					{
					// FreeImage differs per platform
					// PF_BYTE_BGR[A] for little endian (== PF_ARGB native)
					// PF_BYTE_RGB[A] for big endian (== PF_RGBA native)
					// 格式转换
				#if ECHO_ENDIAN==ECHO_ENDIAN_LITTLE
						tempImgInfo.pixFmt = PF_BGR8_UNORM;
				#else
						tempImgInfo.pixFmt = PF_RGB8_UNORM;
				#endif
					}
					break;

				case 32:
					{
					// 格式转换
				#if ECHO_ENDIAN==ECHO_ENDIAN_LITTLE
						tempImgInfo.pixFmt = PF_BGRA8_UNORM;
				#else
						tempImgInfo.pixFmt = PF_RGBA8_UNORM;
				#endif
					}
					break;
				} // switch

			} break;

		case FIT_UINT16:
			tempImgInfo.pixFmt = PF_R16_UINT;
			break;
		case FIT_INT16:
			tempImgInfo.pixFmt = PF_R16_SINT;
			break;
		case FIT_FLOAT:
			// Single-component floating point data
			tempImgInfo.pixFmt = PF_R32_FLOAT;
			break;
		case FIT_RGB16:
			tempImgInfo.pixFmt = PF_RGB16_UNORM;
			break;
		case FIT_RGBA16:
			tempImgInfo.pixFmt = PF_RGBA16_UNORM;
			break;
		case FIT_RGBF:
			tempImgInfo.pixFmt = PF_RGB32_FLOAT;
			break;
		case FIT_RGBAF:
			tempImgInfo.pixFmt = PF_RGBA32_FLOAT;
			break;
		} // switch

		ui32 srcPitch = FreeImage_GetPitch(fiBitmap);

		// Final data - invert image and trim pitch at the same time
		ui32 dstPitch = tempImgInfo.width * PixelUtil::GetPixelSize(tempImgInfo.pixFmt);
		tempImgInfo.size = dstPitch * tempImgInfo.height;

		imgInfo = tempImgInfo;

		// Bind output buffer
		outBuff.allocate(tempImgInfo.size);

		Byte* pSrc;
		Byte* pDst = outBuff.getData();
		for(ui32 y = 0; y < imgInfo.height; ++y)
		{
			pSrc = pSrcData + (imgInfo.height - y - 1) * srcPitch;
			memcpy(pDst, pSrc, dstPitch);
			pDst += dstPitch;
		}

		// 格式转换
		pDst = outBuff.getData();
		if (tempImgInfo.pixFmt == PF_BGR8_UNORM)
		{
			size_t pixNum = imgInfo.width*imgInfo.height;
			for (size_t pix = 0; pix < pixNum; pix++)
			{
				int baseIdx = pix * 3;
				Echo::Math::Swap<Byte>(pDst[baseIdx + FI_RGBA_RED], pDst[baseIdx + FI_RGBA_BLUE]);
			}
			imgInfo.pixFmt = PF_RGB8_UNORM;
		}
		else if (tempImgInfo.pixFmt == PF_BGRA8_UNORM)
		{
			size_t pixNum = imgInfo.width*imgInfo.height;
			for (size_t pix = 0; pix < pixNum; pix++)
			{
				int baseIdx = pix * 4;
				Echo::Math::Swap<Byte>(pDst[baseIdx + FI_RGBA_RED], pDst[baseIdx + FI_RGBA_BLUE]);
			}

			imgInfo.pixFmt = PF_RGBA8_UNORM;
		}

		FreeImage_Unload(fiBitmap);
		FreeImage_CloseMemory(fiMem);

		return true;
	}
}
