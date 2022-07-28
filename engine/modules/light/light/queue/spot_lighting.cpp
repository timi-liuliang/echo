#include "spot_lighting.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	SpotLighting::SpotLighting()
		: DeferredLighting()
	{
	}

	SpotLighting::~SpotLighting()
	{

	}

	void SpotLighting::bindMethods()
	{
	}

	void SpotLighting::render(FrameBufferPtr& frameBuffer)
	{
		onRenderBegin();


		onRenderEnd();
	}
}