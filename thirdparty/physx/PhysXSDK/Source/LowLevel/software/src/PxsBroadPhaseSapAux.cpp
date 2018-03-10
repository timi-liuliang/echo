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

#include "CmPhysXCommon.h"
#include "PxsBroadPhaseSapAux.h"
#include "PxsBroadPhaseCommon.h"
#include "PxsBroadPhaseConfig.h"
#include "GuRevisitedRadixBuffered.h"

using namespace physx;

PX_FORCE_INLINE void PxBpHandleSwap(PxcBpHandle& a, PxcBpHandle& b)													
{ 
	const PxcBpHandle c = a; a = b; b = c;		
}

PX_FORCE_INLINE void Sort(PxcBpHandle& id0, PxcBpHandle& id1)										
{ 
	if(id0>id1)	PxBpHandleSwap(id0, id1);						
}

PX_FORCE_INLINE bool DifferentPair(const PxcBroadPhasePair& p, PxcBpHandle id0, PxcBpHandle id1)	
{ 
	return (id0!=p.mVolA) || (id1!=p.mVolB);						
}

PX_FORCE_INLINE int Hash32Bits_1(int key)
{
	key += ~(key << 15);
	key ^=  (key >> 10);
	key +=  (key << 3);
	key ^=  (key >> 6);
	key += ~(key << 11);
	key ^=  (key >> 16);
	return key;
}

