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


#include "NpReadCheck.h"

#include "NpScene.h"

using namespace physx;

NpReadCheck::NpReadCheck(const NpScene* scene, const char* functionName)
	: mScene(scene), mName(functionName), mErrorCount(0)
{
	if (mScene)
	{
		if (!mScene->startRead())
		{
			if (mScene->getScene().getFlags() & PxSceneFlag::eREQUIRE_RW_LOCK)
			{
				Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, 
					"An API read call (%s) was made from thread %d but PxScene::lockRead() was not called first, note that "
					"when PxSceneFlag::eREQUIRE_RW_LOCK is enabled all API reads and writes must be "
					"wrapped in the appropriate locks.", mName, PxU32(Ps::Thread::getId()));
			}
			else
			{
				Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, 
					"Overlapping API read and write call detected during %s from thread %d! Note that read operations to "
					"the SDK must not be overlapped with write calls, else the resulting behavior is undefined.", mName, PxU32(Ps::Thread::getId()));
			}
		}

		// Record the NpScene read/write error counter which is
		// incremented any time a NpScene::startWrite/startRead fails
		// (see destructor for additional error checking based on this count)
		mErrorCount = mScene->getReadWriteErrorCount();
	}
}


NpReadCheck::~NpReadCheck()
{
	if (mScene)
	{
		// By checking if the NpScene::mConcurrentErrorCount has been incremented
		// we can detect if an erroneous read/write was performed during 
		// this objects lifetime. In this case we also print this function's
		// details so that the user can see which two API calls overlapped
		if (mScene->getReadWriteErrorCount() != mErrorCount && !(mScene->getScene().getFlags() & PxSceneFlag::eREQUIRE_RW_LOCK))
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, 
				"Leaving %s on thread %d, an API overlapping write on another thread was detected.", mName, PxU32(Ps::Thread::getId()));
		}

		mScene->stopRead();
	}
}
