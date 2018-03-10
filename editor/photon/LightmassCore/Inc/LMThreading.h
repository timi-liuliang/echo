/*=============================================================================
	LMThreading.h: Threading/process functionality
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once


namespace Lightmass
{



#ifndef INFINITE
#define INFINITE ((DWORD)-1)
#endif


/** Sleep this thread for Seconds.  0.0 means release the current timeslice to let other threads get some attention. */
void appSleep( FLOAT Seconds );

/**
 * Enforces strict memory load/store ordering across the memory barrier call.
 */
FORCEINLINE void appMemoryBarrier()
{
#if defined _M_IX86 || _WIN64
	// Do nothing on x86/x64; the spec requires load/store ordering even in the absence of a memory barrier.
#else
	#error Unknown platform for appMemoryBarrier implementation.
#endif
}

/**
 * Interlocked style functions for threadsafe atomic operations
 */

/**
 * Atomically increments the value pointed to and returns that to the caller
 */
FORCEINLINE INT appInterlockedIncrement(volatile INT* Value)
{
	return (INT)InterlockedIncrement((LPLONG)Value);
}
FORCEINLINE SSIZE_T appInterlockedIncrement(volatile SSIZE_T* Value)
{
#if defined(_WIN64)
	return InterlockedIncrement64(Value);
#else
	return InterlockedIncrement(Value);
#endif
}

/**
 * Atomically decrements the value pointed to and returns that to the caller
 */
FORCEINLINE INT appInterlockedDecrement(volatile INT* Value)
{
	return (INT)InterlockedDecrement((LPLONG)Value);
}
FORCEINLINE SSIZE_T appInterlockedDecrement(volatile SSIZE_T* Value)
{
#if defined(_WIN64)
	return InterlockedDecrement64(Value);
#else
	return InterlockedDecrement(Value);
#endif
}

/**
 * Atomically adds the amount to the value pointed to and returns the old
 * value to the caller
 */
FORCEINLINE INT appInterlockedAdd(volatile INT* Value,INT Amount)
{
	return (INT)InterlockedExchangeAdd((LPLONG)Value,(LONG)Amount);
}
FORCEINLINE SSIZE_T appInterlockedAdd(volatile SSIZE_T* Value, SSIZE_T Amount)
{
#if defined(_WIN64)
	return InterlockedExchangeAdd64(Value, Amount);
#else
	return InterlockedExchangeAdd(Value, Amount);
#endif
}

/**
 * Atomically swaps two values returning the original value to the caller
 */
FORCEINLINE INT appInterlockedExchange(volatile INT* Value,INT Exchange)
{
	return (INT)InterlockedExchange((LPLONG)Value,(LONG)Exchange);
}
FORCEINLINE SSIZE_T appInterlockedExchange(volatile SSIZE_T* Value, SSIZE_T Exchange)
{
#if defined(_WIN64)
	return InterlockedExchange64(Value, Exchange);
#else
	return InterlockedExchange(Value, Exchange);
#endif
}

/**
 * Atomically compares the value to comperand and replaces with the exchange
 * value if they are equal and returns the original value
 */
FORCEINLINE INT appInterlockedCompareExchange(INT* Dest,INT Exchange,INT Comperand)
{
	return (INT)InterlockedCompareExchange((LPLONG)Dest,(LONG)Exchange,(LONG)Comperand);
}

/**
 * Atomically compares the pointer to comperand and replaces with the exchange
 * pointer if they are equal and returns the original value
 */
FORCEINLINE void* appInterlockedCompareExchangePointer(void** Dest,void* Exchange,void* Comperand)
{
	return InterlockedCompareExchangePointer(Dest,Exchange,Comperand);
}

/**
 * Returns a pseudo-handle to the currently executing thread.
 */
FORCEINLINE HANDLE appGetCurrentThread(void)
{
	return GetCurrentThread();
}

/**
 * Returns the currently executing thread's id
 */
FORCEINLINE DWORD appGetCurrentThreadId(void)
{
	return GetCurrentThreadId();
}

/**
 * Sets the preferred processor for a thread.
 *
 * @param	ThreadHandle		handle for the thread to set affinity for
 * @param	PreferredProcessor	zero-based index of the processor that this thread prefers
 *
 * @return	the number of the processor previously preferred by the thread, MAXIMUM_PROCESSORS
 *			if the thread didn't have a preferred processor, or (DWORD)-1 if the call failed.
 */
FORCEINLINE DWORD appSetThreadAffinity( HANDLE ThreadHandle, DWORD PreferredProcessor )
{
	return SetThreadIdealProcessor(ThreadHandle,PreferredProcessor);
}

/**
 * Allocates a thread local store slot
 */
FORCEINLINE DWORD appAllocTlsSlot(void)
{
	return TlsAlloc();
}

/**
 * Sets a value in the specified TLS slot
 *
 * @param SlotIndex the TLS index to store it in
 * @param Value the value to store in the slot
 */
FORCEINLINE void appSetTlsValue(DWORD SlotIndex,void* Value)
{
	TlsSetValue(SlotIndex,Value);
}

/**
 * Reads the value stored at the specified TLS slot
 *
 * @return the value stored in the slot
 */
FORCEINLINE void* appGetTlsValue(DWORD SlotIndex)
{
	return TlsGetValue(SlotIndex);
}

