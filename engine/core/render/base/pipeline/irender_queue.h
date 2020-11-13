#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	class RenderPipeline;
	class RenderStage;
	class IRenderQueue : public Object
	{
		ECHO_VIRTUAL_CLASS(IRenderQueue, Object);

	public:
		IRenderQueue() {}
		IRenderQueue(RenderStage* stage) : m_stage(stage) {}
		virtual ~IRenderQueue() {}

		// name
		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// stage
		void setStage(RenderStage* stage) { m_stage = stage; }
		RenderStage* getStage() { return m_stage; }

		// render
		virtual void render() {}

	protected:
		String			m_name;
		RenderStage*	m_stage = nullptr;
	};
}
