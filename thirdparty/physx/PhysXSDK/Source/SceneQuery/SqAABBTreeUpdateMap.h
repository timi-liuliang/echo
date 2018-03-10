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

#ifndef SQ_PRUNERTREEMAP_H
#define SQ_PRUNERTREEMAP_H

#include "SqPruner.h"

namespace physx
{
namespace Sq
{
	class AABBTree;
	static const PxU32 INVALID_NODE_ID = 0xFFffFFff;
	static const PxU32 INVALID_POOL_ID = 0xFFffFFff;

	// Maps pruning pool indices to AABB-tree indices (i.e. locates the object's box in the aabb-tree nodes pool)
	// 
	// The map spans pool indices from 0..N-1, where N is the number of pool entries when the map was created from a tree.
	//
	// It maps: 
	//		to node indices in the range 0..M-1, where M is the number of nodes in the tree the map was created from,
	//   or to INVALID_NODE_ID if the pool entry was removed or pool index is outside input domain.
	//
	// The map is the inverse of the tree mapping: (node[map[poolID]].primitive == poolID) is true at all times.

	class AABBTreeUpdateMap 
	{
	public:

		AABBTreeUpdateMap() {}

		void release()
		{
			mMapping.reset();
		}

		void initMap(PxU32 numPoolObjects, const AABBTree& tree);

		void invalidate(PxU32 poolIndex, PxU32 replacementPoolIndex, AABBTree& tree);

		PxU32 operator[](PxU32 i) const
		{ 
			return i < mMapping.size() ? mMapping[i] : INVALID_NODE_ID;
		}

	private:
		bool checkMap(PxU32 numPoolObjects, const AABBTree& tree) const;

		// maps from prunerIndex (index in the PruningPool) to treeNode index
		// this will only map to leaf tree nodes
		Ps::Array<PxU32>	mMapping;
	};

}
}

#endif
