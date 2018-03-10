#include "GLES2RenderStd.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"
#include "GLES2Renderer.h"
#include "GLES2RenderTargetView.h"
#include "GLES2Mapping.h"
#include "GLES2TargetViewGPUProxy.h"
#include "GLES2TargetViewTasks.h"
#include "Foundation/Util/Exception.h"


namespace LORD
{
	GLES2RenderTargetView::GLES2RenderTargetView(PixelFormat pixFmt, uint width, uint height)
		: RenderTargetView(pixFmt, width, height)
		, m_gpuProxy(LordNew(GLES2TargetViewGPUProxy))
	{
		TRenderTask<GLES2TargetViewTaskCreateRenderBuffer>::CreateTask(m_gpuProxy, pixFmt, width, height);
	}

	GLES2RenderTargetView::GLES2RenderTargetView(uint hRBO, uint width, uint height)
		: RenderTargetView(PF_RGBA8_UNORM, width, height)
		, m_gpuProxy(LordNew(GLES2TargetViewGPUProxy))
	{
		m_gpuProxy->m_hRBO = hRBO;
	}

	GLES2RenderTargetView::~GLES2RenderTargetView()
	{
		TRenderTask<GLES2TargetViewTaskDeleteRenderBuffer>::CreateTask(m_gpuProxy);
	}
	
	void GLES2RenderTargetView::onAttached(uint idx)
	{
		LordAssertX(idx == 0, "GLES2 only support one render target.");
		m_attIdx = idx;
		TRenderTask<GLES2TargetViewTaskOnAttached>::CreateTask(m_gpuProxy);
	}
	
	void GLES2RenderTargetView::onDetached()
	{
		m_attIdx = -1;
	}

}
