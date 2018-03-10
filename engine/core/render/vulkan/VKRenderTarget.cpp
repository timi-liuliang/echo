#include "GLES2RenderStd.h"
#include "Render/Renderer.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"
#include "GLES2Mapping.h"
#include "GLES2RenderTarget.h"
#include "GLES2Texture.h"
#include "GLES2TextureGPUProxy.h"
#include "GLES2TargetGPUProxy.h"
#include "GLES2TargetTasks.h"
#include "Foundation/Util/PathUtil.h"

namespace LORD
{
	// 构造函数
	GLES2RenderTarget::GLES2RenderTarget( ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, bool _hasDepth /* = false */, bool _msaa /* = false */, bool _multi_resolution, bool _is_cubemap )
		: RenderTarget( _id, _width, _height, _pixelFormat, _hasDepth, _msaa, _multi_resolution, _is_cubemap )
		, m_gpu_proxy(LordNew(GLES2TargetGPUProxy))
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

	// 析构函数
	GLES2RenderTarget::~GLES2RenderTarget()
	{
		TRenderTask<GLES2TargetTaskDestroyFrameBuffer>::CreateTask(m_gpu_proxy);
		m_gpu_proxy = nullptr;
	}

	// 执行创建
	bool GLES2RenderTarget::doCreate()
	{
		GLES2Texture* texture = dynamic_cast<GLES2Texture*>(m_bindTexture.m_texture);
		LordAssert(texture);
		LordAssert(texture->m_gpu_proxy);
		TRenderTask<GLES2TargetTaskCreateFrameBuffer>::CreateTask(m_gpu_proxy, texture->m_gpu_proxy, m_pixelFormat, m_width, m_height);

		const SamplerState* sampleState = m_bindTexture.m_samplerState;
		LordAssert(sampleState);
		sampleState->active(NULL);

		if( m_bHasDepth )
		{
			GLES2Texture* depthTexture = dynamic_cast<GLES2Texture*>(m_depthTexture.m_texture);
			LordAssert(depthTexture);
			LordAssert(depthTexture->m_gpu_proxy);
			TRenderTask<GLES2TargetTaskCreateDepthBuffer>::CreateTask(m_gpu_proxy, depthTexture->m_gpu_proxy, m_width, m_height);

			const SamplerState* depthSampleState = m_depthTexture.m_samplerState;
			LordAssert(depthSampleState);
			depthSampleState->active(NULL);
		}
		else if (m_depthTarget)
		{
			GLES2Texture* depthTexture = dynamic_cast<GLES2Texture*>(m_depthTarget->getDepthTexture());
			TRenderTask<GLES2TargetTaskBindDepthBuffer>::CreateTask(m_gpu_proxy, depthTexture->m_gpu_proxy);

			const SamplerState* depthSampleState = m_depthTexture.m_samplerState;
			LordAssert(depthSampleState);
			depthSampleState->active(NULL);
		}

		TRenderTask<GLES2TargetTaskPostFrameBufferCreate>::CreateTask(m_gpu_proxy);

		return true;
	} 

	bool GLES2RenderTarget::doCreateCubemap()
	{
		GLES2Texture* texture = dynamic_cast<GLES2Texture*>(m_bindTexture.m_texture);
		LordAssert(texture);
		LordAssert(texture->m_gpu_proxy);
		TRenderTask<GLES2TargetTaskCreateFrameBufferCube>::CreateTask(m_gpu_proxy, texture->m_gpu_proxy, m_pixelFormat, m_width, m_height);

		const SamplerState* sampleState = m_bindTexture.m_samplerState;
		LordAssert(sampleState);
		sampleState->active(NULL);

		if( m_bHasDepth )
		{
			TRenderTask<GLES2TargetTaskCreateDepthBufferCube>::CreateTask(m_gpu_proxy, m_width, m_height);
		}

		TRenderTask<GLES2TargetTaskPostFrameBufferCreate>::CreateTask(m_gpu_proxy);

		return true;
	}

