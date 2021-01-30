#include "engine/core/math/Rect.h"
#include "engine/core/log/Log.h"
#include "engine/core/io/IO.h"
#include "base/image/pixel_format.h"
#include "base/image/Image.h"
#include "base/image/texture_loader.h"
#include "gles_render_base.h"
#include "gles_renderer.h"
#include "gles_texture_render.h"
#include "gles_mapping.h"
#include <iostream>

namespace Echo
{
	GLESTextureRender::GLESTextureRender(const String& name)
		: TextureRender(name)
		, m_glesTexture(0)
	{
	}

	GLESTextureRender::~GLESTextureRender()
	{
		unload();
	}

	bool GLESTextureRender::updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size)
	{
		create2DTexture();

		m_width = width;
		m_height = height;
		m_usage = usage;
		m_isCompressed = false;
		m_compressType = Texture::CompressType_Unknown;
		m_depth = 1;
		m_pixFmt = format;
		m_numMipmaps = 1;
		ui32 pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
		Buffer buff(pixelsSize, data, false);
		set2DSurfaceData(0, m_pixFmt, m_usage, m_width, m_height, buff);

		return true;
	}

	void GLESTextureRender::create2DTexture()
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

	void GLESTextureRender::set2DSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff)
	{
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, m_glesTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		GLenum internalFmt = GLESMapping::MapInternalFormat(pixFmt);
		if (!PixelUtil::IsCompressed(pixFmt))
		{
			GLenum glFmt = GLESMapping::MapFormat(pixFmt);
			GLenum glType = GLESMapping::MapDataType(pixFmt);
			OGLESDebug(glTexImage2D(GL_TEXTURE_2D, level, internalFmt, width, height, 0, glFmt, glType, buff.getData()));
		}

		OGLESDebug(glBindTexture(GL_TEXTURE_2D, 0));
	}

	bool GLESTextureRender::unload()
	{
		if (m_glesTexture)
		{
			OGLESDebug(glDeleteTextures(1, &m_glesTexture));
			m_glesTexture = 0;
		}

		return true;
	}

	GLuint GLESTextureRender::getGlesTexture() 
	{ 
		if (!m_glesTexture)
		{
			if (m_pixFmt != PF_UNKNOWN)
			{
				if (!PixelUtil::IsDepth(m_pixFmt))
				{
					m_usage = Texture::TU_GPU_READ;
					size_t pixelsize = m_width * m_height * PixelUtil::GetPixelSize(m_pixFmt);
					if (pixelsize)
					{
						vector<Byte>::type textureData(pixelsize, 0);
						updateTexture2D(m_pixFmt, m_usage, m_width, m_height, textureData.data(), pixelsize);
					}
				}
				else
				{
					glGenRenderbuffers(1, &m_glesTexture);
					glBindRenderbuffer(GL_RENDERBUFFER, m_glesTexture);
					glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
				}
			}
			else
			{
				// do nothing
			}
		}

		return m_glesTexture; 
	}
}
