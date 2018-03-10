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

           
#include "ScIterators.h"
#include "ScBodySim.h"
#include "ScShapeSim.h"
#include "ScConstraintSim.h"
#include "ScShapeInstancePairLL.h"
#include "ScShapeIterator.h"
#include "GuTriangleMesh.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

Sc::ShapeSim* Sc::ShapeIterator::getNext()	
{ 
	while(1)
	{
		if(!mCurrent)
			return NULL;
		Element* element = mCurrent;
		mCurrent = mCurrent->mNextInActor;
		if(element->getElementType() == Sc::PX_ELEMENT_TYPE_SHAPE)	// PT: this can also be a particle packet!
			return static_cast<Sc::ShapeSim*>(element);
	}
}

void Sc::ShapeIterator::init(const Sc::ActorSim& r)
{ 
	mCurrent = const_cast<Element*>(r.getElements_());
}	

///////////////////////////////////////////////////////////////////////////////

Sc::BodyCore* Sc::BodyIterator::getNext()
{ 
	while (!mRange.empty())
	{
		Sc::Actor* const current = mRange.front();
		mRange.popFront();

		if (current->isDynamicRigid())
			return &static_cast<Sc::BodySim*>(current)->getBodyCore();
	}
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////

Sc::ContactIterator::Pair::Pair(const void*& contactData, PxU32 contactDataSize, const PxReal*& forces, PxU32 numContacts, ShapeSim& shape0, ShapeSim& shape1)
: mIndex(0)
, mNumContacts(numContacts)
, mIter(reinterpret_cast<const PxU8*>(contactData), contactDataSize)
, mForces(forces)
, mShape0(&shape0)
, mShape1(&shape1)
{	
	mCurrentContact.shape0 = shape0.getPxShape();
	mCurrentContact.shape1 = shape1.getPxShape();
	mCurrentContact.normalForceAvailable = (forces != NULL);
}

Sc::ContactIterator::Pair* Sc::ContactIterator::getNextPair()
{ 
	if(!mRange.empty())
	{
		ShapeInstancePairLL* llPair = static_cast<ShapeInstancePairLL*>(mRange.front());

		const void* contactData = NULL;
		PxU32 contactDataSize = 0;
		const PxReal* forces = NULL;
		PxU32 numContacts = 0;

		PxU32 nextOffset = llPair->getContactPointData(contactData, contactDataSize, numContacts, forces, mOffset);

		if (nextOffset == mOffset)
			mRange.popFront();
		else
			mOffset = nextOffset;

		mCurrentPair = Pair(contactData, contactDataSize, forces, numContacts, llPair->getShape0(), llPair->getShape1());
		return &mCurrentPair;
	}
	else
		return NULL;
}

Sc::ContactIterator::Contact* Sc::ContactIterator::Pair::getNextContact()
{
	if(mIndex < mNumContacts)
	{
		if(!mIter.hasNextContact())
		{
			if(!mIter.hasNextPatch())
				return NULL;
			mIter.nextPatch();
		}
		PX_ASSERT(mIter.hasNextContact());
		mIter.nextContact();

		mCurrentContact.normal = mIter.getContactNormal();
		mCurrentContact.point = mIter.getContactPoint();
		mCurrentContact.separation = mIter.getSeparation();
		mCurrentContact.normalForce = mForces ? mForces[mIndex] : 0;
		mCurrentContact.faceIndex0 = mIter.getFaceIndex0();
		mCurrentContact.faceIndex1 = mIter.getFaceIndex1();

		mIndex++;
		return &mCurrentContact;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
