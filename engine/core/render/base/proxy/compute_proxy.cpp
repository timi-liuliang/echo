#include "engine/core/log/Log.h"
#include "compute_proxy.h"
#include "base/shader/shader_program.h"
#include "base/renderer.h"
#include "base/pipeline/render_pipeline.h"
#include "base/shader/material.h"
#include "base/mesh/mesh.h"
#include "engine/core/scene/render_node.h"

namespace Echo
{
	ComputeProxy::ComputeProxy(int identifier)
		: m_identifier(identifier)
	{
	}

	ComputeProxy::~ComputeProxy()
	{
	}

	void ComputeProxy::release()
	{
		ComputeProxy* ptr = this;
		//Renderer::instance()->destroyRenderables(&ptr, 1);
	}

	ComputeProxy* ComputeProxy::create(MeshPtr mesh, Material* material, Render* node)
	{
		ComputeProxy* proxy = Renderer::instance()->createComputeProxy();
		//renderable->setMaterial(material);
		//renderable->setNode(node);
		//renderable->setMesh(mesh);

		return proxy;
	}

	void ComputeProxy::submitToRenderQueue()
	{
		if (m_mesh && m_mesh->isValid())
		{
			//RenderPipeline::current()->addRenderable(m_material->getRenderStage(), getIdentifier());
		}
	}
}
