#include "engine/core/log/Log.h"
#include "base/Renderable.h"
#include "base/ShaderProgram.h"
#include "base/Renderer.h"
#include "base/ShaderProgram.h"
#include "base/pipeline/RenderPipeline.h"
#include "base/Material.h"
#include "base/mesh/MeshRes.h"
#include "engine/core/scene/render_node.h"

namespace Echo
{
	Renderable::Renderable(int identifier)
		: m_identifier(identifier)
	{
	}

	Renderable::~Renderable()
	{
	}

	void Renderable::release()
	{
		Renderable* ptr = this;
		Renderer::instance()->destroyRenderables(&ptr, 1);
	}

	Renderable* Renderable::create(MeshResPtr mesh, Material* material, Render* node)
	{
		// bind shader param
		Renderable* renderable = Renderer::instance()->createRenderable();
		renderable->setMaterial(material);
		renderable->setNode(node);
		renderable->setMesh(mesh);

		return renderable;
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
			RenderPipeline::current()->addRenderable(m_material->getRenderStage(), getIdentifier());
		}
	}
}
