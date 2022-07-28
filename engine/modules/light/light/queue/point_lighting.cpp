#include "point_lighting.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	PointLighting::PointLighting()
		: DeferredLighting()
	{
	}

	PointLighting::~PointLighting()
	{

	}

	void PointLighting::bindMethods()
	{

	}

	void PointLighting::render(FrameBufferPtr& frameBuffer)
	{
		onRenderBegin();


		onRenderEnd();
	}
}