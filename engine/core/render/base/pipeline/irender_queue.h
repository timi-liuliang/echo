#pragma once

#include "engine/core/base/object.h"
#include "base/buffer/frame_buffer.h"

namespace Echo
{
	class RenderPipeline;
	class RenderStage;
	class IRenderQueue : public Object
	{
		ECHO_VIRTUAL_CLASS(IRenderQueue, Object);

	public:
		IRenderQueue() {}
		//IRenderQueue(RenderStage* stage) : m_stage(stage) {}
		virtual ~IRenderQueue() {}

		// name
		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// enable
		bool isEnable() const { return m_enable; }
		void setEnable(bool enable) { m_enable = enable; }

		// stage
		void setStage(RenderStage* stage) { m_stage = stage; }
		RenderStage* getStage() { return m_stage; }

		// render
		virtual void render(FrameBufferPtr& frameBuffer) {}

	public:
		// Modified signal
		DECLARE_SIGNAL(Signal0, onRenderBegin)
		DECLARE_SIGNAL(Signal0, onRenderEnd)

	protected:
		String			m_name;
		bool			m_enable = true;
		RenderStage*	m_stage = nullptr;
	};
}