/**
 * Frees a previously allocated TLS slot
 *
 * @param SlotIndex the TLS index to store it in
 */
FORCEINLINE void appFreeTlsSlot(DWORD SlotIndex)
{
	TlsFree(SlotIndex);
}





// Forward declaration for a platform specific implementation
class FCriticalSection;
/** Simple base class for polymorphic cleanup */
struct FSynchronize
{
	/** Simple destructor */
	virtual ~FSynchronize(void)
	{
	}
};

/**
 * This class is the abstract representation of a waitable event. It is used
 * to wait for another thread to signal that it is ready for the waiting thread
 * to do some work. Very useful for telling groups of threads to exit.
 */
class FEvent : public FSynchronize
{
public:
	/**
	 * Creates the event. Manually reset events stay triggered until reset.
	 * Named events share the same underlying event.
	 *
	 * @param bIsManualReset Whether the event requires manual reseting or not
	 * @param InName Whether to use a commonly shared event or not. If so this
	 * is the name of the event to share.
	 *
	 * @return Returns TRUE if the event was created, FALSE otherwise
	 */
	virtual UBOOL Create(UBOOL bIsManualReset = FALSE,const TCHAR* InName = NULL) = 0;

	/**
	 * Triggers the event so any waiting threads are activated
	 */
	virtual void Trigger(void) = 0;

	/**
	 * Resets the event to an untriggered (waitable) state
	 */
	virtual void Reset(void) = 0;

	/**
	 * Triggers the event and resets the triggered state (like auto reset)
	 */
	virtual void Pulse(void) = 0;

	/**
	 * Waits for the event to be triggered
	 *
	 * @param WaitTime Time in milliseconds to wait before abandoning the event
	 * (DWORD)-1 is treated as wait infinite
	 *
	 * @return TRUE if the event was signaled, FALSE if the wait timed out
	 */
	virtual UBOOL Wait(DWORD WaitTime = INFINITE) = 0;
};

/**
 * This is the factory interface for creating various synchronization objects.
 * It is overloaded on a per platform basis to hide how each platform creates
 * the various synchronization objects. NOTE: The malloc used by it must be 
 * thread safe
 */
class FSynchronizeFactory
{
public:
	/**
	 * Creates a new critical section
	 *
	 * @return The new critical section object or NULL otherwise
	 */
	virtual FCriticalSection* CreateCriticalSection(void) = 0;

	/**
	 * Creates a new event
	 *
	 * @param bIsManualReset Whether the event requires manual reseting or not
	 * @param InName Whether to use a commonly shared event or not. If so this
	 * is the name of the event to share.
	 *
	 * @return Returns the new event object if successful, NULL otherwise
	 */
	virtual FEvent* CreateSynchEvent(UBOOL bIsManualReset = FALSE,const TCHAR* InName = NULL) = 0;

	/**
	 * Cleans up the specified synchronization object using the correct heap
	 *
	 * @param InSynchObj The synchronization object to destroy
	 */
	virtual void Destroy(FSynchronize* InSynchObj) = 0;
};

/*
 *  Global factory for creating synchronization objects
 */
extern FSynchronizeFactory* GSynchronizeFactory;

/**
 * This is the base interface for "runnable" object. A runnable object is an
 * object that is "run" on an arbitrary thread. The call usage pattern is
 * Init(), Run(), Exit(). The thread that is going to "run" this object always
 * uses those calling semantics. It does this on the thread that is created so
 * that any thread specific uses (TLS, etc.) are available in the contexts of
 * those calls. A "runnable" does all initialization in Init(). If
 * initialization fails, the thread stops execution and returns an error code.
 * If it succeeds, Run() is called where the real threaded work is done. Upon
 * completion, Exit() is called to allow correct clean up.
 */
class FRunnable
{
public:
	/**
	 * Allows per runnable object initialization. NOTE: This is called in the
	 * context of the thread object that aggregates this, not the thread that
	 * passes this runnable to a new thread.
	 *
	 * @return True if initialization was successful, false otherwise
	 */
	virtual UBOOL Init(void) = 0;

	/**
	 * This is where all per object thread work is done. This is only called
	 * if the initialization was successful.
	 *
	 * @return The exit code of the runnable object
	 */
	virtual DWORD Run(void) = 0;

	/**
	 * This is called if a thread is requested to terminate early
	 */
	virtual void Stop(void) = 0;

	/**
	 * Called in the context of the aggregating thread to perform any cleanup.
	 */
	virtual void Exit(void) = 0;

	/**
	 * Destructor
	 */
	virtual ~FRunnable(){}
};

/**
 * The list of enumerated thread priorities we support
 */
enum EThreadPriority
{
	TPri_Normal,
	TPri_AboveNormal,
	TPri_BelowNormal
};

/**
 * This is the base interface for all runnable thread classes. It specifies the
 * methods used in managing its life cycle.
 */
class FRunnableThread
{
public:
	/**
	 * Changes the thread priority of the currently running thread
	 *
	 * @param NewPriority The thread priority to change to
	 */
	virtual void SetThreadPriority(EThreadPriority NewPriority) = 0;

