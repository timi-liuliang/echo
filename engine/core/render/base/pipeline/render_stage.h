#pragma once

#include "render_queue.h"
#include "image_filter.h"

namespace Echo
{
	class RenderPipeline;
	class RenderStage : public Object
	{
		ECHO_CLASS(RenderStage, Object)

	public:
		RenderStage() {}
		RenderStage(RenderPipeline* pipeline);
		~RenderStage();

		// name
		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// frame buffer type
		const StringOption& getFrameBufferType() const { return m_frameBufferType; }
		void setFrameBufferType(const StringOption& type);

		// add render able
		void addRenderable(const String& name, RenderableID id);

		// on size
		void onSize(ui32 width, ui32 height);

		// process
		void render();

	public:
		// get pipeline
		void setPipeline(RenderPipeline* pipeline) { m_pipeline = pipeline; }
		RenderPipeline* getPipeline() { return m_pipeline; }

		// get render queues
		vector<IRenderQueue*>::type& getRenderQueues() { return m_renderQueues; }

		// add image filter
		ImageFilter* addImageFilter(const String& name);

		// add
		void addRenderQueue(IRenderQueue* queue, ui32 position=-1);

		// delete|remove
		void removeRenderQueue(IRenderQueue* renderQueue);
		void deleteRenderQueue(IRenderQueue* renderQueue);

	public:
		// load|save
		void parseXml(void* pugiNode);
		void saveXml(void* pugiNode);

	protected:
		String						m_name;
		RenderPipeline*				m_pipeline = nullptr;
		vector<IRenderQueue*>::type	m_renderQueues;
		StringOption				m_frameBufferType = StringOption("WindowSurface", { "OffScreen", "WindowSurface" });
		FrameBuffer*				m_frameBuffer = nullptr;
	};
}