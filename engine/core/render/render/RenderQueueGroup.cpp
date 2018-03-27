#include "engine/core/memory/MemAllocDef.h"
#include "RenderQueueGroup.h"
#include "engine/core/io/DataStream.h"
#include "engine/core/util/LogManager.h"
#include "thirdparty/Rapidxml/rapidxml.hpp"
#include "thirdparty/Rapidxml/rapidxml_helper.hpp"
#include "thirdparty/Rapidxml/rapidxml_utils.hpp"
#include "render/Material.h"

namespace Echo
{
	static RenderQueueGroup* g_inst = nullptr;

	RenderQueueGroup* RenderQueueGroup::instance()
	{
		if (!g_inst)
		{
			g_inst = EchoNew(RenderQueueGroup);
		}

		return g_inst;
	}

	// add render queue
	RenderQueue* RenderQueueGroup::addRenderQueue(const String& name)
	{
		RenderQueue* renderQueue = EchoNew(RenderQueue)(name);
		m_renderQueues.push_back( renderQueue);

		return renderQueue;
	}

	// 根据材质模板添加渲染队列
	RenderQueue* RenderQueueGroup::addRenderQueue(const String& materialTemplate, const char* stage, const String& macros, bool isAlphaTest)
	{
		String queueName = String(stage) + "@" + String(isAlphaTest ? "AlphaTest" : "_") + "@" + macros + "@" + materialTemplate;
		RenderQueue* renderQueue = getRenderQueue(queueName);
		//if (!renderQueue)
		//{
		//	renderQueue = EchoNew(RenderQueue)(queueName);
		//	Material* pMaterial = renderQueue->getMaterial();
		//	pMaterial->loadFromFile(materialTemplate, macros);

		//	int offset = getRenderQueueIndex(stage) + 1;
		//	m_renderQueues.insert(m_renderQueues.begin() + offset, renderQueue);
		//}

		return renderQueue;
	}

	// 删除渲染队列
	void RenderQueueGroup::clear()
	{
		EchoSafeDeleteContainer(m_renderQueues, RenderQueue);
	}


	// 根据渲染队列名称获取渲染队列
	RenderQueue* RenderQueueGroup::getRenderQueue(const String& strQueueName) const
	{
		for (RenderQueue* queue : m_renderQueues)
		{
			if (queue->getName() == strQueueName)
				return queue;
		}

		return nullptr;
	}

	// 根据索引获取渲染队列
	RenderQueue* RenderQueueGroup::getRenderQueueByIndex(ui32 nIndex) const
	{
		if (nIndex >= m_renderQueues.size())
		{
			return NULL;
		}
		else
		{
			return m_renderQueues[nIndex];
		}
	}

	// 根据名称获取队列索引
	ui8 RenderQueueGroup::getRenderQueueIndex(const String& strQueueName) const
	{

		size_t nCount = m_renderQueues.size();
		for (size_t i = 0; i < nCount; ++i)
		{
			if (strQueueName == m_renderQueues[i]->getName())
			{
				return i;
			}
		}

		EchoLogError("Can not found RenderQueue [%s]", strQueueName.c_str());
		return -1;
	}

	// 渲染队列执行(包含endQueue)
	void RenderQueueGroup::execRenderQueue(const String& startQueue, const String& endQueue, bool includeEndQueue)
	{
		// 立即处理渲染队列中数据(直接渲染)
		size_t beginIdx = getRenderQueueIndex(startQueue);
		size_t endIdx = getRenderQueueIndex(endQueue);
		endIdx = includeEndQueue ? endIdx : endIdx - 1;
		for (size_t i = beginIdx; i <= endIdx; ++i)
		{
			Echo::RenderQueue* renderQuene = getRenderQueueByIndex(i);
			if (renderQuene)
				renderQuene->renderQueue();
		}
	}

	// 清空渲染队列(包含endQueue)
	void RenderQueueGroup::clearRenderQueue(const String& startQueue, const String& endQueue, bool includeEndQueue)
	{
		// 立即处理渲染队列中数据(直接渲染)
		size_t beginIdx = getRenderQueueIndex(startQueue);
		size_t endIdx = getRenderQueueIndex(endQueue);
		endIdx = includeEndQueue ? endIdx : endIdx - 1;
		for (size_t i = beginIdx; i <= endIdx; ++i)
		{
			Echo::RenderQueue* renderQuene = getRenderQueueByIndex(i);
			if (renderQuene)
				renderQuene->clearRenderables();
		}
	}

	void RenderQueueGroup::render()
	{
		for (RenderQueue* queue : m_renderQueues)
		{
			if (queue)
			{
				queue->renderQueue();
				queue->beginRender();
			}
		}
	}
}