	/**
	 * Tells the OS the preferred CPU to run the thread on. NOTE: Don't use
	 * this function unless you are absolutely sure of what you are doing
	 * as it can cause the application to run poorly by preventing the
	 * scheduler from doing its job well.
	 *
	 * @param ProcessorNum The preferred processor for executing the thread on
	 */
	virtual void SetProcessorAffinity(DWORD ProcessorNum) = 0;

	/**
	 * Tells the thread to either pause execution or resume depending on the
	 * passed in value.
	 *
	 * @param bShouldPause Whether to pause the thread (true) or resume (false)
	 */
	virtual void Suspend(UBOOL bShouldPause = 1) = 0;

	/**
	 * Tells the thread to exit. If the caller needs to know when the thread
	 * has exited, it should use the bShouldWait value and tell it how long
	 * to wait before deciding that it is deadlocked and needs to be destroyed.
	 * The implementation is responsible for calling Stop() on the runnable.
	 * NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
	 *
	 * @param bShouldWait If true, the call will wait for the thread to exit
	 *
	 * @return True if the thread exited graceful, false otherwise
	 */
	virtual UBOOL Kill(UBOOL bShouldWait = FALSE) = 0;

	/**
	 * Halts the caller until this thread is has completed its work.
	 *
	 * @param WaitTime Time in milliseconds to wait before abandoning the event
	 * (DWORD)-1 is treated as wait infinite
	 *
	 * @return TRUE if the thread completed, FALSE if the wait timed out
	 */
	virtual UBOOL WaitForCompletion( DWORD WaitTime = -1 ) = 0;

	/**
	 * Thread ID for this thread 
	 *
	 * @return ID that was set by CreateThread
	 */
	virtual DWORD GetThreadID(void) = 0;
};

/**
 * This is the factory interface for creating threads. Each platform must
 * implement this with all the correct platform semantics
 */
class FThreadFactory
{
public:
	/**
	 * Creates the thread with the specified stack size and thread priority.
	 *
	 * @param InRunnable The runnable object to execute
	 * @param ThreadName Name of the thread
	 * @param bAutoDeleteSelf Whether to delete this object on exit
	 * @param bAutoDeleteRunnable Whether to delete the runnable object on exit
	 * @param InStackSize The size of the stack to create. 0 means use the
	 * current thread's stack size
	 * @param InThreadPri Tells the thread whether it needs to adjust its
	 * priority or not. Defaults to normal priority
	 *
	 * @return The newly created thread or NULL if it failed
	 */
	virtual FRunnableThread* CreateThread(FRunnable* InRunnable, const ANSICHAR* ThreadName,
		UBOOL bAutoDeleteSelf = 0,UBOOL bAutoDeleteRunnable = 0,
		DWORD InStackSize = 0,EThreadPriority InThreadPri = TPri_Normal) = 0;

	/**
	 * Cleans up the specified thread object using the correct heap
	 *
	 * @param InThread The thread object to destroy
	 */
	virtual void Destroy(FRunnableThread* InThread) = 0;
};

/*
 *  Global factory for creating threads
 */
extern FThreadFactory* GThreadFactory;

/**
 * Some standard IDs for special queued work types
 * These are just setup for future use, these aren't supported yet
 */
enum EWorkID
{
	WORK_None,
	WORK_DMARequest,
	WORK_Skin,
	WORK_ShadowExtrusion,
	WORK_Decompress,
	WORK_Stream,
	WORK_GenerateParticles,

	WORK_MAX
};
/**
 * This interface is a type of runnable object that requires no per thread
 * initialization. It is meant to be used with pools of threads in an
 * abstract way that prevents the pool from needing to know any details
 * about the object being run. This allows queueing of disparate tasks and
 * servicing those tasks with a generic thread pool.
 */
class FQueuedWork
{
public:
	/**
	 * Virtual destructor so that child implementations are guaranteed a chance
	 * to clean up any resources they allocated.
	 */
	virtual ~FQueuedWork(void) {}

	/**
	 * This is where the real thread work is done. All work that is done for
	 * this queued object should be done from within the call to this function.
	 */
	virtual void DoWork(void) = 0;

	/**
	 * Tells the queued work that it is being abandoned so that it can do
	 * per object clean up as needed. This will only be called if it is being
	 * abandoned before completion. NOTE: This requires the object to delete
	 * itself using whatever heap it was allocated in.
	 */
	virtual void Abandon(void) = 0;

	/**
	 * This method is also used to tell the object to cleanup but not before
	 * the object has finished it's work.
	 */ 
	virtual void Dispose(void) = 0;
};

/**
 * This is the interface used for all poolable threads. The usage pattern for
 * a poolable thread is different from a regular thread and this interface
 * reflects that. Queued threads spend most of their life cycle idle, waiting
 * for work to do. When signaled they perform a job and then return themselves
 * to their owning pool via a callback and go back to an idle state.
 */
class FQueuedThread
{
public:
	/**
	 * Virtual destructor so that child implementations are guaranteed a chance
	 * to clean up any resources they allocated.
	 */
	virtual ~FQueuedThread(void) {}

	/**
	 * Creates the thread with the specified stack size and creates the various
	 * events to be able to communicate with it.
	 *
	 * @param InPool The thread pool interface used to place this thread
	 * back into the pool of available threads when its work is done
	 * @param ProcessorMask The processor set to run the thread on
	 * @param InStackSize The size of the stack to create. 0 means use the
	 * current thread's stack size
	 *
	 * @return True if the thread and all of its initialization was successful, false otherwise
	 */
	virtual UBOOL Create(class FQueuedThreadPool* InPool,DWORD ProcessorMask,
		DWORD InStackSize = 0) = 0;
	
