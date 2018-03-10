/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "Ps.h"
#include "PsFoundation.h"
#include "PsAtomic.h"
#include "PsThread.h"
#include "foundation/PxAssert.h"
#if !defined(PX_APPLE) && !defined(ANDROID) && !defined(__CYGWIN__) && !defined(PX_PS4)
#	include <bits/local_lim.h> // PTHREAD_STACK_MIN
#endif
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#ifndef PX_PS4
#include <sys/syscall.h>
#if !defined(PX_APPLE)
#include <asm/unistd.h>
#include <sys/resource.h>
#endif
#endif

#if defined(PX_APPLE)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

// fwd
#if defined(ANDROID)
extern "C" 
{
int android_getCpuCount(void);
}
#endif


#define PxSpinLockPause() asm ("nop")

namespace physx
{
namespace shdfnd
{

namespace 
{

	typedef enum
	{
		_PxThreadNotStarted,
		_PxThreadStarted,
		_PxThreadStopped
	} PxThreadState;

	class _ThreadImpl
	{
	public:
		ThreadImpl::ExecuteFn	fn;
		void*					arg;
		volatile PxI32			quitNow;
		volatile PxI32			threadStarted; 
		volatile PxI32			state;

		pthread_t				thread;
		pid_t					tid;

		PxU32					affinityMask;
	};

	_ThreadImpl* getThread(ThreadImpl* impl)
	{
		return reinterpret_cast<_ThreadImpl*>(impl);
	}
	
	static void setTid(_ThreadImpl& threadImpl)
	{
		// query TID
#if defined(PX_PS4)
	//AM: TODO: neither of the below are implemented
#elif defined(PX_APPLE)
		threadImpl.tid = syscall(SYS_gettid);
#else
		threadImpl.tid = syscall(__NR_gettid);
#endif

		// notify/unblock parent thread
		atomicCompareExchange(&(threadImpl.threadStarted), 1, 0);	
	}
	
