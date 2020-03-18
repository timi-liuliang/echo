#include "RenderPipeline.h"
#include "../Renderer.h"
#include "../FrameBuffer.h"
#include "engine/core/io/IO.h"

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

	RenderPipelinePtr RenderPipeline::current()
	{
		return g_current;
	}

	void RenderPipeline::setCurrent(const ResourcePath& path)
	{

	}

	void RenderPipeline::bindMethods()
	{
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

	}

	Res* RenderPipeline::load(const ResourcePath& path)
	{
		MemoryReader reader(path.getPath());
		if (reader.getSize())
		{
			RenderPipeline* inst = EchoNew(RenderPipeline(path));
			//res->m_srcData = reader.getData<const char*>();

			return inst;
		}

		return nullptr;
	}

	void RenderPipeline::save()
	{
		const char* content = "";// m_srcData.empty() ? luaScriptTemplate : m_srcData.data();
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
