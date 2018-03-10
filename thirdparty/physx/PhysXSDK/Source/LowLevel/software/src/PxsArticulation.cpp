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


#include "PsMathUtils.h"
#include "CmConeLimitHelper.h"
#include "PxcSolverConstraintDesc.h"
#include "PxcSolverConstraint1D.h"
#include "PxsArticulation.h"
#include "PxcArticulationHelper.h"
#include "PxsRigidBody.h"
#include "PxcConstraintBlockStream.h"
#include "PxsSolverConstraint1D.h"
#include "PxsSolverConstraintExt.h"
#include "PxsDynamics.h"
#include "PxsContext.h"
#include "PxcArticulationReference.h"

using namespace physx;

PxsArticulation::PxsArticulation()
:	mSolverDesc(NULL)
{
	PX_ASSERT((reinterpret_cast<size_t>(this) & (PXC_ARTICULATION_MAX_SIZE-1))==0);
}

PxsArticulation::~PxsArticulation()
{
}


/* computes the implicit impulse and the drive scale at the joint, in joint coords */


void PxsArticulation::computeResiduals(const Cm::SpatialVector *v, 
									   const PxcArticulationJointTransforms* jointTransforms,
									   bool /*dump*/) const
{
	typedef PxcArticulationFnsScalar Fns;

	PxReal error = 0, energy = 0;
	for(PxU32 i=1;i<mSolverDesc->linkCount;i++)
	{
		const PxcArticulationJointTransforms &b = jointTransforms[i];
		PxU32 parent = mSolverDesc->links[i].parent;
		const PxsArticulationJointCore &j = *mSolverDesc->links[i].inboundJoint;
		PX_UNUSED(j);

		Cm::SpatialVector residual = Fns::translateMotion(mSolverDesc->poses[i].p - b.cB2w.p, v[i])
								   - Fns::translateMotion(mSolverDesc->poses[parent].p - b.cB2w.p, v[parent]);

		error += residual.linear.magnitudeSquared();
		energy += residual.angular.magnitudeSquared();

	}
//	if(dump)
		printf("Energy %f, Error %f\n", energy, error);
}


Cm::SpatialVector PxsArticulation::computeMomentum(const PxcFsInertia *inertia) const
{
	typedef PxcArticulationFnsScalar Fns;

	Cm::SpatialVector *velocity = reinterpret_cast<Cm::SpatialVector*>(getVelocity(*mSolverDesc->fsData));
	Cm::SpatialVector m = Cm::SpatialVector::zero();
	for(PxU32 i=0;i<mSolverDesc->linkCount;i++)
		m += Fns::translateForce(mSolverDesc->poses[i].p - mSolverDesc->poses[0].p, PxcArticulationFnsScalar::multiply(inertia[i], velocity[i]));
	return m;
}



void PxsArticulation::checkLimits() const
{
	for(PxU32 i=1;i<mSolverDesc->linkCount;i++)
	{
		PxTransform cA2w = mSolverDesc->poses[mSolverDesc->links[i].parent].transform(mSolverDesc->links[i].inboundJoint->parentPose);
		PxTransform cB2w = mSolverDesc->poses[i].transform(mSolverDesc->links[i].inboundJoint->childPose);
		
		PxTransform cB2cA = cA2w.transformInv(cB2w);

		// the relative quat must be the short way round for limits to work...

		if(cB2cA.q.w<0)
			cB2cA.q	= -cB2cA.q;

		const PxsArticulationJointCore& j = *mSolverDesc->links[i].inboundJoint;
		
		PxQuat swing, twist;
		if(j.twistLimited || j.swingLimited)
			Ps::separateSwingTwist(cB2cA.q, swing, twist);
		
		if(j.swingLimited)
		{
			PxReal swingLimitContactDistance = PxMin(j.swingYLimit, j.swingZLimit)/4;

			Cm::ConeLimitHelper eh(PxTan(j.swingYLimit/4), 
								   PxTan(j.swingZLimit/4),
								   PxTan(swingLimitContactDistance/4));

			PxVec3 axis;
			PxReal error;
			if(eh.getLimit(swing, axis, error))
				printf("%u, (%f, %f), %f, (%f, %f, %f), %f\n", i, j.swingYLimit, j.swingZLimit, swingLimitContactDistance, axis.x, axis.y, axis.z, error);
		}

//		if(j.twistLimited)
//		{
//			PxReal tqTwistHigh = PxTan(j.twistLimitHigh/4),
//				   tqTwistLow  = PxTan(j.twistLimitLow/4),
//				   twistPad = (tqTwistHigh - tqTwistLow)*0.25f;
//				   //twistPad = j.twistLimitContactDistance;
//
//			PxVec3 axis = jointTransforms[i].cB2w.rotate(PxVec3(1,0,0));
//			PxReal tqPhi = Ps::tanHalf(twist.x, twist.w);
//
//			if(tqPhi < tqTwistLow + twistPad)
//				constraintData.pushBack(ConstraintData(-axis, -(tqTwistLow - tqPhi)*4));
//
//			if(tqPhi > tqTwistHigh - twistPad)
//				constraintData.pushBack(ConstraintData(axis, (tqTwistHigh - tqPhi)*4));
//		}
	}
	puts("");
}

