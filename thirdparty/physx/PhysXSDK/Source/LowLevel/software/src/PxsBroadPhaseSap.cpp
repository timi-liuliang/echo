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
#include "CmTmpMem.h"
#include "PxcScratchAllocator.h"
#include "PxMath.h"
#include "PxsBroadPhaseSap.h"
#include "PxsBroadPhaseSapAux.h"
#include "PxsBroadPhaseCommon.h"
#include "GuRevisitedRadixBuffered.h"
#include "CmEventProfiler.h"
#include "PxProfileEventId.h"
#ifdef PX_PS3
#include "CellComputeAABBTask.h"
#include "CellTimerMarker.h"
#endif

using namespace physx;

#ifdef PX_PS3
#define DEFAULT_DATA_ARRAY_CAPACITY MAX_NUM_BP_SPU_SAP_OVERLAPS
#else
#define DEFAULT_DATA_ARRAY_CAPACITY 1024
#endif

#ifdef PX_PS3
#define DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY MAX_NUM_BP_SPU_SAP_OVERLAPS
#else 
#define DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY 64
#endif

#ifdef PX_PS3
#define DEFAULT_CREATEDDELETED1AXIS_CAPACITY MAX_NUM_BP_SPU_SAP_OVERLAPS
#else
#define DEFAULT_CREATEDDELETED1AXIS_CAPACITY 8192
#endif

#ifdef PX_PS3

#include "CellBroadPhaseTask.h"
#include "CellSPUProfiling.h"
#include "PS3Support.h"

#include <PxSpuTask.h>

namespace physx
{

class BroadPhaseSapRemoveSpuTask : public physx::PxSpuTask
{
public:

	BroadPhaseSapRemoveSpuTask (PxsBroadPhaseContextSap* context, PxU32 numSpusToUse)
		: PxSpuTask(gPS3GetElfImage(SPU_ELF_BPSAPREMOVE_TASK), gPS3GetElfSize(SPU_ELF_BPSAPREMOVE_TASK), PxMin((PxU32)1,numSpusToUse)), mContext(context)
	{
		PX_ASSERT(MAX_NUM_BP_SPU_SAP_OVERLAPS==context->mPairs.mHashCapacity);
		PX_ASSERT(MAX_NUM_BP_SPU_SAP_OVERLAPS==context->mPairs.mActivePairsCapacity);
		PX_ASSERT(MAX_NUM_BP_SPU_SAP_OVERLAPS==context->mPairs.mMinAllowedHashCapacity);

		// Prepare the input data that will be passed to all the spus.

		mCellBroadPhaseSapSPUInput.mRemovedAABB=const_cast<PxcBpHandle*>(context->mRemoved);
		mCellBroadPhaseSapSPUInput.mNumRemovedAABB=context->mRemovedSize;

		mCellBroadPhaseSapSPUInput.mAsapBoxes0=context->mBoxEndPts[0];
		mCellBroadPhaseSapSPUInput.mAsapBoxes1=context->mBoxEndPts[1];
		mCellBroadPhaseSapSPUInput.mAsapBoxes2=context->mBoxEndPts[2];
		mCellBroadPhaseSapSPUInput.mNumPrevAsapBoxes=context->mBoxesSizePrev;
		mCellBroadPhaseSapSPUInput.mMaxNumAsapBoxes=context->mBoxesCapacity;

		mCellBroadPhaseSapSPUInput.mAsapEndPointValues0=context->mEndPointValues[0];
		mCellBroadPhaseSapSPUInput.mAsapEndPointValues1=context->mEndPointValues[1];
		mCellBroadPhaseSapSPUInput.mAsapEndPointValues2=context->mEndPointValues[2];
		mCellBroadPhaseSapSPUInput.mAsapEndPointDatas0=context->mEndPointDatas[0];
		mCellBroadPhaseSapSPUInput.mAsapEndPointDatas1=context->mEndPointDatas[1];
		mCellBroadPhaseSapSPUInput.mAsapEndPointDatas2=context->mEndPointDatas[2];
		mCellBroadPhaseSapSPUInput.mNumAsapEndPoints=2*(context->mBoxesSize-context->mCreatedSize)+NUM_SENTINELS;
		mCellBroadPhaseSapSPUInput.mMaxNumAsapEndPoints=context->mEndPointsCapacity;

		mCellBroadPhaseSapSPUInput.mHashTable=context->mPairs.mHashTable;
		mCellBroadPhaseSapSPUInput.mNext=context->mPairs.mNext;
		mCellBroadPhaseSapSPUInput.mNumHashes=context->mPairs.mHashSize;
		mCellBroadPhaseSapSPUInput.mMaxNumHashes=context->mPairs.mHashCapacity;
		mCellBroadPhaseSapSPUInput.mMinAllowedMaxNumHashes=context->mPairs.mHashCapacity;
		mCellBroadPhaseSapSPUInput.mActivePairs=context->mPairs.mActivePairs;
		mCellBroadPhaseSapSPUInput.mActivePairStates=context->mPairs.mActivePairStates;
		mCellBroadPhaseSapSPUInput.mNumActivePairs=context->mPairs.mNbActivePairs;
		mCellBroadPhaseSapSPUInput.mMaxNumActivePairs=context->mPairs.mActivePairsCapacity;
		mCellBroadPhaseSapSPUInput.mMask=context->mPairs.mMask;

#if SPU_PROFILE
		for(PxU32 i=0;i<MAX_NUM_SPU_PROFILE_ZONES;i++)
		{
			mProfileCounters[i]=0;
		}
		mCellBroadPhaseSapSPUInput.mProfileZones=mProfileCounters;
#endif

		// Prepare the output data that will be passed to all the spus.

		mCellBroadPhaseSapSPUOutput.mPairManagerHashSize=mContext->mPairs.mHashSize;
		mCellBroadPhaseSapSPUOutput.mPairManagerNumActivePairs=mContext->mPairs.mNbActivePairs;
		mCellBroadPhaseSapSPUOutput.mPairManagerMask=mContext->mPairs.mMask;

		//Start all the spu tasks.
		setArgs(0, 0 | (unsigned int)&mCellBroadPhaseSapSPUOutput, (unsigned int)&mCellBroadPhaseSapSPUInput);
	}

	virtual void release()
	{
		//Update the pair manager.
		mContext->mPairs.mHashSize=mCellBroadPhaseSapSPUOutput.mPairManagerHashSize;
		mContext->mPairs.mNbActivePairs=mCellBroadPhaseSapSPUOutput.mPairManagerNumActivePairs;
		mContext->mPairs.mMask=mCellBroadPhaseSapSPUOutput.mPairManagerMask;

		//Update the counts of sorted boxes.
		const PxU32 boxesSize=mContext->mBoxesSize;
		const PxU32 removedSize=mContext->mRemovedSize;
		const PxU32 createdSize=mContext->mCreatedSize;
		const PxU32 newBoxesSize=boxesSize-removedSize;
		const PxU32 newBoxesSizePrev=newBoxesSize-createdSize;
		mContext->mBoxesSize=newBoxesSize;
		mContext->mBoxesSizePrev=newBoxesSizePrev;

		PxSpuTask::release();
	}

	const char* getName() const { return "PxsBroadPhaseSap.removeSpu"; }

	PxsBroadPhaseContextSap* mContext;

	CellBroadPhaseSapRemoveSPUInput		PX_ALIGN(128, mCellBroadPhaseSapSPUInput);
	CellBroadPhaseSapRemoveSPUOutput	PX_ALIGN(128, mCellBroadPhaseSapSPUOutput);

#define SPU_PROFILE 0
#if SPU_PROFILE
	PxU64								PX_ALIGN(16, mProfileCounters[MAX_NUM_SPU_PROFILE_ZONES])={0};
#endif
};

class BroadPhaseSapCreateSpuTask : public physx::PxSpuTask
{
public:

	BroadPhaseSapCreateSpuTask(PxsBroadPhaseContextSap* context, PxU32 numSpusToUse) 
		: PxSpuTask(gPS3GetElfImage(SPU_ELF_BPSAPCREATE_TASK), gPS3GetElfSize(SPU_ELF_BPSAPCREATE_TASK), PxMin((PxU32)1,numSpusToUse)), mContext(context)
	{

		PX_ASSERT(MAX_NUM_BP_SPU_SAP_OVERLAPS==context->mPairs.mHashCapacity);
		PX_ASSERT(MAX_NUM_BP_SPU_SAP_OVERLAPS==context->mPairs.mActivePairsCapacity);
		PX_ASSERT(MAX_NUM_BP_SPU_SAP_OVERLAPS==context->mPairs.mMinAllowedHashCapacity);

		// Prepare the input data that will be passed to all the spus.

		mCellBroadPhaseSapSPUInput.mCreatedAABB=const_cast<PxcBpHandle*>(context->mCreated);
		mCellBroadPhaseSapSPUInput.mNumCreatedAABB=context->mCreatedSize;

		mCellBroadPhaseSapSPUInput.mAsapBoxes0=context->mBoxEndPts[0];
		mCellBroadPhaseSapSPUInput.mAsapBoxes1=context->mBoxEndPts[1];
		mCellBroadPhaseSapSPUInput.mAsapBoxes2=context->mBoxEndPts[2];
		mCellBroadPhaseSapSPUInput.mBoxMinMaxXYZ=const_cast<IntegerAABB*>(context->mBoxBoundsMinMax);
		mCellBroadPhaseSapSPUInput.mBoxGroups=const_cast<PxcBpHandle*>(context->mBoxGroups);
		mCellBroadPhaseSapSPUInput.mNumAsapBoxes=context->mBoxesSize;
		mCellBroadPhaseSapSPUInput.mNumPrevAsapBoxes=context->mBoxesSizePrev;
		mCellBroadPhaseSapSPUInput.mMaxNumAsapBoxes=context->mBoxesCapacity;

		mCellBroadPhaseSapSPUInput.mAsapEndPointValues0=context->mEndPointValues[0];
		mCellBroadPhaseSapSPUInput.mAsapEndPointValues1=context->mEndPointValues[1];
		mCellBroadPhaseSapSPUInput.mAsapEndPointValues2=context->mEndPointValues[2];
		mCellBroadPhaseSapSPUInput.mAsapEndPointDatas0=context->mEndPointDatas[0];
		mCellBroadPhaseSapSPUInput.mAsapEndPointDatas1=context->mEndPointDatas[1];
		mCellBroadPhaseSapSPUInput.mAsapEndPointDatas2=context->mEndPointDatas[2];
		mCellBroadPhaseSapSPUInput.mNumAsapEndPoints=2*(context->mBoxesSize-context->mCreatedSize)+NUM_SENTINELS;
		mCellBroadPhaseSapSPUInput.mMaxNumAsapEndPoints=context->mEndPointsCapacity;

		mCellBroadPhaseSapSPUInput.mAxisOrder=Gu::AXES_XZY;

		mCellBroadPhaseSapSPUInput.mHashTable=context->mPairs.mHashTable;
		mCellBroadPhaseSapSPUInput.mNext=context->mPairs.mNext;
		mCellBroadPhaseSapSPUInput.mNumHashes=context->mPairs.mHashSize;
		mCellBroadPhaseSapSPUInput.mMaxNumHashes=context->mPairs.mHashCapacity;
		mCellBroadPhaseSapSPUInput.mMinAllowedMaxNumHashes=context->mPairs.mHashCapacity;
		mCellBroadPhaseSapSPUInput.mActivePairs=context->mPairs.mActivePairs;
		mCellBroadPhaseSapSPUInput.mActivePairStates=context->mPairs.mActivePairStates;
		mCellBroadPhaseSapSPUInput.mNumActivePairs=context->mPairs.mNbActivePairs;
		mCellBroadPhaseSapSPUInput.mMaxNumActivePairs=context->mPairs.mActivePairsCapacity;
		mCellBroadPhaseSapSPUInput.mMask=context->mPairs.mMask;

		mCellBroadPhaseSapSPUInput.mPairs0=mContext->mBatchUpdateTasks[0].getPairs();
		mCellBroadPhaseSapSPUInput.mPairs1=mContext->mBatchUpdateTasks[1].getPairs();
		mCellBroadPhaseSapSPUInput.mPairs2=mContext->mBatchUpdateTasks[2].getPairs();
		mCellBroadPhaseSapSPUInput.mNumPairs0=mContext->mBatchUpdateTasks[0].getPairsSize();
		mCellBroadPhaseSapSPUInput.mNumPairs1=mContext->mBatchUpdateTasks[1].getPairsSize();
		mCellBroadPhaseSapSPUInput.mNumPairs2=mContext->mBatchUpdateTasks[2].getPairsSize();

		mCellBroadPhaseSapSPUInput.mCreatedOverlaps=mContext->mCreatedPairsArray;
		mCellBroadPhaseSapSPUInput.mDeletedOverlaps=mContext->mDeletedPairsArray;

#if SPU_PROFILE
		for(PxU32 i=0;i<MAX_NUM_SPU_PROFILE_ZONES;i++)
		{
			mProfileCounters[i]=0;
		}
		mCellBroadPhaseSapSPUInput.mProfileZones=mProfileCounters;
#endif

		// Prepare the output data that will be passed to all the spus.

		mCellBroadPhaseSapSPUOutput.mPairManagerSuccess=CellBroadPhaseSapCreateSPUOutput::eCOMPLETE;
		mCellBroadPhaseSapSPUOutput.mNumCreatedOverlaps=0;
		mCellBroadPhaseSapSPUOutput.mNumDeletedOverlaps=0;
		mCellBroadPhaseSapSPUOutput.mPairManagerHashSize=mContext->mPairs.mHashSize;	
		mCellBroadPhaseSapSPUOutput.mPairManagerNumActivePairs=mContext->mPairs.mNbActivePairs;
		mCellBroadPhaseSapSPUOutput.mPairManagerMask=mContext->mPairs.mMask;

		//Start all the spu tasks.
		setArgs(0, 0 | (unsigned int)&mCellBroadPhaseSapSPUOutput, (unsigned int)&mCellBroadPhaseSapSPUInput);
	}

	virtual void release()
	{
		if(CellBroadPhaseSapCreateSPUOutput::eCOMPLETE==mCellBroadPhaseSapSPUOutput.mPairManagerSuccess)
		{
			//Update the created/deleted overlaps count
			mContext->mCreatedPairsSize=mCellBroadPhaseSapSPUOutput.mNumCreatedOverlaps;
			mContext->mDeletedPairsSize=mCellBroadPhaseSapSPUOutput.mNumDeletedOverlaps;

			//Update the pair manager.
			mContext->mPairs.mHashSize=mCellBroadPhaseSapSPUOutput.mPairManagerHashSize;
			mContext->mPairs.mNbActivePairs=mCellBroadPhaseSapSPUOutput.mPairManagerNumActivePairs;
			mContext->mPairs.mMask=mCellBroadPhaseSapSPUOutput.mPairManagerMask;

			//Prepare for next update.
			PX_ASSERT(mContext->isSelfConsistent());
			mContext->mBoxesSizePrev=mContext->mBoxesSize;
		}
		else if(CellBroadPhaseSapCreateSPUOutput::eAXIS_PAIRS_NO_CREATED_PAIRS==mCellBroadPhaseSapSPUOutput.mPairManagerSuccess)
		{
			//Update the pair manager.
			mContext->mPairs.mHashSize=mCellBroadPhaseSapSPUOutput.mPairManagerHashSize;
			mContext->mPairs.mNbActivePairs=mCellBroadPhaseSapSPUOutput.mPairManagerNumActivePairs;
			mContext->mPairs.mMask=mCellBroadPhaseSapSPUOutput.mPairManagerMask;

			//Perform the box pruning.
			mContext->performBoxPruning((Gu::AxisOrder)mCellBroadPhaseSapSPUInput.mAxisOrder);

			//Compute the lists of created and deleted overlap pairs.
			ComputeCreatedDeletedPairsLists(
				mContext->mBoxGroups,
				mContext->mData,mContext->mDataSize,
				mContext->mCreatedPairsArray,mContext->mCreatedPairsSize,mContext->mCreatedPairsCapacity,
				mContext->mDeletedPairsArray,mContext->mDeletedPairsSize,mContext->mDeletedPairsCapacity,
				mContext->mPairs);

			//Prepare for next update.
			PX_ASSERT(mContext->isSelfConsistent());
			mContext->mBoxesSizePrev=mContext->mBoxesSize;
		}
		else if(CellBroadPhaseSapCreateSPUOutput::eAXIS_PAIRS==mCellBroadPhaseSapSPUOutput.mPairManagerSuccess)
		{
			//Update the pair manager.
			mContext->mPairs.mHashSize=mCellBroadPhaseSapSPUOutput.mPairManagerHashSize;
			mContext->mPairs.mNbActivePairs=mCellBroadPhaseSapSPUOutput.mPairManagerNumActivePairs;
			mContext->mPairs.mMask=mCellBroadPhaseSapSPUOutput.mPairManagerMask;

			const PxU32 numOverlapPairs=mCellBroadPhaseSapSPUOutput.mNumOverlapPairs;
			const PxcBroadPhasePair* PX_RESTRICT overlapPairs=mContext->mBatchUpdateTasks[0].getPairs();
			for(PxU32 i=0;i<numOverlapPairs;i++)
			{
				const PxU32 index0=overlapPairs[i].mVolA;
				const PxU32 index1=overlapPairs[i].mVolB;
				AddPair(index0,index1,mContext->mPairs,mContext->mData,mContext->mDataSize,mContext->mDataCapacity);
			}

			//Compute the lists of created and deleted overlap pairs.
			ComputeCreatedDeletedPairsLists(
				mContext->mBoxGroups,
				mContext->mData,mContext->mDataSize,
				mContext->mCreatedPairsArray,mContext->mCreatedPairsSize,mContext->mCreatedPairsCapacity,
				mContext->mDeletedPairsArray,mContext->mDeletedPairsSize,mContext->mDeletedPairsCapacity,
				mContext->mPairs);

			//Prepare for next update.
			PX_ASSERT(mContext->isSelfConsistent());
			mContext->mBoxesSizePrev=mContext->mBoxesSize;
		}
		else
		{
			PX_ASSERT(CellBroadPhaseSapCreateSPUOutput::eNOTHING==mCellBroadPhaseSapSPUOutput.mPairManagerSuccess);
			mContext->postUpdatePPU(NULL);
		}

		PxSpuTask::release();
	}

