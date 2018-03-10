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

#ifndef SQ_DYNAMICPRUNER2_H
#define SQ_DYNAMICPRUNER2_H

#include "SqPruningPool.h"
#include "SqTreeBuilders.h"
#include "SqBucketPruner.h"
#include "SqAABBTreeUpdateMap.h"
#include "SqTreeBuilders.h"

#include "PsHashSet.h"

namespace physx
{

namespace Sq
{
	class AABBTree;
	class AABBTreeNode;

	enum BuildStatus
	{
		BUILD_NOT_STARTED, // looks like the number of states can be reduced
		BUILD_INIT,
		BUILD_IN_PROGRESS,
		BUILD_FINISHED,

		BUILD_FORCE_DWORD	= 0xffffffff
	};

	// This class implements the Pruner interface for internal SQ use with some additional specialized functions
	// The underlying data structure is a binary AABB tree
	// AABBPruner supports insertions, removals and updates for dynamic objects
	// The tree is either entirely rebuilt in a single frame (static pruner) or progressively rebuilt over multiple frames (dynamic pruner)
	// The rebuild happens on a copy of the tree
	// the copy is then swapped with current tree at the time commit() is called (only if mBuildState is BUILD_FINISHED),
	// otherwise commit() will perform a refit operation applying any pending changes to the current tree
	// While the tree is being rebuilt a temporary data structure (BucketPruner) is also kept in sync and used to speed up
	// queries on updated objects that are not yet in either old or new tree.
	// The requirements on the order of calls:
	// commit() is required to be called before any queries to apply modifications
	// queries can be issued on multiple threads after commit is called
	// commit, buildStep, add/remove/update have to be called from the same thread or otherwise strictly serialized by external code
	// and cannot be issued while a query is running
	class AABBPruner
#if !PX_IS_SPU
		: public Pruner
#endif
	{
		public:
												AABBPruner(bool incrementalRebuild); // true is equivalent to former dynamic pruner
		virtual									~AABBPruner(); // keep this virtual on SPU as well so the layout is identical to CPU

		// Pruner Interface is non-virtual here to avoid SPU vtable patching
						bool					addObjects(PrunerHandle* results, const PxBounds3* bounds, const PrunerPayload* userData, PxU32 count = 1);
						void					removeObjects(const PrunerHandle* handles, PxU32 count = 1);
						void					updateObjects(const PrunerHandle* handles, const PxBounds3* newBounds, PxU32 count = 1);

						PxAgain					raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&)			const;
						PxAgain					sweep(const ShapeData& queryVolume, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&)		const;
						PxAgain					overlap(const ShapeData& queryVolume, PrunerCallback&) const;

						const PrunerPayload&	getPayload(const PrunerHandle& h) const { return mPool.getPayload(h); }
						void					preallocate(PxU32 entries) { mPool.preallocate(entries); }

						void					commit();
						void					shiftOrigin(const PxVec3& shift);
						void					visualize(Cm::RenderOutput& out, PxU32 color) const;

		// non-pruner interface
						void					setRebuildRateHint(PxU32 nbStepsForRebuild);	// Besides the actual rebuild steps, 3 additional steps are needed.
						bool					buildStep();	// returns true if finished
						void					purge();		// gets rid of internal accel struct

		// direct access for SPU and test code

		PX_FORCE_INLINE	PxU32					getNbAddedObjects()	const				{ return mBuf0.size() + mBuf1.size();	}
		PX_FORCE_INLINE	const AABBTree*			getAABBTree()	const					{ PX_ASSERT(!mUncommittedChanges); return mAABBTree;	}
		PX_FORCE_INLINE	AABBTree*				getAABBTree()							{ PX_ASSERT(!mUncommittedChanges); return mAABBTree;	}
		PX_FORCE_INLINE	void					setAABBTree(AABBTree* tree)				{ mAABBTree = tree; }
		PX_FORCE_INLINE	const AABBTree*			hasAABBTree()	const					{ return mAABBTree;	}
				
		// local functions
		private:
						Sq::AABBTree*			mAABBTree; // current active tree
						Sq::AABBTreeBuilder		mBuilder; // this class deals with the details of the actual tree building

						// tree with build in progress, assigned to mAABBTree in commit, when mProgress is BUILD_FINISHED
						Sq::AABBTree*			mNewTree;

						// during rebuild the pool might change so we need a copy of boxes for the tree build
						PxBounds3*				mCachedBoxes;
						PxU32					mNbCachedBoxes;

