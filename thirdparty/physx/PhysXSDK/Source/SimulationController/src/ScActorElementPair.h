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


#ifndef PX_PHYSICS_SCP_ACTOR_ELEMENT_PAIR
#define PX_PHYSICS_SCP_ACTOR_ELEMENT_PAIR

#include "ScActorSim.h"
#include "ScElementSim.h"

namespace physx
{
namespace Sc
{

	// Class shared by all element-element pairs where one element should not be distinguished from
	// its actor (for example: rb-fluid, ...)
	class ActorElementPair
	{
	public:
		PX_INLINE ActorElementPair(ActorSim& actor, ElementSim& element, PxPairFlags pairFlag);
		PX_INLINE ~ActorElementPair();
		
		PX_INLINE ElementSim& getElement() const { return mElement; }
		PX_INLINE ActorSim& getActor() const { return mActor; }

		PX_INLINE void incRefCount() { ++mRefCount; PX_ASSERT(mRefCount>0); }
		PX_INLINE PxU32 decRefCount() { PX_ASSERT(mRefCount>0); return --mRefCount; }
		PX_INLINE PxU32 getRefCount() const { return mRefCount; }

		PX_INLINE PxPairFlags getPairFlags() const { return mPairFlags; }
		PX_INLINE void setPairFlags(PxPairFlags pairFlags) { mPairFlags = pairFlags; }

		PX_INLINE bool isFilterPair() const { return mIsFilterPair; }
		PX_INLINE void markAsFilterPair(bool filterPair) { mIsFilterPair = filterPair; }

		PX_INLINE bool isSuppressed() const { return mIsSuppressed; }
		PX_INLINE void markAsSuppressed(bool suppress) { mIsSuppressed = suppress; }

		PX_INLINE bool isKilled() const { return mIsKilled; }
		PX_INLINE void markAsKilled(bool killed) { mIsKilled = killed; }

		PX_INLINE bool hasBeenRefiltered(PxU32 sceneTimestamp);

	private:
		ActorElementPair& operator=(const ActorElementPair&);
		ActorSim& mActor;
		ElementSim& mElement;
		PxPairFlags mPairFlags;
		PxU32 mRefilterTimestamp;
		PxU16 mRefCount;
		bool mIsFilterPair;
		bool mIsSuppressed;
		bool mIsKilled;
	};

} // namespace Sc


Sc::ActorElementPair::ActorElementPair(ActorSim& actor, ElementSim& element, PxPairFlags pairFlag) : 
	mActor(actor),
	mElement(element),
	mPairFlags(pairFlag),
	mRefilterTimestamp(0),
	mRefCount(0),
	mIsFilterPair(false),
	mIsSuppressed(false),
	mIsKilled(false)
{ 
}


Sc::ActorElementPair::~ActorElementPair()
{
}


PX_INLINE bool Sc::ActorElementPair::hasBeenRefiltered(PxU32 sceneTimestamp)
{
	if (mRefilterTimestamp != sceneTimestamp)
	{
		mRefilterTimestamp = sceneTimestamp;
		return false;
	}
	else
		return true;
}

}

#endif
