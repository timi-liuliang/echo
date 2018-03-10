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


#ifndef PXS_AABB_MANAGER_AUX_H
#define PXS_AABB_MANAGER_AUX_H

#include "PxsBroadPhaseCommon.h"
#include "PxsBroadPhaseConfig.h"
#include "PxcScratchAllocator.h"
#include "CmBitMap.h"

namespace physx
{

class PxsAABBManager;
class PxcRigidBody;
struct PxsRigidCore;

#define ALIGN_SIZE_16(size) (((unsigned)(size)+15)&((unsigned)~15))

template<class T> PX_FORCE_INLINE void copyPodArray(T* tar, const T* src, const PxU32 tarCapacity, const PxU32 srcCapacity)
{
	PX_ASSERT(tarCapacity > srcCapacity);
	PX_ASSERT(tar);
	PX_ASSERT(src || 0==srcCapacity);
	PX_ASSERT(0 == ((uintptr_t)tar & 0x0f));
	PX_ASSERT(0 == ((uintptr_t)src & 0x0f));
	PX_ASSERT(tarCapacity > 0);
	if(src)
	{
		PxMemCopy(tar, src, srcCapacity*sizeof(T));
	}
	PxMemZero(tar + srcCapacity, (tarCapacity-srcCapacity)*sizeof(T));
}

template<class T> PX_FORCE_INLINE T* reallocateFromBuffer(const PxU32 srcCapacity, const T* src, const PxU32 tarCapacity, PxU8*& buffer)
{
	copyPodArray<T>((T*)buffer, src, tarCapacity, srcCapacity);
	T* origBuffer = (T*)buffer;
	buffer += ALIGN_SIZE_16(sizeof(T)*tarCapacity);
	return origBuffer;
}

template<class T> T* resizePODArray(const PxU32 oldMaxNb, const PxU32 newMaxNb, T* elements)
{
	PX_ASSERT(newMaxNb > oldMaxNb);
	PX_ASSERT(newMaxNb > 0);
	PX_ASSERT(0==((newMaxNb*sizeof(T)) & 15)); 
	T* newElements = (T*)PX_ALLOC(sizeof(T)*newMaxNb, PX_DEBUG_EXP("PODArray Elements"));
	PX_ASSERT(0==((uintptr_t)newElements & 0x0f));
	copyPodArray<T>(newElements, elements, newMaxNb, oldMaxNb);
	PX_FREE(elements);
	return newElements;
}

template<class T> T* resizePODArray(const PxU32 oldMaxNb, const PxU32 newMaxNb, PxcScratchAllocator* scratchAllocator, T* elements)
{
	PX_ASSERT(newMaxNb > oldMaxNb);
	PX_ASSERT(newMaxNb > 0);
	PX_ASSERT(0==((newMaxNb*sizeof(T)) & 15)); 
	T* newElements = (T*)scratchAllocator->alloc(sizeof(T)*newMaxNb, true);
	PX_ASSERT(0==((uintptr_t)newElements & 0x0f));
	copyPodArray<T>(newElements, elements, newMaxNb, oldMaxNb);
	if(elements) scratchAllocator->free(elements);
	return newElements;
}

///////////////////////////////////////////////////////////////////////////////

// ### put this in the bitmap class itself?
PX_FORCE_INLINE Ps::IntBool testBitmap(const Cm::BitMap& bitmap, const PxU32* bitmasks, const PxU32 index) 
{
	PX_ASSERT(index<bitmap.getWordCount()*32);
	return Ps::IntBool(bitmap.getWords()[index>>5] & bitmasks[index&31]);
}

PX_FORCE_INLINE void setBitmap(Cm::BitMap& bitmap, const PxU32* bitmasks, const PxU32 index) 
{
	PX_ASSERT(index<bitmap.getWordCount()*32);
	bitmap.getWords()[index>>5] |= bitmasks[index&31];
}

PX_FORCE_INLINE void resetBitmap(Cm::BitMap& bitmap,const PxU32* bitmasks, const  PxU32 index) 
{
	PX_ASSERT(index<bitmap.getWordCount()*32);
	bitmap.getWords()[index>>5] &= ~bitmasks[index&31];
}

///////////////////////////////////////////////////////////////////////////////

//Encode the id for a single aabb so that the id also stores whether the shape belongs to an aggregate.
//Used by createVolumeV
PX_FORCE_INLINE bool canEncodeForClient(const PxU32 id)						{ return (id < (PX_INVALID_BP_HANDLE>>1));	}
PX_FORCE_INLINE PxcBpHandle	encodeAggregateForClient(const PxU32 id)		{ return (PxcBpHandle)((id+id)|1);							}
PX_FORCE_INLINE PxcBpHandle	encodeSingleForClient(const PxU32 id)			{ return (PxcBpHandle)(id+id);								}

//Decode the encoded id returned by createVolume to reveal the internal element id and whether the shape belongs to a aggregate.
//Used by releaseVolume etc
PX_FORCE_INLINE PxcBpHandle	decodeAggregateFromClient(const PxcBpHandle bin)	{ PX_ASSERT(bin&1);	return PxcBpHandle(bin>>1);	}
PX_FORCE_INLINE PxcBpHandle	decodeSingleFromClient(const PxcBpHandle id)		{ return PxcBpHandle(id>>1);						}
PX_FORCE_INLINE Ps::IntBool	isClientVolumeAggregate(const PxcBpHandle volume)	{ return Ps::IntBool(volume & 1);	}
PX_FORCE_INLINE PxcBpHandle	decodeFromClient(const PxcBpHandle id)			{ return !isClientVolumeAggregate(id) ? decodeSingleFromClient(id) : decodeAggregateFromClient(id); }

///////////////////////////////////////////////////////////////////////////////

//A specialized bitmap to store indices of overlapping shape pairs of a self-colliding aggregate.
//SelfCollideBitmap uses less memory than a Cm::BitMap because it only needs to store the elements 
//that are above the diagonal ie we only need elements (i,j) with j > i.
class SelfCollideBitmap
{
public:

	friend class SelfCollideBitmapIterator;

	SelfCollideBitmap()
	{
	}

	PX_INLINE SelfCollideBitmap& operator=(const SelfCollideBitmap& other)
	{
		PxMemCopy(map, other.map, (MAX_AGGREGATE_WORD_COUNT/2)*sizeof(PxU32));
		return *this;
	}

	PX_FORCE_INLINE void set(const PxU32 i, const PxU32 j)
	{
		PX_ASSERT(j > i);
		PX_ASSERT(MAX_AGGREGATE_BOUND_SIZE > i);
		const PxU32 id = computeId(i, j);
		PX_ASSERT((id >> 5) < (MAX_AGGREGATE_WORD_COUNT/2));
		map[id >> 5] |= (1 << (id & 31));
		PX_ASSERT(test(i, j));
	}

	PX_FORCE_INLINE void reset(const PxU32 i, const PxU32 j)
	{
		PX_ASSERT(j > i);
		PX_ASSERT(MAX_AGGREGATE_BOUND_SIZE > i);
		const PxU32 id = computeId(i, j);
		PX_ASSERT((id >> 5) < (MAX_AGGREGATE_WORD_COUNT/2));
		map[id >> 5] &= ~(1 << (id & 31));
		PX_ASSERT(!test(i, j));
	}

	PX_FORCE_INLINE Ps::IntBool test(const PxU32 i, const PxU32 j) 
	{
		PX_ASSERT(j > i);
		PX_ASSERT(MAX_AGGREGATE_BOUND_SIZE > i);
		const PxU32 id = computeId(i, j);
		PX_ASSERT((id >> 5) < (MAX_AGGREGATE_WORD_COUNT/2));
		return Ps::IntBool((map[id >> 5]) & (1 << (id & 31)));
	}

	PX_FORCE_INLINE void resetAll()
	{
		PxMemZero(map, sizeof(PxU32)*(MAX_AGGREGATE_WORD_COUNT/2));
	}

	PX_FORCE_INLINE void promote(const PxU32 size)
	{
		SelfCollideBitmap copy;
		PxMemCopy(copy.map, map, sizeof(PxU32)*(MAX_AGGREGATE_WORD_COUNT/2));
		resetAll();

		for(PxU32 i = 0; i < size; i++)
		{
			for(PxU32 j = i + 1; j < size; j++)
			{
				if(copy.test(i, j))
				{
					set(i+1, j+1);
				}
			}
		}
	}

private:

	PX_FORCE_INLINE static PxU32 computeId(const PxU32 i, const PxU32 j)
	{
		PX_ASSERT(j > i);
		//(0,1)                            <- starts at 0 in the bitmap
		//(0,2) (1,2)                      <- starts at 1 in the bitmap
		//(0,3) (1,3) (2,3)                <- starts at 3 in the bitmap
		//(0,4) (1,4) (2,4) (3,4)          <- starts at 6 in the bitmap
		return (((j*(j-1))/2) + i);
	}

	PX_FORCE_INLINE static PxU32 computeIdIZero(const PxU32 j)
	{
		return computeId(0, j);
	}

	PX_FORCE_INLINE static PxU32 computeIdI(const PxU32 bitIndex, const PxU32 j)
	{
		PX_ASSERT(j > (bitIndex - computeId(0, j)));
		return (bitIndex - computeId(0, j));
	}

	PxU32 map[MAX_AGGREGATE_WORD_COUNT/2];		

private:

};

//Iterate through the elements of a SelfCollideBitmap instance that satisfy bitmap.test(i,j) == true
class SelfCollideBitmapIterator
{
public:

	static const PxU32 DONE = 0xffffffff;

	PX_INLINE SelfCollideBitmapIterator(const SelfCollideBitmap& map) :	mBitMap(map)
	{
		reset();
	}

	PX_INLINE PxU32	getNext(PxU32& indexI, PxU32& indexJ)
	{
		if(mBlock)
		{
			PxU32 bitIndex = mIndex<<5 | Ps::lowestSetBit(mBlock);

			PxU32 j = mJ;
			while(SelfCollideBitmap::computeIdIZero(j+1) <= bitIndex)
			{
				j++;
			}
			mJ = j;
			indexJ = j;
			indexI = SelfCollideBitmap::computeIdI(bitIndex, j);

			mBlock &= mBlock-1;
			while(!mBlock && ++mIndex < (MAX_AGGREGATE_WORD_COUNT/2))
				mBlock = mBitMap.map[mIndex];

			return bitIndex;
		}
		return DONE;
	}

private:

