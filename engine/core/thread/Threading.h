#pragma once

#include "engine/core/base/echo_def.h"

#ifdef ECHO_PLATFORM_WINDOWS
	#undef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#define _CRT_SECURE_NO_WARNINGS
	#include <windows.h>
	#include <process.h>
#else
	#include <pthread.h>
#endif 

#ifndef ECHO_PLATFORM_HTML5
	#include <thread>
	#include <mutex>
	#include <condition_variable>
#endif

namespace Echo
{
	typedef void(*ThreadFunc)(void*);

	// sleepº¯Êý
	void ThreadSleepByMilliSecond(float millSecond);

#ifdef ECHO_PLATFORM_WINDOWS
	//////////////////////////////////////////////////////////////////////////
	class Mutex
	{
	public:
		Mutex()			{ InitializeCriticalSection(&mCriticalSection);	}
		~Mutex()		{ DeleteCriticalSection(&mCriticalSection);		}
		void Lock()		{ EnterCriticalSection(&mCriticalSection);		}
		void Unlock()	{ LeaveCriticalSection(&mCriticalSection);		}

	private:
		CRITICAL_SECTION mCriticalSection;
	};
#elif defined(ECHO_PLATFORM_HTML5)
	class Mutex
	{
	public:
		Mutex()			{}
		~Mutex()		{}
		void Lock()		{}
		void Unlock()	{}
	};
#else
	//////////////////////////////////////////////////////////////////////////
	class Mutex
	{
	public:
		Mutex()
		{
			pthread_mutexattr_t MutexAttributes;
			pthread_mutexattr_init(&MutexAttributes);
			pthread_mutexattr_settype(&MutexAttributes, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&mutex, &MutexAttributes);
		}
		~Mutex(void) { pthread_mutex_destroy(&mutex);}
		void Lock() { pthread_mutex_lock(&mutex);}
		void Unlock() { pthread_mutex_unlock(&mutex);}
	private:
		pthread_mutex_t mutex;
	};

#endif

	class MutexLock
	{
	public:
		MutexLock(Mutex& mutex) : mMutex(mutex) { mMutex.Lock();	}
		~MutexLock()							{ mMutex.Unlock();	}

	private:
		Mutex& mMutex;
	};


	class ThreadEvent
	{
	public:
		ThreadEvent();
		virtual ~ThreadEvent();

		void WaitEvent();
		void SetEvent();

	private:
#ifndef ECHO_PLATFORM_HTML5
		std::mutex m_mtx;
		std::condition_variable m_cv;
		bool m_signaled;
#endif
	};

	class Thread
	{
	public:
		void Start(ThreadFunc func, void* param);
		void Join();
		void detach();

	private:
#ifndef ECHO_PLATFORM_HTML5
		std::thread m_thread;
#endif
	};

#define AUTO_MUTEX_NAME auto_mutex

#define EE_AUTO_MUTEX mutable Mutex AUTO_MUTEX_NAME;
#define EE_LOCK_AUTO_MUTEX MutexLock NameLock(AUTO_MUTEX_NAME);
#define EE_LOCK_MUTEX_NAMED(mutexName, lockName) MutexLock lockName(mutexName);
}

#define EE_MUTEX(name)	mutable Echo::Mutex name;
#define EE_STATIC_MUTEX(name)	static Echo::Mutex name;
#define EE_LOCK_MUTEX(mutexName) Echo::MutexLock _lockName(mutexName);
