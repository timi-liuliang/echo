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

		// 执行渲染(参数为交集渲染)
		void render();

		// 清空可渲染物
		void clear();

		// 获取渲染物数据
		ui32 getRenderableCount(){ return m_renderables.size(); }

		// 获取名称
		const String& getName() { return m_name; }

		// 设置为可渲染
		void setEnable( bool isEnable) { m_isEnable = isEnable; }

		// 是否可用
		bool isEnable() const { return m_isEnable; }

	private:
		// 添加渲染体
		void addRenderable(Renderable* pRenderable);

		// 根据距离排序
		void sortByPos();

	protected:
		String					m_name;
		RenderableList			m_renderables;		
		bool					m_isEnable;
	};
}
