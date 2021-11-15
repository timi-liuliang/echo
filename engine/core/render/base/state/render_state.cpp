#include "render_state.h"
#include "engine/core/math/Math.h"
#include "base/renderer.h"

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

	BlendState::BlendState()
	{
	}

	BlendState::~BlendState()
	{
	}

	void BlendState::setBlendEnable(bool enable)
	{
		if (m_blendEnable != enable)
		{
			m_blendEnable = enable;
			setDirty(true);
		}
	}

	void BlendState::setSrcBlend(BlendFactor blendFactor)
	{
		if (m_srcBlend != blendFactor)
		{
			m_srcBlend = blendFactor;
			setDirty(true);
		}
	}

	void BlendState::setDstBlend(BlendFactor blendFactor)
	{
		if (m_dstBlend != blendFactor)
		{
			m_dstBlend = blendFactor;
			setDirty(true);
		}
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
		if (m_depthEnable != enable)
		{
			m_depthEnable = enable;
			setDirty(true);
		}
	}

	void DepthStencilState::setWriteDepth(bool writeDepth)
	{ 
		if (m_writeDepth != writeDepth)
		{
			m_writeDepth = writeDepth; 
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
		if (m_cullMode != inCullMode)
		{
			m_cullMode = inCullMode;
			setDirty(true);
		}
	}

	void RasterizerState::setLineWidth(float lineWidth)
	{ 
		if (m_lineWidth != lineWidth)
		{
			m_lineWidth = lineWidth;
			setDirty(true);
		}
	}

	SamplerState::SamplerState()
	{
	}

	SamplerState::~SamplerState()
	{
	}

	Res* SamplerState::create()
	{
		return Renderer::instance()->createSamplerState();
	}
}
