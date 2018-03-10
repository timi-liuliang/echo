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


#include "ScConstraintInteraction.h"
#include "ScConstraintSim.h"
#include "ScBodySim.h"
#include "ScScene.h"
#include "PxsConstraint.h"
#include "PxsRigidBody.h"

using namespace physx;

Sc::ConstraintInteraction::ConstraintInteraction(ConstraintSim* constraint, RigidSim& r0, RigidSim& r1) :
	ActorInteraction	(r0, r1, PX_INTERACTION_TYPE_CONSTRAINTSHADER, PX_INTERACTION_FLAG_CONSTRAINT),
	mConstraint			(constraint)
{
	BodySim* b0 = mConstraint->getBody(0);
	BodySim* b1 = mConstraint->getBody(1);

	if (b0)
		b0->onConstraintAttach();
	if (b1)
		b1->onConstraintAttach();
}

Sc::ConstraintInteraction::~ConstraintInteraction()
{
	PX_ASSERT(!mLLIslandHook.isManaged());
}

void Sc::ConstraintInteraction::destroy()
{
	Scene& scene = getScene();

	if (mConstraint->readFlag(ConstraintSim::eBREAKABLE | ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED) == (ConstraintSim::eBREAKABLE | ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED))
		scene.removeActiveBreakableConstraint(mConstraint);

	if(mLLIslandHook.isManaged())
		scene.getInteractionScene().getLLIslandManager().removeEdge(PxsIslandManager::EDGE_TYPE_CONSTRAINT, mLLIslandHook);

	setClean(true);
	ActorInteraction::destroy();

	BodySim* b0 = mConstraint->getBody(0);
	BodySim* b1 = mConstraint->getBody(1);

	if (b0)
		b0->onConstraintDetach();  // Note: Has to be done AFTER destroy (interaction not registered in actors anymore)
	if (b1)
		b1->onConstraintDetach();  // Note: Has to be done AFTER destroy (interaction not registered in actors anymore)
}


void Sc::ConstraintInteraction::updateState(bool removeFromDirtyList)
{
	ActorInteraction::updateState(removeFromDirtyList);
}


bool Sc::ConstraintInteraction::onActivate(PxU32)
{
	PX_ASSERT(!mConstraint->isBroken());
	PX_ASSERT(mConstraint->getLowLevelConstraint());

	BodySim* b0 = mConstraint->getBody(0);
	BodySim* b1 = mConstraint->getBody(1);

	if (!mLLIslandHook.isManaged())
	{
		PxsIslandManager& islandManager = getScene().getInteractionScene().getLLIslandManager();
		islandManager.addEdge(PxsIslandManager::EDGE_TYPE_CONSTRAINT, 
							  b0 ? b0->getLLIslandManagerNodeHook() : PxsIslandManagerNodeHook::INVALID,
							  b1 ? b1->getLLIslandManagerNodeHook() : PxsIslandManagerNodeHook::INVALID,
							  mLLIslandHook);
		islandManager.setEdgeConstraint(mLLIslandHook, static_cast<PxsConstraint *>(mConstraint->getLowLevelConstraint()));
		islandManager.setEdgeConnected(mLLIslandHook);
	}

	bool a0Vote = !b0 || b0->isActive();
	bool a1Vote = !b1 || b1->isActive();

	if (a0Vote || a1Vote)
	{
		if (mConstraint->readFlag(ConstraintSim::eBREAKABLE | ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED) == ConstraintSim::eBREAKABLE)
			getScene().addActiveBreakableConstraint(mConstraint);

		return true;
	}
	else
		return false;
}

bool Sc::ConstraintInteraction::onDeactivate(PxU32)
{
	PX_ASSERT(mConstraint->getLowLevelConstraint());

	if (mConstraint->readFlag(ConstraintSim::eBREAKABLE | ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED) == (ConstraintSim::eBREAKABLE | ConstraintSim::eCHECK_MAX_FORCE_EXCEEDED))
		getScene().removeActiveBreakableConstraint(mConstraint);

	return true;
}
