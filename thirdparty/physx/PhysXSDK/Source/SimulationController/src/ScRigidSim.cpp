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


#include "ScScene.h"
#include "ScRigidSim.h"
#include "ScShapeInstancePairLL.h"
#include "ScTriggerInteraction.h"
#include "ScObjectIDTracker.h"
#include "ScShapeIterator.h"

using namespace physx;

/*
	PT:

	The BP group ID comes from a Cm::IDPool, and RigidSim is the only class releasing the ID.

	The rigid tracker ID comes from a Cm::IDPool internal to an ObjectIDTracker, and RigidSim
	is the only class using it.

	Thus we should:
	- promote the BP group ID stuff to a "tracker" object
	- use the BP group ID as a rigid ID
*/

Sc::RigidSim::RigidSim(Scene& scene, RigidCore& core, IslandNodeInfo::Type type) :
	ActorSim(scene, core, type)
{
	mRigidId	= scene.getRigidIDTracker().createID();

#ifdef PX_CHECKED
#if PX_USE_16_BIT_HANDLES
	PX_CHECK_MSG(getBroadphaseGroupId() < PX_INVALID_BP_HANDLE, "The total of actors in the scene plus the number of adds cannot exceed 65535 between simulate()/fetchResult() calls.  The sdk will can now proceed with unexpected outcomes. \n");
#endif
#endif
}

Sc::RigidSim::~RigidSim()
{
	Sc::Scene& scScene = getScene();
	scScene.getRigidIDTracker().releaseID(mRigidId);
}

void Sc::RigidSim::notifyShapeTranformChange()
{
	for(Element*e = getElements_(); e!=0; e = e->mNextInActor)
	{
		if(e->getElementType() == PX_ELEMENT_TYPE_SHAPE)
			static_cast<Sc::ShapeSim*>(e)->onTransformChange();
	}

}

Sc::ShapeSim &Sc::RigidSim::getSimForShape(Sc::ShapeCore& core) const
{
	// DS: looks painful to traverse a linked list this way
	Sc::ShapeIterator iterator;
	iterator.init(*this);
	Sc::ShapeSim* sim;
	while((sim = iterator.getNext())!=NULL)
	{
		if(&sim->getCore() == &core)
			return *sim;
	}
	PX_ASSERT(0); // should never fail
	return *reinterpret_cast<Sc::ShapeSim*>(1);
}

PxActor* Sc::RigidSim::getPxActor()
{
	return getRigidCore().getPxActor();
}
