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


#ifndef PXS_BROADPHASE_SAP_AUX_H
#define PXS_BROADPHASE_SAP_AUX_H

#include "CmPhysXCommon.h"
#include "PxAssert.h"
#include "PsIntrinsics.h"
#ifndef __SPU__
#include "PsUserAllocated.h"
#endif
#include "PxsBroadPhaseCommon.h"
#include "CmBitMap.h"
#include "GuAxes.h"

namespace physx
{

#define NUM_SENTINELS 2

#define BP_SAP_USE_PREFETCH 1//prefetch in batchUpdate

#define BP_SAP_USE_OVERLAP_TEST_ON_REMOVES	1// "Useless" but faster overall because seriously reduces number of calls (from ~10000 to ~3 sometimes!)

//Set 1 to test for group ids in batchCreate/batchUpdate so we can avoid group id test in ComputeCreatedDeletedPairsLists
//Set 0 to neglect group id test in batchCreate/batchUpdate and delay test until ComputeCreatedDeletedPairsLists
#ifdef __SPU__
#define BP_SAP_TEST_GROUP_ID_CREATEUPDATE 1  
#else
#define BP_SAP_TEST_GROUP_ID_CREATEUPDATE 1
#endif

#if PX_USE_16_BIT_HANDLES
#define MAX_BP_HANDLE			0xffff
#define PX_REMOVED_BP_HANDLE	0xfffd
#define MAX_BP_PAIRS_MESSAGE "Only 65536 broadphase pairs are supported.  This limit has been exceeded and some pairs will be dropped \n"
#else
#define MAX_BP_HANDLE			0x3fffffff
#define PX_REMOVED_BP_HANDLE	0x3ffffffd
#define MAX_BP_PAIRS_MESSAGE "Only 4294967296 broadphase pairs are supported.  This limit has been exceeded and some pairs will be dropped \n"
#endif

PX_FORCE_INLINE	void setMinSentinel(PxcBPValType& v, PxcBpHandle& d)
{
	v = 0x00000000;//0x00800000;  //0x00800000 is -FLT_MAX but setting it to 0 means we don't crash when we get a value outside the float range.
	d = (PX_INVALID_BP_HANDLE & ~1);
}

PX_FORCE_INLINE	void setMaxSentinel(PxcBPValType& v, PxcBpHandle& d)
{										
	v = 0xffffffff;//0xff7fffff;  //0xff7fffff is +FLT_MAX but setting it to 0xffffffff means we don't crash when we get a value outside the float range.
	d = PX_INVALID_BP_HANDLE;
}

PX_FORCE_INLINE	void setData(PxcBpHandle& d, PxU32 owner_box_id, const bool is_max)
{
	//v=v;
	d = (PxcBpHandle)(owner_box_id<<1);
	if(is_max)	d |= 1;
}

PX_FORCE_INLINE	bool isSentinel(const PxcBpHandle& d)	
{ 
	return (d&~1)==(PX_INVALID_BP_HANDLE & ~1);	
}

PX_FORCE_INLINE	PxcBpHandle isMax(const PxcBpHandle& d) 	
{
	return PxcBpHandle(d & 1);		
}

PX_FORCE_INLINE	PxcBpHandle getOwner(const PxcBpHandle& d) 	
{ 
	return PxcBpHandle(d>>1);		
}

class SapBox1D
{
public:

	PX_FORCE_INLINE					SapBox1D()	{}
	PX_FORCE_INLINE					~SapBox1D()	{}

	PxcBpHandle		mMinMax[2];//mMinMax[0]=min, mMinMax[1]=max
};

class SapPairManager
{
public:
	SapPairManager();
	~SapPairManager();

	void						init(const PxU32 size);
	void						release();

	void						shrinkMemory();

