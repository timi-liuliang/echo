#pragma once

#include "Render/RenderTargetView.h"

namespace Echo
{
	class GLES2TargetViewGPUProxy;

	class GLES2RenderTargetView: public RenderTargetView
	{
	public:
		GLES2RenderTargetView(PixelFormat pixFmt, ui32 width, ui32 height);
		GLES2RenderTargetView(ui32 hRBO, ui32 width, ui32 height);
		~GLES2RenderTargetView();

		void onAttached(ui32 idx = 0);
		void onDetached();

#ifdef ECHO_RENDER_THREAD
	private:
		GLES2TargetViewGPUProxy* m_gpuProxy;
#else
	private:
		ui32 m_hRBO;
#endif
	};
}