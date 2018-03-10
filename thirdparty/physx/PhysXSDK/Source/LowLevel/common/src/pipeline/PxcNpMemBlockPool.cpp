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

#include "foundation/PxPreprocessor.h"
#include "PxcNpMemBlockPool.h"
#include "PsUserAllocated.h"

using namespace physx;

PxcNpMemBlockPool::PxcNpMemBlockPool(PxcScratchAllocator& allocator):
  mNpCacheActiveStream(0),
  mFrictionActiveStream(0),
  mCCDCacheActiveStream(0),
  mContactIndex(0),
  mAllocatedBlocks(0),
  mMaxBlocks(0),
  mUsedBlocks(0),
  mMaxUsedBlocks(0),
  mScratchBlockAddr(0),
  mNbScratchBlocks(0),
  mScratchAllocator(allocator),
  mPeakConstraintAllocations(0),
  mConstraintAllocations(0)
#ifdef PX_PS3
  ,mMaxSpuContactBlocks(0)
  ,mMaxSpuFrictionBlocks(0)
  ,mMaxSpuConstraintBlocks(0)
  ,mMaxSpuNpCacheBlocks(0)
#endif
{
}

void PxcNpMemBlockPool::init(PxU32 initialBlockCount, PxU32 maxBlocks)
{
	mMaxBlocks = maxBlocks;
	mInitialBlocks = initialBlockCount;

	PxU32 reserve = PxMax<PxU32>(initialBlockCount, 64);

	mConstraints.reserve(reserve);
	mExceptionalConstraints.reserve(16);

	mFriction[0].reserve(reserve);
	mFriction[1].reserve(reserve);
	mNpCache[0].reserve(reserve);
	mNpCache[1].reserve(reserve);
	mUnused.reserve(reserve);

	setBlockCount(initialBlockCount);
}

PxU32 PxcNpMemBlockPool::getUsedBlockCount() const
{
	return mUsedBlocks;
}

PxU32 PxcNpMemBlockPool::getMaxUsedBlockCount() const
{
	return mMaxUsedBlocks;
}

PxU32 PxcNpMemBlockPool::getPeakConstraintBlockCount() const
{
	return mPeakConstraintAllocations;
}


void PxcNpMemBlockPool::setBlockCount(PxU32 blockCount)
{
	Ps::Mutex::ScopedLock lock(mLock);
	PxU32 current = getUsedBlockCount();
	for(PxU32 i=current;i<blockCount;i++)
	{
		mUnused.pushBack(reinterpret_cast<PxcNpMemBlock *>(PX_ALLOC(PxcNpMemBlock::SIZE, PX_DEBUG_EXP("PxcNpMemBlock"))));
		mAllocatedBlocks++;
	}
}

void PxcNpMemBlockPool::releaseUnusedBlocks()
{
	Ps::Mutex::ScopedLock lock(mLock);
	while(mUnused.size())
	{
		PX_FREE(mUnused.popBack());
		mAllocatedBlocks--;
	}
}


PxcNpMemBlockPool::~PxcNpMemBlockPool()
{
	// swapping twice guarantees all blocks are released from the stream pairs
	swapFrictionStreams();
	swapFrictionStreams();

	swapNpCacheStreams();
	swapNpCacheStreams();

	releaseConstraintMemory();
	releaseContacts();
	releaseContacts();

	PX_ASSERT(mUsedBlocks == 0);

	flushUnused();
}

void PxcNpMemBlockPool::acquireConstraintMemory()
{
	PxU32 size;
	void* addr = mScratchAllocator.allocAll(size);
	size = size&~(PxcNpMemBlock::SIZE-1);

	PX_ASSERT(mScratchBlocks.size()==0);
	mScratchBlockAddr = reinterpret_cast<PxcNpMemBlock*>(addr);
	mNbScratchBlocks =  size/PxcNpMemBlock::SIZE;

	mScratchBlocks.resize(mNbScratchBlocks);
	for(PxU32 i=0;i<mNbScratchBlocks;i++)
		mScratchBlocks[i] = mScratchBlockAddr+i;
}

