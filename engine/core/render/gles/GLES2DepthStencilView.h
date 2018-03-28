#ifndef __ECHO_GLES2DEPTHSTENCILVIEW_H__
#define __ECHO_GLES2DEPTHSTENCILVIEW_H__

#include "Render/DepthStencilView.h"

namespace Echo
{
	class GLES2DepthStencilViewGPUProxy;

	class GLES2DepthStencilView : public DepthStencilView
	{
	public:
		GLES2DepthStencilView(PixelFormat pixFmt, ui32 width, ui32 height);
		~GLES2DepthStencilView();

	public:
		void		onAttached();
		void		onDetached();

#ifdef ECHO_RENDER_THREAD
	private:
		GLES2DepthStencilViewGPUProxy* m_gpuProxy;
#else
		GLuint		getRBOHandle() const;

	private:
		GLuint		m_hRBO;
#endif
	};
}

#endif
