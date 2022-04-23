#include "shadow_depth_render_stage.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	ShadowDepthRenderStage::ShadowDepthRenderStage()
		: RenderStage()
	{

	}

	ShadowDepthRenderStage::~ShadowDepthRenderStage()
	{

	}

	void ShadowDepthRenderStage::bindMethods()
	{

	}

	void ShadowDepthRenderStage::render()
	{
		if (!m_enable)				return;
		if (!m_frameBuffer)			return;
		if (IsGame && m_editorOnly) return;

		if (m_frameBuffer->begin())
		{
			onRenderBegin();
			{
				vector<Light*>::type dirLights = Light::gatherLights(Light::Type::Direction);
				for (Light* light : dirLights)
				{
					DirectionLight* dirLight = ECHO_DOWN_CAST<DirectionLight*>(light);
					Frustum* frustum = dirLight->getFrustum();

					if (frustum)
					{
						vector<RenderProxy*>::type visibleRenderProxies3D = Renderer::instance()->gatherRenderProxies(RenderProxy::RenderType3D, *frustum);
						for (RenderProxy* renderproxy : visibleRenderProxies3D)
						{
							std::unordered_map<i32, RenderProxy*>::const_iterator it = m_shadowDepthRenderProxiers.find(renderproxy->getIdentifier());
							if (it != m_shadowDepthRenderProxiers.end())
							{
								RenderProxy* shadowDepthRenderProxy = it->second;
								Renderer::instance()->draw(shadowDepthRenderProxy, m_frameBuffer);
							}
							else
							{

							}
						}
					}
				}
			}
			onRenderEnd();

			m_frameBuffer->end();
		}
	}
}