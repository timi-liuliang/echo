#include "raytracing_queue.h"
#include "raytracing_world.h"

namespace Echo
{
	RaytracingQueue::~RaytracingQueue()
	{
	}

	void RaytracingQueue::bindMethods()
	{
	}

	void RaytracingQueue::render()
	{
		onRenderBegin();
		{
			RaytracingWorld::instance()->render();
		}
		onRenderEnd();
	}
}