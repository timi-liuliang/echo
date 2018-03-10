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

#include "SqAABBTreeUpdateMap.h"
#include "SqAABBTree.h"

using namespace physx;
using namespace Gu;
using namespace Sq;

#ifdef __SPU__
#error 
#endif

static const PxU32 SHRINK_THRESHOLD = 1024;

void AABBTreeUpdateMap::initMap(PxU32 nbObjects, const Sq::AABBTree& tree)
{
	if(!nbObjects)	
	{
		release();
		return;
	}

	PxU32 mapSize = nbObjects;
	PxU32 currentCapacity = mMapping.capacity();
	PxU32 targetCapacity = mapSize + (mapSize>>2);

	if( ( targetCapacity < (currentCapacity>>1) ) && ( (currentCapacity-targetCapacity) > SHRINK_THRESHOLD ) )
	{
		// trigger reallocation of a smaller array, there is enough memory to save
		currentCapacity = 0;
	}

	if(mapSize > currentCapacity)
	{
		// the mapping values are invalid and reset below in any case
		// so there is no need to copy the values at all
		mMapping.reset();
		mMapping.reserve(targetCapacity);	// since size is 0, reserve will also just allocate
	}

	mMapping.forceSize_Unsafe(mapSize);

	for(PxU32 i=0;i<mapSize;i++)
		mMapping[i] = INVALID_NODE_ID;

	PxU32 nbNodes = tree.getNbNodes();
	const AABBTreeNode*	nodes = tree.getNodes();
	for(PxU32 i=0;i<nbNodes;i++)
	{
		if(nodes[i].isLeaf() && nodes[i].getPrimitives(tree.getIndices()))
		{
			PX_ASSERT(nodes[i].getNbRuntimePrimitives()==1);
			PxU32 index = nodes[i].getPrimitives(tree.getIndices())[0];
			PX_ASSERT(index<nbObjects);		// temp hack
			mMapping[index] = i;
		}
	}
}

bool AABBTreeUpdateMap::checkMap(PxU32 nbPoolObjects, const AABBTree& tree) const
{
	PX_UNUSED(nbPoolObjects);
	if(!mMapping.size()) return false;

	const AABBTreeNode* nodes = tree.getNodes();
	for(PxU32 poolIndex=0; poolIndex<mMapping.size(); poolIndex++)
	{
		PxU32 nodeIndex = mMapping[poolIndex];
		if(nodeIndex == INVALID_NODE_ID) continue;
		PX_ASSERT(nodeIndex < tree.getNbNodes());
		PX_ASSERT(poolIndex == nodes[nodeIndex].getPrimitives(tree.getIndices())[0]);
	}
	
	for(PxU32 nodeIndex=0; nodeIndex<tree.getNbNodes(); nodeIndex++)
	{
		if(nodes[nodeIndex].isLeaf())
		{
			PxU32 poolIndex = nodes[nodeIndex].getPrimitives(tree.getIndices())[0];
			if(poolIndex == INVALID_POOL_ID) continue;
			PX_ASSERT(poolIndex < nbPoolObjects);
			PX_ASSERT(mMapping[poolIndex] == nodeIndex);
		}
	}

	return true;
}

