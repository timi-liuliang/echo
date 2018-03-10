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
#include "PsIntrinsics.h"
#include "PsUserAllocated.h"
#include "PsBitUtils.h"
#include "SqAABBPruner.h"
#include "SqAABBTree.h"
#include "SqTreeBuilders.h"
#include "GuSphere.h"
#include "GuBox.h"
#include "GuCapsule.h"
#include "CmMemFetch.h"
#include "SqPrunerTestsSIMD.h"
#include "CmMemFetch.h"


using namespace physx;
using namespace Gu;
using namespace Sq;
using namespace Cm;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __SPU__
#undef PX_NEW
#undef PX_DELETE_AND_RESET

#define PX_NEW(a) (a*)((a*)NULL)
#define PX_DELETE_AND_RESET(a)
#endif

AABBPruner::AABBPruner(bool incrementalRebuild) 
#ifndef __SPU__
:	mAABBTree			(NULL)
,	mNewTree			(NULL)
,	mCachedBoxes		(NULL)
,	mNbCachedBoxes		(0)
,	mNbCalls			(0)
,	mAddedObjects		(&mBuf0)
,	mToRemoveFromBucket	(&mBuf1)
,	mBucketPruner		(false)
,	mProgress			(BUILD_NOT_STARTED)
,	mRebuildRateHint	(100)
,	mAdaptiveRebuildTerm(0)
,	mIncrementalRebuild	(incrementalRebuild)
,	mUncommittedChanges(false)
,	mNeedsNewTree		(false)
,	mDoSaveFixups		(false)
,	mNewTreeFixups		(PX_DEBUG_EXP("AABBPrunerMapper::mNewTreeFixups"))
#endif
{
	// ensure that there is a vtable
	PX_COMPILE_TIME_ASSERT(PX_OFFSET_OF(Sq::AABBPruner,mAABBTree)==sizeof(void*));
}


