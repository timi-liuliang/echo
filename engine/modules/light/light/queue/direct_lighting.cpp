#include "direct_lighting.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	DirectLighting::DirectLighting()
		: DeferredLighting()
	{
	}

	DirectLighting::~DirectLighting()
	{

	}

	void DirectLighting::bindMethods()
	{

	}

	void DirectLighting::render(FrameBufferPtr& frameBuffer)
	{
		onRenderBegin();


		onRenderEnd();
	}
}