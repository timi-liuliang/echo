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
	GLESTextureCube::GLESTextureCube()
		: TextureCube()
		, m_glesTexture(0)
	{

	}

	GLESTextureCube::GLESTextureCube(const String& name)
		: TextureCube(name)
		, m_glesTexture(0)
	{
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
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, m_glesTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		OGLESDebug(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		OGLESDebug(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
	}

	void GLESTextureCube::setCubeSurfaceData(int face, int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, m_glesTexture));
		OGLESDebug(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		GLenum internalFmt = GLES2Mapping::MapInternalFormat( pixFmt);
		if (PixelUtil::IsCompressed(pixFmt))
		{
			OGLESDebug(glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFmt, width, height, 0, buff.getSize(), buff.getData()));
		}
		else
		{
			GLenum glFmt = GLES2Mapping::MapFormat(pixFmt);
			GLenum glType = GLES2Mapping::MapDataType(pixFmt);
			OGLESDebug(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFmt, width, height, 0, glFmt, glType, buff.getData()));
		}

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
				Image* image = Image::CreateFromMemory(commonTextureBuffer, Image::GetImageFormat(path));
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
					
					setCubeSurfaceData( i, 0, m_pixFmt, m_usage, m_width, m_height, m_numMipmaps, buff);
					EchoSafeDelete(image, Image);
				}
			}
		}

		// generate mip maps
		if (m_isMipMapEnable && !m_compressType)
		{
			//OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, m_glesTexture));
			//OGLESDebug(glGenerateMipmap(m_glesTexture));
			//OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
		}

		return true;
	}
}