	PxU32 mBlock;
	PxU32 mIndex;
	PxU32 mJ;
	const SelfCollideBitmap& mBitMap;

	PX_INLINE void reset()
	{
		mIndex = mBlock = 0;
		mJ = 1;
		while((mIndex < (MAX_AGGREGATE_WORD_COUNT/2)) && ((mBlock = mBitMap.map[mIndex]) == 0))
			++mIndex;
	}

	PX_INLINE SelfCollideBitmapIterator& operator=(const SelfCollideBitmapIterator& other)
	{
		PX_ASSERT(&mBitMap == &other.mBitMap);
		mBlock = other.mBlock;
		mIndex = other.mIndex;
		mJ = other.mJ;
		return *this;
	}

};

///////////////////////////////////////////////////////////////////////////////

//An aggregate is a collection of shapes and actors with a single entry broadphase
//entry that represents the composite of all its shape bounds.
//The Aggregate structure stores important data for each aggregate.
struct Aggregate
{
	//A self-collision bitmap that caches the shape pairs of the aggregate that are in aabb overlap.
	SelfCollideBitmap selfCollBitmap;

	//A flag that records whether the aggregate supports self-collision.
	bool			selfCollide;		

	//The peak number of shapes achieved by the aggregate.  Note that some of the shapes may have been 
	//removed after the peak was achieved.
	PxU8			nbElems;		

	//The current number of shapes in the aggregate. Note that nbActive <= nbElems. 
	PxU8			nbActive;		
	PxU8			pad[1];

	//The index of the aggregate bounds in the array of broadphase bounds stored in the BPElems class.
	PxcBpHandle		bpElemId;											

	//The index of the aggregate's first shape in the array of all individual shape bounds stored in the 
	//AggregateElems class.  Iteration over the entire list of individual shape bounds is possible using
	//AggregateElems::getNextId until PX_INVALID_BP_HANDLE is returned.  Note that not some of the shapes 
	//may have been deleted but the validity of a shape can be tested with 
	//AggregateElems::getGroup(shapeId) != PX_INVALID_BP_HANDLE
	PxcBpHandle		elemHeadID;															

	//A group id for the aggregate that is used by the broadphase.  Each aggregate is assigned a unique 
	//group id.
	PxcBpHandle		group;

	//The index of the aggregate's first actor in the array of all aggregate actors stored in the 
	//AggregateActorManager class.  Iteration over the entire list of actors is possible using 
	//AggregateActorManager::getNextId until PX_INVALID_BP_HANDLE is returned.
	PxcBpHandle		actorHeadID;

	//A pointer to a structure that represents the aggregate in high-level.
	void*			userData;

	void			reset()
	{
		selfCollide	= true;
		nbElems		= 0;
		nbActive	= 0;
		bpElemId	= PX_INVALID_BP_HANDLE;
		elemHeadID	= PX_INVALID_BP_HANDLE;
		group		= PX_INVALID_BP_HANDLE;
		actorHeadID = PX_INVALID_BP_HANDLE;
		userData	= NULL;
	}

	void			init()
	{
		selfCollBitmap.resetAll();
		selfCollide	= true;
		nbElems		= 0;
		nbActive	= 0;
		bpElemId	= PX_INVALID_BP_HANDLE;
		elemHeadID	= PX_INVALID_BP_HANDLE;
		group		= PX_INVALID_BP_HANDLE;
		actorHeadID = PX_INVALID_BP_HANDLE;
		userData	= NULL;
	}

	PX_FORCE_INLINE bool performSelfCollision() const
	{
		return (selfCollide && (nbActive > 0));
	}
};
#if defined(PX_PS3) || defined(PX_X360) 
PX_COMPILE_TIME_ASSERT(0==(sizeof(Aggregate) & 0x0f));
#endif


//If a shape of an aggregate actor is removed it's position in the aggregate's shape list
//is stored in AggregateFreeElemsWords.  The background here is that to preserve the integrity
//of the self-collision bitmap shapes are never truly freed for re-use by another aggregate until 
//the aggregate has all its shapes removed.  If a shape is removed then the list of shapes is 
//unmodified, the removed shape is flagged as inactive, and position in the shape list is stored in 
//AggregateFreeElemsWords.  If a fresh shape is then added to the aggregate we first search for 
//inactive shapes stored in AggregateFreeElemsWords and reuse if we find one.
class AggregateFreeElemsWords
{
public:

	AggregateFreeElemsWords()
	{
		clearWords();
	}
	~AggregateFreeElemsWords(){}

	void clearWords()
	{
		for(PxU32 i = 0; i < (MAX_AGGREGATE_BOUND_SIZE >> 5); i++)
		{
			mWords[i] = 0;
		}
	}

	void set(const PxU32 id)
	{
		mWords[id >> 5] |= (1 << (id & 31));
	}

	void reset(const PxU32 id) 
	{
		mWords[id >> 5] &= ~(1 << (id & 31));
	}

	Ps::IntBool test(const PxU32 id) const
	{
		return Ps::IntBool(mWords[id >> 5] & (1 << (id & 31)));
	}

	void combineOR(const AggregateFreeElemsWords& other)
	{
		for(PxU32 i = 0; i < (MAX_AGGREGATE_BOUND_SIZE >> 5); i++)
		{
			mWords[i] |= other.mWords[i];
		}
	}

	PxU32 findLast()
	{
		for(PxU32 i = (MAX_AGGREGATE_BOUND_SIZE >> 5); i-- > 0;)
		{
			if(mWords[i])
			{
				return ((i << 5) + Ps::highestSetBit(mWords[i]));
			}
		}
		return MAX_AGGREGATE_BOUND_SIZE;
	}

#ifndef NDEBUG
	bool hasNonZeroWords() const
	{
		for(PxU32 i = 0; i < (MAX_AGGREGATE_BOUND_SIZE >> 5);i++)
		{
			if(mWords[i] != 0) return true;
		}
		return false;
	}
#endif

private:

	PxU32 mWords[MAX_AGGREGATE_BOUND_SIZE>>5];
};

//The struct AggregateManager is used to manage all Aggregate instances in the scene.
class AggregateManager 
{
	PX_NOCOPY(AggregateManager)

public:

	AggregateManager() :
		mBuffer						(NULL),
		mAggregates					(NULL),
		mAggregateReleasedElemsWords(NULL),
		mAggregateAvailableElemsWords(NULL),
		mAggregatesSize				(0),
		mAggregatesCapacity			(0),
		mFreeIDs					(NULL),
		mFreeIDsSize				(0),
		mAggregateGroupTide			(PX_INVALID_BP_HANDLE-1),
		mFreeAggregateGroups		(NULL),
		mFreeAggregateGroupsSize	(0)
	  {
	  }

	  ~AggregateManager()
	  {
		  PX_FREE(mBuffer);
	  }

	  void resize(const PxU32 newAggregatesCapacity)
	  {
		  PX_ASSERT(newAggregatesCapacity > mAggregatesCapacity);

		  const PxU32 oldAggregatesCapacity = mAggregatesCapacity;
		  const PxU32 newBitMapWordCount = ((newAggregatesCapacity + 31) >> 5);

		  //Allocate a single buffer for everything we need (Aggregates + free ids + group ids)
		  const PxU32 sizeAggregates = ALIGN_SIZE_16(newAggregatesCapacity*sizeof(Aggregate));
		  const PxU32 sizeHandles = ALIGN_SIZE_16(newAggregatesCapacity*sizeof(PxcBpHandle));
		  const PxU32 sizeFreeElems = ALIGN_SIZE_16(newAggregatesCapacity*sizeof(AggregateFreeElemsWords));
		  const PxU32 sizeBitmap = ALIGN_SIZE_16(newBitMapWordCount*sizeof(PxU32));
		  const PxU32 byteSize = sizeAggregates + sizeHandles + sizeHandles + sizeFreeElems + sizeFreeElems + sizeBitmap + sizeBitmap;
		  PxU8* newBuffer = (PxU8*)PX_ALLOC(byteSize, PX_DEBUG_EXP("AABBManager"));

		  //Copy from the old buffers to the new buffers.

		  //aggregates.
		  mAggregates = reallocateFromBuffer<Aggregate>(oldAggregatesCapacity, mAggregates, newAggregatesCapacity, newBuffer);
		  for(PxU32 i = oldAggregatesCapacity; i < newAggregatesCapacity; i++)
		  {
			  mAggregates[i].init();
		  }

		  //freeIds
		  mFreeIDs = reallocateFromBuffer<PxcBpHandle>(oldAggregatesCapacity, mFreeIDs, newAggregatesCapacity, newBuffer);

		  //freeAggregateGroups
		  mFreeAggregateGroups = reallocateFromBuffer<PxcBpHandle>(oldAggregatesCapacity, mFreeAggregateGroups, newAggregatesCapacity, newBuffer);

		  //released elems
		  mAggregateReleasedElemsWords = reallocateFromBuffer<AggregateFreeElemsWords>(oldAggregatesCapacity, mAggregateReleasedElemsWords, newAggregatesCapacity, newBuffer);

		  //available elems
		  mAggregateAvailableElemsWords = reallocateFromBuffer<AggregateFreeElemsWords>(oldAggregatesCapacity, mAggregateAvailableElemsWords, newAggregatesCapacity, newBuffer);

		  //dirty bitmap
		  PxU32* newDirtyBitMapWords = reallocateFromBuffer<PxU32>(mDirtyBitMap.getWordCount(), mDirtyBitMap.getWords(), newBitMapWordCount, newBuffer);
		  mDirtyBitMap.setWords(newDirtyBitMapWords, newBitMapWordCount);

		  //bp overlap bitmap
		  PxU32* newOverlapBitMapWords = reallocateFromBuffer<PxU32>(mBroadPhaseOverlapBitMap.getWordCount(), mBroadPhaseOverlapBitMap.getWords(), newBitMapWordCount, newBuffer);
		  mBroadPhaseOverlapBitMap.setWords(newOverlapBitMapWords, newBitMapWordCount);
		  PX_ASSERT(newBuffer == ((PxU8*)mAggregates + byteSize));

		  //Set the new capacity.
		  mAggregatesCapacity = newAggregatesCapacity;

		  //Free the old buffer.
		  PX_FREE(mBuffer);
		  mBuffer = (PxU8*)mAggregates;
	  }

