#include "Render/Renderable.h"
#include "Render/ShaderProgram.h"
#include "Render/Renderer.h"
#include "Render/ShaderProgramRes.h"
#include "engine/core/render/renderstage/RenderStage.h"
#include "engine/core/render/Material.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/scene/render_node.h"

namespace Echo
{
	// 构造函数
	Renderable::Renderable(const String& renderStage, ShaderProgramRes* shader, int identifier)
		: m_renderStage(renderStage)
		, m_renderInput(nullptr)
		, m_SParamWriteIndex(0)
		, m_bRenderState(false)
		, m_pBlendState(NULL)
		, m_pRasterizerState(NULL)
		, m_pDepthStencil(NULL)
		, m_identifier(identifier)
	{
		m_shaderProgram = shader;
	}

	// 析构函数
	Renderable::~Renderable()
	{
		EchoSafeDelete(m_renderInput, RenderInput);

		m_shaderParams.clear();
	}

	// release
	void Renderable::release()
	{
		Renderable* ptr = this;
		Renderer::instance()->destroyRenderables(&ptr, 1);
	}

	// 新建
	Renderable* Renderable::create(Mesh* mesh, Material* matInst, Render* node)
	{
		ShaderProgramRes* material = matInst->getShader();
		ShaderProgram* shaderProgram = material->getShaderProgram();
		if (!shaderProgram)
			return nullptr;

		ShaderProgram::UniformArray* uniforms = shaderProgram->getUniforms();

		// bind shader param
		Renderable* renderable = Renderer::instance()->createRenderable(matInst->getRenderStage(), matInst->getShader());
		renderable->setNode(node);
		renderable->setRenderInput(mesh->getVertexBuffer(), mesh->getVertexElements(), mesh->getIndexBuffer(), mesh->getIndexStride(), mesh->getTopologyType());
		renderable->beginShaderParams(uniforms->size());
		for (auto& it : *uniforms)
		{
			const ShaderProgram::Uniform& uniform = it.second;
			if (uniform.m_type != SPT_TEXTURE)
			{
				void* value = node ? node->getGlobalUniformValue(uniform.m_name) : nullptr;
				if (!value) value = matInst->getUniformValue(uniform.m_name);

				renderable->setShaderParam(shaderProgram->getParamPhysicsIndex(uniform.m_name), uniform.m_type, value, uniform.m_count);
			}
			else
			{
				i32* slotIdxPtr  = (i32*)matInst->getUniformValue(uniform.m_name);
				i32  slotIdx = *slotIdxPtr;
				i32* globalTexture = (i32*)node->getGlobalUniformValue(uniform.m_name);
				if (!globalTexture)
				{
					TextureRes* textureRes = matInst->getTexture(slotIdx);
					Texture* texture = textureRes ? textureRes->getTexture() : nullptr;
					const SamplerState* sampleState = Renderer::instance()->getSamplerState(SamplerState::SamplerDesc());
					renderable->setTexture(slotIdx, texture, sampleState);
				}
				else
				{
					renderable->setTexture(slotIdx, *globalTexture, Renderer::instance()->getSamplerState(SamplerState::SamplerDesc()));
				}

				renderable->setShaderParam(shaderProgram->getParamPhysicsIndex(uniform.m_name), uniform.m_type, slotIdxPtr, uniform.m_count);
			}
		}
		renderable->endShaderParams();

		//renderable->setRasterizerState(matInst->getRasterizerState());
		//renderable->setBlendState(matInst->getBlendState());
		//renderable->setDepthStencilState(matInst->getDepthStencilState());

		return renderable;
	}

	// 开始绑定shader参数
	void Renderable::beginShaderParams(size_t paramNum)
	{
		m_shaderParams.resize(paramNum);
		m_SParamWriteIndex = 0;
	}

	// 结束变量绑定
	void Renderable::endShaderParams()
	{
		if (m_SParamWriteIndex != m_shaderParams.size())
		{
			EchoLogError("[Renderable:%d]:: index == %d, shader size == %d Node [%s]", __LINE__, m_SParamWriteIndex, m_shaderParams.size(), m_node->getName().c_str());
		}
	}

	// 设置参数
	void Renderable::setShaderParam(size_t physicsIndex, ShaderParamType type, const void* param, size_t num/* =1 */)
	{
		if (m_SParamWriteIndex < m_shaderParams.size() && param)
		{
			m_shaderParams[m_SParamWriteIndex].physicsIndex = physicsIndex;
			m_shaderParams[m_SParamWriteIndex].stype = type;
			m_shaderParams[m_SParamWriteIndex].pData = param;
			m_shaderParams[m_SParamWriteIndex].ParamsLength = num;
			m_SParamWriteIndex++;
		}
		else
		{
			EchoLogError("Renderable::setShaderParam failed %s", m_node->getName().c_str());
		}
	}

