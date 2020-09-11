#pragma once

#include "IRenderQueue.h"
#include <engine/core/render/base/Renderable.h>
#include <engine/core/scene/node.h>

namespace Echo
{
	class RenderQueue : public IRenderQueue
	{
	public:
		RenderQueue(RenderPipeline* pipeline, RenderStage* stage);
		virtual ~RenderQueue();

		// render
		virtual void render();

		// add render able
		void addRenderable(RenderableID id) { m_renderables.emplace_back(id); }

		// sort
		void setSort(bool isSort) { m_isSort = isSort; }
		bool isSort() const { return m_isSort; }

	protected:
		bool							m_isSort;
		vector<RenderableID>::type		m_renderables;
	};
}