PX_FORCE_INLINE PxU32 Hash(PxcBpHandle id0, PxcBpHandle id1)								
{ 
	return (PxU32)Hash32Bits_1( int(PxU32(id0)|(PxU32(id1)<<16)) );		
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SapPairManager::SapPairManager() :
	mHashTable				(NULL),
	mNext					(NULL),
	mHashSize				(0),
	mHashCapacity			(0),
	mMinAllowedHashCapacity	(0),
	mActivePairs			(NULL),
	mActivePairStates		(NULL),
	mNbActivePairs			(0),
	mActivePairsCapacity	(0),
	mMask					(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SapPairManager::~SapPairManager()
{
#ifndef __SPU__
	PX_ASSERT(NULL==mHashTable);
	PX_ASSERT(NULL==mNext);
	PX_ASSERT(NULL==mActivePairs);
	PX_ASSERT(NULL==mActivePairStates);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void SapPairManager::init(const PxU32 size)
{
#ifndef __SPU__
	#define ALIGN_SIZE_16(size) (((unsigned)(size)+15)&((unsigned)~15))
	mHashTable=(PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16(sizeof(PxcBpHandle)*size), PX_DEBUG_EXP("PxBpHandle"));
	mNext=(PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16(sizeof(PxcBpHandle)*size), PX_DEBUG_EXP("PxBpHandle"));
	mActivePairs=(PxcBroadPhasePair*)PX_ALLOC(ALIGN_SIZE_16(sizeof(PxcBroadPhasePair)*size), PX_DEBUG_EXP("PxcBroadPhasePair"));
	mActivePairStates=(PxU8*)PX_ALLOC(ALIGN_SIZE_16(sizeof(PxU8)*size), PX_DEBUG_EXP("PxsBroadPhaseContextSap ActivePairStates"));
	mHashCapacity=size;
	mMinAllowedHashCapacity = size;
	mActivePairsCapacity=size;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SapPairManager::release()
{
#ifndef __SPU__
	PX_FREE(mHashTable);
	PX_FREE(mNext);
	PX_FREE(mActivePairs);
	PX_FREE(mActivePairStates);
	mHashTable				= NULL;
	mNext					= NULL;
	mActivePairs			= NULL;
	mActivePairStates		= NULL;
	mNext					= 0;
	mHashSize				= 0;
	mHashCapacity			= 0;
	mMinAllowedHashCapacity	= 0;
	mNbActivePairs			= 0;
	mActivePairsCapacity	= 0;
	mMask					= 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const PxcBroadPhasePair* SapPairManager::FindPair(PxcBpHandle id0, PxcBpHandle id1) const
{
	if(0==mHashSize) return NULL;	// Nothing has been allocated yet

	// Order the ids
	Sort(id0, id1);

	// Compute hash value for this pair
	PxU32 HashValue = Hash(id0, id1) & mMask;
	PX_ASSERT(HashValue<mHashCapacity);

	// Look for it in the table
	PX_ASSERT(HashValue<mHashCapacity);
	PxU32 Offset = mHashTable[HashValue];
	PX_ASSERT(PX_INVALID_BP_HANDLE==Offset || Offset<mActivePairsCapacity);
	while(Offset!=PX_INVALID_BP_HANDLE && DifferentPair(mActivePairs[Offset], id0, id1))
	{
		PX_ASSERT(mActivePairs[Offset].mVolA!=PX_INVALID_BP_HANDLE);
		PX_ASSERT(Offset<mHashCapacity);
		Offset = mNext[Offset];		// Better to have a separate array for this
		PX_ASSERT(PX_INVALID_BP_HANDLE==Offset || Offset<mActivePairsCapacity);
	}
	if(Offset==PX_INVALID_BP_HANDLE)	return NULL;
	PX_ASSERT(Offset<mNbActivePairs);
	// Match mActivePairs[Offset] => the pair is persistent
	PX_ASSERT(Offset<mActivePairsCapacity);
	return &mActivePairs[Offset];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Internal version saving hash computation
PX_FORCE_INLINE PxcBroadPhasePair* SapPairManager::FindPair(PxcBpHandle id0, PxcBpHandle id1, PxU32 hash_value) const
{
	if(0==mHashSize) return NULL;	// Nothing has been allocated yet

	// Look for it in the table
	PX_ASSERT(hash_value<mHashCapacity);
	PxU32 Offset = mHashTable[hash_value];
	PX_ASSERT(PX_INVALID_BP_HANDLE==Offset || Offset<mActivePairsCapacity);
	while(Offset!=PX_INVALID_BP_HANDLE && DifferentPair(mActivePairs[Offset], id0, id1))
	{
		PX_ASSERT(mActivePairs[Offset].mVolA!=PX_INVALID_BP_HANDLE);
		PX_ASSERT(Offset<mHashCapacity);
		Offset = mNext[Offset];		// Better to have a separate array for this
		PX_ASSERT(PX_INVALID_BP_HANDLE==Offset || Offset<mActivePairsCapacity);
	}
	if(Offset==PX_INVALID_BP_HANDLE)	return NULL;
	PX_ASSERT(Offset<mNbActivePairs);
	// Match mActivePairs[Offset] => the pair is persistent
	PX_ASSERT(Offset<mActivePairsCapacity);
	return &mActivePairs[Offset];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const PxcBroadPhasePair* SapPairManager::AddPair(PxcBpHandle id0, PxcBpHandle id1, const PxU8 state)
{
	if(MAX_BP_HANDLE == mNbActivePairs)
	{
		PX_WARN_ONCE(true, MAX_BP_PAIRS_MESSAGE);
		return NULL;
	}

	// Order the ids
	Sort(id0, id1);

	PxU32 HashValue = Hash(id0, id1) & mMask;

	PxcBroadPhasePair* P = FindPair(id0, id1, HashValue);
	if(P)
	{
		return P;	// Persistent pair
	}

#ifdef __SPU__
	//Make sure that we never grow the hash table beyond the capacity on spu.
	//We can't do this on spu because we'd need to re-size the hash table in main memory too.
	//Also, we're running out of memory on spu and simply can't afford the extra buffer space.
	if(mNbActivePairs >= mHashSize)
	{
		const PxU32 newHashSize=Ps::nextPowerOfTwo(mNbActivePairs+1);
		if(newHashSize > mHashCapacity)
		{
			return NULL;
		}
	}
#endif

	// This is a new pair
	if(mNbActivePairs >= mHashSize)
	{
		// Get more entries
		mHashSize = Ps::nextPowerOfTwo(mNbActivePairs+1);
		mMask = mHashSize-1;

		reallocPairs(mHashSize>mHashCapacity);

		// Recompute hash value with new hash size
		HashValue = Hash(id0, id1) & mMask;
	}

	PX_ASSERT(mNbActivePairs<mActivePairsCapacity);
	PxcBroadPhasePair* p = &mActivePairs[mNbActivePairs];
	p->mVolA		= id0;	// ### CMOVs would be nice here
	p->mVolB		= id1;
	mActivePairStates[mNbActivePairs]=state;

	PX_ASSERT(mNbActivePairs<mHashSize);
	PX_ASSERT(mNbActivePairs<mHashCapacity);
	PX_ASSERT(HashValue<mHashCapacity);
	mNext[mNbActivePairs] = mHashTable[HashValue];
	mHashTable[HashValue] = (PxcBpHandle)mNbActivePairs++;
	return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SapPairManager::RemovePair(PxcBpHandle /*id0*/, PxcBpHandle /*id1*/, PxU32 hash_value, PxU32 pair_index)
{
	// Walk the hash table to fix mNext
	{
		PX_ASSERT(hash_value<mHashCapacity);
		PxU32 Offset = mHashTable[hash_value];
		PX_ASSERT(Offset!=PX_INVALID_BP_HANDLE);

		PxU32 Previous=PX_INVALID_BP_HANDLE;
		while(Offset!=pair_index)
		{
			Previous = Offset;
			PX_ASSERT(Offset<mHashCapacity);
			Offset = mNext[Offset];
		}

		// Let us go/jump us
		if(Previous!=PX_INVALID_BP_HANDLE)
		{
			PX_ASSERT(Previous<mHashCapacity);
			PX_ASSERT(pair_index<mHashCapacity);
			PX_ASSERT(mNext[Previous]==pair_index);
			mNext[Previous] = mNext[pair_index];
		}
		// else we were the first
		else
		{
			PX_ASSERT(hash_value<mHashCapacity);
			PX_ASSERT(pair_index<mHashCapacity);
			mHashTable[hash_value] = mNext[pair_index];
		}
	}
	// we're now free to reuse mNext[PairIndex] without breaking the list

#ifdef PX_DEBUG
	PX_ASSERT(pair_index<mHashCapacity);
	mNext[pair_index]=PX_INVALID_BP_HANDLE;
#endif
	// Invalidate entry

	// Fill holes
	if(1)
	{
		// 1) Remove last pair
		const PxU32 LastPairIndex = mNbActivePairs-1;
		if(LastPairIndex==pair_index)
		{
			mNbActivePairs--;
		}
		else
		{
			PX_ASSERT(LastPairIndex<mActivePairsCapacity);
			const PxcBroadPhasePair* Last = &mActivePairs[LastPairIndex];
			const PxU32 LastHashValue = Hash(Last->mVolA, Last->mVolB) & mMask;

			// Walk the hash table to fix mNext
			PX_ASSERT(LastHashValue<mHashCapacity);
			PxU32 Offset = mHashTable[LastHashValue];
			PX_ASSERT(Offset!=PX_INVALID_BP_HANDLE);

			PxU32 Previous=PX_INVALID_BP_HANDLE;
			while(Offset!=LastPairIndex)
			{
				Previous = Offset;
				PX_ASSERT(Offset<mHashCapacity);
				Offset = mNext[Offset];
			}

			// Let us go/jump us
			if(Previous!=PX_INVALID_BP_HANDLE)
			{
				PX_ASSERT(Previous<mHashCapacity);
				PX_ASSERT(LastPairIndex<mHashCapacity);
				PX_ASSERT(mNext[Previous]==LastPairIndex);
				mNext[Previous] = mNext[LastPairIndex];
			}
			// else we were the first
			else
			{
				PX_ASSERT(LastHashValue<mHashCapacity);
				PX_ASSERT(LastPairIndex<mHashCapacity);
				mHashTable[LastHashValue] = mNext[LastPairIndex];
			}
			// we're now free to reuse mNext[LastPairIndex] without breaking the list

#ifdef PX_DEBUG
			PX_ASSERT(LastPairIndex<mHashCapacity);
			mNext[LastPairIndex]=PX_INVALID_BP_HANDLE;
#endif

			// Don't invalidate entry since we're going to shrink the array

			// 2) Re-insert in free slot
			PX_ASSERT(pair_index<mActivePairsCapacity);
			PX_ASSERT(LastPairIndex<mActivePairsCapacity);
			mActivePairs[pair_index] = mActivePairs[LastPairIndex];
			mActivePairStates[pair_index] = mActivePairStates[LastPairIndex];
#ifdef PX_DEBUG
			PX_ASSERT(pair_index<mHashCapacity);
			PX_ASSERT(mNext[pair_index]==PX_INVALID_BP_HANDLE);
#endif
			PX_ASSERT(pair_index<mHashCapacity);
			PX_ASSERT(LastHashValue<mHashCapacity);
			mNext[pair_index] = mHashTable[LastHashValue];
			mHashTable[LastHashValue] = (PxcBpHandle)pair_index;

			mNbActivePairs--;
		}
	}
}

bool SapPairManager::RemovePair(PxcBpHandle id0, PxcBpHandle id1)
{
	// Order the ids
	Sort(id0, id1);

	const PxU32 HashValue = Hash(id0, id1) & mMask;
	const PxcBroadPhasePair* P = FindPair(id0, id1, HashValue);
	if(!P)	return false;
	PX_ASSERT(P->mVolA==id0);
	PX_ASSERT(P->mVolB==id1);

	RemovePair(id0, id1, HashValue, GetPairIndex(P));

	shrinkMemory();

	return true;
}

bool SapPairManager::RemovePairs(const Cm::BitMap& removedAABBs)
{
	PxU32 i=0;
	while(i<mNbActivePairs)
	{
		const PxcBpHandle id0 = mActivePairs[i].mVolA;
		const PxcBpHandle id1 = mActivePairs[i].mVolB;
		if(removedAABBs.test(id0) || removedAABBs.test(id1))
		{
			const PxU32 HashValue = Hash(id0, id1) & mMask;
			RemovePair(id0, id1, HashValue, i);
		}
		else i++;
	}
	return true;
}

void SapPairManager::shrinkMemory()
{
	//Compute the hash size given the current number of active pairs.
	const PxU32 correctHashSize = Ps::nextPowerOfTwo(mNbActivePairs);

	//If we have the correct hash size then no action required.
	if(correctHashSize==mHashSize || (correctHashSize < mMinAllowedHashCapacity && mHashSize == mMinAllowedHashCapacity))	
		return;

	//The hash size can be reduced so take action.
	//Don't let the hash size fall below a threshold value.
	PxU32 newHashSize = correctHashSize;
	if(newHashSize < mMinAllowedHashCapacity)
	{
		newHashSize = mMinAllowedHashCapacity;
	}
	mHashSize = newHashSize;
	mMask = newHashSize-1;

	reallocPairs( (newHashSize > mMinAllowedHashCapacity) || (mHashSize <= (mHashCapacity >> 2)) || (mHashSize <= (mActivePairsCapacity >> 2)));
}

#ifdef __SPU__

void SapPairManager::reallocPairs(const bool allocRequired)
{
	for(PxU32 i=0;i<mHashSize;i++)	
	{
		mHashTable[i] = PX_INVALID_BP_HANDLE;
	}

	// ### check it's actually needed... probably only for pairs whose hash value was cut by the and
	// yeah, since Hash(id0, id1) is a constant
	// However it might not be needed to recompute them => only less efficient but still ok
	for(PxU32 i=0;i<mNbActivePairs;i++)
	{
		const PxU32 HashValue = Hash(mActivePairs[i].mVolA, mActivePairs[i].mVolB) & mMask;	// New hash value with new mask
		mNext[i] = mHashTable[HashValue];
		PX_ASSERT(HashValue<mHashCapacity);
		mHashTable[HashValue] = i;
	}
}

#else 

void SapPairManager::reallocPairs(const bool allocRequired)
{
	if(allocRequired)
	{
		PX_FREE(mHashTable);
		mHashCapacity=mHashSize;
		mActivePairsCapacity=mHashSize;
		mHashTable = (PxcBpHandle*)PX_ALLOC(mHashSize*sizeof(PxcBpHandle), PX_DEBUG_EXP("PxBpHandle"));

		for(PxU32 i=0;i<mHashSize;i++)	
		{
			mHashTable[i] = PX_INVALID_BP_HANDLE;
		}

		// Get some bytes for new entries
		PxcBroadPhasePair* NewPairs	= (PxcBroadPhasePair*)PX_ALLOC(mHashSize * sizeof(PxcBroadPhasePair), PX_DEBUG_EXP("PxcBroadPhasePair"));	PX_ASSERT(NewPairs);
		PxcBpHandle* NewNext		= (PxcBpHandle*)PX_ALLOC(mHashSize * sizeof(PxcBpHandle), PX_DEBUG_EXP("PxBpHandle"));		PX_ASSERT(NewNext);
		PxU8* NewPairStates = (PxU8*)PX_ALLOC(mHashSize * sizeof(PxU8), PX_DEBUG_EXP("SapPairStates")); PX_ASSERT(NewPairStates);

		// Copy old data if needed
		if(mNbActivePairs) PxMemCopy(NewPairs, mActivePairs, mNbActivePairs*sizeof(PxcBroadPhasePair));
		if(mNbActivePairs) PxMemCopy(NewPairStates, mActivePairStates, mNbActivePairs*sizeof(PxU8));

		// ### check it's actually needed... probably only for pairs whose hash value was cut by the and
		// yeah, since Hash(id0, id1) is a constant
		// However it might not be needed to recompute them => only less efficient but still ok
		for(PxU32 i=0;i<mNbActivePairs;i++)
		{
			const PxU32 HashValue = Hash(mActivePairs[i].mVolA, mActivePairs[i].mVolB) & mMask;	// New hash value with new mask
			NewNext[i] = mHashTable[HashValue];
			PX_ASSERT(HashValue<mHashCapacity);
			mHashTable[HashValue] = (PxcBpHandle)i;
		}

		// Delete old data
		PX_FREE(mNext);
		PX_FREE(mActivePairs);
		PX_FREE(mActivePairStates);

		// Assign new pointer
		mActivePairs = NewPairs;
		mActivePairStates = NewPairStates;
		mNext = NewNext;
	}
	else
	{
		for(PxU32 i=0;i<mHashSize;i++)	
		{
			mHashTable[i] = PX_INVALID_BP_HANDLE;
		}

		// ### check it's actually needed... probably only for pairs whose hash value was cut by the and
		// yeah, since Hash(id0, id1) is a constant
		// However it might not be needed to recompute them => only less efficient but still ok
		for(PxU32 i=0;i<mNbActivePairs;i++)
		{
			const PxU32 HashValue = Hash(mActivePairs[i].mVolA, mActivePairs[i].mVolB) & mMask;	// New hash value with new mask
			mNext[i] = mHashTable[HashValue];
			PX_ASSERT(HashValue<mHashCapacity);
			mHashTable[HashValue] = (PxcBpHandle)i;
		}
	}
}

#endif

#ifndef __SPU__
void resizeCreatedDeleted(PxcBroadPhasePair*& pairs, PxU32& maxNumPairs)
{
	PX_ASSERT(pairs);
	PX_ASSERT(maxNumPairs>0);
	const PxU32 newMaxNumPairs=2*maxNumPairs;
	PxcBroadPhasePair* newPairs=(PxcBroadPhasePair*)PX_ALLOC(sizeof(PxcBroadPhasePair)*newMaxNumPairs, PX_DEBUG_EXP("PxcBroadPhasePair"));
	PxMemCopy(newPairs, pairs, sizeof(PxcBroadPhasePair)*maxNumPairs);
	PX_FREE(pairs);
	pairs=newPairs;
	maxNumPairs=newMaxNumPairs;
}
#endif

void physx::ComputeCreatedDeletedPairsLists
(const PxcBpHandle* PX_RESTRICT boxGroups, 
 const PxcBpHandle* PX_RESTRICT dataArray, const PxU32 dataArraySize,
 PxcBroadPhasePair*& createdPairsList, PxU32& numCreatedPairs, PxU32& maxNumCreatedPairs,
 PxcBroadPhasePair*& deletedPairsList, PxU32& numDeletedPairs, PxU32& maxNumDeletedPairs,
 SapPairManager& pairManager)
{
	PX_UNUSED(boxGroups);

	for(PxU32 i=0;i<dataArraySize;i++)
	{
		const PxU32 ID = dataArray[i];
		PX_ASSERT(ID<pairManager.mNbActivePairs);

		const PxcBroadPhasePair* PX_RESTRICT UP = pairManager.mActivePairs + ID;
		PX_ASSERT(pairManager.IsInArray(UP));

		if(pairManager.IsRemoved(UP))
		{
			if(!pairManager.IsNew(UP))
			{
#ifndef __SPU__
				// No need to call "ClearInArray" in this case, since the pair will get removed anyway
				if(numDeletedPairs==maxNumDeletedPairs)
				{
					resizeCreatedDeleted(deletedPairsList,maxNumDeletedPairs);
				}
#endif
				PX_ASSERT(numDeletedPairs<maxNumDeletedPairs);
				deletedPairsList[numDeletedPairs]=PxcBroadPhasePair(UP->mVolA,UP->mVolB);
				numDeletedPairs++;
			}
		}
		else
		{
			pairManager.ClearInArray(UP);
			// Add => already there... Might want to create user data, though
			if(pairManager.IsNew(UP))
			{
#if !BP_SAP_TEST_GROUP_ID_CREATEUPDATE
				if(boxGroups[UP->mVolA]!=boxGroups[UP->mVolB])
#endif
				{
#ifndef __SPU__
					if(numCreatedPairs==maxNumCreatedPairs)
					{
						resizeCreatedDeleted(createdPairsList,maxNumCreatedPairs);
					}
#endif
					PX_ASSERT(numCreatedPairs<maxNumCreatedPairs);
					createdPairsList[numCreatedPairs]=PxcBroadPhasePair(UP->mVolA,UP->mVolB);
					numCreatedPairs++;
				}
				pairManager.ClearNew(UP);
			}
		}
	}

	//Record pairs that are to be deleted because they were simultaneously created and removed 
	//from different axis sorts.
	PxU32 numActualDeletedPairs=numDeletedPairs;
	for(PxU32 i=0;i<dataArraySize;i++)
	{
		const PxU32 ID = dataArray[i];
		PX_ASSERT(ID<pairManager.mNbActivePairs);
		const PxcBroadPhasePair* PX_RESTRICT UP = pairManager.mActivePairs + ID;
		if(pairManager.IsRemoved(UP) && pairManager.IsNew(UP))
		{
			PX_ASSERT(pairManager.IsInArray(UP));

#ifndef __SPU__
			if(numActualDeletedPairs==maxNumDeletedPairs)
			{
				resizeCreatedDeleted(deletedPairsList,maxNumDeletedPairs);
			}
#endif

			PX_ASSERT(numActualDeletedPairs<=maxNumDeletedPairs);
			deletedPairsList[numActualDeletedPairs]=PxcBroadPhasePair(UP->mVolA,UP->mVolB);
			numActualDeletedPairs++;
		}
	}

	// #### try batch removal here
	for(PxU32 i=0;i<numActualDeletedPairs;i++)
	{
		const PxcBpHandle id0 = deletedPairsList[i].mVolA;
		const PxcBpHandle id1 = deletedPairsList[i].mVolB;
#ifdef PX_DEBUG
		const bool Status = pairManager.RemovePair(id0, id1);
		PX_ASSERT(Status);
#else
		pairManager.RemovePair(id0, id1);
#endif
	}

	//Only report deleted pairs from different groups.
#if !BP_SAP_TEST_GROUP_ID_CREATEUPDATE
	for(PxU32 i=0;i<numDeletedPairs;i++)
	{
		const PxU32 id0 = deletedPairsList[i].mVolA;
		const PxU32 id1 = deletedPairsList[i].mVolB;
		if(boxGroups[id0]==boxGroups[id1])
		{
			while((numDeletedPairs-1) > i && boxGroups[deletedPairsList[numDeletedPairs-1].mVolA] == boxGroups[deletedPairsList[numDeletedPairs-1].mVolB])
			{
				numDeletedPairs--;
			}
			deletedPairsList[i]=deletedPairsList[numDeletedPairs-1];
			numDeletedPairs--;
		}
	}
#endif
}

#include "PxvBroadPhase.h"
PX_COMPILE_TIME_ASSERT(BP_GROUP_STATICS==0);
void physx::ComputeSortedLists
(Cm::BitMap* PX_RESTRICT bitmap, 
 const PxU32 insertAABBStart, const PxU32 insertAABBEnd, const PxcBpHandle* PX_RESTRICT createdAABBs,
 SapBox1D** PX_RESTRICT asapBoxes, const PxcBpHandle* PX_RESTRICT asapBoxGroupIds, 
 PxcBPValType* PX_RESTRICT /*asapEndPointValues*/, PxcBpHandle* PX_RESTRICT asapEndPointDatas, const PxU32 numSortedEndPoints, 
 const Gu::Axes& axes,
 PxcBpHandle* PX_RESTRICT newBoxIndicesSorted, PxU32& newBoxIndicesCount, PxcBpHandle* PX_RESTRICT oldBoxIndicesSorted, PxU32& oldBoxIndicesCount,
 bool& allNewBoxesStatics, bool& allOldBoxesStatics)
{
	const PxU32 axis0=axes.mAxis0;
	const PxU32 axis1=axes.mAxis1;
	const PxU32 axis2=axes.mAxis2;

	//Set the bitmap for new box ids and compute the aabb that bounds all new boxes.
	IntegerAABB globalAABB;
	globalAABB.mMinMax[IntegerAABB::MIN_X]=PX_MAX_U32;
	globalAABB.mMinMax[IntegerAABB::MIN_Y]=PX_MAX_U32;
	globalAABB.mMinMax[IntegerAABB::MIN_Z]=PX_MAX_U32;
	globalAABB.mMinMax[IntegerAABB::MAX_X]=0;
	globalAABB.mMinMax[IntegerAABB::MAX_Y]=0;
	globalAABB.mMinMax[IntegerAABB::MAX_Z]=0;
	
	for(PxU32 i=insertAABBStart;i<insertAABBEnd;i++)
	{
		const PxU32 boxId=createdAABBs[i];
		bitmap->set(boxId);

		globalAABB.mMinMax[IntegerAABB::MIN_X] = PxMin(globalAABB.mMinMax[IntegerAABB::MIN_X], (PxU32)asapBoxes[axis0][boxId].mMinMax[0]);
		globalAABB.mMinMax[IntegerAABB::MIN_Y] = PxMin(globalAABB.mMinMax[IntegerAABB::MIN_Y], (PxU32)asapBoxes[axis1][boxId].mMinMax[0]);
		globalAABB.mMinMax[IntegerAABB::MIN_Z] = PxMin(globalAABB.mMinMax[IntegerAABB::MIN_Z], (PxU32)asapBoxes[axis2][boxId].mMinMax[0]);
		globalAABB.mMinMax[IntegerAABB::MAX_X] = PxMax(globalAABB.mMinMax[IntegerAABB::MAX_X], (PxU32)asapBoxes[axis0][boxId].mMinMax[1]);
		globalAABB.mMinMax[IntegerAABB::MAX_Y] = PxMax(globalAABB.mMinMax[IntegerAABB::MAX_Y], (PxU32)asapBoxes[axis1][boxId].mMinMax[1]);
		globalAABB.mMinMax[IntegerAABB::MAX_Z] = PxMax(globalAABB.mMinMax[IntegerAABB::MAX_Z], (PxU32)asapBoxes[axis2][boxId].mMinMax[1]);
	}

	PxU32 oldStaticCount=0;
	PxU32 newStaticCount=0;

	//Assign the sorted end pts to the appropriate arrays.
	for(PxU32 i=1;i<numSortedEndPoints-1;i++)
	{
		//Make sure we haven't encountered a sentinel - 
		//they should only be at each end of the array.
		PX_ASSERT(!isSentinel(asapEndPointDatas[i]));
		PX_ASSERT(!isSentinel(asapEndPointDatas[i]));
		PX_ASSERT(!isSentinel(asapEndPointDatas[i]));

		if(!isMax(asapEndPointDatas[i]))
		{
			const PxcBpHandle boxId=(PxcBpHandle)getOwner(asapEndPointDatas[i]);
			if(!bitmap->test(boxId))
			{
				if(Intersect3D(
					globalAABB.mMinMax[IntegerAABB::MIN_X], globalAABB.mMinMax[IntegerAABB::MAX_X], globalAABB.mMinMax[IntegerAABB::MIN_Y], globalAABB.mMinMax[IntegerAABB::MAX_Y], globalAABB.mMinMax[IntegerAABB::MIN_Z], globalAABB.mMinMax[IntegerAABB::MAX_Z],
					asapBoxes[axis0][boxId].mMinMax[0],asapBoxes[axis0][boxId].mMinMax[1],asapBoxes[axis1][boxId].mMinMax[0],asapBoxes[axis1][boxId].mMinMax[1],asapBoxes[axis2][boxId].mMinMax[0],asapBoxes[axis2][boxId].mMinMax[1]))
				{
					oldBoxIndicesSorted[oldBoxIndicesCount]=boxId;
					oldBoxIndicesCount++;
					oldStaticCount+=asapBoxGroupIds[boxId];
				}
			}
			else 
			{
				newBoxIndicesSorted[newBoxIndicesCount]=boxId;
				newBoxIndicesCount++;
				newStaticCount+=asapBoxGroupIds[boxId];
			}
		}
	}

	allOldBoxesStatics = oldStaticCount ? false : true;
	allNewBoxesStatics = newStaticCount ? false : true;

	//Make sure that we've found the correct number of boxes.
	PX_ASSERT(newBoxIndicesCount==(insertAABBEnd-insertAABBStart));
	PX_ASSERT(oldBoxIndicesCount<=((numSortedEndPoints-NUM_SENTINELS)/2));
}

void physx::performBoxPruningNewNew
(const Gu::Axes& axes,
 const PxcBpHandle* PX_RESTRICT newBoxIndicesSorted, const PxU32 newBoxIndicesCount, const bool allNewBoxesStatics,
 PxcBpHandle* PX_RESTRICT minPosList0,
 SapBox1D** PX_RESTRICT asapBoxes, const PxcBpHandle* PX_RESTRICT asapBoxGroupIds, 
#ifndef __SPU__
 SapPairManager& pairManager, PxcBpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
#else
 PxcBroadPhasePair* PX_RESTRICT overlapPairs, PxU32& numOverlapPairs)
#endif
{
	// Checkings
	if(!newBoxIndicesCount)	return;

	// Catch axes
	const PxU32 Axis0 = axes.mAxis0;
	const PxU32 Axis1 = axes.mAxis1;
	const PxU32 Axis2 = axes.mAxis2;

	// 1) Build main list using the primary axis
	for(PxU32 i=0;i<newBoxIndicesCount;i++)	
	{
		const PxcBpHandle boxId = newBoxIndicesSorted[i];
		minPosList0[i] = asapBoxes[Axis0][boxId].mMinMax[0];
	}

	if(allNewBoxesStatics) return;

	// 2) Prune the list

	const PxU32 LastSortedIndex = newBoxIndicesCount;
	PxU32 RunningIndex = 0;
	PxU32 SortedIndex = 0;

	while(RunningIndex<LastSortedIndex && SortedIndex<LastSortedIndex)
	{
		const PxU32 Index0 = SortedIndex++;
		const PxcBpHandle boxId0 = newBoxIndicesSorted[Index0];
		const PxcBpHandle Limit = asapBoxes[Axis0][boxId0].mMinMax[1];

		while(RunningIndex<LastSortedIndex && minPosList0[RunningIndex++]<minPosList0[Index0]);

		if(RunningIndex<LastSortedIndex)
		{
			PxU32 RunningIndex2 = RunningIndex;

			PxU32 Index1;
			while(RunningIndex2<LastSortedIndex && minPosList0[Index1 = RunningIndex2++] <= Limit)
			{
				const PxcBpHandle boxId1 = newBoxIndicesSorted[Index1];
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
				if(asapBoxGroupIds[boxId0]!=asapBoxGroupIds[boxId1])
#endif
				{
					if(Intersect2D(
						asapBoxes[Axis1][boxId0].mMinMax[0],asapBoxes[Axis1][boxId0].mMinMax[1],asapBoxes[Axis2][boxId0].mMinMax[0],asapBoxes[Axis2][boxId0].mMinMax[1],
						asapBoxes[Axis1][boxId1].mMinMax[0],asapBoxes[Axis1][boxId1].mMinMax[1],asapBoxes[Axis2][boxId1].mMinMax[0],asapBoxes[Axis2][boxId1].mMinMax[1]))
					{
#ifndef __SPU__
						AddPair(boxId0,boxId1,pairManager,dataArray,dataArraySize,dataArrayCapacity);
#else
						if(numOverlapPairs<MAX_NUM_BP_SPU_SAP_OVERLAPS)
						{
							overlapPairs[numOverlapPairs].mVolA=boxId0;
							overlapPairs[numOverlapPairs].mVolB=boxId1;
							numOverlapPairs++;				
						}
#endif
					}
				}
			}
		}
	}
}

void physx::performBoxPruningNewOld
(const Gu::Axes& axes,
 const PxcBpHandle* PX_RESTRICT newBoxIndicesSorted, const PxU32 newBoxIndicesCount, const PxcBpHandle* PX_RESTRICT oldBoxIndicesSorted, const PxU32 oldBoxIndicesCount,
 PxcBpHandle* PX_RESTRICT minPosListNew,  PxcBpHandle* PX_RESTRICT minPosListOld,
 SapBox1D** PX_RESTRICT asapBoxes, const PxcBpHandle* PX_RESTRICT asapBoxGroupIds,
#ifndef __SPU__
 SapPairManager& pairManager, PxcBpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
#else
 PxcBroadPhasePair* PX_RESTRICT overlapPairs, PxU32& numOverlapPairs)
#endif

{
	// Checkings
	if(!newBoxIndicesCount || !oldBoxIndicesCount)	return;

	// Catch axes
	const PxU32 Axis0 = axes.mAxis0;
	const PxU32 Axis1 = axes.mAxis1;
	const PxU32 Axis2 = axes.mAxis2;

	PxcBpHandle* PX_RESTRICT minPosList0=minPosListNew;
	PxcBpHandle* PX_RESTRICT minPosList1=minPosListOld;

	// 1) Build main lists using the primary axis
	for(PxU32 i=0;i<newBoxIndicesCount;i++)	
	{
		const PxcBpHandle boxId=newBoxIndicesSorted[i];
		minPosList0[i] = asapBoxes[Axis0][boxId].mMinMax[0];
	}
	for(PxU32 i=0;i<oldBoxIndicesCount;i++)	
	{
		const PxcBpHandle boxId=oldBoxIndicesSorted[i];
		minPosList1[i] = asapBoxes[Axis0][boxId].mMinMax[0];
	}

	// 3) Prune the lists
	const PxU32 LastSortedIndex0 = newBoxIndicesCount;
	const PxU32 LastSortedIndex1 = oldBoxIndicesCount;
	PxU32 RunningIndex0 = 0;
	PxU32 RunningIndex1 = 0;
	PxU32 SortedIndex1 = 0;
	PxU32 SortedIndex0 = 0;


	while(RunningIndex1<LastSortedIndex1 && SortedIndex0<LastSortedIndex0)
	{
		const PxU32 Index0 = SortedIndex0++;
		const PxcBpHandle boxId0 = newBoxIndicesSorted[Index0];
		const PxcBpHandle Limit = asapBoxes[Axis0][boxId0].mMinMax[1];//Box0.mMaxIndex[Axis0];

		while(RunningIndex1<LastSortedIndex1 && minPosList1[RunningIndex1]<minPosList0[Index0])
			RunningIndex1++;

		PxU32 RunningIndex2_1 = RunningIndex1;

		PxU32 Index1;
		while(RunningIndex2_1<LastSortedIndex1 && minPosList1[Index1 = RunningIndex2_1++] <= Limit)
		{
			const PxcBpHandle boxId1 = oldBoxIndicesSorted[Index1];
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
			if(asapBoxGroupIds[boxId0]!=asapBoxGroupIds[boxId1])
#endif
			{
				if(Intersect2D(
					asapBoxes[Axis1][boxId0].mMinMax[0],asapBoxes[Axis1][boxId0].mMinMax[1],asapBoxes[Axis2][boxId0].mMinMax[0],asapBoxes[Axis2][boxId0].mMinMax[1],
					asapBoxes[Axis1][boxId1].mMinMax[0],asapBoxes[Axis1][boxId1].mMinMax[1],asapBoxes[Axis2][boxId1].mMinMax[0],asapBoxes[Axis2][boxId1].mMinMax[1]))
				{
#ifndef __SPU__
					AddPair(boxId0,boxId1,pairManager,dataArray,dataArraySize,dataArrayCapacity);
#else
					if(numOverlapPairs<MAX_NUM_BP_SPU_SAP_OVERLAPS)
					{
						overlapPairs[numOverlapPairs].mVolA=boxId0;
						overlapPairs[numOverlapPairs].mVolB=boxId1;
						numOverlapPairs++;					
					}
#endif
				}
			}
		}
	}

	////

	while(RunningIndex0<LastSortedIndex0 && SortedIndex1<LastSortedIndex1)
	{
		const PxU32 Index0 = SortedIndex1++;
		const PxcBpHandle boxId0 = oldBoxIndicesSorted[Index0];
		const PxcBpHandle Limit = asapBoxes[Axis0][boxId0].mMinMax[1];

		while(RunningIndex0<LastSortedIndex0 && minPosList0[RunningIndex0]<=minPosList1[Index0])
			RunningIndex0++;

		PxU32 RunningIndex2_0 = RunningIndex0;

		PxU32 Index1;
		while(RunningIndex2_0<LastSortedIndex0 && minPosList0[Index1 = RunningIndex2_0++] <= Limit)
		{
			const PxcBpHandle boxId1 = newBoxIndicesSorted[Index1];
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
			if(asapBoxGroupIds[boxId0]!=asapBoxGroupIds[boxId1])
#endif
			{
				if(Intersect2D(
					asapBoxes[Axis1][boxId0].mMinMax[0],asapBoxes[Axis1][boxId0].mMinMax[1],asapBoxes[Axis2][boxId0].mMinMax[0],asapBoxes[Axis2][boxId0].mMinMax[1],
					asapBoxes[Axis1][boxId1].mMinMax[0],asapBoxes[Axis1][boxId1].mMinMax[1],asapBoxes[Axis2][boxId1].mMinMax[0],asapBoxes[Axis2][boxId1].mMinMax[1]))
				{
#ifndef __SPU__
					AddPair(boxId0,boxId1,pairManager,dataArray,dataArraySize,dataArrayCapacity);
#else
					if(numOverlapPairs<MAX_NUM_BP_SPU_SAP_OVERLAPS)
					{
						overlapPairs[numOverlapPairs].mVolA=boxId0;
						overlapPairs[numOverlapPairs].mVolB=boxId1;
						numOverlapPairs++;					
					}
#endif
				}
			}
		}
	}
}



