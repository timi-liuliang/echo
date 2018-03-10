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


#ifndef PX_PHYSICS_SCP_CONSTRAINT_GROUP_NODE
#define PX_PHYSICS_SCP_CONSTRAINT_GROUP_NODE

#include "ScConstraintProjectionTree.h"

namespace physx
{
namespace Sc
{
	class ConstraintSim;
	class BodySim;
	class ConstraintProjectionManager;

	// A 'simulation island' of constraints. Created by a union-find algorithm every time a new constraint is added to any of the involved bodies.
	struct ConstraintGroupNode : public Ps::UserAllocated
	{
		enum StateFlags
		{
			eDISCOVERED						= 1 << 0,	// Used during projection tree generation to mark processed nodes.
			eIN_PROJECTION_PASS_LIST		= 1 << 1,	// Temporarily used to avoid duplicate entries in the list of nodes that should project the pose after the solver
			ePENDING_TREE_UPDATE			= 1 << 2	// Marks the constraint groups that need their projection trees updated. Must only be set on the root group node.
		};

		ConstraintGroupNode(BodySim& b);
		~ConstraintGroupNode()
		{
			PX_ASSERT(!readFlag(ePENDING_TREE_UPDATE));
			PX_ASSERT(projectionFirstRoot == NULL);
		}

		PX_FORCE_INLINE		void					raiseFlag(StateFlags f) { flags |= f; }
		PX_FORCE_INLINE		void					clearFlag(StateFlags f) { flags &= ~f; }
		PX_FORCE_INLINE		bool					readFlag(StateFlags f) const { return (flags & f) != 0; }

							ConstraintGroupNode&	getRoot();

		PX_FORCE_INLINE		void					buildProjectionTrees(); //build the projection trees for a constraint group.
							void					markForProjectionTreeRebuild(ConstraintProjectionManager&);
		PX_FORCE_INLINE		void					purgeProjectionTrees();
		PX_FORCE_INLINE		bool					hasProjectionTreeRoot() { return projectionFirstRoot != NULL; }
		PX_FORCE_INLINE		void					setProjectionTreeRoot(ConstraintGroupNode* root) { projectionFirstRoot = root; }

							void					initProjectionData(ConstraintGroupNode* parent, ConstraintSim* c);
							void					clearProjectionData();

		static				void					projectPose(ConstraintGroupNode& root);


		BodySim*					body;		//the owner body of this node

		//tree for union/find:
		ConstraintGroupNode*		parent;
		ConstraintGroupNode*		tail;		//only valid if this is root of group, points to LList tail node.
		PxU32						rank;		//rank counter for union/find. Initially zero. Is number of hops from root to furthest leaf in tree. This is just a hint to create more balanced trees.
		
		//linked list for traversal:
		ConstraintGroupNode*		next;		//next in list, NULL at tail.

		//projection tree information
		ConstraintGroupNode*		projectionFirstRoot;	//pointer to first projection tree root node. Only set for constraint group roots
		ConstraintGroupNode*		projectionNextRoot;		//pointer to next projection root node. Only set for constraint group roots
															//a constraint group can consist of multiple projection trees if kinematics are involved! Because a kinematic doesn't split
															//the constraint group as a static anchor does.
		ConstraintGroupNode*		projectionParent;		//node to project to
		ConstraintGroupNode*		projectionFirstChild;	//first node which gets projected to this one
		ConstraintGroupNode*		projectionNextSibling;	//the next sibling which gets projected to the same node as this one. NULL if projectionParent is NULL.
		ConstraintSim*				projectionConstraint;	//the constraint to project (constraint to projection parent)

		PxU8						flags;
	};

} // namespace Sc


PX_FORCE_INLINE void Sc::ConstraintGroupNode::buildProjectionTrees()
{
	PX_ASSERT(this == parent);  // Only call for group roots
	PX_ASSERT(!hasProjectionTreeRoot());

	ConstraintProjectionTree::buildProjectionTrees(*this);
}


PX_FORCE_INLINE void Sc::ConstraintGroupNode::purgeProjectionTrees()
{
	PX_ASSERT(this == parent);  // Only call for group roots
	PX_ASSERT(hasProjectionTreeRoot());
	ConstraintProjectionTree::purgeProjectionTrees(*this);
}

}

#endif