	  PX_FORCE_INLINE PxU32 getAggregatesCapacity() const 
	  {
		  return mAggregatesCapacity;
	  }

	  PX_FORCE_INLINE Aggregate* getAggregate(const PxU32 aggregateId) const
	  {
		  PX_ASSERT(aggregateId < mAggregatesCapacity);
		  return &mAggregates[aggregateId];
	  }

	  PxcBpHandle createAggregate(void* userData, const bool selfCollisions)
	  {
		  PxU32 aggregateId = 0xffffffff;
		  Aggregate* aggregate = NULL;

		  if(mFreeIDsSize)
		  {
			  //There is an aggregate that can be reused because it has been used and then released.
			  PX_ASSERT(mFreeIDsSize <= mAggregatesCapacity);
			  aggregateId = mFreeIDs[mFreeIDsSize-1];
			  PX_ASSERT(aggregateId<mAggregatesCapacity);
			  mFreeIDsSize--;
			  aggregate = &mAggregates[aggregateId];
		  }
		  else if(mAggregatesSize < mAggregatesCapacity)
		  {
			  //No aggregates to be reused but we have at least one spare unused aggregate to satisfy this request.
			  aggregateId = mAggregatesSize;
			  aggregate = &mAggregates[aggregateId];
			  mAggregatesSize++;
		  }
		  else
		  {
			  //No aggregates to be reused and no spare unused aggregates for this request.
			  //Need to resize the array of aggregates to make a spare unused aggregate.
			  PX_ASSERT(mAggregatesSize == mAggregatesCapacity);
			  resize(mAggregatesSize + 32);

			  //Now we've got a s spare aggregate to satisfy this request.
			  aggregateId = mAggregatesSize;
			  aggregate = &mAggregates[aggregateId];
			  mAggregatesSize++;
		  }

		  PX_ASSERT(aggregateId < mAggregatesCapacity);
		  PX_ASSERT(0xffffffff != aggregateId);
		  PX_ASSERT(aggregate);
		  PX_ASSERT(0==aggregate->nbElems);
		  PX_ASSERT(0==aggregate->nbActive);
		  PX_ASSERT(PX_INVALID_BP_HANDLE==aggregate->bpElemId);
		  PX_ASSERT(PX_INVALID_BP_HANDLE==aggregate->elemHeadID);
		  PX_ASSERT(PX_INVALID_BP_HANDLE==aggregate->group);
		  PX_ASSERT(PX_INVALID_BP_HANDLE==aggregate->actorHeadID);
		  PX_ASSERT(!mAggregateReleasedElemsWords[aggregateId].hasNonZeroWords());
		  PX_ASSERT(!mAggregateAvailableElemsWords[aggregateId].hasNonZeroWords());
		  PX_ASSERT(!mDirtyBitMap.test(aggregateId));
		  PX_ASSERT(!mBroadPhaseOverlapBitMap.test(aggregateId));

		  //Set the aggregate self-collide flags.
		  aggregate->selfCollide = selfCollisions;

		  //And the userdata
		  aggregate->userData = userData;

		  //Make sure the aggregate is clean.
		  mDirtyBitMap.reset(aggregateId);

		  //Get an unused group id for the aggregate.
		  const PxcBpHandle groupId = getFreeAggregateGroup();
		  aggregate->group = groupId;

		  return (PxcBpHandle)aggregateId;
	  }

	  PX_FORCE_INLINE void clearAggregate(const PxU32 id)
	  {
		  PX_ASSERT(id < mAggregatesCapacity);
		  Aggregate* aggregate=getAggregate(id);
		  PX_ASSERT(0==aggregate->nbActive);

		  //Reset the aggregate data.
		  //reset() resets everything but we don't want to clear the group or actorHeadId.
		  //just store them temporarily, reset the aggregate, then reset the group and actorHeadId
		  //Note: the group is set in createAggregate and is only freed in reuseAggregate.
		  const PxcBpHandle group = aggregate->group;
		  const PxcBpHandle actorHeadId = aggregate->actorHeadID;
		  aggregate->reset();
		  aggregate->group=group;
		  aggregate->actorHeadID = actorHeadId;

		  //Make sure the aggregate is clean.
		  mDirtyBitMap.reset(id);

		  //Clear the record of broadphase overlaps
		  mBroadPhaseOverlapBitMap.reset(id);

		  //Clear the words describing the internal elems that can be reused by the aggregate.
		  mAggregateReleasedElemsWords[id].clearWords();
		  mAggregateAvailableElemsWords[id].clearWords();
	  }

	  PX_FORCE_INLINE void reuseAggregate(const PxU32 id)
	  {
		  //Reuse the aggregate id.
		  PX_ASSERT(id<mAggregatesCapacity);
		  PX_ASSERT(!mAggregates[id].nbActive);
		  PX_ASSERT(mFreeIDsSize<mAggregatesCapacity);
		  mFreeIDs[mFreeIDsSize]=(PxcBpHandle)id;
		  mFreeIDsSize++;

		  //Reuse the group id.
		  PX_ASSERT(id<mAggregatesCapacity);
		  PX_ASSERT(PX_INVALID_BP_HANDLE!=mAggregates[id].group);
		  const PxcBpHandle group=mAggregates[id].group;
		  reuseAggregateGroup(group);

		  //Invalidate the bp and group.
		  mAggregates[id].group=PX_INVALID_BP_HANDLE;

		  //Invalidate the actorHead
		  mAggregates[id].actorHeadID=PX_INVALID_BP_HANDLE;

		  //Clear the words describing the internal elems that can be reused by the aggregate.
		  mAggregateReleasedElemsWords[id].clearWords();
		  mAggregateAvailableElemsWords[id].clearWords();

		  //Clear the record of broadphase overlaps
		  mBroadPhaseOverlapBitMap.reset(id);
	  }

	  PX_FORCE_INLINE void releaseElem(const PxU32 id, const PxcBpHandle index)
	  {
		  PX_ASSERT(id<mAggregatesCapacity);
		  mAggregateReleasedElemsWords[id].set(index);
	  }

	  void markReleasedAggregateElemsAsAvailable()
	  {
		  for(PxU32 i=0;i<mAggregatesCapacity;i++)
		  {
			  mAggregateAvailableElemsWords[i].combineOR(mAggregateReleasedElemsWords[i]);
			  mAggregateReleasedElemsWords[i].clearWords();
		  }
	  }

	  PX_FORCE_INLINE PxcBpHandle getAvailableElem(const PxU32 id)
	  {
		  PX_ASSERT(id<mAggregatesCapacity);
		  const PxU32 last = mAggregateAvailableElemsWords[id].findLast();
		  if (last != MAX_AGGREGATE_BOUND_SIZE)
		  {
			  PX_ASSERT(mAggregateAvailableElemsWords[id].hasNonZeroWords());
			  mAggregateAvailableElemsWords[id].reset(last);
			  return (PxcBpHandle)last;
		  }
		  else
		  {
			  PX_ASSERT(!mAggregateAvailableElemsWords[id].hasNonZeroWords());
			  return PX_INVALID_BP_HANDLE;
		  }
	  }

	  PX_FORCE_INLINE void setClean(const PxcBpHandle id)
	  {
		  PX_ASSERT(id<mAggregatesCapacity);
		  PX_ASSERT(!mAggregates[id].selfCollide);
		  PX_ASSERT(mAggregates[id].nbActive > 0);
		  mDirtyBitMap.reset(id);
	  }

	  PX_FORCE_INLINE void setDirty(const PxcBpHandle id)
	  {
		  PX_ASSERT(id<mAggregatesCapacity);
		  PX_ASSERT(!mAggregates[id].selfCollide);
		  PX_ASSERT(mAggregates[id].nbActive > 0);
		  mDirtyBitMap.set(id);
	  }

	  PX_FORCE_INLINE Ps::IntBool isDirty(const PxcBpHandle id)
	  {
		  PX_ASSERT(id<mAggregatesCapacity);
		  PX_ASSERT(!mDirtyBitMap.test(id) || (!mAggregates[id].selfCollide && mAggregates[id].nbActive> 0));
		  return mDirtyBitMap.test(id);
	  }

	  PX_FORCE_INLINE void clearBroadPhaseOverlapBitMap()
	  {
		  mBroadPhaseOverlapBitMap.clearFast();
	  }

	  PX_FORCE_INLINE void setBroadphaseOverlap(const PxcBpHandle id)
	  {
		  PX_ASSERT(id<mAggregatesCapacity);
		  mBroadPhaseOverlapBitMap.set(id);
	  }

	  PX_FORCE_INLINE Ps::IntBool getHasBroadphaseOverlap(const PxcBpHandle id)
	  {
		  PX_ASSERT(id<mAggregatesCapacity);
		  return mBroadPhaseOverlapBitMap.test(id);
	  }

private:

	//Buffer that stores mAggregates, mFreeIds, and mFreeAggregateGroups
	PxU8*						mBuffer;

	//Array of aggregates.
	Aggregate*					mAggregates;
	AggregateFreeElemsWords*	mAggregateReleasedElemsWords;
	AggregateFreeElemsWords*	mAggregateAvailableElemsWords;
	PxU32						mAggregatesSize;
	PxU32						mAggregatesCapacity;

	//Free aggregate ids.
	PxcBpHandle*				mFreeIDs;	
	PxU32						mFreeIDsSize;

	//Free aggregate group ids.
	PxcBpHandle					mAggregateGroupTide;
	PxcBpHandle*				mFreeAggregateGroups;
	PxU32						mFreeAggregateGroupsSize;

