#include "render_state.h"
#include "engine/core/math/Math.h"

namespace Echo
{
	RenderState::RenderState()
	{
	}

	RenderState::~RenderState()
	{
	}

	BlendState::BlendState(const BlendDesc& desc)
		: m_desc(desc)
	{
	}

	BlendState::~BlendState()
	{
	}

	const BlendState::BlendDesc& BlendState::getDesc() const
	{
		return m_desc;
	}

	DepthStencilState::DepthStencilState(const DepthStencilDesc& desc)
		: m_desc(desc)
	{
	}

	DepthStencilState::~DepthStencilState()
	{
	}

	const DepthStencilState::DepthStencilDesc& DepthStencilState::getDesc() const
	{
		return m_desc;
	}

	RasterizerState::RasterizerState(const RasterizerDesc& desc)
		: m_desc(desc)
	{
	}

	RasterizerState::~RasterizerState()
	{
	}

	const RasterizerState::RasterizerDesc& RasterizerState::getDesc() const
	{
		return m_desc;
	}

	SamplerState::SamplerState(const SamplerDesc& desc)
		: m_desc(desc)
	{
	}

	SamplerState::~SamplerState()
	{
	}

	const SamplerState::SamplerDesc& SamplerState::getDesc() const
	{
		return m_desc;
	}
}
