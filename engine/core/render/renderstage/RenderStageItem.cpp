#include "render/Renderable.h"
#include "engine/core/scene/render_node.h"
#include "RenderStageItem.h"
#include "engine/core/render/RenderTargetManager.h"

namespace Echo
{
	// 构造函数
	RenderStageItem::RenderStageItem()
	{
	}

	// 析构函数
	RenderStageItem::~RenderStageItem()
	{
	}

	// 渲染
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