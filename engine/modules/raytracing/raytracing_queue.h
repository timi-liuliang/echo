#pragma once

#include <engine/core/render/base/pipeline/irender_queue.h>
#include <engine/core/render/base/render_proxy.h>
#include <engine/core/scene/node.h>

namespace Echo
{
	class RaytracingQueue : public IRenderQueue
	{
		ECHO_CLASS(RaytracingQueue, IRenderQueue)

	public:
		RaytracingQueue() {}
		virtual ~RaytracingQueue();

		// render
		virtual void render();

	protected:
	};
}
