#include "RenderPipeline.h"
#include "../Renderer.h"
#include "../FrameBuffer.h"
#include "engine/core/io/IO.h"
#include "RenderStage.h"
#include <thirdparty/pugixml/pugixml.hpp>

static const char* defaultPipelineTemplate = R"(<?xml version="1.0" encoding="utf-8"?>
<pipeline>
	<stage name="Final">
		<queue type="queue" name="Opaque" sort="false" />
		<queue type="queue" name="Transparent" sort="true" />
		<framebuffer id="0" />
	</stage>
</pipeline>
)";

namespace Echo
{
	static RenderPipelinePtr g_current;

	RenderPipeline::RenderPipeline()
	{
        m_framebuffers.insert(FramebufferMap::value_type(0, Renderer::instance()->getWindowFrameBuffer()));
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
		for (RenderStage* stage : m_renderStages)
		{
			stage->addRenderable(name, id);
		}
	}

	void RenderPipeline::render()
	{
		if (!m_isParsed)
		{
			parseXml();

			m_isParsed = true;
		}

		for (RenderStage* stage : m_renderStages)
		{
			stage->render();
		}
	}

	void RenderPipeline::parseXml()
	{
		EchoSafeDeleteContainer(m_renderStages, RenderStage);

		pugi::xml_document doc;
		if (doc.load_buffer(m_srcData.data(), m_srcData.size()));
		{
			pugi::xml_node rootNode = doc.child("pipeline");
			if (rootNode)
			{
				for (pugi::xml_node stageNode = rootNode.child("stage"); stageNode; stageNode = stageNode.next_sibling("stage"))
				{
					RenderStage* stage = EchoNew(RenderStage(this));
					stage->setName(stageNode.attribute("name").as_string());
					stage->parseXml(&stageNode);
					m_renderStages.emplace_back(stage);
				}
			}
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
