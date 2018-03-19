#pragma once

#include "RenderQueue.h"

namespace Echo
{
	class RenderQueueGroup
	{
	public:
		RenderQueueGroup() {}
		~RenderQueueGroup() {}

		// get instance
		static RenderQueueGroup* instance();

		// add render queue
		RenderQueue* addRenderQueue(const String& name);

		// 根据材质模板添加渲染队列
		RenderQueue* addRenderQueue(const String& materialTemplate, const char* stage, const String& macros, bool isAlphaTest);

		// 根据渲染队列名称获取渲染队列
		RenderQueue* getRenderQueue(const String& strQueueName) const;

		// 根据索引获取渲染队列
		RenderQueue* getRenderQueueByIndex(ui32 nIndex) const;

		// 根据名称获取队列索引
		ui8 getRenderQueueIndex(const String& strQueueName) const;

		// 获取队列数量
		size_t getRenderQueueCount() { return m_renderQueues.size(); }

		// 渲染队列执行(包含endQueue)
		void execRenderQueue(const String& startQueue, const String& endQueue, bool includeEndQueue = false);

		// 清空渲染队列(包含endQueue)
		void clearRenderQueue(const String& startQueue, const String& endQueue, bool includeEndQueue = false);

		// clear
		void clear();

		// render
		void render();

	private:
		vector<RenderQueue*>::type  m_renderQueues;
	};
}