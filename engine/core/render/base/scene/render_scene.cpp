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

	void RenderScene::update(const Frustum& frustum)
	{
		m_3dFrustum = frustum;
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
		vector<RenderProxy*>::type visibleRenderProxies3D = Renderer::instance()->gatherRenderProxies(RenderProxy::RenderType3D, m_3dFrustum);
		for (RenderProxy* renderproxy : visibleRenderProxies3D)
		{
			renderproxy->submitToRenderQueue(RenderPipeline::current());
		}

		vector<RenderProxy*>::type visibleRenderProxies2D = Renderer::instance()->gatherRenderProxies(RenderProxy::RenderType2D, AABB(-Vector3::ONE, Vector3::ONE));
		for (RenderProxy* renderproxy : visibleRenderProxies2D)
		{
			renderproxy->submitToRenderQueue(RenderPipeline::current());
		}

		vector<RenderProxy*>::type visibleRenderProxiesUI = Renderer::instance()->gatherRenderProxies(RenderProxy::RenderTypeUI, AABB(-Vector3::ONE, Vector3::ONE));
		for (RenderProxy* renderproxy : visibleRenderProxiesUI)
		{
			renderproxy->submitToRenderQueue(RenderPipeline::current());
		}

		RenderPipeline::current()->render();
	}
}