void PxcNpMemBlockPool::releaseConstraintMemory()
{
	Ps::Mutex::ScopedLock lock(mLock);

	mPeakConstraintAllocations = mConstraintAllocations = 0;
	
	while(mConstraints.size())
	{
		PxcNpMemBlock* block = mConstraints.popBack();
		if(mScratchAllocator.isScratchAddr(block))
			mScratchBlocks.pushBack(block);
		else
		{
			mUnused.pushBack(block);
			PX_ASSERT(mUsedBlocks>0);
			mUsedBlocks--;
		}
	}

	for(PxU32 i=0;i<mExceptionalConstraints.size();i++)
		PX_FREE(mExceptionalConstraints[i]);
	mExceptionalConstraints.clear();

	PX_ASSERT(mScratchBlocks.size()==mNbScratchBlocks); // check we released them all
	mScratchBlocks.clear();

	if(mScratchBlockAddr)
	{
		mScratchAllocator.free(mScratchBlockAddr);
		mScratchBlockAddr = 0;
		mNbScratchBlocks = 0;
	}
}


PxcNpMemBlock* PxcNpMemBlockPool::acquire(PxcNpMemBlockArray& trackingArray, PxU32* allocationCount, PxU32* peakAllocationCount, bool isScratchAllocation)
{
	Ps::Mutex::ScopedLock lock(mLock);
	if(allocationCount && peakAllocationCount)
	{
		*peakAllocationCount = PxMax(*allocationCount + 1, *peakAllocationCount);
		(*allocationCount)++;
	}

	// this is a bit of hack - the logic would be better placed in acquireConstraintBlock, but then we'd have to grab the mutex
	// once there to check the scratch block array and once here if we fail - or, we'd need a larger refactor to separate out
	// locking and acquisition.

	if(isScratchAllocation && mScratchBlocks.size()>0)
	{
		PxcNpMemBlock* block = mScratchBlocks.popBack();
		trackingArray.pushBack(block);
		return block;
	}

	
	if(mUnused.size())
	{
		PxcNpMemBlock* block = mUnused.popBack();
		trackingArray.pushBack(block);
		mMaxUsedBlocks = PxMax<PxU32>(mUsedBlocks+1, mMaxUsedBlocks);
		mUsedBlocks++;
		return block;
	}	


	if(mAllocatedBlocks == mMaxBlocks)
	{
#ifdef PX_CHECKED
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
				"Reached maximum number of allocated blocks so 16k block allocation will fail!");
#endif
		return NULL;
	}

#ifdef PX_CHECKED
	if(mInitialBlocks)
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"Number of required 16k memory blocks has exceeded the initial number of blocks. Allocator is being called. Consider increasing the number of pre-allocated 16k blocks.");
	}
#endif

	// increment here so that if we hit the limit in separate threads we won't overallocated
	mAllocatedBlocks++;
	
	PxcNpMemBlock* block = reinterpret_cast<PxcNpMemBlock*>(PX_ALLOC(sizeof(PxcNpMemBlock), PX_DEBUG_EXP("PxcNpMemBlock")));

	if(block)
	{
		trackingArray.pushBack(block);
		mMaxUsedBlocks = PxMax<PxU32>(mUsedBlocks+1, mMaxUsedBlocks);
		mUsedBlocks++;
	}
	else
		mAllocatedBlocks--;

	return block;
}

PxU8* PxcNpMemBlockPool::acquireExceptionalConstraintMemory(PxU32 size)
{
	PxU8* memory = reinterpret_cast<PxU8*>(PX_ALLOC(size, PX_DEBUG_EXP("PxcNpExceptionalMemory")));
	if(memory)
	{
		Ps::Mutex::ScopedLock lock(mLock);
		mExceptionalConstraints.pushBack(memory);
	}
	return memory;
}

