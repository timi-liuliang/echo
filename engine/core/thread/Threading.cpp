#include "Threading.h"

#ifndef ECHO_PLATFORM_WINDOWS
	#include <unistd.h>
#endif

namespace Echo
{
	// sleep
	void ThreadSleepByMilliSecond(float millSecond)
	{
#ifdef ECHO_PLATFORM_WINDOWS
		Sleep(static_cast<Echo::Dword>(millSecond));
#elif defined(ECHO_PLATFORM_IOS)
		usleep(static_cast<int>(millSecond*1000.f));
#elif defined(ECHO_PLATFORM_ANDROID)
		usleep(static_cast<int>(millSecond*1000.f));
#endif
	}

#ifdef ECHO_PLATFORM_HTML5
	ThreadEvent::ThreadEvent() {}
	ThreadEvent::~ThreadEvent() {}
	void ThreadEvent::WaitEvent() {}
	void ThreadEvent::SetEvent() {}

	void Thread::Start(ThreadFunc func, void* param) {}
	void Thread::Join() {}
	void Thread::detach(){}
#else
	void Thread::Start(ThreadFunc func, void* param)
	{
		m_thread = std::thread(func, param);
	}

	void Thread::Join()
	{
		m_thread.join();
	}

	void Thread::detach()
	{
		m_thread.detach();
	}

	ThreadEvent::ThreadEvent()
		: m_signaled(false)
	{
	}

	ThreadEvent::~ThreadEvent()
	{
	}

	void ThreadEvent::WaitEvent()
	{
		std::unique_lock<std::mutex> lck(m_mtx);
		m_cv.wait(lck, [this]() {return m_signaled; });
		m_signaled = false;
	}

	void ThreadEvent::SetEvent()
	{
		std::unique_lock<std::mutex> lck(m_mtx);
		m_signaled = true;
		m_cv.notify_one();
	}
#endif


}