	const PxcBroadPhasePair*	AddPair		(PxcBpHandle id0, PxcBpHandle id1, const PxU8 state);
	bool						RemovePair	(PxcBpHandle id0, PxcBpHandle id1);
	bool						RemovePairs	(const Cm::BitMap& removedAABBs);
	const PxcBroadPhasePair*	FindPair	(PxcBpHandle id0, PxcBpHandle id1)	const;

	PX_FORCE_INLINE	PxU32		GetPairIndex(const PxcBroadPhasePair* PX_RESTRICT pair)	const
	{
		return ((PxU32)((size_t(pair) - size_t(mActivePairs)))/sizeof(PxcBroadPhasePair));
	}

	PxcBpHandle*			mHashTable;
	PxcBpHandle*			mNext;
	PxU32				mHashSize;
	PxU32				mHashCapacity;
	PxU32				mMinAllowedHashCapacity;
	PxcBroadPhasePair*	mActivePairs;
	PxU8*				mActivePairStates;
	PxU32				mNbActivePairs;
	PxU32				mActivePairsCapacity;
	PxU32				mMask;

	PxcBroadPhasePair*	FindPair	(PxcBpHandle id0, PxcBpHandle id1, PxU32 hash_value) const;
	void				RemovePair	(PxcBpHandle id0, PxcBpHandle id1, PxU32 hash_value, PxU32 pair_index);
	void				reallocPairs(const bool allocRequired);

	enum
	{
		PAIR_INARRAY=1,
		PAIR_REMOVED=2,
		PAIR_NEW=4,
		PAIR_UNKNOWN=8
	};

	PX_FORCE_INLINE bool IsInArray(const PxcBroadPhasePair* PX_RESTRICT pair) const 
	{
		const PxU8 state=mActivePairStates[pair-mActivePairs];
		return state & PAIR_INARRAY ? true : false;
	}
	PX_FORCE_INLINE bool IsRemoved(const PxcBroadPhasePair* PX_RESTRICT pair) const 
	{
		const PxU8 state=mActivePairStates[pair-mActivePairs];
		return state & PAIR_REMOVED ? true : false;
	}
	PX_FORCE_INLINE bool IsNew(const PxcBroadPhasePair* PX_RESTRICT pair) const
	{
		const PxU8 state=mActivePairStates[pair-mActivePairs];
		return state & PAIR_NEW ? true : false;
	}
	PX_FORCE_INLINE bool IsUnknown(const PxcBroadPhasePair* PX_RESTRICT pair) const
	{
		const PxU8 state=mActivePairStates[pair-mActivePairs];
		return state & PAIR_UNKNOWN ? true : false;
	}

	PX_FORCE_INLINE void ClearState(const PxcBroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs]=0;
	}

	PX_FORCE_INLINE void SetInArray(const PxcBroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] |= PAIR_INARRAY;
	}
	PX_FORCE_INLINE void SetRemoved(const PxcBroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] |= PAIR_REMOVED;
	}
	PX_FORCE_INLINE void SetNew(const PxcBroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] |= PAIR_NEW;
	}
	PX_FORCE_INLINE void ClearInArray(const PxcBroadPhasePair* PX_RESTRICT pair)
	{		
		mActivePairStates[pair-mActivePairs] &= ~PAIR_INARRAY;
	}
	PX_FORCE_INLINE void ClearRemoved(const PxcBroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] &= ~PAIR_REMOVED;
	}
	PX_FORCE_INLINE void ClearNew(const PxcBroadPhasePair* PX_RESTRICT pair)
	{
		mActivePairStates[pair-mActivePairs] &= ~PAIR_NEW;
	}
};

