#pragma once

namespace LORD
{
	struct BlendStateParams
	{
		bool isChangeA2C;
		bool isChangeBlendEnable;
		bool isChangeBlendOp;
		bool isChangeBlendFunc;
		bool isChangeColorWriteMask;
		bool isChangeBlendFactor;

		bool isA2CEnable;
		bool isBlendEnable;
		GLenum blend_op;
		GLenum alpha_blend_op;
		GLenum src_blend;
		GLenum dst_blend;
		GLenum src_alpha_blend;
		GLenum dst_alpha_blend;
		GLboolean red_mask;
		GLboolean green_mask;
		GLboolean blue_mask;
		GLboolean alpha_mask;
		Color blendFactor;
	};

	struct DepthStencilStateParams
	{
		bool isChangeDepthTest;
		bool isChangeDepthWrite;
		bool isWriteDepth;
		bool isSetDepthFunc;
		bool isChangeStencilTest;
		bool isSetStencilFuncFront;
		bool isSetStencilOpFront;
		bool isSetStencilMaskFront;
		bool isSetStencilFuncBack;
		bool isSetStencilOpBack;
		bool isSetStencilMaskBack;
		bool isEnableDepthTest;
		bool isEnableStencilTest;
		
		uint frontStencilRef;
		ui16 frontStencilReadMask;
		ui16 frontStencilWriteMask;
		uint backStencilRef;
		ui16 backStencilReadMask;
		ui16 backStencilWriteMask;
		GLenum depthFunc;
		GLenum frontStencilFunc;
		GLenum frontStencilFailOP;
		GLenum frontStencilDepthFailOP;
		GLenum frontStencilPassOP;
		GLenum backStencilFunc;
		GLenum backStencilFailOP;
		GLenum backStencilDepthFailOP;
		GLenum backStencilPassOP;
	};

	class GLES2SamplerStateGPUProxy
	{
	public:
		void active(const SamplerState::SamplerDesc* curr_desc, const SamplerState::SamplerDesc* prev_desc);

		GLint m_glMinFilter;
		GLint m_glMagFilter;
		GLint m_glAddrModeU;
		GLint m_glAddrModeV;
		GLint m_glAddrModeW;
	};

	void ActiveBlendState(const BlendStateParams& params);
	void ActiveDepthStencilState(const DepthStencilStateParams& params);
	void ActiveRasterizerState(const RasterizerState::RasterizerDesc& myDesc, const RasterizerState::RasterizerDesc& currDesc, GLenum frontFace, bool needCompare);
}