#include "Render/RenderThread.h"
#include "RenderTask.h"
#include "RenderTaskAllocator.h"

namespace Echo
{
	static SmallTaskAllocator* g_smallTaskAllocator;
	
	ui32 RenderTask::SMALL_TASK_SIZE = 128;

	void FlushRenderTasks()
	{
		g_render_thread->flushRenderTasks();
	}

	RenderTask::RenderTask()
	{
	}
	RenderTask::~RenderTask()
	{
	}

	void RenderTask::InitTaskAllocator()
	{
		g_smallTaskAllocator = EchoNew(SmallTaskAllocator);
	}

	void RenderTask::DestroyTaskAllocator()
	{
		EchoSafeDelete(g_smallTaskAllocator, SmallTaskAllocator);
	}

	void* RenderTask::AllocTaskMem()
	{
		return g_smallTaskAllocator->alloc();
	}

	void RenderTask::FreeTaskMem(void* mem)
	{
		g_smallTaskAllocator->free(mem);
	}

	void RenderTask::ResetTaskAllocator()
	{
		g_smallTaskAllocator->reset();
	}

}