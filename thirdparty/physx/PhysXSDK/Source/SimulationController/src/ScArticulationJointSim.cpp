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


#include "ScArticulationJointSim.h"
#include "ScArticulationJointCore.h"
#include "ScBodySim.h"
#include "ScScene.h"
#include "PxsRigidBody.h"
#include "PxsArticulation.h"
#include "ScArticulationSim.h"

using namespace physx;

Sc::ArticulationJointSim::ArticulationJointSim(ArticulationJointCore& joint, ActorSim& parent, ActorSim& child) :
	ActorInteraction	(parent, child, PX_INTERACTION_TYPE_ARTICULATION, 0),
	mCore				(joint)
{
	initialize();

	BodySim& childBody = static_cast<BodySim&>(child),
		   & parentBody = static_cast<BodySim&>(parent);

	parentBody.getArticulation()->addBody(childBody, &parentBody, this);

	mCore.setSim(this);
}


Sc::ArticulationJointSim::~ArticulationJointSim()
{
	if(mIslandHook.isManaged())
		getScene().getInteractionScene().getLLIslandManager().removeEdge(PxsIslandManager::EDGE_TYPE_ARTIC, mIslandHook);

	BodySim& child = getChild();
	child.getArticulation()->removeBody(child);

	mCore.setSim(NULL);
}




Sc::BodySim& Sc::ArticulationJointSim::getParent() const
{
	return static_cast<BodySim&>(getActorSim0());
}


Sc::BodySim& Sc::ArticulationJointSim::getChild() const
{
	return static_cast<BodySim&>(getActorSim1());
}


bool Sc::ArticulationJointSim::onActivate(PxU32)
{
	if(!mIslandHook.isManaged())
	{
		PxsIslandManager& islandManager = getParent().getInteractionScene().getLLIslandManager();
		islandManager.addEdge(PxsIslandManager::EDGE_TYPE_ARTIC, getParent().getLLIslandManagerNodeHook(), getChild().getLLIslandManagerNodeHook(), mIslandHook);
		islandManager.setEdgeArticulationJoint(mIslandHook);
		islandManager.setEdgeConnected(mIslandHook);
	}

	if(!(getParent().isActive() && getChild().isActive()))
		return false;

	return true; 
}

bool Sc::ArticulationJointSim::onDeactivate(PxU32)
{ 
	return true; 
}


void Sc::ArticulationJointSim::destroy()
{
	setClean(true);
	ActorInteraction::destroy();
	delete this;
}