	/**
	 * Tells the thread to exit. If the caller needs to know when the thread
	 * has exited, it should use the bShouldWait value and tell it how long
	 * to wait before deciding that it is deadlocked and needs to be destroyed.
	 * NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
	 *
	 * @param bShouldWait If true, the call will wait for the thread to exit
	 * @param bShouldDeleteSelf Whether to delete ourselves upon completion
	 *
	 * @return True if the thread exited graceful, false otherwise
	 */
	virtual UBOOL Kill(UBOOL bShouldWait = FALSE, UBOOL bShouldDeleteSelf = FALSE) = 0;

	/**
	 * Tells the thread there is work to be done. Upon completion, the thread
	 * is responsible for adding itself back into the available pool.
	 *
	 * @param InQueuedWork The queued work to perform
	 */
	virtual void DoWork(FQueuedWork* InQueuedWork) = 0;
};

/**
 * This interface is used by all queued thread pools. It used as a callback by
 * FQueuedThreads and is used to queue asynchronous work for callers.
 */
class FQueuedThreadPool
{
public:
	/**
	 * Virtual destructor so that child implementations are guaranteed a chance
	 * to clean up any resources they allocated.
	 */
	virtual ~FQueuedThreadPool(void) {}

	/**
	 * Creates the thread pool with the specified number of threads
	 *
	 * @param InNumQueuedThreads Specifies the number of threads to use in the pool
	 * @param ProcessorMask Specifies which processors should be used by the pool
	 * @param StackSize The size of stack the threads in the pool need (32K default)
	 *
	 * @return Whether the pool creation was successful or not
	 */
	virtual UBOOL Create(DWORD InNumQueuedThreads,DWORD ProcessorMask = 0,
		DWORD StackSize = (32 * 1024)) = 0;

	/**
	 * Tells the pool to clean up all background threads
	 */
	virtual void Destroy(void) = 0;

	/**
	 * Checks to see if there is a thread available to perform the task. If not,
	 * it queues the work for later. Otherwise it is immediately dispatched.
	 *
	 * @param InQueuedWork The work that needs to be done asynchronously
	 */
	virtual void AddQueuedWork(FQueuedWork* InQueuedWork) = 0;

	/**
	 * Places a thread back into the available pool
	 *
	 * @param InQueuedThread The thread that is ready to be pooled
	 */
	virtual void ReturnToPool(FQueuedThread* InQueuedThread) = 0;
};

/*
 *  Global thread pool for shared async operations
 */
extern FQueuedThreadPool* GThreadPool;

/**
 * A base implementation of a queued thread pool. It provides the common
 * methods & members needed to implement a pool.
 */
class FQueuedThreadPoolBase : public FQueuedThreadPool
{
protected:
	/**
	 * The work queue to pull from
	 */
	TArray<FQueuedWork*> QueuedWork;
	
	/**
	 * The thread pool to dole work out to
	 */
	TArray<FQueuedThread*> QueuedThreads;

	/**
	 * The synchronization object used to protect access to the queued work
	 */
	FCriticalSection* SynchQueue;

	/**
	 * Constructor that creates the zeroes the critical sections
	 */
	FQueuedThreadPoolBase(void)
	{
		SynchQueue = NULL;
	}

public:
	/**
	 * Clean up the synch objects
	 */
	virtual ~FQueuedThreadPoolBase(void);

	/**
	 * Creates the synchronization object for locking the queues
	 *
	 * @return Whether the pool creation was successful or not
	 */
	UBOOL CreateSynchObjects(void);

	/**
	 * Tells the pool to clean up all background threads
	 */
	virtual void Destroy(void);

	/**
	 * Checks to see if there is a thread available to perform the task. If not,
	 * it queues the work for later. Otherwise it is immediately dispatched.
	 *
	 * @param InQueuedWork The work that needs to be done asynchronously
	 */
	void AddQueuedWork(FQueuedWork* InQueuedWork);

	/**
	 * Places a thread back into the available pool if there is no pending work
	 * to be done. If there is, the thread is put right back to work without it
	 * reaching the queue.
	 *
	 * @param InQueuedThread The thread that is ready to be pooled
	 */
	void ReturnToPool(FQueuedThread* InQueuedThread);
};










/**
 * This is the Windows version of a critical section. It uses an aggregate
 * CRITICAL_SECTION to implement its locking.
 */
class FCriticalSection :
	public FSynchronize
{
	/**
	 * The windows specific critical section
	 */
	CRITICAL_SECTION CriticalSection;

public:
	/**
	 * Constructor that initializes the aggregated critical section
	 */
	FORCEINLINE FCriticalSection(void)
	{
		InitializeCriticalSection(&CriticalSection);
		SetCriticalSectionSpinCount(&CriticalSection,4000);
	}

	/**
	 * Destructor cleaning up the critical section
	 */
	FORCEINLINE ~FCriticalSection(void)
	{
		DeleteCriticalSection(&CriticalSection);
	}

	/**
	 * Locks the critical section
	 */
	FORCEINLINE void Lock(void)
	{
		// Spin first before entering critical section, causing ring-0 transition and context switch.
		if( TryEnterCriticalSection(&CriticalSection) == 0 )
		{
			EnterCriticalSection(&CriticalSection);
		}
	}

	/**
	 * Releases the lock on the critical seciton
	 */
	FORCEINLINE void Unlock(void)
	{
		LeaveCriticalSection(&CriticalSection);
	}
};

