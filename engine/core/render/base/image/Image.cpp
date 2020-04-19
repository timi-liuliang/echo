#include <engine/core/log/Log.h>
#include <engine/core/util/Exception.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include "Image.h"
#include "ImageResampler.h"
#include "ImageCodec.h"
#include "ImageCodecMgr.h"
#include <thirdparty/FreeImage/FreeImage.h>

namespace Echo
{
	Image::Image()
		: m_format(PF_UNKNOWN)
		, m_width(0)
		, m_height(0)
		, m_depth(0)
		, m_numMipmaps(0)
		, m_flags(0)
		, m_pixelSize(0)
		, m_size(0)
		, m_data(NULL)
	{
	}

	Image::Image(Byte *pData, ui32 width, ui32 height, ui32 depth, 
		PixelFormat pixFmt, ui32 numFaces, ui32 numMipMaps)
	{
		// Set image metadata
		m_format = pixFmt;
		m_width = width;
		m_height = height;
		m_depth = depth;
		m_numMipmaps = numMipMaps;
		m_flags = 0;
		m_pixelSize = PixelUtil::GetPixelSize(pixFmt);

		// Set flags
		if (PixelUtil::IsCompressed(pixFmt))
			m_flags |= IMGFLAG_COMPRESSED;
		if (m_depth != 1)
			m_flags |= IMGFLAG_3DTEX;
		if(numFaces == 6)
			m_flags |= IMGFLAG_CUBEMAP;
		if(numFaces != 6 && numFaces != 1)
		{
			EchoLogError("Number of faces currently must be 6 or 1.");
		}

		numMipMaps = numMipMaps > 0? numMipMaps : 1;
		m_size = CalculateSize(numMipMaps, numFaces, width, height, depth, pixFmt);
		m_data = EchoNewArray(Byte, m_size);
		if(pData)
		{
			memcpy(m_data, pData, m_size);
		}
		else
		{
			memset(m_data, 0, m_size);
		}
	}

	Image::~Image()
	{
		clear();
	}

	String Image::getImageFormatName(ImageFormat imgFmt)
	{
		switch(imgFmt)
		{
		case IF_BMP:		return "IF_BMP";
		case IF_DDS:		return "IF_DDS";
		case IF_JPG:		return "IF_JPG";
		case IF_PNG:		return "IF_PNG";
		case IF_PVR:		return "IF_PVR";
		case IF_TGA:		return "IF_TGA";
		default:			return "IF_UNKNOWN";
		}
	}

	Image* Image::createFromMemory(const Buffer &inBuff, ImageFormat imgFmt)
	{
		ImageCodec* pImgCodec = ImageCodecMgr::instance()->getCodec(imgFmt);
		if(!pImgCodec)
		{
			EchoLogError("Unable to load image: Image format [%s] is unknown. Unable to identify codec.", getImageFormatName(imgFmt).c_str());
			return NULL;
		}

		Buffer outBuff;
		ImageInfo imgInfo;
		if(!pImgCodec->decode(inBuff, outBuff, imgInfo))
			return NULL;

		Image* pImage = EchoNew(Image);

		pImage->m_width = imgInfo.width;
		pImage->m_height = imgInfo.height;
		pImage->m_depth = imgInfo.depth;
		pImage->m_size = imgInfo.size;
		pImage->m_numMipmaps = imgInfo.numMipmaps;
		pImage->m_flags = imgInfo.flags;

		// Get the format and compute the pixel size
		pImage->m_format = imgInfo.pixFmt;
		pImage->m_pixelSize = static_cast<Byte>(PixelUtil::GetPixelSize(imgInfo.pixFmt));
		// Just use internal buffer of returned memory stream
		outBuff.takeData(pImage->m_data);

		return pImage;
	}

	Image* Image::loadFromFile(const String& fileName)
	{
        MemoryReader memReader(fileName);
        if (memReader.getSize())
        {
            Buffer commonTextureBuffer(memReader.getSize(), memReader.getData<ui8*>(), false);
            Image* image = Image::createFromMemory(commonTextureBuffer, Image::GetImageFormat(fileName));
			if (image)
				image->setFilePath(fileName);

			return image;
        }
        
        return nullptr;
	}

