#include "base/proxy/render_proxy.h"
#include "engine/core/scene/render_node.h"
#include "base/renderer.h"
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

	void RenderQueue::render(FrameBufferPtr& frameBuffer)
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
						RenderProxy* renderableA = Renderer::instance()->getRenderProxy(a);
						RenderProxy* renderableB = Renderer::instance()->getRenderProxy(b);
						return renderableA && renderableB ? renderableA->getNode()->getWorldPosition().z < renderableB->getNode()->getWorldPosition().z : false;
					});
				}

				// render
				for (RenderableID id : m_renderables)
				{
					RenderProxy* renderable = Renderer::instance()->getRenderProxy(id);
					if (renderable)
						render->draw(renderable, frameBuffer);
				}
			}

			m_renderables.clear();
		}
		onRenderEnd();
	}
}