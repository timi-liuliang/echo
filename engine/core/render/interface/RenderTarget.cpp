#include "Renderer.h"
#include "interface/RenderTarget.h"
#include "interface/ShaderProgram.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Echo::ui32 RenderTarget::getMemorySize()
	{
		ui32 memorySize = 0;
		if (m_bindTexture)
			memorySize += PixelUtil::CalcLevelSize(m_bindTexture->getWidth(), m_bindTexture->getHeight(), m_bindTexture->getDepth(), 0, m_bindTexture->getPixelFormat());

		if (m_isHasDepth)
			memorySize += PixelUtil::CalcLevelSize(m_depthTexture->getWidth(), m_depthTexture->getHeight(), m_depthTexture->getDepth(), 0, m_depthTexture->getPixelFormat());

		return memorySize;
	}

	RenderTarget::RenderTarget( ui32 id, ui32 width, ui32 height, PixelFormat format, const Options& option )
		: m_isHasMSAA(option.msaa)
        , m_id( id)
		, m_pixelFormat( format)
        , m_isHasDepth(option.depth)
		, m_width( width)
		, m_height( height)
		, m_isCubemap(option.cubemap)
		, m_depthTarget(nullptr)
	{
	}

	RenderTarget::~RenderTarget()
	{
		m_bindTexture = NULL;
		m_depthTexture = NULL;
		m_depthTarget = nullptr;
	}

	void RenderTarget::bindMethods()
	{

	}
}
