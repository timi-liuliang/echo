#pragma once

#include "engine/core/render/base/pipeline/render_stage.h"

namespace Echo
{
	class DeferredLightingRenderQueue : public IRenderQueue
	{
		ECHO_CLASS(DeferredLightingRenderQueue, IRenderQueue)

	public:
		DeferredLightingRenderQueue();
		virtual ~DeferredLightingRenderQueue();

		// Process
		virtual void render(FrameBufferPtr& frameBuffer) override;

	protected:
		// Init default shadow depth shader
		ShaderProgramPtr initShadowDepthShader();

	protected:
		ShaderProgramPtr						m_shadowDepthShader;
		RasterizerStatePtr						m_shadowDepthRasterizerState;
		MaterialPtr								m_shadowDepthMaterial;
		std::unordered_map<i32, RenderProxy*>	m_shadowDepthRenderProxiers;
	};
}