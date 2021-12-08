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
		CLASS_BIND_METHOD(RenderQueue, isSort);
		CLASS_BIND_METHOD(RenderQueue, setSort);
		CLASS_BIND_METHOD(RenderQueue, getCameraFilter);
		CLASS_BIND_METHOD(RenderQueue, setCameraFilter);

		CLASS_REGISTER_PROPERTY(RenderQueue, "Sort", Variant::Type::Bool, isSort, setSort);
		CLASS_REGISTER_PROPERTY(RenderQueue, "CameraFilter", Variant::Type::Int, getCameraFilter, setCameraFilter);
	}

	void RenderQueue::render(FrameBufferPtr& frameBuffer)
	{
		onRenderBegin();
		{
			Renderer* render = Renderer::instance();
			if (render)
			{
				if (m_sort)
				{
					std::sort(m_renderables.begin(), m_renderables.end(), [](RenderableID a, RenderableID b) -> bool
					{
						RenderProxy* renderproxyA = Renderer::instance()->getRenderProxy(a);
						RenderProxy* renderproxyB = Renderer::instance()->getRenderProxy(b);
						if (renderproxyA && renderproxyB)
						{
							Render* nodeA = renderproxyA->getNode();
							Render* nodeB = renderproxyB->getNode();

							Camera* cameraA = nodeA->getCamera();
							Camera* cameraB = nodeB->getCamera();

							Vector3 vA = nodeA->getWorldPosition() - cameraA->getPosition();
							Vector3 vB = nodeB->getWorldPosition() - cameraB->getPosition();

							float lenA = vA.dot(cameraA->getDirection());
							float lenB = vB.dot(cameraB->getDirection());

							lenA += nodeA->getRenderType().getIdx() == 1 ? 1e6f : 0.f;
							lenB += nodeB->getRenderType().getIdx() == 1 ? 1e6f : 0.f;

							return lenA > lenB;
						}
						else
						{
							return false;
						}
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