	Image* Image::loadFromDataStream(DataStream* stream, const String& name)
	{
		ImageFormat imgFmt = GetImageFormat(name);
		ImageCodec* pImgCodec = ImageCodecMgr::instance()->getCodec(imgFmt);
		if(!pImgCodec)
		{
			EchoLogError("Unable to load image: Image format [%s] is unknown. Unable to identify codec.", getImageFormatName(imgFmt).c_str());
			return NULL;
		}
		ImageInfo imgInfo;
		MemoryDataStream* pResult = (MemoryDataStream*)pImgCodec->decode(stream, imgInfo);
		if(!pResult)
		{
			return NULL;
		}

		Image* pImage = EchoNew(Image);

		pImage->m_width = imgInfo.width;
		pImage->m_height = imgInfo.height;
		pImage->m_depth = imgInfo.depth;
		pImage->m_size = imgInfo.size;
		pImage->m_numMipmaps = imgInfo.numMipmaps;
		pImage->m_flags = imgInfo.flags;

		// Get the format and compute the pixel size
		pImage->m_format = imgInfo.pixFmt;
		pImage->m_pixelSize = static_cast<Byte>(PixelUtil::GetPixelSize(imgInfo.pixFmt));
		pImage->m_data = pResult->getPtr();
		EchoSafeDelete(pResult, MemoryDataStream);

		return pImage;
	}

	bool Image::saveToFile(const String& filename, ImageFormat imgFmt)
	{
		if(!m_data)
		{
			EchoLogError("No image data loaded.");
			return false;
		}

		if(imgFmt == IF_UNKNOWN)
		{
			imgFmt = Image::GetImageFormatByExt(PathUtil::GetFileExt(filename));
			if(imgFmt == IF_UNKNOWN)
			{
				EchoLogError("Unknown image format.");
				return false;
			}
		}

		ImageCodec* pCodec = ImageCodecMgr::instance()->getCodec(imgFmt);
		if(!pCodec)
		{
			EchoLogError("Not found the image format [%s] codec.", Image::getImageFormatExt(imgFmt).c_str());
			return false;
		}

		ImageInfo imgInfo;
		imgInfo.width = m_width;
		imgInfo.height = m_height;
		imgInfo.depth = m_depth;
		imgInfo.size = m_size;
		imgInfo.pixFmt = m_format;

		Buffer buff(m_size, m_data);
		if(!pCodec->codeToFile(imgFmt, buff, imgInfo, filename))
			return false;

		return true;
	}

	void Image::clear()
	{
		EchoSafeFree(m_data);
		m_format = PF_UNKNOWN;
		m_width = 0;
		m_height = 0;
		m_depth = 0;
		m_numMipmaps = 0;
		m_flags = 0;
		m_pixelSize = 0;
		m_size = 0;
	}

	bool Image::hasFlag(ImageFlags imgFlag) const
	{
		if(m_flags & imgFlag)
			return true;
		else
			return false;
	}

	PixelFormat Image::getPixelFormat() const
	{
		if (ECHO_ENDIAN == ECHO_ENDIAN_LITTLE)
		{
			switch (m_format)
			{
			case PF_BGR8_UNORM:		return PF_RGB8_UNORM;
			case PF_BGRA8_UNORM:	return PF_RGBA8_UNORM;
			default:				return m_format;
			}
		}
		else
		{
			return m_format;
		}
	}

	ui32 Image::getWidth() const
	{
		return m_width;
	}

	ui32 Image::getHeight() const
	{
		return m_height;
	}

	ui32 Image::getDepth() const
	{
		return m_depth;
	}

	ui32 Image::getNumMipmaps() const
	{
		return m_numMipmaps;
	}

	ui32 Image::getNumFaces() const
	{
		if(hasFlag(IMGFLAG_CUBEMAP))
			return 6;

		return 1;
	}

	bool Image::hasAlpha() const
	{
		return PixelUtil::HasAlpha(m_format);
	}

	Byte Image::getBPP() const
	{
		return PixelUtil::GetPixelBits(m_format);
	}

	PixelBox Image::getPixelBox(ui32 face, ui32 mipmap) const
	{
		// Image data is arranged as:
		// face 0, top level (mip 0)
		// face 0, mip 1
		// face 0, mip 2
		// face 1, top level (mip 0)
		// face 1, mip 1
		// face 1, mip 2
		// etc
		if(mipmap > getNumMipmaps())
		{
			EchoLogError("Mipmap index out of range");
		}

		if(face >= getNumFaces())
		{
			EchoLogError("Face index out of range");
		}

		// Calculate mipmap offset and size
		ui8* offset = const_cast<ui8*>(m_data);

		// Base offset is number of full faces
		ui32 width = getWidth(), height = getHeight(), depth = getDepth();
		ui32 numMips = getNumMipmaps();

		// Figure out the offsets 
		ui32 fullFaceSize = 0;
		ui32 finalFaceSize = 0;
		ui32 finalWidth = 0, finalHeight = 0, finalDepth = 0;

		for(ui32 mip = 0; mip <= numMips; ++mip)
		{
			if (mip == mipmap)
			{
				finalFaceSize = fullFaceSize;
				finalWidth = width;
				finalHeight = height;
				finalDepth = depth;
			}

			fullFaceSize += PixelUtil::GetMemorySize(width, height, depth, m_format);

			/// Half size in each dimension
			if(width != 1) width /= 2;
			if(height != 1) height /= 2;
			if(depth != 1) depth /= 2;
		}

		// Advance pointer by number of full faces, plus mip offset into
		offset += face * fullFaceSize;
		offset += finalFaceSize;

		// Return subface as pixelbox
		PixelBox src(finalWidth, finalHeight, finalDepth, m_format, offset);

		return src;
	}

