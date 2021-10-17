#include "shadow_depth_render_stage.h"

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
		RenderStage::render();
	}
}