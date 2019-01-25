#include "RenderPipeline.h"
#include "../Renderer.h"
#include "../RenderTarget.h"

namespace Echo
{
	RenderPipeline::RenderPipeline()
	{
		// default backbuffer
		RenderTarget::Options option; option.depth = true;
		RenderTarget* defautRT = Renderer::instance()->createRenderTarget(RTI_DefaultBackBuffer, Renderer::instance()->getScreenWidth(), Renderer::instance()->getScreenHeight(), Renderer::instance()->getBackBufferPixelFormat(), option);
		if (defautRT)
			m_renderTargets.insert(RenderTargetMap::value_type(RTI_DefaultBackBuffer, defautRT));
	}

	RenderPipeline::~RenderPipeline()
	{

	}

	RenderPipeline* RenderPipeline::instance()
	{
		static RenderPipeline* inst = EchoNew(RenderPipeline);
		return inst;
	}

	bool RenderPipeline::beginRenderTarget(ui32 id, bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue, ui32 rbo)
	{
		RenderTargetMap::iterator it = m_renderTargets.find(id);
		return it != m_renderTargets.end() ? it->second->beginRender(clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue) : false;
	}

	bool RenderPipeline::endRenderTarget(ui32 id)
	{
		RenderTargetMap::iterator it = m_renderTargets.find(id);
		return it != m_renderTargets.end() ? it->second->endRender() : false;
	}

	void RenderPipeline::onSize(ui32 width, ui32 height)
	{
		for (auto& it : m_renderTargets)
		{
			it.second->onResize(width, height);
		}
	}
}