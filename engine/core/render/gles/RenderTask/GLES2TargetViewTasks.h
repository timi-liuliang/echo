#pragma once
#include "Render/PixelFormat.h"

namespace Echo
{
	class GLES2TargetViewGPUProxy;

	class GLES2TargetViewTaskBase
	{
	public:
		GLES2TargetViewTaskBase(GLES2TargetViewGPUProxy* proxy);

	protected:
		GLES2TargetViewGPUProxy* m_proxy;
	};

	class GLES2TargetViewTaskCreateRenderBuffer : public GLES2TargetViewTaskBase
	{
	public:
		GLES2TargetViewTaskCreateRenderBuffer(GLES2TargetViewGPUProxy* proxy, PixelFormat pixFmt, ui32 width, ui32 height);
		void Execute();

	private:
		PixelFormat m_pixFmt;
		ui32 m_width;
		ui32 m_height;
	};

	class GLES2TargetViewTaskDeleteRenderBuffer : public GLES2TargetViewTaskBase
	{
	public:
		GLES2TargetViewTaskDeleteRenderBuffer(GLES2TargetViewGPUProxy* proxy);
		void Execute();
	};

	class GLES2TargetViewTaskOnAttached : public GLES2TargetViewTaskBase
	{
	public:
		GLES2TargetViewTaskOnAttached(GLES2TargetViewGPUProxy* proxy);
		void Execute();
	};
}