	Image* Image::getAtla(ui32 face, ui32 mipmap, ui32 left, ui32 top, ui32 width, ui32 height)
	{
		PixelBox faceMipMapBox = getPixelBox(face, mipmap);

		vector<Byte>::type pixelDatas(width * height * m_pixelSize);
		for (i32 w = 0; w < width; w++)
		{
			for (i32 h = 0; h < height; h++)
			{
				for (i32 i = 0; i < m_pixelSize; i++)
				{
					i32 dest = (h * width + w) * m_pixelSize + i;
					i32 source = ((h + top) * m_width + w + left) * m_pixelSize + i;
					pixelDatas[dest] = m_data[source];
				}
			}
		}

		return EchoNew(Image(pixelDatas.data(), width, height, 1, m_format));
	}

	Color Image::getColor(int x, int y, int z) const
	{
		Color rval;
		PixelUtil::UnpackColor(rval, m_format, &m_data[m_pixelSize * (z * m_width * m_height + m_width * y + x)]);

		// reset color by pixel format
		//rval = PixelUtil::ConvertColor(rval, m_pixFmt);

		return rval;
	}

	vector<Color>::type Image::getColors() const
	{
		vector<Color>::type results;
		for (ui32 h = 0; h < m_height; h++)
		{
			for (ui32 w = 0; w < m_width; w++)
			{
				results.push_back(getColor(w, h, 0));
			}
		}

		return results;
	}

	bool Image::scale(ui32 width, ui32 height, ImageFilter filter)
	{
		// scale dynamic images is not supported
		EchoAssert(m_depth == 1);

		// reassign buffer to temp image, make sure auto-delete is true
		Image tempImg(m_data, m_width, m_height, 1, m_format);

		// do not delete[] m_pBuffer!  temp will destroy it

		// set new dimensions, allocate new buffer
		m_width = width;
		m_height = height;
		m_size = PixelUtil::GetMemorySize(m_width, m_height, 1, m_format);
		m_data = ECHO_ALLOC_T(Byte, m_size);
		m_numMipmaps = 0; // Loses precomputed mipmaps

		// scale the image from temp into our resized buffer
		if(!Image::Scale(tempImg.getPixelBox(), getPixelBox(), filter))
			return false;

		return true;
	}

	bool Image::convertFormat(PixelFormat targetFormat)
	{
		PixelBox destBox( m_width, m_height, 1, targetFormat);
		if (targetFormat != m_format)
		{
			destBox.pData = ECHO_ALLOC_T(Byte, destBox.getConsecutiveSize());

			PixelBox srcBox(m_width, m_height, 1, m_format, m_data);
			PixelUtil::BulkPixelConversion(srcBox, destBox);

			EchoSafeFree(m_data);

			m_format = targetFormat;
			m_pixelSize = PixelUtil::GetPixelSize(m_format);
			m_size = PixelUtil::GetMemorySize(m_width, m_height, 1, m_format);
			m_data = static_cast<Byte*>(destBox.pData);
		}

		return true;
	}

	String Image::getImageFormatExt(ImageFormat imgFmt)
	{
		switch(imgFmt)
		{
		case IF_UNKNOWN:		return "UNKNOWN";
		case IF_BMP:			return "BMP";
		case IF_DDS:			return "DDS";
		case IF_JPG:			return "JPG";
		case IF_PNG:			return "PNG";
		case IF_TGA:			return "TGA";
		default:				return "UNKNOWN";
		}
	}

	ImageFormat Image::GetImageFormat(const String &filename)
	{
		ImageFormat imgFmt = ImageCodec::GetImageFormat(filename);
		if(imgFmt == IF_UNKNOWN)
		{
			String fileExt = PathUtil::GetFileExt(filename);
			imgFmt = GetImageFormatByExt(fileExt);
		}

		return imgFmt;
	}

