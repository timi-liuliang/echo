#pragma once

#include "RenderQueue.h"

namespace Echo
{
	class RenderPipeline;
	class RenderStage
	{
	public:
		RenderStage(RenderPipeline* pipeline);
		~RenderStage();

		// name
		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// destroy
		void destroy();

		// add render able
		void addRenderable(const String& name, RenderableID id);

		// process
		void render();

	public:
		// load|save
		void parseXml(void* pugiNode);

	protected:
		String						m_name;
		RenderPipeline*				m_pipeline = nullptr;
		vector<RenderQueue*>::type	m_queues;
		ui32						m_frameBufferId = -1;
	};
}