/**
 * This is the Windows version of an event
 */
class FEventWin : public FEvent
{
	/**
	 * The handle to the event
	 */
	HANDLE Event;

public:
	/**
	 * Constructor that zeroes the handle
	 */
	FEventWin(void);

	/**
	 * Cleans up the event handle if valid
	 */
	virtual ~FEventWin(void);

	/**
	 * Waits for the event to be signaled before returning
	 */
	virtual void Lock(void);

	/**
	 * Triggers the event so any waiting threads are allowed access
	 */
	virtual void Unlock(void);

	/**
	 * Creates the event. Manually reset events stay triggered until reset.
	 * Named events share the same underlying event.
	 *
	 * @param bIsManualReset Whether the event requires manual reseting or not
	 * @param InName Whether to use a commonly shared event or not. If so this
	 * is the name of the event to share.
	 *
	 * @return Returns TRUE if the event was created, FALSE otherwise
	 */
	virtual UBOOL Create(UBOOL bIsManualReset = FALSE,const TCHAR* InName = NULL);

	/**
	 * Triggers the event so any waiting threads are activated
	 */
	virtual void Trigger(void);

	/**
	 * Resets the event to an untriggered (waitable) state
	 */
	virtual void Reset(void);

	/**
	 * Triggers the event and resets the triggered state NOTE: This behaves
	 * differently for auto-reset versus manual reset events. All threads
	 * are released for manual reset events and only one is for auto reset
	 */
	virtual void Pulse(void);

	/**
	 * Waits for the event to be triggered
	 *
	 * @param WaitTime Time in milliseconds to wait before abandoning the event
	 * (DWORD)-1 is treated as wait infinite
	 *
	 * @return TRUE if the event was signaled, FALSE if the wait timed out
	 */
	virtual UBOOL Wait(DWORD WaitTime = (DWORD)-1);
};

/**
 * This is the Windows version of a semaphore.
 * It works like a thread-safe counter which is considered 'signaled' when non-zero.
 * Trigger() will increment the counter and a successful Wait() will decrement it.
 */
class FSemaphoreWin
{
public:
	/**
	 * Constructor that zeroes the handle
	 */
	FSemaphoreWin(void);

	/**
	 * Cleans up the handle if valid
	 */
	virtual ~FSemaphoreWin(void);

	/**
	 * Creates the semaphore.
	 * Named events share the same underlying event.
	 *
	 * @apram MaxCount Maximum value of the semaphore. The semaphore will always be [0..MaxCount].
	 * @param InName Whether to use a commonly shared semaphore or not. If so this
	 * is the name of the event to share.
	 *
	 * @return Returns TRUE if the event was created, FALSE otherwise
	 */
	virtual UBOOL Create(INT MaxCount,const TCHAR* InName = NULL);

	/**
	 * Increments the semaphore by 1.
	 * If multiple threads are waiting, one of them will be released.
	 */
	virtual void Trigger(void);

	/**
	 * Waits for the semaphore to be signaled (non-zero value).
	 * Upon return, decrement the semaphore by 1 (unless the timeout was reached).
	 *
	 * @param WaitTime Time in milliseconds to wait before abandoning the event
	 * (DWORD)-1 is treated as wait infinite
	 *
	 * @return TRUE if the semaphore was signaled, FALSE if the wait timed out
	 */
	virtual UBOOL Wait(DWORD WaitTime = (DWORD)-1);

protected:
	/**
	 * The handle to the semaphore
	 */
	HANDLE Semaphore;
};

/**
 * This is the Windows factory for creating various synchronization objects.
 */
class FSynchronizeFactoryWin : public FSynchronizeFactory
{
public:
	/**
	 * Zeroes its members
	 */
	FSynchronizeFactoryWin(void);

	/**
	 * Creates a new critical section
	 *
	 * @return The new critical section object or NULL otherwise
	 */
	virtual FCriticalSection* CreateCriticalSection(void);

	/**
	 * Creates a new event
	 *
	 * @param bIsManualReset Whether the event requires manual reseting or not
	 * @param InName Whether to use a commonly shared event or not. If so this
	 * is the name of the event to share.
	 *
	 * @return Returns the new event object if successful, NULL otherwise
	 */
	virtual FEvent* CreateSynchEvent(UBOOL bIsManualReset = FALSE,const TCHAR* InName = NULL);

	/**
	 * Cleans up the specified synchronization object using the correct heap
	 *
	 * @param InSynchObj The synchronization object to destroy
	 */
	virtual void Destroy(FSynchronize* InSynchObj);
};

/**
 * This is the Windows class used for all poolable threads
 */
class FQueuedThreadWin : public FQueuedThread
{
	/**
	 * The event that tells the thread there is work to do
	 */
	FEvent* DoWorkEvent;

	/**
	 * The thread handle to clean up. Must be closed or this will leak resources
	 */
	HANDLE ThreadHandle;

	/**
	 * The thread ID for this thread
	 */
	unsigned long  ThreadID;

