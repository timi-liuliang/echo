#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	class RenderPipeline;
	class RenderPass;
	class IRenderQueue : public Object
	{
		ECHO_VIRTUAL_CLASS(IRenderQueue, Object);

	public:
		IRenderQueue() {}
		IRenderQueue(RenderPass* stage) : m_stage(stage) {}
		virtual ~IRenderQueue() {}

		// name
		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// enable
		bool isEnable() const { return m_enable; }
		void setEnable(bool enable) { m_enable = enable; }

		// stage
		void setStage(RenderPass* stage) { m_stage = stage; }
		RenderPass* getStage() { return m_stage; }

		// render
		virtual void render() {}

	protected:
		String			m_name;
		bool			m_enable = true;
		RenderPass*	m_stage = nullptr;
	};
}
