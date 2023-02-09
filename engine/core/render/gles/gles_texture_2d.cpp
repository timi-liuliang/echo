#include "engine/core/math/Rect.h"
#include "engine/core/log/Log.h"
#include "engine/core/io/IO.h"
#include "base/image/pixel_format.h"
#include "base/image/Image.h"
#include "base/image/texture_loader.h"
#include "gles_render_base.h"
#include "gles_renderer.h"
#include "gles_texture_2d.h"
#include "gles_mapping.h"
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

	bool GLESTexture2D::updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size)
	{
		create2DTexture();

		m_isCompressed = false;
		m_compressType = Texture::CompressType_Unknown;
		m_width = width;
		m_height = height;
		m_depth = 1;
		m_pixFmt = format;
		m_numMipmaps = 1;
		ui32 pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
		Buffer buff(pixelsSize, data, false);
		set2DSurfaceData(0, m_pixFmt, m_usage, m_width, m_height, buff);

		// generate mip maps
		if (m_isMipMapEnable && !m_compressType)
		{
			OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_glesTexture));
			OGLESDebug(glGenerateMipmap(GL_TEXTURE_2D));
			OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
		}

		return true;
	}
	
	bool GLESTexture2D::updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size)
	{
		if(level >= m_numMipmaps || !pData)
			return false;

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_glesTexture));

		GLenum glFmt = GLESMapping::MapFormat(m_pixFmt);
		GLenum glType = GLESMapping::MapDataType(m_pixFmt);
		OGLESDebug(glTexSubImage2D(GL_TEXTURE_2D, level, (GLint)rect.left, (GLint)rect.top, (GLsizei)rect.getWidth(), (GLsizei)rect.getHeight(), glFmt, glType, pData));

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));

		return true;
	}

	void GLESTexture2D::create2DTexture()
	{
		if (!m_glesTexture)
		{
			OGLESDebug(glGenTextures(1, &m_glesTexture));
			OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_glesTexture));
			OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			OGLESDebug(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
		}
	}

	void GLESTexture2D::set2DSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff)
	{
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_glesTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		GLenum internalFmt = GLESMapping::MapInternalFormat(pixFmt);
		if (PixelUtil::IsCompressed(pixFmt))
		{
			OGLESDebug(glCompressedTexImage2D(GL_TEXTURE_2D, level, internalFmt, width, height, 0, buff.getSize(), buff.getData()));
		}
		else
		{
			GLenum glFmt = GLESMapping::MapFormat(pixFmt);
			GLenum glType = GLESMapping::MapDataType(pixFmt);
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
			Image* image = Image::createFromMemory(Buffer(memReader.getSize(), memReader.getData<ui8*>(), false), Image::GetImageFormat(getPath()));
			if (image)
			{
				m_isCompressed = false;
				m_compressType = Texture::CompressType_Unknown;
				m_width = image->getWidth();
				m_height = image->getHeight();
				m_depth = image->getDepth();
				m_pixFmt = image->getPixelFormat();
				m_numMipmaps = image->getNumMipmaps() ? image->getNumMipmaps() : 1;
				ui32 pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
				i32 level = 0;

				set2DSurfaceData(level, m_pixFmt, m_usage, m_width, m_height, Buffer(pixelsSize, image->getData(), false));

				// Generate mipmaps
				if (m_isMipMapEnable && !m_compressType)
				{
					while (true)
					{
						i32 halfWidth = image->getWidth() / 2;
						i32 halfHeight = image->getHeight() / 2;
						if (!image->scale(halfWidth, halfHeight))
							break;

						pixelsSize = PixelUtil::CalcSurfaceSize(halfWidth, halfHeight, 1, 1, m_pixFmt);
						set2DSurfaceData(++level, m_pixFmt, m_usage, halfWidth, halfHeight, Buffer(pixelsSize, image->getData(), false));
					}
				}

				EchoSafeDelete(image, Image);

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

	void GLESTexture2D::onLostDevice()
	{
		const String& path = getPath();
		if (IO::instance()->isExist(path))
		{
			unload();
		}
	}

	void GLESTexture2D::onResetDevice()
	{	
		const String& path = getPath();
 		if (IO::instance()->isExist(path))
		{
			load();
		}
	}
}