	/**
	 * If true, the thread should exit
	 */
	UBOOL TimeToDie;

	/**
	 * The work this thread is doing
	 */
	FQueuedWork* QueuedWork;

	/**
	 * The synchronization object for the work member
	 */
	FCriticalSection* QueuedWorkSynch;

	/**
	 * The pool this thread belongs to
	 */
	FQueuedThreadPool* OwningThreadPool;

	/**
	 * The thread entry point. Simply forwards the call on to the right
	 * thread main function
	 */
	static unsigned long  STDCALL _ThreadProc(LPVOID pThis);

	/**
	 * The real thread entry point. It waits for work events to be queued. Once
	 * an event is queued, it executes it and goes back to waiting.
	 */
	void Run(void);

public:
	/**
	 * Zeros any members
	 */
	FQueuedThreadWin(void);

	/**
	 * Deletes any allocated resources. Kills the thread if it is running.
	 */
	virtual ~FQueuedThreadWin(void);

	/**
	 * Creates the thread with the specified stack size and creates the various
	 * events to be able to communicate with it.
	 *
	 * @param InPool The thread pool interface used to place this thread
	 *		  back into the pool of available threads when its work is done
	 * @param ProcessorMask The processor set to run the thread on
	 * @param InStackSize The size of the stack to create. 0 means use the
	 *		  current thread's stack size
	 *
	 * @return True if the thread and all of its initialization was successful, false otherwise
	 */
	virtual UBOOL Create(class FQueuedThreadPool* InPool,DWORD ProcessorMask,
		DWORD InStackSize = 0);
	
	/**
	 * Tells the thread to exit. If the caller needs to know when the thread
	 * has exited, it should use the bShouldWait value and tell it how long
	 * to wait before deciding that it is deadlocked and needs to be destroyed.
	 * NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
	 *
	 * @param bShouldWait If true, the call will wait for the thread to exit
	 * @param bShouldDeleteSelf Whether to delete ourselves upon completion
	 *
	 * @return True if the thread exited graceful, false otherwise
	 */
	virtual UBOOL Kill(UBOOL bShouldWait = FALSE, UBOOL bShouldDeleteSelf = FALSE);

	/**
	 * Tells the thread there is work to be done. Upon completion, the thread
	 * is responsible for adding itself back into the available pool.
	 *
	 * @param InQueuedWork The queued work to perform
	 */
	virtual void DoWork(FQueuedWork* InQueuedWork);
};

/**
 * This class fills in the platform specific features that the parent
 * class doesn't implement. The parent class handles all common, non-
 * platform specific code, while this class provides all of the Windows
 * specific methods. It handles the creation of the threads used in the
 * thread pool.
 */
class FQueuedThreadPoolWin : public FQueuedThreadPoolBase
{
public:
	/**
	 * Cleans up any threads that were allocated in the pool
	 */
	virtual ~FQueuedThreadPoolWin(void);

	/**
	 * Creates the thread pool with the specified number of threads
	 *
	 * @param InNumQueuedThreads Specifies the number of threads to use in the pool
	 * @param ProcessorMask Specifies which processors should be used by the pool
	 * @param StackSize The size of stack the threads in the pool need (32K default)
	 *
	 * @return Whether the pool creation was successful or not
	 */
	virtual UBOOL Create(DWORD InNumQueuedThreads,DWORD ProcessorMask = 0,
		DWORD StackSize = (32 * 1024));
};

/**
 * This is the base interface for all runnable thread classes. It specifies the
 * methods used in managing its life cycle.
 */
class FRunnableThreadWin : public FRunnableThread
{
	/**
	 * The thread handle for the thread
	 */
	HANDLE Thread;

	/**
	 * The runnable object to execute on this thread
	 */
	FRunnable* Runnable;

	/**
	 * Whether we should delete ourselves on thread exit
	 */
	UBOOL bShouldDeleteSelf;

	/**
	 * Whether we should delete the runnable on thread exit
	 */
	UBOOL bShouldDeleteRunnable;

	/**
	 * The priority to run the thread at
	 */
	EThreadPriority ThreadPriority;

	/**
	* ID set during thread creation
	*/
	unsigned long  ThreadID;

	/**
	 * The thread entry point. Simply forwards the call on to the right
	 * thread main function
	 */
	static unsigned long  STDCALL _ThreadProc(LPVOID pThis);

	/**
	 * The real thread entry point. It calls the Init/Run/Exit methods on
	 * the runnable object
	 */
	DWORD Run(void);

public:
	/**
	 * Zeroes members
	 */
	FRunnableThreadWin(void);

	/**
	 * Cleans up any resources
	 */
	~FRunnableThreadWin(void);

	/**
	 * Creates the thread with the specified stack size and thread priority.
	 *
	 * @param InRunnable The runnable object to execute
	 * @param ThreadName Name of the thread
	 * @param bAutoDeleteSelf Whether to delete this object on exit
	 * @param bAutoDeleteRunnable Whether to delete the runnable object on exit
	 * @param InStackSize The size of the stack to create. 0 means use the
	 * current thread's stack size
	 * @param InThreadPri Tells the thread whether it needs to adjust its
	 * priority or not. Defaults to normal priority
	 *
	 * @return True if the thread and all of its initialization was successful, false otherwise
	 */
	UBOOL Create(FRunnable* InRunnable, const ANSICHAR* ThreadName,
		UBOOL bAutoDeleteSelf = 0,UBOOL bAutoDeleteRunnable = 0,DWORD InStackSize = 0,
		EThreadPriority InThreadPri = TPri_Normal);
	
