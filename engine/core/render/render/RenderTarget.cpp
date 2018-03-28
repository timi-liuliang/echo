#include "Render/RenderTarget.h"
#include "Render/Material.h"
#include "Engine/core/main/Root.h"

namespace Echo
{

	void RenderTarget::reusageDepthTarget(RenderTarget* depthTarget)
	{
		m_depthTarget = depthTarget;
	}

	Echo::ui32 RenderTarget::getMemorySize()
	{
		ui32 memorySize = 0;
		Texture* pColorTex = m_bindTexture.m_texture;
		if (pColorTex)
		{
			memorySize += PixelUtil::CalcLevelSize(pColorTex->getWidth(), pColorTex->getHeight(), pColorTex->getDepth(), 0, pColorTex->getPixelFormat());
		}
		if (m_bHasDepth)
		{
			Texture* pDepthTex = m_depthTexture.m_texture;
			memorySize += PixelUtil::CalcLevelSize(pDepthTex->getWidth(), pDepthTex->getHeight(), pDepthTex->getDepth(), 0, pDepthTex->getPixelFormat());
		}

		return memorySize;
	}

	// 构造函数
	RenderTarget::RenderTarget( ui32 _id, ui32 _width, ui32 _height, PixelFormat _format, const Options& option )
		: m_id( _id )
		, m_pixelFormat( _format )
		, m_bHasDepth(option.depth)
		, m_bHasMSAA(option.msaa)
		, m_width( _width )
		, m_height( _height )
		, m_bFrameBufferChange( 0 )
		, m_bViewportChange( 0 )
		, m_bLogicResolution(option.multiResolution)
		, m_isCubemap(option.cubemap)
		, m_tiledRender( false )
		, m_handlerOnsize(false)
		, m_scale(1.0)
		, m_depthTarget(nullptr)
	{
	}

	// 析构函数
	RenderTarget::~RenderTarget()
	{
		m_bindTexture.m_texture = NULL;
		m_depthTexture.m_texture = NULL;
		m_depthTarget = nullptr;
	}

	// 创建
	bool RenderTarget::create()
	{
		EchoAssert(m_bindTexture.m_texture);

		Texture* texture = m_bindTexture.m_texture;
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_depth = 1;

		texture->m_pixFmt = m_pixelFormat;
		texture->m_bCompressed = false;
		texture->m_compressType = Texture::CompressType_Unknown;

		if (!m_isCubemap)
		{
			texture->m_texType = Texture::TT_2D;
			return doCreate();
		}
		else
		{
			texture->m_texType = Texture::TT_CUBE;
			return doCreateCubemap();
		}
	}


	// 设置为当前渲染目标
	bool RenderTarget::beginRender(bool _clearColor, const Color& _backgroundColor, bool _clearDepth, float _depthValue, bool _clearStencil, ui8 _stencilValue)
	{
		//针对rgba16f不支持的情况下,对_backgroundColor做hdr2ldr处理
		Color newClearColor = _backgroundColor;
		if (_clearColor && !Renderer::instance()->getDeviceFeatures().supportHFColorBf())
		{
			Color result = 1.f / 25.f * _backgroundColor;
			float maxValue = Math::Max(result.r, Math::Max(result.g, result.b));
			float m = Math::Clamp(maxValue, 0.f, 1.f);
			m = Math::Ceil(m * 255.f) / 255.f;
			if (m > 0.0)
			{
				result.r = result.r / m;
				result.g = result.g / m;
				result.b = result.b / m;
			}

			newClearColor = result;
		}
		//m_RenderFrameCount = EchoRoot->getCurFrameCount();
		return doBeginRender(_clearColor, newClearColor, _clearDepth, _depthValue, _clearStencil, _stencilValue);
	}

	// 结束渲染，将要切换为另外的渲染目标
	bool RenderTarget::endRender()
	{
		return doEndRender();
	}
    
    bool RenderTarget::invalidateFrameBuffer(bool invalidateColor, bool invalidateDepth, bool invalidateStencil)
	{
		return doInvalidateFrameBuffer(invalidateColor, invalidateDepth, invalidateStencil);
	}

	// 清空
	void RenderTarget::clear(bool _clearColor, const Color& _backgroundColor, bool _clearDepth, float _depthValue, bool _clearStencil, ui8 _stencilValue)
	{
		return doClear(_clearColor, _backgroundColor, _clearDepth, _depthValue, _clearStencil, _stencilValue);
	}
}