	//Bitmap of dirty  aggregates (aggregates without self-collision that 
	//are updated but don't require having their shape bounds updated because 
	//the aggregate doesn't overlap anything).
	Cm::BitMap					mDirtyBitMap;

	//Bitmap of aggregates with broadphase overlaps.
	Cm::BitMap					mBroadPhaseOverlapBitMap;

	PX_FORCE_INLINE void reuseAggregateGroup(const PxcBpHandle group) 
	{
		PX_ASSERT(group != PX_INVALID_BP_HANDLE);
		PX_ASSERT(mFreeAggregateGroupsSize < mAggregatesCapacity);
		mFreeAggregateGroups[mFreeAggregateGroupsSize]=group;
		mFreeAggregateGroupsSize++;
	}

	PX_FORCE_INLINE PxcBpHandle	getFreeAggregateGroup()
	{
		PxcBpHandle group=PX_INVALID_BP_HANDLE;
		if(mFreeAggregateGroupsSize)
		{
			group = mFreeAggregateGroups[mFreeAggregateGroupsSize-1];
			mFreeAggregateGroupsSize--;
		}
		else
		{
			group=mAggregateGroupTide;
			mAggregateGroupTide--;
		}
		PX_ASSERT(group!=PX_INVALID_BP_HANDLE);
		return group;
	}
};

//AggregateActorManager stores the list of actors in each aggregate.  The index of the 
//head of the list is stored in Aggregate.  Iteration through the list is achieved with 
//getNextId.
//Each actor stores a shape count that allows a bitmap of empty actors to be maintained.
//Actors are flagged dirty when they have updated geometry of local pose or their shape
//count changes.
//Each actor stores an aggregate id that references the aggregate in AggregateManager.
struct AggregateActorManager
{
	PX_NOCOPY(AggregateActorManager)

public:

	AggregateActorManager()
		: mBuffer(NULL),
		  mPersistentLocalSpaceBounds(NULL),
		  mShapeCounts(NULL),
		  mNextIds(NULL),
		  mAggregateIds(NULL),
		  mCapacity(0),
		  mFirstFreeElem(PX_INVALID_BP_HANDLE)
	{
	}

	~AggregateActorManager()
	{
		if(mBuffer) PX_FREE(mBuffer);
	}

	PX_FORCE_INLINE PxU32 getCapacity() const {return mCapacity;}

	PX_FORCE_INLINE PxcBpHandle getAvailableElem()
	{
		if(PX_INVALID_BP_HANDLE==mFirstFreeElem)
		{
			resize(PxMax(2*mCapacity, (PxU32)64));
		}

		const PxcBpHandle firstFreeElem = (PxcBpHandle)mFirstFreeElem;
		mFirstFreeElem = *(PxcBpHandle*)&mPersistentLocalSpaceBounds[firstFreeElem];
		setEmptyBounds(firstFreeElem);
		mShapeCounts[firstFreeElem] = 0;

		return firstFreeElem;
	}

	PX_FORCE_INLINE void freeElem(const PxcBpHandle id)
	{
		PX_ASSERT(id < mCapacity);

		(*(PxcBpHandle*)&mPersistentLocalSpaceBounds[id])=(PxcBpHandle)mFirstFreeElem;
		mFirstFreeElem=id;

		mShapeCounts[id] = 0;

		mNextIds[id] = PX_INVALID_BP_HANDLE;

		mAggregateIds[id] = PX_INVALID_BP_HANDLE;

		mDirtyBitMap.reset(id);
		mEmptyBitMap.reset(id);
	}

	PX_FORCE_INLINE void setAggregateId(const PxcBpHandle id, const PxcBpHandle aggregateId)
	{
		PX_ASSERT(id < mCapacity);
		mAggregateIds[id] = aggregateId;
	}

	PX_FORCE_INLINE PxcBpHandle getAggregateId(const PxcBpHandle id) const 
	{
		PX_ASSERT(id < mCapacity);
		return mAggregateIds[id];
	}

	PX_FORCE_INLINE void incrementShapeCount(const PxcBpHandle id)
	{
		PX_ASSERT(id < mCapacity);
		mShapeCounts[id]++;
		mEmptyBitMap.reset(id);
	}

	PX_FORCE_INLINE bool decrementShapeCount(const PxcBpHandle id)
	{
		PX_ASSERT(id < mCapacity);
		PX_ASSERT(mShapeCounts[id] > 0);
		const PxU32 shapeCount = mShapeCounts[id];
		mShapeCounts[id]--;

		if(shapeCount > 1)
		{
			return false;
		}
		else
		{
			mEmptyBitMap.set(id);
			return true;
		}
	}

	PX_FORCE_INLINE PxU8 getShapeCount(const PxcBpHandle id) const
	{
		PX_ASSERT(id < mCapacity);
		return mShapeCounts[id];
	}

	PX_FORCE_INLINE const Cm::BitMap& getEmptyBitMap() const
	{
		return mEmptyBitMap;
	}

	PX_FORCE_INLINE void resetEmptyBitmap()
	{
		mEmptyBitMap.clearFast();
	}

	PX_FORCE_INLINE void setNextId(const PxcBpHandle id, const PxcBpHandle nextId)
	{
		PX_ASSERT(id < mCapacity);
		PX_ASSERT(PX_INVALID_BP_HANDLE == nextId || nextId < mCapacity);
		mNextIds[id] = nextId;
	}

	PX_FORCE_INLINE PxcBpHandle getNextId(const PxcBpHandle id) const 
	{
		PX_ASSERT(id < mCapacity);
		return mNextIds[id];
	}

	PX_FORCE_INLINE void setDirty(const PxcBpHandle id)
	{
		PX_ASSERT(id < mCapacity);
		mDirtyBitMap.set(id);
	}

	PX_FORCE_INLINE void setClean(const PxcBpHandle id)
	{
		PX_ASSERT(id < mCapacity);
		mDirtyBitMap.reset(id);
	}

	PX_FORCE_INLINE void resetDirtyBitmap()
	{
		mDirtyBitMap.clearFast();
	}

	PX_FORCE_INLINE Ps::IntBool getIsDirty(const PxcBpHandle id)
	{
		PX_ASSERT(id < mCapacity);
		return mDirtyBitMap.test(id);
	}

	PX_FORCE_INLINE const Cm::BitMap& getDirtyBitmap() const
	{
		return mDirtyBitMap;
	}

	PX_FORCE_INLINE void freeActors(const PxcBpHandle headId)
	{
		PxcBpHandle id = headId;
		PxcBpHandle firstFreeElem = mFirstFreeElem;
		while(PX_INVALID_BP_HANDLE != id)
		{
			PxcBpHandle nextId = mNextIds[id];

			(*(PxcBpHandle*)&mPersistentLocalSpaceBounds[id])=(PxcBpHandle)firstFreeElem;
			firstFreeElem=id;

			PX_ASSERT(0==mShapeCounts[id]);
			mShapeCounts[id] = 0;

			mNextIds[id] = PX_INVALID_BP_HANDLE;

			mAggregateIds[id] = PX_INVALID_BP_HANDLE;

			mDirtyBitMap.reset(id);
			mEmptyBitMap.reset(id);

			id = nextId;
		}
		mFirstFreeElem = firstFreeElem;
	}

	PX_FORCE_INLINE void setEmptyBounds(const PxcBpHandle id)
	{
		PX_ASSERT(id < mCapacity);
		mPersistentLocalSpaceBounds[id].setEmpty();
	}

	PX_FORCE_INLINE const PxBounds3& getBounds(const PxcBpHandle id) const
	{
		PX_ASSERT(id < mCapacity);
		PX_ASSERT(mShapeCounts[id] > 0);
		PX_ASSERT(!isEmpty(id));
		return mPersistentLocalSpaceBounds[id];
	}

	PX_FORCE_INLINE void expandBounds(const PxcBpHandle id, const PxBounds3& bounds)
	{
		PX_ASSERT(id < mCapacity);
		PX_ASSERT(mShapeCounts[id] > 0);
		PX_ASSERT(!isEmpty(id));
		mPersistentLocalSpaceBounds[id].include(bounds);
	}

	void resize(const PxU32 newCapacity)
	{
		const PxU32 oldCapacity = mCapacity;
		const PxU32 newBitMapBufferSize = ((newCapacity + 31) >> 5);

		const PxU32 sizeBounds = ALIGN_SIZE_16(sizeof(PxBounds3)*newCapacity);
		const PxU32 sizeShapeCounts = ALIGN_SIZE_16(sizeof(PxU8)*newCapacity);
		const PxU32 sizeNextIds = ALIGN_SIZE_16(sizeof(PxcBpHandle)*newCapacity);
		const PxU32 sizeAggregateIds = ALIGN_SIZE_16(sizeof(PxcBpHandle)*newCapacity);
		const PxU32 sizeBitmapBuffer = ALIGN_SIZE_16(sizeof(PxU32)*newBitMapBufferSize);
		const PxU32 byteSize = sizeBounds + sizeShapeCounts + sizeNextIds + sizeAggregateIds + sizeBitmapBuffer + sizeBitmapBuffer;

		PxU8* newBuffer = (PxU8*)PX_ALLOC(byteSize, PX_DEBUG_EXP("PxsAABBManager"));

		mPersistentLocalSpaceBounds = reallocateFromBuffer<PxBounds3>(oldCapacity, mPersistentLocalSpaceBounds, newCapacity, newBuffer);

		mShapeCounts = reallocateFromBuffer<PxU8>(oldCapacity, mShapeCounts, newCapacity, newBuffer);

		mNextIds = reallocateFromBuffer<PxcBpHandle>(oldCapacity, mNextIds, newCapacity, newBuffer);

		mAggregateIds = reallocateFromBuffer<PxcBpHandle>(oldCapacity, mAggregateIds, newCapacity, newBuffer);

		PxU32* newDirtybitmapBuffer = reallocateFromBuffer<PxU32>(mDirtyBitMap.getWordCount(), mDirtyBitMap.getWords(), newBitMapBufferSize, newBuffer);
		mDirtyBitMap.setWords(newDirtybitmapBuffer, newBitMapBufferSize);

		PxU32* newEmptybitmapBuffer = reallocateFromBuffer<PxU32>(mEmptyBitMap.getWordCount(), mEmptyBitMap.getWords(), newBitMapBufferSize, newBuffer);
		mEmptyBitMap.setWords(newEmptybitmapBuffer, newBitMapBufferSize);

		if(mBuffer)
		{
			PX_FREE(mBuffer);
		}
		mBuffer = (PxU8*)mPersistentLocalSpaceBounds;

		mCapacity = newCapacity;

		*(PxcBpHandle*)&mPersistentLocalSpaceBounds[newCapacity-1]=(PxcBpHandle)mFirstFreeElem;
		for(PxU32 i=oldCapacity;i<(newCapacity-1);i++)
		{
			*(PxcBpHandle*)&mPersistentLocalSpaceBounds[i] = (PxcBpHandle)(i+1);
		}
		mFirstFreeElem=(PxcBpHandle)oldCapacity;
	}

private:

