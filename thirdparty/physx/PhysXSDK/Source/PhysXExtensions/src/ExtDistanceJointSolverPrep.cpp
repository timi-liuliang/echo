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


#include "ExtDistanceJoint.h"
#include "PxSimpleTypes.h"

namespace physx
{
namespace Ext
{
	PxU32 DistanceJointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 maxConstraints,
		PxConstraintInvMassScale &invMassScale,
		const void* constantBlock,
		const PxTransform& bA2w,
		const PxTransform& bB2w)
	{
		PX_UNUSED(maxConstraints);

		const DistanceJointData& data = *reinterpret_cast<const DistanceJointData*>(constantBlock);
		invMassScale = data.invMassScale;

		PxTransform cA2w = bA2w.transform(data.c2b[0]);
		PxTransform cB2w = bB2w.transform(data.c2b[1]);

		body0WorldOffset = cB2w.p - bA2w.p;

		PxVec3 direction = cA2w.p - cB2w.p;
		PxReal distance = direction.normalize();

		bool enforceMax = (data.jointFlags & PxDistanceJointFlag::eMAX_DISTANCE_ENABLED);
		bool enforceMin = (data.jointFlags & PxDistanceJointFlag::eMIN_DISTANCE_ENABLED);

		if((!enforceMax || distance<=data.maxDistance) && (!enforceMin || distance>=data.minDistance))
			return 0;

#define EPS_REAL 1.192092896e-07F

		if (distance < EPS_REAL)
			direction = PxVec3(1.f,0,0);

		Px1DConstraint *c = constraints;

		// constraint is breakable, so we need to output forces

		c->flags = Px1DConstraintFlag::eOUTPUT_FORCE;

		c->linear0 = direction;		c->angular0 = (cA2w.p - bA2w.p).cross(c->linear0);		
		c->linear1 = direction;		c->angular1 = (cB2w.p - bB2w.p).cross(c->linear1);		

		if (data.jointFlags & PxDistanceJointFlag::eSPRING_ENABLED)
		{
			c->flags |= Px1DConstraintFlag::eSPRING;
			c->mods.spring.stiffness= data.stiffness;
			c->mods.spring.damping	= data.damping;
		}

		//add tolerance so we don't have contact-style jitter problem.

		if (data.minDistance == data.maxDistance && enforceMin && enforceMax)
		{
			PxReal error = distance - data.maxDistance;
			c->geometricError = error >  data.tolerance ? error - data.tolerance :
				error < -data.tolerance ? error + data.tolerance : 0;
		}
		else if (enforceMax && distance > data.maxDistance)
		{
			c->geometricError = distance - data.maxDistance - data.tolerance;
			c->maxImpulse = 0;
		}
		else if (enforceMin && distance < data.minDistance)
		{
			c->geometricError = distance - data.minDistance + data.tolerance;	
			c->minImpulse = 0;
		}

		return 1;
	}
}//namespace

}

//~PX_SERIALIZATION
