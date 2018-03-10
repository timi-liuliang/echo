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
#include "PsVecMath.h"
#include "PsUserAllocated.h"
#include "PsMathUtils.h"
#include "GuContainer.h"
#include "SqAABBTree.h"
#include "SqTreeBuilders.h"
#ifdef SUPPORT_UPDATE_ARRAY
#include "./GuRevisitedRadix.h"
#endif

#ifdef PX_WIIU  
#pragma ghs nowarning 1656 //within a function using alloca or VLAs, alignment of local variables
#endif

using namespace physx;
using namespace physx::shdfnd::aos;
using namespace Gu;
using namespace Sq;

static PX_FORCE_INLINE PxU32 BitsToDwords(PxU32 nb_bits)
{
	return (nb_bits>>5) + ((nb_bits&31) ? 1 : 0);
}

bool Sq::BitArray::init(PxU32 nb_bits)
{
	mSize = BitsToDwords(nb_bits);
	// Get ram for n bits
	PX_FREE(mBits);
	mBits = (PxU32*)PX_ALLOC(sizeof(PxU32)*mSize, PX_DEBUG_EXP("BitArray::mBits"));
	// Set all bits to 0
	clearAll();
	return true;
}

#ifdef SUPPORT_PROGRESSIVE_BUILDING
class Sq::FIFOStack2 : public Ps::UserAllocated
{
	public:
									FIFOStack2() : mStack(PX_DEBUG_EXP("SQFIFOStack2")), mCurIndex(0) {}
									~FIFOStack2() {}

	PX_FORCE_INLINE	PxU32			getNbEntries() const { return mStack.size(); }
	PX_FORCE_INLINE	void			push(AABBTreeNode* entry, AABBTreeNode* parent)	{ mStack.pushBack(NodeAndParent(entry, parent)); }
					bool			pop(AABBTreeNode*& entry, AABBTreeNode*& parent);
	private:
		struct NodeAndParent
		{
			NodeAndParent(AABBTreeNode* aNode, AABBTreeNode* aParent) : node(aNode), parent(aParent) {}
			AABBTreeNode* node;
			AABBTreeNode* parent;
		};
		Ps::Array<NodeAndParent>	mStack;
		PxU32						mCurIndex;			//!< Current index within the container
};

