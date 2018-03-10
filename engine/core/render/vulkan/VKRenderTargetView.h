#pragma once

#include "Render/RenderTargetView.h"

namespace LORD
{
	class GLES2TargetViewGPUProxy;

	class LORD_GLES2RENDER_API GLES2RenderTargetView: public RenderTargetView
	{
	public:
		GLES2RenderTargetView(PixelFormat pixFmt, uint width, uint height);
		GLES2RenderTargetView(uint hRBO, uint width, uint height);
		~GLES2RenderTargetView();

		void onAttached(uint idx = 0);
		void onDetached();

	
	private:
		GLES2TargetViewGPUProxy* m_gpuProxy;
	};
}