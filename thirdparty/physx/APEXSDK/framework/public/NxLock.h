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


#ifndef NX_LOCK_H
#define NX_LOCK_H

/*!
\file
\brief classes NxSceneReadLock, NxSceneWriteLock
*/

#include "NxApexInterface.h"

namespace physx
{
namespace apex
{

/**
\brief RAII wrapper for the NxApexScene read lock.

Use this class as follows to lock the scene for reading by the current thread 
for the duration of the enclosing scope:

	NxReadLock lock(sceneRef);

\see NxApexScene::apexacquireReadLock(), NxApexScene::apexUnacquireReadLock(), NxApexSceneDesc::useRWLock
*/
class NxReadLock
{
	NxReadLock(const NxReadLock&);
	NxReadLock& operator=(const NxReadLock&);

public:
	
	/**
	\brief Constructor
	\param lockable The object to lock for reading
	\param file Optional string for debugging purposes
	\param line Optional line number for debugging purposes
	*/
	NxReadLock(const NxApexInterface& lockable, const char* file=NULL, PxU32 line=0)
		: mLockable(lockable)
	{
		mLockable.acquireReadLock(file, line);
	}

	~NxReadLock()
	{
		mLockable.releaseReadLock();
	}

private:

	const NxApexInterface& mLockable;
};

/**
\brief RAII wrapper for the NxApexScene write lock.

Use this class as follows to lock the scene for writing by the current thread 
for the duration of the enclosing scope:

	NxWriteLock lock(sceneRef);

\see NxApexScene::apexacquireWriteLock(), NxApexScene::apexUnacquireWriteLock(), NxApexSceneDesc::useRWLock
*/
class NxWriteLock
{
	NxWriteLock(const NxWriteLock&);
	NxWriteLock& operator=(const NxWriteLock&);

public:

	/**
	\brief Constructor
	\param lockable The object to lock for writing
	\param file Optional string for debugging purposes
	\param line Optional line number for debugging purposes
	*/
	NxWriteLock(const NxApexInterface& lockable, const char* file=NULL, PxU32 line=0)
		: mLockable(lockable)
	{
		mLockable.acquireWriteLock(file, line);
	}

	~NxWriteLock()
	{
		mLockable.releaseWriteLock();
	}

private:
	const NxApexInterface& mLockable;
};


} // namespace apex
} // namespace physx

/**
\brief Lock an object for writing by the current thread for the duration of the enclosing scope.
*/
#define NX_WRITE_LOCK(LOCKABLE) physx::apex::NxWriteLock __nxWriteLock(LOCKABLE, __FILE__, __LINE__);
/**
\brief Lock an object for reading by the current thread for the duration of the enclosing scope.
*/
#define NX_READ_LOCK(LOCKABLE) physx::apex::NxReadLock __nxReadLock(LOCKABLE, __FILE__, __LINE__);

/** @} */
#endif