	const char* getName() const { return "PxsBroadPhaseSap.createSpu"; }

	PxsBroadPhaseContextSap* mContext;

	CellBroadPhaseSapCreateSPUInput		PX_ALIGN(128, mCellBroadPhaseSapSPUInput);
	CellBroadPhaseSapCreateSPUOutput	PX_ALIGN(128, mCellBroadPhaseSapSPUOutput);

#define SPU_PROFILE 0
#if SPU_PROFILE
	PxU64								PX_ALIGN(16, mProfileCounters[MAX_NUM_SPU_PROFILE_ZONES])={0};
#endif
};

class BroadPhaseSapUpdateSpuTask : public physx::PxSpuTask
{
public:

	BroadPhaseSapUpdateSpuTask(PxsBroadPhaseContextSap* context, PxU32 numSpusToUse) 
		: PxSpuTask(gPS3GetElfImage(SPU_ELF_BPSAPUPDATE_TASK), gPS3GetElfSize(SPU_ELF_BPSAPUPDATE_TASK), PxMin((PxU32)1,numSpusToUse)), mContext(context)
	{
		PX_ASSERT(MAX_NUM_BP_SPU_SAP_OVERLAPS==context->mPairs.mHashCapacity);
		PX_ASSERT(MAX_NUM_BP_SPU_SAP_OVERLAPS==context->mPairs.mActivePairsCapacity);
		PX_ASSERT(MAX_NUM_BP_SPU_SAP_OVERLAPS==context->mPairs.mMinAllowedHashCapacity);

		// Prepare the input data that will be passed to all the spus.

		//Get the array data for created and updated aabbs.
		mCellBroadPhaseSapSPUInput.mUpdatedAABB=const_cast<PxcBpHandle*>(mContext->mUpdated);
		mCellBroadPhaseSapSPUInput.mNumUpdatedAABB=mContext->mUpdatedSize;

		mCellBroadPhaseSapSPUInput.mAsapBoxes0=context->mBoxEndPts[0];
		mCellBroadPhaseSapSPUInput.mAsapBoxes1=context->mBoxEndPts[1];
		mCellBroadPhaseSapSPUInput.mAsapBoxes2=context->mBoxEndPts[2];
		mCellBroadPhaseSapSPUInput.mBoxMinMaxXYZ=const_cast<IntegerAABB*>(context->mBoxBoundsMinMax);
		mCellBroadPhaseSapSPUInput.mBoxGroups=const_cast<PxcBpHandle*>(context->mBoxGroups);
		mCellBroadPhaseSapSPUInput.mMaxNumAsapBoxes=context->mBoxesCapacity;

		mCellBroadPhaseSapSPUInput.mAsapEndPointValues0=context->mEndPointValues[0];
		mCellBroadPhaseSapSPUInput.mAsapEndPointValues1=context->mEndPointValues[1];
		mCellBroadPhaseSapSPUInput.mAsapEndPointValues2=context->mEndPointValues[2];
		mCellBroadPhaseSapSPUInput.mAsapEndPointDatas0=context->mEndPointDatas[0];
		mCellBroadPhaseSapSPUInput.mAsapEndPointDatas1=context->mEndPointDatas[1];
		mCellBroadPhaseSapSPUInput.mAsapEndPointDatas2=context->mEndPointDatas[2];
		mCellBroadPhaseSapSPUInput.mNumAsapEndPoints=2*(context->mBoxesSize-context->mCreatedSize)+NUM_SENTINELS;
		mCellBroadPhaseSapSPUInput.mMaxNumAsapEndPoints=context->mEndPointsCapacity;

		mCellBroadPhaseSapSPUInput.mPairs0=mContext->mBatchUpdateTasks[0].getPairs();
		mCellBroadPhaseSapSPUInput.mPairs1=mContext->mBatchUpdateTasks[1].getPairs();
		mCellBroadPhaseSapSPUInput.mPairs2=mContext->mBatchUpdateTasks[2].getPairs();

		mCellBroadPhaseSapSPUInput.mActiveTaskCount=getSpuCount();
		mCellBroadPhaseSapSPUInput.mTotalTaskCount=getSpuCount();

#if SPU_PROFILE
		for(PxU32 i=0;i<MAX_NUM_SPU_PROFILE_ZONES;i++)
		{
			mProfileCounters[i]=0;
		}
		mCellBroadPhaseSapSPUInput.mProfileZones=mProfileCounters;
#endif

		// Prepare the output data that will be passed to all the spus.

		for(PxU32 uiTask=0; uiTask < getSpuCount(); uiTask++)
		{
			mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs0=CellBroadPhaseSapUpdateSPUInput::AXIS_SWEEP_UNATTEMPTED;
			mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs1=CellBroadPhaseSapUpdateSPUInput::AXIS_SWEEP_UNATTEMPTED;
			mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs2=CellBroadPhaseSapUpdateSPUInput::AXIS_SWEEP_UNATTEMPTED;
		}

		//Start all the spu tasks.
		for (PxU32 uiTask=0; uiTask < getSpuCount(); uiTask++) 
		{
			setArgs(uiTask, uiTask | (unsigned int)&mCellBroadPhaseSapSPUOutput[uiTask], (unsigned int)&mCellBroadPhaseSapSPUInput);
		}
	}

	virtual void release()
	{
		for (PxU32 uiTask=0; uiTask < getSpuCount(); uiTask++) 
		{
			if(mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs0!=CellBroadPhaseSapUpdateSPUInput::AXIS_SWEEP_UNATTEMPTED)
			{
				if(mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs0!=CellBroadPhaseSapUpdateSPUInput::AXIS_SWEEP_FAILURE)
				{
					mContext->mBatchUpdateTasks[0].setNumPairs(mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs0);
				}
				else
				{
					mContext->mBatchUpdateTasks[0].run();
				}
			}
			if(mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs1!=CellBroadPhaseSapUpdateSPUInput::AXIS_SWEEP_UNATTEMPTED)
			{
				if(mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs1!=CellBroadPhaseSapUpdateSPUInput::AXIS_SWEEP_FAILURE)
				{
					mContext->mBatchUpdateTasks[1].setNumPairs(mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs1);
				}
				else
				{
					mContext->mBatchUpdateTasks[1].run();
				}
			}
			if(mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs2!=CellBroadPhaseSapUpdateSPUInput::AXIS_SWEEP_UNATTEMPTED)
			{
				if(mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs2!=CellBroadPhaseSapUpdateSPUInput::AXIS_SWEEP_FAILURE)
				{
					mContext->mBatchUpdateTasks[2].setNumPairs(mCellBroadPhaseSapSPUOutput[uiTask].mNumPairs2);
				}
				else
				{
					mContext->mBatchUpdateTasks[2].run();
				}
			}
		}

		PxSpuTask::release();
	}

	const char* getName() const { return "PxsBroadPhaseSap.updateSpu"; }

	PxsBroadPhaseContextSap* mContext;

	CellBroadPhaseSapUpdateSPUInput		PX_ALIGN(128, mCellBroadPhaseSapSPUInput);
	CellBroadPhaseSapUpdateSPUOutput	PX_ALIGN(128, mCellBroadPhaseSapSPUOutput[3]);

#define SPU_PROFILE 0
#if SPU_PROFILE
	PxU64								PX_ALIGN(16, mProfileCounters[MAX_NUM_SPU_PROFILE_ZONES])={0};
#endif
};



} // namespace physx

#ifdef PX_PROFILE
#include "libsntuner.h"
#endif 

#endif	// PX_PS3


#define ALIGN_SIZE_16(size) (((unsigned)(size)+15)&((unsigned)~15))

PxsBroadPhaseContextSap::PxsBroadPhaseContextSap(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler)  
: mScratchAllocator(scratchAllocator),
  mEventProfiler(eventProfiler)
{
	//Boxes
	mBoxesSize=0;
	mBoxesSizePrev=0;
	mBoxesCapacity = PX_DEFAULT_BOX_ARRAY_CAPACITY;
	mBoxEndPts[0] = (SapBox1D*)PX_ALLOC(ALIGN_SIZE_16((sizeof(SapBox1D)*mBoxesCapacity)), PX_DEBUG_EXP("SapBox1D"));
	mBoxEndPts[1] = (SapBox1D*)PX_ALLOC(ALIGN_SIZE_16((sizeof(SapBox1D)*mBoxesCapacity)), PX_DEBUG_EXP("SapBox1D"));
	mBoxEndPts[2] = (SapBox1D*)PX_ALLOC(ALIGN_SIZE_16((sizeof(SapBox1D)*mBoxesCapacity)), PX_DEBUG_EXP("SapBox1D"));
	for(PxU32 i=0; i<mBoxesCapacity;i++)
	{
		mBoxEndPts[0][i].mMinMax[0]=PX_INVALID_BP_HANDLE;
		mBoxEndPts[0][i].mMinMax[1]=PX_INVALID_BP_HANDLE;
		mBoxEndPts[1][i].mMinMax[0]=PX_INVALID_BP_HANDLE;
		mBoxEndPts[1][i].mMinMax[1]=PX_INVALID_BP_HANDLE;
		mBoxEndPts[2][i].mMinMax[0]=PX_INVALID_BP_HANDLE;
		mBoxEndPts[2][i].mMinMax[1]=PX_INVALID_BP_HANDLE;
	}
		
	//End points
	mEndPointsCapacity = PX_DEFAULT_BOX_ARRAY_CAPACITY*2 + NUM_SENTINELS;
#if BP_UPDATE_BEFORE_SWAP
	mBoxesUpdated = (PxU8*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxU8)*mBoxesCapacity)), PX_DEBUG_EXP("BoxesUpdated"));
	mSortedUpdateElements = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*mEndPointsCapacity)), PX_DEBUG_EXP("SortedUpdateElements"));
	mActivityPockets = (PxsBroadPhaseActivityPocket*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxsBroadPhaseActivityPocket)*mEndPointsCapacity)), PX_DEBUG_EXP("PxsBroadPhaseActivityPocket"));
	/*mSortHandle[0] = (PxBpActiveEdge*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxBpActiveEdge)*mEndPointsCapacity)), PX_DEBUG_EXP("PxBpActiveEdge"));
	mSortHandle[1] = (PxBpActiveEdge*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxBpActiveEdge)*mEndPointsCapacity)), PX_DEBUG_EXP("PxBpActiveEdge"));
	mSortHandle[2] = (PxBpActiveEdge*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxBpActiveEdge)*mEndPointsCapacity)), PX_DEBUG_EXP("PxBpActiveEdge"));*/
#endif
	mEndPointValues[0] = (PxcBPValType*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBPValType)*(mEndPointsCapacity))), PX_DEBUG_EXP("BPValType"));
	mEndPointValues[1] = (PxcBPValType*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBPValType)*(mEndPointsCapacity))), PX_DEBUG_EXP("BPValType"));
	mEndPointValues[2] = (PxcBPValType*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBPValType)*(mEndPointsCapacity))), PX_DEBUG_EXP("BPValType"));
	mEndPointDatas[0] = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*(mEndPointsCapacity))), PX_DEBUG_EXP("PxBpHandle"));
	mEndPointDatas[1] = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*(mEndPointsCapacity))), PX_DEBUG_EXP("PxBpHandle"));
	mEndPointDatas[2]=  (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*(mEndPointsCapacity))), PX_DEBUG_EXP("PxBpHandle"));

	// Initialize sentinels
	setMinSentinel(mEndPointValues[0][0],mEndPointDatas[0][0]);
	setMaxSentinel(mEndPointValues[0][1],mEndPointDatas[0][1]);
	setMinSentinel(mEndPointValues[1][0],mEndPointDatas[1][0]);
	setMaxSentinel(mEndPointValues[1][1],mEndPointDatas[1][1]);
	setMinSentinel(mEndPointValues[2][0],mEndPointDatas[2][0]);
	setMaxSentinel(mEndPointValues[2][1],mEndPointDatas[2][1]);

	//Initialise data array.
	mData=(PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16(sizeof(PxcBpHandle)*DEFAULT_DATA_ARRAY_CAPACITY), PX_DEBUG_EXP("PxBpHandle"));
	mDataSize=0;
	mDataCapacity=DEFAULT_DATA_ARRAY_CAPACITY;

	//Initialise pairs arrays.
	mCreatedPairsSize=0;
	mDeletedPairsSize=0;
	mCreatedPairsArray=(PxcBroadPhasePair*)PX_ALLOC(ALIGN_SIZE_16(sizeof(PxcBroadPhasePair)*DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY), PX_DEBUG_EXP("PxcBroadPhasePair"));
	mCreatedPairsCapacity=DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY;
	mDeletedPairsArray=(PxcBroadPhasePair*)PX_ALLOC(ALIGN_SIZE_16(sizeof(PxcBroadPhasePair)*DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY), PX_DEBUG_EXP("PxcBroadPhasePair"));
	mDeletedPairsCapacity=DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY;
#if BP_UPDATE_BEFORE_SWAP
	mListNext = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*mEndPointsCapacity)), PX_DEBUG_EXP("NextList"));
	mListPrev = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*mEndPointsCapacity)), PX_DEBUG_EXP("PrevList"));


	for(PxU32 a = 1; a < mEndPointsCapacity; ++a)
	{
		mListNext[a-1] = (PxcBpHandle)a;
		mListPrev[a] = (PxcBpHandle)(a-1);
	}
	mListNext[mEndPointsCapacity-1] = (PxcBpHandle)(mEndPointsCapacity-1);
	mListPrev[0] = 0;
#endif


	//Initialise pair manager arrays.
	mPairs.init(DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY);

	mBatchUpdateTasks[2].set(this,2);
	mBatchUpdateTasks[1].set(this,1);
	mBatchUpdateTasks[0].set(this,0);
	mBatchUpdateTasks[2].setPairs((PxcBroadPhasePair*)PX_ALLOC(sizeof(PxcBroadPhasePair)*DEFAULT_CREATEDDELETED1AXIS_CAPACITY, PX_DEBUG_EXP("PxcBroadPhasePair")), DEFAULT_CREATEDDELETED1AXIS_CAPACITY);
	mBatchUpdateTasks[1].setPairs((PxcBroadPhasePair*)PX_ALLOC(sizeof(PxcBroadPhasePair)*DEFAULT_CREATEDDELETED1AXIS_CAPACITY, PX_DEBUG_EXP("PxcBroadPhasePair")), DEFAULT_CREATEDDELETED1AXIS_CAPACITY);
	mBatchUpdateTasks[0].setPairs((PxcBroadPhasePair*)PX_ALLOC(sizeof(PxcBroadPhasePair)*DEFAULT_CREATEDDELETED1AXIS_CAPACITY, PX_DEBUG_EXP("PxcBroadPhasePair")), DEFAULT_CREATEDDELETED1AXIS_CAPACITY);

#ifdef PX_PS3
	mBroadPhaseRemoveSpuTask = (BroadPhaseSapRemoveSpuTask*)Ps::AlignedAllocator<128>().allocate(sizeof(BroadPhaseSapRemoveSpuTask), __FILE__, __LINE__);
	mBroadPhaseCreateSpuTask = (BroadPhaseSapCreateSpuTask*)Ps::AlignedAllocator<128>().allocate(sizeof(BroadPhaseSapCreateSpuTask), __FILE__, __LINE__);
	mBroadPhaseUpdateSpuTask = (BroadPhaseSapUpdateSpuTask*)Ps::AlignedAllocator<128>().allocate(sizeof(BroadPhaseSapUpdateSpuTask), __FILE__, __LINE__);
#endif
}

PxsBroadPhaseContextSap::~PxsBroadPhaseContextSap()
{
	PX_FREE(mBoxEndPts[0]);
	PX_FREE(mBoxEndPts[1]);
	PX_FREE(mBoxEndPts[2]);

	PX_FREE(mEndPointValues[0]);
	PX_FREE(mEndPointValues[1]);
	PX_FREE(mEndPointValues[2]);
	PX_FREE(mEndPointDatas[0]);
	PX_FREE(mEndPointDatas[1]);
	PX_FREE(mEndPointDatas[2]);

	PX_FREE(mData);

	PX_FREE(mCreatedPairsArray);
	PX_FREE(mDeletedPairsArray);

	mPairs.release();

	PX_FREE(mBatchUpdateTasks[0].getPairs());
	PX_FREE(mBatchUpdateTasks[1].getPairs());
	PX_FREE(mBatchUpdateTasks[2].getPairs());

#if BP_UPDATE_BEFORE_SWAP
	PX_FREE(mListNext);
	PX_FREE(mListPrev);

	PX_FREE(mSortedUpdateElements);
	PX_FREE(mActivityPockets);
	PX_FREE(mBoxesUpdated);
#endif

#ifdef PX_PS3
	Ps::AlignedAllocator<128>().deallocate(mBroadPhaseRemoveSpuTask);
	Ps::AlignedAllocator<128>().deallocate(mBroadPhaseCreateSpuTask);
	Ps::AlignedAllocator<128>().deallocate(mBroadPhaseUpdateSpuTask);
#endif
}

