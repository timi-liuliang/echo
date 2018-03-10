#include "SpineRenderStage.h"
#include "engine/core/render/render/Renderer.h"
#include "engine/core/render/RenderTargetManager.h"

namespace Echo
{
	SpineRenderStage::SpineRenderStage()
		: RenderStage(RSI_Spine)
	{
	}

	SpineRenderStage::~SpineRenderStage()
	{
		destroy();
	}

	bool SpineRenderStage::initialize()
	{
		return true;
	}

	void SpineRenderStage::destroy()
	{
	}

	// Ö´ÐÐäÖÈ¾
	void SpineRenderStage::render()
	{
		if (m_bEnable && !m_renderables.empty())
		{
			RenderTargetManager::instance()->beginRenderTarget(RTI_DefaultBackBuffer, false, Renderer::BGCOLOR, false, 1.0, false);		

			// äÖÈ¾CatUI
			for (int renderableID : m_renderables)
			{
				Renderable* renderable = Renderer::instance()->getRenderable(renderableID);
				if (renderable)
					renderable->render();
			}

			m_renderables.clear();

			RenderTargetManager::instance()->endRenderTarget(RTI_DefaultBackBuffer);
		}
	}
}