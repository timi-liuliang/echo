#ifndef __ECHO_RENDERQUEUE_H__
#define __ECHO_RENDERQUEUE_H__

#include <engine/core/Memory/MemManager.h>
#include "ShaderProgram.h"

namespace Echo
{
	class Renderable;
	class Material;

	class ManualRenderListener
	{
	public:
		virtual void render() = 0;
		virtual void beginRender()=0;
	};

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
		void setManalRenderBegin(ManualRenderListener* mListener);
		void setManalRenderEnd(ManualRenderListener* mListener);

		bool hasRenderables(){ return !m_Renderables.empty(); }

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
		const String& getName() { return m_strName; }

		// 获取材质
		Material* getMaterial() const;

		// 设置为可渲染
		void enableRender() { m_bEnableRender = true; }
		
		// 暂停渲染
		void disableRender(){ m_bEnableRender = false; }

		// 是否可用
		bool isEnable() const { return m_bEnableRender; }

		// 是否进行排序
		//void setIsSort(bool isSort) { m_isSort = isSort; }

		void addGlobalParams(const String& name, ShaderParamType type, const void* value, size_t count);

		void setRenderWithoutMacros(const StringArray& macros,bool refresh = false);

	protected:
		String					m_strName;
		Material*				m_pMaterial;		// 材质
		RenderableList			m_Renderables;		
		ManualRenderListener*	m_ManualBegin;
		ManualRenderListener*	m_ManualEnd;
		bool					m_bEnableRender;
		//bool					m_isSort;			// 渲染前是否进行排序
		RenderQueue*			m_ManualRenderQueue;// 手动配置shader宏渲染 
		bool					m_isInitManualRQ;
		bool					m_useManualRQ;		//只作用一针（渲染完重置成false）

		// 着色器参数
		struct ShaderParam
		{
			String			name;	
			ShaderParamType type;
			const void*		value;
			ui32			count;  // shader constance register num.
		};
		std::vector<ShaderParam>		m_globalParams;
	};
}

#endif
