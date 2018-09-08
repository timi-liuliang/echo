#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "GLES2RenderBase.h"
#include "GLES2Mapping.h"
#include "GLES2RenderTarget.h"
#include "GLES2Texture.h"

namespace Echo
{
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

		m_bindTexture.m_texture = Renderer::instance()->createTexture("rt_" + StringUtil::ToString(_id));
		m_bindTexture.m_samplerState = Renderer::instance()->getSamplerState(desc);
		m_depthTexture.m_texture = Renderer::instance()->createTexture(("rtDEPTH_") + StringUtil::ToString(_id));
		m_depthTexture.m_samplerState = Renderer::instance()->getSamplerState(desc);
	} 

	GLES2RenderTarget::~GLES2RenderTarget()
	{
		if (m_fbo != Invalid_Value)
		{
			OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
		}

		if (m_rbo != Invalid_Value)
		{
			OGLESDebug(glDeleteRenderbuffers(1, &m_rbo));
		}

		m_bindTexture.m_texture->subRefCount();
		m_depthTexture.m_texture->subRefCount();
	}

	bool GLES2RenderTarget::doCreate()
	{
		GLES2Texture* texture = dynamic_cast<GLES2Texture*>(m_bindTexture.m_texture);
		EchoAssert(texture);

		OGLESDebug(glGenTextures(1, &texture->m_hTexture));
		OGLESDebug(glBindTexture(GL_TEXTURE_2D, texture->m_hTexture));
		OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GLES2Mapping::MapInternalFormat(m_pixelFormat), m_width, m_height, 0, GLES2Mapping::MapFormat(m_pixelFormat), GLES2Mapping::MapDataType(m_pixelFormat), (GLvoid*)0));

		OGLESDebug(glGenFramebuffers(1, &m_fbo));
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_hTexture, 0));
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_pixFmt = m_pixelFormat;

		const SamplerState* sampleState = m_bindTexture.m_samplerState;
		EchoAssert(sampleState);
		sampleState->active(NULL);

		if( m_bHasDepth )
		{
			GLES2Texture* depthTexture = dynamic_cast<GLES2Texture*>(m_depthTexture.m_texture);
			EchoAssert(depthTexture);

			// 将深度缓冲区映射到纹理上(这里应该分情况讨论，rbo效率更高，在不需要depth tex时应该优先使用		
			OGLESDebug(glGenTextures(1, &depthTexture->m_hTexture));
			OGLESDebug(glBindTexture(GL_TEXTURE_2D, depthTexture->m_hTexture));


			OGLESDebug(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, 
				GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));

			OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
			OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->m_hTexture, 0));

			depthTexture->m_width = m_width;
			depthTexture->m_height = m_height;
			depthTexture->m_pixFmt = m_pixelFormat;

			const SamplerState* depthSampleState = m_depthTexture.m_samplerState;
			EchoAssert(depthSampleState);
			depthSampleState->active(NULL);
		}
		else if (m_depthTarget)
		{
			GLES2Texture* depthTexture = dynamic_cast<GLES2Texture*>(m_depthTarget->getDepthTexture());

			OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
			OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->m_hTexture, 0));

			const SamplerState* depthSampleState = m_depthTexture.m_samplerState;
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

	bool GLES2RenderTarget::doCreateCubemap()
	{
		GLES2Texture* texture = dynamic_cast<GLES2Texture*>(m_bindTexture.m_texture);
		EchoAssert(texture);

		OGLESDebug(glGenTextures(1, &texture->m_hTexture));
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, texture->m_hTexture));
		for (int f = 0; f < 6; f++)
		{
			OGLESDebug(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GLES2Mapping::MapFormat(m_pixelFormat), 
				m_width, m_height, 0, GLES2Mapping::MapFormat(m_pixelFormat), GL_UNSIGNED_BYTE, (GLvoid*)0));
		}
		OGLESDebug(glGenFramebuffers(1, &m_fbo));
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, texture->m_hTexture, 0));

		const SamplerState* sampleState = m_bindTexture.m_samplerState;
		EchoAssert(sampleState);
		sampleState->active(NULL);

		if( m_bHasDepth )
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

	bool GLES2RenderTarget::doBeginRender( bool _clearColor, const Color& _backgroundColor,  bool _clearDepth, float _depthValue, bool _clearStencil, ui8 _stencilValue )
	{
		//EchoAssert( m_id == RenderTargetManager::Instance()->getInUsingRenderTargetID() );
		if (m_bFrameBufferChange)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		}
		GLint fbo, rbo;
		OGLESDebug(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo));
		OGLESDebug(glGetIntegerv(GL_RENDERBUFFER_BINDING, &rbo));

		if (m_tiledRender)
		{
			OGLESDebug(glViewport(static_cast<GLint>(m_curRenderTiled.x_), static_cast<GLint>(m_curRenderTiled.y_), static_cast<GLsizei>(m_curRenderTiled.width_), static_cast<GLsizei>(m_curRenderTiled.height_)));
		}

		if (m_bViewportChange && !m_tiledRender)
		{
			OGLESDebug(glViewport(0, 0, m_width, m_height));
		}

		doClear( _clearColor, _backgroundColor, _clearDepth, _depthValue, _clearStencil, _stencilValue );

		return true;
	}

	bool GLES2RenderTarget::doEndRender()
	{
		//		RenderTargetManager::Instance()->beginRenderTarget( RTI_DefaultBackBuffer, true, Renderer::BGCOLOR, true, 1.0f, false, 0 );
		return true;
	}

	bool GLES2RenderTarget::doInvalidateFrameBuffer(bool invalidateColor, bool invalidateDepth, bool invalidateStencil)
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
	void GLES2RenderTarget::bindTarget_left()
	{
		OGLESDebug(glViewport(0, 0, m_width / 2, m_height));
	}
	void GLES2RenderTarget::bindTarget_right()
	{
		OGLESDebug(glViewport(m_width / 2, 0, m_width / 2, m_height));
	}

	void GLES2RenderTarget::doClear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value)
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

	void GLES2RenderTarget::doOnResize( ui32 _width, ui32 _height )
	{
		if( m_id != 0)
		{
			m_width = _width;
			m_height = _height;

			if (m_fbo != RenderTarget::Invalid_Value)
			{
				OGLESDebug(glDeleteFramebuffers(1, &m_fbo));
				m_fbo = 0;
			}

			if (m_rbo != RenderTarget::Invalid_Value)
			{
				OGLESDebug(glDeleteRenderbuffers(1, &m_rbo));
				m_rbo = 0;
			}

			EchoAssert(m_bindTexture.m_texture);
			m_bindTexture.m_texture->subRefCount();
			m_bindTexture.m_texture = Renderer::instance()->createTexture("rt_" + StringUtil::ToString(m_id));

			EchoAssert(m_depthTexture.m_texture);
			m_depthTexture.m_texture->subRefCount();
			m_depthTexture.m_texture = Renderer::instance()->createTexture("rtDEPTH_" + StringUtil::ToString(m_id));

			doCreate();
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

		GLES2Texture* texture = ECHO_DOWN_CAST<GLES2Texture*>(m_bindTexture.m_texture);

		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
		OGLESDebug(glBindTexture(GL_TEXTURE_CUBE_MAP, texture->m_hTexture));
		OGLESDebug(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cf, texture->m_hTexture, 0));
		OGLESDebug(glCheckFramebufferStatus(GL_FRAMEBUFFER));
#endif
	}

	// 保存到纹理
	bool GLES2RenderTarget::doSaveTo( const char* file )
	{
#ifdef ECHO_PLATFORM_WINDOWS
		if( PathUtil::IsFileExist(file) )
		{
			if(0 != remove(file))
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
		bmpinfoHeader.biSizeImage = pixel_data_size;
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

		bmpfHeader.bfSize = file_size;

		bmpfHeader.bfReserved1 = 0;
		bmpfHeader.bfReserved2 = 0;
		bmpfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + bmpinfoHeader.biSize + palett_size;

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

	bool GLES2RenderTarget::doStoreDefaultRenderTarget()
	{
		OGLESDebug(glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&(m_fbo)));

		return true;
	}

	bool GLES2RenderTarget::doRestoreDefaultRenderTarget()
	{
		OGLESDebug(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

		return true;
	}

#ifdef ECHO_PLATFORM_WINDOWS
	WORD  DIBNumColors(void* lpbi)
	{
		WORD wBitCount;

		// 对于Windows的DIB, 实际颜色的数目可以比象素的位数要少。
		// 对于这种情况，则返回一个近似的数值。

		// 判断是否是WIN3.0 DIB
		if (IS_WIN30_DIB(lpbi))
		{
			DWORD dwClrUsed;

			// 读取dwClrUsed值
			dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;

			if (dwClrUsed != 0)
			{
				// 如果dwClrUsed（实际用到的颜色数）不为0，直接返回该值
				return (WORD)dwClrUsed;
			}
		}

		// 读取象素的位数
		if (IS_WIN30_DIB(lpbi))
		{
			// 读取biBitCount值
			wBitCount = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;
		}
		else
		{
			// 读取biBitCount值
			wBitCount = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;
		}

		// 按照象素的位数计算颜色数目
		switch (wBitCount)
		{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			return 0;
		}
	}


	WORD  PaletteSize(void* lpbi)
	{
		// 计算DIB中调色板的大小
		if (IS_WIN30_DIB (lpbi))
		{
			//返回颜色数目×RGBQUAD的大小
			return (WORD)(DIBNumColors(lpbi) * sizeof(RGBQUAD));
		}
		else

		{
			//返回颜色数目×RGBTRIPLE的大小
			return (WORD)(DIBNumColors(lpbi) * sizeof(RGBTRIPLE));
		}
	}
#endif
}
