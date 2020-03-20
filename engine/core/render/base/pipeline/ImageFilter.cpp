#include "../Renderable.h"
#include "engine/core/scene/render_node.h"
#include "../Renderer.h"
#include "ImageFilter.h"

namespace Echo
{
	ImageFilter::ImageFilter(RenderPipeline* pipeline, RenderStage* stage)
		: m_pipeline(pipeline)
		, m_stage(stage)
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