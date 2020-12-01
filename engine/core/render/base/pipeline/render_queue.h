#pragma once

#include "irender_queue.h"
#include <engine/core/render/base/renderable.h>
#include <engine/core/scene/node.h>

namespace Echo
{
	class RenderQueue : public IRenderQueue
	{
		ECHO_CLASS(RenderQueue, IRenderQueue)

	public:
		RenderQueue() {}
		RenderQueue(RenderPass* stage);
		virtual ~RenderQueue();

		// render
		virtual void render();

		// add render able
		void addRenderable(RenderableID id) { m_renderables.emplace_back(id); }

		// sort
		void setSort(bool isSort) { m_sort = isSort; }
		bool isSort() const { return m_sort; }

	protected:
		bool							m_sort;
		vector<RenderableID>::type		m_renderables;
	};
}
