#include "GLES2RenderStd.h"
#include "GLES2RenderState.h"
#include "GLES2RenderStateTasks.h"


namespace Echo
{

	GLES2StateTaskActiveBlend::GLES2StateTaskActiveBlend(const BlendStateParams& params)
		: m_blend_params(params)
	{
	}

	void GLES2StateTaskActiveBlend::Execute()
	{
		ActiveBlendState(m_blend_params);
	}


	GLES2StateTaskActiveDepthStencil::GLES2StateTaskActiveDepthStencil(const DepthStencilStateParams& params)
		: m_depthStencilParams(params)
	{
	}

	void GLES2StateTaskActiveDepthStencil::Execute()
	{
		ActiveDepthStencilState(m_depthStencilParams);
	}


	GLES2StateTaskActiveRasterizer::GLES2StateTaskActiveRasterizer(const RasterizerState::RasterizerDesc& myDesc, const RasterizerState::RasterizerDesc* currDesc, GLenum frontFace)
		: m_myDesc(myDesc)
		, m_currDesc(nullptr)
		, m_frontFace(frontFace)
	{
		if (currDesc)
		{
			m_currDesc = EchoNew(RasterizerState::RasterizerDesc);
			*m_currDesc = *currDesc;
		}
	}

	void GLES2StateTaskActiveRasterizer::Execute()
	{
		if (m_currDesc)
		{
			ActiveRasterizerState(m_myDesc, *m_currDesc, m_frontFace, true);
			EchoSafeDelete(m_currDesc, RasterizerDesc);
		}
		else
		{
			ActiveRasterizerState(m_myDesc, m_myDesc, m_frontFace, false);
		}
	}

	
	GLES2RenderStateTaskActiveSampler::GLES2RenderStateTaskActiveSampler(GLES2SamplerStateGPUProxy* proxy, const SamplerState::SamplerDesc* curr_desc, const SamplerState::SamplerDesc* prev_desc)
		: m_sampler_proxy(proxy)
		, m_curr_desc(nullptr)
		, m_prev_desc(nullptr)
	{
		if (curr_desc)
		{
			m_curr_desc = EchoNew(SamplerState::SamplerDesc);
			*m_curr_desc = *curr_desc;
		}

		if (prev_desc)
		{
			m_prev_desc = EchoNew(SamplerState::SamplerDesc);
			*m_prev_desc = *prev_desc;
		}
	}

	void GLES2RenderStateTaskActiveSampler::Execute()
	{
		m_sampler_proxy->active(m_curr_desc, m_prev_desc);
	}
}