bool Sq::FIFOStack2::pop(AABBTreeNode*& entry, AABBTreeNode*& parent)
{
	PxU32 NbEntries = mStack.size(); // Get current number of entries
	if(!NbEntries)
		return false; // Can be NULL when no value has been pushed. This is an invalid pop call.
	entry = mStack[mCurIndex].node; // Get oldest entry, move to next one
	parent = mStack[mCurIndex++].parent; // Get oldest entry, move to next one
	if(mCurIndex==NbEntries)
	{
		// All values have been poped
		mStack.clear();
		mCurIndex=0;
	}
	return true;
}
#endif // #ifdef SUPPORT_PROGRESSIVE_BUILDING

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Splits the node along a given axis.
 *	The list of indices is reorganized according to the split values.
 *	\param		axis		[in] splitting axis index
 *	\param		builder		[in] the tree builder
 *	\return		the number of primitives assigned to the first child
 *	\warning	this method reorganizes the internal list of primitives
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PxU32 AABBTreeNode::split(const PxBounds3& exactBounds, PxU32 axis, AABBTreeBuilder* builder, PxU32* indices)
{
	PX_ASSERT(builder->mNodeBase);

	// Get node split value
	const float SplitValue = builder->getSplittingValue(exactBounds, axis);

	PxU32 NbPos = 0;
	// Loop through all node-related primitives. Their indices range from getNodePrimitives()[0] to getNodePrimitives()[mNbBuildPrimitives-1].
	// Those indices map the global list in the tree builder.
	for(PxU32 i=0;i<getNbBuildPrimitives();i++)
	{
		// Get index in global list
		const PxU32 Index = indices[getNodePrimitives()+i];

		// Test against the splitting value. The primitive value is tested against the enclosing-box center.
		// [We only need an approximate partition of the enclosing box here.]
		const float PrimitiveValue = builder->getSplittingValue(Index, axis);

		// Reorganize the list of indices in this order: positive - negative.
		if(PrimitiveValue > SplitValue)
		{
			// Swap entries
			const PxU32 Tmp = indices[getNodePrimitives()+i];
			indices[getNodePrimitives()+i] = indices[getNodePrimitives()+NbPos];
			indices[getNodePrimitives()+NbPos] = Tmp;
			// Count primitives assigned to positive space
			NbPos++;
		}
	}
	return NbPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Subdivides the node.
 *	
 *	          N
 *	        /   \
 *	      /       \
 *	   N/2         N/2
 *	  /   \       /   \
 *	N/4   N/4   N/4   N/4
 *	(etc)
 *
 *	A well-balanced tree should have a O(log n) depth.
 *	A degenerate tree would have a O(n) depth.
 *	Note a perfectly-balanced tree is not well-suited to collision detection anyway.
 *
 *	\param		builder		[in] the tree builder
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTreeNode::subdivide(const PxBounds3& exactBounds, AABBTreeBuilder* builder, PxU32* indices)
{
	PX_ASSERT(builder);

	// Stop subdividing if we reach a leaf node. This is always performed here,
	// else we could end in trouble if user overrides this.
	if(getNbBuildPrimitives()==1)
	{
		setNbRunTimePrimitives(getNbBuildPrimitives());
		return true;
	}

	// Let the user validate the subdivision
	if(!builder->validateSubdivision(getNbBuildPrimitives()))
	{
		setNbRunTimePrimitives(getNbBuildPrimitives());
		return true;
	}

	bool ValidSplit = true;	// Optimism...
	PxU32 NbPos=0;
	if(builder->mSettings.mRules & SPLIT_LARGEST_AXIS)
	{
		// Find the largest axis to split along
		PxVec3 Extents = exactBounds.getExtents(); // Box extents
		PxU32 Axis	= Ps::largestAxis(Extents); // Index of largest axis

		// Split along the axis
		NbPos = split(exactBounds, Axis, builder, indices);

		// Check split validity
		if(!NbPos || NbPos==getNbBuildPrimitives())	ValidSplit = false;
	}
	else if(builder->mSettings.mRules & SPLIT_SPLATTER_POINTS)
	{
		// Compute the means
		PxU32 numPrims = getNbBuildPrimitives();
		Vec3V nbBuildSplat = V3Load(float(numPrims));
		Vec3V Means = Vec3V_From_FloatV(FZero());
		const PxU32* Prims = indices+getNodePrimitives();
		const PxU32* Last = indices+getNodePrimitives() + numPrims;
		while(Prims<Last)
		{
			PxU32 Index = *Prims++;
			Vec3V Tmp = builder->getSplittingValues(Index);
			Means = V3Add(Means, Tmp);
		}
		Means = V3Mul(Means, V3Recip(nbBuildSplat)); // Means/=float(getNbBuildPrimitives());

		// Compute variances
		Vec3V Vars = Vec3V_From_FloatV(FZero());
		Prims = indices+getNodePrimitives();
		while(Prims!=Last)
		{
			const PxU32 Index = *Prims++;
			Vec3V C = builder->getSplittingValues(Index);
			Vec3V cm = V3Sub(C, Means);
			Vars = V3Add(Vars, V3Mul(cm, cm)); //Vars.x += (C.x - Means.x)*(C.x - Means.x);
		}
		Vars = V3Mul(Vars, V3Recip(V3Sub(nbBuildSplat, Vec3V_From_FloatV(FOne()))));//Vars/=float(getNbBuildPrimitives()-1);

		// Choose axis with greatest variance
		PxVec3 Vars3;
		V3StoreU(Vars, Vars3);
		PxU32 Axis = Ps::largestAxis(Vars3);

		// Split along the axis
		NbPos = split(exactBounds, Axis, builder, indices);

		// Check split validity
		if(!NbPos || NbPos==numPrims)
			ValidSplit = false;
	}
	else if(builder->mSettings.mRules & SPLIT_BALANCED)
	{
		// Test 3 axis, take the best
		float Results[3];
		NbPos = split(exactBounds, 0, builder, indices);	Results[0] = float(NbPos)/float(getNbBuildPrimitives());
		NbPos = split(exactBounds, 1, builder, indices);	Results[1] = float(NbPos)/float(getNbBuildPrimitives());
		NbPos = split(exactBounds, 2, builder, indices);	Results[2] = float(NbPos)/float(getNbBuildPrimitives());
		Results[0]-=0.5f;	Results[0]*=Results[0];
		Results[1]-=0.5f;	Results[1]*=Results[1];
		Results[2]-=0.5f;	Results[2]*=Results[2];
		PxU32 Min=0;
		if(Results[1]<Results[Min])
			Min = 1;
		if(Results[2]<Results[Min])
			Min = 2;
		
		// Split along the axis
		NbPos = split(exactBounds, Min, builder, indices);

		// Check split validity
		if(!NbPos || NbPos==getNbBuildPrimitives())	ValidSplit = false;
	}
	else if(builder->mSettings.mRules & SPLIT_BEST_AXIS)
	{
		// Test largest, then middle, then smallest axis...

		// Sort axis
		PxVec3 Extents = exactBounds.getExtents();	// Box extents
		PxU32 SortedAxis[] = { 0, 1, 2 };
		float* Keys = (float*)&Extents.x;
		for(PxU32 j=0;j<3;j++)
		{
			for(PxU32 i=0;i<2;i++)
			{
				if(Keys[SortedAxis[i]]<Keys[SortedAxis[i+1]])
				{
					PxU32 Tmp = SortedAxis[i];
					SortedAxis[i] = SortedAxis[i+1];
					SortedAxis[i+1] = Tmp;
				}
			}
		}

		// Find the largest axis to split along
		PxU32 CurAxis = 0;
		ValidSplit = false;
		while(!ValidSplit && CurAxis!=3)
		{
			NbPos = split(exactBounds, SortedAxis[CurAxis], builder, indices);
			// Check the subdivision has been successful
			if(!NbPos || NbPos==getNbBuildPrimitives())
				CurAxis++;
			else
				ValidSplit = true;
		}
	}
	else if(builder->mSettings.mRules & SPLIT_FIFTY)
	{
		// Don't even bother splitting (mainly a performance test)
		NbPos = getNbBuildPrimitives()>>1;
	}
	else
	{
		PX_ALWAYS_ASSERT_MESSAGE("Unknown split rule - number of primitives can be clipped.");
		setNbRunTimePrimitives(getNbBuildPrimitives());
		return false;
	}

	// Check the subdivision has been successful
	if(!ValidSplit)
	{
		// Here, all boxes lie in the same sub-space. Two strategies:
		// - if we are over the split limit, make an arbitrary 50-50 split
		// - else stop subdividing
		setNbRunTimePrimitives(getNbBuildPrimitives());
		if(getNbBuildPrimitives()>builder->mSettings.mLimit)
		{
			builder->increaseNbInvalidSplits();
			NbPos = getNbBuildPrimitives()>>1;
		}
		else
		{
			PX_ASSERT(builder->mSettings.mLimit < PxU32(1<<NBRUNTIME_BITS));
			return true;
		}
	}

	// Now create children and assign their pointers.
	// We use a pre-allocated linear pool for complete trees [Opcode 1.3]
	AABBTreeNode* base = builder->mNodeBase;
	PX_ASSERT_WITH_MESSAGE(base ,"Only node pool-relative AABB trees are supported.");
	PxU32 savePrimitives = getNodePrimitives(); // save the primitives ptr since it will be overwritten by getNodePrimitives() (stored in a union)
	clearLeaf(); // clear leaf flag since it's no longer a leaf
	setPos(builder->getCount()); // set pointer to two adjacent nodes

	// Update stats
	builder->increaseCount(2);

	// Assign children
	AABBTreeNode* Pos = (AABBTreeNode*)getPos(base);
	AABBTreeNode* Neg = (AABBTreeNode*)getNeg(base);
	Pos->setPosOrNodePrimitives(savePrimitives);
	Pos->setNbBuildPrimitivesOrParent(NbPos);
	Pos->setNbRunTimePrimitives(Pos->getNbBuildPrimitives()); // can be clipped but it's ok here
	Pos->setLeaf(); // mark as leaf for now, may be cleared during further subdivision
	Neg->setPosOrNodePrimitives(savePrimitives+NbPos);
	Neg->setNbBuildPrimitivesOrParent(getNbBuildPrimitives()-NbPos);
	Neg->setNbRunTimePrimitives(Neg->getNbBuildPrimitives()); // can be clipped but it's ok here
	Neg->setLeaf(); // mark as leaf for now, may be cleared during further subdivision
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Recursive hierarchy building in a top-down fashion.
 *	\param		builder		[in] the tree builder
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static PxU32 Count=0;

// here parentCenter, parentExtents are conservative bounds (decompressed so far)
void AABBTreeNode::_buildHierarchy(AABBTreeBuilder* builder, PxU32* indices)
{
	Count++;

	// 1) Compute the exact global box for current node
	PxBounds3 exactBounds;
	Vec3V bMin, bMax;
	builder->computeGlobalBox(indices+getNodePrimitives(), getNbBuildPrimitives(), exactBounds, &bMin, &bMax);

	// 2) Subdivide current node
	setLeaf();
	subdivide(exactBounds, builder, indices); // subdivide will clear the leaf flag in case the node was split

	compress<1>(bMin, bMax);

	AABBTreeNode* base = builder->mNodeBase;
	// 3) Recurse
	if (!isLeaf())
	{
		AABBTreeNode* Pos = (AABBTreeNode*)getPos(base);
		AABBTreeNode* Neg = (AABBTreeNode*)getNeg(base);
		PX_ASSERT(this >= base && (this-base)<1024*1024); // sanity check
		PX_ASSERT(Pos > this && Neg > this);
		if(Pos != base)
		{
			Pos->_buildHierarchy(builder, indices);
			Pos->setParent(PxU32(this-base)); // has to be done after _BuildHierarchy since getNbBuildPrimitives() is in a union with mParent
		}
		if(Neg != base)
		{
			Neg->_buildHierarchy(builder, indices);
			Neg->setParent(PxU32(this-base)); // has to be done after _BuildHierarchy since getNbBuildPrimitives() is in a union with mParent
		}
	}

	builder->mTotalPrims += getNbBuildPrimitives();
	setParent(PxU32(this-base)); // overwrites getNbBuildPrimitives() which we should not be using anymore
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTree::AABBTree() : mIndices(NULL), mPool(NULL), mRefitHighestSetWord(0), mTotalNbNodes(0), mTotalPrims(0)
{
#ifdef SUPPORT_PROGRESSIVE_BUILDING
	mStack = NULL;
#endif
#ifdef SUPPORT_UPDATE_ARRAY
	mNbRefitNodes	= 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AABBTree::~AABBTree()
{
	release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Releases the tree.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AABBTree::release()
{
#ifdef SUPPORT_PROGRESSIVE_BUILDING
	PX_DELETE_AND_RESET(mStack);
#endif
	PX_DELETE_ARRAY(mPool); 
	PX_FREE_AND_RESET(mIndices);
	mRefitBitmask.clearAll();
	mRefitHighestSetWord = 0;
#ifdef SUPPORT_UPDATE_ARRAY
	mNbRefitNodes	= 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Builds a generic AABB tree from a tree builder.
 *	\param		builder		[in] the tree builder
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AABBTree::build(AABBTreeBuilder* builder)
{
	//gBuildCalls++;
	// Checkings
	if(!builder || !builder->mNbPrimitives)
		return false;

	// Release previous tree
	release();

	// Init stats
	builder->setCount(1);
	builder->setNbInvalidSplits(0);

	// Initialize indices. This list will be modified during build.
	mIndices = (PxU32*)PX_ALLOC(sizeof(PxU32)*builder->mNbPrimitives, PX_DEBUG_EXP("AABB tree indices"));
	// Identity permutation
	for(PxU32 i=0;i<builder->mNbPrimitives;i++)
		mIndices[i] = i;

	// Allocate a pool of nodes
	mPool = PX_NEW(AABBTreeNode)[builder->mNbPrimitives*2 - 1];
	builder->mNodeBase = mPool;

	// Setup initial node. Here we have a complete permutation of the app's primitives.
	mPool->setPosOrNodePrimitives(0);
	mPool->setNbBuildPrimitivesOrParent(builder->mNbPrimitives);

	// Build the hierarchy
	Count = 0;
	mPool->setLeaf();
	mPool->_buildHierarchy(builder, mIndices);

	// Get back total number of nodes
	mTotalNbNodes	= builder->getCount();
	mTotalPrims		= builder->mTotalPrims;

	// For complete trees, check the correct number of nodes has been created [Opcode 1.3]
	if(mPool && builder->mSettings.mLimit==1)
		PX_ASSERT(mTotalNbNodes==builder->mNbPrimitives*2 - 1);

#ifdef PX_DEBUG
	validate();
#endif

	return true;
}

#ifdef SUPPORT_PROGRESSIVE_BUILDING
static PxU32 incrementalBuildHierarchy(FIFOStack2& stack, AABBTreeNode* node, AABBTreeNode* parent, AABBTreeBuilder* builder, PxU32* indices)
{
	//gProgressiveBuildCalls ++;
	// 1) Compute the global box for current node. The box is stored in mBV.
	PX_ASSERT(node->isLeaf());
	PxBounds3 exactBounds;
	Vec3V bMin, bMax;
	builder->computeGlobalBox(node->getPrimitives(indices), node->getNbBuildPrimitives(), exactBounds, &bMin, &bMax);

	// 2) Subdivide current node
	node->setLeaf();
	node->subdivide(exactBounds, builder, indices);

	node->compress<1>(bMin, bMax);

	AABBTreeNode* base = builder->mNodeBase;
	// 3) Recurse
	if (!node->isLeaf())
	{
		AABBTreeNode* Pos = (AABBTreeNode*)node->getPos(base);
		AABBTreeNode* Neg = (AABBTreeNode*)node->getNeg(base);
		PX_ASSERT(node >= base && (node-base)<1024*1024); // sanity check
		PX_ASSERT(Pos > node && Neg > node);
		// need to push parent along with node ptr
		// since we can't set mParent here since it shares memory with, getNbBuildPrimitives(), can only patch later
		if(Pos != base)
			stack.push(Pos, node);
		if(Neg != base)
			stack.push(Neg, node);
	}

	PxU32 saveBuildPrims = node->getNbBuildPrimitives();
	builder->mTotalPrims += saveBuildPrims;
	// current node is finalized for build purposes (not on the stack) -> set the parent pointer
	node->setParent(PxU32(parent-base)); // use parent from the stack, overwrite getNbBuildPrimitives() which we should not be using anymore
	return saveBuildPrims;
}

PxU32 AABBTree::progressiveBuild(AABBTreeBuilder* builder, PxU32 progress, PxU32 limit)
{
	if(progress==0)
	{
		// Checkings
		if(!builder || !builder->mNbPrimitives)
			return PX_INVALID_U32;

		// Release previous tree
		release();

		// Init stats
		builder->setCount(1);
		builder->setNbInvalidSplits(0);

		// Initialize indices. This list will be modified during build.
		mIndices = (PxU32*)PX_ALLOC(sizeof(PxU32)*builder->mNbPrimitives, PX_DEBUG_EXP("AABB tree indices"));
		// Identity permutation
		for(PxU32 i=0;i<builder->mNbPrimitives;i++)
			mIndices[i] = i;

		// Use a linear array for complete trees (since we can predict the final number of nodes) [Opcode 1.3]
		// Allocate a pool of nodes
		mPool = PX_NEW(AABBTreeNode)[builder->mNbPrimitives*2 - 1];
		builder->mNodeBase = mPool;

		// Setup initial node. Here we have a complete permutation of the app's primitives.
		mPool->setPosOrNodePrimitives(0);
		mPool->setNbBuildPrimitivesOrParent(builder->mNbPrimitives);
		mPool->setLeaf();

		mStack = PX_NEW(FIFOStack2);
		AABBTreeNode* FirstNode = mPool;
		mStack->push(FirstNode, FirstNode);
		return progress++;
	}
	else if(progress==1)
	{
		PxU32 stackCount = mStack->getNbEntries();
		if(stackCount)
		{
			PxU32 Total = 0;
			const PxU32 Limit = limit;
			while(Total<Limit)
			{
				AABBTreeNode* Entry, *parent;
				if(mStack->pop(Entry, parent))
					Total += incrementalBuildHierarchy(*mStack, Entry, parent, builder, mIndices);
				else
					break;
			}
			return progress;
		}

		// Get back total number of nodes
		mTotalNbNodes	= builder->getCount();
		mTotalPrims		= builder->mTotalPrims;

		// For complete trees, check the correct number of nodes has been created [Opcode 1.3]
		if(mPool && builder->mSettings.mLimit==1)	PX_ASSERT(mTotalNbNodes==builder->mNbPrimitives*2 - 1);

		PX_DELETE_AND_RESET(mStack);

		return 0;	// Done!
	}
	return PX_INVALID_U32;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes the depth of the tree.
 *	A well-balanced tree should have a log(n) depth. A degenerate tree O(n) depth.
 *	\return		depth of the tree
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PxU32 AABBTree::computeDepth() const
{
	return walk(NULL, NULL);	// Use the walking code without callback
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Walks the tree, calling the user back for each node.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PxU32 AABBTree::walk(WalkingCallback callback, void* user_data) const
{
	// Call it without callback to compute maximum depth
	PxU32 MaxDepth = 0;
	PxU32 CurrentDepth = 0;

	struct Local
	{
		static void _walk(
			const AABBTreeNode* allNodes, const AABBTreeNode* current_node,
			PxU32& max_depth, PxU32& current_depth, WalkingCallback callback_, void* user_data_)
		{
			// Entering a new node => increase depth
			current_depth++;

			// Keep track of maximum depth
			if(current_depth>max_depth)
				max_depth = current_depth;

			// Callback
			if(callback_ && !(callback_)(current_node, current_depth, user_data_))
				return;

			// Recurse
			if(current_node->getPos(allNodes) != allNodes)
			{
				_walk(allNodes, current_node->getPos(allNodes), max_depth, current_depth, callback_, user_data_);
				current_depth--;
			}
			if(current_node->getNeg(allNodes) != allNodes)
			{
				_walk(allNodes, current_node->getNeg(allNodes), max_depth, current_depth, callback_, user_data_);
				current_depth--;
			}
		}
	};

	Local::_walk(mPool, mPool, MaxDepth, CurrentDepth, callback, user_data);
	return MaxDepth;
}

void AABBTree::walk2(WalkingCallback callback, void* user_data) const
{
	if(!callback)
		return;
	struct Local
	{
		static void _walk(const AABBTreeNode* allNodes, const AABBTreeNode* current_node, WalkingCallback callback_, void* user_data_)
		{
			const AABBTreeNode* P = current_node->getPos(allNodes);
			const AABBTreeNode* N = current_node->getNeg(allNodes);

			if(P != allNodes && !(callback_)(P, 0, user_data_))
				return;
			if(N != allNodes && !(callback_)(N, 0, user_data_))
				return;

			if(P != allNodes)
				_walk(allNodes, P, callback_, user_data_);
			if(N != allNodes)
				_walk(allNodes, N, callback_, user_data_);
		}
	};
	if(!(callback)(getNodes(), 0, user_data))
		return;
	Local::_walk(getNodes(), getNodes(), callback, user_data);
}

static PX_FORCE_INLINE void computeUnionBox(Vec3V* aResultMin, Vec3V* aResultMax, const PxU32* primitives, PxU32 nbPrims, const PxBounds3* boxes)
{
	Vec3V resultMin, resultMax;
	if(!nbPrims)
	{
		// Might happen after a node has been invalidated
		resultMin = V3LoadU(PxVec3(0.25f * PX_AABB_COMPRESSION_MAX));
		resultMax = V3LoadU(PxVec3(-0.25f * PX_AABB_COMPRESSION_MAX));
	}
	else
	{
		resultMin = V3LoadU(boxes[*primitives].minimum);
		resultMax = V3LoadU(boxes[*primitives].maximum);

		if(nbPrims>1)
		{
			const PxU32* Last = primitives + nbPrims;
			primitives++;

			while(primitives!=Last)
			{
				Vec3V mn = V3LoadU(boxes[*primitives].minimum);
				Vec3V mx = V3LoadU(boxes[*primitives++].maximum);
				resultMin = V3Min(resultMin, mn);
				resultMax = V3Max(resultMax, mx);
			}
		}
	}

	*aResultMin = resultMin;
	*aResultMax = resultMax;
}

bool AABBTree::refit2(AABBTreeBuilder* builder, PxU32* indices)
{
	if(!builder)
		return false;

	PX_ASSERT(mPool);

	const PxBounds3* Boxes = builder->mAABBArray;

	// Bottom-up update
	PxU32 Index = mTotalNbNodes;
	while(Index--)
	{
		AABBTreeNode& Current = mPool[Index];
		if(Index)
			Ps::prefetch(mPool + Index - 1);

		if(Current.isLeaf())
		{
			// compute AABB for the bottom node
			Vec3V mn, mx;
			computeUnionBox(&mn, &mx, Current.getPrimitives(indices), Current.getNbRuntimePrimitives(), Boxes);
			Current.compress<1>(mn, mx);
		}
		else
		{
			const AABBTreeNode* pos = Current.getPos(mPool);
			const AABBTreeNode* neg = Current.getNeg(mPool);
			Vec3V negCenter, negExtents, posCenter, posExtents;
			neg->getAABBCenterExtentsV(&negCenter, &negExtents);
			pos->getAABBCenterExtentsV(&posCenter, &posExtents);

			// compute bounds in min/max form around both nodes
			Vec3V mnPN = V3Min(V3Sub(posCenter, posExtents), V3Sub(negCenter, negExtents));
			Vec3V mxPN = V3Max(V3Add(posCenter, posExtents), V3Add(negCenter, negExtents));

			// invoke compression
			Current.compress<1>(mnPN, mxPN);
		}
	}

	return true;
}

#ifndef SUPPORT_REFIT_BITMASK
bool AABBTree::	refit3(PxU32 nb_objects, const PxBounds3* boxes, const Container& indices)
{
	PX_ASSERT(mPool);

	if(1)
	{
//		printf("Size: %d\n", sizeof(AABBTreeNode));
		PxU32 Nb = indices.getNbEntries();
		if(!Nb)	return true;

		PxU32 Index = mTotalNbNodes;

		// ### those flags could be written directly, no need for the indices array
		bool* Flags = (bool*)PxAlloca(Index);
		PxMemZero(Flags, Index);

		const PxU32* in = indices.getEntries();
		while(Nb--)
		{
			PxU32 Index = *in++;
			PX_ASSERT(Index<mTotalNbNodes);
			const AABBTreeNode* Current = mPool + Index;
			for (;;)
			{
				PxU32 CurrentIndex = PxU32(size_t(Current) - size_t(mPool)) / sizeof(AABBTreeNode);
				if(Flags[CurrentIndex])
				{
					// We can early exit if we already visited the node!
					break;
				}
				else
				{
					Flags[CurrentIndex] = true;
					const AABBTreeNode* parent = Current->getParent(mPool);
					if (Current == parent)
						break;
					Current = parent;
				}

			}
		}

		while(Index--)
		{
			if(Flags[Index])
			{
				AABBTreeNode* Current = mPool + Index;
				if(Current->isLeaf())
				{
					// compute AABB for the bottom node
					Vec3V mn, mx;
					computeUnionBox(&mn, &mx, Current->getPrimitives(indices.getEntries()), Current->getNbRuntimePrimitives(), boxes);
					PX_ASSERT(FStore(V3GetX(mx)) >= FStore(V3GetX(mn)));
					Current->compress<1>(mn, mx);
				}
				else
				{
					const AABBTreeNode* pos = Current->getPos(mPool);
					const AABBTreeNode* neg = Current->getNeg(mPool);
					Vec3V negCenter, negExtents, posCenter, posExtents;
					neg->getAABBCenterExtentsV(&negCenter, &negExtents);
					pos->getAABBCenterExtentsV(&posCenter, &posExtents);

					// compute bounds in min/max form around both nodes
					Vec3V mnPN = V3Min(V3Sub(posCenter, posExtents), V3Sub(negCenter, negExtents));
					Vec3V mxPN = V3Max(V3Add(posCenter, posExtents), V3Add(negCenter, negExtents));
					PX_ASSERT(FStore(V3GetX(mxPN)) >= FStore(V3GetX(mnPN)));

					// invoke compression
					Current->compress<1>(mnPN, mxPN);
				}
			}
		}
	}
	return true;
}
#endif

void AABBTree::markForRefit(PxU32 index)
{
	if(!mRefitBitmask.getBits())
		mRefitBitmask.init(mTotalNbNodes);

	PX_ASSERT(index<mTotalNbNodes);

	const AABBTreeNode* Current = mPool + index;
	Ps::prefetch(Current);
	while(1)
	{
		const PxU32 CurrentIndex = PxU32(Current - mPool);
		PX_ASSERT(CurrentIndex<mTotalNbNodes);
		if(mRefitBitmask.isSet(CurrentIndex))
		{
			// We can early exit if we already visited the node!
			return;
		}
		else
		{
			mRefitBitmask.setBit(CurrentIndex);
			const PxU32 currentMarkedWord = CurrentIndex>>5;
			mRefitHighestSetWord = PxMax(mRefitHighestSetWord, currentMarkedWord);
#ifdef SUPPORT_UPDATE_ARRAY
			if(mNbRefitNodes<SUPPORT_UPDATE_ARRAY)
				mRefitArray[mNbRefitNodes]=CurrentIndex;
			mNbRefitNodes++;
#endif
			const AABBTreeNode* parent = Current->getParent(mPool);
			Ps::prefetch(parent);
			PX_ASSERT(parent == mPool || parent < Current);
			if (Current == parent)
				break;
			Current = parent;
		}
	}
}

static PX_FORCE_INLINE void refitNode(	AABBTreeNode* PX_RESTRICT pool, PxU32 index,
										const PxBounds3* PX_RESTRICT boxes, PxU32* PX_RESTRICT indices,
										AABBTreeNode*& todoWriteback, AABBTreeNode*& todoCompress,
										VecU32V& prevScale, Vec3V& prevXYZ,
										Vec3V& bMin, Vec3V& bMax,
										Vec3V& wMin, Vec3V& wMax)
{
	// invoke compression on previous node to avoid LHS
	if(todoWriteback)
	{
		todoWriteback->writeBack(prevScale, prevXYZ);
		todoWriteback = NULL;
	}
	if(todoCompress)
	{
		todoCompress->compress<0>(bMin, bMax, &prevScale, &prevXYZ);
		// need to save the bounds because we could be reading from these in the middle of writeback pipeline
		wMin = bMin; wMax = bMax;
		todoWriteback = todoCompress;
		todoCompress = NULL;
	}

	AABBTreeNode* PX_RESTRICT Current = pool + index;
	if(Current->isLeaf())
	{
		computeUnionBox(&bMin, &bMax, Current->getPrimitives(indices), Current->getNbRuntimePrimitives(), boxes);
		todoCompress = Current;
	}
	else
	{
		Vec3V negCenter, negExtents, posCenter, posExtents;
		Vec3V negMin, negMax, posMin, posMax;

		const AABBTreeNode* pos = Current->getPos(pool);
		// since we are manually staggering writebacks in 2 stages to avoid LHS,
		// we need to make sure we read back the most up-to-date data..
		// since todoCompress is always NULL we don't need to check for it, it's left here for clarity
		//if (pos == todoCompress) { posMin = bMin; posMax = bMax; } else
		if(pos == todoWriteback)
		{
			posMin = wMin; posMax = wMax; // reuse from writeback pipeline
		}
		else
		{
			pos->getAABBCenterExtentsV(&posCenter, &posExtents);
			posMin = V3Sub(posCenter, posExtents);
			posMax = V3Add(posCenter, posExtents);
		}

		// do the same for neg child
		const AABBTreeNode* neg = Current->getNeg(pool);
		//if (neg == todoCompress) { negMin = bMin; negMax = bMax; } else // reuse bMin, bMax from the pipeline
		if(neg == todoWriteback)
		{
			negMin = wMin; negMax = wMax; // reuse wMin, wMax from the pipeline
		}
		else
		{
			neg->getAABBCenterExtentsV(&negCenter, &negExtents);
			negMin = V3Sub(negCenter, negExtents);
			negMax = V3Add(negCenter, negExtents);
		}

		// compute bounds in min/max form around both nodes
		bMin = V3Min(negMin, posMin);
		bMax = V3Max(negMax, posMax);

		todoCompress = Current;
	}
}

void AABBTree::refitMarked(PxU32 nb_objects, const PxBounds3* boxes, PxU32* indices)
{
	PX_UNUSED(nb_objects);

	if(!mRefitBitmask.getBits())
		return;	// No refit needed

	VecU32V prevScale;
	Vec3V prevXYZ;
	AABBTreeNode* todoWriteback = NULL;
	AABBTreeNode* todoCompress = NULL;
	Vec3V bMin = Vec3V_From_FloatV(FZero()), bMax = Vec3V_From_FloatV(FZero()); // only have to initialize due to compiler warning
	Vec3V wMin = Vec3V_From_FloatV(FZero()), wMax = Vec3V_From_FloatV(FZero()); // only have to initialize due to compiler warning

#ifdef SUPPORT_UPDATE_ARRAY
	// we need this macro to make sure compiler doesn't generate a warning (comparison always true)
	// in nbRefitNodes<=SUPPORT_UPDATE_ARRAY
	PxU32 nbRefitNodes = mNbRefitNodes;
	mNbRefitNodes = 0;
	if(nbRefitNodes<=SUPPORT_UPDATE_ARRAY)
	{
		// There are two code paths for sorting, this one is if number of changed nodes is <= SUPPORT_UPDATE_ARRAY
		// the second in else clause is based on bitmask scan
		PxU32* ranks0 = (PxU32*)PxAlloca(nbRefitNodes*sizeof(PxU32));
		PxU32* ranks1 = (PxU32*)PxAlloca(nbRefitNodes*sizeof(PxU32));
		StackRadixSort(rs, ranks0, ranks1);

		// sort by ranks so that refit is done in reverse hierarchy order (since we use reverse indexing later)
		const PxU32* sorted = rs.Sort(mRefitArray, nbRefitNodes).GetRanks();

		for(PxU32 i=0;i<nbRefitNodes;i++)
		{
			const PxU32 Index = mRefitArray[sorted[nbRefitNodes-1-i]]; // reverse sorted order, so children first, parents last

			Ps::prefetch(mPool+Index);

			PX_ASSERT(mRefitBitmask.isSet(Index)); // this was set in markForRefit for node and it's parent chain
			mRefitBitmask.clearBit(Index); // clear the bit in refitMask to unmark this node

			// perform refit & node compression
			refitNode(mPool, Index, boxes, indices, todoWriteback, todoCompress, prevScale, prevXYZ, bMin, bMax, wMin, wMax);
		}
	}
	else
#endif
	{
		const PxU32* Bits = mRefitBitmask.getBits();
		PxU32 Size = mRefitHighestSetWord+1;
#ifdef _DEBUG
		if(1)
		{
			const PxU32 TotalSize = mRefitBitmask.getSize();
			for(PxU32 i=Size;i<TotalSize;i++)
			{
				PX_ASSERT(!Bits[i]);
			}
		}
#endif
		while(Size--)
		{
			// Test 32 bits at a time
			if(!Bits[Size])
				continue;

			PxU32 Index = (Size+1)<<5;
			PxU32 _Count=32;
			while(_Count--)
			{
				Index--;
				Ps::prefetch(mPool+Index);

				if(mRefitBitmask.isSet(Index))
				{
					mRefitBitmask.clearBit(Index);

					// todoWriteback, todoCompress are staggered for xbox LHS elimination
					refitNode(mPool, Index, boxes, indices, todoWriteback, todoCompress, prevScale, prevXYZ, bMin, bMax, wMin, wMax);
				}
			}
		}
		mRefitHighestSetWord = 0;
	}

	// todoWriteback, todoCompress are staggered for xbox LHS elimination
	if (todoWriteback)
		todoWriteback->writeBack(prevScale, prevXYZ);
	if (todoCompress)
	{
		todoCompress->compress<0>(bMin, bMax, &prevScale, &prevXYZ);
		todoCompress->writeBack(prevScale, prevXYZ);
	}
}

void AABBTree::shiftOrigin(const PxVec3& shift)
{
	for(PxU32 i=0; i < mTotalNbNodes; i++)
	{
		AABBTreeNode& Current = mPool[i];
		if((i+1) < mTotalNbNodes)
			Ps::prefetch(mPool + i + 1);

		Vec3V centerV, extentsV, shiftV, minV, maxV;
		shiftV = V3LoadU(shift);
		Current.getAABBCenterExtentsV(&centerV, &extentsV);

		centerV = V3Sub(centerV, shiftV);
		minV = V3Sub(centerV, extentsV);
		maxV = V3Add(centerV, extentsV);

		Current.compress<1>(minV, maxV);
	}
}

#ifdef PX_DEBUG
// validate tree parent/child pointer correctness
void AABBTree::validate() const
{
	struct Local
	{
		static void _walk(const AABBTreeNode* parentNode, const AABBTreeNode* currentNode, const AABBTreeNode* root)
		{
			PX_ASSERT(parentNode == currentNode || parentNode < currentNode);
			PX_ASSERT(currentNode->getParent(root) == parentNode);
			if (!currentNode->isLeaf())
			{
				_walk(currentNode, currentNode->getPos(root), root);
				_walk(currentNode, currentNode->getNeg(root), root);
			}
		}
	};
	Local::_walk(mPool, mPool, mPool);
}
#endif
