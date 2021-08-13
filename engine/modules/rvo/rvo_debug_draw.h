#pragma once

#include "engine/core/base/object.h"
#include "rvo_agent.h"

namespace Echo
{
	class RvoDebugDraw
	{
	public:
		RvoDebugDraw();
		virtual ~RvoDebugDraw();

		// update
		void update(float elapsedTime);

	private:
	};
}
