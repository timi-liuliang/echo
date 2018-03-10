/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <stdio.h>
#include "UserAllocator.h"

#include "MemTracker.h"

#include "foundation/PxAssert.h"

#pragma warning(disable:4100 4152)

class PhysX28Allocator;

#ifdef USE_MEM_TRACKER
#if NX_SDK_VERSION_MAJOR == 2
static const char* PHYSX28X = "PHYSX28X";
#endif

static const char* USERALLOCATOR = "UserAllocator";
#endif

/*============  UserPxAllocator  ============*/

#ifdef USE_MEM_TRACKER

#include <windows.h>

class MemMutex
{
public:
	MemMutex(void);
	~MemMutex(void);

public:
	// Blocking Lock.
	void Lock(void);

	// Unlock.
	void Unlock(void);

private:
	CRITICAL_SECTION m_Mutex;
};

//==================================================================================
MemMutex::MemMutex(void)
{
	InitializeCriticalSection(&m_Mutex);
}

//==================================================================================
MemMutex::~MemMutex(void)
{
	DeleteCriticalSection(&m_Mutex);
}

//==================================================================================
// Blocking Lock.
//==================================================================================
void MemMutex::Lock(void)
{
	EnterCriticalSection(&m_Mutex);
}

//==================================================================================
// Unlock.
//==================================================================================
void MemMutex::Unlock(void)
{
	LeaveCriticalSection(&m_Mutex);
}

static inline MemMutex& gMemMutex()
{
	static MemMutex sMemMutex;
	return sMemMutex;
}

static size_t getThreadId(void)
{
	return GetCurrentThreadId();
}

static bool memoryReport(MEM_TRACKER::MemTracker *memTracker,MEM_TRACKER::MemoryReportFormat format,const char *fname,bool reportAllLeaks) // detect memory leaks and, if any, write out a report to the filename specified.
{
	bool ret = false;

	size_t leakCount;
	size_t leaked = memTracker->detectLeaks(leakCount);
	if ( leaked )
	{
		physx::PxU32 dataLen;
		void *mem = memTracker->generateReport(format,fname,dataLen,reportAllLeaks);
		if ( mem )
		{
			FILE *fph = fopen(fname,"wb");
			fwrite(mem,dataLen,1,fph);
			fclose(fph);
			memTracker->releaseReportMemory(mem);
		}
		ret = true; // it leaked memory!
	}
	return ret;
}

class ScopedLock
{
public:
	ScopedLock(MemMutex &mutex, bool enabled) : mMutex(mutex), mEnabled(enabled) 
	{
		if (mEnabled) mMutex.Lock(); 
	}
	~ScopedLock()
	{
		if (mEnabled) mMutex.Unlock(); 
	}

private:
	ScopedLock();
	ScopedLock(const ScopedLock&);
	ScopedLock& operator=(const ScopedLock&);

	MemMutex& mMutex;
	bool mEnabled;
};

#define SCOPED_TRACKER_LOCK_IF(USE_TRACKER) ScopedLock lock(gMemMutex(), (USE_TRACKER))
#define TRACKER_CALL_IF(USE_TRACKER, CALL)  if ((USE_TRACKER)) { (CALL); }

void releaseAndReset(MEM_TRACKER::MemTracker*& memTracker)
{
	MEM_TRACKER::releaseMemTracker(memTracker);
	memTracker = NULL;
}

#else

#define SCOPED_TRACKER_LOCK_IF(USE_TRACKER) 
#define TRACKER_CALL_IF(USE_TRACKER, CALL)  

#endif

int UserPxAllocator::gMemoryTrackerClients = 0;
MEM_TRACKER::MemTracker	*UserPxAllocator::mMemoryTracker=NULL;

unsigned int UserPxAllocator::mNumAllocations = 0;
unsigned int UserPxAllocator::mNumFrees = 0;

UserPxAllocator::UserPxAllocator(const char* context,
                                 const char* /*dllName*/,
                                 bool useTrackerIfSupported /* = true */)
	: mContext(context)
	, mMemoryAllocated(0)
	, mNxAllocator(NULL)
	, mUseTracker(useTrackerIfSupported)
{
	SCOPED_TRACKER_LOCK_IF(mUseTracker);
	TRACKER_CALL_IF(mUseTracker && NULL == mMemoryTracker, mMemoryTracker = MEM_TRACKER::createMemTracker());
	TRACKER_CALL_IF(trackerEnabled(), ++gMemoryTrackerClients);
}

void* UserPxAllocator::allocate(size_t size, const char* typeName, const char* filename, int line)
{
	void* ret = 0;

	PX_UNUSED(typeName);
	PX_UNUSED(filename);
	PX_UNUSED(line);
	SCOPED_TRACKER_LOCK_IF(trackerEnabled());

#ifdef PX_WINDOWS
	ret = ::_aligned_malloc(size, 16);
#elif defined(PX_ANDROID) || defined(PX_LINUX)
	/* Allocate size + (15 + sizeof(void*)) bytes and shift pointer further, write original address in the beginning of block.*/
	/* Weirdly, memalign sometimes returns unaligned address */
	//	ret = ::memalign(size, 16);
	size_t alignment = 16;
	void* originalRet = ::malloc(size + 2*alignment + sizeof(void*));
	// find aligned location
	ret = ((char*)originalRet) + 2 * alignment - (((size_t)originalRet) & 0xF);
	// write block address prior to aligned position, so it could be possible to free it later
	::memcpy((char*)ret - sizeof(originalRet), &originalRet, sizeof(originalRet));
#else
	ret = ::malloc(size);
#endif

	TRACKER_CALL_IF(trackerEnabled(), mMemoryAllocated += size);
	TRACKER_CALL_IF(trackerEnabled(), mMemoryTracker->trackAlloc(getThreadId(),ret, size, MEM_TRACKER::MT_MALLOC, USERALLOCATOR, typeName, filename, (physx::PxU32)line));

	// this should probably be a atomic increment
	mNumAllocations++;

	return ret;
}

