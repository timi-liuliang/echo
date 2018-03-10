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


#include "ScElement.h"
#include "ScElementInteraction.h"
#include "PsFoundation.h"
#include "PxsContext.h"
#include "PxvBroadPhase.h"
#include "PxsAABBManager.h"

using namespace physx;


Sc::ElementInteraction* Sc::Element::ElementInteractionIterator::getNext()
{
	for (; !mInteractions.empty(); mInteractions.popFront())
	{
		Interaction*const it = mInteractions.front();

		if (it->getInteractionFlags() & (PX_INTERACTION_FLAG_RB_ELEMENT | PX_INTERACTION_FLAG_ELEMENT_ACTOR))
		{
#if PX_USE_PARTICLE_SYSTEM_API
			PX_ASSERT(	(it->getType() == PX_INTERACTION_TYPE_MARKER) ||
						(it->getType() == PX_INTERACTION_TYPE_OVERLAP) ||
						(it->getType() == PX_INTERACTION_TYPE_TRIGGER) ||
						(it->getType() == PX_INTERACTION_TYPE_PARTICLE_BODY) );
#else
			PX_ASSERT(	(it->getType() == PX_INTERACTION_TYPE_MARKER) ||
						(it->getType() == PX_INTERACTION_TYPE_OVERLAP) ||
						(it->getType() == PX_INTERACTION_TYPE_TRIGGER) );
#endif

			ElementInteraction* ei = static_cast<ElementInteraction*>(it);
			if ((&ei->getElement0() == mElement) || (&ei->getElement1() == mElement))
			{
				mInteractions.popFront();
				return ei;
			}
		}
	}

	return NULL;
}


Sc::ElementInteraction* Sc::Element::ElementInteractionReverseIterator::getNext()
{
	for (; !mInteractions.empty(); mInteractions.popBack())
	{
		Interaction*const it = mInteractions.back();

		if (it->getInteractionFlags() & (PX_INTERACTION_FLAG_RB_ELEMENT | PX_INTERACTION_FLAG_ELEMENT_ACTOR))
		{
#if PX_USE_PARTICLE_SYSTEM_API
			PX_ASSERT(	(it->getType() == PX_INTERACTION_TYPE_MARKER) ||
						(it->getType() == PX_INTERACTION_TYPE_OVERLAP) ||
						(it->getType() == PX_INTERACTION_TYPE_TRIGGER) ||
						(it->getType() == PX_INTERACTION_TYPE_PARTICLE_BODY) );
#else
			PX_ASSERT(	(it->getType() == PX_INTERACTION_TYPE_MARKER) ||
						(it->getType() == PX_INTERACTION_TYPE_OVERLAP) ||
						(it->getType() == PX_INTERACTION_TYPE_TRIGGER) );
#endif

			ElementInteraction* ei = static_cast<ElementInteraction*>(it);
			if ((&ei->getElement0() == mElement) || (&ei->getElement1() == mElement))
			{
				mInteractions.popBack();
				return ei;
			}
		}
	}

	return NULL;
}


Sc::Element::~Element()
{
	PX_ASSERT(!hasAABBMgrHandle());
	mActor.onElementDetach(*this);
}

bool Sc::Element::createLowLevelVolume(const PxU32 group, const PxBounds3& bounds, const PxU32 aggregateID, const AABBMgrId& aabbMgrId)
{
	PX_ASSERT(!hasAABBMgrHandle());

	PxsAABBManager* aabbMgr = getInteractionScene().getLowLevelContext()->getAABBManager();
	AABBMgrId newAABBMgrId = aabbMgr->createVolume(aggregateID, aabbMgrId.mActorHandle, group, this, bounds);
	mAABBMgrShapeHandle = newAABBMgrId.mShapeHandle;
	mAABBMgrActorHandle = newAABBMgrId.mActorHandle;

	if(newAABBMgrId.mShapeHandle != PX_INVALID_BP_HANDLE)
		return true;

	//If the aabbmgr failed to add the new volume then report an error.
	Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Unable to create broadphase entity because only 32768 shapes are supported");
	return false;
}

bool Sc::Element::destroyLowLevelVolume()
{
	if (hasAABBMgrHandle())
	{
		PxsAABBManager* aabbMgr = getInteractionScene().getLowLevelContext()->getAABBManager();
		bool removingLastShape = aabbMgr->releaseVolume(mAABBMgrShapeHandle);
		mAABBMgrShapeHandle=PX_INVALID_BP_HANDLE;
		mAABBMgrActorHandle=PX_INVALID_BP_HANDLE;
		return removingLastShape;
	}

	return false;
}
