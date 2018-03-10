#include "Render/RenderThread.h"
#include "GLES2TargetViewTasks.h"
#include "GLES2TargetViewGPUProxy.h"

namespace LORD
{

	GLES2TargetViewTaskBase::GLES2TargetViewTaskBase(GLES2TargetViewGPUProxy* proxy)
		: m_proxy(proxy)
	{
	}


	GLES2TargetViewTaskCreateRenderBuffer::GLES2TargetViewTaskCreateRenderBuffer(GLES2TargetViewGPUProxy* proxy, PixelFormat pixFmt, uint width, uint height)
		: GLES2TargetViewTaskBase(proxy)
		, m_pixFmt(pixFmt)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2TargetViewTaskCreateRenderBuffer::Execute()
	{
		m_proxy->createRenderBuffer(m_pixFmt, m_width, m_height);
	}


	GLES2TargetViewTaskDeleteRenderBuffer::GLES2TargetViewTaskDeleteRenderBuffer(GLES2TargetViewGPUProxy* proxy)
		: GLES2TargetViewTaskBase(proxy)
	{
	}

	void GLES2TargetViewTaskDeleteRenderBuffer::Execute()
	{
		m_proxy->deleteRenderBuffer();
		LordSafeDelete(m_proxy);
	}


	GLES2TargetViewTaskOnAttached::GLES2TargetViewTaskOnAttached(GLES2TargetViewGPUProxy* proxy)
		: GLES2TargetViewTaskBase(proxy)
	{
	}

	void GLES2TargetViewTaskOnAttached::Execute()
	{
		m_proxy->onAttached();
	}

}