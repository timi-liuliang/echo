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


#ifndef NP_WRITE_CHECK_H
#define NP_WRITE_CHECK_H

#include "PxSimpleTypes.h"

namespace physx
{

class NpScene;

// RAII wrapper around the PxScene::startWrite() method, note that this
// object does not acquire any scene locks, it is an error checking only mechanism
class NpWriteCheck
{
public: 
	NpWriteCheck(NpScene* scene, const char* functionName, bool allowReentry=true);
	~NpWriteCheck();

private:

	NpScene* mScene;
	const char* mName;
	bool mAllowReentry;
	PxU32 mErrorCount;
};

#if defined(PX_DEBUG) || defined(PX_CHECKED)
	// Creates a scoped write check object that detects whether appropriate scene locks
	// have been acquired and checks if reads/writes overlap, this macro should typically
	// be placed at the beginning of any non-const API methods that are not multi-thread safe. 
	// By default re-entrant  write calls by the same thread are allowed, the error conditions 
	// checked can be summarized as:
	
	// 1. PxSceneFlag::eREQUIRE_RW_LOCK was specified but PxScene::lockWrite() was not yet called
	// 2. Other threads were already reading, or began reading during the object lifetime
	// 3. Other threads were already writing, or began writing during the object lifetime
	#define NP_WRITE_CHECK(npScenePtr) NpWriteCheck npWriteCheck(npScenePtr, __FUNCTION__);

	// Creates a scoped write check object that disallows re-entrant writes, this is used by
	// the NpScene::simulate method to detect when callbacks make write calls to the API
	#define NP_WRITE_CHECK_NOREENTRY(npScenePtr) NpWriteCheck npWriteCheck(npScenePtr, __FUNCTION__, false);
#else
	#define NP_WRITE_CHECK(npScenePtr)
	#define NP_WRITE_CHECK_NOREENTRY(npScenePtr)
#endif

}

#endif // NP_WRITE_CHECK_H
