#include "shadow_depth_render_stage.h"
#include "../light/light.h"

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

		}
	}
}