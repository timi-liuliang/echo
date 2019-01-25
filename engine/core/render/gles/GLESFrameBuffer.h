#pragma once

#include "interface/FrameBuffer.h"
#include "GLESRenderTargetView.h"
#include "GLESDepthStencilView.h"

namespace Echo
{
	class GLES2FrameBuffer: public FrameBuffer
	{
	public:
		GLES2FrameBuffer();
		~GLES2FrameBuffer();

		GLES2FrameBuffer(GLuint hFBO);
	public:
		//void			bind();

		//GLuint			getFBOHandle() const;

	private:
		GLuint			m_hFBO;
	};
}