void PxcNpMemBlockPool::release(PxcNpMemBlockArray& deadArray, PxU32* allocationCount)
{
	Ps::Mutex::ScopedLock lock(mLock);
	PX_ASSERT(mUsedBlocks >= deadArray.size());
	mUsedBlocks -= deadArray.size();
	if(allocationCount)
	{
		*allocationCount -= deadArray.size();
	}
	while(deadArray.size())
	{
		PxcNpMemBlock* block = deadArray.popBack();
		for(PxU32 a = 0; a < mUnused.size(); ++a)
		{
			PX_ASSERT(mUnused[a] != block);
		}
		mUnused.pushBack(block);
	}
}

void PxcNpMemBlockPool::flushUnused()
{
	while(mUnused.size())
		PX_FREE(mUnused.popBack());
}


PxcNpMemBlock* PxcNpMemBlockPool::acquireConstraintBlock()
{
	// we track the scratch blocks in the constraint block array, because the code in acquireMultipleConstraintBlocks
	// assumes that acquired blocks are listed there.

	return acquire(mConstraints);
}

PxcNpMemBlock* PxcNpMemBlockPool::acquireConstraintBlock(PxcNpMemBlockArray& memBlocks)
{
	return acquire(memBlocks, &mConstraintAllocations, &mPeakConstraintAllocations, true);
}

PxcNpMemBlock* PxcNpMemBlockPool::acquireContactBlock()
{
	return acquire(mContacts[mContactIndex], NULL, NULL, true);
}


void PxcNpMemBlockPool::releaseConstraintBlocks(PxcNpMemBlockArray& memBlocks)
{
	Ps::Mutex::ScopedLock lock(mLock);
	
	while(memBlocks.size())
	{
		PxcNpMemBlock* block = memBlocks.popBack();
		if(mScratchAllocator.isScratchAddr(block))
			mScratchBlocks.pushBack(block);
		else
		{
			mUnused.pushBack(block);
			PX_ASSERT(mUsedBlocks>0);
			mUsedBlocks--;
		}
	}
}

void PxcNpMemBlockPool::releaseContacts()
{
	//releaseConstraintBlocks(mContacts);
	release(mContacts[1-mContactIndex]);
	mContactIndex = 1-mContactIndex;
}

PxcNpMemBlock* PxcNpMemBlockPool::acquireFrictionBlock()
{
	return acquire(mFriction[mFrictionActiveStream]);
}

void PxcNpMemBlockPool::swapFrictionStreams()
{
	release(mFriction[1-mFrictionActiveStream]);
	mFrictionActiveStream = 1-mFrictionActiveStream;
}

PxcNpMemBlock* PxcNpMemBlockPool::acquireNpCacheBlock()
{
	return acquire(mNpCache[mNpCacheActiveStream]);
}

void PxcNpMemBlockPool::swapNpCacheStreams()
{
	release(mNpCache[1-mNpCacheActiveStream]);
	mNpCacheActiveStream = 1-mNpCacheActiveStream;
}

#ifdef PX_PS3
const PxcNpMemBlock* const* PxcNpMemBlockPool::acquireMultipleConstraintBlocks(const PxU32 numRequestedBlocks, PxU32& acquiredBlockStart, PxU32& numAcquiredBlocks)
{
	Ps::Mutex::ScopedLock lock(mLock);
	if(0==numRequestedBlocks) return NULL;
	acquiredBlockStart = mConstraints.size();
	acquireMultipleBlocks(numRequestedBlocks, mConstraints, numAcquiredBlocks);
	return numAcquiredBlocks ? &mConstraints[acquiredBlockStart] : NULL;
}

void PxcNpMemBlockPool::releaseMultipleConstraintBlocks(const PxU32 rangeStart, const PxU32 rangeEnd)
{
	Ps::Mutex::ScopedLock lock(mLock);
	if(rangeEnd > rangeStart)
	{
		releaseMultipleBlocks(mConstraints,rangeStart,rangeEnd);
	}
}

void PxcNpMemBlockPool::releaseNullConstraintBlocks()
{
	releaseNullBlocks(mConstraints);
}

