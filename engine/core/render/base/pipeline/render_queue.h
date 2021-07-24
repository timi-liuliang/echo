#pragma once

#include "irender_queue.h"
#include <engine/core/render/base/proxy/render_proxy.h>
#include <engine/core/scene/node.h>

namespace Echo
{
	class RenderQueue : public IRenderQueue
	{
		ECHO_CLASS(RenderQueue, IRenderQueue)

	public:
		RenderQueue() {}
		virtual ~RenderQueue();

		// render
		virtual void render(FrameBufferPtr& frameBuffer);

		// add render able
		void addRenderable(RenderableID id) { m_renderables.emplace_back(id); }

		// sort
		void setSort(bool isSort) { m_sort = isSort; }
		bool isSort() const { return m_sort; }

		// filter
		void setCameraFilter(i32 filter) { m_cameraFilter = filter; }
		i32 getCameraFilter() const { return m_cameraFilter; }

	protected:
		bool							m_sort;
		i32								m_cameraFilter = 0xFFFFFFFF;
		vector<RenderableID>::type		m_renderables;
	};
}
