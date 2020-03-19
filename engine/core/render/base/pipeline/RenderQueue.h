#pragma once

#include <engine/core/memory/MemAllocDef.h>
#include <engine/core/render/base/RenderState.h>
#include <engine/core/render/base/Renderable.h>
#include <engine/core/scene/node.h>

namespace Echo
{
	class RenderPipeline;
	class RenderStage;
	class RenderQueue
	{
	public:
		RenderQueue(RenderPipeline* pipeline, RenderStage* stage);
		virtual ~RenderQueue();

		// render
		virtual void render();

		// add render able
		void addRenderable(RenderableID id) { m_renderables.push_back(id); }

		// name
		void setName(const String& name) { m_name = name; }
		const String& getName() const { return m_name; }

		// sort
		void setSort(bool isSort) { m_isSort = isSort; }
		bool isSort() const { return m_isSort; }

	protected:
		RenderPipeline*					m_pipeline;
		RenderStage*					m_stage;
		String							m_name;
		bool							m_isSort;
		vector<RenderableID>::type		m_renderables;
	};
}
