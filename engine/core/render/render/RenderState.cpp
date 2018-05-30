#include "Render/RenderState.h"
#include "engine/core/Math/Math.h"

namespace Echo
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	RenderState::RenderState()
	{
	}

	RenderState::~RenderState()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	BlendState::BlendState()
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	DepthStencilState::DepthStencilState()
	{
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	RasterizerState::RasterizerState()
	{
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	SamplerState::SamplerState()
	{
		m_desc.minFilter		= FO_LINEAR;
		m_desc.magFilter		= FO_LINEAR;
		m_desc.mipFilter		= FO_LINEAR;
		m_desc.addrUMode		= AM_CLAMP;
		m_desc.addrVMode		= AM_CLAMP;
		m_desc.addrWMode		= AM_CLAMP;
		m_desc.maxAnisotropy	= 16;
		m_desc.cmpFunc			= CF_NEVER;
		m_desc.borderColor		= Color::BLACK;
		m_desc.minLOD			= -Math::MAX_FLOAT;
		m_desc.maxLOD			= Math::MAX_FLOAT;
		m_desc.mipLODBias		= 0.0f;
		//m_pTexture				= NULL;
	}

	SamplerState::SamplerState(const SamplerDesc& desc)
		: m_desc(desc)
		//, m_pTexture(NULL)
	{
	}

	SamplerState::~SamplerState()
	{
	}

	const SamplerState::SamplerDesc& SamplerState::getDesc() const
	{
		return m_desc;
	}

// 	void SamplerState::setTexture(Texture* pTexture)
// 	{
// 		m_pTexture = pTexture;
// 	}
}
