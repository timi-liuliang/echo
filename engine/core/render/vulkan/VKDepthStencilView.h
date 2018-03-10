#ifndef __LORD_GLES2DEPTHSTENCILVIEW_H__
#define __LORD_GLES2DEPTHSTENCILVIEW_H__

#include "Render/DepthStencilView.h"

namespace LORD
{
	class GLES2DepthStencilViewGPUProxy;

	class LORD_GLES2RENDER_API GLES2DepthStencilView: public DepthStencilView
	{
	public:
		GLES2DepthStencilView(PixelFormat pixFmt, uint width, uint height);
		~GLES2DepthStencilView();

	public:
		void		onAttached();
		void		onDetached();

	
	private:
		GLES2DepthStencilViewGPUProxy* m_gpuProxy;
	};
}

#endif
