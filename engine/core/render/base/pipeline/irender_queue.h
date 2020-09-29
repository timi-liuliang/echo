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

		// name
		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// render
		virtual void render()=0;

	protected:
		String			m_name;
		RenderPipeline* m_pipeline = nullptr;
		RenderStage*	m_stage = nullptr;
	};
}
