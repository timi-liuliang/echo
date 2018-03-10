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


#ifndef PXS_BROADPHASE_SAP_H
#define PXS_BROADPHASE_SAP_H

#include "PxvBroadPhase.h"
#include "PxsBroadPhaseSapAux.h"
#include "PxsBroadPhaseCommon.h"
#include "PxsBroadPhaseConfig.h"
#include "PxcPool.h"
#include "CmPhysXCommon.h"
#include "PxsSAPTasks.h"

//KS - This has been tested and found to work on PS3. It will only take effect on PPU fallbacks but can offer better performance in degenerate cases
#ifndef PX_PS3
#define BP_UPDATE_BEFORE_SWAP 1
#else
#define BP_UPDATE_BEFORE_SWAP 0
#endif

namespace physx
{

// Forward declarations
class PxcScratchAllocator;
class SapEndPoint;
class ASAP_Box;
struct IntegerAABB;
class BroadPhaseSapRemoveSpuTask;
class BroadPhaseSapCreateSpuTask;
class BroadPhaseSapUpdateSpuTask;

namespace Gu
{
	class Axes;
}

namespace Cm
{
	class EventProfiler;
}

class PxLightCpuTask;

class BroadPhaseBatchUpdateWorkTask: public Cm::Task
{
public:

	BroadPhaseBatchUpdateWorkTask() 
		: Cm::Task(), 
		  mSap(NULL),
   	      mAxis(0xffffffff),
		  mPairs(NULL),
		  mPairsSize(0),
		  mPairsCapacity(0)
	{
	}

	virtual void runInternal();

	virtual const char* getName() const { return "PxsBroadPhaseSap.broadPhaseBatchUpdate"; }

	void set(class PxsBroadPhaseContextSap* sap, const PxU32 axis) {mSap = sap; mAxis = axis;}

	PxcBroadPhasePair* getPairs() const {return mPairs;}
	PxU32 getPairsSize() const {return mPairsSize;}
	PxU32 getPairsCapacity() const {return mPairsCapacity;}

	void setPairs(PxcBroadPhasePair* pairs, const PxU32 pairsCapacity) {mPairs = pairs; mPairsCapacity = pairsCapacity;}

	void setNumPairs(const PxU32 pairsSize) {mPairsSize=pairsSize;}

private:

	class PxsBroadPhaseContextSap* mSap;
	PxU32 mAxis;

	PxcBroadPhasePair* mPairs;
	PxU32 mPairsSize;
	PxU32 mPairsCapacity;
};

//KS - TODO, this could be reduced to U16 in smaller scenes
struct PxsBroadPhaseActivityPocket
{
	PxU32 mStartIndex;
	PxU32 mEndIndex;
};


class PxsBroadPhaseContextSap : public PxvBroadPhase
{
	PX_NOCOPY(PxsBroadPhaseContextSap)
public:

	friend class BroadPhaseBatchUpdateWorkTask;
	friend class SapUpdateWorkTask;
	friend class SapPostUpdateWorkTask;

										PxsBroadPhaseContextSap(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler);
	virtual								~PxsBroadPhaseContextSap();

	static PxsBroadPhaseContextSap*		create(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler);
	virtual	void						destroy();

	virtual	PxBroadPhaseType::Enum		getType()																			const	{ return PxBroadPhaseType::eSAP;	}

	virtual	void						update(const PxU32 numCpuTasks, const PxU32 numSpus, const PxcBroadPhaseUpdateData& updateData, PxBaseTask* continuation);

	virtual PxU32						getNumCreatedPairs()		const	{ return mCreatedPairsSize;		}
	virtual PxcBroadPhasePair*			getCreatedPairs()					{ return mCreatedPairsArray;	}
	virtual PxU32						getNumDeletedPairs()		const	{ return mDeletedPairsSize;		}
	virtual PxcBroadPhasePair*			getDeletedPairs()					{ return mDeletedPairsArray;	}

	virtual void						freeBuffers();

	virtual void						shiftOrigin(const PxVec3& shift);
	//~PxvBroadPhase

protected:

#ifdef PX_CHECKED
	virtual bool						isValid(const PxcBroadPhaseUpdateData& updateData) const;
#endif

private:

