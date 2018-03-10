#pragma once
#include "Render/PixelFormat.h"

namespace LORD
{
	class GLES2DepthStencilViewGPUProxy
	{
		friend class GLES2DepthStencilView;
	public:
		void createView(PixelFormat pixFmt, uint width, uint height);
		void destroyView();
		void onAttached(PixelFormat pixFmt);
		void onDetached();

	private:
		uint m_hRBO;
	};
}