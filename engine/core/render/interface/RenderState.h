#pragma once

#include "engine/core/math/color.h"

namespace Echo
{
	class RenderState
	{
	public:
		RenderState();
		virtual ~RenderState();

		enum ColorMask
		{
			CMASK_NONE		= 0x00000000,
			CMASK_RED		= 0x00000001,
			CMASK_GREEN		= 0x00000002,
			CMASK_BLUE		= 0x00000004,
			CMASK_ALPHA		= 0x00000008,
			CMASK_COLOR     = CMASK_RED | CMASK_GREEN | CMASK_BLUE,
			CMASK_ALL		= CMASK_RED | CMASK_GREEN | CMASK_BLUE | CMASK_ALPHA
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

		BlendState();
		BlendState(const BlendDesc& desc);
		virtual ~BlendState();

	public:
		virtual void					active() = 0;
		const BlendDesc&				getDesc() const;

	protected:
		BlendDesc		m_desc;
	};

	class DepthStencilState: public RenderState
	{
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

		struct DepthStencilDesc
		{
			bool				bDepthEnable;
			bool				bWriteDepth;
			ComparisonFunc		depthFunc;

			bool				bFrontStencilEnable;
			ComparisonFunc		frontStencilFunc;
			ui16				frontStencilReadMask;
			ui16				frontStencilWriteMask;
			StencilOperation	frontStencilFailOP;
			StencilOperation	frontStencilDepthFailOP;
			StencilOperation	frontStencilPassOP;
			ui32				frontStencilRef;

			bool				bBackStencilEnable;
			ComparisonFunc		backStencilFunc;
			ui16				backStencilReadMask;
			ui16				backStencilWriteMask;
			StencilOperation	backStencilFailOP;
			StencilOperation	backStencilDepthFailOP;
			StencilOperation	backStencilPassOP;
			ui32				backStencilRef;

			void reset()
			{
				bDepthEnable			= true;
				bWriteDepth				= true;
				depthFunc				= CF_LESS;

				bFrontStencilEnable		= false;
				frontStencilFunc		= CF_ALWAYS;
				frontStencilReadMask	= 0xffff;
				frontStencilWriteMask	= 0xffff;
				frontStencilFailOP		= SOP_KEEP;
				frontStencilDepthFailOP	= SOP_INCR;
				frontStencilPassOP		= SOP_KEEP;
				frontStencilRef			= 1;

				bBackStencilEnable		= false;
				backStencilFunc			= CF_ALWAYS;
				backStencilReadMask		= 0xffff;
				backStencilWriteMask	= 0xffff;
				backStencilFailOP		= SOP_KEEP;
				backStencilDepthFailOP	= SOP_DECR;
				backStencilPassOP		= SOP_KEEP;
				backStencilRef			= 1;
			}

			DepthStencilDesc()
			{
				reset();
			}
		};

		DepthStencilState();
		DepthStencilState(const DepthStencilDesc& desc);
		virtual ~DepthStencilState();

	public:
        virtual void						active() {}
		const DepthStencilDesc&				getDesc() const;

	protected:
		DepthStencilDesc		m_desc;
	};

	class RasterizerState : public RenderState
	{
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

		struct RasterizerDesc
		{
			PolygonMode			polygonMode;
			ShadeModel			shadeModel;
			CullMode			cullMode;
			bool				bFrontFaceCCW;

			// depth bias
			// z = depthBiasFactor * DZ + depthBias
			// DZ is max depth slope
			float				depthBias;
			float				depthBiasFactor;

			bool				bDepthClip;
			bool				bScissor;
			bool				bMultisample;

            float               lineWidth = 1.f;

			void reset()
			{
				polygonMode		= PM_FILL;
				shadeModel		= SM_GOURAND;
				cullMode		= CULL_BACK;
				bFrontFaceCCW	= false;
				depthBias		= 0.0f;
				depthBiasFactor	= 0.0f;
				bDepthClip		= true;
				bScissor		= false;
				bMultisample	= false;
			}

			RasterizerDesc()
			{
				reset();
			}
		};

		RasterizerState();
		RasterizerState(const RasterizerDesc& desc);
		virtual ~RasterizerState();

	public:
        virtual void					active() {}
		const RasterizerDesc&			getDesc() const;

	protected:
		RasterizerDesc		m_desc;
	};

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
        virtual void active(const SamplerState* pre) const {}
		const SamplerDesc& getDesc() const;

	protected:
		SamplerState();
		virtual ~SamplerState();
		SamplerState(const SamplerDesc &desc);

		SamplerDesc		m_desc;
	};
}
