#include "render_state.h"
#include "engine/core/math/Math.h"
#include "renderer.h"

namespace Echo
{
	RenderState::RenderState()
	{
	}

	RenderState::~RenderState()
	{
	}

	void RenderState::bindMethods()
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

	DepthStencilState::~DepthStencilState()
	{
	}

	void DepthStencilState::bindMethods()
	{
		CLASS_BIND_METHOD(DepthStencilState, isDepthEnable, DEF_METHOD("isDepthEnable"));
		CLASS_BIND_METHOD(DepthStencilState, setDepthEnable, DEF_METHOD("setDepthEnable"));
		CLASS_BIND_METHOD(DepthStencilState, isWriteDepth, DEF_METHOD("isWriteDepth"));
		CLASS_BIND_METHOD(DepthStencilState, setWriteDepth, DEF_METHOD("setWriteDepth"));

		CLASS_REGISTER_PROPERTY(DepthStencilState, "DepthEnable", Variant::Type::Bool, "isDepthEnable", "setDepthEnable");
		CLASS_REGISTER_PROPERTY(DepthStencilState, "WriteDepth", Variant::Type::Bool, "isWriteDepth", "setWriteDepth");
	}

	Res* DepthStencilState::create()
	{
		return Renderer::instance()->createDepthStencilState();
	}

	void DepthStencilState::setDepthEnable(bool enable)
	{ 
		if (bDepthEnable != enable)
		{
			bDepthEnable = enable;
			setDirty(true);
		}
	}

	void DepthStencilState::setWriteDepth(bool writeDepth)
	{ 
		if (bWriteDepth != writeDepth)
		{
			bWriteDepth = writeDepth; 
			setDirty(true);
		}
	}

	RasterizerState::RasterizerState()
	{
	}

	RasterizerState::~RasterizerState()
	{
	}

	Res* RasterizerState::create()
	{
		return Renderer::instance()->createRasterizerState();
	}

	void RasterizerState::setCullMode(CullMode inCullMode)
	{
		if (cullMode != inCullMode)
		{
			cullMode = inCullMode;
			setDirty(true);
		}
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
