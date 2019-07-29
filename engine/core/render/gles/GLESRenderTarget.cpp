#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "GLESRenderBase.h"
#include "GLESMapping.h"
#include "GLESRenderTarget.h"
#include "GLESTexture2D.h"

namespace Echo
{
#define INVALIDE  0xFFFFFFFF

#ifdef ECHO_PLATFORM_WINDOWS
	#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
	#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))

	WORD  DIBNumColors(void* lpbi)
	{
		if (IS_WIN30_DIB(lpbi))
		{
			DWORD dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;
			if (dwClrUsed != 0)
				return (WORD)dwClrUsed;
		}

		WORD wBitCount = IS_WIN30_DIB(lpbi) ? ((LPBITMAPINFOHEADER)lpbi)->biBitCount : ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;
		switch (wBitCount)
		{
		case 1:		return 2;
		case 4:		return 16;
		case 8:		return 256;
		default:	return 0;
		}
	}

	WORD  PaletteSize(void* lpbi)
	{
		return IS_WIN30_DIB(lpbi) ? (WORD)(DIBNumColors(lpbi) * sizeof(RGBQUAD)) : (WORD)(DIBNumColors(lpbi) * sizeof(RGBTRIPLE));
	}
#endif

	GLES2RenderTarget::GLES2RenderTarget( ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, const Options& option)
		: RenderTarget(_id, _width, _height, _pixelFormat, option)
		, m_fbo(0)
		, m_rbo(0)
	{
		SamplerState::SamplerDesc desc;
		desc.addrUMode = SamplerState::AM_CLAMP;
		desc.addrVMode = SamplerState::AM_CLAMP;
		desc.addrWMode = SamplerState::AM_CLAMP;
		desc.mipFilter = SamplerState::FO_NONE;

		m_bindTexture = Renderer::instance()->createTexture2D("rt_" + StringUtil::ToString(_id));
		m_bindTexture->setSamplerState(desc);
		m_depthTexture = Renderer::instance()->createTexture2D(("rtDEPTH_") + StringUtil::ToString(_id));
		m_depthTexture->setSamplerState(desc);
	}

	GLES2RenderTarget::~GLES2RenderTarget()
	{
		if (m_fbo != INVALIDE)
		{
			OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
		}

		if (m_rbo != INVALIDE)
		{
			OGLESDebug(glDeleteRenderbuffers(1, &m_rbo));
		}

		m_bindTexture->subRefCount();
		m_depthTexture->subRefCount();
	}

	bool GLES2RenderTarget::create()
	{
		EchoAssert(m_bindTexture);
		Texture* texture = m_bindTexture;
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_depth = 1;
		texture->m_pixFmt = m_pixelFormat;
		texture->m_isCompressed = false;
		texture->m_compressType = Texture::CompressType_Unknown;

		return m_isCubemap ? createCubemap() : createTexture2D();
	}

	bool GLES2RenderTarget::createTexture2D()
	{
		GLESTexture2D* texture = dynamic_cast<GLESTexture2D*>(m_bindTexture);
		EchoAssert(texture);

		OGLESDebug(glGenTextures(1, &texture->m_glesTexture));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, texture->m_glesTexture));
		OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GLES2Mapping::MapInternalFormat(m_pixelFormat), m_width, m_height, 0, GLES2Mapping::MapFormat(m_pixelFormat), GLES2Mapping::MapDataType(m_pixelFormat), (GLvoid*)0));

		OGLESDebug(glGenFramebuffers(1, &m_fbo));
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_glesTexture, 0));
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_pixFmt = m_pixelFormat;

		const SamplerState* sampleState = m_bindTexture->getSamplerState();
		EchoAssert(sampleState);
		sampleState->active(NULL);

		if( m_isHasDepth )
		{
			GLESTexture2D* depthTexture = dynamic_cast<GLESTexture2D*>(m_depthTexture);
			EchoAssert(depthTexture);

			// 将深度缓冲区映射到纹理上(这里应该分情况讨论，rbo效率更高，在不需要depth tex时应该优先使用
			OGLESDebug(glGenTextures(1, &depthTexture->m_glesTexture));
			OGLESDebug(glBindTexture(GL_TEXTURE_2D, depthTexture->m_glesTexture));
			OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0,  GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
			OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
			OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->m_glesTexture, 0));

			depthTexture->m_width = m_width;
			depthTexture->m_height = m_height;
			depthTexture->m_pixFmt = m_pixelFormat;

			const SamplerState* depthSampleState = m_depthTexture->getSamplerState();
			EchoAssert(depthSampleState);
			depthSampleState->active(NULL);
		}
		else if (m_depthTarget)
		{
			GLESTexture2D* depthTexture = dynamic_cast<GLESTexture2D*>(m_depthTarget->getDepthTexture());

			OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
			OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->m_glesTexture, 0));

			const SamplerState* depthSampleState = m_depthTexture->getSamplerState();
			EchoAssert(depthSampleState);
			depthSampleState->active(NULL);
		}

		GLuint uStatus = OGLESDebug(glCheckFramebufferStatus(GL_FRAMEBUFFER));
		if (uStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			EchoLogError("Create RenderTarget Failed !");
		}

