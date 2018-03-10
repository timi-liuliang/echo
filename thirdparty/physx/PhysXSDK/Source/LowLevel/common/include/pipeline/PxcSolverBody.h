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



#ifndef PXC_SOLVERBODY_H
#define PXC_SOLVERBODY_H

#include "PxvConfig.h"
#include "PxSimpleTypes.h"
#include "PxVec3.h"
#include "PxMat33.h"

namespace physx
{

class PxsRigidBody;
class PxsArticulation;

struct PxcSolverBody;

struct PxcSolverBodyData
{
	PxMat33			invInertia;			// 36 inverse inertia in world space
	PxReal			reportThreshold;    // 40 contact force threshold
	PxsRigidBody*	originalBody;		// 44 or 48 depending on ptr size
	PxcSolverBody*  solverBody;			// 48 or 56 depending on ptr size
	PxReal			invMass;			// 52 or 60 inverse mass
	PxReal			penBiasClamp;		// 56 or 64 the penetration bias clamp

#if !defined(PX_P64)
	PxU8    pad[8];
#endif
};

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverBodyData) == 64);


struct PxcSolverBody
{
	PxVec3				linearVelocity;					// 12 post-solver linear velocity in world space
	PxU16				maxSolverNormalProgress;
	PxU16				maxSolverFrictionProgress;
					
	PxVec3				angularVelocity;				// 28 post-solver angular velocity in world space	
	
	PxU32				solverProgress;					// 16

	PX_FORCE_INLINE void getResponse(const PxVec3& linImpulse, const PxVec3& angImpulse,
								     PxVec3& linDeltaV, PxVec3& angDeltaV, const PxcSolverBodyData& _solverBodyData) const
	{
		linDeltaV = linImpulse * _solverBodyData.invMass;
		angDeltaV = _solverBodyData.invInertia * angImpulse;
	}

	PX_FORCE_INLINE	PxReal getResponse(const PxVec3& normal, const PxVec3& rXn, const PxcSolverBodyData& _solverBodyData)	const
	{
		return (normal.magnitudeSquared() * _solverBodyData.invMass) + rXn.dot(_solverBodyData.invInertia * rXn);
	}

	PX_FORCE_INLINE	PxReal getResponse(const PxVec3& normal, const PxVec3& rXn, 
									   PxVec3& angDeltaV, const PxcSolverBodyData& _solverBodyData,
									   PxReal invMassScale, PxReal invInertiaScale)	const
	{
		const PxVec3 angDelta = (_solverBodyData.invInertia * rXn) * invInertiaScale;
		angDeltaV = angDelta;
		return (normal.magnitudeSquared() * _solverBodyData.invMass * invMassScale) + rXn.dot(angDelta);
	}

	PX_FORCE_INLINE PxReal projectVelocity(const PxVec3& lin, const PxVec3& ang)	const
	{
		return linearVelocity.dot(lin) + angularVelocity.dot(ang);
	}


	PX_FORCE_INLINE	void applyImpulse(const PxVec3& lin, const PxVec3& ang, const PxcSolverBodyData& _solverBodyData)
	{
		linearVelocity += lin * _solverBodyData.invMass;
		angularVelocity += _solverBodyData.invInertia * ang;
		PX_ASSERT(linearVelocity.isFinite());
		PX_ASSERT(angularVelocity.isFinite());
	}

	/*PX_FORCE_INLINE	void saveVelocity(PxcSolverBodyData& _solverBodyData)
	{
		PX_ASSERT(linearVelocity.isFinite());
		PX_ASSERT(angularVelocity.isFinite());
		_solverBodyData.motionLinearVelocity = linearVelocity;
		_solverBodyData.motionAngularVelocity = angularVelocity;
	}*/
};

#define SOLVER_BODY_SOLVER_PROGRESS_OFFSET 28
#define SOLVER_BODY_MAX_SOLVER_PROGRESS_OFFSET 12
#define SOLVER_BODY_MAX_SOLVER_FRICTION_PROGRESS_OFFSET 14

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverBody) == 32);

}

#endif //PXC_SOLVERBODY_H
