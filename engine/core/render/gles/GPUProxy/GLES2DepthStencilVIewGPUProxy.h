#pragma once
#include "Render/PixelFormat.h"

namespace Echo
{
	class GLES2DepthStencilViewGPUProxy
	{
		friend class GLES2DepthStencilView;
	public:
		void createView(PixelFormat pixFmt, ui32 width, ui32 height);
		void destroyView();
		void onAttached(PixelFormat pixFmt);
		void onDetached();

	private:
		ui32 m_hRBO;
	};
}