	void Renderable::modifyShaderParam(ui32 physics, ShaderParamType type, void* param, size_t num/* =1 */)
	{
		EchoAssert(m_SParamWriteIndex == m_shaderParams.size());
		ui32 modifyIndex = 0;
		for (modifyIndex; modifyIndex < m_shaderParams.size(); modifyIndex++)
		{
			if (m_shaderParams[modifyIndex].physicsIndex == physics)
				break;
		}
		EchoAssert(modifyIndex < m_shaderParams.size());
		EchoAssert(m_shaderParams[modifyIndex].physicsIndex == physics);

		m_shaderParams[modifyIndex].stype = type;
		m_shaderParams[modifyIndex].pData = param;
		m_shaderParams[modifyIndex].ParamsLength = num;
	}

	// 向纹理槽中设置纹理
	void Renderable::setTexture(ui32 stage, Texture* texture, const SamplerState* state)
	{
		m_textures[stage].m_texture = texture;
		m_textures[stage].m_samplerState = state;
	}

	// use global texture
	void Renderable::setTexture(ui32 stage, ui32 globalTexture, const SamplerState* state)
	{
		m_textures[stage].m_globalTexture = globalTexture;
		m_textures[stage].m_samplerState = state;
	}

	// 绑定参数
	void Renderable::bindShaderParams()
	{
		// 向纹理槽中设置纹理
		bindTextures();
			
		if (m_shaderProgram)
		{
			ShaderProgram* shaderProgram = m_shaderProgram->getShaderProgram();
			for (size_t i = 0; i < m_shaderParams.size(); ++i)
			{
				ShaderParam& param = m_shaderParams[i];
				switch (param.stype)
				{
				case SPT_VEC4:
				case SPT_MAT4:
				case SPT_INT:
				case SPT_FLOAT:
				case SPT_VEC2:
				case SPT_VEC3:
				case SPT_TEXTURE:	shaderProgram->setUniform(param.physicsIndex, param.pData, param.stype, param.ParamsLength);	break;
				default:			EchoLogError("unknow shader param format! %s", m_node->getName().c_str());							break;
				}
			}
		}
	}

	// 绑定纹理
	void Renderable::bindTextures()
	{
		for (size_t i = 0; i < MAX_TEXTURE_SAMPLER; ++i)
		{
			Texture* texture = m_textures[i].getTexture();
			if (texture)
			{
				Renderer::instance()->setTexture(i, m_textures[i]);
			}
		}
	}

	// 绑定渲染状态
	void Renderable::bindRenderState()
	{
		if (m_shaderProgram)
		{
			Renderer* pRenderer = Renderer::instance();
			pRenderer->setDepthStencilState(m_pDepthStencil ? m_pDepthStencil : m_shaderProgram->getDepthState());
			pRenderer->setRasterizerState(m_pRasterizerState ? m_pRasterizerState : m_shaderProgram->getRasterizerState());
			pRenderer->setBlendState(m_pBlendState ? m_pBlendState : m_shaderProgram->getBlendState());
		}
	}

	// 执行渲染
	void Renderable::render()
	{
		if (m_shaderProgram)
		{
			// 绑定着色器
			m_shaderProgram->bind();

			// 绑定渲染状态
			bindRenderState();

			// 绑定参数
			bindShaderParams();

			// 执行渲染
			EchoAssert(m_renderInput);
			Renderer::instance()->render(m_renderInput, m_shaderProgram->getShaderProgram());
		}
	}

	// 提交到渲染队列
	void Renderable::submitToRenderQueue()
	{
		RenderStage::instance()->addRenderable(m_renderStage, getIdentifier());
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

	// 设置渲染几何数据
	void Renderable::setRenderInput(GPUBuffer* vertexStream, const RenderInput::VertexElementList& vertElements, GPUBuffer* indexStream, ui32 idxStride, RenderInput::TopologyType topologyType)
	{
		EchoSafeDelete(m_renderInput, RenderInput);

		if (m_shaderProgram)
		{
			ShaderProgram* program = m_shaderProgram->getShaderProgram();
			if (program)
			{
				m_renderInput = Renderer::instance()->createRenderInput(program);
				m_renderInput->setTopologyType(topologyType);
				m_renderInput->bindVertexStream(vertElements, vertexStream);
				m_renderInput->bindIndexStream(indexStream, idxStride);
			}
		}
	}
}