#include "engine/core/math/Rect.h"
#include "engine/core/log/Log.h"
#include "interface/image/PixelFormat.h"
#include "interface/image/Image.h"
#include "interface/image/TextureLoader.h"
#include "GLESRenderBase.h"
#include "GLESRenderer.h"
#include "GLESTexture2D.h"
#include "GLESMapping.h"
#include <iostream>

namespace Echo
{
	GLESTexture2D::GLESTexture2D(const String& name)
		: Texture(name)
		, m_glesTexture(0)
	{
	}

	GLESTexture2D::~GLESTexture2D()
	{
		unload();
	}
	
	bool GLESTexture2D::updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size)
	{
		if(level >= m_numMipmaps || !pData)
			return false;

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_glesTexture));

		GLenum glFmt = GLES2Mapping::MapFormat(m_pixFmt);
		GLenum glType = GLES2Mapping::MapDataType(m_pixFmt);
		OGLESDebug(glTexSubImage2D(GL_TEXTURE_2D, level, (GLint)rect.left, (GLint)rect.top, (GLsizei)rect.getWidth(), (GLsizei)rect.getHeight(), glFmt, glType, pData));

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));

		return true;
	}

	void GLESTexture2D::create2DTexture()
	{
		unload();

		OGLESDebug(glGenTextures(1, &m_glesTexture));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_glesTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
	}

	void GLESTexture2D::set2DSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff)
	{
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_glesTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		GLenum internalFmt = GLES2Mapping::MapInternalFormat(pixFmt);
		if (PixelUtil::IsCompressed(pixFmt))
		{
			OGLESDebug(glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFmt, width, height, 0, buff.getSize(), buff.getData()));
		}
		else
		{
			GLenum glFmt = GLES2Mapping::MapFormat(pixFmt);
			GLenum glType = GLES2Mapping::MapDataType(pixFmt);
			OGLESDebug(glTexImage2D(GL_TEXTURE_2D, level, internalFmt, width, height, 0, glFmt, glType, buff.getData()));
		}

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
	}

	bool GLESTexture2D::load()
	{
		create2DTexture();

		MemoryReader memReader(getPath());
		if (memReader.getSize())
		{
			Buffer commonTextureBuffer(memReader.getSize(), memReader.getData<ui8*>(), false);
			Image* image = Image::CreateFromMemory(commonTextureBuffer, Image::GetImageFormat(getPath()));
			if (image)
			{
				m_isCompressed = false;
				m_compressType = Texture::CompressType_Unknown;
				PixelFormat pixFmt = image->getPixelFormat();
				m_width = image->getWidth();
				m_height = image->getHeight();
				m_depth = image->getDepth();
				m_pixFmt = pixFmt;
				m_numMipmaps = image->getNumMipmaps() ? image->getNumMipmaps() : 1;
				ui32 pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
				Buffer buff(pixelsSize, image->getData(), false);

				set2DSurfaceData( 0, m_pixFmt, m_usage, m_width, m_height, buff);
				EchoSafeDelete(image, Image);

				// generate mip maps
				if (m_isMipMapEnable && !m_compressType)
				{
					OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_glesTexture));
					OGLESDebug(glGenerateMipmap(m_glesTexture));
					OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
				}

				return true;
			}
		}

		return false;
	}

	bool GLESTexture2D::unload()
	{
		if (m_glesTexture)
		{
			OGLESDebug(glDeleteTextures(1, &m_glesTexture));
			m_glesTexture = 0;
		}

		return true;
	}
}
