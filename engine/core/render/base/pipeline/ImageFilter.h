#pragma once

#include "IRenderQueue.h"

namespace Echo
{
	class ImageFilter : public IRenderQueue
	{
	public:
		ImageFilter(RenderPipeline* pipeline, RenderStage* stage);
		virtual ~ImageFilter();

		// render
		virtual void render();

	protected:
	};
}
