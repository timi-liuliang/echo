#pragma once

#include "deferred_lighting.h"

namespace Echo
{
	class PointLighting : public DeferredLighting
	{
		ECHO_CLASS(PointLighting, DeferredLighting)

	public:
		PointLighting();
		virtual ~PointLighting();

		// Process
		virtual void render(FrameBufferPtr& frameBuffer) override;

	protected:
	};
}