#if defined(ECHO_PLATFORM_WINDOWS) || defined(ECHO_PLATFORM_ANDROID)
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif

		return true;
	}

	bool GLES2RenderTarget::createCubemap()
	{
		GLESTexture2D* texture = dynamic_cast<GLESTexture2D*>(m_bindTexture);
		EchoAssert(texture);

		OGLESDebug(glGenTextures(1, &texture->m_glesTexture));
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, texture->m_glesTexture));
		for (int f = 0; f < 6; f++)
		{
			OGLESDebug(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GLES2Mapping::MapFormat(m_pixelFormat), m_width, m_height, 0, GLES2Mapping::MapFormat(m_pixelFormat), GL_UNSIGNED_BYTE, (GLvoid*)0));
		}
		OGLESDebug(glGenFramebuffers(1, &m_fbo));
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, texture->m_glesTexture, 0));

		const SamplerState* sampleState = m_bindTexture->getSamplerState();
		EchoAssert(sampleState);
		sampleState->active(NULL);

		if( m_isHasDepth)
		{
			OGLESDebug(glGenRenderbuffers(1, &m_rbo));
			OGLESDebug(glBindRenderbuffer(GL_RENDERBUFFER, m_rbo));
			OGLESDebug(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height));
			OGLESDebug(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo));
		}

		GLuint uStatus = OGLESDebug(glCheckFramebufferStatus(GL_FRAMEBUFFER));
		if (uStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			EchoLogError("Create RenderTarget Failed !");
		}

#if defined(ECHO_PLATFORM_WINDOWS) || defined(ECHO_PLATFORM_ANDROID)
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif

		return true;
	}

	bool GLES2RenderTarget::beginRender(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
	{
		// bind frame buffer
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glViewport(0, 0, m_width, m_height));

		// clear
		clear( clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue );

		return true;
	}

	bool GLES2RenderTarget::endRender()
	{
		return true;
	}

	bool GLES2RenderTarget::invalide(bool invalidateColor, bool invalidateDepth, bool invalidateStencil)
	{
		int attachment_count = 0;
		GLenum attachments[128] = { 0 };
		if (invalidateColor)
			attachments[attachment_count++] = GL_COLOR_ATTACHMENT0;
		if (invalidateDepth)
			attachments[attachment_count++] = GL_DEPTH_ATTACHMENT;
		if (invalidateStencil)
			attachments[attachment_count++] = GL_STENCIL_ATTACHMENT;

		if (attachment_count > 0)
		{
			OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

#ifndef ECHO_PLATFORM_ANDROID
			OGLESDebug(glInvalidateFramebuffer(GL_FRAMEBUFFER, attachment_count, attachments));
#endif
		}
		return true;
	}

	void GLES2RenderTarget::clear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value)
	{
		GLbitfield mask = 0;

		if (clear_color)
		{
			OGLESDebug(glClearColor(color.r, color.g, color.b, color.a));
			mask |= GL_COLOR_BUFFER_BIT;
		}

		if (clear_depth)
		{
			OGLESDebug(glClearDepthf(depth_value));
			mask |= GL_DEPTH_BUFFER_BIT;
		}

		OGLESDebug(glDepthMask(clear_depth));

		if (clear_stencil)
		{
			OGLESDebug(glClearStencil(stencil_value));
			mask |= GL_STENCIL_BUFFER_BIT;
		}

		OGLESDebug(glStencilMask(clear_stencil));

		if (mask != 0)
			OGLESDebug(glClear(mask));

		Renderer::instance()->setDepthStencilState( Renderer::instance()->getDefaultDepthStencilState());
	}

	void GLES2RenderTarget::onResize( ui32 _width, ui32 _height )
	{
		if( m_id != 0)
		{
			m_width = _width;
			m_height = _height;

			if (m_fbo != INVALIDE)
			{
				OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
				m_fbo = 0;
			}

			if (m_rbo != INVALIDE)
			{
				OGLESDebug(glDeleteRenderbuffers(1, &m_rbo));
				m_rbo = 0;
			}

			EchoAssert(m_bindTexture);
			m_bindTexture->subRefCount();
			m_bindTexture = Renderer::instance()->createTexture2D("rt_" + StringUtil::ToString(m_id));

			EchoAssert(m_depthTexture);
			m_depthTexture->subRefCount();
			m_depthTexture = Renderer::instance()->createTexture2D("rtDEPTH_" + StringUtil::ToString(m_id));

			// recreate
			create();
		}
		else
		{
			m_width  = _width;
			m_height = _height;
		}
	}

	void GLES2RenderTarget::doSetCubeFace( Texture::CubeFace cf )
	{
#ifdef ECHO_PLATFORM_WINDOWS
		EchoAssert( m_isCubemap );

		if( cf == 1 )
		{
			EchoAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_NEGATIVE_X );
		}
		else if( cf == 2 )
		{
			EchoAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_POSITIVE_Y );
		}
		else if( cf == 3 )
		{
			EchoAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y );
		}
		else if( cf == 4 )
		{
			EchoAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_POSITIVE_Z );
		}
		else if( cf == 5 )
		{
			EchoAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z );
		}

		GLESTexture2D* texture = ECHO_DOWN_CAST<GLESTexture2D*>(m_bindTexture);

		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, texture->m_glesTexture));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cf, texture->m_glesTexture, 0));
		OGLESDebug(glCheckFramebufferStatus(GL_FRAMEBUFFER));
