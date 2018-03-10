#include "Render/Renderable.h"
#include "Render/ShaderProgram.h"
#include "Render/Renderer.h"
#include "Render/Material.h"

namespace Echo
{
	// 构造函数
	Renderable::Renderable(RenderQueue* pRenderQueue, Material* material, int identifier)
		: m_RenderInput(NULL)
		, m_SParamWriteIndex(0)
		, m_LowSParamWriteIndex(0)
		, m_pRenderQueue(pRenderQueue)
		, m_pLowRenderQueue(NULL)
		, m_stageRenderQueue(NULL)
		, m_visible(NULL)
		, m_bRenderState(false)
		, m_pBlendState(NULL)
		, m_pRasterizerState(NULL)
		, m_pDepthStencil(NULL)
		, m_identifier(identifier)
		, m_useDefaultShaderParams(false)
	{
		m_materialID = (pRenderQueue ? pRenderQueue->getMaterial() : material)->getIdentifier();
	}

	// 析构函数
	Renderable::~Renderable()
	{
		m_ShaderParams.clear();
		
		m_LowShaderParams.clear();
	}

	// 开始绑定shader参数
	void Renderable::beginShaderParams(size_t paramNum)
	{
		m_ShaderParams.resize(paramNum);
		m_SParamWriteIndex = 0;
	}

	// 结束变量绑定
	void Renderable::endShaderParams()
	{
		if (m_SParamWriteIndex != m_ShaderParams.size())
		{
			EchoLogError("[Renderable:%d]:: index == %d, shader size == %d  %s", __LINE__, m_SParamWriteIndex, m_ShaderParams.size(), m_ownerDesc.c_str());
		}
	}

	// 设置参数
	void Renderable::setShaderParam(size_t physicsIndex, ShaderParamType type, const void* param, size_t num/* =1 */)
	{
		if (m_SParamWriteIndex < m_ShaderParams.size() && param)
		{
			m_ShaderParams[m_SParamWriteIndex].physicsIndex = physicsIndex;
			m_ShaderParams[m_SParamWriteIndex].stype = type;
			m_ShaderParams[m_SParamWriteIndex].pData = param;
			m_ShaderParams[m_SParamWriteIndex].ParamsLength = num;
			m_SParamWriteIndex++;
		}
		else
		{
			EchoLogError("Renderable::setShaderParam failed %s", m_ownerDesc.c_str());
			EchoAssert(false);
		}
	}

	void Renderable::modifyShaderParam(ui32 physics, ShaderParamType type, void* param, size_t num/* =1 */)
	{
		EchoAssert(m_SParamWriteIndex == m_ShaderParams.size());
		ui32 modifyIndex = 0;
		for (modifyIndex; modifyIndex < m_ShaderParams.size(); modifyIndex++)
		{
			if (m_ShaderParams[modifyIndex].physicsIndex == physics)
				break;
		}
		EchoAssert(modifyIndex < m_ShaderParams.size());
		EchoAssert(m_ShaderParams[modifyIndex].physicsIndex == physics);

		m_ShaderParams[modifyIndex].stype = type;
		m_ShaderParams[modifyIndex].pData = param;
		m_ShaderParams[modifyIndex].ParamsLength = num;
	}

	void Renderable::modifyLowShaderParam(ui32 physics, ShaderParamType type, void* param, ui32 id, size_t num/* =1 */)
	{
		ui32 modifyIndex = 0;
		for (modifyIndex; modifyIndex < m_LowShaderParams[id].size(); modifyIndex++)
		{
			if (m_LowShaderParams[id][modifyIndex].physicsIndex == physics)
				break;
		}
		EchoAssert(modifyIndex < m_LowShaderParams[id].size());
		EchoAssert(m_LowShaderParams[id][modifyIndex].physicsIndex == physics);

		//m_LowShaderParams[id][modifyIndex].stype = type;
		m_LowShaderParams[id][modifyIndex].pData = param;
		//m_LowShaderParams[id][modifyIndex].ParamsLength = num;
	}

	// 向纹理槽中设置纹理
	void Renderable::setTexture(ui32 stage, Texture* texture, const SamplerState* state)
	{
		m_Textures[stage].m_texture = texture;
		if (NULL != texture && NULL != state && texture->getNumMipmaps() > 1)
		{
			SamplerState::SamplerDesc desc = state->getDesc();
			desc.mipFilter = SamplerState::FO_POINT;
			m_Textures[stage].m_samplerState = Renderer::instance()->getSamplerState(desc);
		}
		else
		{
			m_Textures[stage].m_samplerState = state;
		}
	}

