#include "../Renderable.h"
#include "engine/core/scene/render_node.h"
#include "RenderStageItem.h"
#include "engine/core/render/interface/RenderTargetManager.h"

namespace Echo
{
	RenderStageItem::RenderStageItem()
	{
	}

	RenderStageItem::~RenderStageItem()
	{
	}

	void RenderStageItem::render()
	{
		for (RenderableID id : m_renderables)
		{
			Renderable* renderable = Renderer::instance()->getRenderable( id);
			if( renderable)
				renderable->render();
		}

		m_renderables.clear();
	}

	DefaultRenderStageItemOpaque::DefaultRenderStageItemOpaque()
	{
		setName("Opaque");
	}

	// render
	void DefaultRenderStageItemOpaque::render()
	{
		RenderTargetManager::instance()->beginRenderTarget(RTI_DefaultBackBuffer);
		RenderStageItem::render();
	}

	DefaultRenderStageItemTransparent::DefaultRenderStageItemTransparent()
	{
		setName("Transparent");
	}

	// sort
	void DefaultRenderStageItemTransparent::sort()
	{
		std::sort(m_renderables.begin(), m_renderables.end(), [](RenderableID a, RenderableID b) -> bool
		{
			Renderable* renderableA = Renderer::instance()->getRenderable(a);
			Renderable* renderableB = Renderer::instance()->getRenderable(b);
			return renderableA->getNode()->getWorldPosition().z < renderableB->getNode()->getWorldPosition().z;
		});
	}

	// render
	void DefaultRenderStageItemTransparent::render()
	{
		sort();
		RenderStageItem::render();

		RenderTargetManager::instance()->endRenderTarget(RTI_DefaultBackBuffer);
	}
}