	//Based address for all arrays.
	PxU8* mBuffer;

	//Composite local space bounds of each actor.
	PxBounds3* mPersistentLocalSpaceBounds;
	//Number of shapes attached to each actor
	PxU8* mShapeCounts;
	//Iteration over all of all actors in an aggregate (head of list stored in Aggregate).
	PxcBpHandle* mNextIds;
	//id of aggregate of each actor
	PxcBpHandle* mAggregateIds;
	//Capacity of all arrays above.
	PxU32 mCapacity;
	//Bitmap of all actors that are "dirty" (actors having shapes with modified local pose/geometry or freshly added/removed shapes).
	Cm::BitMap mDirtyBitMap;
	//Bitmap of all actors with zero shapes attached.
	Cm::BitMap mEmptyBitMap;
	//Iteration over unused ids in arrays and bitmaps above.
	PxcBpHandle mFirstFreeElem;

	PX_FORCE_INLINE Ps::IntBool isEmpty(const PxcBpHandle id) const 
	{
		PX_ASSERT(id < mCapacity);
		return mEmptyBitMap.test(id);
	}
};

///////////////////////////////////////////////////////////////////////////////


//A single is a single actor whose shapes have individual broadphase entries.
//Each single stores just headID, which references the first shape in the actor's
//shape list. Iteration over the entire list of shapes is achieved with 
//BPElems::getNextId until PX_INVALID_BP_HANDLE is reached.
struct Single
{
	PxcBpHandle		headID;

	void reset() {headID = PX_INVALID_BP_HANDLE;}
};

//The struct SingleManager manages the entire collection of Single instances.
class SingleManager 
{

public:

	SingleManager()
		: mBuffer(NULL),
	      mSingles(NULL),
		  mSinglesSize(0),
	 	  mSinglesCapacity(0),
		  mFreeIDs(NULL),
		  mFreeIDsSize(0)
	{
	}

	~SingleManager()
	{
		PX_FREE(mBuffer);
	}

	PX_FORCE_INLINE PxU32 getCapacity() const 
	{
		return mSinglesCapacity;
	}

	PX_FORCE_INLINE Single* getSingle(const PxU32 singleId)
	{
		PX_ASSERT(singleId < mSinglesCapacity);
		return &mSingles[singleId];
	}

	void resize(PxU32 newCapacity)
	{
		PX_ASSERT(newCapacity > mSinglesCapacity);

		const PxU32 oldCapacity=mSinglesCapacity;

		const PxU32 sizeSingles = ALIGN_SIZE_16(newCapacity*sizeof(Single));
		const PxU32 sizeHandles = ALIGN_SIZE_16(newCapacity*sizeof(PxcBpHandle));
 		PxU8* newBuffer=(PxU8*)PX_ALLOC(sizeSingles + sizeHandles, PX_DEBUG_EXP("AABBManager"));

		mSingles = reallocateFromBuffer<Single>(oldCapacity, mSingles, newCapacity, newBuffer);
		for(PxU32 i = oldCapacity; i < newCapacity; i++)  // mainly for origin shift but is safer in general anyway
		{
			mSingles[i].reset();
		}

		mFreeIDs = reallocateFromBuffer<PxcBpHandle>(oldCapacity, mFreeIDs, newCapacity, newBuffer);

		mSinglesCapacity = newCapacity;

		PX_FREE(mBuffer);
		mBuffer =  (PxU8*)mSingles;
	}


	PxcBpHandle createSingle()
	{
		PxU32 singleId=0xffffffff;
		Single* single=NULL;

		if(mFreeIDsSize)
		{
			//There is a aggregate that can be reused because it has been used and then released.
			PX_ASSERT(mFreeIDsSize <= mSinglesCapacity);
			singleId = mFreeIDs[mFreeIDsSize-1];
			PX_ASSERT(singleId<mSinglesCapacity);
			mFreeIDsSize--;
			single = &mSingles[singleId];
		}
		else if(mSinglesSize < mSinglesCapacity)
		{
			//No aggregates to be reused but we have at least one spare unused aggregate to satisfy this request.
			singleId = mSinglesSize;
			single = &mSingles[singleId];
			mSinglesSize++;
		}
		else
		{
			//No aggregates to be reused and no spare unused aggregates for this request.
			//Need to resize the array of aggregates to make a spare unused aggregate.
			resize(mSinglesCapacity+32);

			//Now we've got a s spare aggregate to satisfy this request.
			singleId = mSinglesSize;
			single = &mSingles[singleId];
			mSinglesSize++;
		}

		PX_ASSERT(singleId < mSinglesCapacity);
		PX_ASSERT(0xffffffff != singleId);
		PX_ASSERT(single);
		PX_UNUSED(single);

		return (PxcBpHandle)singleId;
	}

	PX_FORCE_INLINE void clearSingle(const PxU32 id)
	{
		PX_ASSERT(id < mSinglesCapacity);
		Single* single=getSingle(id);
		single->reset();
	}

	PX_FORCE_INLINE void reuseSingle(const PxU32 id)
	{
		//Reuse the aggregate id.
		PX_ASSERT(id<mSinglesCapacity);
		PX_ASSERT(mFreeIDsSize<mSinglesCapacity);
		mFreeIDs[mFreeIDsSize]=(PxcBpHandle)id;
		mFreeIDsSize++;
	}

private:

		//Single allocation buffer for singles and freeIds.		
		PxU8*					mBuffer;

		//Array of singles.
		Single*					mSingles;
		PxU32					mSinglesSize;
		PxU32					mSinglesCapacity;

		//Free single ids.
		PxcBpHandle*			mFreeIDs;	
		PxU32					mFreeIDsSize;
};

///////////////////////////////////////////////////////////////////////////////

struct PxsShapeCore;
class PxcRigidBody;
struct PxsRigidCore;

//PxcAABBDataStatic stores pointers to data structures necessary for the 
//computation of shape bounds of shapes attached to static actors.
//Each shape of a Single stored in BPElems or of an Aggregate stored in AggregateElems
//has a PxcAABBDataStatic instance associated with it if it is attached to a static 
//actor.
struct PxcAABBDataStatic
{
	void setEmpty()
	{
		mShapeCore = NULL;
		mRigidCore = NULL;
	}
	const PxsShapeCore* mShapeCore;
	const PxsRigidCore* mRigidCore;
};

//PxcAABBDataStatic stores pointers to data structures necessary for the 
//computation of shape bounds of shapes attached to dynamic actors.
//Each shape of a Single stored in BPElems or of an Aggregate stored in AggregateElems
//has a PxcAABBDataStatic instance associated with it if it is attached to a dynamic
//actor.
struct PxcAABBDataDynamic
{
	void setEmpty()
	{
		mShapeCore = NULL;
		mBodyAtom = NULL;
		mRigidCore = NULL;
		mLocalSpaceAABB = NULL;
	}
	const PxsShapeCore* mShapeCore;
	const PxcRigidBody* mBodyAtom;
	const PxsRigidCore* mRigidCore;
	const PxBounds3* mLocalSpaceAABB;
};

//A handy data structure that is passed around internally during the AABBmManager update.
struct PxsComputeAABBParams	
{
	PxU32 numCpuTasks;
	PxU32 numSpusAABB;
	PxU32 numSpusBP;
	bool secondBroadPhase;
	PxI32* numFastMovingShapes;
};

void updateBodyShapeAABBs
(const PxcBpHandle* PX_RESTRICT updatedAABBHandles, const PxU32 numUPdatedAABBHandles, 
 const PxcBpHandle* PX_RESTRICT aabbDataHandles, const PxcAABBDataStatic* PX_RESTRICT aabbData, 
 IntegerAABB* boundsIAABB, const PxU32 maxNumBounds);

PxU32 updateBodyShapeAABBs
(const PxcBpHandle* PX_RESTRICT updatedAABBHandles, const PxU32 numUPdatedAABBHandles, 
 const PxcBpHandle* PX_RESTRICT aabbDataHandles, const PxcAABBDataDynamic* PX_RESTRICT aabbData, 
 const bool secondBroadPhase,
 IntegerAABB* boundsIAABB, const PxU32 maxNumBounds);

//Compute the world-space bounds of a shape attached to a static actor
void PxsComputeAABB
(const PxcAABBDataStatic& aabbDataStatic,  PxBounds3& updatedBodyShapeBounds);

//Compute the world-space bounds of a shape attached to a dynamic actor
//Returns 1 if this is a fast-moving object
PxF32 PxsComputeAABB
(const bool secondBroadphase, const PxcAABBDataDynamic& aabbDataDynamic, PxBounds3& updatedBodyShapeBounds);

//Compute the local-space bounds of a shape attached to a static actor.
//This is used for actors that are in aggregates so that we can compute the 
//local-space bounds of the actor.
void PxsComputeAABBLocalSpace(const PxcAABBDataStatic& aabbData, PxBounds3& updatedBodyShapeBounds);

//Compute the local-space bounds of a shape attached to a dynamic actor.
//This is used for actors that are in aggregates so that we can compute the 
//local-space bounds of the actor.
void PxsComputeAABBLocalSpace(const PxcAABBDataDynamic& aabbData, PxBounds3& updatedBodyShapeBounds);

//Compute the world-space bounds of a static actor from its local-space bounds
void PxsComputeAABB(const PxsRigidCore* rigidCore, const PxBounds3& localSpaceActorBounds, PxBounds3& updatedWorldSpaceActorBounds);

//Compute the world-space bounds of a dynamic actor from its local-space bounds
PxF32 PxsComputeAABB
(const bool secondBroadphase, const PxcRigidBody* bodyAtom, const PxsRigidCore* rigidCore, const PxBounds3& localSpaceAtorBounds, PxBounds3& updatedWorldSpaceActorBounds);


///////////////////////////////////////////////////////////////////////////////

struct AggregatePair 
{
	PxcBpHandle		mBPElemId0;
	PxcBpHandle		mBPElemId1;
	Cm::BitMap*		aggregateCollBitmap;
};

///////////////////////////////////////////////////////////////////////////////

//ChangeList maintains a bitmap (and list) of all ids that have changed.
class ChangeList 
{
	PX_NOCOPY(ChangeList )

public:

