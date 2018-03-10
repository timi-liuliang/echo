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


#ifndef PX_PHYSICS_SQUTILITIES
#define PX_PHYSICS_SQUTILITIES

#include "CmPhysXCommon.h"
#include "NpActor.h"

namespace physx
{
	class PxRigidActor;
	class NpShape;
	class PxShape;

	namespace Scb
	{
		class Actor;
		class Shape;
	}

namespace Sq
{
	class SceneQueryManager;
	struct ActorShape;
	struct PrunerPayload;

struct PxActorShape2 : PxActorShape
{
	const Scb::Shape* scbShape;
	const Scb::Actor* scbActor;

	PxActorShape2() : PxActorShape() {}

	PxActorShape2(PxRigidActor* eaActor, PxShape* eaShape, Scb::Shape* sShape, Scb::Actor* sActor) : PxActorShape(eaActor, eaShape)
	{
		scbShape = sShape;
		scbActor = sActor;
	}
};

	PxTransform		getGlobalPose(const NpShape& shape, const PxRigidActor& actor);
	PxTransform 	getGlobalPose(const Scb::Shape& scbShape, const Scb::Actor& scbActor);
	PxBounds3		computeWorldAABB(const Scb::Shape& scbShape, const Scb::Actor& scbActor);

	ActorShape*   	populate(const ActorShape*);
	void			populate(const PrunerPayload&,PxActorShape2&);

	PxRigidActor*	getHitActor(const Scb::Actor* scbActor);
	PxShape*		getHitShape(const Scb::Shape* scbShape);

	PX_FORCE_INLINE PxBounds3 inflateBounds(const PxBounds3& bounds)
	{
		PxVec3 e = bounds.getExtents() * 0.01f;
		return PxBounds3(bounds.minimum - e, bounds.maximum + e);
	}

}  // namespace Sq

}

#endif
