#pragma once

#include "base/RenderState.h"
#include "GLESRenderBase.h"

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

	class GLES2BlendState: public BlendState
	{
	public:
		GLES2BlendState(const BlendDesc &desc);
		~GLES2BlendState();

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
	
	class GLES2DepthStencilState: public DepthStencilState
	{
	public:
		GLES2DepthStencilState(const DepthStencilDesc& desc);
		~GLES2DepthStencilState();

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

	class GLES2RasterizerState: public RasterizerState
	{
	public:
		GLES2RasterizerState(const RasterizerDesc& desc);
		~GLES2RasterizerState();

	public:
		void						active();

	private:
		void						create();

	private:
		//GLenum					m_glPolygonMode;
		//GLenum					m_glShadeModel;
		GLenum						m_glFrontFace;
	};

	class GLES2SamplerState: public SamplerState
	{
		friend class GLES2Renderer;
	private:
		GLES2SamplerState(const SamplerDesc& desc);
		~GLES2SamplerState();

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

