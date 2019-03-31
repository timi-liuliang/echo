#include "engine/core/log/Log.h"
#include "interface/Renderable.h"
#include "interface/ShaderProgram.h"
#include "interface/Renderer.h"
#include "interface/ShaderProgram.h"
#include "engine/core/render/interface/pipeline/RenderStage.h"
#include "engine/core/render/interface/Material.h"
#include "engine/core/render/interface/mesh/Mesh.h"
#include "engine/core/scene/render_node.h"

namespace Echo
{
	Renderable::Renderable(const String& renderStage, ShaderProgram* shader, int identifier)
		: m_identifier(identifier)
        , m_renderStage(renderStage)
	{
		m_shaderProgram = shader;
		m_textures.assign(nullptr);
	}

	Renderable::~Renderable()
	{
		m_shaderParams.clear();
	}

	// release
	void Renderable::release()
	{
		Renderable* ptr = this;
		Renderer::instance()->destroyRenderables(&ptr, 1);
	}

	Renderable* Renderable::create(Mesh* mesh, Material* matInst, Render* node)
	{
		ShaderProgram* shaderProgram = matInst->getShader();
		if (!shaderProgram)
			return nullptr;

		ShaderProgram::UniformArray* uniforms = shaderProgram->getUniforms();

		// bind shader param
		Renderable* renderable = Renderer::instance()->createRenderable(matInst->getRenderStage(), matInst->getShader());
		renderable->setNode(node);
		renderable->setMesh(mesh);
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
				Texture* texture = matInst->getTexture(*slotIdxPtr);
				renderable->setTexture(*slotIdxPtr, texture);

				renderable->setShaderParam(shaderProgram->getParamPhysicsIndex(uniform.m_name), uniform.m_type, slotIdxPtr, uniform.m_count);
			}
		}
		renderable->endShaderParams();

		//renderable->setRasterizerState(matInst->getRasterizerState());
		//renderable->setBlendState(matInst->getBlendState());
		//renderable->setDepthStencilState(matInst->getDepthStencilState());

		return renderable;
	}

	void Renderable::beginShaderParams(size_t paramNum)
	{
		m_shaderParams.resize(paramNum);
		m_SParamWriteIndex = 0;
	}

	void Renderable::endShaderParams()
	{
		if (m_SParamWriteIndex != m_shaderParams.size())
		{
			EchoLogError("[Renderable:%d]:: index == %d, shader size == %d Node [%s]", __LINE__, m_SParamWriteIndex, m_shaderParams.size(), m_node->getName().c_str());
		}
	}

	void Renderable::setShaderParam(size_t physicsIndex, ShaderParamType type, const void* param, size_t num/* =1 */)
	{
		if (m_SParamWriteIndex < m_shaderParams.size() && param)
		{
			m_shaderParams[m_SParamWriteIndex].physicsIndex = static_cast<ui32>(physicsIndex);
			m_shaderParams[m_SParamWriteIndex].stype = type;
			m_shaderParams[m_SParamWriteIndex].pData = param;
			m_shaderParams[m_SParamWriteIndex].ParamsLength = static_cast<ui32>(num);
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
		for (ui32 modifyIndex = 0; modifyIndex < m_shaderParams.size(); modifyIndex++)
		{
			if (m_shaderParams[modifyIndex].physicsIndex == physics)
            {
                m_shaderParams[modifyIndex].stype = type;
                m_shaderParams[modifyIndex].pData = param;
                m_shaderParams[modifyIndex].ParamsLength = static_cast<ui32>(num);
                
                break;
            }
		}
	}

	void Renderable::setTexture(ui32 stage, Texture* texture)
	{
		m_textures[stage] = texture;
	}

	void Renderable::bindShaderParams()
	{
		bindTextures();
			
		if (m_shaderProgram)
		{
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
				case SPT_TEXTURE:	m_shaderProgram->setUniform(param.physicsIndex, param.pData, param.stype, param.ParamsLength);	break;
				default:			EchoLogError("unknow shader param format! %s", m_node->getName().c_str());							break;
				}
			}
		}
	}

	void Renderable::bindTextures()
	{
		for (int i = 0; i < MAX_TEXTURE_SAMPLER; ++i)
		{
			Texture* texture = m_textures[i];
			if (texture)
				Renderer::instance()->setTexture(i, texture);
		}
	}

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

	void Renderable::render()
	{
		if (m_shaderProgram)
		{
			m_shaderProgram->bind();
			bindRenderState();
			bindShaderParams();

			Renderer::instance()->draw( this);
		}
	}

	void Renderable::submitToRenderQueue()
	{
		if (m_mesh->isValid())
		{
			RenderStage::instance()->addRenderable(m_renderStage, getIdentifier());
		}
	}

	void Renderable::setBlendState(BlendState* pState)
	{
		m_pBlendState = pState;
		m_bRenderState = pState ? true : false;
	}

	void Renderable::setRasterizerState(RasterizerState* state)
	{
		m_pRasterizerState = state;
		m_bRenderState = state ? true : false;
	}

	void Renderable::setDepthStencilState(DepthStencilState* state)
	{
		m_pDepthStencil = state;
		m_bRenderState = state ? true : false;
	}

	// get shader
	ShaderProgram* Renderable::getShader()
	{
		return m_shaderProgram;
	}

	void Renderable::setMesh(Mesh* mesh)
	{
		m_mesh = mesh;
		link();
	}
}