#ifndef __SPU__
AABBPruner::~AABBPruner()
{
	release();
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Add, Remove, Update methods
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool AABBPruner::addObjects(PrunerHandle* results, const PxBounds3* bounds, const PrunerPayload* payload, PxU32 count)
{
	// no need to do refitMarked for added objects since they are not in the tree

	mUncommittedChanges = true;

	PxU32 valid = 0;

	for(PxU32 i=0;i<count;i++)
	{
		PrunerHandle h = mPool.addObject(bounds[i], payload[i]);
		results[i] = h;
		// pool can return an invalid handle if out of preallocated mem
		// should probably have an error here or not handle this
		if (h == INVALID_PRUNERHANDLE)
			break;

		valid++;
	}


	// Bucket pruner is only used while the dynamic pruner is rebuilding
	// For the static pruner a full rebuild will happen in commit() every time we modify something
	if(mIncrementalRebuild && mAABBTree)
	{
		mNeedsNewTree = true; // each add forces a tree rebuild

		for(PxU32 i=0;i<valid;i++)
		{
			PrunerHandle h = results[i];
			mAddedObjects->insert(h);
			mBucketPruner.addObject(payload[i], bounds[i]);
		}
	}

	return valid==count;
}

void AABBPruner::updateObjects(const PrunerHandle* handles, const PxBounds3* newBounds, PxU32 count)
{
	mUncommittedChanges = true;

	for(PxU32 i=0; i<count; i++)
		mPool.updateObject(handles[i],newBounds[i]); // only updates the bounds

	if(mIncrementalRebuild && mAABBTree) 
	{
		mNeedsNewTree = true; // each update forces a tree rebuild
		for(PxU32 i=0; i<count; i++)
		{
			PxU32 poolIndex = mPool.getIndex(handles[i]);
			PxU32 treeNodeIndex = mTreeMap[poolIndex];
			if(treeNodeIndex!=INVALID_PRUNERHANDLE) // this means it's in the current tree still and hasn't been removed
				mAABBTree->markForRefit(treeNodeIndex);
			else // otherwise it means it should be in the bucket pruner
			{
				bool found = mBucketPruner.updateObject(newBounds[i], mPool.getPayload(handles[i]));
				PX_UNUSED(found); PX_ASSERT(found);
			}
		}
	}
}

void AABBPruner::removeObjects(const PrunerHandle* handles, PxU32 count)
{	
	mUncommittedChanges = true;

	for(PxU32 i=0; i<count; i++)
	{
		PrunerHandle h = handles[i];
		// copy the payload before removing it since we need to know the payload to remove it from the bucket pruner
		const PrunerPayload removedPayload = mPool.getPayload(h);
		PxU32 poolIndex = mPool.getIndex(h); // save the pool index for removed object
		PxU32 poolRelocatedLastIndex = mPool.removeObject(h); // save the lastIndex returned by removeObject

		if(mIncrementalRebuild && mAABBTree)
		{
			mNeedsNewTree = true;
			
			PxU32 treeIndexForPoolIndex = mTreeMap[poolIndex]; // already removed from pool but still in tree map
			if(treeIndexForPoolIndex != INVALID_PRUNERHANDLE) // can be invalid if removed
				mAABBTree->markForRefit(treeIndexForPoolIndex); // mark the spot as blank

			if(mBuf0.erase(h) || mBuf1.erase(h)) // has to be in either one of the buffers to be in the bucket pruner
				mBucketPruner.removeObject(removedPayload);

			mTreeMap.invalidate(poolIndex, poolRelocatedLastIndex, *mAABBTree);
			if(mDoSaveFixups)
				mNewTreeFixups.pushBack(NewTreeFixup(poolIndex, poolRelocatedLastIndex));
		}
	}

	if (mPool.getNbActiveObjects()==0)
	{
		// this is just to make sure we release all the internal data once all the objects are out of the pruner
		// since this is the only place we know that and we we don't want to keep memory reserved
		release();

		// Pruner API requires a commit before the next query, even if we ended up removing the entire tree here. This
		// forces that to happen.
		mUncommittedChanges = true;
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Query Implementation
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __SPU__
	#define NODE_PARAMS	mAABBTreeNodes, mNodesEA, mIndicesEA
#else
	#define NODE_PARAMS	mAABBTree->getNodes(), mAABBTree->getNodes(), mAABBTree->GetIndices()
#endif

// this function performs translation from PxU32 object stored in AABBTree to PrunerPayload* that is expected to be returned
// by PrunerInterface
static PX_FORCE_INLINE PrunerPayload* translatePxU32ToPrunerPayload(PxU32 storedInAABBtree, PrunerPayload* objectsBase)
{
	if (PX_IS_SPU && storedInAABBtree == PxU32(-1)) // AP: can happen if the object is deleted, why only on SPU though?
		return NULL;

	MemFetchSmallBuffer buf;
	// we don't store payloads directly in the tree so we fetch another PPU address here
	//pxPrintf("SQRawQuerySweepVisitor, objectIndex = %d\n", objectIndex);
	PrunerPayload* p = memFetchAsync<PrunerPayload>(MemFetchPtr(objectsBase+storedInAABBtree), 4, buf);
	memFetchWait(4);

	return p;
}

#define RAW_TRAVERSAL_STACK_SIZE 256

template<typename Test>
class AABBTreeOverlap
{
public:
	bool operator()(PrunerPayload* objects, const AABBTree& tree, const Test& test, PrunerCallback &visitor)
	{
		using namespace Cm;

		const AABBTreeNode* stack[RAW_TRAVERSAL_STACK_SIZE];
		const AABBTreeNode* nodeBase = tree.getNodes();
		stack[0] = nodeBase;
		PxU32 stackIndex = 1;

		MemFetchBufferU<AABBTreeNode> nodeBuf;
		MemFetchBufferU<AABBTreeNode, 2> childBuf;
		MemFetchSmallBuffer buf;
		PxU32* prunableIndex;

		while(stackIndex>0)
		{
			const AABBTreeNode* node = memFetchAsync<AABBTreeNode>(MemFetchPtr(stack[--stackIndex]), 3, nodeBuf);	
			memFetchWait(3);
			Vec3V center, extents;
			node->getAABBCenterExtentsV(&center, &extents);
			while(test(center, extents))
			{
				if(node->isLeaf())
				{
					prunableIndex = memFetchAsync<PxU32>(MemFetchPtr(node->getPrimitives(tree.getIndices())), 4, buf);
					memFetchWait(4);
					PxReal unusedDistance;
					if (!visitor.invoke(unusedDistance, translatePxU32ToPrunerPayload(*prunableIndex, objects), 1))
						return false;
					break;
				}

				const AABBTreeNode *childrenEA = node->getPos(nodeBase);
				const AABBTreeNode *children = memFetchAsync<AABBTreeNode,2>(MemFetchPtr(childrenEA), 3, childBuf);
				memFetchWait(3);

				node = children;
				stack[stackIndex++] = childrenEA+1;
				PX_ASSERT(stackIndex < RAW_TRAVERSAL_STACK_SIZE);
				node->getAABBCenterExtentsV(&center, &extents);
			}
		}
		return true;
	}
};

template <bool tInflate> // use inflate=true for sweeps, inflate=false for raycasts
class AABBTreeRaycast
{
public:
	bool operator()(
		PrunerPayload* objects, const AABBTree& tree,
		const PxVec3& origin, const PxVec3& unitDir, PxReal &maxDist, const PxVec3& inflation,
		PrunerCallback& pcb)
	{
		using namespace Cm;

		Gu::RayAABBTest test(origin, unitDir, maxDist, inflation);

		const AABBTreeNode* stack[RAW_TRAVERSAL_STACK_SIZE]; // stack always contains PPU addresses
		const AABBTreeNode* nodeBase = tree.getNodes();
		stack[0] = nodeBase;
		PxU32 stackIndex = 1;

		MemFetchBufferU<AABBTreeNode> nodeBuf;
		MemFetchBufferU<AABBTreeNode, 2> childBuf;
		MemFetchSmallBuffer buf;
		PxU32* prunableIndex;

		PxReal oldMaxDist;
		while(stackIndex--)
		{
			const AABBTreeNode* node = memFetchAsync<AABBTreeNode>(MemFetchPtr(stack[stackIndex]), 3, nodeBuf);	
			memFetchWait(3);
			Vec3V center, extents;
			node->getAABBCenterExtentsV(&center, &extents);
			if(test.check<tInflate>(center, extents))	// TODO: try timestamp ray shortening to skip this
			{
				PxReal md = maxDist; // has to be before the goto below to avoid compile error
				while(!node->isLeaf())
				{
					const AABBTreeNode *childrenEA = node->getPos(nodeBase);
					const AABBTreeNode *children = memFetchAsync<AABBTreeNode,2>(MemFetchPtr(childrenEA), 3, childBuf);
					memFetchWait(3);
					Vec3V c0, c1, e0, e1;
					children[0].getAABBCenterExtentsV(&c0, &e0);
					const PxU32 b0 = test.check<tInflate>(c0, e0);

					children[1].getAABBCenterExtentsV(&c1, &e1);
					const PxU32 b1 = test.check<tInflate>(c1, e1);

					if(b0 && b1)	// if both intersect, push the one with the further center on the stack for later
					{
						// & 1 because FAllGrtr behavior differs across platforms
						PxU32 bit = FAllGrtr(V3Dot(V3Sub(c1, c0), test.mDir), FZero()) & 1;
						stack[stackIndex++] = childrenEA + bit;
						node = children + (1 - bit);
						PX_ASSERT(stackIndex < RAW_TRAVERSAL_STACK_SIZE);
					}
					else if (b0)
						node = children;
					else if (b1)
						node = children+1;
					else
						goto skip_leaf_code;
				}

				oldMaxDist = maxDist; // we copy since maxDist can be updated in the callback and md<maxDist test below can fail
				prunableIndex = memFetchAsync<PxU32>(MemFetchPtr(node->getPrimitives(tree.getIndices())), 4, buf);
				memFetchWait(4);
				if (!pcb.invoke(md, translatePxU32ToPrunerPayload(*prunableIndex, objects), 1))
					return false;

				if(md < oldMaxDist)
				{
					maxDist = md;
					test.setDistance(md);
				}
skip_leaf_code:;
			}
		}
		return true;
	}
};



PxAgain AABBPruner::overlap(const ShapeData& queryVolume, PrunerCallback& pcb) const
{
	PX_ASSERT(!mUncommittedChanges);

	PxAgain again = true;
	
	if(mAABBTree)
	{
		switch(queryVolume.getOriginalPxGeom().getType())
		{
		case PxGeometryType::eBOX:
			{
				if (PxAbs(queryVolume.getPrunerWorldTransform().q.w) < 0.999999f)
				{	
					Gu::OBBAABBTest test(queryVolume.getPrunerWorldTransform(), queryVolume.getPrunerBoxGeom(), SQ_PRUNER_INFLATION);
					again = AABBTreeOverlap<Gu::OBBAABBTest>()(mPool.getObjects(), *mAABBTree, test, pcb);
				}
				else
				{
					Gu::AABBAABBTest test(queryVolume.getPrunerInflatedWorldAABB());
					again = AABBTreeOverlap<Gu::AABBAABBTest>()(mPool.getObjects(), *mAABBTree, test, pcb);
				}
			}
			break;
		case PxGeometryType::eCAPSULE:
			{
				Gu::CapsuleAABBTest test(queryVolume.getPrunerWorldTransform(), queryVolume.getNPPxCapsule(), SQ_PRUNER_INFLATION);
				again = AABBTreeOverlap<Gu::CapsuleAABBTest>()(mPool.getObjects(), *mAABBTree, test, pcb);
			}
			break;
		case PxGeometryType::eSPHERE:
			{
				const Gu::Sphere& sphere = queryVolume.getNPGuSphere();
				Gu::SphereAABBTest test(sphere.center, sphere.radius);
				again = AABBTreeOverlap<Gu::SphereAABBTest>()(mPool.getObjects(), *mAABBTree, test, pcb);
			}
			break;
		case PxGeometryType::eCONVEXMESH:
			{
				Gu::OBBAABBTest test(queryVolume.getPrunerWorldTransform(), queryVolume.getPrunerBoxGeom(), SQ_PRUNER_INFLATION);
				again = AABBTreeOverlap<Gu::OBBAABBTest>()(mPool.getObjects(), *mAABBTree, test, pcb);			
			}
			break;
		case PxGeometryType::ePLANE:
		case PxGeometryType::eTRIANGLEMESH:
		case PxGeometryType::eHEIGHTFIELD:
		case PxGeometryType::eGEOMETRY_COUNT:
		case PxGeometryType::eINVALID:
		default:
			PX_ALWAYS_ASSERT_MESSAGE("unsupported overlap query volume geometry type");
		}
	}

	if(again && mIncrementalRebuild && (mBuf0.size() || mBuf1.size()))
		again = mBucketPruner.overlap(queryVolume, pcb);

	return again;
}

PxAgain AABBPruner::sweep(const ShapeData& queryVolume, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback& pcb) const
{
	PX_ASSERT(!mUncommittedChanges);

	PxAgain again = true;

	if(mAABBTree)
	{
		const PxBounds3& aabb = queryVolume.getPrunerInflatedWorldAABB();
		PxVec3 extents = aabb.getExtents();
		again = AABBTreeRaycast<true>()(mPool.getObjects(), *mAABBTree, aabb.getCenter(), unitDir, inOutDistance, extents, pcb);
	}

	if(again && mIncrementalRebuild && (mBuf0.size() || mBuf1.size()))
		again = mBucketPruner.sweep(queryVolume, unitDir, inOutDistance, pcb);

	return again;
}

PxAgain AABBPruner::raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback& pcb) const
{
	PX_ASSERT(!mUncommittedChanges);

	PxAgain again = true;

	if(mAABBTree)
		again = AABBTreeRaycast<false>()(mPool.getObjects(), *mAABBTree, origin, unitDir, inOutDistance, PxVec3(0.0f), pcb);
		
	if(again && mIncrementalRebuild && (mBuf0.size() || mBuf1.size()))
		again = mBucketPruner.raycast(origin, unitDir, inOutDistance, pcb);

	return again;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Other methods of Pruner Interface
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This isn't part of the pruner virtual interface, but it is part of the public interface
// of AABBPruner - it gets called by SqManager to force a rebuild, and requires a commit() before 
// queries can take place

void AABBPruner::purge()
{
	release();
	mUncommittedChanges = true; // this ensures a commit() must happen before any query
} 

void AABBPruner::setRebuildRateHint(PxU32 nbStepsForRebuild) 
{ 
	PX_ASSERT(nbStepsForRebuild > 3);
	mRebuildRateHint = (nbStepsForRebuild-3); // looks like a magic number to account for the rebuild pipeline latency
	mAdaptiveRebuildTerm = 0; 
}

// Commit either performs a refit if background rebuild is not yet finished
// or swaps the current tree for the second tree rebuilt in the background
void AABBPruner::commit()
{
	if(!mUncommittedChanges)
		// Q: seems like this is both for refit and finalization so is this is correct?
		// i.e. in a situation when we started rebuilding a tree and didn't add anything since
		// who is going to set mUncommittedChanges to true?
		// A: it's set in buildStep at final stage, so that finalization is forced.
		// Seems a bit difficult to follow and verify correctness.
		return;

	mUncommittedChanges = false;

	if(!mAABBTree || !mIncrementalRebuild)
	{
#ifdef PX_CHECKED
		if(!mIncrementalRebuild && mAABBTree)
			Ps::getFoundation().error(PxErrorCode::ePERF_WARNING, __FILE__, __LINE__, "SceneQuery static AABB Tree rebuilt, because a shape attached to a static actor was added, removed or moved, and PxSceneDesc::staticStructure is set to eSTATIC_AABB_TREE.");
#endif
		fullRebuildAABBTree();
		return;
	}

	// Note: it is not safe to call AABBPruner::build() here
	// because the first thread will perform one step of the incremental update,
	// continue raycasting, while the second thread performs the next step in
	// the incremental update

	// Calling Refit() below is safe. It will call 
	// StaticPruner::build() when necessary. Both will early
	// exit if the tree is already up to date, if it is not already, then we 
	// must be the first thread performing raycasts on a dirty tree and other 
	// scene query threads will be locked out by the write lock in 
	// SceneQueryManager::flushUpdates()


	if (mProgress != BUILD_FINISHED)	
	{
		// Calling refit because the second tree is not ready to be swapped in (mProgress != BUILD_FINISHED)
		// Generally speaking as long as things keep moving the second build will never catch up with true state
		refitUpdatedAndRemoved();
	}
	else
	{
		// Finalize
		PX_FREE_AND_RESET(mCachedBoxes);
		mProgress = BUILD_NOT_STARTED; // reset the build state to initial

		// Adjust adaptive term to get closer to specified rebuild rate.
		// perform an even division correction to make sure the rebuild rate adds up
		if (mNbCalls > mRebuildRateHint)
			mAdaptiveRebuildTerm++;
		else if (mNbCalls < mRebuildRateHint)
			mAdaptiveRebuildTerm--;

		// Switch trees
		PX_DELETE(mAABBTree); // delete the old tree
		#ifdef PX_DEBUG
		mNewTree->validate();
		#endif
		mAABBTree = mNewTree; // set current tree to progressively rebuilt tree
		mNewTree = NULL; // clear out the progressively rebuild tree pointer

		// rebuild the tree map to match the current (newly built) tree
		mTreeMap.initMap(PxMax(mPool.getNbActiveObjects(),mNbCachedBoxes),*mAABBTree);

		// The new mapping has been computed using only indices stored in the new tree. Those indices map the pruning pool
		// we had when starting to build the tree. We need to re-apply recorded moves to fix the tree that finished rebuilding.
		// AP: the problem here is while we are rebuilding the tree there are ongoing modifications to the current tree
		// but the background build has a cached copy of all the AABBs at the time it was started
		// (and will produce indices referencing those)
		// Things that can happen in the meantime: update, remove, add, commit
		for(NewTreeFixup* r = mNewTreeFixups.begin(); r < mNewTreeFixups.end(); r++)
			mTreeMap.invalidate(r->removedIndex, r->relocatedLastIndex, *mAABBTree);
		mNewTreeFixups.clear(); // clear out the fixups since we just applied them all
		mDoSaveFixups = false; // stop saving the fixups until we restart the build again


		// We need to refit the new tree because objects may have moved while we were building it.
		// So this is a fixup for AABBs in addition to mNewTreeFixups
		// We perform a full refit here which can be improved from a perf standpoint (use refitMarked)
		PxU32 NbObjects = mPool.getNbActiveObjects();

		AABBTreeBuilder TB;
		TB.mNbPrimitives	= NbObjects;
		TB.mAABBArray		= mPool.getCurrentWorldBoxes();
		TB.mSettings.mRules	= SPLIT_SPLATTER_POINTS;
		TB.mSettings.mLimit	= 1;
		((AABBTree*)getAABBTree())->refit2(&TB, (PxU32*)getAABBTree()->getIndices());

		// mToRemoveFromBucket contains the list of PrunerHandles added
		// back from when current tree finished rebuilding up to the start of ongoing tree rebuild
		// so bucket pruner has to contain objects that are in the recently added, and tree being rebuilt but not in the current tree
		// and we need to remove objects promoted to current tree from the tree that was being rebuilt
		for(PrunerHandleSet::Iterator it1 = mToRemoveFromBucket->getIterator(); !it1.done(); ++it1)
		{
			PrunerHandle p = *it1;
			mBucketPruner.removeObject(mPool.getPayload(p));
		}

		{
			// if inside commit we have objects in mAddedObjects we have to rebuild the tree
			// mAddedObjects was used to accumulate the saved state while we were rebuilding
			// so here we are, done with rebuilding, just swapped the tree and now set the flag
			// to true if there were any added objects
			mNeedsNewTree = mAddedObjects->size()>0;

			// clear out the second buffer since we don't need it anymore (only used to sync up bucketpruner state)
			// and we need space after we swap the pairs
			mToRemoveFromBucket->clear();
			
			// we could also swap mAddedObjects and mToRemoveFromBucket right here
			// but we do it in buildStep, BUILD_NOT_STARTED stage
		}
	}

	updateBucketPruner();
}


void AABBPruner::shiftOrigin(const PxVec3& shift)
{
	mPool.shiftOrigin(shift);

	if(mAABBTree)
		mAABBTree->shiftOrigin(shift);

	if(mIncrementalRebuild)
		mBucketPruner.shiftOrigin(shift);

	// currently there is no need to shift the build-in-progress tree because when we switch to that tree, it gets
	// refit anyway. And that tree is built on the basis of cached bounds which do not change either.
	// see mCachedBoxes, mNewTree
}

#ifndef __SPU__
#include "CmRenderOutput.h"
void AABBPruner::visualize(Cm::RenderOutput& out, PxU32 color) const
{
	// getAABBTree() asserts when pruner is dirty. NpScene::visualization() does not enforce flushUpdate. see DE7834
	const AABBTree* tree = mAABBTree;

	if(tree)
	{
		struct Local
		{
			static void _Draw(const AABBTreeNode* root, const AABBTreeNode* node, Cm::RenderOutput& out_)
			{
				out_ << Cm::DebugBox(PxBounds3::centerExtents(node->getAABBCenter(), node->getAABBExtents()), true);
				if (node->isLeaf())
					return;
				_Draw(root, node->getPos(root), out_);
				_Draw(root, node->getNeg(root), out_);
			}
		};
		out << PxTransform(PxIdentity);
		out << color;
		Local::_Draw(tree->getNodes(), tree->getNodes(), out);
	}

	// Render added objects not yet in the tree
	out << PxTransform(PxIdentity);
	out << PxU32(PxDebugColor::eARGB_WHITE);

	for(PrunerHandleSet::Iterator it0 = const_cast<AABBPruner*>(this)->mBuf0.getIterator(); !it0.done(); ++it0)
	{
		PrunerHandle p = *it0;
		out << Cm::DebugBox(mPool.getWorldAABB(p), true);
	}
	
	for(PrunerHandleSet::Iterator it1 = const_cast<AABBPruner*>(this)->mBuf1.getIterator(); !it1.done(); ++it1)
	{
		PrunerHandle p = *it1;
		out << Cm::DebugBox(mPool.getWorldAABB(p), true);		
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Internal methods
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool AABBPruner::buildStep()
{
	PX_ASSERT(mIncrementalRebuild);
	if(mNeedsNewTree)
	{
		if(mProgress==BUILD_NOT_STARTED)
		{
			const PxU32 nbObjects = mPool.getNbActiveObjects();
			if(!nbObjects)
				return true;

			PX_DELETE(mNewTree);
			mNewTree = PX_NEW(AABBTree);

			mNbCachedBoxes = nbObjects;
			mCachedBoxes = (PxBounds3*)PX_ALLOC(sizeof(PxBounds3)*nbObjects, PX_DEBUG_EXP("PxBound3"));

			PxBounds3* aabbs = mPool.getCurrentWorldBoxes();
			for(PxU32 i=0;i<nbObjects;i++)
				mCachedBoxes[i] = aabbs[i];

			// Objects currently in mAddedObjects will be in the new tree. However more objects can
			// get added while we compute the new tree, and those ones will not be part of it.
			// We need to save a list of previously added objects so we can remove them from the bucket pruner
			// since they will be consequently incorporated into the current tree to avoid duplication
			// Also see comments in the header for mAddedObjects, mToRemoveFromBucket
			Ps::swap(mAddedObjects, mToRemoveFromBucket);

			mBuilder.reset();
			mBuilder.mNbPrimitives		= mNbCachedBoxes;
			mBuilder.mAABBArray			= mCachedBoxes;
			mBuilder.mSettings.mRules	= SPLIT_SPLATTER_POINTS;
			mBuilder.mSettings.mLimit	= 1;

			// start recording modifications to the tree made during rebuild to reapply (fix the new tree) eventually
			mDoSaveFixups = true;
			PX_ASSERT(mNewTreeFixups.size()==0);

			mProgress = BUILD_INIT;
		}
		else if(mProgress==BUILD_INIT)
		{
			mNewTree->progressiveBuild(&mBuilder, 0, 0);
			mProgress = BUILD_IN_PROGRESS;
			mNbCalls = 0;

			// Use a heuristic to estimate the number of work units needed for rebuilding the tree.
			// The general idea is to use the number of work units of the previous tree to build the new tree.
			// This works fine as long as the number of leaves remains more or less the same for the old and the
			// new tree. If that is not the case, this estimate can be way off and the work units per step will
			// be either much too small or too large. Hence, in that case we will try to estimate the number of work
			// units based on the number of leaves of the new tree as follows:
 			//
			// - Assume new tree with n leaves is perfectly-balanced
			// - Compute the depth of perfectly-balanced tree with n leaves
			// - Estimate number of working units for the new tree

			PxU32 depth = Ps::ilog2(mBuilder.mNbPrimitives);	// Note: This is the depth without counting the leaf layer
			PxU32 estimatedNbWorkUnits = depth * mBuilder.mNbPrimitives;	// Estimated number of work units for new tree
			PxU32 estimatedNbWorkUnitsOld = mAABBTree->getTotalPrims();
			if ((estimatedNbWorkUnits <= (estimatedNbWorkUnitsOld << 1)) && (estimatedNbWorkUnits >= (estimatedNbWorkUnitsOld >> 1)))
				// The two estimates do not differ by more than a factor 2
				mTotalWorkUnits = estimatedNbWorkUnitsOld;
 			else
			{
 				mAdaptiveRebuildTerm = 0;
				mTotalWorkUnits = estimatedNbWorkUnits;
 			}
 
 			PxI32 totalWorkUnits = PxI32(mTotalWorkUnits + (mAdaptiveRebuildTerm * mBuilder.mNbPrimitives));
 			mTotalWorkUnits = (PxU32)PxMax(totalWorkUnits, 0);
		}
		else if(mProgress==BUILD_IN_PROGRESS)
		{
			mNbCalls++;
			const PxU32 Limit = 1 + (mTotalWorkUnits / mRebuildRateHint);
			// looks like progressiveRebuild returns 0 when finished
			if (!mNewTree->progressiveBuild(&mBuilder, 1, Limit))
			{
				// Done
				mProgress = BUILD_FINISHED;
				#ifdef PX_DEBUG
				mNewTree->validate();
				#endif
			}
		}

		// This is required to be set because commit handles both refit and a portion of build finalization (why?)
		// This is overly conservative also only necessary in case there were no updates at all to the tree since the last tree swap
		// It also overly conservative in a sense that it could be set only if mProgress was just set to BUILD_FINISHED
		mUncommittedChanges = true;

		return mProgress==BUILD_FINISHED;
	}

	return true;
}



// ---------------------- internal methods


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds an AABB-tree for objects in the pruning pool.
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBPruner::fullRebuildAABBTree()
{
#ifndef __SPU__
	// Release possibly already existing tree
	PX_DELETE_AND_RESET(mAABBTree);

	// Don't bother building an AABB-tree if there isn't a single static object
	const PxU32 nbObjects = mPool.getNbActiveObjects();
	if(!nbObjects)
		return true;

	bool Status;
	{
		// Create a new tree
		mAABBTree = PX_NEW(AABBTree);

		AABBTreeBuilder TB;
		TB.mNbPrimitives	= nbObjects;
		TB.mAABBArray		= mPool.getCurrentWorldBoxes();
		TB.mSettings.mRules	= SPLIT_SPLATTER_POINTS;
		TB.mSettings.mLimit	= 1; // 1 pool object per tree node
		Status = mAABBTree->build(&TB);
	}

	// No need for the tree map for static pruner
	if(mIncrementalRebuild)
		mTreeMap.initMap(PxMax(nbObjects,mNbCachedBoxes),*mAABBTree);

	return Status;
#else
	return true;
#endif
}

// called in the end of commit(), but only if mIncrementalRebuild is true
void AABBPruner::updateBucketPruner()
{
	PX_ASSERT(mIncrementalRebuild);
	mBucketPruner.build();
}

#ifndef __SPU__

void AABBPruner::release() // this can be called from purge()
{
	
	mBucketPruner.release();

	mBuf0.clear();
	mBuf1.clear();
	mAddedObjects = &mBuf0;
	mToRemoveFromBucket = &mBuf1;

	mTreeMap.release();

	PX_FREE_AND_RESET(mCachedBoxes);
	mBuilder.reset();
	PX_DELETE_AND_RESET(mNewTree);
	PX_DELETE_AND_RESET(mAABBTree);

	mNbCachedBoxes = 0;
	mProgress = BUILD_NOT_STARTED;
	mNewTreeFixups.clear();
	mDoSaveFixups = false;
	mUncommittedChanges = false;
}

// Refit current tree
void AABBPruner::refitUpdatedAndRemoved()
{
	PX_ASSERT(mIncrementalRebuild);
	AABBTree* Tree = getAABBTree();
	if(!Tree)
		return;

#ifdef PX_DEBUG
	Tree->validate();
#endif

	//### missing a way to skip work if not needed

	PxU32 nbObjects = mPool.getNbActiveObjects();
	// At this point there still can be objects in the tree that are blanked out so it's an optimization shortcut (not required)
	if(!nbObjects)
		return;

	Tree->refitMarked(nbObjects, mPool.getCurrentWorldBoxes(), Tree->getIndices());
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


