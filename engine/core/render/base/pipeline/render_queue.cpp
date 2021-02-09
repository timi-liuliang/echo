#include "../renderable.h"
#include "engine/core/scene/render_node.h"
#include "../renderer.h"
#include "render_queue.h"

namespace Echo
{
	RenderQueue::~RenderQueue()
	{
	}

	void RenderQueue::bindMethods()
	{
		CLASS_BIND_METHOD(RenderQueue, isSort, DEF_METHOD("isSort"));
		CLASS_BIND_METHOD(RenderQueue, setSort, DEF_METHOD("setSort"));

		CLASS_REGISTER_PROPERTY(RenderQueue, "Sort", Variant::Type::Bool, "isSort", "setSort");
	}

	void RenderQueue::render()
	{
		onRenderBegin();
		{
			Renderer* render = Renderer::instance();
			if (render)
			{
				// sort
				if (m_sort)
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
		onRenderEnd();
	}
}