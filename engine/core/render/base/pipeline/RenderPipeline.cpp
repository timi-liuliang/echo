#include "RenderPipeline.h"
#include "../Renderer.h"
#include "../FrameBuffer.h"
#include "engine/core/io/IO.h"

static const char* defaultPipelineTemplate = R"(<?xml version="1.0" encoding="utf-8"?>
<pipeline>
</pipeline>
)";

namespace Echo
{
	static RenderPipelinePtr g_current;

	RenderPipeline::RenderPipeline()
	{
        m_framebuffers.insert(FramebufferMap::value_type(FB_Window, Renderer::instance()->getWindowFrameBuffer()));
	}

	RenderPipeline::RenderPipeline(const ResourcePath& path)
		: Res(path)
	{
	}

	RenderPipeline::~RenderPipeline()
	{
        m_framebuffers.clear();
	}

	void RenderPipeline::bindMethods()
	{
	}

	RenderPipelinePtr RenderPipeline::current()
	{
		if (!g_current)
		{
			g_current = EchoNew( RenderPipeline);
			g_current->setSrc(defaultPipelineTemplate);
		}

		return g_current;
	}

	void RenderPipeline::setCurrent(const ResourcePath& path)
	{
		if (g_current)
		{
			EchoSafeDelete(g_current, RenderPipeline);
		}

		g_current = ECHO_DOWN_CAST<RenderPipeline*>(Res::get(path));
	}

	void RenderPipeline::setSrc(const String& src)
	{
		m_srcData = src;
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

	void RenderPipeline::addRenderable(const String& name, RenderableID id)
	{
		//for (RenderQueue* item : m_items)
		//{
		//	if (item->getName() == name)
		//		item->addRenderable(id);
		//}
	}

	void RenderPipeline::process()
	{
		if (!m_isParsed)
		{
			m_isParsed = true;
		}
	}

	Res* RenderPipeline::load(const ResourcePath& path)
	{
		MemoryReader reader(path.getPath());
		if (reader.getSize())
		{
			RenderPipeline* res = EchoNew(RenderPipeline(path));
			res->setSrc(reader.getData<const char*>());

			return res;
		}

		return nullptr;
	}

	void RenderPipeline::save()
	{
		const char* content = m_srcData.empty() ? defaultPipelineTemplate : m_srcData.data();
		if (content)
		{
			String fullPath = IO::instance()->convertResPathToFullPath(m_path.getPath());
			std::ofstream f(fullPath.c_str());

			f << content;

			f.flush();
			f.close();
		}
	}
}
