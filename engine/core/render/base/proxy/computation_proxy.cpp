#include "engine/core/log/Log.h"
#include "computation_proxy.h"
#include "base/shader_program.h"
#include "base/renderer.h"
#include "base/pipeline/render_pipeline.h"
#include "base/material.h"
#include "base/mesh/mesh.h"
#include "engine/core/scene/render_node.h"

namespace Echo
{
	ComputationProxy::ComputationProxy(int identifier)
		: m_identifier(identifier)
	{
	}

	ComputationProxy::~ComputationProxy()
	{
	}

	void ComputationProxy::release()
	{
		ComputationProxy* ptr = this;
		//Renderer::instance()->destroyRenderables(&ptr, 1);
	}

	ComputationProxy* ComputationProxy::create(MeshPtr mesh, Material* material, Render* node)
	{
		return nullptr;

		// bind shader param
		//ComputationProxy* renderable = Renderer::instance()->createRenderable();
		//renderable->setMaterial(material);
		//renderable->setNode(node);
		//renderable->setMesh(mesh);

		//return renderable;
	}

	void ComputationProxy::submitToRenderQueue()
	{
		if (m_mesh && m_mesh->isValid())
		{
			//RenderPipeline::current()->addRenderable(m_material->getRenderStage(), getIdentifier());
		}
	}
}
