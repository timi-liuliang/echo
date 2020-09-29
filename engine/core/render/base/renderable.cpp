#include "engine/core/log/Log.h"
#include "base/renderable.h"
#include "base/shader_program.h"
#include "base/renderer.h"
#include "base/pipeline/render_pipeline.h"
#include "base/material.h"
#include "base/mesh/mesh.h"
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

	Renderable* Renderable::create(MeshPtr mesh, Material* material, Render* node)
	{
		// bind shader param
		Renderable* renderable = Renderer::instance()->createRenderable();
		renderable->setMaterial(material);
		renderable->setNode(node);
		renderable->setMesh(mesh);

		return renderable;
	}

	void Renderable::submitToRenderQueue()
	{
		if (m_mesh && m_mesh->isValid())
		{
			RenderPipeline::current()->addRenderable(m_material->getRenderStage(), getIdentifier());
		}
	}
}
