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


#ifndef PX_PHYSX_PROFILE_SCOPED_MUTEX_LOCK_H
#define PX_PHYSX_PROFILE_SCOPED_MUTEX_LOCK_H
#include "PxProfileBase.h"

namespace physx { namespace profile {

	/**
	 *	Generic class to wrap any mutex type that has lock and unlock methods
	 */
	template<typename TMutexType>
	struct ScopedLockImpl
	{
		TMutexType* mMutex;
		ScopedLockImpl( TMutexType* inM ) : mMutex( inM )
		{
			if ( mMutex ) mMutex->lock();
		}
		~ScopedLockImpl()
		{
			if ( mMutex ) mMutex->unlock();
		}
	};

	/**
	 *	Null locking system that does nothing.
	 */
	struct NullLock
	{
		template<typename TDataType> NullLock( TDataType*) {}
	};
}}

#endif
