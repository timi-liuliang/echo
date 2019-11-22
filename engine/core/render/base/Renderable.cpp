#include "engine/core/log/Log.h"
#include "base/Renderable.h"
#include "base/ShaderProgram.h"
#include "base/Renderer.h"
#include "base/ShaderProgram.h"
#include "base/pipeline/RenderStage.h"
#include "base/Material.h"
#include "base/mesh/Mesh.h"
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
	}

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
		for (auto& it : *uniforms)
		{
			const ShaderProgram::Uniform& uniform = it.second;
			if (uniform.m_type != SPT_TEXTURE)
			{
				void* value = node ? node->getGlobalUniformValue(uniform.m_name) : nullptr;
				if (!value) value = matInst->getUniformValue(uniform.m_name);

				renderable->setShaderParam(uniform.m_name, uniform.m_type, value, uniform.m_count);
			}
			else
			{
				i32* slotIdxPtr  = (i32*)matInst->getUniformValue(uniform.m_name);
				Texture* texture = matInst->getTexture(*slotIdxPtr);
				renderable->setTexture(*slotIdxPtr, texture);

				renderable->setShaderParam(uniform.m_name, uniform.m_type, slotIdxPtr, uniform.m_count);
			}
		}

		//renderable->setRasterizerState(matInst->getRasterizerState());
		//renderable->setBlendState(matInst->getBlendState());
		//renderable->setDepthStencilState(matInst->getDepthStencilState());

		return renderable;
	}

	void Renderable::setTexture(ui32 stage, Texture* texture)
	{
		m_textures[stage] = texture;
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
			pRenderer->setDepthStencilState(m_depthStencilState ? m_depthStencilState : m_shaderProgram->getDepthState());
			pRenderer->setRasterizerState(m_rasterizerState ? m_rasterizerState : m_shaderProgram->getRasterizerState());
			pRenderer->setBlendState(m_blendState ? m_blendState : m_shaderProgram->getBlendState());
		}
	}

	void Renderable::submitToRenderQueue()
	{
		if (m_mesh && m_mesh->isValid())
		{
			RenderStage::instance()->addRenderable(m_renderStage, getIdentifier());
		}
	}

	void Renderable::setBlendState(BlendState* pState)
	{
		m_blendState = pState;
	}

	void Renderable::setRasterizerState(RasterizerState* state)
	{
		m_rasterizerState = state;
	}

	void Renderable::setDepthStencilState(DepthStencilState* state)
	{
		m_depthStencilState = state;
	}

	ShaderProgram* Renderable::getShader()
	{
		return m_shaderProgram;
	}

	void Renderable::setShaderParam(const String& name, ShaderParamType type, const void* param, size_t num/* =1 */)
	{
		ShaderParam sp;
		sp.name = name;
		sp.type = type;
		sp.data = param;
		sp.length = ui32(num);

		m_shaderParams[name] = sp;
	}
}
