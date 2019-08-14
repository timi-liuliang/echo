#include "RenderPipeline.h"
#include "../Renderer.h"
#include "../FrameBuffer.h"

namespace Echo
{
	RenderPipeline::RenderPipeline()
	{
        Renderer* renderer = Renderer::instance();
        ui32 screenWidth = renderer->getScreenWidth();
        ui32 screenHeight = renderer->getScreenHeight();

        // create default framebuffer
		FrameBuffer* defaultFB = Renderer::instance()->createFramebuffer(RTI_DefaultBackBuffer, screenWidth, screenHeight, Renderer::instance()->getBackBufferPixelFormat());
        if (defaultFB)
        {
            RenderView* colorView = renderer->createRenderView(screenWidth, screenHeight, Renderer::instance()->getBackBufferPixelFormat());
            RenderView* depthView = renderer->createRenderView(screenWidth, screenHeight, PF_D32_FLOAT);
            defaultFB->attach(FrameBuffer::Attachment::Color0, colorView);
            defaultFB->attach(FrameBuffer::Attachment::DepthStencil, depthView);

            m_framebuffers.insert(FramebufferMap::value_type(RTI_DefaultBackBuffer, defaultFB));
        }
	}

	RenderPipeline::~RenderPipeline()
	{
        EchoSafeDeleteMap(m_framebuffers, FrameBuffer);
	}

	void RenderPipeline::bindMethods()
	{
	}

	RenderPipeline* RenderPipeline::instance()
	{
		static RenderPipeline* inst = EchoNew(RenderPipeline);
		return inst;
	}

	bool RenderPipeline::beginFramebuffer(ui32 id, bool clearColor, const Color& bgColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue, ui32 rbo)
	{
		FramebufferMap::iterator it = m_framebuffers.find(id);
		return it != m_framebuffers.end() ? it->second->beginRender(clearColor, bgColor, clearDepth, depthValue, clearStencil, stencilValue) : false;
	}

	bool RenderPipeline::endFramebuffer(ui32 id)
	{
		FramebufferMap::iterator it = m_framebuffers.find(id);
		return it != m_framebuffers.end() ? it->second->endRender() : false;
	}

	void RenderPipeline::onSize(ui32 width, ui32 height)
	{
		for (auto& it : m_framebuffers)
		{
			it.second->onSize(width, height);
		}
	}
}
