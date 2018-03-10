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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "PxMemory.h"
#include "SqPruningPool.h"

using namespace physx;
using namespace Sq;
using namespace Cm;

#ifdef __SPU__
#undef PX_NEW
#undef PX_DELETE_AND_RESET

#define PX_NEW(a) (a*)((a*)NULL)
#define PX_DELETE_AND_RESET(a)
#endif

PruningPool::PruningPool()
:	mNbObjects(0)
,	mMaxNbObjects(0)
,	mWorldBoxes(NULL)
,	mObjects(NULL)
,	mHandleToIndex(0)
,	mIndexToHandle(0)
,	mFirstFreshHandle(0)
,	mHandleFreeList(INVALID_PRUNERHANDLE)
{}


#ifndef __SPU__
PruningPool::~PruningPool()
{
	PX_FREE_AND_RESET(mWorldBoxes);
	PX_FREE_AND_RESET(mObjects);
	PX_FREE_AND_RESET(mHandleToIndex);
	PX_FREE_AND_RESET(mIndexToHandle);
}
#endif




void PruningPool::resize(PxU32 newCapacity)
{
#ifndef __SPU__
	PxBounds3*		newBoxes			= (PxBounds3*)		PX_ALLOC(sizeof(PxBounds3)*newCapacity, PX_DEBUG_EXP("PxBounds3"));
	PrunerPayload*	newData				= (PrunerPayload*)	PX_ALLOC(sizeof(PrunerPayload)*newCapacity, PX_DEBUG_EXP("PrunerPayload*"));
	PxU32*			newIndexToHandle	= (PxU32*)			PX_ALLOC(sizeof(PxU32)*newCapacity, PX_DEBUG_EXP("Pruner Index Mapping"));
	PxU32*			newHandleToIndex	= (PxU32*)			PX_ALLOC(sizeof(PxU32)*newCapacity, PX_DEBUG_EXP("Pruner Index Mapping"));

	if( (NULL==newBoxes) || (NULL==newData) || (NULL==newIndexToHandle) || (NULL==newHandleToIndex) )
	{
		PX_FREE_AND_RESET(newBoxes);
		PX_FREE_AND_RESET(newData);
		PX_FREE_AND_RESET(newIndexToHandle);
		PX_FREE_AND_RESET(newHandleToIndex);
		return;
	}

	if(mWorldBoxes)		PxMemCopy(newBoxes, mWorldBoxes, mNbObjects*sizeof(PxBounds3));
	if(mObjects)		PxMemCopy(newData, mObjects, mNbObjects*sizeof(PrunerPayload));
	if(mIndexToHandle)	PxMemCopy(newIndexToHandle, mIndexToHandle, mNbObjects*sizeof(PxU32));
	if(mHandleToIndex)	PxMemCopy(newHandleToIndex, mHandleToIndex, mMaxNbObjects*sizeof(PxU32));

	mMaxNbObjects = newCapacity;

	PX_FREE_AND_RESET(mWorldBoxes);
	PX_FREE_AND_RESET(mObjects);
	PX_FREE_AND_RESET(mHandleToIndex);
	PX_FREE_AND_RESET(mIndexToHandle);

	mWorldBoxes		= newBoxes;
	mObjects		= newData;
	mHandleToIndex	= newHandleToIndex;
	mIndexToHandle	= newIndexToHandle;
#endif
}

void PruningPool::preallocate(PxU32 newCapacity)
{
	if(newCapacity>mMaxNbObjects)
		resize(newCapacity);
}

PrunerHandle PruningPool::addObject(const PxBounds3& worldAABB, const PrunerPayload& payload)
{
	if(mNbObjects==mMaxNbObjects) // increase the capacity on overflow
		resize(PxMax<PxU32>(mMaxNbObjects*2, 64));

	if(mNbObjects==mMaxNbObjects)
		return INVALID_PRUNERHANDLE;

	const PxU32 index = mNbObjects++;
	mWorldBoxes[index] = worldAABB; // store the payload and AABB in parallel arrays
	mObjects[index] = payload;

	// update mHandleToIndex and mIndexToHandle mappings
	PxU32 handle;
	if(mHandleFreeList != INVALID_PRUNERHANDLE)
		// mHandleFreeList is an entry into a freelist for removed slots
		// this path is only taken if we have any removed slots
	{
		handle = mHandleFreeList;
		mHandleFreeList = mHandleToIndex[handle];
	} else
		// if we just keep adding objects this is the only path we take
		handle = mFirstFreshHandle++;

	mIndexToHandle[index] = handle;
	mHandleToIndex[handle] = index;
	return handle;
}

PxU32 PruningPool::removeObject(PrunerHandle h)
{
	PX_ASSERT(mNbObjects);

	// remove the object and it's AABB by provided PrunerHandle and update mHandleToIndex and mIndexToHandle mappings
	PxU32 index = mHandleToIndex[h]; // retrieve objectindex from handle

	const PxU32 lastIndex = --mNbObjects; // swap the object at last index with index
	if(lastIndex!=index)
	{
		PxU32 lastIndexHandle = mIndexToHandle[lastIndex];

		mWorldBoxes[index]				= mWorldBoxes[lastIndex];
		mObjects[index]					= mObjects[lastIndex];
		mIndexToHandle[index]			= lastIndexHandle;
		mHandleToIndex[lastIndexHandle] = index;
	}

	// mHandleToIndex also stores the freelist for removed handles (in place of holes formed by removed handles)
	mHandleToIndex[h] = mHandleFreeList; // save the list head in the newly opened hole
	mHandleFreeList = h; // update the list head

	return lastIndex;
}


void PruningPool::shiftOrigin(const PxVec3& shift)
{
	for(PxU32 i=0; i < mNbObjects; i++)
	{
		mWorldBoxes[i].minimum -= shift;
		mWorldBoxes[i].maximum -= shift;
	}
}
