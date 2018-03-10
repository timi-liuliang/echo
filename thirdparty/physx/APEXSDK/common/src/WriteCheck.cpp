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

#include "WriteCheck.h"
#include "ApexRWLockable.h"
#include "PsThread.h"
#include "NiApexSDK.h"

namespace physx
{
namespace apex
{

WriteCheck::WriteCheck(ApexRWLockable* scene, const char* functionName, bool allowReentry) 
: mLockable(scene), mName(functionName), mAllowReentry(allowReentry), mErrorCount(0)
{
	if (NxGetApexSDK()->isConcurrencyCheckEnabled() && mLockable)
	{
		if (!mLockable->startWrite(mAllowReentry) && !mLockable->isEnabled())
		{
			APEX_INTERNAL_ERROR("An API write call (%s) was made from thread %d but acquireWriteLock() was not called first, note that "
				"when NxApexSDKDesc::enableConcurrencyCheck is enabled all API reads and writes must be "
				"wrapped in the appropriate locks.", mName, PxU32(physx::shdfnd::Thread::getId()));
		}

		// Record the NxApexScene read/write error counter which is
		// incremented any time a NxApexScene::apexStartWrite/apexStartRead fails
		// (see destructor for additional error checking based on this count)
		mErrorCount = mLockable->getReadWriteErrorCount();
	}
}


WriteCheck::~WriteCheck()
{
	if (NxGetApexSDK()->isConcurrencyCheckEnabled() && mLockable)
	{
		// By checking if the NpScene::mConcurrentErrorCount has been incremented
		// we can detect if an erroneous read/write was performed during 
		// this objects lifetime. In this case we also print this function's
		// details so that the user can see which two API calls overlapped
		if (mLockable->getReadWriteErrorCount() != mErrorCount && !mLockable->isEnabled())
		{
			APEX_INTERNAL_ERROR("Leaving %s on thread %d, an overlapping API read or write by another thread was detected.", mName, PxU32(physx::shdfnd::Thread::getId()));
		}

		mLockable->stopWrite(mAllowReentry);
	}
}

}
}