void PxsBroadPhaseContextSap::destroy()
{
	this->~PxsBroadPhaseContextSap();
	PX_FREE(this);
}

PxsBroadPhaseContextSap* PxsBroadPhaseContextSap::create(PxcScratchAllocator& scratchAllocator, Cm::EventProfiler& eventProfiler)
{	
	PxsBroadPhaseContextSap* bpc = reinterpret_cast<PxsBroadPhaseContextSap*>(PX_ALLOC(sizeof(PxsBroadPhaseContextSap), PX_DEBUG_EXP("PxsBroadPhaseContextSap")));

	if(bpc)
	{
		new(bpc) PxsBroadPhaseContextSap(scratchAllocator, eventProfiler);
	}
	return bpc;
}

static void resizeCreatedDeletedPairs(PxcBroadPhasePair*& pairs, PxU32& pairsCapacity)
{
	if(pairsCapacity!=DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY)
	{
		PxcBroadPhasePair* newPairs=(PxcBroadPhasePair*)PX_ALLOC(ALIGN_SIZE_16(sizeof(PxcBroadPhasePair)*DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY), PX_DEBUG_EXP("PxcBroadPhasePair"));
		if(pairs) PX_FREE(pairs);
		pairs=newPairs;
		pairsCapacity=DEFAULT_CREATEDDELETED_PAIR_ARRAY_CAPACITY;
	}
}

void PxsBroadPhaseContextSap::freeBuffers()
{
	//Free created/deleted pairs arrays (don't need these until next update)
	mCreatedPairsSize=0;
	mDeletedPairsSize=0;
	resizeCreatedDeletedPairs(mCreatedPairsArray,mCreatedPairsCapacity);
	resizeCreatedDeletedPairs(mDeletedPairsArray,mDeletedPairsCapacity);

	//Free data array (don't need this array until next update).
	mDataSize=0;
	if(mDataCapacity>DEFAULT_DATA_ARRAY_CAPACITY)
	{
		PX_FREE(mData);
		mData=(PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16(sizeof(PxcBpHandle)*DEFAULT_DATA_ARRAY_CAPACITY), PX_DEBUG_EXP("PxBpHandle"));
		mDataCapacity=DEFAULT_DATA_ARRAY_CAPACITY;
	}

	//Free created/deleted pairs for each axis sweep (don't need these until next update).
	for(PxU32 i=0;i<3;i++)
	{
		if(mBatchUpdateTasks[i].getPairsCapacity() > DEFAULT_CREATEDDELETED1AXIS_CAPACITY)
		{
			PX_FREE(mBatchUpdateTasks[i].getPairs());
			mBatchUpdateTasks[i].setPairs((PxcBroadPhasePair*)PX_ALLOC(sizeof(PxcBroadPhasePair)*DEFAULT_CREATEDDELETED1AXIS_CAPACITY, PX_DEBUG_EXP("PxcBroadPhasePair")), DEFAULT_CREATEDDELETED1AXIS_CAPACITY);
		}
		mBatchUpdateTasks[i].setNumPairs(0);
	}

	//Shrink pair manager buffers it they are larger than needed but only let them shrink to a minimum size.
	mPairs.shrinkMemory();
}

PX_FORCE_INLINE static void shiftCoord3(const PxcBPValType val0, const PxcBpHandle handle0,
										const PxcBPValType val1, const PxcBpHandle handle1,
										const PxcBPValType val2, const PxcBpHandle handle2,
										const PxF32* shift, PxcBPValType& oVal0, PxcBPValType& oVal1, PxcBPValType& oVal2)
{
	PX_ASSERT(!isSentinel(handle0));
	PX_ASSERT(!isSentinel(handle1));
	PX_ASSERT(!isSentinel(handle2));

	PxF32 fl0, fl1, fl2;
	PxcBPValType* PX_RESTRICT bpVal0 = PxUnionCast<PxcBPValType*, PxF32*>(&fl0);
	PxcBPValType* PX_RESTRICT bpVal1 = PxUnionCast<PxcBPValType*, PxF32*>(&fl1);
	PxcBPValType* PX_RESTRICT bpVal2 = PxUnionCast<PxcBPValType*, PxF32*>(&fl2);
	*bpVal0 = PxsBpDecodeFloat(val0);
	*bpVal1 = PxsBpDecodeFloat(val1);
	*bpVal2 = PxsBpDecodeFloat(val2);
	fl0 -= shift[0];
	fl1 -= shift[1];
	fl2 -= shift[2];
	oVal0 = (isMax(handle0)) ? (PxsBpEncodeFloatMax(*bpVal0) | 1) : ((PxsBpEncodeFloatMin(*bpVal0) + 1) & ~1);
	oVal1 = (isMax(handle1)) ? (PxsBpEncodeFloatMax(*bpVal1) | 1) : ((PxsBpEncodeFloatMin(*bpVal1) + 1) & ~1);
	oVal2 = (isMax(handle2)) ? (PxsBpEncodeFloatMax(*bpVal2) | 1) : ((PxsBpEncodeFloatMin(*bpVal2) + 1) & ~1);
}

PX_FORCE_INLINE static void testPostShiftOrder(const PxcBPValType prevVal, PxcBPValType& currVal, const PxcBpHandle prevIsMax, const PxcBpHandle currIsMax)
{
	if(currVal < prevVal)
	{
		//The order has been broken by the lossy shift.
		//Correct currVal so that it is greater than prevVal.
		//If currVal is a box max then ensure that the box is of finite extent.
		const PxcBPValType shiftCorrection = (prevIsMax==currIsMax) ? (PxcBPValType)0 : (PxcBPValType)1;
		currVal = prevVal + shiftCorrection;
	}
}

void PxsBroadPhaseContextSap::shiftOrigin(const PxVec3& shift)
{
	//
	// Note: shifting the bounds does not necessarily preserve the order of the broadphase interval endpoints. The encoding of the float bounds is a lossy
	//       operation, thus it is not possible to get the original float values back and shift them. The only goal of this method is to shift the endpoints
	//       such that the order is preserved. The new intervals might no reflect the correct bounds! Since all bounds have been marked dirty, they will get
	//       recomputed in the next frame anyway. This method makes sure that the next frame update can start from a valid configuration that is close to
	//       the correct one and does not require too many swaps.
	//

	if(0==mBoxesSize)
	{
		return;
	}

	//
	// Note: processing all the axis at once improved performance on XBox 360 and PS3 because it allows to compensate for stalls
	//

	const PxF32 shiftAxis[3] = { shift.x, shift.y, shift.z };
	const PxcBpHandle* PX_RESTRICT epData0 = mEndPointDatas[0];
	PxcBPValType* PX_RESTRICT epValues0 = mEndPointValues[0];
	const PxcBpHandle* PX_RESTRICT epData1 = mEndPointDatas[1];
	PxcBPValType* PX_RESTRICT epValues1 = mEndPointValues[1];
	const PxcBpHandle* PX_RESTRICT epData2 = mEndPointDatas[2];
	PxcBPValType* PX_RESTRICT epValues2 = mEndPointValues[2];
#if defined(PX_PS3) || defined(PX_X360)  // on PC prefetching had a negative effect
	Ps::prefetchLine(epData0);
	Ps::prefetchLine(epValues0);
	Ps::prefetchLine(epData1);
	Ps::prefetchLine(epValues1);
	Ps::prefetchLine(epData2);
	Ps::prefetchLine(epValues2);
#endif

	//Shift the first value in the array of sorted values.
	{
		//Shifted min (first element must be a min by definition).
		shiftCoord3(epValues0[1], epData0[1], epValues1[1], epData1[1], epValues2[1], epData2[1], shiftAxis, epValues0[1], epValues1[1], epValues2[1]);
		PX_ASSERT(!isMax(epData0[1]));
		PX_ASSERT(!isMax(epData1[1]));
		PX_ASSERT(!isMax(epData2[1]));
	}

	//Shift the remainder.
	PxcBPValType prevVal0 = epValues0[1];
	PxcBpHandle prevIsMax0 = isMax(epData0[1]);
	PxcBPValType prevVal1 = epValues1[1];
	PxcBpHandle prevIsMax1 = isMax(epData1[1]);
	PxcBPValType prevVal2 = epValues2[1];
	PxcBpHandle prevIsMax2 = isMax(epData2[1]);
	for(PxU32 i=2; i <= mBoxesSize*2; i++)
	{
		const PxcBpHandle handle0 = epData0[i];
		const PxcBpHandle handle1 = epData1[i];
		const PxcBpHandle handle2 = epData2[i];
		PX_ASSERT(!isSentinel(handle0));
		PX_ASSERT(!isSentinel(handle1));
		PX_ASSERT(!isSentinel(handle2));

		//Get the relevant prev and curr values after the shift.
		const PxcBpHandle currIsMax0 = isMax(epData0[i]);
		const PxcBpHandle currIsMax1 = isMax(epData1[i]);
		const PxcBpHandle currIsMax2 = isMax(epData2[i]);
		PxcBPValType currVal0, currVal1, currVal2;
		shiftCoord3(epValues0[i], handle0, epValues1[i], handle1, epValues2[i], handle2, shiftAxis, currVal0, currVal1, currVal2);
			
		//Test if the order has been preserved by the lossy shift.
		testPostShiftOrder(prevVal0, currVal0, prevIsMax0, currIsMax0);
		testPostShiftOrder(prevVal1, currVal1, prevIsMax1, currIsMax1);
		testPostShiftOrder(prevVal2, currVal2, prevIsMax2, currIsMax2);
		
		prevIsMax0 = currIsMax0;
		prevVal0 = currVal0;
		prevIsMax1 = currIsMax1;
		prevVal1 = currVal1;
		prevIsMax2 = currIsMax2;
		prevVal2 = currVal2;

		epValues0[i] = currVal0;
		epValues1[i] = currVal1;
		epValues2[i] = currVal2;

#if defined(PX_PS3) || defined(PX_X360)
		Ps::prefetchLine(epData0 + i + 16);
		Ps::prefetchLine(epValues0 + i + 16);
		Ps::prefetchLine(epData1 + i + 16);
		Ps::prefetchLine(epValues1 + i + 16);
		Ps::prefetchLine(epData2 + i + 16);
		Ps::prefetchLine(epValues2 + i + 16);
#endif
	}

	PX_ASSERT(isSelfOrdered());
}

#ifdef PX_CHECKED
bool PxsBroadPhaseContextSap::isValid(const PxcBroadPhaseUpdateData& updateData) const
{
	//Test that the created bounds haven't been added already (without first being removed).
	const PxcBpHandle* created=updateData.getCreatedHandles();
	const PxU32 numCreated=updateData.getNumCreatedHandles();
	for(PxU32 i=0;i<numCreated;i++)
	{
		const PxcBpHandle id=created[i];

		//If id >=mBoxesCapacity then we need to resize to add this id, meaning that the id must be new.
		if(id<mBoxesCapacity)
		{
			for(PxU32 j=0;j<3;j++)
			{
				const SapBox1D& box1d=mBoxEndPts[j][id];
				if(box1d.mMinMax[0] != PX_INVALID_BP_HANDLE && box1d.mMinMax[0] != PX_REMOVED_BP_HANDLE)
				{
					//This box has been added already but without being removed.
					return false;
				}
				if(box1d.mMinMax[1] != PX_INVALID_BP_HANDLE && box1d.mMinMax[1] != PX_REMOVED_BP_HANDLE)
				{
					//This box has been added already but without being removed.
					return false;
				}
			}
		}
	}

	//Test that the updated bounds have valid ids.
	const PxcBpHandle* updated=updateData.getUpdatedHandles();
	const PxU32 numUpdated=updateData.getNumUpdatedHandles();
	for(PxU32 i=0;i<numUpdated;i++)
	{
		const PxcBpHandle id = updated[i];
		if(id >= mBoxesCapacity)
		{
			return false;
		}
	}

	//Test that the updated bounds have been been added without being removed.
	for(PxU32 i=0;i<numUpdated;i++)
	{
		const PxcBpHandle id = updated[i];

		for(PxU32 j=0;j<3;j++)
		{
			const SapBox1D& box1d=mBoxEndPts[j][id];

			if(PX_INVALID_BP_HANDLE == box1d.mMinMax[0] || PX_REMOVED_BP_HANDLE == box1d.mMinMax[0])
			{
				//This box has either not been added or has been removed
				return false;
			}
			if(PX_INVALID_BP_HANDLE == box1d.mMinMax[1] || PX_REMOVED_BP_HANDLE == box1d.mMinMax[1])
			{
				//This box has either not been added or has been removed
				return false;
			}
		}
	}

	//Test that the removed bounds have valid ids.
	const PxcBpHandle* removed=updateData.getRemovedHandles();
	const PxU32 numRemoved=updateData.getNumRemovedHandles();
	for(PxU32 i=0;i<numRemoved;i++)
	{
		const PxcBpHandle id = removed[i];
		if(id >= mBoxesCapacity)
		{
			return false;
		}
	}

	//Test that the removed bounds have already been added and haven't been removed.
	for(PxU32 i=0;i<numRemoved;i++)
	{
		const PxcBpHandle id = removed[i];

		for(PxU32 j=0;j<3;j++)
		{
			const SapBox1D& box1d=mBoxEndPts[j][id];

			if(PX_INVALID_BP_HANDLE == box1d.mMinMax[0] || PX_REMOVED_BP_HANDLE == box1d.mMinMax[0])
			{
				//This box has either not been added or has been removed
				return false;
			}
			if(PX_INVALID_BP_HANDLE == box1d.mMinMax[1] || PX_REMOVED_BP_HANDLE == box1d.mMinMax[1])
			{
				//This box has either not been added or has been removed
				return false;
			}
		}
	}

	return true;
}
#endif

void PxsBroadPhaseContextSap::update( const PxU32 numCpuTasks, const PxU32 numSpus, const PxcBroadPhaseUpdateData& updateData, PxBaseTask* continuation)
{
	const bool success = setUpdateData(updateData);

	if(success)
	{
		mSapPostUpdateWorkTask.setBroadPhase(this);
		mSapUpdateWorkTask.setBroadPhase(this);

		mSapPostUpdateWorkTask.set(numCpuTasks, numSpus);
		mSapUpdateWorkTask.set(numCpuTasks, numSpus);

		mSapPostUpdateWorkTask.setContinuation(continuation);
		mSapUpdateWorkTask.setContinuation(&mSapPostUpdateWorkTask);

		mSapPostUpdateWorkTask.removeReference();
		mSapUpdateWorkTask.removeReference();
	}
}

bool PxsBroadPhaseContextSap::setUpdateData(const PxcBroadPhaseUpdateData& updateData) 
{
	PX_ASSERT(0==mCreatedPairsSize);
	PX_ASSERT(0==mDeletedPairsSize);

#ifdef PX_CHECKED
	if(!PxcBroadPhaseUpdateData::isValid(updateData, *this))
	{
		PX_CHECK_MSG(false, "Illegal PxcBroadPhaseUpdateData \n");
		mCreated			= NULL;
		mCreatedSize		= 0;
		mUpdated			= NULL;
		mUpdatedSize		= 0;
		mRemoved			= NULL;
		mRemovedSize		= 0;
		mBoxBoundsMinMax	= updateData.getAABBs();
		mBoxGroups			= updateData.getGroups();
		return false;
	}
#endif

	//Copy across the data ptrs and sizes.
	mCreated			= updateData.getCreatedHandles();
	mCreatedSize		= updateData.getNumCreatedHandles();
	mUpdated			= updateData.getUpdatedHandles();
	mUpdatedSize		= updateData.getNumUpdatedHandles();
	mRemoved			= updateData.getRemovedHandles();
	mRemovedSize		= updateData.getNumRemovedHandles();
	mBoxBoundsMinMax	= updateData.getAABBs();
	mBoxGroups			= updateData.getGroups();

	//Do we need more memory to store the positions of each box min/max in the arrays of sorted boxes min/max?
	if(updateData.getCapacity() > mBoxesCapacity)
	{
		const PxU32 oldBoxesCapacity=mBoxesCapacity;
		const PxU32 newBoxesCapacity=updateData.getCapacity();
		SapBox1D* newBoxEndPts0 = (SapBox1D*)PX_ALLOC(ALIGN_SIZE_16((sizeof(SapBox1D)*newBoxesCapacity)), PX_DEBUG_EXP("SapBox1D"));
		SapBox1D* newBoxEndPts1 = (SapBox1D*)PX_ALLOC(ALIGN_SIZE_16((sizeof(SapBox1D)*newBoxesCapacity)), PX_DEBUG_EXP("SapBox1D"));
		SapBox1D* newBoxEndPts2 = (SapBox1D*)PX_ALLOC(ALIGN_SIZE_16((sizeof(SapBox1D)*newBoxesCapacity)), PX_DEBUG_EXP("SapBox1D"));

		PxMemCopy(newBoxEndPts0, mBoxEndPts[0], sizeof(SapBox1D)*oldBoxesCapacity);
		PxMemCopy(newBoxEndPts1, mBoxEndPts[1], sizeof(SapBox1D)*oldBoxesCapacity);
		PxMemCopy(newBoxEndPts2, mBoxEndPts[2], sizeof(SapBox1D)*oldBoxesCapacity);
		for(PxU32 i=oldBoxesCapacity;i<newBoxesCapacity;i++)
		{
			newBoxEndPts0[i].mMinMax[0]=PX_INVALID_BP_HANDLE;
			newBoxEndPts0[i].mMinMax[1]=PX_INVALID_BP_HANDLE;
			newBoxEndPts1[i].mMinMax[0]=PX_INVALID_BP_HANDLE;
			newBoxEndPts1[i].mMinMax[1]=PX_INVALID_BP_HANDLE;
			newBoxEndPts2[i].mMinMax[0]=PX_INVALID_BP_HANDLE;
			newBoxEndPts2[i].mMinMax[1]=PX_INVALID_BP_HANDLE;
		}
		PX_FREE(mBoxEndPts[0]);
		PX_FREE(mBoxEndPts[1]);
		PX_FREE(mBoxEndPts[2]);
		mBoxEndPts[0] = newBoxEndPts0;
		mBoxEndPts[1] = newBoxEndPts1;
		mBoxEndPts[2] = newBoxEndPts2;
		mBoxesCapacity = newBoxesCapacity;

		

#if BP_UPDATE_BEFORE_SWAP
		PX_FREE(mBoxesUpdated);
		mBoxesUpdated = (PxU8*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxU8))*newBoxesCapacity), PX_DEBUG_EXP("Updated Boxes"));		
