#include "RenderThread.h"
#include "RenderTask.h"
#include "RenderTaskQueue.h"
#include "UniformCache.h"
#include "engine/core/Thread/Threading.h"
#include "engine/core/Util/AssertX.h"
#include "engine/core/Memory/MemManager.h"
#include "engine/core/Thread/Threading.h"

//#include "scl/thread.h"

namespace Echo
{
	RenderThread* g_render_thread = nullptr;

	class FlushRenderTask
	{
	public:
    #ifdef ECHO_RENDER_THREAD_LOCK_FREE
		//FlushRenderTask(scl::semaphore* sem) : m_semaphore(sem) { }
    #else
		FlushRenderTask(ThreadEvent* thread_event) : m_thread_event(thread_event) { }
    #endif
        
		~FlushRenderTask()
		{
		}

		virtual void Execute()
		{
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
            //m_semaphore->post();
#else
            m_thread_event->SetEvent();
#endif
		}

	private:
    
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
        //scl::semaphore* m_semaphore;
#else
        ThreadEvent* m_thread_event;
#endif
	};

	RenderThread::RenderThread(const Config& config)
		: m_task_queue(EchoNew(RenderTaskQueue))
		, m_enable_threaded_rendering(config.enable)
		, m_thread_end(false)
		, m_thread(EchoNew(Thread))
		, m_render_finish_signaled(0)
		, m_frame_end(false)
	{
		RenderTask::InitTaskAllocator();
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
       // m_flush_semaphore.create();
       // m_push_semaphore.create();
#else
        m_queue_empty_event = EchoNew(ThreadEvent);
#endif
	}

	RenderThread::~RenderThread()
	{
		while (!m_enable_threaded_rendering && !m_render_finish_signaled)
		{
			std::this_thread::yield();
		}

		EchoSafeDelete(m_task_queue, RenderTaskQueue);
		EchoSafeDelete(m_thread, Thread);
#if !defined(ECHO_RENDER_THREAD_LOCK_FREE)
        EchoSafeDelete(m_queue_empty_event, ThreadEvent);
#endif
		RenderTask::DestroyTaskAllocator();
	}

	void RenderThread::run()
	{
		if (m_enable_threaded_rendering)
		{
			//SetThreadAffinityMask(2);
		}
		
		while (!m_thread_end)
		{
			if (!m_enable_threaded_rendering)
			{
				ThreadSleepByMilliSecond(1000.0f);
				continue;
			}
			executeTask();
		}
		if (!m_enable_threaded_rendering && m_thread_end)
		{
			m_render_finish_signaled = 1;
		}
	}

	void RenderThread::pushTask(RenderTask* task)
	{
		if (!m_enable_threaded_rendering)
		{
			task->Execute();
			return;
		}

		m_task_queue->push(task);
#if defined(ECHO_RENDER_THREAD_LOCK_FREE)
        //m_push_semaphore.post();
#else
        m_queue_empty_event->SetEvent();
#endif
	}

	void RenderThread::enableThreadedRendering(bool enable)
	{
		if (m_enable_threaded_rendering == enable)
		{
			return;
		}

		if (m_enable_threaded_rendering)
		{
			flushRenderTasks();
			m_enable_threaded_rendering = false;
		}
		else
		{
			m_enable_threaded_rendering = true;
		}
	}

	void RenderThread::Start()
	{
#ifndef ECHO_PLATFORM_ANDROID
		m_thread->Start(threadProc, this);
#endif
	}

	void RenderThread::End()
	{
		EchoAssert(m_thread_end == false);

		flushRenderTasks();
		m_thread_end = true;
        
#if defined(ECHO_RENDER_THREAD_LOCK_FREE)
		//m_push_semaphore.post();
#else
        m_queue_empty_event->SetEvent();
#endif

#ifndef ECHO_PLATFORM_ANDROID
		m_thread->Join();
#endif
	}

	void RenderThread::threadProc(void* render_thread)
	{
		static_cast<RenderThread*>(render_thread)->run();
	}
 
	void RenderThread::flushRenderTasks()
	{
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
        //TRenderTask<FlushRenderTask>::CreateTask(&m_flush_semaphore);
        //m_flush_semaphore.wait();
#else
        Echo::ThreadEvent* flush_render_event = EchoNew(Echo::ThreadEvent);
        TRenderTask<FlushRenderTask>::CreateTask(flush_render_event);
        flush_render_event->WaitEvent();
        EchoSafeDelete(flush_render_event, ThreadEvent);
#endif
	}

	void RenderThread::syncFrame()
	{
		flushRenderTasks();
		RenderTask::ResetTaskAllocator();
		g_uniform_cache->ResetInfoCache();
	}

	void RenderThread::ExecuteTasksForOneFrame()
	{
		while (!m_frame_end)
		{
			executeTask();
		}
		m_frame_end = false;
	}

	void RenderThread::OnFrameEnd()
	{
		m_frame_end = true;
	}

	void RenderThread::executeTask()
	{
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
			RenderTask* task = m_task_queue->pop();
			if (task == nullptr)
			{
                //m_push_semaphore.wait();
			}
            else
            {
                task->Execute();
            }
#else
			RenderTask* task = m_task_queue->pop();
			if (task == nullptr)
			{
                m_queue_empty_event->WaitEvent();
				return;
			}
			task->Execute();
#endif
	}

}

void SuspendRenderThread()
{
	if (!Echo::g_render_thread || !Echo::g_render_thread->isThreadedRendering())
		return;

	Echo::g_render_thread->flushRenderTasks();
}

void ResumeRenderThread()
{
	if (!Echo::g_render_thread || !Echo::g_render_thread->isThreadedRendering())
		return;
}