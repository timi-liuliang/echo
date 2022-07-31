#pragma once

#include "engine/core/render/base/pipeline/render_stage.h"

namespace Echo
{
	class CustomDepth : public IRenderQueue
	{
		ECHO_CLASS(CustomDepth, IRenderQueue)

	public:
		CustomDepth();
		virtual ~CustomDepth();

		// Process
		virtual void render(FrameBufferPtr& frameBuffer) override;

	protected:
		// Init default shadow depth shader
		ShaderProgramPtr initCustomDepthShader();

	protected:
		ShaderProgramPtr						m_customDepthShader;
		RasterizerStatePtr						m_customDepthRasterizerState;
		MaterialPtr								m_customDepthMaterial;
		std::unordered_map<i32, RenderProxy*>	m_customDepthRenderProxiers;
	};
}