						// incremented in commit(), serves as a progress counter for rebuild
						PxU32					mNbCalls;
						typedef Ps::HashSet<PrunerHandle> PrunerHandleSet;

						// swappable buffer object, can be pointed to either by mAddedObjects or mToRemoveFromBucket
						PrunerHandleSet			mBuf0;

						// Includes handles added to mBucketPruner
						PrunerHandleSet*		mAddedObjects;

						// swappable buffer object, can be pointed to either by mAddedObjects or mToRemoveFromBucket
						PrunerHandleSet			mBuf1;

						// PrunerHandles that were added to the bucket pruner up to the point the rebuild of the new tree started
						// these is effectively the same subset as mCachedBoxes
						// and will be removed from the bucket pruner by the time rebuild completes
						// in the meantime all the newly added objects will be inserted into mAddedObjects (and mBucketPruner)
						PrunerHandleSet*		mToRemoveFromBucket;

						// this pruner is used for queries on objects that are not in the current tree yet
						// includes both the objects in the tree being rebuilt and all the objects added later
						BucketPrunerCore		mBucketPruner;

						BuildStatus				mProgress;		// current state of second tree build progress

						// Fraction (as in 1/Nth) of the total number of primitives
						// that should be processed per step by the AABB builder
						// so if this value is 1, all primitives will be rebuilt, 2 => 1/2 of primitives per step etc.
						// see also mNbCalls, mNbCalls varies from 0 to mRebuildRateHint-1
						PxU32					mRebuildRateHint;

						// Estimate for how much work has to be done to rebuild the tree.
						PxU32					mTotalWorkUnits;

						// Term to correct the work unit estimate if the rebuild rate is not matched
						PxI32					mAdaptiveRebuildTerm;

						PruningPool				mPool; // Pool of AABBs

						// maps pruning pool indices to aabb tree indices
						// maps to INVALID_NODE_ID if the pool entry was removed or "pool index is outside input domain"
						// The map is the inverse of the tree mapping: (node[map[poolID]].primitive == poolID)
						// So:
						// treeNodeIndex = mTreeMap.operator[](poolIndex)
						// aabbTree->treeNodes[treeNodeIndex].primitives[0] == poolIndex
						AABBTreeUpdateMap		mTreeMap;

						// This is only set once in the constructor and is equivalent to isDynamicTree
						// if it set to false then a 1-shot rebuild is performed in commit()
						// bucket pruner is only used with incremental rebuild
						bool					mIncrementalRebuild;

						// A rebuild can be triggered even when the Pruner is not dirty
						// mUncommittedChanges is set to true in add, remove, update and buildStep
						// mUncommittedChanges is set to false in commit
						// mUncommittedChanges has to be false (commit() has to be called) in order to run a query as defined by the
						// Pruner higher level API
						bool					mUncommittedChanges;

						// A new AABB tree is built if an object was added, removed or updated
						// Changing objects during a build will trigger another rebuild right afterwards
						// this is set to true if a new tree has to be created again after the current rebuild is done
						bool					mNeedsNewTree;

						// set to false in commit(), if mProgress is BUILD_FINISHED
						// set to true in buildStep(), BUILD_NOT_STARTED
						// This is used to show that the there is a tree rebuild going on in progress
						// and indicates that we have to start saving the fixups
						bool					mDoSaveFixups;

						// This struct is used to record modifications made to the pruner state
						// while a tree is building in the background
						// this is so we can apply the modifications to the tree at the time of completion
						// the recorded fixup information is: removedIndex (in ::remove()) and 
						// lastIndexMoved which is the last index in the pruner array
						// (since the way we remove from PruningPool is by swapping last into removed slot,
						// we need to apply a fixup so that it syncs up that operation in the new tree)
						struct NewTreeFixup
						{
							NewTreeFixup(PxU32 removedIndex_, PxU32 relocatedLastIndex_)
								: removedIndex(removedIndex_), relocatedLastIndex(relocatedLastIndex_) {}
							PxU32 removedIndex;
							PxU32 relocatedLastIndex;
						};
						Ps::Array<NewTreeFixup>	mNewTreeFixups;

						// Internal methods
						bool					fullRebuildAABBTree(); // full rebuild function, used with static pruner mode
						void					release();
						void					refitUpdatedAndRemoved();
						void					updateBucketPruner();
	};

} // namespace Sq

}

#endif // SQ_DYNAMICPRUNER2_H