	/**
	 * Changes the thread priority of the currently running thread
	 *
	 * @param NewPriority The thread priority to change to
	 */
	virtual void SetThreadPriority(EThreadPriority NewPriority);

	/**
	 * Tells the OS the preferred CPU to run the thread on. NOTE: Don't use
	 * this function unless you are absolutely sure of what you are doing
	 * as it can cause the application to run poorly by preventing the
	 * scheduler from doing its job well.
	 *
	 * @param ProcessorNum The preferred processor for executing the thread on
	 */
	virtual void SetProcessorAffinity(DWORD ProcessorNum);

	/**
	 * Tells the thread to either pause execution or resume depending on the
	 * passed in value.
	 *
	 * @param bShouldPause Whether to pause the thread (true) or resume (false)
	 */
	virtual void Suspend(UBOOL bShouldPause = 1);

	/**
	 * Tells the thread to exit. If the caller needs to know when the thread
	 * has exited, it should use the bShouldWait value and tell it how long
	 * to wait before deciding that it is deadlocked and needs to be destroyed.
	 * NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
	 *
	 * @param bShouldWait If true, the call will wait for the thread to exit
	 *
	 * @return True if the thread exited graceful, false otherwise
	 */
	virtual UBOOL Kill(UBOOL bShouldWait = FALSE);

	/**
	 * Halts the caller until this thread is has completed its work.
	 *
	 * @param WaitTime Time in milliseconds to wait before abandoning the event
	 * (DWORD)-1 is treated as wait infinite
	 *
	 * @return TRUE if the thread completed, FALSE if the wait timed out
	 */
	virtual UBOOL WaitForCompletion( DWORD WaitTime = -1 );

	/**
	* Thread ID for this thread 
	*
	* @return ID that was set by CreateThread
	*/
	virtual DWORD GetThreadID(void);
};

/**
 * This is the factory interface for creating threads on Windows
 */
class FThreadFactoryWin : public FThreadFactory
{
public:
	/**
	 * Creates the thread with the specified stack size and thread priority.
	 *
	 * @param InRunnable The runnable object to execute
	 * @param ThreadName Name of the thread
	 * @param bAutoDeleteSelf Whether to delete this object on exit
	 * @param bAutoDeleteRunnable Whether to delete the runnable object on exit
	 * @param InStackSize The size of the stack to create. 0 means use the
	 * current thread's stack size
	 * @param InThreadPri Tells the thread whether it needs to adjust its
	 * priority or not. Defaults to normal priority
	 *
	 * @return The newly created thread or NULL if it failed
	 */
	virtual FRunnableThread* CreateThread(FRunnable* InRunnable, const ANSICHAR* ThreadName,
		UBOOL bAutoDeleteSelf = 0,UBOOL bAutoDeleteRunnable = 0,
		DWORD InStackSize = 0,EThreadPriority InThreadPri = TPri_Normal);

	/**
	 * Cleans up the specified thread object using the correct heap
	 *
	 * @param InThread The thread object to destroy
	 */
	virtual void Destroy(FRunnableThread* InThread);
};



/** Thread safe counter */
class FThreadSafeCounter
{
public:
	/** Default constructor, initializing Value to 0 */
	FThreadSafeCounter()
	{
		Value = 0;
	}
	/**
	 * Constructor, initializing counter to passed in value.
	 *
	 * @param InValue	Value to initialize counter to
	 */
	FThreadSafeCounter( INT InValue )
	{
		Value = InValue;
	}

	/**
	 * Increment and return new value.	
	 *
	 * @return the incremented value
	 */
	INT Increment()
	{
		return appInterlockedIncrement(&Value);
	}

	/**
	 * Decrement and return new value.
	 *
	 * @return the decremented value
	 */
	INT Decrement()
	{
		return appInterlockedDecrement(&Value);
	}

	/**
	 * Resets the counter's value to zero.
	 *
	 * @return the old value.
	 */
	INT Reset()
	{
		return appInterlockedExchange(&Value, 0);
	}

	/**
	 * Return the current value.
	 *
	 * @return the current value
	 */
	INT GetValue() const
	{
		return Value;
	}

private:
	// Hidden on purpose as usage wouldn't be thread safe.
	FThreadSafeCounter( const FThreadSafeCounter& Other ){}
	void operator=(const FThreadSafeCounter& Other){}

	/** Value of counter, aligned to 32 bits as required by Windows */
	volatile INT Value;
};


/**
 * This is a utility class that handles scope level locking. It's very useful
 * to keep from causing deadlocks due to exceptions being caught and knowing
 * about the number of locks a given thread has on a resource. Example:
 *
 * <code>
 *	{
 *		// Syncronize thread access to the following data
 *		FScopeLock ScopeLock(SynchObject);
 *		// Access data that is shared among multiple threads
 *		...
 *		// When ScopeLock goes out of scope, other threads can access data
 *	}
 * </code>
 */
class FScopeLock
{
	/**
	 * The synchronization object to aggregate and scope manage
	 */
	FCriticalSection* SynchObject;

