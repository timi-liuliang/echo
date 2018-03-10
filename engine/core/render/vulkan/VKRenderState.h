#ifndef __LORD_GLES2RENDERSTATE_H__
#define __LORD_GLES2RENDERSTATE_H__

#include "Render/RenderState.h"

namespace LORD
{
	class GLES2SamplerStateGPUProxy;
	

	class LORD_GLES2RENDER_API GLES2BlendState: public BlendState
	{
	public:
		GLES2BlendState();
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
	
	class LORD_GLES2RENDER_API GLES2DepthStencilState: public DepthStencilState
	{
	public:
		GLES2DepthStencilState();
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

	class LORD_GLES2RENDER_API GLES2RasterizerState: public RasterizerState
	{
	public:
		GLES2RasterizerState();
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

	class LORD_GLES2RENDER_API GLES2SamplerState: public SamplerState
	{
		friend class GLES2Renderer;
	private:
		GLES2SamplerState();
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
		GLES2SamplerStateGPUProxy* m_sampler_proxy;
	};
}

#endif
