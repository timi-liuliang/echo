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
#include "PsUserAllocated.h"
#include "PsMutex.h"
#include "foundation/PxAssert.h"
#include "PsAtomic.h"
#include "PsThread.h"

#include <pthread.h>

namespace physx
{
namespace shdfnd
{

namespace 
{
	struct MutexUnixImpl
	{
		pthread_mutex_t lock;
		Thread::Id owner;
	};

	MutexUnixImpl* getMutex(MutexImpl* impl)
	{
		return reinterpret_cast<MutexUnixImpl*>(impl);
	}
}

MutexImpl::MutexImpl() 
{ 
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#if !defined(PX_ANDROID)
	// mimic default windows behavior where applicable
	pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
#endif
	pthread_mutex_init(&getMutex(this)->lock, &attr);
	pthread_mutexattr_destroy(&attr);
}

MutexImpl::~MutexImpl() 
{ 
	pthread_mutex_destroy(&getMutex(this)->lock); 
}

void MutexImpl::lock()
{
	int err = pthread_mutex_lock(&getMutex(this)->lock);
	PX_ASSERT(!err);
	PX_UNUSED(err);

#if PX_DEBUG
	getMutex(this)->owner = Thread::getId();
#endif
}

bool MutexImpl::trylock()
{
	bool success = !pthread_mutex_trylock(&getMutex(this)->lock);
#if PX_DEBUG
	if (success)
		getMutex(this)->owner = Thread::getId();
#endif
	return success;
}

void MutexImpl::unlock()
{
#if PX_DEBUG
	if (getMutex(this)->owner != Thread::getId())
	{
		shdfnd::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Mutex must be unlocked only by thread that has already acquired lock");
		return;
	}	
#endif

	int err = pthread_mutex_unlock(&getMutex(this)->lock);
	PX_ASSERT(!err);
	PX_UNUSED(err);
}

const PxU32 gSize = sizeof(MutexUnixImpl);
const PxU32& MutexImpl::getSize()
{
	return gSize;
}

class ReadWriteLockImpl
{
public:
    Mutex	mutex;
    volatile int	readerCounter;
};

ReadWriteLock::ReadWriteLock()
{
    mImpl = reinterpret_cast<ReadWriteLockImpl *>(PX_ALLOC(sizeof(ReadWriteLockImpl), PX_DEBUG_EXP("ReadWriteLockImpl")));
	PX_PLACEMENT_NEW(mImpl, ReadWriteLockImpl);

    mImpl->readerCounter = 0;
}

ReadWriteLock::~ReadWriteLock()
{
	mImpl->~ReadWriteLockImpl();
	PX_FREE( mImpl );
}

void ReadWriteLock::lockReader()
{
	mImpl->mutex.lock();

	atomicIncrement(&mImpl->readerCounter);

	mImpl->mutex.unlock();
}

void ReadWriteLock::lockWriter()
{
	mImpl->mutex.lock();
	
	while(mImpl->readerCounter != 0);
}

void ReadWriteLock::unlockReader()
{
	atomicDecrement(&mImpl->readerCounter);
}

void ReadWriteLock::unlockWriter()
{
	mImpl->mutex.unlock();
}


} // namespace shdfnd
} // namespace physx

