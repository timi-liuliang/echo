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


#ifndef PXS_SOLVERCONTACT_H
#define PXS_SOLVERCONTACT_H

#include "PxvConfig.h"
#include "PxcSolverContact.h"
#include "PxcSolverContactPF.h"
#include "PsMathUtils.h"

namespace physx
{
	namespace Gu
	{
		class ContactBuffer;
	}

struct PxcSolverConstraintDesc;
struct PxcNpWorkUnit;
struct PxcCorrelationBuffer;
struct PxcCorrelationBufferCoulomb;
class PxcNpThreadContext;
struct PxsMaterialInfo;
class PxsContext;
class PxsConstraintBlockManager;
struct PxcSolverContact4Desc;


PX_INLINE void completeContactPoint(PxcSolverContact& c,
							PxReal unitResponse,
							PxReal vrel,
							PxReal recipDt,
							PxReal restitution,
							PxReal bounceThreshold)
{
	PxReal velMultiplier = unitResponse ? 1.0f/unitResponse : 0.0f;
	PxReal scaledBias = c.getScaledBiasPxF32() * (velMultiplier * 0.8f) * recipDt;

	PxReal targetVelocity	= 0.0f;

	if(restitution > 0.0f && vrel < bounceThreshold)
	{
		scaledBias = 0.0f;
		targetVelocity = -vrel * restitution;
	}

	c.setVelMultiplier(velMultiplier);
	c.setScaledBias(scaledBias);
	c.setTargetVelocity(targetVelocity);
	c.setAppliedForce(0.0f);
}

PX_INLINE void completeContactPoint(PxcSolverContactPoint& c,
							PxReal unitResponse,
							PxReal vrel,
							PxReal recipDt,
							PxReal restitution,
							PxReal bounceThreshold)
{
	PxReal velMultiplier = unitResponse ? 1.0f/unitResponse : 0.0f;
	PxReal scaledBias = c.getScaledBiasPxF32() * (velMultiplier * 0.8f) * recipDt;

	PxReal targetVelocity	= 0.0f;

	if(restitution > 0.0f && vrel < bounceThreshold)
	{
		scaledBias = 0.0f;
		targetVelocity = -vrel * restitution;
	}

	c.setVelMultiplier(velMultiplier);
	c.setScaledBias(scaledBias);
	c.setTargetVelocity(targetVelocity);
	c.setAppliedForce(0.0f);
}

PX_INLINE void computeFrictionTangents(const PxVec3& vrel,const PxVec3& unitNormal, PxVec3& t0, PxVec3& t1)
{
	PX_ASSERT(PxAbs(unitNormal.magnitude()-1)<1e-3f);

	t0 = vrel - unitNormal * unitNormal.dot(vrel);
	PxReal ll = t0.magnitudeSquared();

	if (ll > 0.1f)										//can set as low as 0.
	{
		t0 *= PxRecipSqrt(ll);
		t1 = unitNormal.cross(t0);
	}
	else
		Ps::normalToTangents(unitNormal, t0, t1);		//fallback
}

PX_INLINE void completeFrictionRow(PxcSolverContactFriction& f,
							const PxVec3& dir,
							PxReal recipDt)
{
	// in contact processing we stuck ra in f.raXn, rb in f.rbXn, error in normal

    
	f.setRaXn(f.getRaXnPxVec3().cross(dir));
	f.setRbXn(f.getRbXnPxVec3().cross(dir));
	f.delAngVel0_BrokenW = V4SetW(f.delAngVel0_BrokenW, FZero());
	f.setBias(dir.dot(f.getNormalPxVec3()) * recipDt);
	f.setNormal(dir);
	f.setAppliedForce(0.0f);
}


PX_INLINE void completeFrictionRow(PxcSolverFriction& f,
							const PxVec3& dir,
							PxReal recipDt)
{
	PX_UNUSED(recipDt);

	// in contact processing we stuck ra in f.raXn, rb in f.rbXn, error in normal
	f.setRaXn(f.getRaXnPxVec3().cross(dir));
	f.setRbXn(f.getRbXnPxVec3().cross(dir));
	f.setNormal(dir);
	f.setAppliedForce(0.0f);
}

void computeBlockStreamByteSizes4(PxcSolverContact4Desc* descs,
								PxU32& _solverConstraintByteSize, PxU32& _axisConstraintCount,
								PxcNpThreadContext& threadContext);

void setupFinalizeSolverConstraints4(PxcSolverContact4Desc* descs, PxcNpThreadContext& threadContext,const PxReal invDtF32, PxReal bounceThresholdF32);

void setupFinalizeSolverConstraints(PxcNpWorkUnit& n,
							const Gu::ContactBuffer& buffer,
							const PxcCorrelationBuffer& c,
							const PxTransform& bodyFrame0,
							const PxTransform& bodyFrame1,
							bool perPointFriction,
							PxU8* workspace,
							const PxcSolverBody& b0,
							const PxcSolverBody& b1,
							const PxcSolverBodyData& data0,
							const PxcSolverBodyData& data1,
							const PxReal invDtF32,
							PxReal bounceThresholdF32,
							PxReal invMassScale0, PxReal invInertiaScale0, 
							PxReal invMassScale1, PxReal invInertiaScale1);



bool setupFinalizeSolverConstraintsCoulomb(PxcNpWorkUnit& n,
						    const Gu::ContactBuffer& buffer,
							const PxcCorrelationBufferCoulomb& c,
							const PxTransform& bodyFrame0,
							const PxTransform& bodyFrame1,
							bool perPointFriction,
							PxU8* workspace,
							const PxcSolverBody& b0,
							const PxcSolverBody& b1,
							const PxcSolverBodyData& data0,
							const PxcSolverBodyData& data1,
							const PxReal invDtF32,
							PxReal bounceThresholdF32,
							PxU32 frictionCountPerPoint,
							PxReal invMassScale0, PxReal invInertiaScale0, 
							PxReal invMassScale1, PxReal invInertiaScale1);



}

#endif
