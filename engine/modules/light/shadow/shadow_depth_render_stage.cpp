#include "shadow_depth_render_stage.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"

namespace Echo
{
	ShadowDepthRenderStage::ShadowDepthRenderStage()
		: RenderStage()
	{
		// Add default render queue
		Echo::IRenderQueue* renderQueue = EchoNew(RenderQueue);
		if (renderQueue)
		{
			renderQueue->setName("Shadow Depth");
			addRenderQueue(renderQueue);
		}
	}

	ShadowDepthRenderStage::~ShadowDepthRenderStage()
	{

	}

	void ShadowDepthRenderStage::bindMethods()
	{

	}

	void ShadowDepthRenderStage::render()
	{
		vector<Light*>::type dirLights = Light::gatherLights(Light::Type::Direction);
		for (Light* light : dirLights)
		{
			DirectionLight* dirLight = ECHO_DOWN_CAST<DirectionLight*>(light);
			Frustum*		frustum  = dirLight->getFrustum();

			if (frustum)
			{
				vector<RenderProxy*>::type visibleRenderProxies3D = Renderer::instance()->gatherRenderProxies(RenderProxy::RenderType3D, *frustum);
				if (!visibleRenderProxies3D.empty())
				{
					int a = 10;
				}
			}
		}

		RenderStage::render();
	}
}