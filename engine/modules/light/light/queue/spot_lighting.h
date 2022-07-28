#pragma once

#include "deferred_lighting.h"

namespace Echo
{
	class SpotLighting : public DeferredLighting
	{
		ECHO_CLASS(SpotLighting, DeferredLighting)

	public:
		SpotLighting();
		virtual ~SpotLighting();

		// Process
		virtual void render(FrameBufferPtr& frameBuffer) override;

	protected:
	};
}