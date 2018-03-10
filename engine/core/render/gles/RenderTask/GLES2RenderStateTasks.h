#pragma once
#include "GLES2StateGPUProxy.h"

namespace Echo
{
	class GLES2SamplerStateGPUProxy;

	class GLES2StateTaskActiveBlend
	{
	public:
		GLES2StateTaskActiveBlend(const BlendStateParams& params);
		void Execute();
	private:
		BlendStateParams m_blend_params;
	};

	class GLES2StateTaskActiveDepthStencil
	{
	public:
		GLES2StateTaskActiveDepthStencil(const DepthStencilStateParams& params);
		void Execute();
	private:
		DepthStencilStateParams m_depthStencilParams;
	};

	class GLES2StateTaskActiveRasterizer
	{
	public:
		GLES2StateTaskActiveRasterizer(const RasterizerState::RasterizerDesc& myDesc, const RasterizerState::RasterizerDesc* currDesc, GLenum frontFace);
		void Execute();
	private:
		RasterizerState::RasterizerDesc m_myDesc;
		RasterizerState::RasterizerDesc* m_currDesc;
		GLenum m_frontFace;
	};

	class GLES2RenderStateTaskActiveSampler
	{
	public:
		GLES2RenderStateTaskActiveSampler(GLES2SamplerStateGPUProxy* proxy, const SamplerState::SamplerDesc* curr_desc, const SamplerState::SamplerDesc* prev_desc);
		void Execute();
	private:
		GLES2SamplerStateGPUProxy* m_sampler_proxy;
		SamplerState::SamplerDesc* m_curr_desc;
		SamplerState::SamplerDesc* m_prev_desc;
	};

}