#include "GLES2DepthStencilViewTasks.h"
#include "GLES2DepthStencilVIewGPUProxy.h"

namespace LORD
{

	GLES2DepthStencilViewTaskBase::GLES2DepthStencilViewTaskBase(GLES2DepthStencilViewGPUProxy* proxy)
		: m_proxy(proxy)
	{
	}

	GLES2DepthStencilViewTaskCreateView::GLES2DepthStencilViewTaskCreateView(GLES2DepthStencilViewGPUProxy* proxy, PixelFormat pixFmt, uint width, uint height)
		: GLES2DepthStencilViewTaskBase(proxy)
		, m_pixFmt(pixFmt)
		, m_width(width)
		, m_height(height)
	{
	}

	void GLES2DepthStencilViewTaskCreateView::Execute()
	{
		m_proxy->createView(m_pixFmt, m_width, m_height);
	}

	GLES2DepthStencilViewTaskDestroyView::GLES2DepthStencilViewTaskDestroyView(GLES2DepthStencilViewGPUProxy* proxy)
		: GLES2DepthStencilViewTaskBase(proxy)
	{
	}

	void GLES2DepthStencilViewTaskDestroyView::Execute()
	{
		m_proxy->destroyView();
		LordSafeDelete(m_proxy);
	}

	GLES2DepthStencilViewTaskOnAttached::GLES2DepthStencilViewTaskOnAttached(GLES2DepthStencilViewGPUProxy* proxy, PixelFormat pixFmt)
		: GLES2DepthStencilViewTaskBase(proxy)
		, m_pixFmt(pixFmt)
	{
	}

	void GLES2DepthStencilViewTaskOnAttached::Execute()
	{
		m_proxy->onAttached(m_pixFmt);
	}

	GLES2DepthStencilViewTaskOnDetached::GLES2DepthStencilViewTaskOnDetached(GLES2DepthStencilViewGPUProxy* proxy)
		: GLES2DepthStencilViewTaskBase(proxy)
	{
	}

	void GLES2DepthStencilViewTaskOnDetached::Execute()
	{
		m_proxy->onDetached();
	}

}