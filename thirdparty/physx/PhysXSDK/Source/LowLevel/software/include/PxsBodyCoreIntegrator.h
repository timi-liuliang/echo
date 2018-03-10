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


#ifndef PXS_BODYCORE_INTEGRATOR_H
#define PXS_BODYCORE_INTEGRATOR_H

#include "CmPhysXCommon.h"
#include "PxvDynamics.h"
#include "PsMathUtils.h"
#include "PxsRigidBody.h"

namespace physx
{

PX_FORCE_INLINE void bodyCoreComputeUnconstrainedVelocity
(PxsBodyCore& core, const PxVec3& linearAccel, const PxVec3& angularAccel, const PxReal dt)
{
	//TODO - rewrite this with simd instructions.

	//Grab what we need from the body.
	PxVec3 linearVelocity=core.linearVelocity;
	PxVec3 angularVelocity=core.angularVelocity;
	const PxReal linearDamping=core.linearDamping;
	const PxReal angularDamping=core.angularDamping;
	const PxReal maxLinearVelocitySq=core.maxLinearVelocitySq;
	const PxReal maxAngularVelocitySq=core.maxAngularVelocitySq;

	//Multiply everything that needs multiplied by dt to improve code generation.
	const PxVec3 linearAccelTimesDT=linearAccel*dt;
	const PxVec3 angularAccelTimesDT=angularAccel*dt;
	const PxReal linearDampingTimesDT=linearDamping*dt;
	const PxReal angularDampingTimesDT=angularDamping*dt;
	const PxReal oneMinusLinearDampingTimesDT=1.0f-linearDampingTimesDT;
	const PxReal oneMinusAngularDampingTimesDT=1.0f-angularDampingTimesDT;

	//TODO context-global gravity
	linearVelocity += linearAccelTimesDT;
	angularVelocity += angularAccelTimesDT;

	//Apply damping.
	const PxReal linVelMultiplier = physx::intrinsics::fsel(oneMinusLinearDampingTimesDT, oneMinusLinearDampingTimesDT, 0.0f);
	const PxReal angVelMultiplier = physx::intrinsics::fsel(oneMinusAngularDampingTimesDT, oneMinusAngularDampingTimesDT, 0.0f);
	linearVelocity*=linVelMultiplier;
	angularVelocity*=angVelMultiplier;

	// Clamp velocity
	const PxReal linVelSq = linearVelocity.magnitudeSquared();
	if(linVelSq > maxLinearVelocitySq)
	{
		linearVelocity *= PxSqrt(maxLinearVelocitySq / linVelSq);
	}
	const PxReal angVelSq = angularVelocity.magnitudeSquared();
	if(angVelSq > maxAngularVelocitySq)
	{
		angularVelocity *= PxSqrt(maxAngularVelocitySq / angVelSq);
	}

	//Update the body.
	core.linearVelocity=linearVelocity;
	core.angularVelocity=angularVelocity;
}


PX_FORCE_INLINE void integrateCore(PxsBodyCore& core, PxsRigidBody& body, Cm::SpatialVector& motionVelocity, const PxF32 dt) 
{

	// Integrate linear part
	PxVec3 delta = motionVelocity.linear * dt;
	PxReal w = motionVelocity.angular.magnitudeSquared();
	core.body2World.p += delta;
	PX_ASSERT(core.body2World.p.isFinite());

	//Cache the motion velocities in the acceleration
	body.mAcceleration = motionVelocity;
	
	// Integrate the rotation using closed form quaternion integrator
	
	w = PxSqrt(w);
	// Perform a post-solver clamping
	// TODO(dsequeira): ignore this for the moment
		//just clamp motionVel to half float-range
	const PxReal maxW = 1e+7f;		//Should be about sqrt(PX_MAX_REAL/2) or smaller
	if(w > maxW)
	{
		motionVelocity.angular = motionVelocity.angular.getNormalized() * maxW;
		w = maxW;
	}

	if (w != 0.0f)
	{
		const PxReal v = dt * w * 0.5f;
		PxReal s, q;
		Ps::sincos(v, s, q);
		s /= w;

		const PxVec3 pqr = motionVelocity.angular * s;
		const PxQuat quatVel(pqr.x, pqr.y, pqr.z, 0);
		PxQuat result = quatVel * core.body2World.q;

		result += core.body2World.q * q;

		core.body2World.q = result.getNormalized();
		PX_ASSERT(core.body2World.q.isSane());
		PX_ASSERT(core.body2World.q.isFinite());
	}
}

}

#endif //PXS_BODYCORE_INTEGRATOR_H
