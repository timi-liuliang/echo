#include "Render/RenderQueue.h"
#include "Render/Renderer.h"
#include "Render/Renderable.h"
#include "Render/ShaderProgram.h"
#include "Engine/core/Scene/NodeTree.h"
#include "Engine/core/Render/MaterialInst.h"

namespace Echo
{
	bool compareByPos(int p1, int p2)
	{
		Renderable* pr1 = Renderer::instance()->getRenderable(p1);
		Renderable* pr2 = Renderer::instance()->getRenderable(p2);
		
		const Vector3& camPos = NodeTree::instance()->get3dCamera()->getPosition();
		float d1 = (pr1->m_worldPos - camPos).lenSqr();
		float d2 = (pr2->m_worldPos - camPos).lenSqr();

		return d1 > d2;
	}

	// 构造函数
	RenderQueue::RenderQueue(const String& strName)
		: m_name(strName)
		, m_isEnable(true)
	{
		m_renderables.reserve(100);
	}

	// 析构函数
	RenderQueue::~RenderQueue()
	{
	}

	// 添加渲染体
	void RenderQueue::addRenderable(Renderable* pRenderable)
	{
		m_renderables.push_back(pRenderable->getIdentifier());
	}

	void RenderQueue::sortByPos()
	{
		if (!m_renderables.empty())
			std::sort(m_renderables.begin(), m_renderables.end(), compareByPos);
	}

	// 执行渲染
	void RenderQueue::render()
	{
		if( !m_isEnable ) 
			return ;

		//sortByPos();

		for (int renderableID : m_renderables)
		{
			Renderable* pRenderable = Renderer::instance()->getRenderable(renderableID);
			if( pRenderable)
			{	
				pRenderable->render();
			}
		}
	}

	// 清空可渲染物
	void RenderQueue::clear()
	{
		m_renderables.clear();
	}
}