#endif
	}

	//Do we need more memory for the array of sorted boxes?
	if(2*(mBoxesSize + mCreatedSize) + NUM_SENTINELS > mEndPointsCapacity)
	{
		const PxU32 newEndPointsCapacity = 2*(mBoxesSize + mCreatedSize) + NUM_SENTINELS;

		PxcBPValType* newEndPointValuesX = (PxcBPValType*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBPValType)*(newEndPointsCapacity))), PX_DEBUG_EXP("BPValType"));
		PxcBPValType* newEndPointValuesY = (PxcBPValType*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBPValType)*(newEndPointsCapacity))), PX_DEBUG_EXP("BPValType"));
		PxcBPValType* newEndPointValuesZ = (PxcBPValType*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBPValType)*(newEndPointsCapacity))), PX_DEBUG_EXP("BPValType"));
		PxcBpHandle* newEndPointDatasX = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*(newEndPointsCapacity))), PX_DEBUG_EXP("PxBpHandle"));
		PxcBpHandle* newEndPointDatasY = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*(newEndPointsCapacity))), PX_DEBUG_EXP("PxBpHandle"));
		PxcBpHandle* newEndPointDatasZ = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*(newEndPointsCapacity))), PX_DEBUG_EXP("PxBpHandle"));

#if BP_UPDATE_BEFORE_SWAP
		PX_FREE(mListNext);
		PX_FREE(mListPrev);

		mListNext = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*newEndPointsCapacity)), PX_DEBUG_EXP("NextList"));
		mListPrev = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*newEndPointsCapacity)), PX_DEBUG_EXP("Prev"));


		for(PxU32 a = 1; a < newEndPointsCapacity; ++a)
		{
			mListNext[a-1] = (PxcBpHandle)a;
			mListPrev[a] = (PxcBpHandle)(a-1);
		}
		mListNext[newEndPointsCapacity-1] = (PxcBpHandle)(newEndPointsCapacity-1);
		mListPrev[0] = 0;
#endif

		PxMemCopy(newEndPointValuesX, mEndPointValues[0], sizeof(PxcBPValType)*(mBoxesSize*2+NUM_SENTINELS));
		PxMemCopy(newEndPointValuesY, mEndPointValues[1], sizeof(PxcBPValType)*(mBoxesSize*2+NUM_SENTINELS));
		PxMemCopy(newEndPointValuesZ, mEndPointValues[2], sizeof(PxcBPValType)*(mBoxesSize*2+NUM_SENTINELS));
		PxMemCopy(newEndPointDatasX, mEndPointDatas[0], sizeof(PxcBpHandle)*(mBoxesSize*2+NUM_SENTINELS));
		PxMemCopy(newEndPointDatasY, mEndPointDatas[1], sizeof(PxcBpHandle)*(mBoxesSize*2+NUM_SENTINELS));
		PxMemCopy(newEndPointDatasZ, mEndPointDatas[2], sizeof(PxcBpHandle)*(mBoxesSize*2+NUM_SENTINELS));
		PX_FREE(mEndPointValues[0]);
		PX_FREE(mEndPointValues[1]);
		PX_FREE(mEndPointValues[2]);
		PX_FREE(mEndPointDatas[0]);
		PX_FREE(mEndPointDatas[1]);
		PX_FREE(mEndPointDatas[2]);
		mEndPointValues[0] = newEndPointValuesX;
		mEndPointValues[1] = newEndPointValuesY;
		mEndPointValues[2] = newEndPointValuesZ;
		mEndPointDatas[0] = newEndPointDatasX;
		mEndPointDatas[1] = newEndPointDatasY;
		mEndPointDatas[2] = newEndPointDatasZ;
		mEndPointsCapacity = newEndPointsCapacity;

#if BP_UPDATE_BEFORE_SWAP
		PX_FREE(mSortedUpdateElements);
		PX_FREE(mActivityPockets);
		mSortedUpdateElements = (PxcBpHandle*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxcBpHandle)*newEndPointsCapacity)), PX_DEBUG_EXP("SortedUpdateElements"));
		mActivityPockets = (PxsBroadPhaseActivityPocket*)PX_ALLOC(ALIGN_SIZE_16((sizeof(PxsBroadPhaseActivityPocket)*newEndPointsCapacity)), PX_DEBUG_EXP("PxsBroadPhaseActivityPocket"));
#endif

	}

#if BP_UPDATE_BEFORE_SWAP
	PxMemZero(mBoxesUpdated, sizeof(PxU8) * (mBoxesCapacity));	

	for(PxU32 a=0;a<mUpdatedSize;a++)
	{
		const PxU32 handle=mUpdated[a];
		mBoxesUpdated[handle] = 1;
	}
#endif
	

	//Update the size of the sorted boxes arrays.
	PX_ASSERT(mBoxesSize==mBoxesSizePrev);
	mBoxesSize += mCreatedSize;
	PX_ASSERT(2*mBoxesSize+NUM_SENTINELS <= mEndPointsCapacity);

	return true;
}

#ifdef PX_PS3

bool PxsBroadPhaseContextSap::canRunOnSpu() const
{
	return 
	(
		(mCreatedSize > 0 || mUpdatedSize > 0 || mRemovedSize > 0) &&
		mBoxesCapacity<=MAX_NUM_BP_SPU_SAP_AABB && 
		mBoxesSize<=MAX_NUM_BP_SPU_SAP_AABB && 
		mPairs.mHashCapacity<=MAX_NUM_BP_SPU_SAP_OVERLAPS && 
		mBatchUpdateTasks[0].getPairsCapacity()<=MAX_NUM_BP_SPU_SAP_OVERLAPS && 
		mBatchUpdateTasks[1].getPairsCapacity()<=MAX_NUM_BP_SPU_SAP_OVERLAPS && 
		mBatchUpdateTasks[2].getPairsCapacity()<=MAX_NUM_BP_SPU_SAP_OVERLAPS
	);
}

void PxsBroadPhaseContextSap::updateSPU(const PxU32 numSpusToUse, PxBaseTask* continuation)
{
#ifdef PX_PROFILE
	CM_PROFILE_START_CROSSTHREAD(mEventProfiler, Cm::ProfileEventId::BroadPhase::GetSapUpdateSPU());
#ifdef PX_PS3
	startTimerMarker(eUPDATE_SAP_SPU);
#endif
#endif

	//Check that spu update is valid.
	PX_ASSERT(canRunOnSpu() && numSpusToUse>0);

	//Check that the input data is valid.
	PX_ASSERT((mBoxesSize-mCreatedSize) == mBoxesSizePrev);

	//Check that the overlap pairs per axis have been reset.
	PX_ASSERT(0==mBatchUpdateTasks[0].getPairsSize());
	PX_ASSERT(0==mBatchUpdateTasks[1].getPairsSize());
	PX_ASSERT(0==mBatchUpdateTasks[2].getPairsSize());

	//Start the spu tasks:  batchRemove then batchUpdate.
	mBroadPhaseUpdateSpuTask = PX_PLACEMENT_NEW(mBroadPhaseUpdateSpuTask, BroadPhaseSapUpdateSpuTask)(this, numSpusToUse);
	if(0!=mRemovedSize)
	{
		mBroadPhaseRemoveSpuTask = PX_PLACEMENT_NEW(mBroadPhaseRemoveSpuTask, BroadPhaseSapRemoveSpuTask)(this, numSpusToUse);
	}
	mBroadPhaseUpdateSpuTask->setContinuation(continuation);
	if(0!=mRemovedSize)
	{
		mBroadPhaseRemoveSpuTask->setContinuation(mBroadPhaseUpdateSpuTask);
	}
	mBroadPhaseUpdateSpuTask->removeReference();
	if(0!=mRemovedSize)
	{
		mBroadPhaseRemoveSpuTask->removeReference();
	}
}

void PxsBroadPhaseContextSap::postUpdateSPU(const PxU32 numSpusToUse, PxBaseTask* continuation)
{
#ifdef PX_PROFILE
	CM_PROFILE_STOP_CROSSTHREAD(mEventProfiler, Cm::ProfileEventId::BroadPhase::GetSapUpdateSPU());
	CM_PROFILE_START_CROSSTHREAD(mEventProfiler, Cm::ProfileEventId::BroadPhase::GetSapPostUpdateSPU());
#ifdef PX_PS3
	stopTimerMarker(eUPDATE_SAP_SPU);
	startTimerMarker(ePOSTUPDATE_SAP_SPU);
#endif
#endif

	mBroadPhaseCreateSpuTask = PX_PLACEMENT_NEW(mBroadPhaseCreateSpuTask, BroadPhaseSapCreateSpuTask)(this, numSpusToUse);
	mBroadPhaseCreateSpuTask->setContinuation(continuation);
	mBroadPhaseCreateSpuTask->removeReference();

#ifdef PX_PROFILE
	CM_PROFILE_STOP_CROSSTHREAD(mEventProfiler, Cm::ProfileEventId::BroadPhase::GetSapPostUpdateSPU());
#ifdef PX_PS3
	stopTimerMarker(ePOSTUPDATE_SAP_SPU);
#endif
#endif
}

#endif  //PS_PS3

void PxsBroadPhaseContextSap::postUpdatePPU(PxBaseTask* /*continuation*/)
{
#ifdef PX_PROFILE
	Cm::EventProfiler* profiler = &mEventProfiler;
	CM_PROFILE_START(profiler, Cm::ProfileEventId::BroadPhase::GetSapPostUpdate());
#ifdef PX_PS3
	startTimerMarker(ePOSTUPDATE_SAP_PPU);
#endif
#endif

	for(PxU32 i=0;i<3;i++)
	{
		const PxU32 numPairs=mBatchUpdateTasks[i].getPairsSize();
		const PxcBroadPhasePair* PX_RESTRICT pairs=mBatchUpdateTasks[i].getPairs();
		for(PxU32 j=0;j<numPairs;j++)
		{
			const PxcBroadPhasePair& pair=pairs[j];
			const PxcBpHandle volA=pair.mVolA;
			const PxcBpHandle volB=pair.mVolB;
			if(volA > volB)
			{
				AddPair(volA, volB, mPairs, mData, mDataSize, mDataCapacity);
			}
			else
			{
				RemovePair(volA, volB, mPairs, mData, mDataSize, mDataCapacity);
			}
		}
	}

	batchCreate();

	//Compute the lists of created and deleted overlap pairs.

	ComputeCreatedDeletedPairsLists(
		mBoxGroups,
		mData,mDataSize,
		mCreatedPairsArray,mCreatedPairsSize,mCreatedPairsCapacity,
		mDeletedPairsArray,mDeletedPairsSize,mDeletedPairsCapacity,
		mPairs);

	PX_ASSERT(isSelfConsistent());
	mBoxesSizePrev=mBoxesSize;

#ifdef PX_PROFILE
	CM_PROFILE_STOP(profiler, Cm::ProfileEventId::BroadPhase::GetSapPostUpdate());
#ifdef PX_PS3
	stopTimerMarker(ePOSTUPDATE_SAP_PPU);
#endif
#endif
}

void BroadPhaseBatchUpdateWorkTask::runInternal()
{
	mPairsSize=0;
	mSap->batchUpdate(mAxis, mPairs, mPairsSize, mPairsCapacity);
}

void PxsBroadPhaseContextSap::updatePPU(PxBaseTask* continuation)
{
	PX_UNUSED(continuation);

#ifdef PX_PROFILE
	Cm::EventProfiler* profiler = &mEventProfiler;
	CM_PROFILE_START(profiler, Cm::ProfileEventId::BroadPhase::GetSapUpdate());
#ifdef PX_PS3
	startTimerMarker(eUPDATE_SAP_PPU);
#endif
#endif

	batchRemove();

	//Check that the overlap pairs per axis have been reset.
	PX_ASSERT(0==mBatchUpdateTasks[0].getPairsSize());
	PX_ASSERT(0==mBatchUpdateTasks[1].getPairsSize());
	PX_ASSERT(0==mBatchUpdateTasks[2].getPairsSize());

	mBatchUpdateTasks[0].runInternal();
	mBatchUpdateTasks[1].runInternal();
	mBatchUpdateTasks[2].runInternal();

#ifdef PX_PROFILE
	CM_PROFILE_STOP(profiler, Cm::ProfileEventId::BroadPhase::GetSapUpdate());
#ifdef PX_PS3
	stopTimerMarker(eUPDATE_SAP_PPU);
#endif
#endif
}

void PxsBroadPhaseContextSap::batchCreate()
{
	if(!mCreatedSize)	return;	// Early-exit if no object has been created

	//Number of newly-created boxes (still to be sorted) and number of old boxes (already sorted).
	const PxU32 numNewBoxes=mCreatedSize;
	//const PxU32 numOldBoxes = mBoxesSize - mCreatedSize;

	//Array of newly-created box indices.
	const PxcBpHandle* PX_RESTRICT created = mCreated;

	//Arrays of min and max coords for each box for each axis.
	const IntegerAABB* PX_RESTRICT minMax = mBoxBoundsMinMax;

	//Insert new boxes into sorted endpoints lists.
	{
		const PxU32 numEndPoints = numNewBoxes*2;

		Cm::TmpMem<PxcBPValType, 32> nepsv(numEndPoints), bv(numEndPoints);
		Cm::TmpMem<PxcBpHandle, 32> nepsd(numEndPoints), bd(numEndPoints);

		PxcBPValType* newEPSortedValues = nepsv.getBase();
		PxcBpHandle* newEPSortedDatas = nepsd.getBase();
		PxcBPValType* bufferValues = bv.getBase();
		PxcBpHandle* bufferDatas = bd.getBase();

		Gu::RadixSortBuffered RS;

		for(PxU32 Axis=0;Axis<3;Axis++)
		{
			for(PxU32 i=0;i<numNewBoxes;i++)
			{
				const PxU32 boxIndex = (PxU32)created[i];
				PX_ASSERT(mBoxEndPts[Axis][boxIndex].mMinMax[0]==PX_INVALID_BP_HANDLE || mBoxEndPts[Axis][boxIndex].mMinMax[0]==PX_REMOVED_BP_HANDLE);
				PX_ASSERT(mBoxEndPts[Axis][boxIndex].mMinMax[1]==PX_INVALID_BP_HANDLE || mBoxEndPts[Axis][boxIndex].mMinMax[1]==PX_REMOVED_BP_HANDLE);

				const PxcBPValType minValue = minMax[boxIndex].getMin(Axis);
				const PxcBPValType maxValue = minMax[boxIndex].getMax(Axis);
				
				newEPSortedValues[i*2+0]=minValue;
				setData(newEPSortedDatas[i*2+0],boxIndex, false);
				newEPSortedValues[i*2+1]=maxValue;
				setData(newEPSortedDatas[i*2+1], boxIndex, true);
			}

			// Sort endpoints backwards
			{
				PxU32* keys = (PxU32*)bufferValues;
				for(PxU32 i=0;i<numEndPoints;i++)
				{
					keys[i] = newEPSortedValues[i];
				}

				const PxU32* Sorted = RS.Sort(keys, numEndPoints, Gu::RADIX_UNSIGNED).GetRanks();

				for(PxU32 i=0;i<numEndPoints;i++)
				{
					bufferValues[i] = newEPSortedValues[Sorted[numEndPoints-1-i]];
					bufferDatas[i] = newEPSortedDatas[Sorted[numEndPoints-1-i]];
				}
			}

			InsertEndPoints(bufferValues, bufferDatas, numEndPoints, mEndPointValues[Axis], mEndPointDatas[Axis], 2*(mBoxesSize-mCreatedSize)+NUM_SENTINELS, mBoxEndPts[Axis]);
		}
	}

	//Some debug tests.
#ifdef PX_DEBUG
	{
		for(PxU32 i=0;i<numNewBoxes;i++)
		{
			PxU32 BoxIndex = (PxU32)created[i];
			PX_ASSERT(mBoxEndPts[0][BoxIndex].mMinMax[0]!=PX_INVALID_BP_HANDLE && mBoxEndPts[0][BoxIndex].mMinMax[0]!=PX_REMOVED_BP_HANDLE);
			PX_ASSERT(mBoxEndPts[0][BoxIndex].mMinMax[1]!=PX_INVALID_BP_HANDLE && mBoxEndPts[0][BoxIndex].mMinMax[1]!=PX_REMOVED_BP_HANDLE);
			PX_ASSERT(mBoxEndPts[1][BoxIndex].mMinMax[0]!=PX_INVALID_BP_HANDLE && mBoxEndPts[1][BoxIndex].mMinMax[0]!=PX_REMOVED_BP_HANDLE);
			PX_ASSERT(mBoxEndPts[1][BoxIndex].mMinMax[1]!=PX_INVALID_BP_HANDLE && mBoxEndPts[1][BoxIndex].mMinMax[1]!=PX_REMOVED_BP_HANDLE);
			PX_ASSERT(mBoxEndPts[2][BoxIndex].mMinMax[0]!=PX_INVALID_BP_HANDLE && mBoxEndPts[2][BoxIndex].mMinMax[0]!=PX_REMOVED_BP_HANDLE);
			PX_ASSERT(mBoxEndPts[2][BoxIndex].mMinMax[1]!=PX_INVALID_BP_HANDLE && mBoxEndPts[2][BoxIndex].mMinMax[1]!=PX_REMOVED_BP_HANDLE);
		}
		for(PxU32 i=0;i<mBoxesSize*2+1;i++)
		{
			PX_ASSERT(mEndPointValues[0][i] <= mEndPointValues[0][i+1]);
			PX_ASSERT(mEndPointValues[1][i] <= mEndPointValues[1][i+1]);
			PX_ASSERT(mEndPointValues[2][i] <= mEndPointValues[2][i+1]);
		}
	}
#endif

	//Axes used to compute overlaps involving newly-created boxes.
	const Gu::Axes axes(Gu::AXES_XYZ);
	performBoxPruning(axes);
}

