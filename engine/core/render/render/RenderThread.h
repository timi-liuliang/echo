#pragma once
#include "engine/core/Memory/MemManager.h"
//#include "scl/thread.h"

namespace Echo
{
	class Thread;
	class ThreadEvent;
	class RenderTask;
	class RenderTaskQueue;

	class RenderThread final
	{
	public:
		struct Config
		{
			bool enable;
		};
	public:
		RenderThread(const Config& config);
		~RenderThread();

		void Start();
		void End();

		void run();
		void pushTask(RenderTask* task);
		void enableThreadedRendering(bool enable);
		void setThreadRendering(bool enable) { m_enable_threaded_rendering = enable; }
		void flushRenderTasks();
		void syncFrame();
		bool isThreadedRendering() const { return m_enable_threaded_rendering; }

		void ExecuteTasksForOneFrame();

		void OnFrameEnd();

		static void threadProc(void* render_thread);

	private:
		void executeTask();

		RenderTaskQueue* m_task_queue;
		bool m_enable_threaded_rendering;
		bool m_thread_end;
		bool m_frame_end;
		Thread* m_thread;
#if defined(ECHO_RENDER_THREAD_LOCK_FREE)
        //scl::semaphore  m_flush_semaphore;
        //scl::semaphore  m_push_semaphore;
#else
        ThreadEvent* m_queue_empty_event;
#endif
		int m_render_finish_signaled;

	};

	extern RenderThread* g_render_thread;
}
