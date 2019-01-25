#include "../Renderable.h"
#include "engine/core/scene/render_node.h"
#include "../Renderer.h"
#include "RenderQueue.h"

namespace Echo
{
	RenderQueue::RenderQueue()
	{
	}

	RenderQueue::~RenderQueue()
	{
	}

	void RenderQueue::render()
	{
		for (RenderableID id : m_renderables)
		{
			Renderable* renderable = Renderer::instance()->getRenderable( id);
			if( renderable)
				renderable->render();
		}

		m_renderables.clear();
	}

	DefaultRenderQueueOpaque::DefaultRenderQueueOpaque()
	{
		setName("Opaque");
	}

	// render
	void DefaultRenderQueueOpaque::render()
	{
		RenderQueue::render();
	}

	DefaultRenderQueueTransparent::DefaultRenderQueueTransparent()
	{
		setName("Transparent");
	}

	// sort
	void DefaultRenderQueueTransparent::sort()
	{
		std::sort(m_renderables.begin(), m_renderables.end(), [](RenderableID a, RenderableID b) -> bool
		{
			Renderable* renderableA = Renderer::instance()->getRenderable(a);
			Renderable* renderableB = Renderer::instance()->getRenderable(b);
			return renderableA->getNode()->getWorldPosition().z < renderableB->getNode()->getWorldPosition().z;
		});
	}

	// render
	void DefaultRenderQueueTransparent::render()
	{
		sort();
		RenderQueue::render();
	}
}