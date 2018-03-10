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


#include "windows/PsWindowsInclude.h"
#include "PsUserAllocated.h"
#include "PsMutex.h"
#include "PsThread.h"
#include "foundation/PxAssert.h"

namespace physx
{
namespace shdfnd
{

namespace 
{
	struct MutexWinImpl
	{
		CRITICAL_SECTION mLock;
		Thread::Id mOwner;
	};

	MutexWinImpl* getMutex(MutexImpl* impl)
	{
		return reinterpret_cast<MutexWinImpl*>(impl);
	}
}

static const DWORD gWin8ArmSpinCount = 4096;

MutexImpl::MutexImpl() 
{ 
#ifndef PX_WINMODERN
    InitializeCriticalSection(&getMutex(this)->mLock);
#else
	InitializeCriticalSectionEx(&getMutex(this)->mLock, gWin8ArmSpinCount, CRITICAL_SECTION_NO_DEBUG_INFO);
#endif

	getMutex(this)->mOwner = 0;
}

MutexImpl::~MutexImpl() 
{ 
	DeleteCriticalSection(&getMutex(this)->mLock); 
}

void MutexImpl::lock()
{
	EnterCriticalSection(&getMutex(this)->mLock);

#ifdef PX_DEBUG
	getMutex(this)->mOwner = Thread::getId();
#endif
}

bool MutexImpl::trylock()
{
	bool success = TryEnterCriticalSection(&getMutex(this)->mLock) != 0;
#ifdef PX_DEBUG
	if (success)
		getMutex(this)->mOwner = Thread::getId();
#endif
	return success;
}

void MutexImpl::unlock()
{
#ifdef PX_DEBUG
	// ensure we are already holding the lock
	if (getMutex(this)->mOwner != Thread::getId())
	{
		shdfnd::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Mutex must be unlocked only by thread that has already acquired lock");
		return;
	}
	
#endif

	LeaveCriticalSection(&getMutex(this)->mLock);
}

static const PxU32 gSize = sizeof(MutexWinImpl);

const PxU32& MutexImpl::getSize()  { return gSize; }

class ReadWriteLockImpl
{
	PX_NOCOPY(ReadWriteLockImpl)
public:
	ReadWriteLockImpl() {}
    Mutex				mutex;
    volatile LONG		readerCount;	//handle recursive writer locking
};

ReadWriteLock::ReadWriteLock()
{
    mImpl = reinterpret_cast<ReadWriteLockImpl*>(PX_ALLOC(sizeof(ReadWriteLockImpl), PX_DEBUG_EXP("ReadWriteLockImpl")));
	PX_PLACEMENT_NEW(mImpl, ReadWriteLockImpl);

    mImpl->readerCount = 0;
}

ReadWriteLock::~ReadWriteLock()
{
	mImpl->~ReadWriteLockImpl();
    PX_FREE( mImpl );
}

void ReadWriteLock::lockReader()
{
	mImpl->mutex.lock();

	InterlockedIncrement( &mImpl->readerCount);

	mImpl->mutex.unlock();
}

void ReadWriteLock::lockWriter()
{
	mImpl->mutex.lock();

	// spin lock until no readers
	while (mImpl->readerCount);
}

void ReadWriteLock::unlockReader()
{
	InterlockedDecrement( &mImpl->readerCount );
}

void ReadWriteLock::unlockWriter()
{
	mImpl->mutex.unlock();
}

} // namespace shdfnd
} // namespace physx

