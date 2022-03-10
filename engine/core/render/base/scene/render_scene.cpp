#include "render_scene.h"
#include <vector>
#include "../pipeline/render_pipeline.h"

namespace Echo
{
	static std::vector<RenderScene*> g_renderScenes;

	RenderScene::RenderScene()
	{
		g_renderScenes.emplace_back(this);
	}

	RenderScene::~RenderScene()
	{
		g_renderScenes.erase(std::remove(g_renderScenes.begin(), g_renderScenes.end(), this), g_renderScenes.end());
	}

	void RenderScene::renderAll()
	{
		for (RenderScene* renderScene : g_renderScenes)
		{
			renderScene->render();
		}
	}

	void RenderScene::render()
	{
		// frustum culling
		vector<RenderProxy*>::type visibleRenderProxies = Renderer::instance()->gatherRenderProxies();
		for (RenderProxy* renderproxy : visibleRenderProxies)
		{
			renderproxy->submitToRenderQueue(RenderPipeline::current());
		}

		RenderPipeline::current()->render();
	}
}