	PX_FORCE_INLINE const PxcBpHandle* getElems() const {return mElems;}
	PX_FORCE_INLINE PxU32 getElemsSize() const {return mElemsSize;}

	ChangeList ()
		: mElems(NULL),
		  mElemsSize(0),
		  mElemsCapacity(0),
		  mDefaultElemsCapacity(64)
	{
	}

	~ChangeList ()
	{
		PX_FREE(mBitMap.getWords());
		PX_FREE(mElems);
	}

	Cm::BitMap mBitMap;
	PxcBpHandle* mElems;
	PxU32 mElemsSize;
	PxU32 mElemsCapacity;
	PxU32 mDefaultElemsCapacity;


	void setDefaultElemsCapacity(const PxU32 defaultCapacity)
	{
		mDefaultElemsCapacity=PxMax(mDefaultElemsCapacity,defaultCapacity);
		if(mDefaultElemsCapacity>mElemsCapacity)
		{
			growElems(mDefaultElemsCapacity);
		}
	}

	void free()
	{
		if(mElemsCapacity>mDefaultElemsCapacity)
		{
			PX_FREE(mElems);
			mElems=(PxcBpHandle*)PX_ALLOC(sizeof(PxcBpHandle)*mDefaultElemsCapacity, PX_DEBUG_EXP("AABBManager"));
			mElemsCapacity=mDefaultElemsCapacity;
		}
		mElemsSize=0;

		if(mBitMap.getWords())
		{
			PX_ASSERT(mBitMap.getWordCount());
			mBitMap.clearFast();
		}
	}

	PX_FORCE_INLINE void addElem(const PxcBpHandle id)
	{
		PX_ASSERT(!isInList(id));
		mBitMap.set(id);
	}

	PX_FORCE_INLINE void removeElem(const PxcBpHandle id)
	{
		PX_ASSERT(isInList(id));
		mBitMap.reset(id);
	}

	PX_FORCE_INLINE Ps::IntBool isInList(const PxcBpHandle id) const
	{
		PX_ASSERT(mBitMap.getWords() && mBitMap.getWordCount());
		return mBitMap.test(id);
	}

	void growBitmap(const PxU32 maxNumElems)
	{
		const PxU32 newBitMapWordCount = (((maxNumElems + 31) >> 5) + 3) & (~3);
		PX_ASSERT(newBitMapWordCount > 0);
		if(newBitMapWordCount > mBitMap.getWordCount())
		{
			PxU32* newBitMapWords=resizePODArray<PxU32>(mBitMap.getWordCount(), newBitMapWordCount,  mBitMap.getWords());
			mBitMap.setWords(newBitMapWords, newBitMapWordCount);
		}
	}

	void computeList()
	{
		if(NULL==mBitMap.getWords())
		{
			PX_ASSERT(0==mBitMap.getWordCount());
			return;
		}

		//Grow to the correct size.
		PxU32 numElems = mBitMap.count();
		if(numElems > mElemsCapacity)
		{
			growElems(numElems);
		}

		//Compute the list.
		numElems=0;
		Cm::BitMap::Iterator it(mBitMap);
		PxU32 index = it.getNext();
		while(index != Cm::BitMap::Iterator::DONE)
		{
			PX_ASSERT(numElems<mElemsCapacity);
			mElems[numElems]=(PxcBpHandle)index;
			numElems++;
			index = it.getNext();
		}
		mElemsSize=numElems;
	}

private:

	void growElems(const PxU32 _newCapacity)
	{
		PX_ASSERT(_newCapacity > mElemsCapacity);
		const PxU32 oldCapacity = mElemsCapacity;
		const PxU32 newCapacity = ((_newCapacity + 31) & ~31);
		mElems = resizePODArray<PxcBpHandle>(oldCapacity, newCapacity, mElems);
		mElemsCapacity = newCapacity;
	}
};

//AABBDataManager manages the entire collection of PxcAABBDataDynamic/PxcAABBDataStatic
//instances.  These are stored in the Elems class.
template <class T> class AABBDataManager
{
public:

	AABBDataManager()
		: mData(0),
		  mCapacity(0),
		  mFirstFreeElem(PX_INVALID_BP_HANDLE)
	{
	}

	~AABBDataManager()
	{
		PX_FREE(mData);
	}

	PX_FORCE_INLINE const T& get(const PxcBpHandle id) const {PX_ASSERT(id<mCapacity); return mData[id];}
	PX_FORCE_INLINE void set(const PxcBpHandle id, const T& aabbData) {PX_ASSERT(id<mCapacity); mData[id]=aabbData;}

	PX_FORCE_INLINE PxU32 getCapacity() const {return mCapacity;}

	PX_FORCE_INLINE PxcBpHandle getAvailableElem()
	{
		if(PX_INVALID_BP_HANDLE==mFirstFreeElem)
		{
			grow(PxMax(2*mCapacity, (PxU32)64));
		}

		const PxcBpHandle firstFreeElem=(PxcBpHandle)mFirstFreeElem;
		mFirstFreeElem=*(PxcBpHandle*)&mData[firstFreeElem];
		mData[firstFreeElem].setEmpty();
		return firstFreeElem;
	}

	PX_FORCE_INLINE void freeElem(const PxcBpHandle id) 
	{
		PX_ASSERT(id<mCapacity);
		(*(PxcBpHandle*)&mData[id])=(PxcBpHandle)mFirstFreeElem;
		mFirstFreeElem=id;
	}

	PX_FORCE_INLINE const T* getArray() const 
	{
		return mData;
	}

	PX_FORCE_INLINE void setDefaultCapacity(const PxU32 newCapacity)
	{
		if(mCapacity < newCapacity)
			grow(newCapacity);
	}

private:

	PxU32 countNumFreeElems() const
	{
		if(!mData)
		{
			PX_ASSERT(0 == mCapacity);
			return 0;
		}

		PxU32 count = 0;
		PxU32 freeElem = mFirstFreeElem;
		while(freeElem != PX_INVALID_BP_HANDLE)
		{
			count++;
			freeElem = *(PxcBpHandle*)&mData[freeElem];
		}
		return count;
	}

	void grow(const PxU32 newCapacity)
	{
#ifndef NDEBUG
		const PxU32 numFreeElemsStart = countNumFreeElems();
#endif
		const PxU32 oldCapacity = mCapacity;

		T* newData = (T*)PX_ALLOC(sizeof(T)*newCapacity, PX_DEBUG_EXP("AABBManager"));

		if(mData) copyPodArray<T>(newData, mData, newCapacity, oldCapacity);

		*(PxcBpHandle*)&newData[newCapacity-1]=(PxcBpHandle)mFirstFreeElem;
		for(PxU32 i=oldCapacity;i<(newCapacity-1);i++)
		{
			*(PxcBpHandle*)&newData[i] = (PxcBpHandle)(i+1);
		}
		mFirstFreeElem=mCapacity;

		PX_FREE(mData);
		mData=newData;

#ifndef NDEBUG
		const PxU32 numFreeElemsEnd = countNumFreeElems();
		PX_ASSERT((numFreeElemsStart + newCapacity-mCapacity) == numFreeElemsEnd);
#endif

		mCapacity = newCapacity;
	}

	T* mData;
	PxU32 mCapacity;
	PxU32 mFirstFreeElem;
};

///////////////////////////////////////////////////////////////////////////////

//The Elems class stores the entire collection of shapes for either 
//(i)  shapes that have broadphase entries or 
//(ii) shapes that are in aggregates. 
//This class is sub-classed with BPElems and AggregateElems to handle these cases.
class Elems
{
public:

	PX_FORCE_INLINE PxcBpHandle getFirstFreeElem() const { return (PxcBpHandle)mFirstFreeElem; }

	PX_FORCE_INLINE void freeElem(const PxcBpHandle id) 
	{
		const PxcBpHandle aabbDataHandle=mAABBDataHandles[id];
		if(PX_INVALID_BP_HANDLE!=aabbDataHandle)
		{
			const PxcBpHandle group=mGroups[id];
			if(0==group)
				mStaticAABBDataManager.freeElem(aabbDataHandle);
			else
				mDynamicAABBDataManager.freeElem(aabbDataHandle);
		}
		resetElem(id);

		PX_ASSERT(id < mCapacity);
		mGroups[id] = (PxcBpHandle)mFirstFreeElem;
		mFirstFreeElem = id;
	}

	PX_FORCE_INLINE PxcBpHandle useFirstFreeElem()
	{
		PX_ASSERT(PX_INVALID_BP_HANDLE != mFirstFreeElem);
		const PxU32 elemId = mFirstFreeElem;
		mFirstFreeElem = mGroups[mFirstFreeElem];
		resetElem(elemId);
		return (PxcBpHandle)elemId;
	}

	void setDefaultCapacity(const PxU32 nbStatics, const PxU32 nbDynamics)
	{
		if(nbStatics+nbDynamics > mCapacity)
		{
			grow(nbStatics+nbDynamics);
		}
		mStaticAABBDataManager.setDefaultCapacity(nbStatics);
		mDynamicAABBDataManager.setDefaultCapacity(nbDynamics);
	}

