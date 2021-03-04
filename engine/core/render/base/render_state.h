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

		struct RenderTargetBlendDesc
		{
			
		};

		struct BlendDesc
		{
			bool				bBlendEnable;
			BlendFactor			srcBlend;
			BlendFactor			dstBlend;
			BlendOperation		blendOP;
			BlendFactor			srcAlphaBlend;
			BlendFactor			dstAlphaBlend;
			BlendOperation		alphaBlendOP;
			ui8					colorWriteMask;

			bool				bA2CEnable;
			bool				bIndependBlendEnable;
			Color				blendFactor;

			void reset()
			{
				bBlendEnable	= false;
				srcBlend		= BF_ONE;
				dstBlend		= BF_ZERO;
				blendOP			= BOP_ADD;
				srcAlphaBlend	= BF_ONE;
				dstAlphaBlend	= BF_ZERO;
				alphaBlendOP	= BOP_ADD;
				colorWriteMask	= CMASK_COLOR;

				bA2CEnable = false;
				bIndependBlendEnable	= false;

				blendFactor.r = 0;
				blendFactor.g = 0;
				blendFactor.b = 0;
				blendFactor.a = 0;
			}

			BlendDesc()
			{
				reset();
			}
		};

		BlendState(const BlendDesc& desc);
		virtual ~BlendState();

	public:
		const BlendDesc&	getDesc() const;

	protected:
		BlendDesc		m_desc;
	};

	class DepthStencilState: public RenderState
	{
		ECHO_RES(DepthStencilState, RenderState, ".edss", DepthStencilState::create, Res::load)

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
		bool isDepthEnable() const { return bDepthEnable; }
		void setDepthEnable(bool enable);

		// write depth
		bool isWriteDepth() const { return bWriteDepth; }
		void setWriteDepth(bool writeDepth);

	public:
		bool				bDepthEnable = true;
		bool				bWriteDepth = true;
		ComparisonFunc		depthFunc = CF_LESS;

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
		ECHO_RES(DepthStencilState, RenderState, ".ers", RasterizerState::create, Res::load)

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
		CullMode getCullMode() { return cullMode; }
		void setCullMode(CullMode cullMode);

		// Frontface
		bool isFrontFaceCCW() const { return m_frontFaceCCW; }

		// depth bias
		float getDepthBias() const { return m_depthBias; }

		// depth bias factor
		float getDepthBiasFactor() const { return m_depthBiasFactor; }

		// scissor
		bool isScissor() const { return m_scissor; }

	protected:
		PolygonMode			polygonMode = PM_FILL;
		ShadeModel			shadeModel = SM_GOURAND;
		CullMode			cullMode = CULL_BACK;
		bool				m_frontFaceCCW = false;

		// depth bias
		// z = depthBiasFactor * DZ + depthBias
		// DZ is max depth slope
		float				m_depthBias = 0.f;
		float				m_depthBiasFactor = 0.f;

		bool				bDepthClip = true;
		bool				m_scissor = false;
		bool				bMultisample = false;

		float               m_lineWidth = 1.f;
	};
	typedef ResRef<RasterizerState> RasterizerStatePtr;

	class SamplerState : public RenderState
	{
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

		// sampler description
		struct SamplerDesc
		{
			FilterOption	minFilter;
			FilterOption	magFilter;
			FilterOption	mipFilter;
			AddressMode		addrUMode;
			AddressMode		addrVMode;
			AddressMode		addrWMode;
			ui8				maxAnisotropy;
			ComparisonFunc	cmpFunc;
			Color			borderColor;
			float			minLOD;
			float			maxLOD;
			float			mipLODBias;

			// constructor
			SamplerDesc()
			{
				reset();
			}

			// reset
			void reset()
			{
				minFilter = FO_LINEAR;
				magFilter = FO_LINEAR;
				mipFilter = FO_NONE;
				addrUMode = AM_WRAP;
				addrVMode = AM_WRAP;
				addrWMode = AM_WRAP;
				maxAnisotropy = 16;
				cmpFunc		= CF_NEVER;
				borderColor = Color::BLACK;
				minLOD		= 0.0f;
				maxLOD		= Math::MAX_FLOAT;
				mipLODBias	= 0.0f;
			}
			bool operator==(const SamplerDesc& rhs) const
			{
				return minFilter == rhs.minFilter &&
					magFilter == rhs.magFilter &&
					mipFilter == rhs.mipFilter &&
					addrUMode == rhs.addrUMode &&
					addrWMode == rhs.addrWMode &&
					maxAnisotropy == rhs.maxAnisotropy &&
					cmpFunc == rhs.cmpFunc &&
					borderColor == rhs.borderColor;
			}
		};

	public:
		const SamplerDesc& getDesc() const;

	protected:
		virtual ~SamplerState();
		SamplerState(const SamplerDesc &desc);

		SamplerDesc		m_desc;
	};

    class MultisampleState : public RenderState
    {
    public:
    };
}
