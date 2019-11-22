#include "RenderPipeline.h"
#include "../Renderer.h"
#include "../FrameBuffer.h"

namespace Echo
{
	RenderPipeline::RenderPipeline()
	{
        m_framebuffers.insert(FramebufferMap::value_type(FB_Window, Renderer::instance()->getWindowFrameBuffer()));
	}

	RenderPipeline::~RenderPipeline()
	{
        m_framebuffers.clear();
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
        if (it != m_framebuffers.end())
        {
            FrameBuffer* frameBuffer = it->second;
            if (frameBuffer)
                return frameBuffer->begin(clearColor, bgColor, clearDepth, depthValue, clearStencil, stencilValue);
        }

		return false;
	}

	bool RenderPipeline::endFramebuffer(ui32 id)
	{
		FramebufferMap::iterator it = m_framebuffers.find(id);
		return it != m_framebuffers.end() ? it->second->end() : false;
	}

	void RenderPipeline::onSize(ui32 width, ui32 height)
	{
		for (auto& it : m_framebuffers)
		{
            FrameBuffer* fb = it.second;
            if(fb)
                fb->onSize(width, height);
		}
	}
}