	void grow(const PxU32 newCapacity)
	{
		PX_ASSERT(newCapacity > mCapacity);
		PX_ASSERT(0==(newCapacity & 31));

#ifndef NDEBUG
		const PxU32 numFreeElemsStart=countNumFreeelems();
#endif

		const PxU32 sizeBounds = ALIGN_SIZE_16(newCapacity*sizeof(IntegerAABB)); 
		const PxU32 sizeVoidStars = ALIGN_SIZE_16(newCapacity*sizeof(void*));
		const PxU32 sizeHandles = ALIGN_SIZE_16(newCapacity*sizeof(PxcBpHandle));
		const PxU32 sizeAABBDataHandles = ALIGN_SIZE_16(newCapacity*sizeof(PxcBpHandle));
		const PxU32 newByteSize = sizeBounds + sizeVoidStars + sizeHandles +  sizeHandles +  sizeAABBDataHandles + sizeHandles;

		PxU8* newBuffer = (PxU8*)PX_ALLOC(newByteSize, PX_DEBUG_EXP("AABBManager"));

		mBounds = reallocateFromBuffer<IntegerAABB>(mCapacity, mBounds, newCapacity, newBuffer);

		mUserDatas = reallocateFromBuffer<void*>(mCapacity, mUserDatas, newCapacity, newBuffer);

		mGroups = reallocateFromBuffer<PxcBpHandle>(mCapacity, mGroups, newCapacity, newBuffer);

		mOwnerIds = reallocateFromBuffer<PxcBpHandle>(mCapacity, mOwnerIds, newCapacity, newBuffer);

		mAABBDataHandles = reallocateFromBuffer<PxcBpHandle>(mCapacity, mAABBDataHandles, newCapacity, newBuffer);

		mElemNextIds = reallocateFromBuffer<PxcBpHandle>(mCapacity, mElemNextIds, newCapacity, newBuffer);

		//Set up the free elements.
		mGroups[newCapacity-1]=(PxcBpHandle)mFirstFreeElem;
		for(PxU32 i=mCapacity;i<(newCapacity-1);i++)
		{
			mGroups[i]=(PxcBpHandle)(i+1);
		}
		mFirstFreeElem=mCapacity;

#ifndef NDEBUG
		const PxU32 numFreeElemsEnd=countNumFreeelems();
		PX_ASSERT(numFreeElemsStart + (newCapacity-mCapacity) == numFreeElemsEnd);
#endif

		PX_FREE(mBuffer);
		mBuffer = (PxU8*)mBounds;
		mCapacity = newCapacity;

	}

	PX_FORCE_INLINE void setAABB(const PxU32 bpElemId, const IntegerAABB& aabb){PX_ASSERT(bpElemId<mCapacity);	mBounds[bpElemId] = aabb;}
	PX_FORCE_INLINE void setUserData(const PxU32 id, void* userData) {PX_ASSERT(id < mCapacity); mUserDatas[id] = userData;}
	PX_FORCE_INLINE void setGroup(const PxU32 id, const PxcBpHandle group) { PX_ASSERT(id < mCapacity); mGroups[id] = group; }
	PX_FORCE_INLINE void setNextId(const PxU32 id, const PxcBpHandle nextId) { PX_ASSERT(id < mCapacity); mElemNextIds[id] = nextId;}

	PX_FORCE_INLINE void* getUserData(const PxU32 id) const { PX_ASSERT(id < mCapacity); return mUserDatas[id]; }
	PX_FORCE_INLINE PxcBpHandle getGroup(const PxU32 id) const { PX_ASSERT(id < mCapacity); return mGroups[id]; }
	PX_FORCE_INLINE PxcBpHandle getNextId(const PxU32 id) const { PX_ASSERT(id < mCapacity); return mElemNextIds[id];}
	PX_FORCE_INLINE const IntegerAABB& getAABB(const PxU32 bpElemId) const{PX_ASSERT(bpElemId<mCapacity);return mBounds[bpElemId];}

	PX_FORCE_INLINE void setEmptyAABB(const PxU32 id) 
	{ 
		PX_ASSERT(id < mCapacity); 
		mAABBDataHandles[id]=PX_INVALID_BP_HANDLE;
	}

	PX_FORCE_INLINE const PxcBpHandle* getGroups() const { return mGroups; }
	PX_FORCE_INLINE const PxcBpHandle* getAABBDataHandles() const {return mAABBDataHandles;}
	PX_FORCE_INLINE IntegerAABB* getBounds() const {return mBounds;}
	PX_FORCE_INLINE PxU32 getCapacity() const { return mCapacity; }


	PX_FORCE_INLINE void setDynamicAABBData(const PxcBpHandle id, const PxcAABBDataDynamic& aabbData)
	{
		PX_ASSERT(id<mCapacity);
		PX_ASSERT(0!=mGroups[id]);
		const PxcBpHandle handle=mDynamicAABBDataManager.getAvailableElem();
		mDynamicAABBDataManager.set(handle,aabbData);
		mAABBDataHandles[id]=handle;
	}
	PX_FORCE_INLINE void setStaticAABBData(const PxcBpHandle id, const PxcAABBDataStatic& aabbData)
	{
		PX_ASSERT(id<mCapacity);
		PX_ASSERT(0==mGroups[id]);
		const PxcBpHandle handle=mStaticAABBDataManager.getAvailableElem();
		mStaticAABBDataManager.set(handle,aabbData);
		mAABBDataHandles[id]=handle;
	}
	PX_FORCE_INLINE const PxcRigidBody* getBodyAtom(const PxcBpHandle id) const
	{
		PX_ASSERT(id<mCapacity);
		return (0==mGroups[id] ? NULL : mDynamicAABBDataManager.get(mAABBDataHandles[id]).mBodyAtom); 
	}
	PX_FORCE_INLINE const PxsShapeCore* getShapeCore(const PxcBpHandle id) const
	{
		PX_ASSERT(id<mCapacity);
		return (0==mGroups[id] ? mStaticAABBDataManager.get(mAABBDataHandles[id]).mShapeCore : mDynamicAABBDataManager.get(mAABBDataHandles[id]).mShapeCore);
	}

	PX_FORCE_INLINE const PxcAABBDataStatic* getStaticAABBDataArray() const {return mStaticAABBDataManager.getArray();}
	PX_FORCE_INLINE PxU32 getStaticAABBDataArrayCapacity() const {return mStaticAABBDataManager.getCapacity();}
	PX_FORCE_INLINE const PxcAABBDataDynamic* getDynamicAABBDataArray() const {return mDynamicAABBDataManager.getArray();}
	PX_FORCE_INLINE PxU32 getDynamicAABBDataArrayCapacity() const {return mDynamicAABBDataManager.getCapacity();}

	PX_FORCE_INLINE const PxcAABBDataStatic& getStaticAABBData(const PxcBpHandle id) const
	{
		PX_ASSERT(id<mCapacity);
		PX_ASSERT(0==mGroups[id]);
		return mStaticAABBDataManager.get(mAABBDataHandles[id]);
	}
	PX_FORCE_INLINE const PxcAABBDataDynamic& getDynamicAABBData(const PxcBpHandle id) const
	{
		PX_ASSERT(id<mCapacity);
		PX_ASSERT(0!=mGroups[id]);
		PX_ASSERT(PX_INVALID_BP_HANDLE!=mAABBDataHandles[id]);
		return mDynamicAABBDataManager.get(mAABBDataHandles[id]);
	}

protected:

	Elems()
		: mBuffer(NULL), 
		mBounds(NULL),
		mUserDatas(NULL),
		mGroups(NULL),
		mOwnerIds(NULL),
		mElemNextIds(NULL),
		mAABBDataHandles(NULL),
		mCapacity(0),
		mFirstFreeElem(PX_INVALID_BP_HANDLE)
	{
	}

	~Elems()
	{
	}

	void free()
	{
		PX_FREE(mBuffer);
	}

	PxU8* mBuffer;
	IntegerAABB* mBounds;
	void** mUserDatas;
	PxcBpHandle* mGroups;
	PxcBpHandle* mOwnerIds; //id of aggregate array if it is an aggregate, id of single array if it is a single
	PxcBpHandle* mElemNextIds;	//definitely need this
	PxcBpHandle* mAABBDataHandles;
	PxU32 mCapacity;
	PxU32 mFirstFreeElem;

	AABBDataManager<PxcAABBDataStatic> mStaticAABBDataManager;
	AABBDataManager<PxcAABBDataDynamic> mDynamicAABBDataManager;

private:

	PX_FORCE_INLINE void resetElem(const PxU32 id)
	{
		PX_ASSERT(id<mCapacity);
		mBounds[id].setEmpty();
		mUserDatas[id] = NULL;
		mGroups[id] = PX_INVALID_BP_HANDLE;
		mOwnerIds[id] = PX_INVALID_BP_HANDLE;
		mAABBDataHandles[id] = PX_INVALID_BP_HANDLE;
		mElemNextIds[id] = PX_INVALID_BP_HANDLE;
	}

	PxU32 countNumFreeelems() const
	{
		if(!mGroups)
		{
			PX_ASSERT(0==mCapacity);
			return 0;
		}

		PxU32 count=0;
		PxU32 freeElem=mFirstFreeElem;
		while(freeElem!=PX_INVALID_BP_HANDLE)
		{
			count++;
			freeElem=mGroups[freeElem];
		}
		return count;
	}
};

//The BPElems class stores the entire collection of shapes for shapes 
//that have broadphase entries.  Additionally, it stores the broadphase entry 
//for aggregates.
class BPElems : public Elems
{
public:

	BPElems() 
		: Elems()
	{
	}

	~BPElems()
	{
		free();
	}

	//If a broadphase entry is the bounds of an aggregate then set/get the id of the corresponding Aggregate.
	PX_FORCE_INLINE void setAggregateOwnerId(const PxU32 id, const PxcBpHandle aggregateOwnerId) { PX_ASSERT(id < mCapacity); PX_ASSERT(aggregateOwnerId != PX_INVALID_BP_HANDLE); mOwnerIds[id] = encodeAggregateForClient(aggregateOwnerId); }
	PX_FORCE_INLINE PxcBpHandle getAggregateOwnerId(const PxU32 id) const { PX_ASSERT(id < mCapacity); PX_ASSERT(isOwnerAggregate(id)); return decodeAggregateFromClient(mOwnerIds[id]); }