	void *PxThreadStart(void *arg)
	{
		_ThreadImpl* impl = getThread((ThreadImpl*)arg);
		impl->state = _PxThreadStarted;
		
		// run setTid in thread's context
		setTid(*impl);

		// then run either the passed in function or execute from the derived class (Runnable).
		if(impl->fn)
			(*impl->fn)(impl->arg);
		else if(impl->arg)
			((Runnable*)impl->arg)->execute();
		return 0;
	}
}

static const PxU32 gSize = sizeof(_ThreadImpl);
const PxU32& ThreadImpl::getSize()  { return gSize; }


ThreadImpl::Id ThreadImpl::getId()
{
	return Id(pthread_self());
}

ThreadImpl::ThreadImpl()
{
	getThread(this)->thread = 0;
	getThread(this)->tid = 0;
	getThread(this)->state = _PxThreadNotStarted;
	getThread(this)->quitNow = 0;
	getThread(this)->threadStarted = 0;
	getThread(this)->fn = NULL;
	getThread(this)->arg = NULL;
	getThread(this)->affinityMask = 0;
}

ThreadImpl::ThreadImpl(ThreadImpl::ExecuteFn fn, void *arg)
{
	getThread(this)->thread = 0;
	getThread(this)->tid = 0;
	getThread(this)->state = _PxThreadNotStarted;
	getThread(this)->quitNow = 0;
	getThread(this)->threadStarted = 0;
	getThread(this)->fn = fn;
	getThread(this)->arg = arg;
	getThread(this)->affinityMask = 0;

	start(0, NULL);
}

ThreadImpl::~ThreadImpl()
{
	if(getThread(this)->state == _PxThreadStarted)
		kill();
}


void ThreadImpl::start(PxU32 stackSize, Runnable* runnable)
{
	if(getThread(this)->state != _PxThreadNotStarted)
		return;

	if (stackSize == 0)
		stackSize = getDefaultStackSize();

#if defined(PTHREAD_STACK_MIN) && !defined(ANDROID)
	if(stackSize < PTHREAD_STACK_MIN)
	{
		shdfnd::getFoundation().error(PX_WARN, "ThreadImpl::start(): stack size was set below PTHREAD_STACK_MIN");
		stackSize = PTHREAD_STACK_MIN;
	}
#endif

	if(runnable && !getThread(this)->arg && ! getThread(this)->fn)
		getThread(this)->arg = runnable;

	pthread_attr_t attr;
	int status = pthread_attr_init(&attr);
	PX_ASSERT(!status);
	PX_UNUSED(status);

	status = pthread_attr_setstacksize(&attr, stackSize);
	PX_ASSERT(!status);
	status = pthread_create(&getThread(this)->thread, &attr, PxThreadStart, this);
	PX_ASSERT(!status);

	// wait for thread to startup and write out TID
	// otherwise TID dependent calls like setAffinity will fail.
	while(atomicCompareExchange(&(getThread(this)->threadStarted), 1, 1) == 0)
		yield();
		
	// here we are sure that getThread(this)->state >= _PxThreadStarted

	status = pthread_attr_destroy(&attr);
	PX_ASSERT(!status);
	
	// apply stored affinity mask
	if (getThread(this)->affinityMask)
		setAffinityMask(getThread(this)->affinityMask);
}


void ThreadImpl::signalQuit()
{
	atomicIncrement(&(getThread(this)->quitNow));
}

bool ThreadImpl::waitForQuit()
{
	if(getThread(this)->state == _PxThreadNotStarted)
		return false;

	// works also with a stopped/exited thread if the handle is still valid
	pthread_join(getThread(this)->thread, NULL);
	return true;
}


bool ThreadImpl::quitIsSignalled()
{
	return atomicCompareExchange(&(getThread(this)->quitNow), 0, 0) != 0;
}

void ThreadImpl::quit()
{
	getThread(this)->state = _PxThreadStopped;
	pthread_exit(0);
}

void ThreadImpl::kill()
{
#ifndef ANDROID
	if(getThread(this)->state == _PxThreadStarted)
		pthread_cancel(getThread(this)->thread);
	getThread(this)->state = _PxThreadStopped;
#else
	shdfnd::getFoundation().error(PX_WARN, "ThreadImpl::kill() called, but is not implemented");
#endif
}

void ThreadImpl::sleep(PxU32 ms)
{
	timespec sleepTime;
	PxU32 remainder = ms % 1000;
	sleepTime.tv_sec = ms - remainder;
	sleepTime.tv_nsec = remainder * 1000000L;

	while(nanosleep(&sleepTime, &sleepTime) == -1)
		continue;
}

void ThreadImpl::yield()
{
	sched_yield();
}

#ifdef PX_PS4
PxU32 setAffinityMaskPS4(pthread_t, PxU32);
#endif

PxU32 ThreadImpl::setAffinityMask(PxU32 mask)
{
	// Same as windows impl if mask is zero
	if(!mask)
		return 0;

	getThread(this)->affinityMask = mask;

	PxU64 prevMask = 0;

	if (getThread(this)->state == _PxThreadStarted)
	{
#ifdef PX_PS4
		prevMask = setAffinityMaskPS4(getThread(this)->thread, mask);
#elif !defined(PX_APPLE) // Apple doesn't support syscall with getaffinity and setaffinity
		PxI32 errGet = syscall(__NR_sched_getaffinity, getThread(this)->tid, sizeof(prevMask), &prevMask);
		if(errGet < 0)
			return 0;

		PxI32 errSet = syscall(__NR_sched_setaffinity, getThread(this)->tid, sizeof(mask), &mask);
		if(errSet != 0)
			return 0;
#endif

	}

	return PxU32(prevMask);
}

void ThreadImpl::setName(const char *name)
{
#if (defined(ANDROID) && (__ANDROID_API__ > 8))
	pthread_setname_np(getThread(this)->thread, name);
#else
	// not implemented because most unix APIs expect setName() 
	// to be called from the thread's context. Example see next comment:

	// this works only with the current thread and can rename 
	// the main process if used in the wrong context:
	// prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name) ,0,0,0);
#endif
}

#if !defined(PX_APPLE)
static ThreadPriority::Enum convertPriorityFromLinux(PxU32 inPrio, int policy)
{
	PX_COMPILE_TIME_ASSERT(ThreadPriority::eLOW > ThreadPriority::eHIGH);
	PX_COMPILE_TIME_ASSERT(ThreadPriority::eHIGH == 0);

	int maxL = sched_get_priority_max(policy);
	int minL = sched_get_priority_min(policy);
	int rangeL = maxL - minL;
	int rangePx = ThreadPriority::eLOW - ThreadPriority::eHIGH;

	// case for default scheduler policy
	if(rangeL == 0)
		return ThreadPriority::eNORMAL;

	float floatPrio = (float(maxL - inPrio)*float(rangePx))/float(rangeL);

	return ThreadPriority::Enum(int(roundf(floatPrio)));
}

static int convertPriorityToLinux(ThreadPriority::Enum inPrio, int policy)
{
	int maxL = sched_get_priority_max(policy);
	int minL = sched_get_priority_min(policy);
	int rangeL = maxL - minL;
	int rangePx = ThreadPriority::eLOW - ThreadPriority::eHIGH; 

	// case for default scheduler policy
	if(rangeL == 0)
		return 0;

	float floatPrio = (float(ThreadPriority::eLOW - inPrio)*float(rangeL))/float(rangePx);

	return  minL + int(roundf(floatPrio));
}
#endif 

void ThreadImpl::setPriority(ThreadPriority::Enum val)
{
#if !defined(PX_APPLE)
	int policy;
	sched_param s_param;
	pthread_getschedparam(getThread(this)->thread, &policy, &s_param);
	s_param.sched_priority = convertPriorityToLinux(val, policy);
	pthread_setschedparam(getThread(this)->thread, policy, &s_param);
#endif
}

ThreadPriority::Enum ThreadImpl::getPriority(Id pthread)
{
#if !defined(PX_APPLE)
	int policy;
	sched_param s_param;
	int ret = pthread_getschedparam(pthread_t(pthread), &policy, &s_param);
	if(ret == 0)
		return convertPriorityFromLinux(s_param.sched_priority, policy);
	else
		return ThreadPriority::eNORMAL;
#else
	return ThreadPriority::eNORMAL;
#endif
}

PxU32 ThreadImpl::getNbPhysicalCores()
{
#if defined(PX_APPLE)
	int count;
	size_t size=sizeof(count);
	return sysctlbyname("hw.physicalcpu",&count,&size,NULL,0) ? 0 : count;
#elif defined(ANDROID)
	return android_getCpuCount();
#else
	// Linux exposes CPU topology using /sys/devices/system/cpu
	// https://www.kernel.org/doc/Documentation/cputopology.txt
	if (FILE* f = fopen("/sys/devices/system/cpu/possible", "r"))
	{
		int minIndex, maxIndex;
		int n = fscanf(f, "%d-%d", &minIndex, &maxIndex);
		fclose(f);

		if (n == 2)
			return (maxIndex-minIndex)+1;
		else if (n == 1)
			return minIndex+1;
	}

#ifdef PX_PS4
	//Reducing to 6 to take into account that the OS appears to use 2 cores at peak currently.
	return 6;
#else
	// For non-Linux kernels this fallback is possibly the best we can do
	// but will report logical (hyper-threaded) counts
	int n = sysconf(_SC_NPROCESSORS_CONF);
	if (n < 0)
		return 0;
	else
		return n;
#endif
#endif
}

PxU32 TlsAlloc()
{
	pthread_key_t key;
	int status = pthread_key_create(&key, NULL);
	PX_ASSERT(!status);
	PX_UNUSED(status);
	return (PxU32)key;
}

void TlsFree(PxU32 index)
{
	int status = pthread_key_delete((pthread_key_t)index);
	PX_ASSERT(!status);
	PX_UNUSED(status);
}

void* TlsGet(PxU32 index)
{
	return (void*)pthread_getspecific((pthread_key_t)index);
}

PxU32 TlsSet(PxU32 index, void *value)
{
	int status = pthread_setspecific((pthread_key_t)index, value);
	PX_ASSERT(!status);
	return !status;
}

// DM: On Linux x86-32, without implementation-specific restrictions
// the default stack size for a new thread should be 2 megabytes (kernel.org).
// NOTE: take care of this value on other architecutres!
PxU32 ThreadImpl::getDefaultStackSize() 
{
	return 1 << 21;
}

} // namespace shdfnd
} // namespace physx

