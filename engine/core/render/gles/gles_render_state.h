#pragma once

#include "base/render_state.h"
#include "gles_render_base.h"

namespace Echo
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

		ui32 frontStencilRef;
		ui16 frontStencilReadMask;
		ui16 frontStencilWriteMask;
		ui32 backStencilRef;
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

	class GLESBlendState: public BlendState
	{
	public:
		GLESBlendState(const BlendDesc &desc);
		~GLESBlendState();

	public:
		void						active();

	private:
		void						create();

	private:
		GLenum						m_glBlendOP;
		GLenum						m_glAlphaBlendOP;
		GLenum						m_glSrcBlend;
		GLenum						m_glDstBlend;
		GLenum						m_glSrcAlphaBlend;
		GLenum						m_glDstAlphaBlend;
		GLboolean					m_glRedMask;
		GLboolean					m_glGreenMask;
		GLboolean					m_glBlueMask;
		GLboolean					m_glAlphaMask;
	};
	
	class GLESDepthStencilState: public DepthStencilState
	{
	public:
		GLESDepthStencilState(const DepthStencilDesc& desc);
		~GLESDepthStencilState();

	public:
		void						active();

	private:
		void						create();

	private:
		GLboolean					m_glDepthMask;
		GLenum						m_glDepthFunc;
		GLenum						m_glFrontStencilFunc;
		GLenum						m_glFrontStencilFailOP;
		GLenum						m_glFrontStencilDepthFailOP;
		GLenum						m_glFrontStencilPassOP;
		GLenum						m_glBackStencilFunc;
		GLenum						m_glBackStencilFailOP;
		GLenum						m_glBackStencilDepthFailOP;
		GLenum						m_glBackStencilPassOP;
	};

	class GLESRasterizerState: public RasterizerState
	{
	public:
		GLESRasterizerState(const RasterizerDesc& desc);
		~GLESRasterizerState();

	public:
		void						active();

	private:
		void						create();

	private:
		GLenum						m_glFrontFace;
	};

	class GLESSamplerState: public SamplerState
	{
		friend class GLESRenderer;
	private:
		GLESSamplerState(const SamplerDesc& desc);
		~GLESSamplerState();

	public:
		void			active(const SamplerState* pre) const;
		GLint			getGLMinFilter() const;
		GLint			getGLMagFilter() const;
		GLint			getGLAddrModeU() const;
		GLint			getGLAddrModeV() const;
		GLint			getGLAddrModeW() const;

	private:
		void			create();

	private:
		GLint			m_glMinFilter;
		GLint			m_glMagFilter;
		GLint			m_glAddrModeU;
		GLint			m_glAddrModeV;
		GLint			m_glAddrModeW;
	};
}