	//If a broadphase entry is the bounds of an actor shape then set/get the id of the corresponding Single.
	PX_FORCE_INLINE void setSingleOwnerId(const PxU32 id, const PxcBpHandle singleOwnerId) { PX_ASSERT(id < mCapacity); PX_ASSERT(singleOwnerId != PX_INVALID_BP_HANDLE); mOwnerIds[id] = encodeSingleForClient(singleOwnerId); }
	PX_FORCE_INLINE PxcBpHandle getSingleOwnerId(const PxU32 id) const { PX_ASSERT(id < mCapacity); PX_ASSERT(!isOwnerAggregate(id)); return decodeSingleFromClient(mOwnerIds[id]); }

	//Return true if the broadphase entry represents the bounds of an aggregate.
	//Return false if the broadphase entry represents the bounds of a single shape.
	PX_FORCE_INLINE Ps::IntBool isOwnerAggregate(const PxU32 id) const 
	{ 
		PX_ASSERT(id < mCapacity); 
		PX_ASSERT(mOwnerIds[id] != PX_INVALID_BP_HANDLE); 
		return isClientVolumeAggregate(mOwnerIds[id]);
	}

	//Return false if the broadphase entry represents the bounds of an aggregate.
	//Return true if the broadphase entry represents the bounds of a single shape.
	PX_FORCE_INLINE bool isSingle(const PxcBpHandle id) const 
	{
		PX_ASSERT(id < mCapacity);
		PX_ASSERT( !isOwnerAggregate(id) == (mUserDatas[id] ? true : false));
		return !isOwnerAggregate(id);
	}

	PX_FORCE_INLINE bool isValid(const PxcBpHandle id) const 
	{
		PX_ASSERT(id < mCapacity);
		return mOwnerIds[id] != PX_INVALID_BP_HANDLE;
	}

	//Initialise the broadphase bounds if it represents the bounds of an individual shape.
	PX_FORCE_INLINE void initAsSingle(const PxU32 bpElemId, void* userData, const PxcBpHandle group, const IntegerAABB& aabb)
	{
		setUserData(bpElemId, userData);
		setGroup(bpElemId,group);
		setAABB(bpElemId,aabb);
		PX_ASSERT(bpElemId < mCapacity);
		PX_ASSERT(PX_INVALID_BP_HANDLE==mOwnerIds[bpElemId]);
	}

	//Initialise the broadphase bounds if it represents the bounds of an aggregate.
	PX_FORCE_INLINE void initAsAggregate(const PxU32 bpElemId, const IntegerAABB& aabb)
	{
		setAABB(bpElemId,aabb);
		PX_ASSERT(NULL == getUserData(bpElemId));
		PX_ASSERT(PX_INVALID_BP_HANDLE != getGroup(bpElemId));
		PX_ASSERT(bpElemId < mCapacity);
		PX_ASSERT(PX_INVALID_BP_HANDLE!=mOwnerIds[bpElemId]);
		PX_ASSERT(isOwnerAggregate(bpElemId));
	}

private:
};

//The AggregateElems class stores the entire collection of shapes for  
//shapes that are in aggregates.
class AggregateElems : public Elems
{
public:

	AggregateElems()
		: Elems()
	{
	}

	~AggregateElems()
	{
		free();
	}

	//Set the id of the corresponding actor stored in AggregateActorManager
	PX_FORCE_INLINE void setAggregateActorId(const PxcBpHandle id, const PxcBpHandle actorId)
	{
		PX_ASSERT(id < mCapacity); 
		PX_ASSERT(actorId != PX_INVALID_BP_HANDLE); 
		mOwnerIds[id] = encodeAggregateForClient(actorId); 
	}
	//Get the id of the corresponding actor stored in AggregateActorManager
	PX_FORCE_INLINE PxcBpHandle getAggregateActorId(const PxcBpHandle id) const
	{
		PX_ASSERT(id < mCapacity); 
		return decodeAggregateFromClient(mOwnerIds[id]); 
	}

	PX_FORCE_INLINE void reinit(const PxcBpHandle id, void* userData, const PxcBpHandle group, const IntegerAABB& aabb)
	{
		PX_ASSERT(id < getCapacity());
		setUserData(id,userData);
		setGroup(id, group);
		setAABB(id,aabb);
		mAABBDataHandles[id]=PX_INVALID_BP_HANDLE;
	}

	PX_FORCE_INLINE void init(const PxcBpHandle id, void* userData, const PxcBpHandle group, const IntegerAABB& aabb, const PxcBpHandle nextId)
	{
		PX_ASSERT(id < getCapacity());
		setUserData(id,userData);
		setGroup(id, group);
		setAABB(id,aabb);
		setNextId(id,nextId);
		mAABBDataHandles[id]=PX_INVALID_BP_HANDLE;
	}

	void resize(const PxU32 nb)
	{
		if(nb > mCapacity)
		{
			mFirstFreeElem = PX_INVALID_BP_HANDLE;
			grow(nb);
		}
	}

private:
};

//A structure used to accelerate self-, aggregate-aggregate and aggregate-single overlaps.
struct AggregateSortedData
{
	PxcBpHandle* rankIds;
	PxcBpHandle* elemIds;
};

//A handy class that stores a fixed-size array and performs an allocation
//only if the required size is greater than the fixed-size.  The allocation
//is performed using the scratch allocator if this is available.
template<class T, PxU32 STACKSIZE> class InlineBuffer
{
	PX_NOCOPY(InlineBuffer)

public:

	InlineBuffer(PxcScratchAllocator* scratchAllocator = NULL)
		: mScratchAllocator(scratchAllocator),
		  mBuffer(NULL),
		  mBufferSize(0)
	{
	}
	~InlineBuffer()
	{
		free();
	}

	void resize(const PxU32 requiredSize)
	{
		PX_ASSERT(!mBuffer);
		PX_ASSERT(!mBufferSize);

		if(0==requiredSize)
			return;

		if(requiredSize > STACKSIZE)
		{
			const PxU32 byteSize = ((sizeof(T)*requiredSize + 15) & ~15);
			if(mScratchAllocator)
			{
				mBuffer = (T*)mScratchAllocator->alloc(byteSize, true);  
			}
			else
			{
				mBuffer = (T*)PX_ALLOC(byteSize, PX_DEBUG_EXP("PxsAABBManager"));
			}
		}
		else
		{
			mBuffer = mStackBuffer;
		}
		PX_ASSERT(0 == (((size_t)mBuffer) & 0x0f));

		PxMemZero(mBuffer, sizeof(T)*requiredSize);
		mBufferSize = requiredSize;
	}

	void free()
	{
		if(mBuffer && mBuffer != mStackBuffer)
		{
			if(mScratchAllocator)
			{
				mScratchAllocator->free(mBuffer);
			}
			else
			{
				PX_FREE(mBuffer);
			}
		}
		mBuffer = NULL;
		mBufferSize = 0;
	}

private:
	T PX_ALIGN(16, mStackBuffer[STACKSIZE]);
	PxcScratchAllocator* mScratchAllocator;

public:

	T* mBuffer;
	PxU32 mBufferSize;
};

//A handy class that stores a fixed-size array of bitmap words and performs an allocation
//only if the required size is greater than the fixed-size.  The allocation
//is performed using the scratch allocator if this is available.
template<PxU32 STACKSIZE> class InlineBitmap
{
	PX_NOCOPY(InlineBitmap)

public:

	InlineBitmap(const PxU32 requiredSize, PxcScratchAllocator* scratchAllocator = NULL)
		: mScratchAllocator(scratchAllocator)
	{
		PX_ASSERT(!mBitmap.getWords());
		PX_ASSERT(!mBitmap.getWordCount());

		if(0==requiredSize)
			return;

		const PxU32 wordCount = ((requiredSize + 31) >> 5);
		if(requiredSize > STACKSIZE)
		{
			const PxU32 byteSize = ((sizeof(PxU32)*wordCount + 15) & ~15);
			if(mScratchAllocator)
			{
				mBitmap.setWords((PxU32*)mScratchAllocator->alloc(byteSize, true), wordCount);  
			}
			else
			{
				mBitmap.setWords((PxU32*)PX_ALLOC(byteSize, PX_DEBUG_EXP("PxsAABBManager")), wordCount);  
			}
		}
		else
		{
			mBitmap.setWords(mStackBuffer, wordCount);
		}
		PX_ASSERT(0 == (((size_t)mBitmap.getWords()) & 0x0f));
		resetAll();
	}
	~InlineBitmap()
	{
		free();
	}

	void free()
	{
		if(mBitmap.getWords() && (mBitmap.getWords() != mStackBuffer))
		{
			if(mScratchAllocator)
			{
				mScratchAllocator->free(mBitmap.getWords());
			}
			else
			{
				PX_FREE(mBitmap.getWords());
			}
		}
		mBitmap.setWords(NULL, 0);
	}

	PX_FORCE_INLINE void set(const PxU32 id) {mBitmap.set(id);}
	PX_FORCE_INLINE void reset(const PxU32 id) {mBitmap.reset(id);}
	PX_FORCE_INLINE void resetAll() {PxMemZero(mBitmap.getWords(), sizeof(PxU32)*mBitmap.getWordCount());}
	PX_FORCE_INLINE Ps::IntBool test(const PxU32 id) const {return mBitmap.test(id);}
	PX_FORCE_INLINE PxU32 findLast() const {return mBitmap.findLast();}
	PX_FORCE_INLINE const PxU32* getWords() const {return mBitmap.getWords();}
	PX_FORCE_INLINE PxU32 getWordCount() const {return mBitmap.getWordCount();}

private:

	PxU32 PX_ALIGN(16, mStackBuffer[(STACKSIZE + 31) >> 5]);
	PxcScratchAllocator* mScratchAllocator;

public:

	Cm::BitMap mBitmap;
};


} //namespace physx

#endif //PXS_AABB_MANAGER_AUX_H