	/**
	 * Default constructor hidden on purpose
	 */
	FScopeLock(void);

	/**
	 * Copy constructor hidden on purpose
	 *
	 * @param InScopeLock ignored
	 */
	FScopeLock(FScopeLock* InScopeLock);

	/**
	 * Assignment operator hidden on purpose
	 *
	 * @param InScopeLock ignored
	 */
	FScopeLock& operator=(FScopeLock& InScopeLock) { return *this; }

public:
	/**
	 * Constructor that performs a lock on the synchronization object
	 *
	 * @param InSynchObject The synchronization object to manage
	 */
	FScopeLock(FCriticalSection* InSynchObject) :
		SynchObject(InSynchObject)
	{
		check(SynchObject);
		SynchObject->Lock();
	}

	/**
	 * Destructor that performs a release on the synchronization object
	 */
	~FScopeLock(void)
	{
		check(SynchObject);
		SynchObject->Unlock();
	}
};


/**
 * Thread-safe FIFO queue with a fixed maximum size.
 */
template <typename ElementType>
class TQueueThreadSafe : private TQueue<ElementType>
{
public:
	/**
	 * Constructor, allocates the buffer.
	 * @param InMaxNumElements	- Maximum number of elements in the queue
	 */
	TQueueThreadSafe( INT InMaxNumElements )
	:	TQueue( InMaxNumElements )
	{
	}

	/**
	 * Returns the current number of elements stored in the queue.
	 * @return	- Current number of elements stored in the queue
	 */
	volatile INT Num() const
	{
		return TQueue<ElementType>::Num();
	}

	/**
	 * Returns the maximum number of elements that can be stored in the queue.
	 * @return	- Maximum number of elements that can be stored in the queue
	 */
	volatile INT GetMaxNumElements() const
	{
		return TQueue<ElementType>::GetMaxNumElements();
	}

	/**
	 * Adds an element to the head of the queue.
	 * @param Element	- Element to be added
	 * @return			- TRUE if the element got added, FALSE if the queue was already full
	 */
	UBOOL	Push( const ElementType& Element )
	{
		FScopeLock Lock( &CriticalSecion );
		return TQueue<ElementType>::Push( Element );
	}

	/**
	 * Removes and returns the tail of the queue (the 'oldest' element).
	 * @param Element	- [out] If successful, contains the element that was removed
	 * @return			- TRUE if the element got removed, FALSE if the queue was empty
	 */
	UBOOL	Pop( ElementType& Element )
	{
		FScopeLock Lock( &CriticalSecion );
		return TQueue<ElementType>::Pop( Element );
	}

protected:
	/** Used for synchronizing access to the queue. */
	FCriticalSection	CriticalSecion;
};


/**
 * Thread-safe Producer/Consumer FIFO queue with a fixed maximum size.
 * It supports multiple producers and multiple consumers.
 */
template <typename ElementType>
class TProducerConsumerQueue : private TQueueThreadSafe<ElementType>
{
public:
	/**
	 * Constructor, allocates the buffer.
	 * @param InMaxNumElements	- Maximum number of elements in the queue
	 */
	TProducerConsumerQueue( INT InMaxNumElements )
	:	TQueueThreadSafe( InMaxNumElements )
	{
		Semaphore.Create( InMaxNumElements );
	}

	/**
	 * Returns the current number of elements stored in the queue.
	 * @return	- Current number of elements stored in the queue
	 */
	volatile INT Num() const
	{
		return TQueueThreadSafe<ElementType>::Num();
	}

	/**
	 * Returns the maximum number of elements that can be stored in the queue.
	 * @return	- Maximum number of elements that can be stored in the queue
	 */
	volatile INT GetMaxNumElements() const
	{
		return TQueueThreadSafe<ElementType>::GetMaxNumElements();
	}

	/**
	 * Adds an element to the head of the queue.
	 *
	 * @param Element	- Element to be added
	 * @return			- TRUE if the element got added, FALSE if the queue was already full
	 */
	UBOOL	Push( const ElementType& Element )
	{
		UBOOL bSucceeded = TQueueThreadSafe<ElementType>::Push( Element );
		if ( bSucceeded )
		{
			Semaphore.Trigger();
		}
		return bSucceeded;
	}

	/**
	 * Removes and returns the tail of the queue (the 'oldest' element).
	 * If the queue is empty, wait for an element to come in.
	 *
	 * @param Element	- [out] If successful, contains the element that was removed
	 * @param Timeout	- How long to wait for new elements if the queue is empty, in milliseconds. -1 means INFINITE.
	 * @return			- TRUE if the element got removed, FALSE if the queue was empty
	 */
	UBOOL	Pop( ElementType& Element, DWORD Timeout )
	{
		Semaphore.Wait( Timeout );
		return TQueueThreadSafe<ElementType>::Pop( Element );
	}

	/** Bumps the semaphore to maximum, to release up to 'MaxNumElements' waiting threads right away. */
	void	TriggerAll()
	{
		for ( INT NumPotentialThreads=0; NumPotentialThreads < GetMaxNumElements(); ++NumPotentialThreads )
		{
			Semaphore.Trigger();
		}
	}

protected:
	/** Used for blocking Pops. The count represents the current number of elements in the queue. */
	FSemaphoreWin		Semaphore;
};


}
