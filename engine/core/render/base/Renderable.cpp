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
	Renderable::Renderable(const MaterialPtr& material, int identifier)
		: m_identifier(identifier)
	{
		m_material = material;
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
		Renderable* renderable = Renderer::instance()->createRenderable(matInst);
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

		return renderable;
	}

	void Renderable::setTexture(ui32 stage, Texture* texture)
	{
        if(stage<m_textures.size())
            m_textures[stage] = texture;
        else
            EchoLogError("Renderable set texture failed");
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
		ShaderProgram* shaderProgram = m_material->getShader();
		if (shaderProgram)
		{
			Renderer* pRenderer = Renderer::instance();
			pRenderer->setDepthStencilState(shaderProgram->getDepthState());
			pRenderer->setRasterizerState(shaderProgram->getRasterizerState());
			pRenderer->setBlendState(shaderProgram->getBlendState());
		}
	}

	void Renderable::submitToRenderQueue()
	{
		if (m_mesh && m_mesh->isValid())
		{
			RenderStage::instance()->addRenderable(m_material->getRenderStage(), getIdentifier());
		}
	}

	Material* Renderable::getMaterial()
	{
		return m_material;
	}

	void Renderable::setShaderParam(const String& name, ShaderParamType type, const void* param, size_t num/* =1 */)
	{
		ShaderParam shaderParam;
		shaderParam.name = name;
		shaderParam.type = type;
		shaderParam.data = param;
		shaderParam.length = ui32(num);

		m_shaderParams[name] = shaderParam;
	}
}