			PxcScratchAllocator&		mScratchAllocator;
			Cm::EventProfiler&			mEventProfiler;

			SapUpdateWorkTask			mSapUpdateWorkTask;
			SapPostUpdateWorkTask		mSapPostUpdateWorkTask;

	//Data passed in from updateV.
			const PxcBpHandle*			mCreated;				
			PxU32						mCreatedSize;			
			const PxcBpHandle*			mRemoved;				
			PxU32						mRemovedSize;				
			const PxcBpHandle*			mUpdated;				
			PxU32						mUpdatedSize;				
			const IntegerAABB*			mBoxBoundsMinMax;			
			const PxcBpHandle*			mBoxGroups;		
			PxU32						mBoxesCapacity;

	//Boxes.
			SapBox1D*					mBoxEndPts[3];			//Position of box min/max in sorted arrays of end pts (needs to have mBoxesCapacity).

	//End pts (endpts of boxes sorted along each axis).
			PxcBPValType*				mEndPointValues[3];		//Sorted arrays of min and max box coords
			PxcBpHandle*				mEndPointDatas[3];		//Corresponding owner id and isMin/isMax for each entry in the sorted arrays of min and max box coords.
#if BP_UPDATE_BEFORE_SWAP
			PxU8*						mBoxesUpdated;	
			PxcBpHandle*				mSortedUpdateElements;	
			PxsBroadPhaseActivityPocket* mActivityPockets;
			PxcBpHandle*				mListNext;
			PxcBpHandle*				mListPrev;
#endif

			

			PxU32						mBoxesSize;				//Number of sorted boxes + number of unsorted (new) boxes
			PxU32						mBoxesSizePrev;			//Number of sorted boxes 
			PxU32						mEndPointsCapacity;		//Capacity of sorted arrays. 

	//Box-box overlap pairs created or removed each update.
			PxcBpHandle*				mData;
			PxU32						mDataSize;
			PxU32						mDataCapacity;

	//All current box-box overlap pairs.
			SapPairManager				mPairs;

	//Created and deleted overlap pairs reported back through api.
			PxcBroadPhasePair*			mCreatedPairsArray;
			PxU32						mCreatedPairsSize;
			PxU32						mCreatedPairsCapacity;
			PxcBroadPhasePair*			mDeletedPairsArray;
			PxU32						mDeletedPairsSize;
			PxU32						mDeletedPairsCapacity;

			bool						setUpdateData(const PxcBroadPhaseUpdateData& updateData);
			void						updatePPU(PxBaseTask* continuation);
			void						postUpdatePPU(PxBaseTask* continuation);
#ifdef PX_PS3
			bool						canRunOnSpu() const;
			void						updateSPU(const PxU32 numSpus, PxBaseTask* continuation);
			void						postUpdateSPU(const PxU32 numSpus, PxBaseTask* continuation);
#endif

	//Batch create/remove/update.
			void						batchCreate();
			void						batchRemove();
			void						batchUpdate();

			void						batchUpdate(const PxU32 Axis, PxcBroadPhasePair*& pairs, PxU32& pairsSize, PxU32& pairsCapacity);
#if BP_UPDATE_BEFORE_SWAP
			void						batchUpdateFewUpdates(const PxU32 Axis, PxcBroadPhasePair*& pairs, PxU32& pairsSize, PxU32& pairsCapacity);
#endif
			void						performBoxPruning(const Gu::Axes axes);

#ifdef PX_PS3
	friend class BroadPhaseSapRemoveSpuTask;
	friend class BroadPhaseSapCreateSpuTask;
	friend class BroadPhaseSapUpdateSpuTask;
			BroadPhaseSapRemoveSpuTask*	mBroadPhaseRemoveSpuTask;
			BroadPhaseSapCreateSpuTask*	mBroadPhaseCreateSpuTask;
			BroadPhaseSapUpdateSpuTask*	mBroadPhaseUpdateSpuTask;
#endif

			BroadPhaseBatchUpdateWorkTask mBatchUpdateTasks[3];

#ifdef PX_DEBUG
			bool					isSelfOrdered() const;
			bool					isSelfConsistent() const;
#endif
};

} //namespace physx

#endif //PXS_BROADPHASE_SAP_H
