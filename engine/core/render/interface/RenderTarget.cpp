#include "Renderer.h"
#include "interface/RenderTarget.h"
#include "interface/ShaderProgramRes.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	void RenderTarget::reusageDepthTarget(RenderTarget* depthTarget)
	{
		m_depthTarget = depthTarget;
	}

	Echo::ui32 RenderTarget::getMemorySize()
	{
		ui32 memorySize = 0;
		if (m_bindTexture)
			memorySize += PixelUtil::CalcLevelSize(m_bindTexture->getWidth(), m_bindTexture->getHeight(), m_bindTexture->getDepth(), 0, m_bindTexture->getPixelFormat());

		if (m_bHasDepth)
			memorySize += PixelUtil::CalcLevelSize(m_depthTexture->getWidth(), m_depthTexture->getHeight(), m_depthTexture->getDepth(), 0, m_depthTexture->getPixelFormat());

		return memorySize;
	}

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

	RenderTarget::~RenderTarget()
	{
		m_bindTexture = NULL;
		m_depthTexture = NULL;
		m_depthTarget = nullptr;
	}

	bool RenderTarget::create()
	{
		EchoAssert(m_bindTexture);

		Texture* texture = m_bindTexture;
		texture->m_width = m_width;
		texture->m_height = m_height;
		texture->m_depth = 1;

		texture->m_pixFmt = m_pixelFormat;
		texture->m_isCompressed = false;
		texture->m_compressType = Texture::CompressType_Unknown;

		if (!m_isCubemap)
		{
			return doCreate();
		}
		else
		{
			return doCreateCubemap();
		}
	}

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

	bool RenderTarget::endRender()
	{
		return doEndRender();
	}
    
    bool RenderTarget::invalidateFrameBuffer(bool invalidateColor, bool invalidateDepth, bool invalidateStencil)
	{
		return doInvalidateFrameBuffer(invalidateColor, invalidateDepth, invalidateStencil);
	}

	void RenderTarget::clear(bool _clearColor, const Color& _backgroundColor, bool _clearDepth, float _depthValue, bool _clearStencil, ui8 _stencilValue)
	{
		return doClear(_clearColor, _backgroundColor, _clearDepth, _depthValue, _clearStencil, _stencilValue);
	}
}