	// 绑定参数
	void Renderable::bindShaderParams()
	{
		// 向纹理槽中设置纹理
		bindTextures();

		// 遍历设置所有参数
		if (m_pLowRenderQueue && !m_useDefaultShaderParams )
		{
			ShaderProgram* shaderProgram = m_pLowRenderQueue->getMaterial()->getShaderProgram();
			for (size_t i = 0; i < m_LowShaderParams[m_LowShaderID].size(); ++i)
			{
				ShaderParam& param = m_LowShaderParams[m_LowShaderID][i];
				switch (param.stype)
				{
				case SPT_VEC4:
				case SPT_MAT4:
				case SPT_INT:
				case SPT_FLOAT:
				case SPT_VEC2:
				case SPT_VEC3:
				case SPT_TEXTURE:
				{
					shaderProgram->setUniform(param.physicsIndex, param.pData, param.stype, param.ParamsLength);
				}
				break;
// 				default:
// 				{
// 					EchoAssertX(0, "unknow shader param format!");
// 				}
// 					break;
				}
			}
		}
		else
		{
			Material* material = m_useDefaultShaderParams ? m_pLowRenderQueue->getMaterial() : IdToPtr(Material, m_materialID);
			if (material)
			{
				ShaderProgram* shaderProgram = material->getShaderProgram();
				for (size_t i = 0; i < m_ShaderParams.size(); ++i)
				{
					ShaderParam& param = m_ShaderParams[i];
					switch (param.stype)
					{
					case SPT_VEC4:
					case SPT_MAT4:
					case SPT_INT:
					case SPT_FLOAT:
					case SPT_VEC2:
					case SPT_VEC3:
					case SPT_TEXTURE:
					{
						shaderProgram->setUniform(param.physicsIndex, param.pData, param.stype, param.ParamsLength);
						break;
					}

					default:
					{
						EchoAssertX(0, "unknow shader param format! %s", m_ownerDesc.c_str());
						break;
					}

					}
				}
				m_useDefaultShaderParams = false;
			}
		}
	}

	// 绑定纹理
	void Renderable::bindTextures()
	{
		for (size_t i = 0; i < MAX_TEXTURE_SAMPLER; ++i)
		{
			Texture* texture = m_Textures[i].m_texture;
			if (texture)
			{
				Renderer::instance()->setTexture(i, m_Textures[i]);
			}
// 			else
// 			{
// 				Renderer::instance()->setTexture(i, TextureSampler());
// 			}
		}
	}

	// 绑定渲染状态
	void Renderable::bindRenderState()
	{
		Material* material = IdToPtr(Material, m_materialID);
		if (material)
		{
			Renderer* pRenderer = Renderer::instance();
			pRenderer->setDepthStencilState(m_pDepthStencil ? m_pDepthStencil : material->getDepthState());
			pRenderer->setRasterizerState(m_pRasterizerState ? m_pRasterizerState : material->getRasterizerState());
			pRenderer->setBlendState(m_pBlendState ? m_pBlendState : material->getBlendState());
		}
	}

	// 执行渲染
	void Renderable::render()
	{
		if (m_pLowRenderQueue)
		{
			// 绑定着色器
			m_pLowRenderQueue->getMaterial()->activeShader();

			// 绑定渲染状态
			bindRenderState();

			// 绑定参数
			bindShaderParams();

			// 执行渲染
			EchoAssert(m_RenderInput);
			Renderer::instance()->render(m_RenderInput, m_pLowRenderQueue->getMaterial()->getShaderProgram());

// 			m_pLowRenderQueue = NULL;
// 			m_useDefaultShaderParams = false;
		}
		else
		{
			Material* material = IdToPtr(Material, m_materialID);
			if (m_pRenderQueue)
			{
				material = m_pRenderQueue->getMaterial();
			}
			if (material)
			{
				// 绑定着色器
				material->activeShader();

				// 绑定渲染状态
				bindRenderState();

				// 绑定参数
				bindShaderParams();

				// 执行渲染
				EchoAssert(m_RenderInput);
				Renderer::instance()->render(m_RenderInput, material->getShaderProgram());
			}
		}
	}

	void Renderable::setLowRenderQueue(RenderQueue* pRenderQueue, int id, int reSetPhysicsIndex)
	{
		m_pLowRenderQueue = pRenderQueue; 
		m_LowShaderID = id;
		if (reSetPhysicsIndex)
		{
			m_LowShaderParams[id].clear();
			m_LowShaderParams[id].resize(reSetPhysicsIndex);
			m_LowSParamWriteIndex.push_back(0);
		}
	}

	// 设置
	void Renderable::setStageRenderQueue(RenderQueue* stageRenderQueue)
	{
		m_stageRenderQueue = stageRenderQueue;
	}

	void Renderable::setLowShaderParam(ui32 id, size_t physicsIndex, ShaderParamType type, const void* param, size_t num /*= 1*/)
	{
		if (m_LowSParamWriteIndex[id] < m_LowShaderParams[id].size() && param)
		{
			m_LowShaderParams[id][m_LowSParamWriteIndex[id]].physicsIndex = physicsIndex;
			m_LowShaderParams[id][m_LowSParamWriteIndex[id]].stype = type;
			m_LowShaderParams[id][m_LowSParamWriteIndex[id]].pData = param;
			m_LowShaderParams[id][m_LowSParamWriteIndex[id]].ParamsLength = num;
			++m_LowSParamWriteIndex[id];
		}
		else
		{
			EchoLogError("Renderable::setShaderParam failed %s", m_ownerDesc.c_str());
			EchoAssert(false);
		}
	}

	// 提交到渲染队列
	void Renderable::submitToRenderQueue()
	{
		if (m_stageRenderQueue)
		{
			m_stageRenderQueue->addRenderable(this);
		}
		else if (m_pLowRenderQueue)
		{
			m_pLowRenderQueue->addRenderable(this);
		}
		else
		{
			m_pRenderQueue->addRenderable(this);
		}	
	}

	// 设置混合状态
	void Renderable::setBlendState(BlendState* pState)
	{
		m_pBlendState = pState;
		m_bRenderState = pState ? true : false;
	}

	// 设置光栅化状态
	void Renderable::setRasterizerState(RasterizerState* state)
	{
		m_pRasterizerState = state;
		m_bRenderState = state ? true : false;
	}

	// 设置深度模板状态
	void Renderable::setDepthStencilState(DepthStencilState* state)
	{
		m_pDepthStencil = state;
		m_bRenderState = state ? true : false;
	}
}