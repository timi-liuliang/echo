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


#ifndef PXS_CONTACTCACHE_H
#define PXS_CONTACTCACHE_H

#include "PxvConfig.h"
#include "CmBitMap.h"
#include "PxTransform.h"
#include "CmMatrix34.h"
#include "PxcThreadCoherantCache.h"
#include "PxsThresholdTable.h"
#include "PxcSolverBody.h"
#include "PsAllocator.h"
#include "PsAlignedMalloc.h"
#include "PxcNpThreadContext.h"
#include "PxcSolverConstraintDesc.h"
#include "PxvDynamics.h"
#include "PxcArticulation.h"

namespace physx
{
class PxsRigidBody;
struct PxcSolverBody;
class PxsSolverBodyPool;
class PxsContactManager;
struct PxsIndexedContactManager;

/*!
Cache information specific to the software implementation(non common).

See PxcgetThreadContext.

Not thread-safe, so remember to have one object per thread!

TODO! refactor this and rename(it is a general per thread cache). Move transform cache into its own class.
*/
class PxsThreadContext : 
	public PxcThreadCoherantCache<PxsThreadContext>::EntryBase, public PxcNpThreadContext
{
	PX_NOCOPY(PxsThreadContext)
public:

	// PX_ENABLE_SIM_STATS
	struct ThreadSimStats
	{
		void clear()
		{
#if PX_ENABLE_SIM_STATS
			numActiveConstraints = 0;
			numActiveDynamicBodies = 0;
			numActiveKinematicBodies = 0;
			numAxisSolverConstraints = 0;
#endif
		}

		PxU32 numActiveConstraints;
		PxU32 numActiveDynamicBodies;
		PxU32 numActiveKinematicBodies;
		PxU32 numAxisSolverConstraints;

	};

	//TODO: tune cache size based on number of active objects.
	PxsThreadContext(PxsContext *context);
	void reset(PxU32 shapeCount, PxU32 cmCount);
	void resizeArrays(PxU32 bodyCount, PxU32 cmCount, PxU32 contactConstraintDescCount, PxU32 frictionConstraintDescCount, PxU32 articulationCount);


	PX_FORCE_INLINE void										addLocalNewTouchCount(PxU32 newTouchCMCount)	{ mLocalNewTouchCount += newTouchCMCount;	}
	PX_FORCE_INLINE void										addLocalLostTouchCount(PxU32 lostTouchCMCount)	{ mLocalLostTouchCount += lostTouchCMCount;	}
	PX_FORCE_INLINE PxU32										getLocalNewTouchCount()					const	{ return mLocalNewTouchCount;				}
	PX_FORCE_INLINE PxU32										getLocalLostTouchCount()				const	{ return mLocalLostTouchCount;				}

	PX_FORCE_INLINE Cm::BitMap&									getLocalChangeTouch()							{ return mLocalChangeTouch;					}
	PX_FORCE_INLINE Cm::BitMap&									getLocalChangedActors()							{ return mLocalChangedActors;				}

	PX_FORCE_INLINE	Ps::Array<PxcArticulationSolverDesc>&		getArticulations()								{ return mArticulations;					}
	PX_FORCE_INLINE PxsThresholdStream&							getThresholdStream()							{ return mThresholdStream;					}
	PX_FORCE_INLINE void										setCreateContactStream(bool to)					{ mCreateContactStream = to;				}



	// PX_ENABLE_SIM_STATS
	PX_FORCE_INLINE ThreadSimStats& getSimStats()
	{
		return mThreadSimStats;
	}


	// this stuff is just used for reformatting the solver data. Hopefully we should have a more
	// sane format for this when the dust settles - so it's just temporary. If we keep this around
	// here we should move these from public to private

	//PX_ALIGN(16, PxU32					mConstraintsPerPartition[33]);
	PxU32 mSuccessfulSpuConstraintPartitionCount;
	PxU32 mNumDifferentBodyConstraints;
	PxU32 mNumDifferentBodyFrictionConstraints;
	PxU32 mNumSelfConstraints;
	PxU32 mNumSelfFrictionConstraints;
	PxU32 mNumSelfConstraintBlocks;
	PxU32 mNumSelfConstraintFrictionBlocks;

	Ps::Array<PxU32>					mConstraintsPerPartition;
	Ps::Array<PxU32>					mFrictionConstraintsPerPartition;
	Ps::Array<PxU32>					mPartitionNormalizationBitmap;
	Ps::Array<PxsBodyCore*>				bodyCoreArray;
	Ps::Array<Cm::SpatialVector>		accelerationArray;
	Ps::Array<Cm::SpatialVector>		motionVelocityArray;
	Ps::Array<PxcSolverConstraintDesc>	contactConstraintDescArray;
	Ps::Array<PxcSolverConstraintDesc>	tempConstraintDescArray;
	Ps::Array<PxcSolverConstraintDesc>	frictionConstraintDescArray;
	Ps::Array<PxcSolverConstraintDesc>	orderedContactConstraints;
	Ps::Array<PxsConstraintBatchHeader> contactConstraintBatchHeaders;
	Ps::Array<PxsConstraintBatchHeader> frictionConstraintBatchHeaders;
	Ps::Array<PxsCompoundContactManager> compoundConstraints;
	Ps::Array<const PxsIndexedContactManager*> orderedContactList;
	Ps::Array<const PxsIndexedContactManager*> tempContactList;
	Ps::Array<PxU32>					sortIndexArray;

	PxU32								numDifferentBodyBatchHeaders;
	PxU32								numSelfConstraintBatchHeaders;

	
	PxU32								mOrderedContactDescCount;
	PxU32								mOrderedFrictionDescCount;

	PxU32 mAxisConstraintCount;
	bool mSuccessfulSpuConstraintPartition;
	PxcFsSelfConstraintBlock* mSelfConstraintBlocks;
	
	PxcFsSelfConstraintBlock* mSelfConstraintFrictionBlocks;
	
	
	

	PxU32 mMaxPartitions;
	PxU32 mMaxFrictionPartitions;
	PxU32 mMaxSolverPositionIterations;
	PxU32 mMaxSolverVelocityIterations;
	PxU32 mThresholdPairCount;
	PxU32 mMaxArticulationLength;
	
	PxcSolverConstraintDesc* mContactDescPtr;
	PxcSolverConstraintDesc* mStartContactDescPtr;
	PxcSolverConstraintDesc* mFrictionDescPtr;

	PxI32 mSolverProgressCounters[8];
	

private:

	// Solver caches

	// first two of these are transient, we should just temp allocate them
	PxsThresholdStream						mThresholdStream;

	Ps::Array<PxcArticulationSolverDesc>	mArticulations;

	// change touch handling.
	Cm::BitMap mLocalChangeTouch;
	PxU32 mLocalNewTouchCount;
	PxU32 mLocalLostTouchCount;

	//Shapes changed handling(keeps track of which shapes and therefore their volumes changed within a particular thread).
	Cm::BitMap mLocalChangedActors;

	// PX_ENABLE_SIM_STATS
	ThreadSimStats				mThreadSimStats;


};

}

#endif
