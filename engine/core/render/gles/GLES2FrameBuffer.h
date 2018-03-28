#pragma once

#include "GLES2RenderTargetView.h"
#include "GLES2DepthStencilView.h"
#include "Render/FrameBuffer.h"

namespace Echo
{
	class GLES2FrameBuffer: public FrameBuffer
	{
	public:
		GLES2FrameBuffer();
		~GLES2FrameBuffer();

#ifndef ECHO_RENDER_THREAD
		GLES2FrameBuffer(GLuint hFBO);
	public:
		void			clear(Dword clearFlags, const Color& color, float depth, int stencil);
		inline void		swap();
		void			bind();

		GLuint			getFBOHandle() const;

	private:
		GLuint			m_hFBO;
#endif
	};
}