#ifndef __SPU__
PX_FORCE_INLINE void ResizeData(PxcBpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
{
	PX_ASSERT(dataArraySize==dataArrayCapacity);
	PX_UNUSED(dataArraySize);
	const PxU32 newDataArrayCapacity=(dataArrayCapacity ? dataArrayCapacity*2 : 64);
	PxcBpHandle* newDataArray=(PxcBpHandle*)PX_ALLOC(newDataArrayCapacity*sizeof(PxcBpHandle), PX_DEBUG_EXP("PxBpHandle"));
	PxMemCopy(newDataArray,dataArray,dataArrayCapacity*sizeof(PxcBpHandle));
	PX_FREE(dataArray);
	dataArray=newDataArray;
	dataArrayCapacity=newDataArrayCapacity;
}
#endif

PX_FORCE_INLINE void AddData(const PxU32 data, PxcBpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
{
#ifndef __SPU__
	if(dataArraySize==dataArrayCapacity)
	{
		ResizeData(dataArray,dataArraySize,dataArrayCapacity);
	}
#endif

	PX_ASSERT(dataArraySize<dataArrayCapacity);
	dataArray[dataArraySize]=(PxcBpHandle)data;
	dataArraySize++;
}

PX_FORCE_INLINE	bool AddPair
(const PxcBpHandle id0, const PxcBpHandle id1, 
 SapPairManager& pairManager, PxcBpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
{
	PxcBroadPhasePair* UP = (PxcBroadPhasePair*)pairManager.AddPair(id0, id1, SapPairManager::PAIR_UNKNOWN);

	//If the hash table has reached its limit then we're unable to add a new pair.
	if(NULL==UP)
	{
		return false;
	}

	PX_ASSERT(UP);
	if(pairManager.IsUnknown(UP))
	{
		pairManager.ClearState(UP);
		pairManager.SetInArray(UP);
		AddData(pairManager.GetPairIndex(UP),dataArray,dataArraySize,dataArrayCapacity);
		pairManager.SetNew(UP);
	}
	pairManager.ClearRemoved(UP);

	return true;
}

PX_FORCE_INLINE	void RemovePair
(PxcBpHandle id0, PxcBpHandle id1, 
 SapPairManager& pairManager,  PxcBpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity)
{
	PxcBroadPhasePair* UP = (PxcBroadPhasePair*)pairManager.FindPair(id0, id1);
	if(UP)
	{
		if(!pairManager.IsInArray(UP))
		{
			pairManager.SetInArray(UP);
			AddData(pairManager.GetPairIndex(UP),dataArray,dataArraySize,dataArrayCapacity);
		}
		pairManager.SetRemoved(UP);
	}
}

PX_FORCE_INLINE void InsertEndPoints
(const PxcBPValType* PX_RESTRICT newEndPointValues, const PxcBpHandle* PX_RESTRICT newEndPointDatas, PxU32 numNewEndPoints,
 PxcBPValType* PX_RESTRICT endPointValues, PxcBpHandle* PX_RESTRICT endPointDatas, const PxU32 numEndPoints, 
 SapBox1D* PX_RESTRICT boxes)
{
	PxcBPValType* const BaseEPValue = endPointValues;
	PxcBpHandle* const BaseEPData = endPointDatas;

	const PxU32 OldSize = numEndPoints-NUM_SENTINELS;
	const PxU32 NewSize = numEndPoints-NUM_SENTINELS+numNewEndPoints;

	BaseEPValue[NewSize + 1] = BaseEPValue[OldSize + 1];
	BaseEPData[NewSize + 1] = BaseEPData[OldSize + 1];

	PxI32 WriteIdx = (PxI32)NewSize;
	PxU32 CurrInsIdx = 0;

	//const SapValType* FirstValue = &BaseEPValue[0];
	const PxcBpHandle* FirstData = &BaseEPData[0];
	const PxcBPValType* CurrentValue = &BaseEPValue[OldSize];
	const PxcBpHandle* CurrentData = &BaseEPData[OldSize];
	while(CurrentData>=FirstData)
	{
		const PxcBPValType& SrcValue = *CurrentValue;
		const PxcBpHandle& SrcData = *CurrentData;
		const PxcBPValType& InsValue = newEndPointValues[CurrInsIdx];
		const PxcBpHandle& InsData = newEndPointDatas[CurrInsIdx];

		// We need to make sure we insert maxs before mins to handle exactly equal endpoints correctly
		const bool ShouldInsert = isMax(InsData) ? (SrcValue <= InsValue) : (SrcValue < InsValue);

		const PxcBPValType& MovedValue = ShouldInsert ? InsValue : SrcValue;
		const PxcBpHandle& MovedData = ShouldInsert ? InsData : SrcData;
		BaseEPValue[WriteIdx] = MovedValue;
		BaseEPData[WriteIdx] = MovedData;
		boxes[getOwner(MovedData)].mMinMax[isMax(MovedData)] = (PxcBpHandle)WriteIdx--;

		if(ShouldInsert)
		{
			CurrInsIdx++;
			if(CurrInsIdx >= numNewEndPoints)
				break;//we just inserted the last endpoint
		}
		else
		{
			CurrentValue--;
			CurrentData--;
		}
	}
}

void ComputeCreatedDeletedPairsLists
(const PxcBpHandle* PX_RESTRICT boxGroups, 
 const PxcBpHandle* PX_RESTRICT dataArray, const PxU32 dataArraySize,
 PxcBroadPhasePair* & createdPairsList, PxU32& numCreatedPairs, PxU32& maxNumCreatdPairs,
 PxcBroadPhasePair* & deletedPairsList, PxU32& numDeletedPairs, PxU32& maxNumDeletedPairs,
 SapPairManager& pairManager);

void ComputeSortedLists
(Cm::BitMap* PX_RESTRICT bitmap, 
 const PxU32 insertAABBStart, const PxU32 insertAABBEnd, const PxcBpHandle* PX_RESTRICT createdAABBs,
 SapBox1D** PX_RESTRICT asapBoxes, const PxcBpHandle* PX_RESTRICT asapBoxGroupIds, 
 PxcBPValType* PX_RESTRICT asapEndPointValues, PxcBpHandle* PX_RESTRICT asapEndPointDatas, const PxU32 numSortedEndPoints, 
 const Gu::Axes& axes,
 PxcBpHandle* PX_RESTRICT newBoxIndicesSorted, PxU32& newBoxIndicesCount, PxcBpHandle* PX_RESTRICT oldBoxIndicesSorted, PxU32& oldBoxIndicesCount,
 bool& allNewBoxesStatics, bool& allOldBoxesStatics);

void performBoxPruningNewNew
(const Gu::Axes& axes,
 const PxcBpHandle* PX_RESTRICT newBoxIndicesSorted, const PxU32 newBoxIndicesCount,  const bool allNewBoxesStatics,
 PxcBpHandle* PX_RESTRICT minPosList0,
 SapBox1D** PX_RESTRICT asapBoxes, const PxcBpHandle* PX_RESTRICT asapBoxGroupIds, 
#ifndef __SPU__
 SapPairManager& pairManager, PxcBpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity);
#else
 PxcBroadPhasePair* PX_RESTRICT overlapPairs, PxU32& numOverlapPairs);
#endif

void performBoxPruningNewOld
(const Gu::Axes& axes,
 const PxcBpHandle* PX_RESTRICT newBoxIndicesSorted, const PxU32 newBoxIndicesCount, const PxcBpHandle* PX_RESTRICT oldBoxIndicesSorted, const PxU32 oldBoxIndicesCount,
 PxcBpHandle* PX_RESTRICT minPosListNew,  PxcBpHandle* PX_RESTRICT minPosListOld,
 SapBox1D** PX_RESTRICT asapBoxes, const PxcBpHandle* PX_RESTRICT asapBoxGroupIds,
#ifndef __SPU__
 SapPairManager& pairManager, PxcBpHandle*& dataArray, PxU32& dataArraySize, PxU32& dataArrayCapacity);
#else
  PxcBroadPhasePair* PX_RESTRICT overlapPairs, PxU32& numOverlapPairs);
#endif

PX_FORCE_INLINE bool Intersect2D(SapBox1D** PX_RESTRICT c, SapBox1D** PX_RESTRICT b, const PxU32 axis1, const PxU32 axis2)
{
	return (b[axis1]->mMinMax[1] >= c[axis1]->mMinMax[0] && c[axis1]->mMinMax[1] >= b[axis1]->mMinMax[0] &&
		    b[axis2]->mMinMax[1] >= c[axis2]->mMinMax[0] && c[axis2]->mMinMax[1] >= b[axis2]->mMinMax[0]);
}

PX_FORCE_INLINE bool Intersect3D(SapBox1D** PX_RESTRICT c, SapBox1D** PX_RESTRICT b)
{
	return (b[0]->mMinMax[1] >= c[0]->mMinMax[0] && c[0]->mMinMax[1] >= b[0]->mMinMax[0] &&
		    b[1]->mMinMax[1] >= c[1]->mMinMax[0] && c[1]->mMinMax[1] >= b[1]->mMinMax[0] &&
			b[2]->mMinMax[1] >= c[2]->mMinMax[0] && c[2]->mMinMax[1] >= b[2]->mMinMax[0]);       
}

PX_FORCE_INLINE bool Intersect1D_Min(const PxcBPValType aMin, const PxcBPValType /*aMax*/, SapBox1D* PX_RESTRICT b, const PxcBPValType* PX_RESTRICT endPointValues)
{
	const PxcBPValType& endPointValue=endPointValues[b->mMinMax[1]];
	return (endPointValue >= aMin);
}

PX_FORCE_INLINE bool Intersect1D_Max(const PxcBPValType /*aMin*/, const PxcBPValType aMax, SapBox1D* PX_RESTRICT b, const PxcBPValType* PX_RESTRICT endPointValues)
{
	const PxcBPValType& endPointValue=endPointValues[b->mMinMax[0]];
	return (endPointValue < aMax);
}

PX_FORCE_INLINE bool Intersect2D
(const PxcBPValType bDir1Min, const PxcBPValType bDir1Max, const PxcBPValType bDir2Min, const PxcBPValType bDir2Max,
 const PxcBPValType cDir1Min, const PxcBPValType cDir1Max, const PxcBPValType cDir2Min, const PxcBPValType cDir2Max)
{
	return (bDir1Max >= cDir1Min && cDir1Max >= bDir1Min && 
			bDir2Max >= cDir2Min && cDir2Max >= bDir2Min);        
}

PX_FORCE_INLINE bool Intersect2D_Handle
(const PxcBpHandle bDir1Min, const PxcBpHandle bDir1Max, const PxcBpHandle bDir2Min, const PxcBpHandle bDir2Max,
 const PxcBpHandle cDir1Min, const PxcBpHandle cDir1Max, const PxcBpHandle cDir2Min, const PxcBpHandle cDir2Max)
{
	return (bDir1Max > cDir1Min && cDir1Max > bDir1Min && 
			bDir2Max > cDir2Min && cDir2Max > bDir2Min);        
}

PX_FORCE_INLINE bool Intersect3D
(const PxcBPValType bDir1Min, const PxcBPValType bDir1Max, const PxcBPValType bDir2Min, const PxcBPValType bDir2Max, const PxcBPValType bDir3Min, const PxcBPValType bDir3Max,
 const PxcBPValType cDir1Min, const PxcBPValType cDir1Max, const PxcBPValType cDir2Min, const PxcBPValType cDir2Max, const PxcBPValType cDir3Min, const PxcBPValType cDir3Max)
{
	return (bDir1Max >= cDir1Min && cDir1Max >= bDir1Min && 
			bDir2Max >= cDir2Min && cDir2Max >= bDir2Min &&
			bDir3Max >= cDir3Min && cDir3Max >= bDir3Min);       
}

}

#endif //PXS_BROADPHASE_SAP_H