	ImageFormat Image::GetImageFormatByExt(const String &imgExt)
	{
		String imgExtStr = imgExt;
		StringUtil::UpperCase(imgExtStr);
		if(imgExtStr == "UNKNOWN")
			return IF_UNKNOWN;
		else if(imgExtStr == "BMP")
			return IF_BMP;
		else if(imgExtStr == "DDS")
			return IF_DDS;
		else if(imgExtStr == "JPG" || imgExtStr == "JPEG" || imgExtStr == "JPE")
			return IF_JPG;
		else if(imgExtStr == "PNG")
			return IF_PNG;
		else if(imgExtStr == "TGA")
			return IF_TGA;
		else
			return IF_UNKNOWN;
	}

	ui32 Image::CalculateSize(ui32 mipmaps, ui32 faces, ui32 width, ui32 height, ui32 depth, PixelFormat pixFmt)
	{
		ui32 size = 0;
		for(ui32 mip = 0; mip < mipmaps; ++mip)
		{
			size += PixelUtil::GetMemorySize(width, height, depth, pixFmt) * faces; 
			if(width != 1) width /= 2;
			if(height != 1) height /= 2;
			if(depth != 1) depth /= 2;
		}

		return size;
	}

	bool Image::Scale(const PixelBox &src, const PixelBox &dst, ImageFilter filter)
	{
		EchoAssert(PixelUtil::IsAccessible(src.pixFmt));
		EchoAssert(PixelUtil::IsAccessible(dst.pixFmt));

		Buffer buf; // For auto-delete
		PixelBox temp;
		switch (filter) 
		{
		default:
		case IMGFILTER_NEAREST:
			{
				if(src.pixFmt == dst.pixFmt) 
				{
					// No intermediate buffer needed
					temp = dst;
				}
				else
				{
					// Allocate temporary buffer of destination size in source format 
					temp = PixelBox(dst.getWidth(), dst.getHeight(), dst.getDepth(), src.pixFmt);
					buf.allocate(temp.getConsecutiveSize());
					temp.pData = buf.getData();
				}
				// super-optimized: no conversion
				switch (PixelUtil::GetPixelSize(src.pixFmt)) 
				{
				case 1: NearestResampler<1>::Scale(src, temp); break;
				case 2: NearestResampler<2>::Scale(src, temp); break;
				case 3: NearestResampler<3>::Scale(src, temp); break;
				case 4: NearestResampler<4>::Scale(src, temp); break;
				case 6: NearestResampler<6>::Scale(src, temp); break;
				case 8: NearestResampler<8>::Scale(src, temp); break;
				case 12: NearestResampler<12>::Scale(src, temp); break;
				case 16: NearestResampler<16>::Scale(src, temp); break;
				default:
					{
						// never reached
						EchoAssert(false);
						return false;
					}
				}

				if(temp.pData != dst.pData)
				{
					// Blit temp buffer
					PixelUtil::BulkPixelConversion(temp, dst);
				} 
			} break;
		case IMGFILTER_LINEAR:
		case IMGFILTER_BILINEAR:
			{
				switch (src.pixFmt)
				{
				case PF_R8_UNORM: 
				case PF_R8_SNORM: 
				case PF_R8_UINT: 
				case PF_R8_SINT: 
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
						if(src.pixFmt == dst.pixFmt) 
						{
							// No intermediate buffer needed
							temp = dst;
						}
						else
						{
							// Allocate temp buffer of destination size in source format 
							temp = PixelBox(dst.getWidth(), dst.getHeight(), dst.getDepth(), src.pixFmt);
							buf.allocate(temp.getConsecutiveSize());
							temp.pData = buf.getData();
						}
						// super-optimized: byte-oriented math, no conversion
						switch (PixelUtil::GetPixelSize(src.pixFmt)) 
						{
						case 1: LinearResamplerByte<1>::Scale(src, temp); break;
						case 2: LinearResamplerByte<2>::Scale(src, temp); break;
						case 3: LinearResamplerByte<3>::Scale(src, temp); break;
						case 4: LinearResamplerByte<4>::Scale(src, temp); break;
						default:
							{
								// never reached
								EchoAssert(false);
								return false;
							}
						}
						if(temp.pData != dst.pData)
						{
							// Blit temp buffer
							PixelUtil::BulkPixelConversion(temp, dst);
						}
					} break;
				case PF_RGB32_FLOAT:
				case PF_RGBA32_FLOAT:
					if (dst.pixFmt == PF_RGB32_FLOAT || dst.pixFmt == PF_RGBA32_FLOAT)
					{
						// float32 to float32, avoid unpack/repack overhead
						LinearResamplerFloat32::Scale(src, dst);
						break;
					}
					// else, fall through
				default:
					// non-optimized: floating-point math, performs conversion but always works
					LinearResampler::Scale(src, dst);
				}
			} break;
		}

		return true;
	}

	Byte* Image::getData() const
	{
		return m_data;
	}
}
