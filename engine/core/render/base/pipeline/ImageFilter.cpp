#include "engine/core/render/base/Renderer.h"
#include "ImageFilter.h"

namespace Echo
{
	ImageFilter::ImageFilter(RenderPipeline* pipeline, RenderStage* stage)
		: IRenderQueue(pipeline, stage)
	{
	}

	ImageFilter::~ImageFilter()
	{
	}

	void ImageFilter::render()
	{
		Renderer* render = Renderer::instance();
		if (render)
		{

		}
	}
}