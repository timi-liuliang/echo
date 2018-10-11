#include "engine/core/math/Rect.h"
#include "engine/core/log/Log.h"
#include "engine/core/io/IO.h"
#include "interface/image/PixelFormat.h"
#include "interface/image/Image.h"
#include "interface/image/TextureLoader.h"
#include "GLESRenderBase.h"
#include "GLESRenderer.h"
#include "GLESTextureCube.h"
#include "GLESMapping.h"
#include <iostream>

namespace Echo
{
	GLESTextureCube::GLESTextureCube(const String& name)
		: TextureCube(name)
	{
		m_glesTexture = 0;
	}

	GLESTextureCube::GLESTextureCube(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth, ui32 numMipmaps, const Buffer &buff, bool bBak)
		//: TextureCube(texType, pixFmt, usage, width, height, depth, numMipmaps, buff)
	{
		m_glesTexture = 0;

		create2D(pixFmt, usage, width, height, numMipmaps, buff);
	}
	
	GLESTextureCube::~GLESTextureCube()
	{
		unload();
	}

	bool GLESTextureCube::unload()
	{
		if (m_glesTexture)
		{
			OGLESDebug(glDeleteTextures(1, &m_glesTexture));
			m_glesTexture = 0;
		}

		return true;
	} 

	void GLESTextureCube::createCubeTexture()
	{
		unload();

		OGLESDebug(glGenTextures(1, &m_glesTexture));

		// generate gpu texture
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, m_glesTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		// set default sampler state
		OGLESDebug(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		OGLESDebug(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		// bind texture to empty
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
	}

	// set surface data
	void GLESTextureCube::setCubeSurfaceData(int face, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, m_glesTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));


		//---------------------------
		PixelFormat srcPixFmt = pixFmt;
		PixelFormat dstPixFmt = pixFmt;
		switch (pixFmt)
		{
		case PF_R8_UNORM:
		{
			srcPixFmt = PF_R8_UNORM;
			dstPixFmt = PF_RGBA8_UNORM;
		} break;
		case PF_RGBA8_UNORM:
		{
			srcPixFmt = PF_BGRA8_UNORM;
			dstPixFmt = PF_RGBA8_UNORM;
		} break;
		case PF_RGB8_UNORM:
		{
			srcPixFmt = PF_BGR8_UNORM;
			dstPixFmt = PF_RGB8_UNORM;
		} break;
		default: break;
		}

		GLenum internalFmt = GLES2Mapping::MapInternalFormat(dstPixFmt);
		GLenum glFmt = GLES2Mapping::MapFormat(dstPixFmt);
		GLenum glType = GLES2Mapping::MapDataType(dstPixFmt);

		bool bRequiredConvert = (srcPixFmt == dstPixFmt ? false : true);

		//for (ui32 face = 0; face < 6; face++)
		//{
		//	texWidth = width;
		//	texHeight = height;

		//	for (ui32 level = 0; level < numMipmaps; ++level)
		//	{

		//		Byte* curMipData = 0;

		//		if (bHasData)
		//		{
		//			if (bRequiredConvert)
		//			{
		//				PixelBox srcBox(texWidth, texHeight, 1, srcPixFmt, pixel_data[face]);
		//				PixelBox dstBox(texWidth, texHeight, 1, dstPixFmt);
		//				dstBox.pData = ECHO_ALLOC_T(Byte, dstBox.getConsecutiveSize());

		//				PixelUtil::BulkPixelConversion(srcBox, dstBox);
		//				curMipData = (Byte*)dstBox.pData;
		//			}
		//			else
		//			{
		//				curMipData = pixel_data[face];
		//			}
		//		}

		//		OGLESDebug(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFmt, texWidth, texHeight, 0, glFmt, glType, curMipData));

		//		if (bHasData)
		//		{
		//			if (bRequiredConvert)
		//				ECHO_FREE(curMipData);
		//		}
		//	}
		//}

		//----------------------------

		// mip map
		if (m_isMipMapEnable)
			OGLESDebug(glGenerateMipmap(m_glesTexture));

		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
	}

	bool GLESTextureCube::load()
	{
		// check
		for (size_t i = 0; i < m_surfaces.size(); i++)
		{
			if (!IO::instance()->isResourceExists(m_surfaces[i].getPath()))
				return false;
		}

		// create gles texture
		createCubeTexture();

		// set surface data
		for (size_t i = 0; i < m_surfaces.size(); i++)
		{
			const String& path = m_surfaces[i].getPath();
			MemoryReader memReader( path);
			if (memReader.getSize())
			{
				Buffer commonTextureBuffer(memReader.getSize(), memReader.getData<ui8*>(), false);
				Image* image = Image::CreateFromMemory(commonTextureBuffer, Image::GetImageFormat(getPath()));
				if (image)
				{
					m_bCompressed = false;
					m_compressType = Texture::CompressType_Unknown;
					PixelFormat pixFmt = image->getPixelFormat();
					m_width = image->getWidth();
					m_height = image->getHeight();
					m_depth = image->getDepth();
					m_pixFmt = pixFmt;
					m_numMipmaps = image->getNumMipmaps();
					if (m_numMipmaps == 0)
						m_numMipmaps = 1;

					m_pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);

					// load to gpu
					Buffer buff(m_pixelsSize, image->getData(), false);
					create2D(m_pixFmt, m_usage, m_width, m_height, m_numMipmaps, buff);

					EchoSafeDelete(image, Image);
				}
			}
		}

		return true;
	}
}