void PxsBroadPhaseContextSap::performBoxPruning(const Gu::Axes axes)
{
	const PxU32 axis0=axes.mAxis0;

	//Number of newly-created boxes (still to be sorted) and number of old boxes (already sorted).
	const PxU32 numNewBoxes=mCreatedSize;
	const PxU32 numOldBoxes = mBoxesSize - mCreatedSize;

	//Gather two list of sorted boxes along the preferred axis direction: 
	//one list for new boxes and one list for existing boxes.
	//Only gather the existing boxes that overlap the bounding box of 
	//all new boxes.
	Cm::TmpMem<PxcBpHandle, 8> oldBoxesIndicesSortedMem(numOldBoxes);
	Cm::TmpMem<PxcBpHandle, 8> newBoxesIndicesSortedMem(numNewBoxes);
	PxcBpHandle* oldBoxesIndicesSorted=oldBoxesIndicesSortedMem.getBase();
	PxcBpHandle* newBoxesIndicesSorted=newBoxesIndicesSortedMem.getBase();
	PxU32 oldBoxCount=0;
	PxU32 newBoxCount=0;

	//To help us gather the two lists of sorted boxes we are going to use 
	//a bitmap and our knowledge of the indices of the new boxes
	const PxU32 bitmapWordCount = ((mBoxesCapacity*2 + 31) & ~31)/32;
	Cm::TmpMem<PxU32, 8> bitMapMem(bitmapWordCount);
	PxU32* bitMapWords=bitMapMem.getBase();
	PxMemSet(bitMapWords, 0, sizeof(PxU32)*bitmapWordCount);
	Cm::BitMap bitmap;
	bitmap.setWords(bitMapWords, bitmapWordCount);

	//Ready to gather the two lists now.
	bool allNewBoxesStatics=false;
	bool allOldBoxesStatics=false;
	ComputeSortedLists
		(&bitmap,
		 0,mCreatedSize,mCreated,
		 mBoxEndPts, mBoxGroups, 
		 mEndPointValues[axis0],mEndPointDatas[axis0],mBoxesSize*2 + NUM_SENTINELS,
		 axes,
		 newBoxesIndicesSorted,newBoxCount,oldBoxesIndicesSorted,oldBoxCount,allNewBoxesStatics,allOldBoxesStatics);


	//Intersect new boxes with new boxes and new boxes with existing boxes.
	if(!allNewBoxesStatics || !allOldBoxesStatics)
	{
		Cm::TmpMem<PxcBpHandle, 8> minPosListNewMem(numNewBoxes+1);
		PxcBpHandle* minPosListNew=minPosListNewMem.getBase();
		performBoxPruningNewNew
			(axes,
			 newBoxesIndicesSorted,newBoxCount,allNewBoxesStatics,
			 minPosListNew,mBoxEndPts,mBoxGroups,
			 mPairs,mData,mDataSize,mDataCapacity);

		// the old boxes are not the first ones in the array
		if(numOldBoxes)
		{
			Cm::TmpMem<PxcBpHandle, 8> minPosListOldMem(numOldBoxes);
			PxcBpHandle* minPosListOld=minPosListOldMem.getBase();
			performBoxPruningNewOld
				(axes,
				 newBoxesIndicesSorted,newBoxCount,oldBoxesIndicesSorted,oldBoxCount,
				 minPosListNew,minPosListOld,
				 mBoxEndPts,mBoxGroups,
				 mPairs,mData,mDataSize,mDataCapacity);
		}
	}
}

void PxsBroadPhaseContextSap::batchRemove()
{
	if(!mRemovedSize)	return;	// Early-exit if no object has been removed

	//The box count is incremented when boxes are added to the create list but these boxes
	//haven't yet been added to the pair manager or the sorted axis lists.  We need to 
	//pretend that the box count is the value it was when the bp was last updated.
	//Then, at the end, we need to set the box count to the number that includes the boxes
	//in the create list and subtract off the boxes that have been removed.
	PxU32 currBoxesSize=mBoxesSize;
	mBoxesSize=mBoxesSizePrev;

	for(PxU32 Axis=0;Axis<3;Axis++)
	{
		PxcBPValType* const BaseEPValue = mEndPointValues[Axis];
		PxcBpHandle* const BaseEPData = mEndPointDatas[Axis];
		PxU32 MinMinIndex = PX_MAX_U32;
		for(PxU32 i=0;i<mRemovedSize;i++)
		{
			PX_ASSERT(mRemoved[i]<mBoxesCapacity);

			const PxU32 MinIndex = mBoxEndPts[Axis][mRemoved[i]].mMinMax[0];
			PX_ASSERT(MinIndex<mBoxesCapacity*2+2);
			PX_ASSERT(getOwner(BaseEPData[MinIndex])==mRemoved[i]);

			const PxU32 MaxIndex = mBoxEndPts[Axis][mRemoved[i]].mMinMax[1];
			PX_ASSERT(MaxIndex<mBoxesCapacity*2+2);
			PX_ASSERT(getOwner(BaseEPData[MaxIndex])==mRemoved[i]);

			PX_ASSERT(MinIndex<MaxIndex);

			BaseEPData[MinIndex] = PX_REMOVED_BP_HANDLE;
			BaseEPData[MaxIndex] = PX_REMOVED_BP_HANDLE;

			if(MinIndex<MinMinIndex)	
				MinMinIndex = MinIndex;
		}


		PxU32 ReadIndex = MinMinIndex;
		PxU32 DestIndex = MinMinIndex;
		const PxU32 Limit = mBoxesSize*2+NUM_SENTINELS;
		while(ReadIndex!=Limit)
		{
			Ps::prefetchLine(&BaseEPData[ReadIndex],128);
			while(ReadIndex!=Limit && BaseEPData[ReadIndex] == PX_REMOVED_BP_HANDLE)
			{
				Ps::prefetchLine(&BaseEPData[ReadIndex],128);
				ReadIndex++;
			}
			if(ReadIndex!=Limit)
			{
				if(ReadIndex!=DestIndex)
				{
					BaseEPValue[DestIndex] = BaseEPValue[ReadIndex];
					BaseEPData[DestIndex] = BaseEPData[ReadIndex];
					PX_ASSERT(BaseEPData[DestIndex] != PX_REMOVED_BP_HANDLE);
					if(!isSentinel(BaseEPData[DestIndex]))
					{
						PxcBpHandle BoxOwner = getOwner(BaseEPData[DestIndex]);
						PX_ASSERT(BoxOwner<mBoxesCapacity);
						mBoxEndPts[Axis][BoxOwner].mMinMax[isMax(BaseEPData[DestIndex])] = (PxcBpHandle)DestIndex;
					}
				}
				DestIndex++;
				ReadIndex++;
			}
		}
	}

	for(PxU32 i=0;i<mRemovedSize;i++)
	{
		const PxU32 handle=mRemoved[i];
		mBoxEndPts[0][handle].mMinMax[0]=PX_REMOVED_BP_HANDLE;
		mBoxEndPts[0][handle].mMinMax[1]=PX_REMOVED_BP_HANDLE;
		mBoxEndPts[1][handle].mMinMax[0]=PX_REMOVED_BP_HANDLE;
		mBoxEndPts[1][handle].mMinMax[1]=PX_REMOVED_BP_HANDLE;
		mBoxEndPts[2][handle].mMinMax[0]=PX_REMOVED_BP_HANDLE;
		mBoxEndPts[2][handle].mMinMax[1]=PX_REMOVED_BP_HANDLE;
	}

	const PxU32 bitmapWordCount=1+(mBoxesCapacity>>5);
	Cm::TmpMem<PxU32, 128> bitmapWords(bitmapWordCount);
	PxMemZero(bitmapWords.getBase(),sizeof(PxU32)*bitmapWordCount);
	Cm::BitMap bitmap;
	bitmap.setWords(bitmapWords.getBase(),bitmapWordCount);
	for(PxU32 i=0;i<mRemovedSize;i++)
	{
		PxU32 Index = mRemoved[i];
		PX_ASSERT(Index<mBoxesCapacity);
		PX_ASSERT(0==bitmap.test(Index));
		bitmap.set(Index);
	}
	mPairs.RemovePairs(bitmap);

	mBoxesSize=currBoxesSize;
	mBoxesSize-=mRemovedSize;
	mBoxesSizePrev=mBoxesSize-mCreatedSize;
}

PX_FORCE_INLINE bool intersect2D(	const SapBox1D*const* PX_RESTRICT c,
									const SapBox1D*const* PX_RESTRICT boxEndPts,
									PxU32 ownerId,
									const PxU32 axis1, const PxU32 axis2)
{
	const SapBox1D* PX_RESTRICT b1 = boxEndPts[axis1] + ownerId;
	const SapBox1D* PX_RESTRICT b2 = boxEndPts[axis2] + ownerId;

	return (b1->mMinMax[1] >= c[axis1]->mMinMax[0] && c[axis1]->mMinMax[1] >= b1->mMinMax[0] &&
			b2->mMinMax[1] >= c[axis2]->mMinMax[0] && c[axis2]->mMinMax[1] >= b2->mMinMax[0]);
}

static PxcBroadPhasePair* resizeBroadPhasePairArray(const PxU32 oldMaxNb, const PxU32 newMaxNb, PxcBroadPhasePair* elements)
{

	PX_ASSERT(newMaxNb > oldMaxNb);
	PX_ASSERT(newMaxNb > 0);
	PX_ASSERT(0==((newMaxNb*sizeof(PxcBroadPhasePair)) & 15)); 
	PxcBroadPhasePair* newElements = (PxcBroadPhasePair*)PX_ALLOC(sizeof(PxcBroadPhasePair)*newMaxNb, PX_DEBUG_EXP("PxcBroadPhasePair"));
	PX_ASSERT(0==((uintptr_t)newElements & 0x0f));
	PxMemCopy(newElements, elements, oldMaxNb*sizeof(PxcBroadPhasePair));
	PX_FREE(elements);
	return newElements;
}

#define PERFORM_COMPARISONS 1

