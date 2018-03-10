#pragma once

namespace Echo
{
	class GLES2TargetViewGPUProxy
	{
		friend class GLES2RenderTargetView;
	public:
		void createRenderBuffer(PixelFormat pixFmt, ui32 width, ui32 height);
		void deleteRenderBuffer();
		void onAttached();

	private:
		ui32 m_hRBO;
	};
}