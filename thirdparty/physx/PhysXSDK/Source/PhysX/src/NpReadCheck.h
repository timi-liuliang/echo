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


#ifndef NP_READ_CHECK_H
#define NP_READ_CHECK_H

#include "PxSimpleTypes.h"

namespace physx
{

class NpScene;

// RAII wrapper around the PxScene::startRead() method, note that this
// object does not acquire any scene locks, it is an error checking only mechanism
class NpReadCheck
{
public: 
	NpReadCheck(const NpScene* scene, const char* functionName);
	~NpReadCheck();
private:
	const NpScene* mScene;
	const char* mName;
	PxU32 mErrorCount;
};

#if !PX_IS_SPU && (defined(PX_DEBUG) || defined(PX_CHECKED))
	// Creates a scoped read check object that detects whether appropriate scene locks
	// have been acquired and checks if reads/writes overlap, this macro should typically
	// be placed at the beginning of any const API methods that are not multi-thread safe, 
	// the error conditions checked can be summarized as:
	
	// 1. PxSceneFlag::eREQUIRE_RW_LOCK was specified but PxScene::lockRead() was not yet called
	// 2. Other threads were already writing, or began writing during the object lifetime
	#define NP_READ_CHECK(npScenePtr) NpReadCheck npReadCheck(static_cast<const NpScene*>(npScenePtr), __FUNCTION__);
#else
	#define NP_READ_CHECK(npScenePtr)
#endif

}

#endif // NP_Read_CHECK_H
