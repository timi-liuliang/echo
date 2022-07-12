#pragma once

#include "engine/core/math/color.h"
#include "engine/core/resource/Res.h"

namespace Echo
{
	class RenderState : public Res
	{
		ECHO_VIRTUAL_CLASS(RenderState, Res)

	public:
		enum ColorMask
		{
			CMASK_NONE = 0x00000000,
			CMASK_RED = 0x00000001,
			CMASK_GREEN = 0x00000002,
			CMASK_BLUE = 0x00000004,
			CMASK_ALPHA = 0x00000008,
			CMASK_COLOR = CMASK_RED | CMASK_GREEN | CMASK_BLUE,
			CMASK_ALL = CMASK_RED | CMASK_GREEN | CMASK_BLUE | CMASK_ALPHA
		};

		enum ComparisonFunc
		{
			CF_NEVER,
			CF_LESS,
			CF_LESS_EQUAL,
			CF_EQUAL,
			CF_GREATER_EQUAL,
			CF_GREATER,
			CF_ALWAYS,
			CF_NOT_EQUAL,
			CF_MAXNUM
		};

	public:
		RenderState();
		virtual ~RenderState();

		// dirty
		bool isDirty() { return m_dirty; }
		void setDirty(bool dirty) { m_dirty = dirty; }

		// active
		virtual void active() {}

	protected:
		bool	m_dirty = true;
	};

	class BlendState: public RenderState
	{
	public:
		enum BlendOperation
		{
			BOP_ADD, 
			BOP_SUB, 
			BOP_REV_SUB, 
			BOP_MIN, 
			BOP_MAX,
		};

		enum BlendFactor
		{
			BF_ZERO,
			BF_ONE,
			BF_SRC_ALPHA,
			BF_DST_ALPHA,
			BF_INV_SRC_ALPHA,
			BF_INV_DST_ALPHA,
			BF_SRC_COLOR,
			BF_DST_COLOR,
			BF_INV_SRC_COLOR,
			BF_INV_DST_COLOR,
			BF_SRC_ALPHA_SAT, 
			BF_MAX,
		};

		BlendState();
		virtual ~BlendState();

		// blend enable
		bool isBlendEnable() const { return m_blendEnable; }
		void setBlendEnable(bool enable);

		// src blend
		BlendFactor getSrcBlend() const { return m_srcBlend; }
		void setSrcBlend(BlendFactor blendFactor);

		// dst blend
		BlendFactor getDstBlend() const { return m_dstBlend; }
		void setDstBlend(BlendFactor blendFactor);

	public:
		bool				m_blendEnable = false;
		BlendFactor			m_srcBlend = BF_ONE;
		BlendFactor			m_dstBlend = BF_ZERO;
		BlendOperation		m_blendOP = BOP_ADD;
		BlendFactor			m_srcAlphaBlend = BF_ONE;
		BlendFactor			m_dstAlphaBlend = BF_ZERO;
		BlendOperation		m_alphaBlendOP = BOP_ADD;
		ui8					m_colorWriteMask = CMASK_COLOR;
		bool				m_a2cEnable = false;
		Color				m_blendFactor = { 0.f, 0.f, 0.f, 0.f};
	};

	class DepthStencilState: public RenderState
	{
		ECHO_RES(DepthStencilState, RenderState, ".DepthStencilState", DepthStencilState::create, Res::load)

	public:
		enum StencilOperation
		{
			// Keep the existing stencil data.
			SOP_KEEP,
			// Set the stencil data to 0
			SOP_ZERO,
			// Set the stencil value to the reference value
			SOP_REPLACE,
			// Increase the stencil value by 1, clamping at the maximum value
			SOP_INCR,
			// Decrease the stencil value by 1, clamping at 0
			SOP_DECR,
			// Invert the stencil data
			SOP_INVERT,
			// Increase the stencil value by 1, wrap the result if necessary
			SOP_INCR_WRAP,
			// Decrease the stencil value by 1, wrap the result if necessary
			SOP_DECR_WRAP,
			SOP_MAX
		};

	public:
		DepthStencilState() {}
		virtual ~DepthStencilState();

		// create fun
		static Res* create();

	public:
		// depth enable
		bool isDepthEnable() const { return m_depthEnable; }
		void setDepthEnable(bool enable);

		// write depth
		bool isWriteDepth() const { return m_writeDepth; }
		void setWriteDepth(bool writeDepth);

	public:
		bool				m_depthEnable = true;
		bool				m_writeDepth = true;
		ComparisonFunc		m_depthFunc = CF_LESS;