const PxcNpMemBlock* const* PxcNpMemBlockPool::acquireMultipleFrictionBlocks(const PxU32 numRequestedBlocks, PxU32& acquiredBlockStart, PxU32& numAcquiredBlocks)
{
	Ps::Mutex::ScopedLock lock(mLock);
	acquiredBlockStart = mFriction[mFrictionActiveStream].size();
	acquireMultipleBlocks(numRequestedBlocks, mFriction[mFrictionActiveStream], numAcquiredBlocks);
	return numAcquiredBlocks ? &mFriction[mFrictionActiveStream][acquiredBlockStart] : NULL;
}

void PxcNpMemBlockPool::releaseMultipleFrictionBlocks(const PxU32 rangeStart, const PxU32 rangeEnd)
{
	Ps::Mutex::ScopedLock lock(mLock);
	if(rangeEnd > rangeStart)
	{
		releaseMultipleBlocks(mFriction[mFrictionActiveStream],rangeStart,rangeEnd);
	}
}

void PxcNpMemBlockPool::releaseNullFrictionBlocks()
{
	releaseNullBlocks(mFriction[mFrictionActiveStream]);
}

const PxcNpMemBlock* const* PxcNpMemBlockPool::acquireMultipleCacheBlocks(const PxU32 numRequestedBlocks, PxU32& acquiredBlockStart, PxU32& numAcquiredBlocks)
{
	Ps::Mutex::ScopedLock lock(mLock);
	acquiredBlockStart=mNpCache[mNpCacheActiveStream].size();
	acquireMultipleBlocks(numRequestedBlocks, mNpCache[mNpCacheActiveStream], numAcquiredBlocks);
	return numAcquiredBlocks ? &mNpCache[mNpCacheActiveStream][acquiredBlockStart] : NULL;
}

void PxcNpMemBlockPool::releaseMultipleCacheBlocks(const PxU32 rangeStart, const PxU32 rangeEnd)
{
	Ps::Mutex::ScopedLock lock(mLock);
	if(rangeEnd > rangeStart)
	{
		releaseMultipleBlocks(mNpCache[mNpCacheActiveStream],rangeStart,rangeEnd);
		mNpCache[mNpCacheActiveStream].removeRange(rangeStart, rangeEnd - rangeStart);
	}
}

const PxcNpMemBlock* const* PxcNpMemBlockPool::acquireMultipleContactBlocks(const PxU32 numRequestedBlocks, PxU32& acquiredBlockStart, PxU32& numAcquiredBlocks)
{
	Ps::Mutex::ScopedLock lock(mLock);
	if(0==numRequestedBlocks) return NULL;
	acquiredBlockStart = mContacts[mContactIndex].size();
	acquireMultipleBlocks(numRequestedBlocks, mContacts[mContactIndex], numAcquiredBlocks);
	return mContacts[mContactIndex].size() ? &mContacts[mContactIndex][acquiredBlockStart] : NULL;
}

void PxcNpMemBlockPool::releaseMultipleContactBlocks(const PxU32 rangeStart, const PxU32 rangeEnd)
{
	Ps::Mutex::ScopedLock lock(mLock);
	if(rangeEnd > rangeStart)
	{
		releaseMultipleBlocks(mContacts[mContactIndex],rangeStart,rangeEnd);
		mContacts[mContactIndex].removeRange(rangeStart, rangeEnd - rangeStart);
	}
}

void PxcNpMemBlockPool::acquireMultipleBlocks(const PxU32 numRequestedBlocks, PxcNpMemBlockArray& trackingArray, PxU32& numAcquiredBlocks)
{
	numAcquiredBlocks = 0;
	{
		while(numAcquiredBlocks < numRequestedBlocks)
		{
			if(acquire(trackingArray))
			{
				numAcquiredBlocks++;
			}
			else
			{
				break;
			}
		}
		mMaxUsedBlocks = PxMax<PxU32>(mMaxUsedBlocks, mUsedBlocks);
	}	
}

