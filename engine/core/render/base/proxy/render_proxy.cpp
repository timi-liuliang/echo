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
	RenderProxy::RenderProxy()
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
		if (mesh->isValid())
		{
			RenderProxy* renderProxy = Renderer::instance()->createRenderProxy();
			renderProxy->setRaytracing(raytracing);
			renderProxy->setMaterial(material);
			renderProxy->setNode(node);
			renderProxy->setMesh(mesh);

			return renderProxy;
		}

		return nullptr;
	}

	void RenderProxy::setSubmitToRenderQueue(bool enable)
	{ 
		m_isSubmitToRenderQueue = enable;

		Renderer::instance()->updateRenderProxyBvh(this);
	}

	void RenderProxy::submitToRenderQueue(RenderPipeline* pipeline)
	{
		if (m_mesh && m_mesh->isValid())
		{
			if (m_node->isRenderTypeUi())
			{
				pipeline->addRenderable("Ui", getId());
			}
			else
			{
				pipeline->addRenderable(m_material->getRenderStage(), getId());
			}
		}
	}
}
