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
#include "SqUtilities.h"
#include "NpActor.h"
#include "ScbRigidStatic.h"
#include "ScbBody.h"
#include "NpShape.h"
#include "SqPruner.h"
#include "CmTransformUtils.h"
#include "SqSceneQueryManager.h"

using namespace physx;
using namespace shdfnd::aos;

PxTransform Sq::getGlobalPose(const NpShape& shape, const PxRigidActor& actor)
{
	const Scb::Actor& scbActor = gOffsetTable.convertPxActor2Scb(actor);
	const Scb::Shape& scbShape = shape.getScbShape();

	return getGlobalPose(scbShape, scbActor);
}


PxBounds3 Sq::computeWorldAABB(const Scb::Shape& scbShape, const Scb::Actor& scbActor)
{
	PxActorType::Enum actorType = gOffsetTable.convertScbActor2Sc(scbActor).getActorCoreType();
	const Gu::GeometryUnion& geom = scbShape.getGeometryUnion();
	const PxTransform& shape2Actor = scbShape.getShape2Actor();

	PX_ALIGN(16, PxTransform) globalPose;

	if(actorType==PxActorType::eRIGID_STATIC)
		Cm::getStaticGlobalPoseAligned(static_cast<const Scb::RigidStatic&>(scbActor).getActor2World(), shape2Actor, globalPose);
	else
	{
		const Scb::Body& body = static_cast<const Scb::Body&>(scbActor);
		PX_ALIGN(16, PxTransform) kinematicTarget;
		PxU16 sqktFlags = PxRigidBodyFlag::eKINEMATIC|PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES;		
		bool useTarget = (PxU16(body.getFlags()) & sqktFlags) == sqktFlags;
		const PxTransform& body2World = (useTarget && body.getKinematicTarget(kinematicTarget)) ? kinematicTarget : body.getBody2World();
		Cm::getDynamicGlobalPoseAligned(body2World, shape2Actor, body.getBody2Actor(), globalPose);
	}

	return inflateBounds(geom.computeBounds(globalPose));

}

PxTransform Sq::getGlobalPose(const Scb::Shape& scbShape, const Scb::Actor& scbActor)
{
	PxActorType::Enum actorType = gOffsetTable.convertScbActor2Sc(scbActor).getActorCoreType();
	if(actorType==PxActorType::eRIGID_STATIC)
	{
		return static_cast<const Scb::RigidStatic&>(scbActor).getActor2World() * scbShape.getShape2Actor();
	}
	else 
	{
		PX_ASSERT(actorType==PxActorType::eRIGID_DYNAMIC || actorType == PxActorType::eARTICULATION_LINK);

		PxTransform body2World;
		const Scb::Body& body = static_cast<const Scb::Body&>(scbActor);
		if (!(body.getFlags() & PxRigidBodyFlag::eKINEMATIC))
			body2World = body.getBody2World();
		else
		{
			PxTransform bodyTarget;
			if (!body.getKinematicTarget(bodyTarget) || (!(body.getFlags() & PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES)))
				body2World = body.getBody2World();
			else
				body2World = bodyTarget;
		}

		return body2World * body.getBody2Actor().getInverse() * scbShape.getShape2Actor();
	}
}

Sq::ActorShape* Sq::populate(const ActorShape* p)
{
	return const_cast<ActorShape*>(p);
}

void Sq::populate(const PrunerPayload& payload, PxActorShape2& as)
{
	as.scbShape = (Scb::Shape*)payload.data[0];
	as.scbActor = (Scb::Actor*)payload.data[1];

	as.actor = getHitActor(as.scbActor);
	as.shape = getHitShape(as.scbShape);
}

PxRigidActor*	Sq::getHitActor(const Scb::Actor* scbActor)
{
	return scbActor ? 
		static_cast<PxRigidActor*>(static_cast<const Sc::RigidCore&>(gOffsetTable.convertScbActor2Sc(*scbActor)).getPxActor()) 
		: NULL;
}
PxShape*		Sq::getHitShape(const Scb::Shape* scbShape)
{
	return scbShape ? 
		const_cast<PxShape*>(scbShape->getScShape().getPxShape())
		: (PxShape*)scbShape;
}