	bool GLES2RenderTarget::doBeginRender( bool _clearColor, const Color& _backgroundColor,  bool _clearDepth, float _depthValue, bool _clearStencil, ui8 _stencilValue )
	{
		//LordAssert( m_id == RenderTargetManager::Instance()->getInUsingRenderTargetID() );

		TRenderTask<GLES2TargetTaskBeginRender>::CreateTask(m_gpu_proxy, m_bFrameBufferChange, m_tiledRender, m_curRenderTiled, m_bViewportChange, m_width, m_height);
		doClear( _clearColor, _backgroundColor, _clearDepth, _depthValue, _clearStencil, _stencilValue );

		return true;
	}

	bool GLES2RenderTarget::doEndRender()
	{
		//		RenderTargetManager::Instance()->beginRenderTarget( RTI_DefaultBackBuffer, true, Renderer::BGCOLOR, true, 1.0f, false, 0 );
		return true;
	}

	void GLES2RenderTarget::bindTarget_left()
	{
		TRenderTask<GLES2TargetTaskBindTargetLeft>::CreateTask(m_gpu_proxy, m_width, m_height);
	}
	void GLES2RenderTarget::bindTarget_right()
	{
		TRenderTask<GLES2TargetTaskBindTargetRight>::CreateTask(m_gpu_proxy, m_width, m_height);
	}

	// 清空渲染目标
	void GLES2RenderTarget::doClear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value)
	{
		TRenderTask<GLES2TargetTaskClear>::CreateTask(m_gpu_proxy, clear_color, color, clear_depth, depth_value, clear_stencil, stencil_value);
		Renderer::instance()->setDepthStencilState( Renderer::instance()->getDefaultDepthStencilState());
	}

	void GLES2RenderTarget::doOnResize( ui32 _width, ui32 _height )
	{
		if( m_id != 0)
		{
			m_width = _width;
			m_height = _height;

			TRenderTask<GLES2TargetTaskDeleteBuffersOnly>::CreateTask(m_gpu_proxy);
			FlushRenderTasks();

			LordAssert(m_bindTexture.m_texture);
			Renderer::instance()->releaseTexture(m_bindTexture.m_texture);
			m_bindTexture.m_texture = Renderer::instance()->createTexture("rt_" + StringUtil::ToString(m_id));

			LordAssert(m_depthTexture.m_texture);
			Renderer::instance()->releaseTexture(m_depthTexture.m_texture);
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
#ifdef LORD_PLATFORM_WINDOWS
		LordAssert( m_isCubemap );

		if( cf == 1 )
		{
			LordAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_NEGATIVE_X );
		}
		else if( cf == 2 )
		{
			LordAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_POSITIVE_Y );
		}
		else if( cf == 3 )
		{
			LordAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y );
		}
		else if( cf == 4 )
		{
			LordAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_POSITIVE_Z );
		}
		else if( cf == 5 )
		{
			LordAssert( GL_TEXTURE_CUBE_MAP_POSITIVE_X+cf == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z );
		}

		GLES2Texture* texture = LORD_DOWN_CAST<GLES2Texture*>(m_bindTexture.m_texture);
		TRenderTask<GLES2TargetTaskSetCubeFace>::CreateTask(m_gpu_proxy, texture->m_gpu_proxy, cf);
#endif
	}

	// 保存到纹理
	bool GLES2RenderTarget::doSaveTo( const char* file )
	{
#ifdef LORD_PLATFORM_WINDOWS
		if( PathUtil::IsFileExist(file) )
		{
			if(0 != remove(file))
			{
				LordLogError( "save file[%s] Failed !", file );
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
		LordAssert( pixels );

		TRenderTask<GLES2TargetTaskReadPixels>::CreateTask(m_gpu_proxy, m_width, m_height, pixels);
		FlushRenderTasks();
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
		LordAssert( bmpfile );
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
		TRenderTask<GLES2TargetTaskStoreTarget>::CreateTask(m_gpu_proxy);
		return true;
	}

	bool GLES2RenderTarget::doRestoreDefaultRenderTarget()
	{
		TRenderTask<GLES2TargetTaskRestoreTarget>::CreateTask(m_gpu_proxy);
		return true;
	}

#ifdef LORD_PLATFORM_WINDOWS
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
