#include "../renderable.h"
#include "engine/core/scene/render_node.h"
#include "../renderer.h"
#include "render_queue.h"

namespace Echo
{
	RenderQueue::RenderQueue(RenderStage* stage)
		: IRenderQueue(stage)
	{
	}

	RenderQueue::~RenderQueue()
	{
	}

	void RenderQueue::bindMethods()
	{

	}

	void RenderQueue::render()
	{
		Renderer* render = Renderer::instance();
		if (render)
		{
			// sort
			if (m_isSort)
			{
				std::sort(m_renderables.begin(), m_renderables.end(), [](RenderableID a, RenderableID b) -> bool
				{
					Renderable* renderableA = Renderer::instance()->getRenderable(a);
					Renderable* renderableB = Renderer::instance()->getRenderable(b);
					return renderableA && renderableB ? renderableA->getNode()->getWorldPosition().z < renderableB->getNode()->getWorldPosition().z : false;
				});
			}

			// render
			for (RenderableID id : m_renderables)
			{
				Renderable* renderable = Renderer::instance()->getRenderable(id);
				if (renderable)
					render->draw(renderable);
			}
		}


		m_renderables.clear();
	}
}