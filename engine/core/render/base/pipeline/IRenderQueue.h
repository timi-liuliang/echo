#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class RenderPipeline;
	class RenderStage;
	class IRenderQueue
	{
	public:
		IRenderQueue() {}
		IRenderQueue(RenderPipeline* pipeline, RenderStage* stage) : m_pipeline(pipeline), m_stage(stage) {}
		virtual ~IRenderQueue() {}

		// render
		virtual void render()=0;

	protected:
		RenderPipeline* m_pipeline = nullptr;
		RenderStage*	m_stage = nullptr;
		String			m_name;
	};
}