#if !BP_UPDATE_BEFORE_SWAP
void PxsBroadPhaseContextSap::batchUpdate
(const PxU32 Axis, PxcBroadPhasePair*& pairs, PxU32& pairsSize, PxU32& pairsCapacity)
{
	PxU32 numPairs=0;
	PxU32 maxNumPairs=pairsCapacity;

	const IntegerAABB* PX_RESTRICT boxMinMax3D = mBoxBoundsMinMax;
	SapBox1D* boxMinMax2D[6]={mBoxEndPts[1],mBoxEndPts[2],mBoxEndPts[2],mBoxEndPts[0],mBoxEndPts[0],mBoxEndPts[1]};

#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE 
	const PxcBpHandle* PX_RESTRICT asapBoxGroupIds=mBoxGroups;
#endif

	//for(PxU32 Axis=0;Axis<3;Axis++)
	{
		SapBox1D* PX_RESTRICT asapBoxes=mBoxEndPts[Axis];

		const SapBox1D* PX_RESTRICT boxMinMax0=boxMinMax2D[2*Axis+0];
		const SapBox1D* PX_RESTRICT boxMinMax1=boxMinMax2D[2*Axis+1];


		PxcBPValType* asapEndPointValues=mEndPointValues[Axis];
		PxcBpHandle* asapEndPointDatas=mEndPointDatas[Axis];

		PxcBPValType* const BaseEPValues = asapEndPointValues;
		PxcBpHandle* const BaseEPDatas = asapEndPointDatas;			


		const PxU32 updatedSizeMin1 = mUpdatedSize - 1;
		for(PxU32 i=0;i<mUpdatedSize;i++)
		{
			//Get the handle of the curr updated aabb.
			const PxcBpHandle handle=mUpdated[i];
			
#if BP_SAP_USE_PREFETCH
			const PxU32 nextHandle = mUpdated[PxMin(i+1, updatedSizeMin1)];
			Ps::prefetchLine(&asapBoxes[nextHandle]);
			Ps::prefetchLine(&boxMinMax3D[nextHandle]);
			Ps::prefetchLine(&boxMinMax0[2*nextHandle]);
			Ps::prefetchLine(&boxMinMax1[2*nextHandle]);
#endif

			PX_ASSERT(handle!=PX_INVALID_BP_HANDLE);
			PX_ASSERT(asapBoxes[handle].mMinMax[0]!=PX_INVALID_BP_HANDLE);

			//Get the box1d of the curr aabb.
			const SapBox1D* Object=&asapBoxes[handle];
			
			PX_ASSERT(Object->mMinMax[0]!=PX_INVALID_BP_HANDLE);
			PX_ASSERT(Object->mMinMax[1]!=PX_INVALID_BP_HANDLE);

			//Get the bounds of the curr aabb.
			const PxcBPValType boxMin = boxMinMax3D[handle].getMin(Axis);
			const PxcBPValType boxMax = boxMinMax3D[handle].getMax(Axis);

			// Update min
			{
				PxcBPValType* CurrentMinValue = BaseEPValues + Object->mMinMax[0];
				PxcBpHandle* CurrentMinData = BaseEPDatas + Object->mMinMax[0];
				PX_ASSERT(!isMax(*CurrentMinData));

				const PxcBPValType Limit = boxMin;
				if(Limit < *CurrentMinValue)
				{
					*CurrentMinValue = Limit;

					// Min is moving left:
					PxcBPValType SavedValue = *CurrentMinValue;
					PxcBpHandle SavedData = *CurrentMinData;
					PxU32 EPIndex = (PxU32)((size_t(CurrentMinData) - size_t(BaseEPDatas)))/sizeof(PxcBpHandle);
					const PxU32 SavedIndex = EPIndex;

					CurrentMinValue--;
					CurrentMinData--;
					while((*CurrentMinValue) > Limit)
					{
#if BP_SAP_USE_PREFETCH
						Ps::prefetchLine(CurrentMinValue-2);
						Ps::prefetchLine(CurrentMinData-2);
						const PxcBpHandle nextOwner = getOwner(*(CurrentMinData-1));
						Ps::prefetchLine(&asapBoxes[nextOwner]);
						Ps::prefetchLine(&boxMinMax0[2*nextOwner]);
						Ps::prefetchLine(&boxMinMax1[2*nextOwner]);
#endif

						const PxcBpHandle ownerId=getOwner(*CurrentMinData);
						SapBox1D* id1 = asapBoxes + ownerId;
						const PxcBpHandle IsMax = isMax(*CurrentMinData);
						if(IsMax)
						{
							// Our min passed a max => start overlap
							if(
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								(asapBoxGroupIds[handle]!=asapBoxGroupIds[ownerId]) && 
#endif								
								Intersect1D_Max(boxMin, boxMax, id1, BaseEPValues) &&
								Intersect2D(boxMinMax0[handle].mMinMax[0], boxMinMax0[handle].mMinMax[1], boxMinMax1[handle].mMinMax[0], boxMinMax1[handle].mMinMax[1],
								            boxMinMax0[ownerId].mMinMax[0],boxMinMax0[ownerId].mMinMax[1],boxMinMax1[ownerId].mMinMax[0],boxMinMax1[ownerId].mMinMax[1]) &&
								Object!=id1)
							{
								if(numPairs==maxNumPairs)
								{
									const PxU32 newMaxNumPairs=maxNumPairs*2;
									pairs = (PxcBroadPhasePair*)resizeBroadPhasePairArray(maxNumPairs, newMaxNumPairs, pairs);
									maxNumPairs=newMaxNumPairs;
								}
								PX_ASSERT(numPairs<maxNumPairs);
								pairs[numPairs].mVolA=PxMax(handle, getOwner(*CurrentMinData));
								pairs[numPairs].mVolB=PxMin(handle, getOwner(*CurrentMinData));
								numPairs++;
								//AddPair(handle, getOwner(*CurrentMinData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}

						id1->mMinMax[IsMax] = EPIndex--;
						*(CurrentMinValue+1) = *CurrentMinValue;
						*(CurrentMinData+1) = *CurrentMinData;

						CurrentMinValue--;
						CurrentMinData--;
					}

					if(SavedIndex!=EPIndex)
					{
						asapBoxes[getOwner(SavedData)].mMinMax[isMax(SavedData)] = EPIndex;
						BaseEPValues[EPIndex] = SavedValue;
						BaseEPDatas[EPIndex] = SavedData;
					}
				}
				else if(Limit > (*CurrentMinValue))
				{
					*CurrentMinValue = Limit;

					// Min is moving right:
					PxcBPValType SavedValue = *CurrentMinValue;
					PxcBpHandle SavedData = *CurrentMinData;

					PxU32 EPIndex = (PxU32)((size_t(CurrentMinData) - size_t(BaseEPDatas)))/sizeof(PxcBpHandle);
					const PxU32 SavedIndex = EPIndex;

					//while((++CurrentMin)->mValue < Limit)
					CurrentMinValue++;
					CurrentMinData++;
					while(Limit > (*CurrentMinValue))
					{
#if BP_SAP_USE_PREFETCH
						Ps::prefetchLine(CurrentMinValue+2);
						Ps::prefetchLine(CurrentMinData+2);
						const PxcBpHandle nextOwner = getOwner(*(CurrentMinData+1));
						Ps::prefetchLine(&asapBoxes[nextOwner]);
						Ps::prefetchLine(&boxMinMax0[2*nextOwner]);
						Ps::prefetchLine(&boxMinMax1[2*nextOwner]);
#endif

						const PxcBpHandle ownerId=getOwner(*CurrentMinData);
						SapBox1D* id1 = asapBoxes + ownerId;
						const PxcBpHandle IsMax = isMax(*CurrentMinData);
						if(IsMax)
						{
							// Our min passed a max => stop overlap
							if( 
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								(asapBoxGroupIds[handle]!=asapBoxGroupIds[ownerId]) && 
#endif
#if BP_SAP_USE_OVERLAP_TEST_ON_REMOVES
								Intersect2D(boxMinMax0[handle].mMinMax[0], boxMinMax0[handle].mMinMax[1], boxMinMax1[handle].mMinMax[0], boxMinMax1[handle].mMinMax[1],
								            boxMinMax0[ownerId].mMinMax[0],boxMinMax0[ownerId].mMinMax[1],boxMinMax1[ownerId].mMinMax[0],boxMinMax1[ownerId].mMinMax[1]) &&
#endif
								Object!=id1)
							{
								if(numPairs==maxNumPairs)
								{
									const PxU32 newMaxNumPairs=maxNumPairs*2;
									pairs = (PxcBroadPhasePair*)resizeBroadPhasePairArray(maxNumPairs, newMaxNumPairs, pairs);
									maxNumPairs=newMaxNumPairs;
								}
								PX_ASSERT(numPairs<maxNumPairs);
								pairs[numPairs].mVolA=PxMin(handle, getOwner(*CurrentMinData));
								pairs[numPairs].mVolB=PxMax(handle, getOwner(*CurrentMinData));
								numPairs++;
								//RemovePair(handle, getOwner(*CurrentMinData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}

						id1->mMinMax[IsMax] = EPIndex++;
						*(CurrentMinValue-1) = *CurrentMinValue;
						*(CurrentMinData-1) = *CurrentMinData;

						CurrentMinValue++;
						CurrentMinData++;
					}

					if(SavedIndex!=EPIndex)
					{
						asapBoxes[getOwner(SavedData)].mMinMax[isMax(SavedData)] = EPIndex;
						BaseEPValues[EPIndex] = SavedValue;
						BaseEPDatas[EPIndex] = SavedData;
					}
				}
			}//Update min

			// Update max
			{
				PxcBPValType* CurrentMaxValue = BaseEPValues + Object->mMinMax[1];
				PxcBpHandle* CurrentMaxData = BaseEPDatas + Object->mMinMax[1];

				PX_ASSERT(isMax(*CurrentMaxData));

				const PxcBPValType Limit = boxMax;
				if(Limit > (*CurrentMaxValue))
				{
					*CurrentMaxValue = Limit;

					// Max is moving right:
					PxcBPValType SavedValue = *CurrentMaxValue;
					PxcBpHandle SavedData = *CurrentMaxData;
					PxU32 EPIndex = (PxU32)((size_t(CurrentMaxData) - size_t(BaseEPDatas)))/sizeof(PxcBpHandle);
					const PxU32 SavedIndex = EPIndex;

					CurrentMaxValue++;
					CurrentMaxData++;
					while((*CurrentMaxValue) < Limit)
					{
#if BP_SAP_USE_PREFETCH
						Ps::prefetchLine(CurrentMaxValue+2);
						Ps::prefetchLine(CurrentMaxData+2);
						const PxcBpHandle nextOwner = getOwner(*(CurrentMaxData+1));
						Ps::prefetchLine(&asapBoxes[nextOwner]);
						Ps::prefetchLine(&boxMinMax0[2*nextOwner]);
						Ps::prefetchLine(&boxMinMax1[2*nextOwner]);
#endif
						const PxcBpHandle ownerId=getOwner(*CurrentMaxData);
						SapBox1D* id1 = asapBoxes + ownerId;
						const PxcBpHandle IsMax = isMax(*CurrentMaxData);
						if(!IsMax)
						{
							// Our max passed a min => start overlap
							if(
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								(asapBoxGroupIds[handle]!=asapBoxGroupIds[ownerId]) && 
#endif
								Intersect1D_Min(boxMin, boxMax, id1, BaseEPValues) &&
								Intersect2D(boxMinMax0[handle].mMinMax[0], boxMinMax0[handle].mMinMax[1], boxMinMax1[handle].mMinMax[0], boxMinMax1[handle].mMinMax[1],
								            boxMinMax0[ownerId].mMinMax[0],boxMinMax0[ownerId].mMinMax[1],boxMinMax1[ownerId].mMinMax[0],boxMinMax1[ownerId].mMinMax[1]) &&
								Object!=id1)
							{
								if(numPairs==maxNumPairs)
								{
									const PxU32 newMaxNumPairs=maxNumPairs*2;
									pairs = (PxcBroadPhasePair*)resizeBroadPhasePairArray(maxNumPairs, newMaxNumPairs, pairs);
									maxNumPairs=newMaxNumPairs;
								}
								PX_ASSERT(numPairs<maxNumPairs);
								pairs[numPairs].mVolA=PxMax(handle, getOwner(*CurrentMaxData));
								pairs[numPairs].mVolB=PxMin(handle, getOwner(*CurrentMaxData));
								numPairs++;
								//AddPair(handle, getOwner(*CurrentMaxData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}

						id1->mMinMax[IsMax] = EPIndex++;
						*(CurrentMaxValue-1) = *CurrentMaxValue;
						*(CurrentMaxData-1) = *CurrentMaxData;

						CurrentMaxValue++;
						CurrentMaxData++;
					}

					if(SavedIndex!=EPIndex)
					{
						asapBoxes[getOwner(SavedData)].mMinMax[isMax(SavedData)] = EPIndex;
						BaseEPValues[EPIndex] = SavedValue;
						BaseEPDatas[EPIndex] = SavedData;
					}
				}
				else if(Limit < (*CurrentMaxValue))
				{
					*CurrentMaxValue = Limit;

					// Max is moving left:
					PxcBPValType SavedValue = *CurrentMaxValue;
					PxcBpHandle SavedData = *CurrentMaxData;
					PxU32 EPIndex = (PxU32)((size_t(CurrentMaxData) - size_t(BaseEPDatas)))/sizeof(PxcBpHandle);
					const PxU32 SavedIndex = EPIndex;

					//while((--CurrentMax)->mValue > Limit)
					CurrentMaxValue--;
					CurrentMaxData--;
					while(Limit < (*CurrentMaxValue))
					{
#if BP_SAP_USE_PREFETCH
						Ps::prefetchLine(CurrentMaxValue-2);
						Ps::prefetchLine(CurrentMaxData-2);
						const PxcBpHandle nextOwner = getOwner(*(CurrentMaxData-1));
						Ps::prefetchLine(&asapBoxes[nextOwner]);
						Ps::prefetchLine(&boxMinMax0[2*nextOwner]);
						Ps::prefetchLine(&boxMinMax1[2*nextOwner]);
#endif
						const PxcBpHandle ownerId=getOwner(*CurrentMaxData);
						SapBox1D* id1 = asapBoxes + ownerId;
						const PxcBpHandle IsMax = isMax(*CurrentMaxData);
						if(!IsMax)
						{
							// Our max passed a min => stop overlap
							if(
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								(asapBoxGroupIds[handle]!=asapBoxGroupIds[ownerId]) && 
#endif
#if BP_SAP_USE_OVERLAP_TEST_ON_REMOVES
								Intersect2D(boxMinMax0[handle].mMinMax[0], boxMinMax0[handle].mMinMax[1], boxMinMax1[handle].mMinMax[0], boxMinMax1[handle].mMinMax[1],
								            boxMinMax0[ownerId].mMinMax[0],boxMinMax0[ownerId].mMinMax[1],boxMinMax1[ownerId].mMinMax[0],boxMinMax1[ownerId].mMinMax[1]) &&
#endif
								Object!=id1)
							{
								if(numPairs==maxNumPairs)
								{
									const PxU32 newMaxNumPairs=maxNumPairs*2;
									pairs = (PxcBroadPhasePair*)resizeBroadPhasePairArray(maxNumPairs, newMaxNumPairs, pairs);
									maxNumPairs=newMaxNumPairs;
								}
								PX_ASSERT(numPairs<maxNumPairs);
								pairs[numPairs].mVolA=PxMin(handle, getOwner(*CurrentMaxData));
								pairs[numPairs].mVolB=PxMax(handle, getOwner(*CurrentMaxData));
								numPairs++;
								//RemovePair(handle, getOwner(*CurrentMaxData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}

						id1->mMinMax[IsMax] = EPIndex--;
						*(CurrentMaxValue+1) = *CurrentMaxValue;
						*(CurrentMaxData+1) = *CurrentMaxData;

						CurrentMaxValue--;
						CurrentMaxData--;
					}

					if(SavedIndex!=EPIndex)
					{
						asapBoxes[getOwner(SavedData)].mMinMax[isMax(SavedData)] = EPIndex;
						BaseEPValues[EPIndex] = SavedValue;
						BaseEPDatas[EPIndex] = SavedData;
					}
				}
			}// update max
		}// updated aabbs
	}

	pairsSize=numPairs;
	pairsCapacity=maxNumPairs;
}

#else

void PxsBroadPhaseContextSap::batchUpdate
(const PxU32 Axis, PxcBroadPhasePair*& pairs, PxU32& pairsSize, PxU32& pairsCapacity)
{
	//Nothin updated so don't do anything
	if(mUpdatedSize == 0)
		return;

		//If number updated is sufficiently fewer than number of boxes (say less than 20%)
	if((mUpdatedSize*5) < mBoxesSize)
	{
		batchUpdateFewUpdates(Axis, pairs, pairsSize, pairsCapacity);
		return;
	}

	PxU32 numPairs=0;
	PxU32 maxNumPairs=pairsCapacity;

	const IntegerAABB* PX_RESTRICT boxMinMax3D = mBoxBoundsMinMax;
	SapBox1D* boxMinMax2D[6]={mBoxEndPts[1],mBoxEndPts[2],mBoxEndPts[2],mBoxEndPts[0],mBoxEndPts[0],mBoxEndPts[1]};

	const SapBox1D* PX_RESTRICT boxMinMax0=boxMinMax2D[2*Axis+0];
	const SapBox1D* PX_RESTRICT boxMinMax1=boxMinMax2D[2*Axis+1];


#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE 
	const PxcBpHandle* PX_RESTRICT asapBoxGroupIds=mBoxGroups;
#endif

	SapBox1D* PX_RESTRICT asapBoxes=mBoxEndPts[Axis];

	PxcBPValType* PX_RESTRICT asapEndPointValues=mEndPointValues[Axis];
	PxcBpHandle* PX_RESTRICT asapEndPointDatas=mEndPointDatas[Axis];

	PxcBPValType* const PX_RESTRICT BaseEPValues = asapEndPointValues;
	PxcBpHandle* const PX_RESTRICT BaseEPDatas = asapEndPointDatas;			

	PxU8* PX_RESTRICT updated = mBoxesUpdated;

	//KS - can we lazy create these inside the loop? Might benefit us

	//There are no extents, jus the sentinels, so exit early.
	if(isSentinel(BaseEPDatas[1]))
		return;

	//We are going to skip the 1st element in the array (the sublist will be sorted)
	//but we must first update its value if it has moved
	//const PxU32 startIsMax = isMax(BaseEPDatas[1]);
	PX_ASSERT(!isMax(BaseEPDatas[1]));
	const PxcBpHandle startHandle = getOwner(BaseEPDatas[1]);

	//KS - in theory, we should just be able to grab the min element but there's some issue where a body's max < min (i.e. an invalid extents) that
	//appears in a unit test
	PxcBPValType ThisValue_ = boxMinMax3D[startHandle].getMin(Axis);
	BaseEPValues[1] = ThisValue_;
	
	PxU32 updateCounter = mUpdatedSize*2;

	updateCounter -= updated[startHandle];

	//We'll never overlap with this sentinel but it just ensures that we don't need to branch to see if
	//there's a pocket that we need to test against
	
	PxsBroadPhaseActivityPocket* PX_RESTRICT currentPocket = mActivityPockets;

	currentPocket->mEndIndex = 0;
	currentPocket->mStartIndex = 0;


	PxcBpHandle ind = 2;
	PxU8 wasUpdated = updated[startHandle];
	for(; !isSentinel(BaseEPDatas[ind]); ++ind)
	{
		PxcBpHandle ThisData = BaseEPDatas[ind];

		const PxcBpHandle handle = getOwner(ThisData);

		if(updated[handle] || wasUpdated)
		{
			wasUpdated = updated[handle];
			updateCounter -= wasUpdated;

			PxcBpHandle ThisIndex = ind;

			const PxcBpHandle startIsMax = isMax(ThisData);


			//Access and write back the updated values. TODO - can we avoid this when we're walking through inactive nodes?
			//BPValType ThisValue = boxMinMax1D[Axis][twoHandle+startIsMax];
			//BPValType ThisValue = startIsMax ? boxMinMax3D[handle].getMax(Axis) : boxMinMax3D[handle].getMin(Axis);
			PxcBPValType ThisValue = boxMinMax3D[handle].getExtent(startIsMax, Axis);
			BaseEPValues[ThisIndex] = ThisValue;

			PX_ASSERT(handle!=PX_INVALID_BP_HANDLE);

			//We always iterate back through the list...

			PxcBpHandle CurrentIndex = mListPrev[ThisIndex];
			PxcBPValType CurrentValue = BaseEPValues[CurrentIndex];
			//PxBpHandle CurrentData = BaseEPDatas[CurrentIndex];

			if(CurrentValue > ThisValue)
			{
				wasUpdated = 1;
				//Get the bounds of the curr aabb.
				//Get the box1d of the curr aabb.
				/*const SapBox1D* PX_RESTRICT Object=&asapBoxes[handle];
				PX_ASSERT(Object->mMinMax[0]!=PX_INVALID_BP_HANDLE);
				PX_ASSERT(Object->mMinMax[1]!=PX_INVALID_BP_HANDLE);*/
				
				const PxcBPValType boxMax=boxMinMax3D[handle].getMax(Axis);

				PxU32 endIndex = ind;
				PxU32 startIndex = ind;

#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
				PxcBPValType group = asapBoxGroupIds[handle];
#endif

				if(!isMax(ThisData))
				{
					do
					{
						PxcBpHandle CurrentData = BaseEPDatas[CurrentIndex];
						const PxcBpHandle IsMax = isMax(CurrentData);
						
	#if PERFORM_COMPARISONS
						if(IsMax)
						{		
							const PxcBpHandle ownerId=getOwner(CurrentData);
							SapBox1D* PX_RESTRICT id1 = asapBoxes + ownerId;
							// Our min passed a max => start overlap

							if(
								BaseEPValues[id1->mMinMax[0]] < boxMax && 
								//2D intersection test using up-to-date values
								Intersect2D_Handle(boxMinMax0[handle].mMinMax[0], boxMinMax0[handle].mMinMax[1], boxMinMax1[handle].mMinMax[0], boxMinMax1[handle].mMinMax[1],
								            boxMinMax0[ownerId].mMinMax[0],boxMinMax0[ownerId].mMinMax[1],boxMinMax1[ownerId].mMinMax[0],boxMinMax1[ownerId].mMinMax[1])

	#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								&& (group!=asapBoxGroupIds[ownerId])
	#else
								&& handle!=ownerId
	#endif
								)
							{
								if(numPairs==maxNumPairs)
								{
									const PxU32 newMaxNumPairs=maxNumPairs*2;
									pairs = (PxcBroadPhasePair*)resizeBroadPhasePairArray(maxNumPairs, newMaxNumPairs, pairs);
									maxNumPairs=newMaxNumPairs;
								}
								PX_ASSERT(numPairs<maxNumPairs);
								pairs[numPairs].mVolA=(PxcBpHandle)PxMax(handle, ownerId);
								pairs[numPairs].mVolB=(PxcBpHandle)PxMin(handle, ownerId);
								numPairs++;
								//AddPair(handle, getOwner(*CurrentMinData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}
	#endif
						startIndex--;
						CurrentIndex = mListPrev[CurrentIndex];
						CurrentValue = BaseEPValues[CurrentIndex];
					}
					while(ThisValue < CurrentValue);
				}			
				else 
				{
					// Max is moving left:
					do
					{
						PxcBpHandle CurrentData = BaseEPDatas[CurrentIndex];
						const PxcBpHandle IsMax = isMax(CurrentData);
						
	#if PERFORM_COMPARISONS
						if(!IsMax)
						{
							// Our max passed a min => stop overlap
							const PxcBpHandle ownerId=getOwner(CurrentData);

#if 1
							if(
#if BP_SAP_USE_OVERLAP_TEST_ON_REMOVES
								Intersect2D_Handle(boxMinMax0[handle].mMinMax[0], boxMinMax0[handle].mMinMax[1], boxMinMax1[handle].mMinMax[0], boxMinMax1[handle].mMinMax[1],
								       boxMinMax0[ownerId].mMinMax[0],boxMinMax0[ownerId].mMinMax[1],boxMinMax1[ownerId].mMinMax[0],boxMinMax1[ownerId].mMinMax[1])
#endif
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								&& (group!=asapBoxGroupIds[ownerId])
#else
								&& handle!=ownerId
#endif
								)
#endif
							{
								if(numPairs==maxNumPairs)
								{
									const PxU32 newMaxNumPairs=maxNumPairs*2;
									pairs = (PxcBroadPhasePair*)resizeBroadPhasePairArray(maxNumPairs, newMaxNumPairs, pairs);
									maxNumPairs=newMaxNumPairs;
								}
								PX_ASSERT(numPairs<maxNumPairs);
								pairs[numPairs].mVolA=(PxcBpHandle)PxMin(handle, ownerId);
								pairs[numPairs].mVolB=(PxcBpHandle)PxMax(handle, ownerId);
								numPairs++;
								//RemovePair(handle, getOwner(*CurrentMaxData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}
	#endif
						startIndex--;
						CurrentIndex = mListPrev[CurrentIndex];
						CurrentValue = BaseEPValues[CurrentIndex];
					}
					while(ThisValue < CurrentValue);
				}

				//This test is unnecessary. If we entered the outer loop, we're doing the swap in here
				{
					//Unlink from old position and re-link to new position
					PxcBpHandle oldNextIndex = mListNext[ThisIndex];
					PxcBpHandle oldPrevIndex = mListPrev[ThisIndex];

					PxcBpHandle newNextIndex = mListNext[CurrentIndex];
					PxcBpHandle newPrevIndex = CurrentIndex;
					
					//Unlink this node
					mListNext[oldPrevIndex] = oldNextIndex;
					mListPrev[oldNextIndex] = oldPrevIndex;

					//Link it to it's new place in the list
					mListNext[ThisIndex] = newNextIndex;
					mListPrev[ThisIndex] = newPrevIndex;
					mListPrev[newNextIndex] = ThisIndex;
					mListNext[newPrevIndex] = ThisIndex;
				}

				//There is a sentinel with 0 index, so we don't need
				//to worry about walking off the array				
				while(startIndex < currentPocket->mStartIndex)
				{
					currentPocket--;
				}
				//If our start index > currentPocket->mEndIndex, then we don't overlap so create a new pocket
				if(currentPocket == mActivityPockets || startIndex > (currentPocket->mEndIndex+1))
				{
					currentPocket++;
					currentPocket->mStartIndex = startIndex;
				}
				currentPocket->mEndIndex = endIndex;
			}// update max
			//ind++;
		}
		else if (updateCounter == 0) //We've updated all the bodies and neither this nor the previous body was updated, so we're done
			break;

	}// updated aabbs

	pairsSize=numPairs;
	pairsCapacity=maxNumPairs;


	PxsBroadPhaseActivityPocket* pocket = mActivityPockets+1;

	while(pocket <= currentPocket)
	{
		for(PxU32 a = pocket->mStartIndex; a <= pocket->mEndIndex; ++a)
		{
			mListPrev[a] = (PxcBpHandle)a;
		}

		//Now copy all the data to the array, updating the remap table

		PxU32 CurrIndex = pocket->mStartIndex-1;
		for(PxU32 a = pocket->mStartIndex; a <= pocket->mEndIndex; ++a)
		{
			CurrIndex = mListNext[CurrIndex];
			PxU32 origIndex =  CurrIndex;
			PxcBpHandle remappedIndex = mListPrev[origIndex];

			if(origIndex != a)
			{
				const PxcBpHandle ownerId=getOwner(BaseEPDatas[remappedIndex]);
				const PxcBpHandle IsMax = isMax(BaseEPDatas[remappedIndex]);
				PxcBPValType tmp = BaseEPValues[a];
				PxcBpHandle tmpHandle = BaseEPDatas[a];

				BaseEPValues[a] = BaseEPValues[remappedIndex];
				BaseEPDatas[a] = BaseEPDatas[remappedIndex];

				BaseEPValues[remappedIndex] = tmp;
				BaseEPDatas[remappedIndex] = tmpHandle;

				mListPrev[remappedIndex] = mListPrev[a];
				//Write back remap index (should be an immediate jump to original index)
				mListPrev[mListPrev[a]] = remappedIndex;
				asapBoxes[ownerId].mMinMax[IsMax] = (PxcBpHandle)a;
			}
			
		}

		////Reset next and prev ptrs back
		for(PxU32 a = pocket->mStartIndex-1; a <= pocket->mEndIndex; ++a)
		{
			mListPrev[a+1] = (PxcBpHandle)a;
			mListNext[a] = (PxcBpHandle)(a+1);
		}

		pocket++;
	}
	mListPrev[0] = 0;
}


void PxsBroadPhaseContextSap::batchUpdateFewUpdates
(const PxU32 Axis, PxcBroadPhasePair*& pairs, PxU32& pairsSize, PxU32& pairsCapacity)
{
	PxU32 numPairs=0;
	PxU32 maxNumPairs=pairsCapacity;

	const IntegerAABB* PX_RESTRICT boxMinMax3D = mBoxBoundsMinMax;
	SapBox1D* boxMinMax2D[6]={mBoxEndPts[1],mBoxEndPts[2],mBoxEndPts[2],mBoxEndPts[0],mBoxEndPts[0],mBoxEndPts[1]};

#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE 
	const PxcBpHandle* PX_RESTRICT asapBoxGroupIds=mBoxGroups;
#endif

	SapBox1D* PX_RESTRICT asapBoxes=mBoxEndPts[Axis];

	/*const BPValType* PX_RESTRICT boxMinMax0=boxMinMax2D[2*Axis];
	const BPValType* PX_RESTRICT boxMinMax1=boxMinMax2D[2*Axis+1];*/

	PxcBPValType* PX_RESTRICT asapEndPointValues=mEndPointValues[Axis];
	PxcBpHandle* PX_RESTRICT asapEndPointDatas=mEndPointDatas[Axis];

	PxcBPValType* const PX_RESTRICT BaseEPValues = asapEndPointValues;
	PxcBpHandle* const PX_RESTRICT BaseEPDatas = asapEndPointDatas;			

	const SapBox1D* PX_RESTRICT boxMinMax0=boxMinMax2D[2*Axis+0];
	const SapBox1D* PX_RESTRICT boxMinMax1=boxMinMax2D[2*Axis+1];

	PxU8* PX_RESTRICT updated = mBoxesUpdated;

	const PxU32 endPointSize = mBoxesSize*2 + 1;

	//There are no extents, just the sentinels, so exit early.
	if(isSentinel(BaseEPDatas[1]))
		return;

	PxU32 ind_ = 0;

	PxU32 index = 1;

	if(mUpdatedSize < 512)
	{
		//The array of updated elements is small, so use qsort to sort them
		for(PxU32 a = 0; a < mUpdatedSize; ++a)
		{
			const PxU32 handle=mUpdated[a];

			const SapBox1D* Object=&asapBoxes[handle];
				
			PX_ASSERT(Object->mMinMax[0]!=PX_INVALID_BP_HANDLE);
			PX_ASSERT(Object->mMinMax[1]!=PX_INVALID_BP_HANDLE);

			//Get the bounds of the curr aabb.

			const PxcBPValType boxMin=boxMinMax3D[handle].getMin(Axis);
			const PxcBPValType boxMax=boxMinMax3D[handle].getMax(Axis);

			BaseEPValues[Object->mMinMax[0]] = boxMin;
			BaseEPValues[Object->mMinMax[1]] = boxMax;

			mSortedUpdateElements[ind_++] = Object->mMinMax[0];
			mSortedUpdateElements[ind_++] = Object->mMinMax[1];
		}
		Ps::sort(mSortedUpdateElements, ind_);
	}
	else
	{
		//The array of updated elements is large so use a bucket sort to sort them
		for(; index < endPointSize; ++index)
		{
			if(isSentinel( BaseEPDatas[index] ))
				break;
			PxcBpHandle ThisData = BaseEPDatas[index];
			PxcBpHandle owner = (PxcBpHandle)getOwner(ThisData);
			if(updated[owner])
			{
				//BPValType ThisValue = isMax(ThisData) ? boxMinMax3D[owner].getMax(Axis) : boxMinMax3D[owner].getMin(Axis);
				PxcBPValType ThisValue = boxMinMax3D[owner].getExtent(isMax(ThisData), Axis);
				BaseEPValues[index] = ThisValue;
				mSortedUpdateElements[ind_++] = (PxcBpHandle)index;
			}
		}
	}

	const PxU32 updateCounter = ind_;
	
	//We'll never overlap with this sentinel but it just ensures that we don't need to branch to see if
	//there's a pocket that we need to test against
	PxsBroadPhaseActivityPocket* PX_RESTRICT currentPocket = mActivityPockets;
	currentPocket->mEndIndex = 0;
	currentPocket->mStartIndex = 0;

	for(PxU32 a = 0; a < updateCounter; ++a)
	{
		PxcBpHandle ind = mSortedUpdateElements[a];

		PxcBpHandle NextData;
		PxcBpHandle PrevData;
		do
		{
			PxcBpHandle ThisData = BaseEPDatas[ind];

			const PxcBpHandle handle = getOwner(ThisData);

			PxcBpHandle ThisIndex = ind;
			PxcBPValType ThisValue = BaseEPValues[ThisIndex];

			//Get the box1d of the curr aabb.
			const SapBox1D* PX_RESTRICT Object=&asapBoxes[handle];

			PX_ASSERT(handle!=PX_INVALID_BP_HANDLE);

			PX_ASSERT(Object->mMinMax[0]!=PX_INVALID_BP_HANDLE);
			PX_ASSERT(Object->mMinMax[1]!=PX_INVALID_BP_HANDLE);
			PX_UNUSED(Object);

			//Get the bounds of the curr aabb.
			//const PxU32 twoHandle = 2*handle;
			
			const PxcBPValType boxMax=boxMinMax3D[handle].getMax(Axis);

			//We always iterate back through the list...
			PxcBpHandle CurrentIndex = mListPrev[ThisIndex];
			PxcBPValType CurrentValue = BaseEPValues[CurrentIndex];

			if(CurrentValue > ThisValue)
			{
				//We're performing some swaps so we need an activity pocket here. This structure allows us to keep track of the range of 
				//modifications in the sorted lists. Doesn't help when everything's moving but makes a really big difference to reconstituting the 
				//list when only a small number of things are moving

				PxU32 endIndex = ind;
				PxU32 startIndex = ind;

				//const BPValType* PX_RESTRICT box0MinMax0 = &boxMinMax0[twoHandle];
				//const BPValType* PX_RESTRICT box0MinMax1 = &boxMinMax1[twoHandle];
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
				PxcBPValType group = asapBoxGroupIds[handle];
#endif
				if(!isMax(ThisData))
				{
					do
					{
						PxcBpHandle CurrentData = BaseEPDatas[CurrentIndex];
						const PxcBpHandle IsMax = isMax(CurrentData);
						
	#if PERFORM_COMPARISONS
						if(IsMax)
						{		
							const PxcBpHandle ownerId=getOwner(CurrentData);
							SapBox1D* PX_RESTRICT id1 = asapBoxes + ownerId;
							// Our min passed a max => start overlap

							if(
								BaseEPValues[id1->mMinMax[0]] < boxMax && 
								//2D intersection test using up-to-date values
								Intersect2D_Handle(boxMinMax0[handle].mMinMax[0], boxMinMax0[handle].mMinMax[1], boxMinMax1[handle].mMinMax[0], boxMinMax1[handle].mMinMax[1],
								       boxMinMax0[ownerId].mMinMax[0],boxMinMax0[ownerId].mMinMax[1],boxMinMax1[ownerId].mMinMax[0],boxMinMax1[ownerId].mMinMax[1])
	#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								&& (group!=asapBoxGroupIds[ownerId])
	#else
								&& Object!=id1
	#endif
								)
							{
								if(numPairs==maxNumPairs)
								{
									const PxU32 newMaxNumPairs=maxNumPairs*2;
									pairs = (PxcBroadPhasePair*)resizeBroadPhasePairArray(maxNumPairs, newMaxNumPairs, pairs);
									maxNumPairs=newMaxNumPairs;
								}
								PX_ASSERT(numPairs<maxNumPairs);
								pairs[numPairs].mVolA=(PxcBpHandle)PxMax(handle, ownerId);
								pairs[numPairs].mVolB=(PxcBpHandle)PxMin(handle, ownerId);
								numPairs++;
								//AddPair(handle, getOwner(*CurrentMinData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}
	#endif
						startIndex--;
						CurrentIndex = mListPrev[CurrentIndex];
						CurrentValue = BaseEPValues[CurrentIndex];
					}
					while(ThisValue < CurrentValue);
				}			
				else 
				{
					// Max is moving left:
					do
					{
						PxcBpHandle CurrentData = BaseEPDatas[CurrentIndex];
						const PxcBpHandle IsMax = isMax(CurrentData);
						
	#if PERFORM_COMPARISONS
						if(!IsMax)
						{
							// Our max passed a min => stop overlap
							const PxcBpHandle ownerId=getOwner(CurrentData);

#if 1
							if(
#if BP_SAP_USE_OVERLAP_TEST_ON_REMOVES
								Intersect2D_Handle(boxMinMax0[handle].mMinMax[0], boxMinMax0[handle].mMinMax[1], boxMinMax1[handle].mMinMax[0], boxMinMax1[handle].mMinMax[1],
								       boxMinMax0[ownerId].mMinMax[0],boxMinMax0[ownerId].mMinMax[1],boxMinMax1[ownerId].mMinMax[0],boxMinMax1[ownerId].mMinMax[1])
#endif
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								&& (group!=asapBoxGroupIds[ownerId])
#else
								&& Object!=id1
#endif
								)
#endif
							{
								if(numPairs==maxNumPairs)
								{
									const PxU32 newMaxNumPairs=maxNumPairs*2;
									pairs = (PxcBroadPhasePair*)resizeBroadPhasePairArray(maxNumPairs, newMaxNumPairs, pairs);
									maxNumPairs=newMaxNumPairs;
								}
								PX_ASSERT(numPairs<maxNumPairs);
								pairs[numPairs].mVolA=(PxcBpHandle)PxMin(handle, ownerId);
								pairs[numPairs].mVolB=(PxcBpHandle)PxMax(handle, ownerId);
								numPairs++;
								//RemovePair(handle, getOwner(*CurrentMaxData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}
	#endif
						startIndex--;
						CurrentIndex = mListPrev[CurrentIndex];
						CurrentValue = BaseEPValues[CurrentIndex];
					}
					while(ThisValue < CurrentValue);
				}

				//This test is unnecessary. If we entered the outer loop, we're doing the swap in here
				{
					//Unlink from old position and re-link to new position
					PxcBpHandle oldNextIndex = mListNext[ThisIndex];
					PxcBpHandle oldPrevIndex = mListPrev[ThisIndex];

					PxcBpHandle newNextIndex = mListNext[CurrentIndex];
					PxcBpHandle newPrevIndex = CurrentIndex;
					
					//Unlink this node
					mListNext[oldPrevIndex] = oldNextIndex;
					mListPrev[oldNextIndex] = oldPrevIndex;

					//Link it to it's new place in the list
					mListNext[ThisIndex] = newNextIndex;
					mListPrev[ThisIndex] = newPrevIndex;
					mListPrev[newNextIndex] = ThisIndex;
					mListNext[newPrevIndex] = ThisIndex;
				}

				//Loop over the activity pocket stack to make sure this set of shuffles didn't 
				//interfere with the previous set. If it did, we roll this pocket into the previous
				//pockets. If everything in the scene is moving, we should result in just 1 pocket
				while(startIndex < currentPocket->mStartIndex)
				{
					currentPocket--;
				}
				//If our start index > currentPocket->mEndIndex, then we don't overlap so create a new pocket
				if(currentPocket == mActivityPockets || startIndex > (currentPocket->mEndIndex+1))
				{
					currentPocket++;
					currentPocket->mStartIndex = startIndex;
				}
				currentPocket->mEndIndex = endIndex;
			}// update max
			//Get prev and next ptr...

			NextData = BaseEPDatas[++ind];
			PrevData = BaseEPDatas[mListPrev[ind]];

		}while(!isSentinel(NextData) && !updated[getOwner(NextData)] && updated[getOwner(PrevData)]);
		
	}// updated aabbs

	pairsSize=numPairs;
	pairsCapacity=maxNumPairs;


	PxsBroadPhaseActivityPocket* pocket = mActivityPockets+1;

	while(pocket <= currentPocket)
	{
		//PxU32 CurrIndex = mListPrev[pocket->mStartIndex];
		for(PxU32 a = pocket->mStartIndex; a <= pocket->mEndIndex; ++a)
		{
			mListPrev[a] = (PxcBpHandle)a;
		}

		//Now copy all the data to the array, updating the remap table
		PxU32 CurrIndex = pocket->mStartIndex-1;
		for(PxU32 a = pocket->mStartIndex; a <= pocket->mEndIndex; ++a)
		{
			CurrIndex = mListNext[CurrIndex];
			PxU32 origIndex =  CurrIndex;
			PxcBpHandle remappedIndex = mListPrev[origIndex];

			if(origIndex != a)
			{
				const PxcBpHandle ownerId=getOwner(BaseEPDatas[remappedIndex]);
				const PxcBpHandle IsMax = isMax(BaseEPDatas[remappedIndex]);
				PxcBPValType tmp = BaseEPValues[a];
				PxcBpHandle tmpHandle = BaseEPDatas[a];

				BaseEPValues[a] = BaseEPValues[remappedIndex];
				BaseEPDatas[a] = BaseEPDatas[remappedIndex];

				BaseEPValues[remappedIndex] = tmp;
				BaseEPDatas[remappedIndex] = tmpHandle;

				mListPrev[remappedIndex] = mListPrev[a];
				//Write back remap index (should be an immediate jump to original index)
				mListPrev[mListPrev[a]] = remappedIndex;
				asapBoxes[ownerId].mMinMax[IsMax] = (PxcBpHandle)a;
			}
			
		}

		for(PxU32 a = pocket->mStartIndex-1; a <= pocket->mEndIndex; ++a)
		{
			mListPrev[a+1] = (PxcBpHandle)a;
			mListNext[a] = (PxcBpHandle)(a+1);
		}
		pocket++;
	}
}


#endif



#ifdef PX_DEBUG

bool PxsBroadPhaseContextSap::isSelfOrdered() const 
{
	if(0==mBoxesSize)
	{
		return true;
	}

	for(PxU32 Axis=0;Axis<3;Axis++)
	{
		PxU32 it=1;
		PX_ASSERT(mEndPointDatas[Axis]);
		while(!isSentinel(mEndPointDatas[Axis][it]))
		{
			//Test the array is sorted.
			const PxcBPValType prevVal=mEndPointValues[Axis][it-1];
			const PxcBPValType currVal=mEndPointValues[Axis][it];
			if(currVal<prevVal)
			{
				return false;
			}

			//Test the end point array is consistent.
			const PxcBpHandle ismax=isMax(mEndPointDatas[Axis][it]);
			const PxcBpHandle ownerId=getOwner(mEndPointDatas[Axis][it]);
			if(mBoxEndPts[Axis][ownerId].mMinMax[ismax]!=it)
			{
				return false;
			}

			//Test the mins are even, the maxes are odd, and the extents are finite.
			const PxcBPValType boxMin = mEndPointValues[Axis][mBoxEndPts[Axis][ownerId].mMinMax[0]];
			const PxcBPValType boxMax = mEndPointValues[Axis][mBoxEndPts[Axis][ownerId].mMinMax[1]];
			if(boxMin & 1)
			{
				return false;
			}
			if(0==(boxMax & 1))
			{
				return false;
			}
			if(boxMax<=boxMin)
			{
				return false;
			}

			it++;
		}
	}

	return true;
}

bool PxsBroadPhaseContextSap::isSelfConsistent() const 
{
	if(0==mBoxesSize)
	{
		return true;
	}

	for(PxU32 Axis=0;Axis<3;Axis++)
	{
		PxU32 it=1;
		PxcBPValType prevVal=0;
		const IntegerAABB* PX_RESTRICT boxMinMax = mBoxBoundsMinMax;
		PX_ASSERT(mEndPointDatas[Axis]);
		while(!isSentinel(mEndPointDatas[Axis][it]))
		{
			const PxcBpHandle ownerId=getOwner(mEndPointDatas[Axis][it]);
			const PxcBpHandle ismax=isMax(mEndPointDatas[Axis][it]);
			const PxcBPValType boxMinMaxs[2] = {boxMinMax[ownerId].getMin(Axis), boxMinMax[ownerId].getMax(Axis)};
			const PxcBPValType test1=boxMinMaxs[ismax];
			const PxcBPValType test2=mEndPointValues[Axis][it];
			if(test1!=test2)
			{
				return false;
			}
			if(test2<prevVal)
			{
				return false;
			}
			prevVal=test2;

			if(mBoxEndPts[Axis][ownerId].mMinMax[ismax]!=it)
			{
				return false;
			}

			it++;
		}
	}

	for(PxU32 i=0;i<mCreatedPairsSize;i++)
	{
		const PxU32 a=mCreatedPairsArray[i].mVolA;
		const PxU32 b=mCreatedPairsArray[i].mVolB;
		IntegerAABB aabb0=mBoxBoundsMinMax[a];
		IntegerAABB aabb1=mBoxBoundsMinMax[b];
		if(!aabb0.intersects(aabb1))
		{
			return false;
		}
	}

	for(PxU32 i=0;i<mDeletedPairsSize;i++)
	{
		const PxU32 a=mDeletedPairsArray[i].mVolA;
		const PxU32 b=mDeletedPairsArray[i].mVolB;

		bool isDeleted=false;
		for(PxU32 j=0;j<mRemovedSize;j++)
		{
			if(a==mRemoved[j] || b==mRemoved[j])
			{
				isDeleted=true;
			}
		}

		if(!isDeleted)
		{
			IntegerAABB aabb0=mBoxBoundsMinMax[a];
			IntegerAABB aabb1=mBoxBoundsMinMax[b];
			if(aabb0.intersects(aabb1))
			{
				//Do we have two identical boxes co-located?  
				//I think it is possible to spawn two objects at exactly the same coords but for them to find no box-box contacts.
				//It is a bit tricky (but not impossible) to recreate the calculation that led to removal from the sap so our rather
				//blunt overlap test could subtly differ from the sap calculation that led to the pair removal.
				//Rather than try to recreate the sap removal calculation just test for almost identical boxes and only return a fault
				//if the boxes are really different.
				PxBounds3 bounds0;
				aabb0.decode(bounds0);
				PxBounds3 bounds1;
				aabb0.decode(bounds1);
				const PxVec3 diffMin=bounds0.minimum-bounds1.minimum;
				const PxVec3 diffMax=bounds0.maximum-bounds1.maximum;
				const PxF32 distMin2=diffMin.magnitudeSquared();
				const PxF32 distMax2=diffMax.magnitudeSquared();
				if(distMin2 > (1e-4f*1e-4f) || distMax2 > (1e-4f*1e-4f))
				{
					//No the boxes are either different or at different coords.
					//We have really found a problem that the sap reported a removal but the aabbs still overlap.
					return false;
				}
			}
		}
	}

	return true;
}
#endif



/*

PX_FORCE_INLINE bool intersect1D_Max(	const SAP_AABB& a,
									 const SapBox1D*const* PX_RESTRICT boxEndPts,
									 PxU32 ownerId,
									 const BPValType* const endPointValues, PxU32 axis)
{
	const SapBox1D* PX_RESTRICT b = boxEndPts[axis] + ownerId;

	const BPValType& endPointValue = endPointValues[b->mMinMax[0]];
	return (endPointValue < a.GetMax(axis));
}

PX_FORCE_INLINE bool intersect1D_Min(	const SAP_AABB& a,
									 const SapBox1D*const* PX_RESTRICT boxEndPts,
									 PxU32 ownerId,
									 const BPValType* PX_RESTRICT endPointValues,
									 PxU32 axis)
{
	const SapBox1D* PX_RESTRICT b = boxEndPts[axis] + ownerId;

	const BPValType& endPointValue = endPointValues[b->mMinMax[1]];
	return (endPointValue >= a.GetMin(axis));
}

void PxsBroadPhaseContextSap::batchUpdate()
{
	for(PxU32 i=0;i<mUpdatedSize;i++)
	{
		const PxU32 handle = mUpdated[i];
		PX_ASSERT(handle!=PX_INVALID_BP_HANDLE);
		PX_ASSERT(mBoxEndPts[0][handle].mMinMax[0]!=PX_INVALID_BP_HANDLE);

		SapBox1D* Object[3] = {&mBoxEndPts[0][handle], &mBoxEndPts[1][handle], &mBoxEndPts[2][handle]};

		PX_ASSERT(mBoxEndPts[0][handle].mMinMax[0]!=PX_INVALID_BP_HANDLE);
		PX_ASSERT(mBoxEndPts[0][handle].mMinMax[1]!=PX_INVALID_BP_HANDLE);
		PX_ASSERT(mBoxEndPts[1][handle].mMinMax[0]!=PX_INVALID_BP_HANDLE);
		PX_ASSERT(mBoxEndPts[1][handle].mMinMax[1]!=PX_INVALID_BP_HANDLE);
		PX_ASSERT(mBoxEndPts[2][handle].mMinMax[0]!=PX_INVALID_BP_HANDLE);
		PX_ASSERT(mBoxEndPts[2][handle].mMinMax[1]!=PX_INVALID_BP_HANDLE);

		IntegerAABB box;
		box.mMinX = mBoxBoundsMinMaxX[2*handle+0];
		box.mMaxX = mBoxBoundsMinMaxX[2*handle+1];
		box.mMinY = mBoxBoundsMinMaxY[2*handle+0];
		box.mMaxY = mBoxBoundsMinMaxY[2*handle+1];
		box.mMinZ = mBoxBoundsMinMaxZ[2*handle+0];
		box.mMaxZ = mBoxBoundsMinMaxZ[2*handle+1];

		//	PxU32 Axis=0;
		for(PxU32 Axis=0;Axis<3;Axis++)
		{
			const SapBox1D* Object_Axis = &mBoxEndPts[Axis][handle];

			const PxU32 Axis1 = (1  << Axis) & 3;
			const PxU32 Axis2 = (1  << Axis1) & 3;

			BPValType* const BaseEPValue = mEndPointValues[Axis];
			PxBpHandle* const BaseEPData = mEndPointDatas[Axis];

			// Update min
			{
				const PxBpHandle MinMaxIndex = Object_Axis->mMinMax[0];
				BPValType* CurrentMinValue = BaseEPValue + MinMaxIndex;
				PxBpHandle* CurrentMinData = BaseEPData + MinMaxIndex;
				PX_ASSERT(!isMax(*CurrentMinData));

				const BPValType Limit = box.GetMin(Axis);
				if(Limit < *CurrentMinValue)
				{
					*CurrentMinValue = Limit;

					// Min is moving left:
					BPValType SavedValue = *CurrentMinValue;
					PxBpHandle SavedData = *CurrentMinData;
					PxU32 EPIndex = PxU32(size_t(CurrentMinData) - size_t(BaseEPData))/sizeof(PxBpHandle);
					const PxU32 SavedIndex = EPIndex;

					CurrentMinData--;
					CurrentMinValue--;
					while(*CurrentMinValue > Limit)
					{
#if BP_SAP_USE_PREFETCH
						Ps::prefetchLine(CurrentMinValue-1);
						Ps::prefetchLine(CurrentMinData-1);
#endif
						const PxU32 ownerId = getOwner(*CurrentMinData);
						SapBox1D* id1box = mBoxEndPts[Axis] + ownerId;

						const PxU32 IsMax = isMax(*CurrentMinData);
						if(IsMax)
						{
							// Our min passed a max => start overlap
							if(
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								(mBoxGroups[handle]!=mBoxGroups[ownerId]) && 
#endif								
								intersect2D(Object, mBoxEndPts, ownerId, Axis1, Axis2) &&
								intersect1D_Max(box, mBoxEndPts, ownerId, BaseEPValue, Axis) &&
								Object_Axis != id1box)
							{
								AddPair(handle, getOwner(*CurrentMinData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}

						id1box->mMinMax[IsMax] = EPIndex--;
						*(CurrentMinValue+1) = *CurrentMinValue;
						*(CurrentMinData+1) = *CurrentMinData;

						CurrentMinValue--;
						CurrentMinData--;
					}

					if(SavedIndex!=EPIndex)
					{
						mBoxEndPts[Axis][getOwner(SavedData)].mMinMax[isMax(SavedData)] = EPIndex;
						BaseEPValue[EPIndex] = SavedValue;
						BaseEPData[EPIndex] = SavedData;
					}
				}
				else if(Limit > *CurrentMinValue)
				{
					*CurrentMinValue = Limit;

					// Min is moving right:
					BPValType SavedValue = *CurrentMinValue;
					PxBpHandle SavedData = *CurrentMinData;

					PxU32 EPIndex = PxU32(size_t(CurrentMinData) - size_t(BaseEPData))/sizeof(PxBpHandle);
					const PxU32 SavedIndex = EPIndex;

					CurrentMinValue++;
					CurrentMinData++;
					while(Limit > (*CurrentMinValue))
					{
#if BP_SAP_USE_PREFETCH
						Ps::prefetchLine(CurrentMinValue+1);
						Ps::prefetchLine(CurrentMinData+1);
#endif
						const PxU32 ownerId = getOwner(*CurrentMinData);
						SapBox1D* id1box = mBoxEndPts[Axis] + ownerId;

						const PxU32 IsMax = isMax(*CurrentMinData);
						if(IsMax)
						{
							// Our min passed a max => stop overlap
							if(
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								(mBoxGroups[handle]!=mBoxGroups[ownerId]) && 
#endif								
#if BP_SAP_USE_OVERLAP_TEST_ON_REMOVES
								intersect2D(Object, mBoxEndPts, ownerId, Axis1, Axis2) &&
#endif
								Object_Axis != id1box)
							{
								RemovePair(handle, getOwner(*CurrentMinData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}

						id1box->mMinMax[IsMax] = EPIndex++;
						*(CurrentMinValue-1) = *CurrentMinValue;
						*(CurrentMinData-1) = *CurrentMinData;

						CurrentMinValue++;
						CurrentMinData++;
					}

					if(SavedIndex!=EPIndex)
					{
						mBoxEndPts[Axis][getOwner(SavedData)].mMinMax[isMax(SavedData)] = EPIndex;
						BaseEPValue[EPIndex] = SavedValue;
						BaseEPData[EPIndex] = SavedData;
					}
				}
			}

			// Update max
			{
				const PxBpHandle MinMaxIndex = Object_Axis->mMinMax[1];
				BPValType* CurrentMaxValue = BaseEPValue + MinMaxIndex;
				PxBpHandle* CurrentMaxData = BaseEPData + MinMaxIndex;
				PX_ASSERT(isMax(*CurrentMaxData));

				const BPValType Limit = box.GetMax(Axis);
				if(Limit > *CurrentMaxValue)
				{
					*CurrentMaxValue = Limit;

					// Max is moving right:
					BPValType SavedValue = *CurrentMaxValue;
					PxBpHandle SavedData = *CurrentMaxData;

					PxU32 EPIndex = PxU32(size_t(CurrentMaxData) - size_t(BaseEPData))/sizeof(PxBpHandle);
					const PxU32 SavedIndex = EPIndex;

					CurrentMaxValue++;
					CurrentMaxData++;
					while((*CurrentMaxValue) < Limit)
					{
#if BP_SAP_USE_PREFETCH
						Ps::prefetchLine(CurrentMaxValue+1);
						Ps::prefetchLine(CurrentMaxData+1);
#endif
						const PxU32 ownerId = getOwner(*CurrentMaxData);
						SapBox1D* id1box = mBoxEndPts[Axis] + ownerId;

						const PxU32 IsMax = isMax(*CurrentMaxData);
						if(!IsMax)
						{
							// Our max passed a min => start overlap
							if(
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								(mBoxGroups[handle]!=mBoxGroups[ownerId]) && 
#endif								
								intersect2D(Object, mBoxEndPts, ownerId, Axis1, Axis2) &&
								intersect1D_Min(box, mBoxEndPts, ownerId, BaseEPValue, Axis) &&
								Object_Axis != id1box)
							{
								AddPair(handle, getOwner(*CurrentMaxData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}

						id1box->mMinMax[IsMax] = EPIndex++;
						*(CurrentMaxValue-1) = *CurrentMaxValue;
						*(CurrentMaxData-1) = *CurrentMaxData;

						CurrentMaxValue++;
						CurrentMaxData++;
					}

					if(SavedIndex!=EPIndex)
					{
						mBoxEndPts[Axis][getOwner(SavedData)].mMinMax[isMax(SavedData)] = EPIndex;
						BaseEPValue[EPIndex] = SavedValue;
						BaseEPData[EPIndex] = SavedData;
					}
				}
				else if(Limit < *CurrentMaxValue)
				{
					*CurrentMaxValue = Limit;

					// Max is moving left:
					BPValType SavedValue = *CurrentMaxValue;
					PxBpHandle SavedData = *CurrentMaxData;

					PxU32 EPIndex = PxU32(size_t(CurrentMaxData) - size_t(BaseEPData))/sizeof(PxBpHandle);
					const PxU32 SavedIndex = EPIndex;

					CurrentMaxData--;
					CurrentMaxValue--;
					while(Limit < (*CurrentMaxValue))
					{
#if BP_SAP_USE_PREFETCH
						Ps::prefetchLine(CurrentMaxValue-1);
						Ps::prefetchLine(CurrentMaxData-1);
#endif
						const PxU32 ownerId = getOwner(*CurrentMaxData);
						SapBox1D* id1box = mBoxEndPts[Axis] + ownerId;

						const PxU32 IsMax = isMax(*CurrentMaxData);
						if(!IsMax)
						{
							// Our max passed a min => stop overlap
							if(
#if BP_SAP_TEST_GROUP_ID_CREATEUPDATE
								(mBoxGroups[handle]!=mBoxGroups[ownerId]) && 
#endif								
#if BP_SAP_USE_OVERLAP_TEST_ON_REMOVES
								intersect2D(Object, mBoxEndPts, ownerId, Axis1, Axis2) &&
#endif
								Object_Axis != id1box)
							{
								RemovePair(handle, getOwner(*CurrentMaxData), mPairs, mData, mDataSize, mDataCapacity);
							}
						}

						id1box->mMinMax[IsMax] = EPIndex--;
						*(CurrentMaxValue+1) = *CurrentMaxValue;
						*(CurrentMaxData+1) = *CurrentMaxData;

						CurrentMaxData--;
						CurrentMaxValue--;
					}

					if(SavedIndex!=EPIndex)
					{
						mBoxEndPts[Axis][getOwner(SavedData)].mMinMax[isMax(SavedData)] = EPIndex;
						BaseEPValue[EPIndex] = SavedValue;
						BaseEPData[EPIndex] = SavedData;
					}
				}
			}
		}
	}
}
*/




