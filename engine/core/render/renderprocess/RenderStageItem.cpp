#include "render/Renderable.h"
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

	// render
	void DefaultRenderStageItemTransparent::render()
	{
		RenderStageItem::render();

		RenderTargetManager::instance()->endRenderTarget(RTI_DefaultBackBuffer);
	}
}