void UserPxAllocator::deallocate(void* memory)
{
	SCOPED_TRACKER_LOCK_IF(trackerEnabled());

	if (memory)
	{
#ifdef PX_WINDOWS
		::_aligned_free(memory);
#elif defined(PX_ANDROID) || defined(PX_LINUX)
		// Looks scary, but all it does is getting original unaligned block address back from 4/8 bytes (depends on pointer size) prior to <memory> pointer and frees this memory 
		void* originalPtr = (void*)(*(size_t*)((char*)memory - sizeof(void*)));
		::free(originalPtr);
#else
		::free(memory);
#endif

		// this should probably be a atomic decrement
		mNumFrees++;
	}

	TRACKER_CALL_IF(trackerEnabled(), mMemoryTracker->trackFree(getThreadId(), memory, MEM_TRACKER::MT_FREE, USERALLOCATOR, __FILE__, __LINE__));
}


#if NX_SDK_VERSION_MAJOR == 2

#include "NxUserAllocator.h"

#pragma warning(disable:4512)

class PhysX28Allocator : public NxUserAllocator
{
public:
	PhysX28Allocator(UserPxAllocator& parent,MEM_TRACKER::MemTracker *memoryTracker) : mAlloc(parent), mMemoryTracker(memoryTracker) {}
	~PhysX28Allocator() {}

	virtual void* mallocDEBUG(size_t size, const char* fileName, int line)
	{
		return mallocDEBUG(size,fileName,line,"PhysX284",NX_MEMORY_PERSISTENT);
	}
	virtual void* mallocDEBUG(size_t size, const char* fileName, int line, const char* className, NxMemoryType type);
	virtual void* malloc(size_t size)
	{
		return mallocDEBUG(size,__FILE__,__LINE__,"PhysX284",NX_MEMORY_PERSISTENT);
	}

	virtual void* malloc(size_t size, NxMemoryType type)
	{
		return mallocDEBUG(size,__FILE__,__LINE__,"PhysX284",type);
	}

	virtual void* realloc(void* memory, size_t size);
	virtual void free(void* memory);

private:
	bool					trackerEnabled() const { return NULL != mMemoryTracker; }

	UserPxAllocator&		mAlloc;
	MEM_TRACKER::MemTracker	*mMemoryTracker;
};



void* PhysX28Allocator::mallocDEBUG(size_t size, const char* file, int line,const char *className,NxMemoryType /*type*/)
{
	void* ret = 0;

	SCOPED_TRACKER_LOCK_IF(trackerEnabled());

#ifdef PX_WINDOWS
	ret = ::_aligned_malloc(size, 16);
#else
	ret = ::malloc(size);
#endif

	TRACKER_CALL_IF(trackerEnabled(), mMemoryTracker->trackAlloc(getThreadId(), ret, size, MEM_TRACKER::MT_MALLOC, PHYSX28X, className, file, line));

	return ret;
}

void* PhysX28Allocator::realloc(void* _memory, size_t size)
{
	void* ret = 0;

	SCOPED_TRACKER_LOCK_IF(trackerEnabled());

#ifdef PX_WINDOWS
	ret = ::_aligned_realloc(_memory, size, 16);
#else
	ret = ::realloc(_memory, size);
#endif

	TRACKER_CALL_IF(trackerEnabled(), mMemoryTracker->trackRealloc(getThreadId(), _memory, ret, size, PHYSX28X, PHYSX28X, __FILE__, __LINE__));

	return ret;
}

void PhysX28Allocator::free(void* memory)
{
	SCOPED_TRACKER_LOCK_IF(trackerEnabled());

#ifdef PX_WINDOWS
	::_aligned_free(memory);
#else
	::free(memory);
#endif

	TRACKER_CALL_IF(trackerEnabled(), mMemoryTracker->trackFree(getThreadId(), memory, MEM_TRACKER::MT_FREE, PHYSX28X, __FILE__, __LINE__));
}

NxUserAllocator* UserPxAllocator::get28PhysXAllocator()
{
	if (!mNxAllocator)
	{
		mNxAllocator = ::new PhysX28Allocator(*this, trackerEnabled() ? mMemoryTracker : NULL);
	}
	return mNxAllocator;
}

#else

NxUserAllocator* UserPxAllocator::get28PhysXAllocator()
{
	return NULL;
}

#endif

UserPxAllocator::~UserPxAllocator()
{
#if NX_SDK_VERSION_MAJOR == 2
	delete mNxAllocator;
#endif

	SCOPED_TRACKER_LOCK_IF(trackerEnabled());
	TRACKER_CALL_IF(trackerEnabled() && (0 == --gMemoryTrackerClients), releaseAndReset(mMemoryTracker));
}

bool UserPxAllocator::dumpMemoryLeaks(const char* filename)
{
	bool leaked = false;

	PX_UNUSED(filename);
	TRACKER_CALL_IF(mMemoryTracker, leaked = memoryReport(mMemoryTracker, MEM_TRACKER::MRF_SIMPLE_HTML, filename, true));

	return leaked;
}

#undef SCOPED_TRACKER_LOCK_IF
#undef TRACKER_CALL_IF