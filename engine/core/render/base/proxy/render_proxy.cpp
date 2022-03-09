#include "engine/core/log/Log.h"
#include "base/proxy/render_proxy.h"
#include "base/renderer.h"
#include "base/pipeline/render_pipeline.h"
#include "base/shader/material.h"
#include "base/mesh/mesh.h"
#include "engine/core/scene/render_node.h"
#include "engine/core/main/engine.h"

namespace Echo
{
	RenderProxy::RenderProxy(int identifier)
		: m_identifier(identifier)
	{
	}

	RenderProxy::~RenderProxy()
	{
	}

	void RenderProxy::subRefCount()
	{
		m_refCount--;
		if (m_refCount <= 0)
		{
			RenderProxy* ptr = this;
			Renderer::instance()->destroyRenderProxies(&ptr, 1);
		}
	}

	RenderProxy* RenderProxy::create(MeshPtr mesh, Material* material, Render* node, bool raytracing)
	{
		// bind shader param
		RenderProxy* renderProxy = Renderer::instance()->createRenderProxy();
		renderProxy->setRaytracing(raytracing);
		renderProxy->setMaterial(material);
		renderProxy->setNode(node);
		renderProxy->setMesh(mesh);

		return renderProxy;
	}

	void RenderProxy::submitToRenderQueue(RenderPipeline* pipeline)
	{
		if (m_mesh && m_mesh->isValid())
		{
			pipeline->addRenderable(m_material->getRenderStage(), getIdentifier());
		}
	}
}
