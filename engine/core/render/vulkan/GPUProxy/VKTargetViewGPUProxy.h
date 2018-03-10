#pragma once

namespace LORD
{
	class GLES2TargetViewGPUProxy
	{
		friend class GLES2RenderTargetView;
	public:
		void createRenderBuffer(PixelFormat pixFmt, uint width, uint height);
		void deleteRenderBuffer();
		void onAttached();

	private:
		uint m_hRBO;
	};
}