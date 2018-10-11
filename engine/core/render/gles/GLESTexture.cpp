#include "engine/core/math/Rect.h"
#include "engine/core/log/Log.h"
#include "interface/image/PixelFormat.h"
#include "interface/image/Image.h"
#include "interface/image/TextureLoader.h"
#include "GLESRenderBase.h"
#include "GLESRenderer.h"
#include "GLESTexture.h"
#include "GLESMapping.h"
#include <iostream>

namespace Echo
{
	GLESTexture2D::GLESTexture2D(const String& name)
		: Texture(name)
		, m_hTexture(0)
	{
		load();
	}

	GLESTexture2D::GLESTexture2D(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth, ui32 numMipmaps, const Buffer &buff, bool bBak)
		: Texture(texType, pixFmt, usage, width, height, depth, numMipmaps, buff)
	{
		m_hTexture = 0;

		create2D(pixFmt, usage, width, height, numMipmaps, buff);
	}

	GLESTexture2D::~GLESTexture2D()
	{
		EchoSafeDelete(m_memeryData, MemoryReader);

		unload();
	}
	
	bool GLESTexture2D::updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size)
	{
		if(level >= m_numMipmaps || !pData)
			return false;

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_hTexture));

		GLenum glFmt = GLES2Mapping::MapFormat(m_pixFmt);
		GLenum glType = GLES2Mapping::MapDataType(m_pixFmt);
		OGLESDebug(glTexSubImage2D(GL_TEXTURE_2D, level, (GLint)rect.left, (GLint)rect.top, (GLsizei)rect.getWidth(), (GLsizei)rect.getHeight(), glFmt, glType, pData));

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));

		return true;
	}

	bool GLESTexture2D::create2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		for (ui32 level = 0; level < m_numMipmaps; ++level)
		{
			ui32 curMipSize = PixelUtil::CalcLevelSize(width, height, 1, level, pixFmt);
			m_uploadedSize += curMipSize;
		}
		m_width = width;
		m_height = height;
		m_pixFmt = pixFmt;

		OGLESDebug(glGenTextures(1, &m_hTexture));
		if (!m_hTexture)
		{
			EchoLogError("Create GLES2Texture [%s] failed.", PixelUtil::GetPixelFormatName(pixFmt).c_str());
			return false;
		}

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_hTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		ui32 texWidth = width;
		ui32 texHeight = height;
		Byte* pData = buff.getData();

		bool bHasData = (pData != NULL ? true : false);

		PixelFormat srcPixFmt = pixFmt;
		PixelFormat dstPixFmt = pixFmt;
		switch (pixFmt)
		{
			case PF_R8_UNORM:
			{
				srcPixFmt = PF_R8_UNORM;
				dstPixFmt = PF_RGBA8_UNORM;
			} break;
			default: break;
		}

		bool bRequiredConvert = (srcPixFmt == dstPixFmt ? false : true);
		for (ui32 level = 0; level < m_numMipmaps; ++level)
		{
			Byte* curMipData = 0;
			if (bHasData)
			{
				if (bRequiredConvert)
				{
					PixelBox srcBox(texWidth, texHeight, 1, srcPixFmt, pData);
					PixelBox dstBox(texWidth, texHeight, 1, dstPixFmt);
					dstBox.pData = ECHO_ALLOC_T(Byte, dstBox.getConsecutiveSize());

					PixelUtil::BulkPixelConversion(srcBox, dstBox);
					curMipData = (Byte*)dstBox.pData;
				}
				else
				{
					curMipData = pData;
				}
			}

			ui32 curMipSize = PixelUtil::CalcLevelSize(width, height, 1, level, pixFmt);
			GLenum internalFmt = GLES2Mapping::MapInternalFormat(dstPixFmt);
			if (PixelUtil::IsCompressed(pixFmt))
			{
				OGLESDebug(glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFmt, texWidth, texHeight, 0, curMipSize, curMipData));
			}
			else
			{
				GLenum glFmt = GLES2Mapping::MapFormat(dstPixFmt);
				GLenum glType = GLES2Mapping::MapDataType(dstPixFmt);
				OGLESDebug(glTexImage2D(GL_TEXTURE_2D, level, internalFmt, texWidth, texHeight, 0, glFmt, glType, curMipData));
			}

			if (bHasData)
			{
				if (bRequiredConvert)
					ECHO_FREE(curMipData);
			}

			pData += curMipSize;

			texWidth = (texWidth > 1) ? texWidth >> 1 : 1;
			texHeight = (texHeight > 1) ? texHeight >> 1 : 1;
		}

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
		return true;
	}

	bool GLESTexture2D::load()
	{
		MemoryReader memReader(getPath());
		if (memReader.getSize())
		{
			Buffer commonTextureBuffer(memReader.getSize(), memReader.getData<ui8*>(), false);
			Image* image = Image::CreateFromMemory(commonTextureBuffer, Image::GetImageFormat(getPath()));
			if (image)
			{
				m_bCompressed = false;
				m_compressType = Texture::CompressType_Unknown;
				PixelFormat pixFmt = image->getPixelFormat();

				if (ECHO_ENDIAN == ECHO_ENDIAN_LITTLE)
				{
					switch (pixFmt)
					{
					case PF_BGR8_UNORM:		pixFmt = PF_RGB8_UNORM;		break;
					case PF_BGRA8_UNORM:	pixFmt = PF_RGBA8_UNORM;	break;
					default:;
					}
				}

				m_width = image->getWidth();
				m_height = image->getHeight();
				m_depth = image->getDepth();
				m_pixFmt = pixFmt;
				m_numMipmaps = image->getNumMipmaps();
				if (m_numMipmaps == 0)
					m_numMipmaps = 1;

				m_pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
				EchoSafeDelete(m_memeryData, MemoryReader);
				m_memeryData = EchoNew(MemoryReader((const char*)image->getData(), m_pixelsSize));

				EchoSafeDelete(image, Image);

				// load to gpu
				Buffer buff(m_pixelsSize, m_memeryData->getData<ui8*>());
				return create2D(m_pixFmt, m_usage, m_width, m_height, m_numMipmaps, buff);
			}
		}

		return false;
	}

	bool GLESTexture2D::unload()
	{
		if (m_hTexture)
		{
			OGLESDebug(glDeleteTextures(1, &m_hTexture));
			m_hTexture = 0;
		}

		return true;
	}
}
