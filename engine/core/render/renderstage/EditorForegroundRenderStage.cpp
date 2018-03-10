#include "EditorForegroundRenderStage.h"
#include <engine/core/render/render/RenderQueue.h>
#include "Engine/core/Render/RenderTargetManager.h"
#include "Engine/core/Scene/Scene_Manager.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	EditorForegroundRenderStage::EditorForegroundRenderStage()
		: RenderStage(RSI_EditorFrontground)
	{
	}

	EditorForegroundRenderStage::~EditorForegroundRenderStage()
	{
		destroy();
	}

	bool EditorForegroundRenderStage::initialize()
	{
		return true;
	}

	void EditorForegroundRenderStage::destroy()
	{
	}

	void EditorForegroundRenderStage::render()
	{
		EchoAssert(SceneManager::instance());
		RenderQueue* pRenderQueue = SceneManager::instance()->getRenderQueue("HelperForEditor");
		if (!pRenderQueue) return;
		RenderTargetManager::instance()->beginRenderTarget(RTI_DefaultBackBuffer, false, Renderer::BGCOLOR, true, 1.0);
		pRenderQueue->renderQueue();
		pRenderQueue->beginRender();
		RenderTargetManager::instance()->endRenderTarget(RTI_DefaultBackBuffer);
	}
#endif
}