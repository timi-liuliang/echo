#pragma once

#include <engine/core/memory/MemAllocDef.h>
#include <engine/core/render/base/RenderState.h>
#include <engine/core/render/base/Renderable.h>
#include <engine/core/scene/node.h>

namespace Echo
{
	class RenderPipeline;
	class RenderStage;
	class ImageFilter
	{
	public:
		ImageFilter(RenderPipeline* pipeline, RenderStage* stage);
		virtual ~ImageFilter();

		// render
		virtual void render();

		// name
		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

	protected:
		RenderPipeline*					m_pipeline;
		RenderStage*					m_stage;
		String							m_name;
	};
}
