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

#include "PxcConstraintBlockStream.h"
#include "PxcNpThreadContext.h"

using namespace physx;

PxcNpThreadContext::PxcNpThreadContext(PxReal meshContactMargin, PxReal correlationDist, PxReal toleranceLength, const Cm::RenderOutput& renderOutput, PxcNpMemBlockPool& memBlockPool, bool createContacts) : 
	mRenderOutput			(renderOutput),
	mConstraintBlockManager	(memBlockPool),
	mConstraintBlockStream	(memBlockPool),
	mContactBlockStream		(memBlockPool),
	mFrictionPatchStreamPair(memBlockPool),
	mNpCacheStreamPair		(memBlockPool),
	mFrictionType			(PxFrictionType::ePATCH),
	mPCM					(false),
	mContactCache			(false),
	mCreateContactStream	(createContacts),
	mCreateAveragePoint		(false),
	mCompressedCacheSize	(0),
	mConstraintSize			(0),
	mToleranceLength		(toleranceLength),
	mCorrelationDistance	(correlationDist)
{
	mContactBuffer.meshContactMargin	= meshContactMargin;

#if PX_ENABLE_SIM_STATS
	clearStats();
#endif
}

PxcNpThreadContext::~PxcNpThreadContext()
{
}

#if PX_ENABLE_SIM_STATS
void PxcNpThreadContext::clearStats()
{
	PxMemSet(discreteContactPairs, 0, sizeof(discreteContactPairs));
	mCompressedCacheSize = 0;
	mConstraintSize = 0;
}
#endif
