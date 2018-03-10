#include "UIRenderStage.h"
#include "engine/core/Scene/scene_manager.h"
#include "Engine/core/Render/RenderTargetManager.h"

namespace Echo
{
	UIRenderStage::UIRenderStage()
		: RenderStage(RSI_UI)
	{
	}

	UIRenderStage::~UIRenderStage()
	{
		destroy();
	}

	bool UIRenderStage::initialize()
	{
		return true;
	}

	void UIRenderStage::destroy()
	{
	}

	// Ö´ÐÐäÖÈ¾
	void UIRenderStage::render()
	{
        if (!m_bEnable)
            return;

		_renderCatui3DModel();

		RenderTargetManager::instance()->beginRenderTarget(RTI_DefaultBackBuffer, false, Renderer::BGCOLOR, true, 1.0);

		_renderCatui();

		_renderCEGUI();

		_renderCatuiTop();

		_postRenderClear();
	}

	void UIRenderStage::addCatUIManualRenderable(IManualRenderable* renderable)
	{
		m_catuiManualRenderables.push_back(renderable);
	}

	void UIRenderStage::addCatUIRenderable(IManualRenderable* renderable, const int renderLevel)
	{
		if (renderLevel == 0)
			m_catuiRenderables.push_back(renderable); 
		else
			m_catuiTopRenderables.push_back(renderable);
	}

	void UIRenderStage::addCatUIEffectRenderable(IManualRenderable* renderable, const int renderLevel)
	{
		addCatUIRenderable(renderable, renderLevel);
		m_autoDeletes.push_back(renderable);
	}

	void UIRenderStage::_renderCatui3DModel()
	{
		for (IManualRenderable* renderable : m_catuiManualRenderables)
		{
			renderable->render();
		}
		m_catuiManualRenderables.clear();
	}

	void UIRenderStage::_renderCatuiTop()
	{
		for (IManualRenderable* renderable : m_catuiTopRenderables)
		{
			if (renderable)
				renderable->render();
		}
	}

	void UIRenderStage::_renderCEGUI()
	{
		EchoAssert(SceneManager::instance());
		RenderQueue* pRenderQueue = SceneManager::instance()->getRenderQueue("UI");
		if (!pRenderQueue) return;
		pRenderQueue->renderQueue();
		pRenderQueue->beginRender();
	}

	void UIRenderStage::_renderCatui()
	{
		for (IManualRenderable* renderable : m_catuiRenderables)
		{
			//Renderable* renderable = Renderer::instance()->getRenderable(renderableID);
			if (renderable)
				renderable->render();
		}
	}

	void UIRenderStage::_postRenderClear()
	{
		m_catuiRenderables.clear();
		m_catuiTopRenderables.clear();

		RenderTargetManager::instance()->endRenderTarget(RTI_DefaultBackBuffer);

		for (IManualRenderable* del : m_autoDeletes)
		{
			EchoSafeDelete(del, IManualRenderable);
		}
		m_autoDeletes.clear();
	}

}
