#pragma once

#include "deferred_lighting.h"

namespace Echo
{
	class DirectLighting : public DeferredLighting
	{
		ECHO_CLASS(DirectLighting, DeferredLighting)

	public:
		DirectLighting();
		virtual ~DirectLighting();

		// Process
		virtual void render(FrameBufferPtr& frameBuffer) override;

	protected:
	};
}