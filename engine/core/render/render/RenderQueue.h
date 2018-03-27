#pragma once

#include <engine/core/Memory/MemManager.h>

namespace Echo
{
	class Renderable;

	/**
	 * 渲染队列
	 */
	class RenderQueue
	{
		friend class Renderable;
		typedef vector<int>::type RenderableList;

	public:
		RenderQueue(const String& strName);
		~RenderQueue();

		// 开始渲染
		void beginRender();

		bool hasRenderables(){ return !m_renderables.empty(); }

		void sortByPos();

	private:
		// 添加渲染体
		void addRenderable(Renderable* pRenderable);

	public:
		// 执行渲染(参数为交集渲染)
		void renderQueue();

		// 清空可渲染物
		void clearRenderables();

		// 获取名称
		const String& getName() { return m_name; }

		// 设置为可渲染
		void setEnable( bool isEnable) { m_isEnable = isEnable; }

		// 是否可用
		bool isEnable() const { return m_isEnable; }

	protected:
		String					m_name;
		RenderableList			m_renderables;		
		bool					m_isEnable;
	};
}
