#include "GLES2RenderStd.h"
#include "Render/RenderThread.h"
#include "GLES2Renderer.h"
#include "GLES2DepthStencilView.h"
#include "GLES2Mapping.h"
#include "GLES2DepthStencilVIewGPUProxy.h"
#include "GLES2DepthStencilViewTasks.h"
#include <Foundation/Util/Exception.h>
#include "Render/PixelFormat.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"



namespace LORD
{
	GLES2DepthStencilView::GLES2DepthStencilView(PixelFormat pixFmt, uint width, uint height)
		: DepthStencilView(pixFmt, width, height)
		, m_gpuProxy(LordNew(GLES2DepthStencilViewGPUProxy))
	{
		TRenderTask<GLES2DepthStencilViewTaskCreateView>::CreateTask(m_gpuProxy, pixFmt, width, height);
	}
	
	GLES2DepthStencilView::~GLES2DepthStencilView()
	{
		TRenderTask<GLES2DepthStencilViewTaskDestroyView>::CreateTask(m_gpuProxy);
	}
	
	void GLES2DepthStencilView::onAttached()
	{
		TRenderTask<GLES2DepthStencilViewTaskOnAttached>::CreateTask(m_gpuProxy, m_pixFmt);
	}
	
	void GLES2DepthStencilView::onDetached()
	{
		TRenderTask<GLES2DepthStencilViewTaskOnDetached>::CreateTask(m_gpuProxy);
	}
	


}
