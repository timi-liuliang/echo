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


#include "ScConstraintGroupNode.h"
#include "ScConstraintProjectionManager.h"
#include "PsFoundation.h"
#include "ScBodySim.h"
#include "ScConstraintSim.h"
#include "ScConstraintInteraction.h"

using namespace physx;

Sc::ConstraintGroupNode::ConstraintGroupNode(BodySim& b) : 
	body(&b),
	parent(this),
	tail(this),
	rank(0),
	next(NULL),

	projectionFirstRoot(NULL),
	projectionNextRoot(NULL),
	projectionParent(NULL),
	projectionFirstChild(NULL),
	projectionNextSibling(NULL),
	projectionConstraint(NULL),

	flags(0)
{
}


//
// Implementation of FIND of 
// UNION-FIND algo.
//
Sc::ConstraintGroupNode& Sc::ConstraintGroupNode::getRoot()
{
	PX_ASSERT(parent);

	ConstraintGroupNode* root = parent;

	if (root->parent == root)
		return *root;
	else
	{
		PxU32 nbHops = 1;
		root = root->parent;

		while(root != root->parent)
		{
			root = root->parent;
			nbHops++;
		}

		// Write root to all nodes on the path
		ConstraintGroupNode* curr = this;
		while(nbHops)
		{
			ConstraintGroupNode* n = curr->parent;
			curr->parent = root;
			curr = n;
			nbHops--;
		}

		return *root;
	}
}


void Sc::ConstraintGroupNode::markForProjectionTreeRebuild(ConstraintProjectionManager& cpManager)
{
	ConstraintGroupNode& root = getRoot();
	if (!root.readFlag(ConstraintGroupNode::ePENDING_TREE_UPDATE))
	{
		cpManager.addToPendingTreeUpdates(root);
	}
}


void Sc::ConstraintGroupNode::initProjectionData(ConstraintGroupNode* parent_, ConstraintSim* c)
{
	projectionConstraint = c;

	//add us to parent's child list:
	if (parent_)
	{
		projectionNextSibling = parent_->projectionFirstChild;
		parent_->projectionFirstChild = this;

		projectionParent = parent_;
	}
}


void Sc::ConstraintGroupNode::clearProjectionData()
{
	projectionFirstRoot = NULL;
	projectionNextRoot = NULL;
	projectionParent = NULL;
	projectionFirstChild = NULL;
	projectionNextSibling = NULL;
	projectionConstraint = NULL;
}


void Sc::ConstraintGroupNode::projectPose(ConstraintGroupNode& node)
{
	PX_ASSERT(node.hasProjectionTreeRoot());

	Sc::ConstraintProjectionTree::projectPose(node);
}