#endif
	}

	bool GLES2RenderTarget::save( const char* file)
	{
#ifdef ECHO_PLATFORM_WINDOWS
		if( PathUtil::IsFileExist(file) )
		{
			if(0 != ::remove(file))
			{
				EchoLogError( "save file[%s] Failed !", file );
				return false;
			}
		}

		struct SPixel
		{
			unsigned char r_, g_, b_, a_;

			void convertBGRA()
			{
				unsigned char tmp = r_;
				r_ = b_;
				b_ = tmp;
			}
		};


		GLvoid* pixels = new char[m_width * m_height * sizeof(int)];
		EchoAssert( pixels );

		OGLESDebug(glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

		SPixel* rgbaPixel = (SPixel*)pixels;

		for( ui32 i = 0; i < m_width * m_height; i++ )
		{
			SPixel& rgba = *(rgbaPixel + i);

			rgba.convertBGRA();
		}


		BITMAPFILEHEADER bmpfHeader = {0};
		bmpfHeader.bfType = DIB_HEADER_MARKER;
		BITMAPINFOHEADER bmpinfoHeader = {0};

		size_t pixel_data_size = m_width * m_height * 4;

		bmpinfoHeader.biWidth = m_width;
		bmpinfoHeader.biHeight = m_height;
		bmpinfoHeader.biSizeImage = static_cast<DWORD>(pixel_data_size);
		bmpinfoHeader.biSize = 40;
		bmpinfoHeader.biPlanes = 1;
		bmpinfoHeader.biBitCount = 4 * 8;
		bmpinfoHeader.biCompression = 0;
		bmpinfoHeader.biXPelsPerMeter = 0;
		bmpinfoHeader.biYPelsPerMeter = 0;
		bmpinfoHeader.biClrUsed = 0;
		bmpinfoHeader.biClrImportant = 0;

		unsigned char* bmpImage = new unsigned char[sizeof(BITMAPINFOHEADER)+pixel_data_size];

		memcpy( bmpImage, &bmpinfoHeader, sizeof(BITMAPINFOHEADER) );

		if( ((*(LPDWORD)(&bmpinfoHeader)) != sizeof(BITMAPINFOHEADER)) )
		{
			return false;
		}

		size_t palett_size = PaletteSize(&bmpinfoHeader);

		size_t file_size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + palett_size + pixel_data_size;

		bmpfHeader.bfSize = static_cast<DWORD>(file_size);

		bmpfHeader.bfReserved1 = 0;
		bmpfHeader.bfReserved2 = 0;
		bmpfHeader.bfOffBits = static_cast<DWORD>(sizeof(BITMAPFILEHEADER) + bmpinfoHeader.biSize + palett_size);

		unsigned char *bmpfile = new unsigned char[file_size];
		EchoAssert( bmpfile );
		memcpy( bmpfile, &bmpfHeader, sizeof(BITMAPFILEHEADER) );
		memcpy( bmpfile + sizeof(BITMAPFILEHEADER), &bmpinfoHeader, sizeof(BITMAPINFOHEADER) );
		memcpy( bmpfile + sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER) + palett_size, pixels, pixel_data_size );

		std::ofstream outputfile;

		outputfile.open(file,std::ios::binary | std::ios::out);

		outputfile.write((char *)bmpfile, file_size);

		outputfile.close();

		delete [] bmpfile;
		delete [] pixels;
#endif
		return true;
	}
}
