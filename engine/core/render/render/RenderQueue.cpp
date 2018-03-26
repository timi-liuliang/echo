#include "Render/RenderQueue.h"
#include "Render/Renderer.h"
#include "Render/Renderable.h"
#include "Render/ShaderProgram.h"
#include "engine/core/Util/LogManager.h"
#include "Render/Material.h"
#include "Engine/core/Scene/NodeTree.h"
#include "Engine/core/Render/MaterialInst.h"
#include "RenderQueueGroup.h"

namespace Echo
{
	bool compareByPos(int p1, int p2)
	{
		Renderable* pr1 = Renderer::instance()->getRenderable(p1);
		Renderable* pr2 = Renderer::instance()->getRenderable(p2);
		
		const Vector3& camPos = NodeTree::instance()->getMainCamera()->getPosition();
		float d1 = (pr1->m_worldPos - camPos).lenSqr();
		float d2 = (pr2->m_worldPos - camPos).lenSqr();

		return d1 > d2;
	}

	// 构造函数
	RenderQueue::RenderQueue(const String& strName)
		: m_strName(strName)
		, m_ManualBegin(NULL)
		, m_ManualEnd(NULL)
		, m_bEnableRender(true)
		, m_ManualRenderQueue(NULL)
		, m_isInitManualRQ(false)
		, m_useManualRQ(false)
	{
		m_pMaterial = MaterialManager::instance()->createMaterial();
		m_pMaterial->setQueue(this);
		m_Renderables.reserve(100);
	}

	// 析构函数
	RenderQueue::~RenderQueue()
	{
		MaterialManager::instance()->destroyMaterial(m_pMaterial);
	}

	// 获取材质
	Material* RenderQueue::getMaterial() const 
	{
		return m_pMaterial;
	}

	void RenderQueue::beginRender()
	{
		m_Renderables.clear();

		if(m_ManualEnd)
			m_ManualEnd->beginRender();
	}

	// 添加渲染体
	void RenderQueue::addRenderable(Renderable* pRenderable)
	{
		//EchoAssert(m_pMaterial->getVertexStrite() == pRenderable->getRenderInput()->getVertexStride());
		m_Renderables.push_back(pRenderable->getIdentifier());
	}

	void RenderQueue::setManalRenderBegin(ManualRenderListener* mListener)
	{
		m_ManualBegin = mListener;
	}

	void RenderQueue::setManalRenderEnd(ManualRenderListener* mListener)
	{
		m_ManualEnd = mListener;
	}

	void RenderQueue::sortByPos()
	{
		if (m_Renderables.empty())
		{
			return;
		}

		std::sort(m_Renderables.begin(), m_Renderables.end(), compareByPos);
	}

	// 执行渲染
	void RenderQueue::renderQueue()
	{
		if( !m_bEnableRender ) return ;

		if(m_ManualBegin)
			m_ManualBegin->render();

		if(!m_Renderables.empty())
		{
			// ！！！ 排序已无意义
			// sort the render queue by the texture.
			//if (m_isSort)
			//	std::sort(m_Renderables.begin(), m_Renderables.end(), compare);

			// 全局变量，类似效果，开放获取Renderable接口，由客户端直接设置!引擎不提供某材质全局变量的概念。
			// then set the shader,render states.
// 			Renderer* pRenderer = Renderer::instance();
 			ShaderProgram* pShaderProgram = m_pMaterial->getShaderProgram();
 			for (ui32 i = 0; i < m_globalParams.size(); ++i)
 			{
 				RenderQueue::ShaderParam& param = m_globalParams[i];
 				pShaderProgram->setUniform(param.name.c_str(), param.value, param.type, param.count);
 			}

			// 执行渲染
			bool manualRender = m_ManualRenderQueue && m_useManualRQ;

			for (int renderableID : m_Renderables)
			{
				Renderable* pRenderable = Renderer::instance()->getRenderable(renderableID);
				if( pRenderable)
				{	
					if (manualRender)
					{
						pRenderable->setLowRenderQueue(m_ManualRenderQueue);
						pRenderable->setUseDefaultShaderParams(true);
					}

					pRenderable->render();
				}
			}

			m_useManualRQ = false;
		}

		if(m_ManualEnd)
			m_ManualEnd->render();
	}

	// 清空可渲染物
	void RenderQueue::clearRenderables()
	{
		m_Renderables.clear();
	}

	void RenderQueue::addGlobalParams(const String& name, ShaderParamType type, const void* value, size_t count)
	{
		ShaderParam p;
		p.name	= name;
		p.type	= type;
		p.value = value;
		p.count = count;
		m_globalParams.push_back(p);
	}

	void RenderQueue::setRenderWithoutMacros(const StringArray& macros, bool refresh /*= false*/)
	{
		m_useManualRQ = true;

		if (refresh || (!m_isInitManualRQ && !m_ManualRenderQueue))
		{
			StringArray queueName = StringUtil::Split(m_strName,"@");
			
			if (queueName.size() == 4)
			{
				bool isfind = false;
				String	generatedMacros;
				StringArray currentMacros = StringUtil::Split(queueName[2], "#define ");
				StringArray tmp = currentMacros;
				currentMacros.clear();
				String curMacro = "";
				for (size_t i = 0; i < tmp.size(); ++i)
				{
					String tmpMacro = tmp[i];
					bool result = StringUtil::EndWith(tmpMacro, "\n");
					if (result)
					{
						curMacro += tmpMacro;
						currentMacros.push_back(curMacro);
						curMacro = "";
					}
					else
					{
						curMacro = tmpMacro + " ";
					}
				}

				for (size_t i = 0; i < currentMacros.size();++i)
				{
					if (currentMacros[i] == macros[0])
					{
						isfind = true;
						continue;
					}
					generatedMacros += ("#define " + currentMacros[i]);
				}

				if (isfind)
				{
					String manualQueueName = queueName[0] + "@" + queueName[1] + "@" + generatedMacros + "@" + queueName[3];
					m_ManualRenderQueue = RenderQueueGroup::instance()->getRenderQueue(manualQueueName);
					if (!m_ManualRenderQueue)
					{
						m_ManualRenderQueue = RenderQueueGroup::instance()->addRenderQueue(queueName[3], queueName[0].c_str(), generatedMacros, queueName[1] == "_" ? false : true);
					}
				}
			}

			m_isInitManualRQ = true;
		}
	}

}