		bool				bFrontStencilEnable = false;
		ComparisonFunc		frontStencilFunc = CF_ALWAYS;
		ui16				frontStencilReadMask = 0xffff;
		ui16				frontStencilWriteMask = 0xffff;
		StencilOperation	frontStencilFailOP = SOP_KEEP;
		StencilOperation	frontStencilDepthFailOP = SOP_INCR;
		StencilOperation	frontStencilPassOP = SOP_KEEP;
		ui32				frontStencilRef = 1;

		bool				bBackStencilEnable = false;
		ComparisonFunc		backStencilFunc = CF_ALWAYS;
		ui16				backStencilReadMask = 0xffff;
		ui16				backStencilWriteMask = 0xffff;
		StencilOperation	backStencilFailOP = SOP_KEEP;
		StencilOperation	backStencilDepthFailOP = SOP_DECR;
		StencilOperation	backStencilPassOP = SOP_KEEP;
		ui32				backStencilRef = 1;
	};
	typedef ResRef<DepthStencilState> DepthStencilStatePtr;

	class RasterizerState : public RenderState
	{
		ECHO_RES(RasterizerState, RenderState, ".RasterizerState", RasterizerState::create, Res::load)

	public:
		enum PolygonMode
		{
			PM_POINT, 
			PM_LINE, 
			PM_FILL, 
		};

		enum ShadeModel
		{
			SM_FLAT, 
			SM_GOURAND, 
		};

		enum CullMode
		{
			CULL_NONE, 
			CULL_FRONT, 
			CULL_BACK, 
		};

	public:
		RasterizerState();
		virtual ~RasterizerState();

		// create fun
		static Res* create();

		// cull mode
		CullMode getCullMode() { return m_cullMode; }
		void setCullMode(CullMode cullMode);

		// Frontface
		bool isFrontFaceCCW() const { return m_frontFaceCCW; }

		// depth bias
		float getDepthBias() const { return m_depthBias; }

		// depth bias factor
		float getDepthBiasFactor() const { return m_depthBiasFactor; }

		// scissor
		bool isScissor() const { return m_scissor; }

		// line width
		float getLineWidth() const { return m_lineWidth; }
		void setLineWidth(float lineWidth);

	protected:
		PolygonMode			m_polygonMode = PM_FILL;
		ShadeModel			m_shadeModel = SM_GOURAND;
		CullMode			m_cullMode = CULL_BACK;
		bool				m_frontFaceCCW = false;

		// depth bias
		// z = depthBiasFactor * DZ + depthBias
		// DZ is max depth slope
		float				m_depthBias = 0.f;
		float				m_depthBiasFactor = 0.f;

		bool				m_depthClip = true;
		bool				m_scissor = false;

		float               m_lineWidth = 1.f;
	};
	typedef ResRef<RasterizerState> RasterizerStatePtr;

	class SamplerState : public RenderState
	{
		ECHO_RES(SamplerState, RenderState, ".SampleState", SamplerState::create, Res::load)

	public:
		// texture filter option
		enum FilterOption
		{
			FO_NONE,		// No filtering, used for FILT_MIP to turn off mipmapping
			FO_POINT,		// Use the closest pixel
			FO_LINEAR,		// Average of a 2x2 pixel area, denotes bilinear for MIN and MAG, trilinear for MIP
			FO_ANISOTROPIC, // Similar to FO_LINEAR, but compensates for the angle of the texture plane
			FO_MAX
		};

		// texture address mode
		enum AddressMode
		{
			AM_WRAP,
			AM_MIRROR,
			AM_CLAMP,
			AM_BORDER,
			AM_MAX
		};

	public:
		SamplerState();
		virtual ~SamplerState();

		// create fun
		static Res* create();

	public:
		FilterOption	minFilter = FO_LINEAR;
		FilterOption	magFilter = FO_LINEAR;
		FilterOption	mipFilter = FO_NONE;
		AddressMode		addrUMode = AM_WRAP;
		AddressMode		addrVMode = AM_WRAP;
		AddressMode		addrWMode = AM_WRAP;
		ui8				maxAnisotropy = 16;
		ComparisonFunc	cmpFunc = CF_NEVER;
		Color			borderColor = Color::BLACK;
		float			minLOD = 0.f;
		float			maxLOD = Math::MAX_FLOAT;
		float			mipLODBias = 0.f;
	};
	typedef ResRef<SamplerState> SamplerStatePtr;

    class MultisampleState : public RenderState
    {
    public:
    };
	typedef ResRef<MultisampleState> MultisampleStatePtr;
}
