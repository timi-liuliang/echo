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

		// destroy
		void destroy() {}

		// add render able
		void addRenderable(const String& name, RenderableID id);

		// process
		void render();

	public:
		// get pipeline
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
		ui32						m_frameBufferId = 0;
	};
}