void AABBTreeUpdateMap::invalidate(PxU32 prunerIndex0, PxU32 prunerIndex1, AABBTree& tree)
{
	// prunerIndex0 and prunerIndex1 are both indices into the pool, not handles
	// prunerIndex0 is the index in the pruning pool for the node that was just removed
	// prunerIndex1 is the index in the pruning pool for the node
	PxU32 nodeIndex0 = prunerIndex0<mMapping.size() ? mMapping[prunerIndex0] : INVALID_NODE_ID;
	PxU32 nodeIndex1 = prunerIndex1<mMapping.size() ? mMapping[prunerIndex1] : INVALID_NODE_ID;

	//printf("map invalidate pi0:%x ni0:%x\t",prunerIndex0,nodeIndex0);
	//printf("  replace with pi1:%x ni1:%x\n",prunerIndex1,nodeIndex1);

	// if nodeIndex0 exists:
	//		invalidate node 0 
	//		invalidate map prunerIndex0
	// if nodeIndex1 exists: 
	//		point node 1 to prunerIndex0
	//		map prunerIndex0 to node 1
	//		invalidate map prunerIndex1

	// eventually: 
	// - node 0 is invalid
	// - prunerIndex0 is mapped to node 1 or
	//		is not mapped if prunerIndex1 is not mapped
	//		is not mapped if prunerIndex0==prunerIndex1
	// - node 1 points to prunerIndex0 or  
	//		is invalid if prunerIndex1 is not mapped
	//		is invalid if prunerIndex0==prunerIndex1
	// - prunerIndex1 is not mapped

	AABBTreeNode* nodes = tree.getNodes();

	if(nodeIndex0!=INVALID_NODE_ID)
	{
		PX_ASSERT(nodeIndex0 < tree.getNbNodes());
		PX_ASSERT(nodes[nodeIndex0].isLeaf());
		PX_ASSERT(nodes[nodeIndex0].getNbRuntimePrimitives() == 1);

		// retrieve the primitives pointer (we expect only one primitive per node currently)
		PxU32* primitives = nodes[nodeIndex0].getPrimitives(tree.getIndices());
		PX_ASSERT(primitives);
		PX_ASSERT(prunerIndex0 == primitives[0]); // verify that the index is in sync

		nodes[nodeIndex0].setNbRunTimePrimitives(0);// zero out the primitive count
		primitives[0] = INVALID_POOL_ID;			// Mark primitive index as invalid in the node
		mMapping[prunerIndex0] = INVALID_NODE_ID;	// invalidate the node index for pool 0
	}

	if (nodeIndex1!=INVALID_NODE_ID)
	{
		if(nodeIndex0!=nodeIndex1)
		{
			PX_ASSERT(nodeIndex1 < tree.getNbNodes());
			PX_ASSERT(nodes[nodeIndex1].isLeaf());
			PX_ASSERT(nodes[nodeIndex1].getNbRuntimePrimitives()==1);

			PxU32* primitives = nodes[nodeIndex1].getPrimitives(tree.getIndices());
			PX_ASSERT(primitives[0] == prunerIndex1);
			
			primitives[0] = prunerIndex0;				// point node 1 to the pool object moved to ID 0
			mMapping[prunerIndex0] = nodeIndex1;		// pool 0 is pointed at by node 1 now
			mMapping[prunerIndex1] = INVALID_NODE_ID;	// pool 1 is no longer stored in the tree
		}
		else
		{
			// node 1 was invalidated just before, because of nodeIndex1==nodeIndex0
			PX_ASSERT(nodes[nodeIndex1].getPrimitives(tree.getIndices())[0] == INVALID_POOL_ID);
			PX_ASSERT(mMapping[prunerIndex1] == INVALID_NODE_ID); // this also means prunerIndex0==prunerIndex1
		}
	}

	// eventually: 
	// - node 0 is invalid
	if(nodeIndex0!=INVALID_NODE_ID)
	{
		PX_ASSERT(nodes[nodeIndex0].getPrimitives(tree.getIndices())[0] == INVALID_POOL_ID);
		PX_ASSERT(nodes[nodeIndex0].getNbRuntimePrimitives() == 0);
	}
	// - prunerIndex0 is mapped to node 1 or
	//		is not mapped if prunerIndex1 is not mapped
	//		is not mapped if prunerIndex0==prunerIndex1
	// - node 1 points to prunerIndex0 or  
	//		is invalid if prunerIndex1 is not mapped
	//		is invalid if prunerIndex0==prunerIndex1
	if(nodeIndex1==INVALID_NODE_ID)
	{
		PX_ASSERT(prunerIndex0 >= mMapping.size() || mMapping[prunerIndex0] == INVALID_NODE_ID);
	}
	else if(prunerIndex0==prunerIndex1)
	{
		PX_ASSERT(prunerIndex0 >= mMapping.size() || mMapping[prunerIndex0] == INVALID_NODE_ID);
		PX_ASSERT(nodes[nodeIndex1].getPrimitives(tree.getIndices())[0] == INVALID_POOL_ID);
		PX_ASSERT(nodes[nodeIndex1].getNbRuntimePrimitives() == 0);
	}
	else
	{
		PX_ASSERT(mMapping[prunerIndex0] == nodeIndex1);
		PX_ASSERT(nodes[nodeIndex1].getPrimitives(tree.getIndices())[0] == prunerIndex0);
		PX_ASSERT(nodes[nodeIndex1].getNbRuntimePrimitives() == 1);
	}
	// - prunerIndex1 is not mapped
	PX_ASSERT(prunerIndex1 >= mMapping.size() || mMapping[prunerIndex1] == INVALID_NODE_ID);

}

