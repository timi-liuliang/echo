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


#include "ScActorCore.h"
#include "ScActorSim.h"
#include "ScShapeIterator.h"
#include "ScShapeCore.h"
#include "ScShapeSim.h"
#include "ScBodySim.h"

using namespace physx;

Sc::ActorCore::ActorCore(PxActorType::Enum actorType, PxU16 actorFlags, PxClientID owner, PxU8 behavior, PxDominanceGroup dominanceGroup) :
// PX_AGGREGATE
	mAggregateID				(PX_INVALID_U32),
//~PX_AGGREGATE
	mSim					(NULL),
	mActorFlags				(actorFlags),
	mActorType				(PxU8(actorType)),
	mClientBehaviorFlags	(behavior),
	mDominanceGroup			(dominanceGroup),
	mOwnerClient			(owner)
{
	PX_ASSERT((actorType & 0xff) == actorType);
}

Sc::ActorCore::~ActorCore()
{
}

void Sc::ActorCore::setActorFlags(PxActorFlags af)	
{ 
//	mActorFlags = af; 

	PxActorFlags old = mActorFlags;
	if(af!=old)
	{
		mActorFlags = af;

		if(mSim)
			mSim->postActorFlagChange(old, af);
	}
}	

void Sc::ActorCore::setDominanceGroup(PxDominanceGroup g)
{
	mDominanceGroup = g;
	if(mSim)
		mSim->postDominanceGroupChange();
}

void Sc::ActorCore::reinsertShapes()
{
	PX_ASSERT(mSim);
	if(!mSim)
		return;

	//We need to reset the PxsRigidBody's aabbMgr id because we're going to remove every single shape of the body.
	BodySim* bodySim=NULL;
	if(PxActorType::eRIGID_DYNAMIC==getActorCoreType() || PxActorType::eARTICULATION_LINK==getActorCoreType())
	{
		Sc::BodyCore* bodyCore=(Sc::BodyCore*)this;
		bodySim=bodyCore->getSim();
		bodySim->getLowLevelBody().resetAABBMgrId();
	}

	ShapeIterator shapeIterator;
	shapeIterator.init(*mSim);

	ShapeSim* nonNullSim = NULL;
	ShapeSim* sim=NULL;
	while(NULL != (sim = shapeIterator.getNext()) )
	{
		nonNullSim = sim;
		sim->reinsertBroadPhase();
	}

	//Reset the PxsRigidBody's aabbMgr id.
	if(bodySim && nonNullSim)
		bodySim->getLowLevelBody().setAABBMgrId(nonNullSim->getAABBMgrId());
}
