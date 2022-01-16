#include  "core/util/PathUtil.h"
#include "engine/core/log/Log.h"
#include "engine/core/io/stream/MemoryDataStream.h"
#include "image_codec.h"
#include "stb/stb_image.h"

#ifdef _WINDOWS_
#undef _WINDOWS_
#endif

#if (ECHO_ENDIAN == ECHO_ENDIAN_BIG)
#	define FREEIMAGE_BIGENDIAN
#endif

// freeimage 3.9.1~3.11.0 interoperability fix
#ifndef FREEIMAGE_COLORORDER
// we have freeimage 3.9.1, define these symbols in such way as 3.9.1 really work (do not use 3.11.0 definition, as color order was changed between these two versions on Apple systems)
#	define FREEIMAGE_COLORORDER_BGR    0
#	define FREEIMAGE_COLORORDER_RGB    1
#	if defined(FREEIMAGE_BIGENDIAN)
#		define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_RGB
#	else
#		define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_BGR
#	endif
#endif

#if defined(ECHO_PLATFORM_IOS)
#	ifdef FREEIMAGE_COLORORDER
#		undef FREEIMAGE_COLORORDER
#	endif

#	define FREEIMAGE_COLORORDER FREEIMAGE_COLORORDER_RGB
#endif

namespace Echo
{
    stbi_uc* decodeImpl(stbi_uc const *buffer, int len,Image::ImageInfo &imgInfo)
    {
        int width = 0;
        int height = 0;
        int channels_in_file = 0;

        if (stbi_is_16_bit_from_memory(buffer, len))
        {
            int depth = 2;

            stbi_us* pixels = stbi_load_16_from_memory(buffer, len, &width, &height, &channels_in_file, 0);
            if (pixels) 
            {
                imgInfo.depth = depth;         // only 2D formats handled by this codec
                imgInfo.width = width;
                imgInfo.height = height;
                imgInfo.size = channels_in_file * width * height * depth;
                imgInfo.numMipmaps = 0;     // no mipmaps in non-DDS
                imgInfo.flags = 0;

                switch (channels_in_file)
                {
                case 1:imgInfo.pixFmt = PF_R16_UNORM; break;
                case 3:imgInfo.pixFmt = PF_RGB16_UNORM; break;
                case 4:imgInfo.pixFmt = PF_RGBA16_UNORM; break;
                default: {
                    free(pixels);
                    EchoLogError("unsupported pixel format");
                    return nullptr;
                }
                }
            }

            return (stbi_uc*)pixels;
        }
        else
        {
            stbi_uc* pixels = stbi_load_from_memory(buffer, len, &width, &height, &channels_in_file, 0);
            if (pixels) 
            {
                imgInfo.depth = 1;         // only 2D formats handled by this codec
                imgInfo.width = width;
                imgInfo.height = height;
                imgInfo.size = channels_in_file * width * height;
                imgInfo.numMipmaps = 0;     // no mipmaps in non-DDS
                imgInfo.flags = 0;

                switch (channels_in_file)
                {
                case 1:imgInfo.pixFmt = PF_R8_UNORM; break;
                case 3:imgInfo.pixFmt = PF_RGB8_UNORM; break;
                case 4:imgInfo.pixFmt = PF_RGBA8_UNORM; break;
                default: {
                    free(pixels);
                    EchoLogError("unsupported pixel format");
                    return nullptr;
                }
                }
            }

            return pixels;
        }
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

    bool ImageCodec::codeToFile(ImageFormat imgFmt, const Buffer &buff, const Image::ImageInfo &imgInfo, const String &filename) const 
    {
        Buffer outBuff;
        if (!encode(buff, outBuff, imgInfo)) 
            return false;

        Byte *pData = NULL;
        outBuff.takeData(pData);

        return true;
    }

    bool ImageCodec::encode(const Buffer &inBuff, Buffer &outBuff, const Image::ImageInfo &imgInfo) const 
    {
        return doEncode(m_imgFmt, inBuff, outBuff, imgInfo);
    }

    bool ImageCodec::decode(const Buffer &inBuff, Buffer &outBuff, Image::ImageInfo &imgInfo) 
    {
        return doDecode(m_imgFmt, inBuff, outBuff, imgInfo);
    }

    DataStream *ImageCodec::decode(DataStream *inStream, Image::ImageInfo &imgInfo) 
    {
        return doDecode(m_imgFmt, inStream, imgInfo);
    }

    ImageFormat ImageCodec::GetImageFormat(const String &filename) 
    {
//        FREE_IMAGE_FORMAT fiImgFmt;
//        fiImgFmt = FreeImage_GetFileType(filename.c_str());
        return Image::GetImageFormatByExt(PathUtil::GetFileExt(filename));
    }

    bool ImageCodec::doEncode(ImageFormat imgFmt, const Buffer &inBuff, Buffer &outBuff,
                              const Image::ImageInfo &imgInfo) const {
        // The required format, which will adjust to the format
        // actually supported by FreeImage.
        PixelFormat requiredFormat = imgInfo.pixFmt;

        // determine the settings
        PixelFormat determiningFormat = imgInfo.pixFmt;

        //////////////////////////////////////////////////////////////////////////

        switch (determiningFormat) {
            case PF_RGB8_UNORM:
            case PF_RGB8_SNORM:
            case PF_RGB8_UINT:
            case PF_RGB8_SINT:

            case PF_BGR8_UNORM:

            case PF_RGBA8_UNORM:
            case PF_RGBA8_SNORM:
            case PF_RGBA8_UINT:
            case PF_RGBA8_SINT:

            case PF_BGRA8_UNORM: {
                // I'd like to be able to use r/g/b masks to get FreeImage to load the data
                // in it's existing format, but that doesn't work, FreeImage needs to have
                // data in RGB[A] (big endian) and BGR[A] (little endian), always.
                if (PixelUtil::HasAlpha(determiningFormat)) {
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
                    requiredFormat = PF_RGBA8_UNORM;
#else
                    requiredFormat = PF_BGRA8_UNORM;
#endif
                } else {
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
                    requiredFormat = PF_RGB8_UNORM;
#else
                    requiredFormat = PF_BGR8_UNORM;
#endif
                }
            }
                break;

                // fall through
            case PF_R16_UNORM:
            case PF_R16_UINT: {
                requiredFormat = PF_R16_UNORM;
            }
                break;
            case PF_R16_SNORM:
            case PF_R16_SINT: {
                requiredFormat = PF_R16_SNORM;
            }
                break;

            case PF_RG16_UNORM:
            case PF_RG16_SNORM:
            case PF_RG16_UINT:
            case PF_RG16_SINT:

            case PF_RGB16_UNORM:
            case PF_RGB16_SNORM:
            case PF_RGB16_UINT:
            case PF_RGB16_SINT: {
                requiredFormat = PF_RGB16_UNORM;
            }
                break;

            case PF_RGBA16_UNORM:
            case PF_RGBA16_SNORM:
            case PF_RGBA16_UINT:
            case PF_RGBA16_SINT: {
                requiredFormat = PF_RGBA16_UNORM;
            }
                break;

            case PF_R32_UNORM:
            case PF_R32_UINT: {
                requiredFormat = PF_R32_UNORM;
            }
                break;
            case PF_R32_SNORM:
            case PF_R32_SINT: {
                requiredFormat = PF_R32_SNORM;
            }
                break;

            case PF_R16_FLOAT:
            case PF_R32_FLOAT: {
                requiredFormat = PF_R32_FLOAT;
            }
                break;

            case PF_RG16_FLOAT:
            case PF_RGB16_FLOAT:
            case PF_RG32_FLOAT:
            case PF_RGB32_FLOAT: {
                requiredFormat = PF_RGB32_FLOAT;
            }
                break;

            case PF_RGBA16_FLOAT:
            case PF_RGBA32_FLOAT: {
                requiredFormat = PF_RGBA32_FLOAT;
            }
                break;
            default:;
        }

        //////////////////////////////////////////////////////////////////////////

//        FREE_IMAGE_TYPE fiImgType = GetFIPixelFormat(requiredFormat);
//
//        if (fiImgType == FIT_UNKNOWN) {
//            EchoLogError("Don't support the pixel format [%s].", PixelUtil::GetPixelFormatName(requiredFormat).c_str());
//            return false;
//        }
//
//        FREE_IMAGE_FORMAT fiImgFmt = ConvertToFIImageFormat(imgFmt);
//
//
//        // Check support for this image type & bit depth
//        if (!FreeImage_FIFSupportsExportType(fiImgFmt, fiImgType) ||
//            !FreeImage_FIFSupportsExportBPP(fiImgFmt, (int) PixelUtil::GetPixelBits(requiredFormat))) {
//            // Ok, need to allocate a fallback
//            // Only deal with RGBA -> RGB for now
//            switch (requiredFormat) {
//                case PF_RGBA8_UNORM:
//                    requiredFormat = PF_RGB8_UNORM;
//                    break;
//                case PF_BGRA8_UNORM:
//                    requiredFormat = PF_BGR8_UNORM;
//                    break;
//                default:
//                    break;
//            };
//        }
//
//        const Byte *pSrcData = inBuff.getData();
//
//        // Check BPP
//        ui32 bpp = PixelUtil::GetPixelBits(requiredFormat);
//        if (!FreeImage_FIFSupportsExportBPP(fiImgFmt, (int) bpp)) {
//            if (bpp == 32 && PixelUtil::HasAlpha(requiredFormat) && FreeImage_FIFSupportsExportBPP(fiImgFmt, 24)) {
//                // drop to 24 bit (lose alpha)
//#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
//                requiredFormat = PF_RGB8_UNORM;
//#else
//                requiredFormat = PF_BGR8_UNORM;
//#endif
//                bpp = 24;
//            } else if (bpp == 128 && PixelUtil::HasAlpha(requiredFormat) &&
//                       FreeImage_FIFSupportsExportBPP(fiImgFmt, 96)) {
//                // drop to 96-bit floating point (lose alpha)
//                requiredFormat = PF_RGB32_FLOAT;
//            } else {
//                EchoLogError("Don't support the image type ( ImageFormat [%s], BPP [%d], PixelFormat [%s] ).",
//                             Image::getImageFormatName(imgFmt).c_str(),
//                             PixelUtil::GetPixelBits(requiredFormat),
//                             PixelUtil::GetPixelFormatName(requiredFormat).c_str());
//                return false;
//            }
//        }
//
//        bool conversionRequired = false;
//
//        PixelBox convBox(imgInfo.width, imgInfo.height, 1, requiredFormat);
//        if (requiredFormat != imgInfo.pixFmt) {
//            conversionRequired = true;
//            // Allocate memory
//            convBox.data = ECHO_ALLOC_T(Byte, convBox.getConsecutiveSize());
//            // perform conversion and reassign source
//
//            Buffer memBuff(inBuff);
//            PixelBox newSrc(imgInfo.width, imgInfo.height, 1, imgInfo.pixFmt, memBuff.getData());
//            PixelUtil::BulkPixelConversion(newSrc, convBox);
//            pSrcData = static_cast<Byte *>(convBox.data);
//        }
//
//        FIBITMAP *pBitmap = FreeImage_AllocateT(fiImgType, static_cast<int>(imgInfo.width),
//                                                static_cast<int>(imgInfo.height), bpp);
//        if (!pBitmap) {
//            if (conversionRequired)
//                ECHO_FREE(convBox.data);
//
//            EchoLogError("FreeImage_AllocateT failed - possibly out of memory.");
//            EchoAssertX(false, "FreeImage_AllocateT failed - possibly out of memory.");
//
//            return false;
//        }
//
//        if ((requiredFormat == PF_R8_UNORM) ||
//            (requiredFormat == PF_R8_SNORM) ||
//            (requiredFormat == PF_R8_UINT) ||
//            (requiredFormat == PF_R8_SINT) ||
//            (requiredFormat == PF_A8_UNORM)) {
//            // Must explicitly tell FreeImage that this is greyscale by setting a "grey" palette
//            // (otherwise it will save as a normal RGB palettized image)
//            FIBITMAP *pTemp = FreeImage_ConvertToGreyscale(pBitmap);
//            FreeImage_Unload(pBitmap);
//            pBitmap = pTemp;
//        }
//
//        ui32 dstPitch = FreeImage_GetPitch(pBitmap);
//        ui32 srcPitch = imgInfo.width * PixelUtil::GetPixelSize(requiredFormat);
//
//        // Copy data, invert scanlines and respect FreeImage pitch
//        Byte *pDst = FreeImage_GetBits(pBitmap);
//        ui32 dibSize = FreeImage_GetDIBSize(pBitmap);
//        for (ui32 y = 0; y < imgInfo.height; ++y) {
//            const Byte *pSrc = pSrcData + (imgInfo.height - y - 1) * srcPitch;
//            memcpy(pDst, pSrc, srcPitch);
//            pDst += dstPitch;
//        }
//
//        if (conversionRequired) {
//            // delete temporary conversion area
//            ECHO_FREE(convBox.data);
//        }
//
//        //ui32 buffSize = srcPitch * imgInfo.height;
//        outBuff.set(dibSize, (void *) pBitmap);

        return true;
    }

    DataStream *ImageCodec::doDecode(ImageFormat imgFmt, DataStream *inStream, Image::ImageInfo &imgInfo) {
        MemoryDataStream memStream(inStream, true);
        stbi_uc *pixels = decodeImpl(memStream.getPtr(),memStream.size(),imgInfo);
        if(!pixels){
            return nullptr;
        }
        MemoryDataStream *pResult = EchoNew(MemoryDataStream(pixels,imgInfo.size, false));
        return pResult;
    }



    bool ImageCodec::doDecode(ImageFormat imgFmt, const Buffer &inBuff, Buffer &outBuff, Image::ImageInfo &imgInfo) {
        // Buffer stream into memory (TODO: override IO functions instead?)
        stbi_uc *pixels = decodeImpl(inBuff.getData(),inBuff.getSize(),imgInfo);
        if(!pixels){
            return false;
        }
        outBuff.set(imgInfo.size,pixels,true);
        return true;
    }
}
