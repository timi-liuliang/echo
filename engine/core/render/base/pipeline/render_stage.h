#pragma once

#include "render_queue.h"
#include "image_filter.h"
#include "base/buffer/frame_buffer.h"

namespace Echo
{
	class RenderPipeline;
	class RenderStage : public Object
	{
		ECHO_CLASS(RenderStage, Object)

	public:
		RenderStage() {}
		~RenderStage();

		// name
		virtual void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }
		
		// enable
		void setEnable(bool enable) { m_enable = enable; }
		bool isEnable() const { return m_enable; }

		// editor only
		void setEditorOnly(bool editorOnly) { m_editorOnly = editorOnly; }
		bool isEditorOnly() const { return m_editorOnly; }

		// frame buffer
		FrameBuffer* getFrameBuffer() const { return m_frameBuffer; }
		void setFrameBuffer(Object* fb) { m_frameBuffer = (FrameBuffer*)fb; }

		// add render able
		void addRenderable(const String& name, RenderableID id);

		// on size
		void onSize(ui32 width, ui32 height);

		// process
		virtual void render();

	public:
		// get pipeline
		void setPipeline(RenderPipeline* pipeline) { m_pipeline = pipeline; }
		RenderPipeline* getPipeline() { return m_pipeline; }

		// get render queues
		vector<IRenderQueue*>::type& getRenderQueues() { return m_renderQueues; }

		// add
		void addRenderQueue(IRenderQueue* queue, ui32 position=-1);

		// delete|remove
		void removeRenderQueue(IRenderQueue* renderQueue);
		void deleteRenderQueue(IRenderQueue* renderQueue);

		// load|save
		void parseXml(void* pugiNode);
		void saveXml(void* pugiNode);

	public:
		// Modified signal
		DECLARE_SIGNAL(Signal0, onRenderBegin)
		DECLARE_SIGNAL(Signal0, onRenderEnd)

	protected:
		String						m_name;
		bool						m_enable = true;
		bool						m_editorOnly = false;
		RenderPipeline*				m_pipeline = nullptr;
		vector<IRenderQueue*>::type	m_renderQueues;
		FrameBufferPtr				m_frameBuffer;
	};
}