void PxcNpMemBlockPool::releaseMultipleBlocks(PxcNpMemBlockArray& deadArray, const PxU32 rangeStart, const PxU32 rangeEnd)
{
	PX_ASSERT(rangeEnd > rangeStart);
	PX_ASSERT(deadArray.size() >= rangeEnd);

	for(PxU32 i = rangeStart; i< rangeEnd; i++)
	{
		PxcNpMemBlock* block = deadArray[i];
		if(block>=mScratchBlockAddr && block < mScratchBlockAddr+mNbScratchBlocks)
			mScratchBlocks.pushBack(block);
		else
		{
			mUnused.pushBack(block);
			PX_ASSERT(mUsedBlocks>0);
			mUsedBlocks--;
		}
		deadArray[i] = NULL;
	}
}

void PxcNpMemBlockPool::releaseNullBlocks(PxcNpMemBlockArray& deadArray)
{
	Ps::InlineArray<PxU32, 8> deadZoneStarts;
	Ps::InlineArray<PxU32, 8> deadZoneEnds;

	//Compute the dead zones.
	{
		bool seekingDeadZoneStart = true;
		const PxU32 N = deadArray.size();
		for(PxU32 i = 0; i < N; i++)
		{
			if(NULL == deadArray[i] && seekingDeadZoneStart)
			{
				deadZoneStarts.pushBack(i);
				deadZoneEnds.pushBack(N);
				seekingDeadZoneStart = false;
			}

			if(NULL != deadArray[i] && !seekingDeadZoneStart)
			{
				deadZoneEnds[deadZoneEnds.size()-1] = i;
				seekingDeadZoneStart = true;
			}
		}
	}

	//Remove the dead zone ranges
	{
		const PxU32 N = deadZoneStarts.size();
		for(PxU32 i = 0; i < N; i++)
		{
			deadArray.removeRange(deadZoneStarts[N - 1 - i], deadZoneEnds[N - 1 - i] - deadZoneStarts[N - 1 - i]);
		}
	}
}

void PxcNpMemBlockPool::getSpuMemBlockCounters(PxU32& numNpContactStreamBlocks, PxU32& numNpCacheBlocks, PxU32& numDyFrictionBlocks, PxU32& numDyConstraintBlocks)
{
	numNpContactStreamBlocks = mMaxSpuContactBlocks;
	numDyFrictionBlocks = mMaxSpuFrictionBlocks;
	numDyConstraintBlocks = mMaxSpuConstraintBlocks;
	numNpCacheBlocks = mMaxSpuNpCacheBlocks;
}

void PxcNpMemBlockPool::updateSpuNpCacheBlockCount()
{
	// PPU uses npCache blocks whereas SPU uses friction blocks for the same data
	// adding up the two leads to a better guess
	PxU32 newNbNpCacheBlocks = mNpCache[mNpCacheActiveStream].size();
	mMaxSpuNpCacheBlocks = PxMax(mMaxSpuNpCacheBlocks,newNbNpCacheBlocks);
}

void PxcNpMemBlockPool::updateSpuFrictionBlockCount()
{
	// PPU uses npCache blocks whereas SPU uses friction blocks for the same data
	// adding up the two leads to a better guess
	PxU32 newNbFrictionBlocks = mFriction[mFrictionActiveStream].size();
	mMaxSpuFrictionBlocks = PxMax(mMaxSpuFrictionBlocks,newNbFrictionBlocks);
}

void PxcNpMemBlockPool::updateSpuConstraintBlockCount()
{
	PxU32 currentNbConstraintBlocks = mConstraints.size();
	mMaxSpuConstraintBlocks = PxMax(mMaxSpuConstraintBlocks, currentNbConstraintBlocks + mConstraintAllocations);
}

void PxcNpMemBlockPool::updateSpuContactBlockCount()
{
	mCurrentSpuContactBlocks = mContacts[mContactIndex].size(); //This is actually contact buffer stuff!
	mMaxSpuContactBlocks = PxMax(mMaxSpuContactBlocks, mCurrentSpuContactBlocks);
}

#endif //PX_PS3



