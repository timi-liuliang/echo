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


#include "PxsThreadContext.h"
#include "PxsContext.h"

using namespace physx;

PxsThreadContext::PxsThreadContext(PxsContext* context):
	PxcNpThreadContext(	context->getMeshContactMargin(),
						context->getCorrelationDistance(),
						context->getToleranceLength(),
						context->getRenderOutput(),
						context->getNpMemBlockPool(),
						context->getCreateContactStream()),
					   mNumDifferentBodyConstraints(0),
	mNumSelfConstraints(0),
	
	mNumSelfConstraintBlocks(0),
	mConstraintsPerPartition(PX_DEBUG_EXP("PxsThreadContext::mConstraintsPerPartition")),
	mFrictionConstraintsPerPartition(PX_DEBUG_EXP("PxsThreadContext::frictionsConstraintsPerPartition")),
	mPartitionNormalizationBitmap(PX_DEBUG_EXP("PxsThreadContext::mPartitionNormalizationBitmap")),
	bodyCoreArray(PX_DEBUG_EXP("PxsThreadContext::bodyCoreArray")),
	accelerationArray(PX_DEBUG_EXP("PxsThreadContext::accelerationArray")),
	motionVelocityArray(PX_DEBUG_EXP("PxsThreadContext::motionVelocityArray")),
	contactConstraintDescArray(PX_DEBUG_EXP("PxsThreadContext::solverContactConstraintArray")),
	tempConstraintDescArray(PX_DEBUG_EXP("PxsThreadContext::tempContactConstraintArray")),
	frictionConstraintDescArray(PX_DEBUG_EXP("PxsThreadContext::solverFrictionConstraintArray")),
	orderedContactConstraints(PX_DEBUG_EXP("PxsThreadContext::orderedContactContraintArray")),
	//orderedFrictionConstraints(PX_DEBUG_EXP("PxsThreadContext::orderedFrictionContraintArray")),
	contactConstraintBatchHeaders(PX_DEBUG_EXP("PxsThreadContext::contactConstraintBatchHeaders")),
	frictionConstraintBatchHeaders(PX_DEBUG_EXP("PxsThreadContext::frictionConstraintBatchHeaders")),
	compoundConstraints(PX_DEBUG_EXP("PxsThreadContext::compoundConstraints")),
	orderedContactList(PX_DEBUG_EXP("PxsThreadContext::orderedContactList")),
	tempContactList(PX_DEBUG_EXP("PxsThreadContext::tempContactList")),
	sortIndexArray(PX_DEBUG_EXP("PxsThreadContext::sortIndexArray")),
	mAxisConstraintCount(0),
	mSuccessfulSpuConstraintPartition(false),
	mSelfConstraintBlocks(NULL),
	mMaxPartitions(0),
	mMaxSolverPositionIterations(0),
	mMaxSolverVelocityIterations(0),
	mThresholdPairCount(0),
	mMaxArticulationLength(0),
	mContactDescPtr(NULL),
	mFrictionDescPtr(NULL),
	mArticulations(PX_DEBUG_EXP("PxsThreadContext::articulations")),
	mLocalNewTouchCount(0), 
	mLocalLostTouchCount(0)
  {
#if PX_ENABLE_SIM_STATS
	  mThreadSimStats.clear();
#endif
	  //Defaulted to have space for 16384 bodies
	  mPartitionNormalizationBitmap.reserve(512); 
	  //Defaulted to have space for 128 partitions (should be more-than-enough)
	  mConstraintsPerPartition.reserve(128);
  }

void PxsThreadContext::resizeArrays(PxU32 bodyCount, PxU32 cmCount, PxU32 contactConstraintDescCount, PxU32 frictionConstraintDescCount, PxU32 articulationCount)
{
	// resize resizes smaller arrays to the exact target size, which can generate a lot of churn

	bodyCoreArray.forceSize_Unsafe(0);
	bodyCoreArray.reserve(PxMax<PxU32>(Ps::nextPowerOfTwo(bodyCount), 64));
	bodyCoreArray.forceSize_Unsafe(bodyCount);

	accelerationArray.forceSize_Unsafe(0);
	accelerationArray.reserve((bodyCount+63)&~63);
	accelerationArray.forceSize_Unsafe(bodyCount);

	motionVelocityArray.forceSize_Unsafe(0);
	motionVelocityArray.reserve((bodyCount+63)&~63);
	motionVelocityArray.forceSize_Unsafe(bodyCount);

	contactConstraintDescArray.forceSize_Unsafe(0);
	contactConstraintDescArray.reserve((contactConstraintDescCount+63)&~63);

	orderedContactConstraints.forceSize_Unsafe(0);
	orderedContactConstraints.reserve((contactConstraintDescCount + 63) & ~63);

	tempConstraintDescArray.forceSize_Unsafe(0);
	tempConstraintDescArray.reserve((contactConstraintDescCount + 63) & ~63);

	frictionConstraintDescArray.forceSize_Unsafe(0);
	frictionConstraintDescArray.reserve((frictionConstraintDescCount+63)&~63);

	mThresholdStream.forceSize_Unsafe(0);
	mThresholdStream.reserve(PxMax<PxU32>(Ps::nextPowerOfTwo(cmCount), 64));
	mThresholdStream.forceSize_Unsafe(cmCount);

	mArticulations.forceSize_Unsafe(0);
	mArticulations.reserve(PxMax<PxU32>(Ps::nextPowerOfTwo(articulationCount), 16));
	mArticulations.forceSize_Unsafe(articulationCount);

	mContactDescPtr = contactConstraintDescArray.begin();
	mFrictionDescPtr = frictionConstraintDescArray.begin();
}

void PxsThreadContext::reset(PxU32 shapeCount, PxU32 cmCount)
{
	// TODO: move these to the PxcNpThreadContext
	mFrictionPatchStreamPair.reset();
	mConstraintBlockStream.reset();
	mContactBlockStream.reset();
	mNpCacheStreamPair.reset();

	mLocalChangeTouch.clear();
	mLocalChangeTouch.resize(cmCount);
	mLocalNewTouchCount = 0;
	mLocalLostTouchCount = 0;

	mLocalChangedActors.clear();
	mLocalChangedActors.resize(shapeCount);

	mContactDescPtr = contactConstraintDescArray.begin();
	mFrictionDescPtr = frictionConstraintDescArray.begin();

	mThresholdPairCount = 0;
	mAxisConstraintCount = 0;
	mMaxSolverPositionIterations = 0;
	mMaxSolverVelocityIterations = 0;
	mSuccessfulSpuConstraintPartition = false;
	mNumDifferentBodyConstraints = 0;
	mNumSelfConstraints = 0;
	mSelfConstraintBlocks = NULL;
	mNumSelfConstraintBlocks = 0;

	orderedContactConstraints.forceSize_Unsafe(0);
}

