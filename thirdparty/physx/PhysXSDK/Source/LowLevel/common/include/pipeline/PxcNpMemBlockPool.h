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



#ifndef PXC_NP_MEM_BLOCK_POOL_H
#define PXC_NP_MEM_BLOCK_POOL_H

#include "PxvConfig.h"
#include "PsMutex.h"
#include "PxAssert.h"
#include "PsArray.h"
#include "PxcScratchAllocator.h"

namespace physx
{

struct PxcNpMemBlock
{
	enum
	{
		SIZE = 16384
	};
	PxU8 data[SIZE];
};

typedef Ps::Array<PxcNpMemBlock*> PxcNpMemBlockArray;

class PxcNpMemBlockPool
{
	PX_NOCOPY(PxcNpMemBlockPool)
public:
	PxcNpMemBlockPool(PxcScratchAllocator& allocator);
	~PxcNpMemBlockPool();

	void			init(PxU32 initial16KDataBlocks, PxU32 maxBlocks);
	void			flush();
	void			setBlockCount(PxU32 count);
	PxU32			getUsedBlockCount() const;
	PxU32			getMaxUsedBlockCount() const;
	PxU32			getPeakConstraintBlockCount() const;
	void			releaseUnusedBlocks();

	PxcNpMemBlock*	acquireConstraintBlock();
	PxcNpMemBlock*	acquireConstraintBlock(PxcNpMemBlockArray& memBlocks);
	PxcNpMemBlock*	acquireContactBlock();
	PxcNpMemBlock*	acquireFrictionBlock();
	PxcNpMemBlock*	acquireNpCacheBlock();

	PxU8*			acquireExceptionalConstraintMemory(PxU32 size);

#ifdef PX_PS3

	const PxcNpMemBlock* const* acquireMultipleConstraintBlocks(const PxU32 numRequestedBlocks, PxU32& acquiredBlockStart, PxU32& numAcquiredBlocks);
	const PxcNpMemBlock* const* acquireMultipleFrictionBlocks(const PxU32 numRequestedBlocks, PxU32& acquiredBlockStart, PxU32& numAcquiredBlocks);
	const PxcNpMemBlock* const* acquireMultipleCacheBlocks(const PxU32 numRequestedBlocks, PxU32& acquiredBlockStart, PxU32& numAcquiredBlocks);
	const PxcNpMemBlock* const* acquireMultipleContactBlocks(const PxU32 numRequestedBlocks, PxU32& acquiredBlockStart, PxU32& numAcquiredBlocks);

	void releaseMultipleConstraintBlocks(const PxU32 rangeStart, const PxU32 rangeEnd);
	void releaseMultipleFrictionBlocks(const PxU32 rangeStart, const PxU32 rangeEnd);
	void releaseMultipleCacheBlocks(const PxU32 rangeStart, const PxU32 rangeEnd);
	void releaseMultipleContactBlocks(const PxU32 rangeStart, const PxU32 rangeEnd);

	//acquire/releaseMultipleCacheBlocks and acquire/releaseMultipleContactBlocks are only ever called from a single thread per scene.  
	//As a consequence, releaseMultipleCacheBlocks and releaseMultipleContactBlocks can safely modify the corresponding block array sizes
	//because no subsequent call will attempt to release block ranges that will have been invalidated by the first call. 
	//In contrast, acquire/releaseMultipleConstraintBlocks and acquire/releaseMultipleFrictionBlocks can be called from multiple threads per scene.
	//As a consequence, it is not safe for releaseMultipleConstraintBlocks and releaseMultipleFrictionBlocks to modify the corresponding block 
	//array sizes because subsequent calls will receive block ranges that will have been invalidated by the first call.
	//The trick here is to set the elements of the relevant blocks to null in releaseMultipleConstraintBlocks and releaseMultipleFrictionBlocks and then 
	//to remove the null ranges in a subsequent function that is called from a single thread.
	void releaseNullConstraintBlocks();
	void releaseNullFrictionBlocks();

	void getSpuMemBlockCounters(PxU32& numNpContactStreamBlocks, PxU32& numNpCacheBlocks, PxU32& numDyFrictionBlocks, PxU32& numDyConstraintBlocks);

	/*
	 * KS - we now have a single buffer for constraints and a single buffer for contact stream. The memory is allocated "mixed"
	 * so there is no way of differentiating between memory allocated for contacts and joints (shaders). However, there are simulation
	 * statistics that tell us how much memory was required for contact constraints in total (in bytes) so this allows us to get an approximate
	 * overhead for the joints in the scene as the total constraint memory = (contacts constraints + shader constraints).
	 *
	 * We also now have separate buffers for NpCache and friction blocks. As such, we need another function to keep track of these buffers
	*/
	void updateSpuContactBlockCount();
	void updateSpuFrictionBlockCount();
	void updateSpuConstraintBlockCount();
	void updateSpuNpCacheBlockCount();
#endif

	void			acquireConstraintMemory();
	void			releaseConstraintMemory();
	void			releaseConstraintBlocks(PxcNpMemBlockArray& memBlocks);
	void			releaseContacts();
	void			swapFrictionStreams();
	void			swapNpCacheStreams();

	void			flushUnused();
	
private:


	Ps::Mutex				mLock;
	PxcNpMemBlockArray		mConstraints;
	PxcNpMemBlockArray		mContacts[2];
	PxcNpMemBlockArray		mFriction[2];
	PxcNpMemBlockArray		mNpCache[2];
	PxcNpMemBlockArray		mScratchBlocks;
	Ps::Array<PxU8*>		mExceptionalConstraints;

	PxcNpMemBlockArray		mUnused;

	PxU32					mNpCacheActiveStream;
	PxU32					mFrictionActiveStream;
	PxU32					mCCDCacheActiveStream;
	PxU32					mContactIndex;
	PxU32					mAllocatedBlocks;
	PxU32					mMaxBlocks;
	PxU32					mInitialBlocks;
	PxU32					mUsedBlocks;
	PxU32					mMaxUsedBlocks;
	PxcNpMemBlock*			mScratchBlockAddr;
	PxU32					mNbScratchBlocks;
	PxcScratchAllocator&	mScratchAllocator;

	PxU32					mPeakConstraintAllocations;
	PxU32					mConstraintAllocations;

	PxcNpMemBlock*	acquire(PxcNpMemBlockArray& trackingArray, PxU32* allocationCount = NULL, PxU32* peakAllocationCount = NULL, bool isScratchAllocation = false);
	void			release(PxcNpMemBlockArray& deadArray, PxU32* allocationCount = NULL);

#ifdef PX_PS3
	PxU32			mMaxSpuContactBlocks;
	PxU32			mCurrentSpuContactBlocks;
	PxU32			mMaxSpuFrictionBlocks;
	PxU32			mMaxSpuConstraintBlocks;
	PxU32			mMaxSpuNpCacheBlocks;
	void			acquireMultipleBlocks(const PxU32 numRequestedBlocks, PxcNpMemBlockArray& mConstraints, PxU32& numAcquiredBlocks);
	void			releaseMultipleBlocks(PxcNpMemBlockArray& deadArray, const PxU32 rangeStart, const PxU32 rangeEnd);
	void			releaseNullBlocks(PxcNpMemBlockArray& deadArray);
#endif
};

}

#endif
