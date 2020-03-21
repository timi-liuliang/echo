#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class RenderPipeline;
	class RenderStage;
	class IRenderQueue
	{
	public:
		IRenderQueue(RenderPipeline* pipeline, RenderStage* stage) : m_pipeline(pipeline), m_stage(stage) {}
		virtual ~IRenderQueue() {}

		// render
		virtual void render()=0;

		// name
		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

	protected:
		RenderPipeline* m_pipeline;
		RenderStage*	m_stage;
		String			m_name;
	};
}
